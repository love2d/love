/*
 * LANES.C   	                          Copyright (c) 2007-08, Asko Kauppi
 *
 * Multithreading in Lua.
 * 
 * History:
 *      20-Oct-08 (2.0.2): Added closing of free-running threads, but it does
 *                  not seem to eliminate the occasional segfaults at process
 *                  exit.
 *          ...
 *      24-Jun-08 .. 14-Aug-08 AKa: Major revise, Lanes 2008 version (2.0 rc1)
 *          ...
 *      18-Sep-06 AKa: Started the module.
 *
 * Platforms (tested internally):
 *      OS X (10.5.4 PowerPC/Intel)
 *      Linux x86 (Ubuntu 8.04)
 *      Win32 (Windows XP Home SP2, Visual C++ 2005/2008 Express)
 *      PocketPC (TBD)
 *
 * Platforms (tested externally):
 *      Win32 (MSYS) by Ross Berteig.
 *
 * Platforms (testers appreciated):
 *      Win64 - should work???
 *      Linux x64 - should work
 *      FreeBSD - should work
 *      QNX - porting shouldn't be hard
 *      Sun Solaris - porting shouldn't be hard
 *
 * References:
 *      "Porting multithreaded applications from Win32 to Mac OS X":
 *      <http://developer.apple.com/macosx/multithreadedprogramming.html>
 *
 *      Pthreads:
 *      <http://vergil.chemistry.gatech.edu/resources/programming/threads.html>
 *
 *      MSDN: <http://msdn2.microsoft.com/en-us/library/ms686679.aspx>
 *
 *      <http://ridiculousfish.com/blog/archives/2007/02/17/barrier>
 *
 * Defines:
 *      -DLINUX_SCHED_RR: all threads are lifted to SCHED_RR category, to
 *          allow negative priorities (-2,-1) be used. Even without this,
 *          using priorities will require 'sudo' privileges on Linux.
 *
 *		-DUSE_PTHREAD_TIMEDJOIN: use 'pthread_timedjoin_np()' for waiting
 *          for threads with a timeout. This changes the thread cleanup
 *          mechanism slightly (cleans up at the join, not once the thread
 *          has finished). May or may not be a good idea to use it.
 *          Available only in selected operating systems (Linux).
 *
 * Bugs:
 *
 * To-do:
 *
 *      ...
 */

const char * VERSION = "2.0.3";

/*
===============================================================================

Copyright (C) 2007-08 Asko Kauppi <akauppi@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

===============================================================================
*/
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"

#include "threading.h"
#include "tools.h"

#if !((defined PLATFORM_WIN32) || (defined PLATFORM_POCKETPC))
# include <sys/time.h>
#endif

/* geteuid() */
#ifdef PLATFORM_LINUX
# include <unistd.h>
# include <sys/types.h>
#endif

/* The selected number is not optimal; needs to be tested. Even using just
* one keeper state may be good enough (depends on the number of Lindas used
* in the applications).
*/
#define KEEPER_STATES_N 1   // 6

/* Do you want full call stacks, or just the line where the error happened?
*
* TBD: The full stack feature does not seem to work (try 'make error').
*/
#define ERROR_FULL_STACK

#ifdef ERROR_FULL_STACK
# define STACK_TRACE_KEY ((void*)lane_error)     // used as registry key
#endif

/*
* Lua code for the keeper states (baked in)
*/
static char keeper_chunk[]= 
#include "keeper.lch"

struct s_lane;
static bool_t cancel_test( lua_State *L );
static void cancel_error( lua_State *L );

#define CANCEL_TEST_KEY ((void*)cancel_test)    // used as registry key
#define CANCEL_ERROR ((void*)cancel_error)      // 'cancel_error' sentinel

/*
* registry[FINALIZER_REG_KEY] is either nil (no finalizers) or a table
* of functions that Lanes will call after the executing 'pcall' has ended.
*
* We're NOT using the GC system for finalizer mainly because providing the
* error (and maybe stack trace) parameters to the finalizer functions would
* anyways complicate that approach.
*/
#define FINALIZER_REG_KEY ((void*)LG_set_finalizer)

struct s_Linda;

#if 1
# define DEBUG_SIGNAL( msg, signal_ref ) /* */
#else
# define DEBUG_SIGNAL( msg, signal_ref ) \
    { int i; unsigned char *ptr; char buf[999]; \
      sprintf( buf, ">>> " msg ": %p\t", (signal_ref) ); \
      ptr= (unsigned char *)signal_ref; \
      for( i=0; i<sizeof(*signal_ref); i++ ) { \
        sprintf( strchr(buf,'\0'), "%02x %c ", ptr[i], ptr[i] ); \
      } \
      fprintf( stderr, "%s\n", buf ); \
    }
#endif

static bool_t thread_cancel( struct s_lane *s, double secs, bool_t force );


/*
* Push a table stored in registry onto Lua stack.
*
* If there is no existing table, create one if 'create' is TRUE.
* 
* Returns: TRUE if a table was pushed
*          FALSE if no table found, not created, and nothing pushed
*/
static bool_t push_registry_table( lua_State *L, void *key, bool_t create ) {

    STACK_GROW(L,3);
    
    lua_pushlightuserdata( L, key );
    lua_gettable( L, LUA_REGISTRYINDEX );
    
    if (lua_isnil(L,-1)) {
        lua_pop(L,1);

        if (!create) return FALSE;  // nothing pushed

        lua_newtable(L);
        lua_pushlightuserdata( L, key );
        lua_pushvalue(L,-2);    // duplicate of the table
        lua_settable( L, LUA_REGISTRYINDEX );
        
        // [-1]: table that's also bound in registry
    }
    return TRUE;    // table pushed
}


/*---=== Serialize require ===---
*/

static MUTEX_T require_cs;

//---
// [val]= new_require( ... )
//
// Call 'old_require' but only one lane at a time.
//
// Upvalues: [1]: original 'require' function
//
static int new_require( lua_State *L ) {
    int rc;
    int args= lua_gettop(L);

  STACK_GROW(L,1);
  STACK_CHECK(L)
    
    // Using 'lua_pcall()' to catch errors; otherwise a failing 'require' would
    // leave us locked, blocking any future 'require' calls from other lanes.
    //
    MUTEX_LOCK( &require_cs );
    {
        lua_pushvalue( L, lua_upvalueindex(1) );
        lua_insert( L, 1 );

        rc= lua_pcall( L, args, 1 /*retvals*/, 0 /*errfunc*/ );
            //
            // LUA_ERRRUN / LUA_ERRMEM
    }
    MUTEX_UNLOCK( &require_cs );

    if (rc) lua_error(L);   // error message already at [-1]

  STACK_END(L,0)
    return 1;
}

/*
* Serialize calls to 'require', if it exists
*/
static 
void serialize_require( lua_State *L ) {

  STACK_GROW(L,1);  
  STACK_CHECK(L)
    
    // Check 'require' is there; if not, do nothing
    //
    lua_getglobal( L, "require" );
    if (lua_isfunction( L, -1 )) {
        // [-1]: original 'require' function

        lua_pushcclosure( L, new_require, 1 /*upvalues*/ );
        lua_setglobal( L, "require" );

    } else {
        // [-1]: nil
        lua_pop(L,1);
    }

  STACK_END(L,0)
}


/*---=== Keeper states ===---
*/

/*
* Pool of keeper states
*
* Access to keeper states is locked (only one OS thread at a time) so the 
* bigger the pool, the less chances of unnecessary waits. Lindas map to the
* keepers randomly, by a hash.
*/
struct s_Keeper {
    MUTEX_T lock_;
    lua_State *L;
} keeper[ KEEPER_STATES_N ];

/* We could use an empty table in 'keeper.lua' as the sentinel, but maybe
* checking for a lightuserdata is faster.
*/
static bool_t nil_sentinel;

/*
* Initialize keeper states
*
* If there is a problem, return an error message (NULL for okay).
*
* Note: Any problems would be design flaws; the created Lua state is left
*       unclosed, because it does not really matter. In production code, this
*       function never fails.
*/
static const char *init_keepers(void) {
    unsigned int i;
    for( i=0; i<KEEPER_STATES_N; i++ ) {
        
        // Initialize Keeper states with bare minimum of libs (those required
        // by 'keeper.lua')
        //
        lua_State *L= luaL_newstate();
        if (!L) return "out of memory";

        luaG_openlibs( L, "io,table" );     // 'io' for debugging messages

        lua_pushlightuserdata( L, &nil_sentinel );
        lua_setglobal( L, "nil_sentinel" );

        // Read in the preloaded chunk (and run it)
        //
        if (luaL_loadbuffer( L, keeper_chunk, sizeof(keeper_chunk), "=lanes_keeper" ))
            return "luaL_loadbuffer() failed";   // LUA_ERRMEM

        if (lua_pcall( L, 0 /*args*/, 0 /*results*/, 0 /*errfunc*/ )) {
            // LUA_ERRRUN / LUA_ERRMEM / LUA_ERRERR
            //
            const char *err= lua_tostring(L,-1);
            assert(err);
            return err;
        }

        MUTEX_INIT( &keeper[i].lock_ );
        keeper[i].L= L;
    }
    return NULL;    // ok
}

static 
struct s_Keeper *keeper_acquire( const void *ptr ) {
    /*
    * Any hashing will do that maps pointers to 0..KEEPER_STATES_N-1 
    * consistently.
    *
    * Pointers are often aligned by 8 or so - ignore the low order bits
    */
    unsigned int i= ((unsigned long)(ptr) >> 3) % KEEPER_STATES_N;
    struct s_Keeper *K= &keeper[i];

    MUTEX_LOCK( &K->lock_ );
    return K;
}

static 
void keeper_release( struct s_Keeper *K ) {
    MUTEX_UNLOCK( &K->lock_ );
}

/*
* Call a function ('func_name') in the keeper state, and pass on the returned
* values to 'L'.
*
* 'linda':          deep Linda pointer (used only as a unique table key, first parameter)
* 'starting_index': first of the rest of parameters (none if 0)
*
* Returns:  number of return values (pushed to 'L')
*/
static
int keeper_call( lua_State* K, const char *func_name, 
                  lua_State *L, struct s_Linda *linda, uint_t starting_index ) {

    int args= starting_index ? (lua_gettop(L) - starting_index +1) : 0;
    int Ktos= lua_gettop(K);
    int retvals;

    lua_getglobal( K, func_name );
    ASSERT_L( lua_isfunction(K,-1) );

    STACK_GROW( K, 1 );
    lua_pushlightuserdata( K, linda );

    luaG_inter_copy( L,K, args );   // L->K
    lua_call( K, 1+args, LUA_MULTRET );

    retvals= lua_gettop(K) - Ktos;

    luaG_inter_move( K,L, retvals );    // K->L
    return retvals;
}


/*---=== Linda ===---
*/

/*
* Actual data is kept within a keeper state, which is hashed by the 's_Linda'
* pointer (which is same to all userdatas pointing to it).
*/
struct s_Linda {
    SIGNAL_T read_happened;
    SIGNAL_T write_happened;
};

static int LG_linda_id( lua_State* );

#define lua_toLinda(L,n) ((struct s_Linda *)luaG_todeep( L, LG_linda_id, n ))


/*
* bool= linda_send( linda_ud, [timeout_secs=-1,] key_num|str|bool|lightuserdata, ... )
*
* Send one or more values to a Linda. If there is a limit, all values must fit.
*
* Returns:  'true' if the value was queued
*           'false' for timeout (only happens when the queue size is limited)
*/
LUAG_FUNC( linda_send ) {
    struct s_Linda *linda= lua_toLinda( L, 1 );
    bool_t ret;
    bool_t cancel= FALSE;
    struct s_Keeper *K;
    time_d timeout= -1.0;
    uint_t key_i= 2;    // index of first key, if timeout not there

    if (lua_isnumber(L,2)) {
        timeout= SIGNAL_TIMEOUT_PREPARE( lua_tonumber(L,2) );
        key_i++;
    } else if (lua_isnil(L,2))
        key_i++;

    if (lua_isnil(L,key_i))
        luaL_error( L, "nil key" );

    STACK_GROW(L,1);

    K= keeper_acquire( linda );
    {
        lua_State *KL= K->L;    // need to do this for 'STACK_CHECK'
STACK_CHECK(KL)
        while(TRUE) {
            int pushed;
        
STACK_MID(KL,0)
            pushed= keeper_call( K->L, "send", L, linda, key_i );
            ASSERT_L( pushed==1 );
        
            ret= lua_toboolean(L,-1);
            lua_pop(L,1);
        
            if (ret) {
                // Wake up ALL waiting threads
                //
                SIGNAL_ALL( &linda->write_happened );
                break;

            } else if (timeout==0.0) {
                break;  /* no wait; instant timeout */

            } else {
                /* limit faced; push until timeout */
                    
                cancel= cancel_test( L );   // testing here causes no delays
                if (cancel) break;

                // K lock will be released for the duration of wait and re-acquired
                //
                if (!SIGNAL_WAIT( &linda->read_happened, &K->lock_, timeout ))
                    break;  // timeout
            }
        }
STACK_END(KL,0)
    }
    keeper_release(K);

    if (cancel)
        cancel_error(L);
    
    lua_pushboolean( L, ret );
    return 1;
}


/*
* [val, key]= linda_receive( linda_ud, [timeout_secs_num=-1], key_num|str|bool|lightuserdata [, ...] )
*
* Receive a value from Linda, consuming it.
*
* Returns:  value received (which is consumed from the slot)
*           key which had it
*/
LUAG_FUNC( linda_receive ) {
    struct s_Linda *linda= lua_toLinda( L, 1 );
    int pushed;
    bool_t cancel= FALSE;
    struct s_Keeper *K;
    time_d timeout= -1.0;
    uint_t key_i= 2;

    if (lua_isnumber(L,2)) {
        timeout= SIGNAL_TIMEOUT_PREPARE( lua_tonumber(L,2) );
        key_i++;
    } else if (lua_isnil(L,2))
        key_i++;

    K= keeper_acquire( linda );
    {
        while(TRUE) {
            pushed= keeper_call( K->L, "receive", L, linda, key_i );
            if (pushed) {
                ASSERT_L( pushed==2 );

                // To be done from within the 'K' locking area
                //
                SIGNAL_ALL( &linda->read_happened );
                break;

            } else if (timeout==0.0) {
                break;  /* instant timeout */

            } else {    /* nothing received; wait until timeout */
    
                cancel= cancel_test( L );   // testing here causes no delays
                if (cancel) break;

                // Release the K lock for the duration of wait, and re-acquire
                //
                if (!SIGNAL_WAIT( &linda->write_happened, &K->lock_, timeout ))
                    break;
            }
        }
    }
    keeper_release(K);

    if (cancel)
        cancel_error(L);

    return pushed;
}


/*
* = linda_set( linda_ud, key_num|str|bool|lightuserdata [,value] )
*
* Set a value to Linda.
*
* Existing slot value is replaced, and possible queue entries removed.
*/
LUAG_FUNC( linda_set ) {
    struct s_Linda *linda= lua_toLinda( L, 1 );
    bool_t has_value= !lua_isnil(L,3);

    struct s_Keeper *K= keeper_acquire( linda );
    {
        int pushed= keeper_call( K->L, "set", L, linda, 2 );
        ASSERT_L( pushed==0 );

        /* Set the signal from within 'K' locking.
        */
        if (has_value) {
            SIGNAL_ALL( &linda->write_happened );
        }
    }
    keeper_release(K);

    return 0;
}


/*
* [val]= linda_get( linda_ud, key_num|str|bool|lightuserdata )
*
* Get a value from Linda.
*/
LUAG_FUNC( linda_get ) {
    struct s_Linda *linda= lua_toLinda( L, 1 );
    int pushed;

    struct s_Keeper *K= keeper_acquire( linda );
    {
        pushed= keeper_call( K->L, "get", L, linda, 2 );
        ASSERT_L( pushed==0 || pushed==1 );
    }
    keeper_release(K);

    return pushed;
}


/*
* = linda_limit( linda_ud, key_num|str|bool|lightuserdata, uint [, ...] )
*
* Set limits to 1 or more Linda keys.
*/
LUAG_FUNC( linda_limit ) {
    struct s_Linda *linda= lua_toLinda( L, 1 );

    struct s_Keeper *K= keeper_acquire( linda );
    {
        int pushed= keeper_call( K->L, "limit", L, linda, 2 );
        ASSERT_L( pushed==0 );
    }
    keeper_release(K);

    return 0;
}


/*
* lightuserdata= linda_deep( linda_ud )
*
* Return the 'deep' userdata pointer, identifying the Linda.
*
* This is needed for using Lindas as key indices (timer system needs it);
* separately created proxies of the same underlying deep object will have
* different userdata and won't be known to be essentially the same deep one
* without this.
*/
LUAG_FUNC( linda_deep ) {
    struct s_Linda *linda= lua_toLinda( L, 1 );
    lua_pushlightuserdata( L, linda );      // just the address
    return 1;
}


/*
* Identity function of a shared userdata object.
* 
*   lightuserdata= linda_id( "new" [, ...] )
*   = linda_id( "delete", lightuserdata )
*
* Creation and cleanup of actual 'deep' objects. 'luaG_...' will wrap them into
* regular userdata proxies, per each state using the deep data.
*
*   tbl= linda_id( "metatable" )
*
* Returns a metatable for the proxy objects ('__gc' method not needed; will
* be added by 'luaG_...')
*
*   = linda_id( str, ... )
*
* For any other strings, the ID function must not react at all. This allows
* future extensions of the system. 
*/
LUAG_FUNC( linda_id ) {
    const char *which= lua_tostring(L,1);

    if (strcmp( which, "new" )==0) {
        struct s_Linda *s;

        /* We don't use any parameters, but one could (they're at [2..TOS])
        */
        ASSERT_L( lua_gettop(L)==1 );

        /* The deep data is allocated separately of Lua stack; we might no
        * longer be around when last reference to it is being released.
        * One can use any memory allocation scheme.
        */
        s= (struct s_Linda *) malloc( sizeof(struct s_Linda) );
        ASSERT_L(s);

        SIGNAL_INIT( &s->read_happened );
        SIGNAL_INIT( &s->write_happened );

        lua_pushlightuserdata( L, s );
        return 1;

    } else if (strcmp( which, "delete" )==0) {
        struct s_Keeper *K;
        struct s_Linda *s= lua_touserdata(L,2);
        ASSERT_L(s);

        /* Clean associated structures in the keeper state.
        */
        K= keeper_acquire(s);
        {
            keeper_call( K->L, "clear", L, s, 0 );
        }
        keeper_release(K);

        /* There aren't any lanes waiting on these lindas, since all proxies
        * have been gc'ed. Right?
        */
        SIGNAL_FREE( &s->read_happened );
        SIGNAL_FREE( &s->write_happened );
        free(s);

        return 0;

    } else if (strcmp( which, "metatable" )==0) {

      STACK_CHECK(L)
        lua_newtable(L);
        lua_newtable(L);
            //
            // [-2]: linda metatable
            // [-1]: metatable's to-be .__index table
    
        lua_pushcfunction( L, LG_linda_send );
        lua_setfield( L, -2, "send" );
    
        lua_pushcfunction( L, LG_linda_receive );
        lua_setfield( L, -2, "receive" );
    
        lua_pushcfunction( L, LG_linda_limit );
        lua_setfield( L, -2, "limit" );

        lua_pushcfunction( L, LG_linda_set );
        lua_setfield( L, -2, "set" );
    
        lua_pushcfunction( L, LG_linda_get );
        lua_setfield( L, -2, "get" );

        lua_pushcfunction( L, LG_linda_deep );
        lua_setfield( L, -2, "deep" );

        lua_setfield( L, -2, "__index" );
      STACK_END(L,1)
    
        return 1;
    }
    
    return 0;   // unknown request, be quiet
}


/*---=== Finalizer ===---
*/

//---
// void= finalizer( finalizer_func )
//
// finalizer_func( [err, stack_tbl] )
//
// Add a function that will be called when exiting the lane, either via
// normal return or an error.
//
LUAG_FUNC( set_finalizer )
{
    STACK_GROW(L,3);
    
    // Get the current finalizer table (if any)
    //
    push_registry_table( L, FINALIZER_REG_KEY, TRUE /*do create if none*/ );

    lua_pushinteger( L, lua_objlen(L,-1)+1 );
    lua_pushvalue( L, 1 );  // copy of the function
    lua_settable( L, -3 );
    
    lua_pop(L,1);
    return 0;
}


//---
// Run finalizers - if any - with the given parameters
//
// If 'rc' is nonzero, error message and stack index are available as:
//      [-1]: stack trace (table)
//      [-2]: error message (any type)
//
// Returns:
//      0 if finalizers were run without error (or there were none)
//      LUA_ERRxxx return code if any of the finalizers failed
//
// TBD: should we add stack trace on failing finalizer, wouldn't be hard..
//
static int run_finalizers( lua_State *L, int lua_rc )
{
    unsigned error_index, tbl_index;
    unsigned n;
    int rc= 0;
    
    if (!push_registry_table(L, FINALIZER_REG_KEY, FALSE /*don't create one*/))
        return 0;   // no finalizers

    tbl_index= lua_gettop(L);
    error_index= (lua_rc!=0) ? tbl_index-1 : 0;   // absolute indices

    STACK_GROW(L,4);

    // [-1]: { func [, ...] }
    //
    for( n= lua_objlen(L,-1); n>0; n-- ) {
        unsigned args= 0;
        lua_pushinteger( L,n );
        lua_gettable( L, -2 );
        
        // [-1]: function
        // [-2]: finalizers table

        if (error_index) {
            lua_pushvalue( L, error_index );
            lua_pushvalue( L, error_index+1 );  // stack trace
            args= 2;
        }

        rc= lua_pcall( L, args, 0 /*retvals*/, 0 /*no errfunc*/ );
            //
            // LUA_ERRRUN / LUA_ERRMEM
    
        if (rc!=0) {
            // [-1]: error message
            //
            // If one finalizer fails, don't run the others. Return this
            // as the 'real' error, preceding that we could have had (or not)
            // from the actual code.
            //
            break;
        }
    }
    
    lua_remove(L,tbl_index);   // take finalizer table out of stack

    return rc;
}


/*---=== Threads ===---
*/

// NOTE: values to be changed by either thread, during execution, without
//       locking, are marked "volatile"
//
struct s_lane {
    THREAD_T thread;
        //
        // M: sub-thread OS thread
        // S: not used

    lua_State *L;
        //
        // M: prepares the state, and reads results
        // S: while S is running, M must keep out of modifying the state

    volatile enum e_status status;
        // 
        // M: sets to PENDING (before launching)
        // S: updates -> RUNNING/WAITING -> DONE/ERROR_ST/CANCELLED
    
    volatile bool_t cancel_request;
        //
        // M: sets to FALSE, flags TRUE for cancel request
        // S: reads to see if cancel is requested

#if !( (defined PLATFORM_WIN32) || (defined PLATFORM_POCKETPC) || (defined PTHREAD_TIMEDJOIN) )
    SIGNAL_T done_signal_;
        //
        // M: Waited upon at lane ending  (if Posix with no PTHREAD_TIMEDJOIN)
        // S: sets the signal once cancellation is noticed (avoids a kill)

    MUTEX_T done_lock_;
        // 
        // Lock required by 'done_signal' condition variable, protecting
        // lane status changes to DONE/ERROR_ST/CANCELLED.
#endif

    volatile enum { 
        NORMAL,         // normal master side state
        KILLED          // issued an OS kill
    } mstatus;
        //
        // M: sets to NORMAL, if issued a kill changes to KILLED
        // S: not used
        
    struct s_lane * volatile selfdestruct_next;
        //
        // M: sets to non-NULL if facing lane handle '__gc' cycle but the lane
        //    is still running
        // S: cleans up after itself if non-NULL at lane exit
};

static MUTEX_T selfdestruct_cs;
    //
    // Protects modifying the selfdestruct chain

#define SELFDESTRUCT_END ((struct s_lane *)(-1))
    //
    // The chain is ended by '(struct s_lane*)(-1)', not NULL:
    //      'selfdestruct_first -> ... -> ... -> (-1)'

struct s_lane * volatile selfdestruct_first= SELFDESTRUCT_END;

/*
* Add the lane to selfdestruct chain; the ones still running at the end of the
* whole process will be cancelled.
*/
static void selfdestruct_add( struct s_lane *s ) {

    MUTEX_LOCK( &selfdestruct_cs );
    {
        assert( s->selfdestruct_next == NULL );

        s->selfdestruct_next= selfdestruct_first;
        selfdestruct_first= s;
    }
    MUTEX_UNLOCK( &selfdestruct_cs );
}

/*
* A free-running lane has ended; remove it from selfdestruct chain
*/
static void selfdestruct_remove( struct s_lane *s ) {

    MUTEX_LOCK( &selfdestruct_cs );
    {
        // Make sure (within the MUTEX) that we actually are in the chain
        // still (at process exit they will remove us from chain and then
        // cancel/kill).
        //
        if (s->selfdestruct_next != NULL) {
            struct s_lane **ref= (struct s_lane **) &selfdestruct_first;
            bool_t found= FALSE;
    
            while( *ref != SELFDESTRUCT_END ) {
                if (*ref == s) {
                    *ref= s->selfdestruct_next;
                    s->selfdestruct_next= NULL;
                    found= TRUE;
                    break;
                }
                ref= (struct s_lane **) &((*ref)->selfdestruct_next);
            }
            assert( found );
        }
    }
    MUTEX_UNLOCK( &selfdestruct_cs );
}

/*
* Process end; cancel any still free-running threads
*/
static void selfdestruct_atexit( void ) {

    if (selfdestruct_first == SELFDESTRUCT_END) return;    // no free-running threads

    // Signal _all_ still running threads to exit
    //
    MUTEX_LOCK( &selfdestruct_cs );
    {
        struct s_lane *s= selfdestruct_first;
        while( s != SELFDESTRUCT_END ) {
            s->cancel_request= TRUE;
            s= s->selfdestruct_next;
        }
    }
    MUTEX_UNLOCK( &selfdestruct_cs );

    // When noticing their cancel, the lanes will remove themselves from
    // the selfdestruct chain.

    // TBD: Not sure if Windows (multi core) will require the timed approach,
    //      or single Yield. I don't have machine to test that (so leaving
    //      for timed approach).    -- AKa 25-Oct-2008
 
#ifdef PLATFORM_LINUX
    // It seems enough for Linux to have a single yield here, which allows
    // other threads (timer lane) to proceed. Without the yield, there is
    // segfault.
    //
    YIELD();
#else
    // OS X 10.5 (Intel) needs more to avoid segfaults.
    //
    // "make test" is okay. 100's of "make require" are okay.
    //
    // Tested on MacBook Core Duo 2GHz and 10.5.5:
    //  -- AKa 25-Oct-2008
    //
    #ifndef ATEXIT_WAIT_SECS
    # define ATEXIT_WAIT_SECS (0.1)
    #endif
    {
        double t_until= now_secs() + ATEXIT_WAIT_SECS;
    
        while( selfdestruct_first != SELFDESTRUCT_END ) {
            YIELD();    // give threads time to act on their cancel
            
            if (now_secs() >= t_until) break;
        }
    }
#endif

    //---
    // Kill the still free running threads
    //
    if ( selfdestruct_first != SELFDESTRUCT_END ) {
        unsigned n=0;
        MUTEX_LOCK( &selfdestruct_cs );
        {
            struct s_lane *s= selfdestruct_first;
            while( s != SELFDESTRUCT_END ) {
                n++;
                s= s->selfdestruct_next;
            }
        }
        MUTEX_UNLOCK( &selfdestruct_cs );

    // Linux (at least 64-bit): CAUSES A SEGFAULT IF THIS BLOCK IS ENABLED
    //       and works without the block (so let's leave those lanes running)
    //
#if 1
        // 2.0.2: at least timer lane is still here
        //
        //fprintf( stderr, "Left %d lane(s) with cancel request at process end.\n", n );
#else
        MUTEX_LOCK( &selfdestruct_cs );
        {
            struct s_lane *s= selfdestruct_first;
            while( s != SELFDESTRUCT_END ) {
                struct s_lane *next_s= s->selfdestruct_next;
                s->selfdestruct_next= NULL;     // detach from selfdestruct chain

                THREAD_KILL( &s->thread );
                s= next_s;
                n++;
            }
            selfdestruct_first= SELFDESTRUCT_END;
        }
        MUTEX_UNLOCK( &selfdestruct_cs );

        fprintf( stderr, "Killed %d lane(s) at process end.\n", n );
#endif
    }
}


// To allow free-running threads (longer lifespan than the handle's)
// 'struct s_lane' are malloc/free'd and the handle only carries a pointer.
// This is not deep userdata since the handle's not portable among lanes.
//
#define lua_toLane(L,i)  (* ((struct s_lane**) lua_touserdata(L,i)))


/*
* Check if the thread in question ('L') has been signalled for cancel.
*
* Called by cancellation hooks and/or pending Linda operations (because then
* the check won't affect performance).
*
* Returns TRUE if any locks are to be exited, and 'cancel_error()' called,
* to make execution of the lane end.
*/
static bool_t cancel_test( lua_State *L ) {
    struct s_lane *s;

    STACK_GROW(L,1);

  STACK_CHECK(L)
    lua_pushlightuserdata( L, CANCEL_TEST_KEY );
    lua_rawget( L, LUA_REGISTRYINDEX );
    s= lua_touserdata( L, -1 );     // lightuserdata (true 's_lane' pointer) / nil
    lua_pop(L,1);
  STACK_END(L,0)

    // 's' is NULL for the original main state (no-one can cancel that)
    //
    return s && s->cancel_request;
}

static void cancel_error( lua_State *L ) {
    STACK_GROW(L,1);
    lua_pushlightuserdata( L, CANCEL_ERROR );    // special error value
    lua_error(L);   // no return
}

static void cancel_hook( lua_State *L, lua_Debug *ar ) {
    (void)ar;
    if (cancel_test(L)) cancel_error(L);
}


//---
// = _single( [cores_uint=1] )
//
// Limits the process to use only 'cores' CPU cores. To be used for performance
// testing on multicore devices. DEBUGGING ONLY!
//
LUAG_FUNC( _single ) {
	uint_t cores= luaG_optunsigned(L,1,1);

#ifdef PLATFORM_OSX
  #ifdef _UTILBINDTHREADTOCPU
	if (cores > 1) luaL_error( L, "Limiting to N>1 cores not possible." );
    // requires 'chudInitialize()'
    utilBindThreadToCPU(0);     // # of CPU to run on (we cannot limit to 2..N CPUs?)
  #else
    luaL_error( L, "Not available: compile with _UTILBINDTHREADTOCPU" );
  #endif
#else
    luaL_error( L, "not implemented!" );
#endif
	(void)cores;
	
	return 0;
}


/*
* str= lane_error( error_val|str )
*
* Called if there's an error in some lane; add call stack to error message 
* just like 'lua.c' normally does.
*
* ".. will be called with the error message and its return value will be the 
*     message returned on the stack by lua_pcall."
*
* Note: Rather than modifying the error message itself, it would be better
*     to provide the call stack (as string) completely separated. This would
*     work great with non-string error values as well (current system does not).
*     (This is NOT possible with the Lua 5.1 'lua_pcall()'; we could of course
*     implement a Lanes-specific 'pcall' of our own that does this). TBD!!! :)
*       --AKa 22-Jan-2009
*/
#ifdef ERROR_FULL_STACK

static int lane_error( lua_State *L ) {
    lua_Debug ar;
    unsigned lev,n;

    // [1]: error message (any type)

    assert( lua_gettop(L)==1 );

    // Don't do stack survey for cancelled lanes.
    //
#if 1
    if (lua_touserdata(L,1) == CANCEL_ERROR)
        return 1;   // just pass on
#endif

    // Place stack trace at 'registry[lane_error]' for the 'luc_pcall()'
    // caller to fetch. This bypasses the Lua 5.1 limitation of only one
    // return value from error handler to 'lua_pcall()' caller.

    // It's adequate to push stack trace as a table. This gives the receiver
    // of the stack best means to format it to their liking. Also, it allows
    // us to add more stack info later, if needed.
    //
    // table of { "sourcefile.lua:<line>", ... }
    //
    STACK_GROW(L,3);
    lua_newtable(L);

    // Best to start from level 1, but in some cases it might be a C function
    // and we don't get '.currentline' for that. It's okay - just keep level
    // and table index growing separate.    --AKa 22-Jan-2009
    //
    lev= 0;
    n=1;
    while( lua_getstack(L, ++lev, &ar ) ) {
        lua_getinfo(L, "Sl", &ar);
        if (ar.currentline > 0) {
            lua_pushinteger( L, n++ );
            lua_pushfstring( L, "%s:%d", ar.short_src, ar.currentline );
            lua_settable( L, -3 );
        }
    }

    lua_pushlightuserdata( L, STACK_TRACE_KEY );
    lua_insert(L,-2);
    lua_settable( L, LUA_REGISTRYINDEX );

    assert( lua_gettop(L)== 1 );

    return 1;   // the untouched error value
}
#endif


//---
#if (defined PLATFORM_WIN32) || (defined PLATFORM_POCKETPC)
  static THREAD_RETURN_T __stdcall lane_main( void *vs )
#else
  static THREAD_RETURN_T lane_main( void *vs )
#endif
{
    struct s_lane *s= (struct s_lane *)vs;
    int rc, rc2;
    lua_State *L= s->L;

    s->status= RUNNING;  // PENDING -> RUNNING

    // Tie "set_finalizer()" to the state
    //
    lua_pushcfunction( L, LG_set_finalizer );
    lua_setglobal( L, "set_finalizer" );

#ifdef ERROR_FULL_STACK
    STACK_GROW( L, 1 );
    lua_pushcfunction( L, lane_error );
    lua_insert( L, 1 );

    // [1]: error handler
    // [2]: function to run
    // [3..top]: parameters
    //
    rc= lua_pcall( L, lua_gettop(L)-2, LUA_MULTRET, 1 /*error handler*/ );
        // 0: no error
        // LUA_ERRRUN: a runtime error (error pushed on stack)
        // LUA_ERRMEM: memory allocation error
        // LUA_ERRERR: error while running the error handler (if any)

    assert( rc!=LUA_ERRERR );   // since we've authored it

    lua_remove(L,1);    // remove error handler

    // Lua 5.1 error handler is limited to one return value; taking stack trace
    // via registry
    //
    if (rc!=0) {    
        STACK_GROW(L,1);
        lua_pushlightuserdata( L, STACK_TRACE_KEY );
        lua_gettable(L, LUA_REGISTRYINDEX);

        // For cancellation, a stack trace isn't placed
        //
        assert( lua_istable(L,2) || (lua_touserdata(L,1)==CANCEL_ERROR) );
        
        // Just leaving the stack trace table on the stack is enough to get
        // it through to the master.
    }

#else
    // This code does not use 'lane_error'
    //
    // [1]: function to run
    // [2..top]: parameters
    //
    rc= lua_pcall( L, lua_gettop(L)-1, LUA_MULTRET, 0 /*no error handler*/ );
        // 0: no error
        // LUA_ERRRUN: a runtime error (error pushed on stack)
        // LUA_ERRMEM: memory allocation error
#endif

//STACK_DUMP(L);
    // Call finalizers, if the script has set them up.
    //
    rc2= run_finalizers(L,rc);
    if (rc2!=0) {
        // Error within a finalizer!  
        // 
        // [-1]: error message

        rc= rc2;    // we're overruling the earlier script error or normal return

        lua_insert( L,1 );  // make error message [1]
        lua_settop( L,1 );  // remove all rest

        // Place an empty stack table just to keep the API simple (always when
        // there's an error, there's also stack table - though it may be empty).
        //
        lua_newtable(L);
    }

    if (s->selfdestruct_next != NULL) {
        // We're a free-running thread and no-one's there to clean us up.
        //
        lua_close( s->L );
        L= 0;

    #if !( (defined PLATFORM_WIN32) || (defined PLATFORM_POCKETPC) || (defined PTHREAD_TIMEDJOIN) )
        SIGNAL_FREE( &s->done_signal_ );
        MUTEX_FREE( &s->done_lock_ );
    #endif
        selfdestruct_remove(s);     // away from selfdestruct chain
        free(s);

    } else {
        // leave results (1..top) or error message + stack trace (1..2) on the stack - master will copy them

        enum e_status st= 
            (rc==0) ? DONE 
                    : (lua_touserdata(L,1)==CANCEL_ERROR) ? CANCELLED 
                    : ERROR_ST;

        // Posix no PTHREAD_TIMEDJOIN:
        // 		'done_lock' protects the -> DONE|ERROR_ST|CANCELLED state change
        //
    #if (defined PLATFORM_WIN32) || (defined PLATFORM_POCKETPC) || (defined PTHREAD_TIMEDJOIN)
        s->status= st;
    #else
        MUTEX_LOCK( &s->done_lock_ );
        {
            s->status= st;
            SIGNAL_ONE( &s->done_signal_ );   // wake up master (while 's->done_lock' is on)
        }
        MUTEX_UNLOCK( &s->done_lock_ );
    #endif
    }

    return 0;   // ignored
}


//---
// lane_ud= thread_new( function, [libs_str], 
//                          [cancelstep_uint=0], 
//                          [prio_int=0],
//                          [globals_tbl],
//                          [... args ...] )
//
// Upvalues: metatable to use for 'lane_ud'
//
LUAG_FUNC( thread_new )
{
    lua_State *L2;
    struct s_lane *s;
    struct s_lane **ud;

    const char *libs= lua_tostring( L, 2 );
    uint_t cs= luaG_optunsigned( L, 3,0);
    int prio= luaL_optinteger( L, 4,0);
    uint_t glob= luaG_isany(L,5) ? 5:0;

    #define FIXED_ARGS (5)
    uint_t args= lua_gettop(L) - FIXED_ARGS;

    if (prio < THREAD_PRIO_MIN || prio > THREAD_PRIO_MAX) {
        luaL_error( L, "Priority out of range: %d..+%d (%d)", 
                            THREAD_PRIO_MIN, THREAD_PRIO_MAX, prio );
    }

    /* --- Create and prepare the sub state --- */

    L2 = luaL_newstate();   // uses standard 'realloc()'-based allocator,
                            // sets the panic callback

    if (!L2) luaL_error( L, "'luaL_newstate()' failed; out of memory" );

    STACK_GROW( L,2 );

    // Setting the globals table (needs to be done before loading stdlibs,
    // and the lane function)
    //
    if (glob!=0) {
STACK_CHECK(L)
        if (!lua_istable(L,glob)) 
            luaL_error( L, "Expected table, got %s", luaG_typename(L,glob) );

        lua_pushvalue( L, glob );
        luaG_inter_move( L,L2, 1 );     // moves the table to L2

        // L2 [-1]: table of globals

        // "You can change the global environment of a Lua thread using lua_replace"
        // (refman-5.0.pdf p. 30) 
        //
        lua_replace( L2, LUA_GLOBALSINDEX );
STACK_END(L,0)
    }

    // Selected libraries
    //
    if (libs) {
        const char *err= luaG_openlibs( L2, libs );
        ASSERT_L( !err );   // bad libs should have been noticed by 'lanes.lua'

        serialize_require( L2 );
    }

    // Lane main function
    //
STACK_CHECK(L)
    lua_pushvalue( L, 1 );
    luaG_inter_move( L,L2, 1 );    // L->L2
STACK_MID(L,0)

    ASSERT_L( lua_gettop(L2) == 1 );
    ASSERT_L( lua_isfunction(L2,1) );

    // revive arguments
    //
    if (args) luaG_inter_copy( L,L2, args );    // L->L2
STACK_MID(L,0)

ASSERT_L( (uint_t)lua_gettop(L2) == 1+args );
ASSERT_L( lua_isfunction(L2,1) );

    // 's' is allocated from heap, not Lua, since its life span may surpass 
    // the handle's (if free running thread)
    //
    ud= lua_newuserdata( L, sizeof(struct s_lane*) );
    ASSERT_L(ud);

    s= *ud= malloc( sizeof(struct s_lane) );
    ASSERT_L(s);

    //memset( s, 0, sizeof(struct s_lane) );
    s->L= L2;
    s->status= PENDING;
    s->cancel_request= FALSE;

#if !( (defined PLATFORM_WIN32) || (defined PLATFORM_POCKETPC) || (defined PTHREAD_TIMEDJOIN) )
    MUTEX_INIT( &s->done_lock_ );
    SIGNAL_INIT( &s->done_signal_ );
#endif
    s->mstatus= NORMAL;
    s->selfdestruct_next= NULL;

    // Set metatable for the userdata
    //
    lua_pushvalue( L, lua_upvalueindex(1) );
    lua_setmetatable( L, -2 );
STACK_MID(L,1)

    // Place 's' to registry, for 'cancel_test()' (even if 'cs'==0 we still
    // do cancel tests at pending send/receive).
    //
    lua_pushlightuserdata( L2, CANCEL_TEST_KEY );
    lua_pushlightuserdata( L2, s );
    lua_rawset( L2, LUA_REGISTRYINDEX );

    if (cs) {
        lua_sethook( L2, cancel_hook, LUA_MASKCOUNT, cs );
    }

    THREAD_CREATE( &s->thread, lane_main, s, prio );
STACK_END(L,1)

    return 1;
}


//---
// = thread_gc( lane_ud )
//
// Cleanup for a thread userdata. If the thread is still executing, leave it
// alive as a free-running thread (will clean up itself).
//
// * Why NOT cancel/kill a loose thread: 
//
// At least timer system uses a free-running thread, they should be handy
// and the issue of cancelling/killing threads at gc is not very nice, either
// (would easily cause waits at gc cycle, which we don't want).
//
// * Why YES kill a loose thread:
//
// Current way causes segfaults at program exit, if free-running threads are
// in certain stages. Details are not clear, but this is the core reason.
// If gc would kill threads then at process exit only one thread would remain.
//
// Todo: Maybe we should have a clear #define for selecting either behaviour.
//
LUAG_FUNC( thread_gc ) {
    struct s_lane *s= lua_toLane(L,1);

    // We can read 's->status' without locks, but not wait for it
    //
    if (s->status < DONE) {
        //
        selfdestruct_add(s);
        assert( s->selfdestruct_next );
        return 0;

    } else if (s->mstatus==KILLED) {
        // Make sure a kill has proceeded, before cleaning up the data structure.
        //
        // If not doing 'THREAD_WAIT()' we should close the Lua state here
        // (can it be out of order, since we killed the lane abruptly?)
        //
#if 0
        lua_close( s->L );
#else
fprintf( stderr, "** Joining with a killed thread (needs testing) **" );
#if (defined PLATFORM_WIN32) || (defined PLATFORM_POCKETPC) || (defined PTHREAD_TIMEDJOIN)
        THREAD_WAIT( &s->thread, -1 );
#else
        THREAD_WAIT( &s->thread, &s->done_signal_, &s->done_lock_, &s->status, -1 );
#endif
fprintf( stderr, "** Joined ok **" );
#endif
    }

    // Clean up after a (finished) thread
    //
#if (! ((defined PLATFORM_WIN32) || (defined PLATFORM_POCKETPC) || (defined PTHREAD_TIMEDJOIN)))
    SIGNAL_FREE( &s->done_signal_ );
    MUTEX_FREE( &s->done_lock_ );
    free(s);
#endif

    return 0;
}


//---
// = thread_cancel( lane_ud [,timeout_secs=0.0] [,force_kill_bool=false] )
//
// The originator thread asking us specifically to cancel the other thread.
//
// 'timeout': <0: wait forever, until the lane is finished
//            0.0: just signal it to cancel, no time waited
//            >0: time to wait for the lane to detect cancellation
//
// 'force_kill': if true, and lane does not detect cancellation within timeout,
//            it is forcefully killed. Using this with 0.0 timeout means just kill
//            (unless the lane is already finished).
//
// Returns: true if the lane was already finished (DONE/ERROR_ST/CANCELLED) or if we
//          managed to cancel it.
//          false if the cancellation timed out, or a kill was needed.
//
LUAG_FUNC( thread_cancel )
{
    struct s_lane *s= lua_toLane(L,1);
    double secs= 0.0;
    uint_t force_i=2;
    bool_t force, done= TRUE;
    
    if (lua_isnumber(L,2)) {
        secs= lua_tonumber(L,2);
        force_i++;
    } else if (lua_isnil(L,2))
        force_i++;

    force= lua_toboolean(L,force_i);     // FALSE if nothing there
    
    // We can read 's->status' without locks, but not wait for it (if Posix no PTHREAD_TIMEDJOIN)
    //
    if (s->status < DONE) {
        s->cancel_request= TRUE;    // it's now signalled to stop

        done= thread_cancel( s, secs, force );
    }

    lua_pushboolean( L, done );
    return 1;
}

static bool_t thread_cancel( struct s_lane *s, double secs, bool_t force )
{
    bool_t done= 
#if (defined PLATFORM_WIN32) || (defined PLATFORM_POCKETPC) || (defined PTHREAD_TIMEDJOIN)
        THREAD_WAIT( &s->thread, secs );
#else
        THREAD_WAIT( &s->thread, &s->done_signal_, &s->done_lock_, &s->status, secs );
#endif

    if ((!done) && force) {
        // Killing is asynchronous; we _will_ wait for it to be done at
        // GC, to make sure the data structure can be released (alternative
        // would be use of "cancellation cleanup handlers" that at least
        // PThread seems to have).
        //
        THREAD_KILL( &s->thread );
        s->mstatus= KILLED;     // mark 'gc' to wait for it
    }
    return done;
}


//---
// str= thread_status( lane_ud )
//
// Returns: "pending"   not started yet
//          -> "running"   started, doing its work..
//             <-> "waiting"   blocked in a receive()
//                -> "done"     finished, results are there
//                   / "error"     finished at an error, error value is there
//                   / "cancelled"   execution cancelled by M (state gone)
//
LUAG_FUNC( thread_status )
{
    struct s_lane *s= lua_toLane(L,1);
    enum e_status st= s->status;    // read just once (volatile)
    const char *str;
    
    if (s->mstatus == KILLED)
        st= CANCELLED;

    str= (st==PENDING) ? "pending" :
         (st==RUNNING) ? "running" :    // like in 'co.status()'
         (st==WAITING) ? "waiting" :
         (st==DONE) ? "done" :
         (st==ERROR_ST) ? "error" :
         (st==CANCELLED) ? "cancelled" : NULL;
    ASSERT_L(str);

    lua_pushstring( L, str );
    return 1;
}


//---
// [...] | [nil, err_any, stack_tbl]= thread_join( lane_ud [, wait_secs=-1] )
//
//  timeout:   returns nil
//  done:      returns return values (0..N)
//  error:     returns nil + error value + stack table
//  cancelled: returns nil
//
LUAG_FUNC( thread_join )
{
    struct s_lane *s= lua_toLane(L,1);
    double wait_secs= luaL_optnumber(L,2,-1.0);
    lua_State *L2= s->L;
    int ret;

    bool_t done= 
#if (defined PLATFORM_WIN32) || (defined PLATFORM_POCKETPC) || (defined PTHREAD_TIMEDJOIN)
        THREAD_WAIT( &s->thread, wait_secs );
#else
        THREAD_WAIT( &s->thread, &s->done_signal_, &s->done_lock_, &s->status, wait_secs );
#endif
    if (!done)
        return 0;      // timeout: pushes none, leaves 'L2' alive

    // Thread is DONE/ERROR_ST/CANCELLED; all ours now

    STACK_GROW( L, 1 );

    switch( s->status ) {
        case DONE: {   
            uint_t n= lua_gettop(L2);       // whole L2 stack
            luaG_inter_move( L2,L, n );
            ret= n;
            } break;

        case ERROR_ST:
            lua_pushnil(L);
            luaG_inter_move( L2,L, 2 );    // error message at [-2], stack trace at [-1]
            ret= 3;
            break;

        case CANCELLED:
            ret= 0;
            break;
        
        default:
            fprintf( stderr, "Status: %d\n", s->status );
            ASSERT_L( FALSE ); ret= 0;
    }
    lua_close(L2);

    return ret;
}


/*---=== Timer support ===---
*/

/*
* Push a timer gateway Linda object; only one deep userdata is
* created for this, each lane will get its own proxy.
*
* Note: this needs to be done on the C side; Lua wouldn't be able
*       to even see, when we've been initialized for the very first
*       time (with us, they will be).
*/
static
void push_timer_gateway( lua_State *L ) {

    /* No need to lock; 'static' is just fine
    */
    static DEEP_PRELUDE *p;  // = NULL

  STACK_CHECK(L)
    if (!p) {
        // Create the Linda (only on first time)
        //
        // proxy_ud= deep_userdata( idfunc )
        //
        lua_pushcfunction( L, luaG_deep_userdata );
        lua_pushcfunction( L, LG_linda_id );
        lua_call( L, 1 /*args*/, 1 /*retvals*/ );

        ASSERT_L( lua_isuserdata(L,-1) );
        
        // Proxy userdata contents is only a 'DEEP_PRELUDE*' pointer
        //
        p= * (DEEP_PRELUDE**) lua_touserdata( L, -1 );
        ASSERT_L(p && p->refcount==1 && p->deep);

        // [-1]: proxy for accessing the Linda

    } else {
        /* Push a proxy based on the deep userdata we stored. 
        */
        luaG_push_proxy( L, LG_linda_id, p );
    }
  STACK_END(L,1)
}

/*
* secs= now_secs()
*
* Returns the current time, as seconds (millisecond resolution).
*/
LUAG_FUNC( now_secs )
{
    lua_pushnumber( L, now_secs() );
    return 1;
}

/*
* wakeup_at_secs= wakeup_conv( date_tbl )
*/
LUAG_FUNC( wakeup_conv )
{
    int year, month, day, hour, min, sec, isdst;
    struct tm tm= {0};
        //
        // .year (four digits)
        // .month (1..12)
        // .day (1..31)
        // .hour (0..23)
        // .min (0..59)
        // .sec (0..61)
        // .yday (day of the year)
        // .isdst (daylight saving on/off)

  STACK_CHECK(L)    
    lua_getfield( L, 1, "year" ); year= lua_tointeger(L,-1); lua_pop(L,1);
    lua_getfield( L, 1, "month" ); month= lua_tointeger(L,-1); lua_pop(L,1);
    lua_getfield( L, 1, "day" ); day= lua_tointeger(L,-1); lua_pop(L,1);
    lua_getfield( L, 1, "hour" ); hour= lua_tointeger(L,-1); lua_pop(L,1);
    lua_getfield( L, 1, "min" ); min= lua_tointeger(L,-1); lua_pop(L,1);
    lua_getfield( L, 1, "sec" ); sec= lua_tointeger(L,-1); lua_pop(L,1);

    // If Lua table has '.isdst' we trust that. If it does not, we'll let
    // 'mktime' decide on whether the time is within DST or not (value -1).
    //
    lua_getfield( L, 1, "isdst" );
    isdst= lua_isboolean(L,-1) ? lua_toboolean(L,-1) : -1;
    lua_pop(L,1);
  STACK_END(L,0)

    tm.tm_year= year-1900;
    tm.tm_mon= month-1;     // 0..11
    tm.tm_mday= day;        // 1..31
    tm.tm_hour= hour;       // 0..23
    tm.tm_min= min;         // 0..59
    tm.tm_sec= sec;         // 0..60
    tm.tm_isdst= isdst;     // 0/1/negative

    lua_pushnumber( L, (double) mktime( &tm ) );   // ms=0
    return 1;
}


/*---=== Module linkage ===---
*/

#define REG_FUNC( name ) \
    lua_pushcfunction( L, LG_##name ); \
    lua_setglobal( L, #name )

#define REG_FUNC2( name, val ) \
    lua_pushcfunction( L, val ); \
    lua_setglobal( L, #name )

#define REG_STR2( name, val ) \
    lua_pushstring( L, val ); \
    lua_setglobal( L, #name )

#define REG_INT2( name, val ) \
    lua_pushinteger( L, val ); \
    lua_setglobal( L, #name )


int 
#if (defined PLATFORM_WIN32) || (defined PLATFORM_POCKETPC)
__declspec(dllexport)
#endif
	luaopen_lanes( lua_State *L ) {
    const char *err;
    static volatile char been_here;  // =0

    // One time initializations:
    //
    if (!been_here) {
        been_here= TRUE;

#if (defined PLATFORM_WIN32) || (defined PLATFORM_POCKETPC)
        now_secs();     // initialize 'now_secs()' internal offset
#endif

#if (defined PLATFORM_OSX) && (defined _UTILBINDTHREADTOCPU)
        chudInitialize();
#endif
    
        // Locks for 'tools.c' inc/dec counters
        //
        MUTEX_INIT( &deep_lock );
        MUTEX_INIT( &mtid_lock );
    
        // Serialize calls to 'require' from now on, also in the primary state
        //
        MUTEX_RECURSIVE_INIT( &require_cs );

        serialize_require( L );

        // Selfdestruct chain handling
        //
        MUTEX_INIT( &selfdestruct_cs );
        atexit( selfdestruct_atexit );

        //---
        // Linux needs SCHED_RR to change thread priorities, and that is only
        // allowed for sudo'ers. SCHED_OTHER (default) has no priorities.
        // SCHED_OTHER threads are always lower priority than SCHED_RR.
        //
        // ^-- those apply to 2.6 kernel.  IF **wishful thinking** these 
        //     constraints will change in the future, non-sudo priorities can 
        //     be enabled also for Linux.
        //
#ifdef PLATFORM_LINUX
        sudo= geteuid()==0;     // we are root?

        // If lower priorities (-2..-1) are wanted, we need to lift the main
        // thread to SCHED_RR and 50 (medium) level. Otherwise, we're always below 
        // the launched threads (even -2).
	    //
  #ifdef LINUX_SCHED_RR
        if (sudo) {
            struct sched_param sp= {0}; sp.sched_priority= _PRIO_0;
            PT_CALL( pthread_setschedparam( pthread_self(), SCHED_RR, &sp) );
        }
  #endif
#endif
        err= init_keepers();
        if (err) 
            luaL_error( L, "Unable to initialize: %s", err );
    }
    
    // Linda identity function
    //
    REG_FUNC( linda_id );

    // metatable for threads
    //
    lua_newtable( L );
    lua_pushcfunction( L, LG_thread_gc );
    lua_setfield( L, -2, "__gc" );

    lua_pushcclosure( L, LG_thread_new, 1 );    // metatable as closure param
    lua_setglobal( L, "thread_new" );

    REG_FUNC( thread_status );
    REG_FUNC( thread_join );
    REG_FUNC( thread_cancel );

    REG_STR2( _version, VERSION );
    REG_FUNC( _single );

    REG_FUNC2( _deep_userdata, luaG_deep_userdata );

    REG_FUNC( now_secs );
    REG_FUNC( wakeup_conv );

    push_timer_gateway(L);    
    lua_setglobal( L, "timer_gateway" );

    REG_INT2( max_prio, THREAD_PRIO_MAX );

    lua_pushlightuserdata( L, CANCEL_ERROR );
    lua_setglobal( L, "cancel_error" );

    return 0;
}


