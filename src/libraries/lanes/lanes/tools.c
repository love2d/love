/*
 * TOOLS.C   	                    Copyright (c) 2002-08, Asko Kauppi
 *
 * Lua tools to support Lanes.
*/

/*
===============================================================================

Copyright (C) 2002-08 Asko Kauppi <akauppi@gmail.com>

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

#include "tools.h"

#include "lualib.h"
#include "lauxlib.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static volatile lua_CFunction hijacked_tostring;     // = NULL

MUTEX_T deep_lock;
MUTEX_T mtid_lock;

/*---=== luaG_dump ===---*/

void luaG_dump( lua_State* L ) {

    int top= lua_gettop(L);
    int i;

	fprintf( stderr, "\n\tDEBUG STACK:\n" );

	if (top==0)
		fprintf( stderr, "\t(none)\n" );

	for( i=1; i<=top; i++ ) {
		int type= lua_type( L, i );

		fprintf( stderr, "\t[%d]= (%s) ", i, lua_typename(L,type) );

		// Print item contents here...
		//
		// Note: this requires 'tostring()' to be defined. If it is NOT,
		//       enable it for more debugging.
		//
    STACK_CHECK(L)
        STACK_GROW( L, 2 )

        lua_getglobal( L, "tostring" );
            //
            // [-1]: tostring function, or nil
        
        if (!lua_isfunction(L,-1)) {
             fprintf( stderr, "('tostring' not available)" );
         } else {
             lua_pushvalue( L, i );
             lua_call( L, 1 /*args*/, 1 /*retvals*/ );

             // Don't trust the string contents
             //                
             fprintf( stderr, "%s", lua_tostring(L,-1) );
         }
         lua_pop(L,1);
    STACK_END(L,0)
		fprintf( stderr, "\n" );
		}
	fprintf( stderr, "\n" );
}


/*---=== luaG_openlibs ===---*/

static const luaL_Reg libs[] = {
  { LUA_LOADLIBNAME, luaopen_package },
  { LUA_TABLIBNAME, luaopen_table },
  { LUA_IOLIBNAME, luaopen_io },
  { LUA_OSLIBNAME, luaopen_os },
  { LUA_STRLIBNAME, luaopen_string },
  { LUA_MATHLIBNAME, luaopen_math },
  { LUA_DBLIBNAME, luaopen_debug },
  //
  { "base", NULL },         // ignore "base" (already acquired it)
  { "coroutine", NULL },    // part of Lua 5.1 base package
  { NULL, NULL }
};

static bool_t openlib( lua_State *L, const char *name, size_t len ) {

    unsigned i;
    bool_t all= strncmp( name, "*", len ) == 0;

    for( i=0; libs[i].name; i++ ) {
        if (all || (strncmp(name, libs[i].name, len) ==0)) {
            if (libs[i].func) {
                STACK_GROW(L,2);
                lua_pushcfunction( L, libs[i].func );
                lua_pushstring( L, libs[i].name );
                lua_call( L, 1, 0 );
            }
            if (!all) return TRUE;
        }
    }
    return all;
}

/* 
* Like 'luaL_openlibs()' but allows the set of libraries be selected
*
*   NULL    no libraries, not even base
*   ""      base library only
*   "io,string"     named libraries
*   "*"     all libraries
*
* Base ("unpack", "print" etc.) is always added, unless 'libs' is NULL.
*
* Returns NULL for ok, position of error within 'libs' on failure.
*/
#define is_name_char(c) (isalpha(c) || (c)=='*')

const char *luaG_openlibs( lua_State *L, const char *libs ) {
    const char *p;
    unsigned len;

	if (!libs) return NULL;     // no libs, not even 'base'

    // 'lua.c' stops GC during initialization so perhaps its a good idea. :)
    //
    lua_gc(L, LUA_GCSTOP, 0);

    // Anything causes 'base' to be taken in
    //
    STACK_GROW(L,2);
    lua_pushcfunction( L, luaopen_base );
    lua_pushliteral( L, "" );
    lua_call( L, 1, 0 );

    for( p= libs; *p; p+=len ) {
        len=0;
        while (*p && !is_name_char(*p)) p++;    // bypass delimiters
        while (is_name_char(p[len])) len++;     // bypass name
        if (len && (!openlib( L, p, len )))
            break;
    }
    lua_gc(L, LUA_GCRESTART, 0);

    return *p ? p : NULL;
}



/*---=== Deep userdata ===---*/

/* The deep portion must be allocated separately of any Lua state's; it's
* lifespan may be longer than that of the creating state.
*/
#define DEEP_MALLOC malloc
#define DEEP_FREE   free

/* 
* 'registry[REGKEY]' is a two-way lookup table for 'idfunc's and those type's
* metatables:
*
*   metatable   ->  idfunc
*   idfunc      ->  metatable
*/
#define DEEP_LOOKUP_KEY ((void*)set_deep_lookup)
    // any unique light userdata

static void push_registry_subtable( lua_State *L, void *token );

/*
* Sets up [-1]<->[-2] two-way lookups, and ensures the lookup table exists.
* Pops the both values off the stack.
*/
void set_deep_lookup( lua_State *L ) {

    STACK_GROW(L,3);

  STACK_CHECK(L)
#if 1
    push_registry_subtable( L, DEEP_LOOKUP_KEY );
#else
    /* ..to be removed.. */
    lua_pushlightuserdata( L, DEEP_LOOKUP_KEY );
    lua_rawget( L, LUA_REGISTRYINDEX );

    if (lua_isnil(L,-1)) {
        // First time here; let's make the lookup
        //
        lua_pop(L,1);

        lua_newtable(L);
        lua_pushlightuserdata( L, DEEP_LOOKUP_KEY );
        lua_pushvalue(L,-2);
            //
            // [-3]: {} (2nd ref)
            // [-2]: DEEP_LOOKUP_KEY
            // [-1]: {}

        lua_rawset( L, LUA_REGISTRYINDEX );
            //
            // [-1]: lookup table (empty)
    }
#endif
  STACK_MID(L,1)

    lua_insert(L,-3);

    // [-3]: lookup table
    // [-2]: A
    // [-1]: B
    
    lua_pushvalue( L,-1 );  // B
    lua_pushvalue( L,-3 );  // A
    lua_rawset( L, -5 );    // B->A
    lua_rawset( L, -3 );    // A->B
    lua_pop( L,1 );

  STACK_END(L,-2)
}

/*
* Pops the key (metatable or idfunc) off the stack, and replaces with the
* deep lookup value (idfunc/metatable/nil).
*/
void get_deep_lookup( lua_State *L ) {
    
    STACK_GROW(L,1);

  STACK_CHECK(L)    
    lua_pushlightuserdata( L, DEEP_LOOKUP_KEY );
    lua_rawget( L, LUA_REGISTRYINDEX );
    
    if (!lua_isnil(L,-1)) {
        // [-2]: key (metatable or idfunc)
        // [-1]: lookup table
    
        lua_insert( L, -2 );
        lua_rawget( L, -2 );
    
        // [-2]: lookup table
        // [-1]: value (metatable / idfunc / nil)
    }    
    lua_remove(L,-2);
        // remove lookup, or unused key
  STACK_END(L,0)
}

/*
* Return the registered ID function for 'index' (deep userdata proxy),
* or NULL if 'index' is not a deep userdata proxy.
*/
static
lua_CFunction get_idfunc( lua_State *L, int index ) {
    lua_CFunction ret;

    index= STACK_ABS(L,index);

    STACK_GROW(L,1);

  STACK_CHECK(L)
    if (!lua_getmetatable( L, index ))
        return NULL;    // no metatable
    
    // [-1]: metatable of [index]

    get_deep_lookup(L);
        //    
        // [-1]: idfunc/nil

    ret= lua_tocfunction(L,-1);
    lua_pop(L,1);
  STACK_END(L,0)
    return ret;
}


/*
* void= mt.__gc( proxy_ud )
*
* End of life for a proxy object; reduce the deep reference count and clean
* it up if reaches 0.
*/
static
int deep_userdata_gc( lua_State *L ) {
    DEEP_PRELUDE **proxy= (DEEP_PRELUDE**)lua_touserdata( L, 1 );
    DEEP_PRELUDE *p= *proxy;
    int v;

    *proxy= 0;  // make sure we don't use it any more

    MUTEX_LOCK( &deep_lock );
      v= --(p->refcount);
    MUTEX_UNLOCK( &deep_lock );

    if (v==0) {
        int pushed;

        // Call 'idfunc( "delete", deep_ptr )' to make deep cleanup
        //
        lua_CFunction idfunc= get_idfunc(L,1);
        ASSERT_L(idfunc);
        
        lua_settop(L,0);    // clean stack so we can call 'idfunc' directly

        // void= idfunc( "delete", lightuserdata )
        //
        lua_pushliteral( L, "delete" );
        lua_pushlightuserdata( L, p->deep );
        pushed= idfunc(L);
        
        if (pushed)
            luaL_error( L, "Bad idfunc on \"delete\": returned something" );

        DEEP_FREE( (void*)p );
    }
    return 0;
}


/*
* Push a proxy userdata on the stack.
*
* Initializes necessary structures if it's the first time 'idfunc' is being
* used in this Lua state (metatable, registring it). Otherwise, increments the
* reference count.
*/
void luaG_push_proxy( lua_State *L, lua_CFunction idfunc, DEEP_PRELUDE *prelude ) {
    DEEP_PRELUDE **proxy;

    MUTEX_LOCK( &deep_lock );
      ++(prelude->refcount);  // one more proxy pointing to this deep data
    MUTEX_UNLOCK( &deep_lock );

    STACK_GROW(L,4);

  STACK_CHECK(L)

    proxy= lua_newuserdata( L, sizeof( DEEP_PRELUDE* ) );
    ASSERT_L(proxy);
    *proxy= prelude;

    // Get/create metatable for 'idfunc' (in this state)
    //
    lua_pushcfunction( L, idfunc );    // key
    get_deep_lookup(L);
        //
        // [-2]: proxy
        // [-1]: metatable / nil
    
    if (lua_isnil(L,-1)) {
        // No metatable yet; make one and register it
        //
        lua_pop(L,1);

        // tbl= idfunc( "metatable" )
        //
        lua_pushcfunction( L, idfunc );
        lua_pushliteral( L, "metatable" );
        lua_call( L, 1 /*args*/, 1 /*results*/ );
            //
            // [-2]: proxy
            // [-1]: metatable (returned by 'idfunc')

        if (!lua_istable(L,-1))
            luaL_error( L, "Bad idfunc on \"metatable\": did not return one" );

        // Add '__gc' method
        //
        lua_pushcfunction( L, deep_userdata_gc );
        lua_setfield( L, -2, "__gc" );

        // Memorize for later rounds
        //
        lua_pushvalue( L,-1 );
        lua_pushcfunction( L, idfunc );
            //
            // [-4]: proxy
            // [-3]: metatable (2nd ref)
            // [-2]: metatable
            // [-1]: idfunc

        set_deep_lookup(L);
    } 
  STACK_MID(L,2)
    ASSERT_L( lua_isuserdata(L,-2) );
    ASSERT_L( lua_istable(L,-1) );

    // [-2]: proxy userdata
    // [-1]: metatable to use

    lua_setmetatable( L, -2 );
    
  STACK_END(L,1)
    // [-1]: proxy userdata
}


/*
* Create a deep userdata
*
*   proxy_ud= deep_userdata( idfunc [, ...] )
*
* Creates a deep userdata entry of the type defined by 'idfunc'.
* Other parameters are passed on to the 'idfunc' "new" invocation.
*
* 'idfunc' must fulfill the following features:
*
*   lightuserdata= idfunc( "new" [, ...] )      -- creates a new deep data instance
*   void= idfunc( "delete", lightuserdata )     -- releases a deep data instance
*   tbl= idfunc( "metatable" )          -- gives metatable for userdata proxies
*
* Reference counting and true userdata proxying are taken care of for the
* actual data type.
*
* Types using the deep userdata system (and only those!) can be passed between
* separate Lua states via 'luaG_inter_move()'.
*
* Returns:  'proxy' userdata for accessing the deep data via 'luaG_todeep()'
*/
int luaG_deep_userdata( lua_State *L ) {
    lua_CFunction idfunc= lua_tocfunction( L,1 );
    int pushed;

    DEEP_PRELUDE *prelude= DEEP_MALLOC( sizeof(DEEP_PRELUDE) );
    ASSERT_L(prelude);

    prelude->refcount= 0;   // 'luaG_push_proxy' will lift it to 1

    STACK_GROW(L,1);
  STACK_CHECK(L)

    // Replace 'idfunc' with "new" in the stack (keep possible other params)
    //
    lua_remove(L,1);
    lua_pushliteral( L, "new" );
    lua_insert(L,1);

    // lightuserdata= idfunc( "new" [, ...] )
    //
    pushed= idfunc(L);

    if ((pushed!=1) || lua_type(L,-1) != LUA_TLIGHTUSERDATA)
        luaL_error( L, "Bad idfunc on \"new\": did not return light userdata" );

    prelude->deep= lua_touserdata(L,-1);
    ASSERT_L(prelude->deep);

    lua_pop(L,1);   // pop deep data

    luaG_push_proxy( L, idfunc, prelude );
        //
        // [-1]: proxy userdata

  STACK_END(L,1)
    return 1;
}


/*
* Access deep userdata through a proxy.
*
* Reference count is not changed, and access to the deep userdata is not
* serialized. It is the module's responsibility to prevent conflicting usage.
*/
void *luaG_todeep( lua_State *L, lua_CFunction idfunc, int index ) {
    DEEP_PRELUDE **proxy;

  STACK_CHECK(L)
    if (get_idfunc(L,index) != idfunc)
        return NULL;    // no metatable, or wrong kind

    proxy= (DEEP_PRELUDE**)lua_touserdata( L, index );
  STACK_END(L,0)

    return (*proxy)->deep;
}


/*
* Copy deep userdata between two separate Lua states.
*
* Returns:
*   the id function of the copied value, or NULL for non-deep userdata
*   (not copied)
*/
static
lua_CFunction luaG_copydeep( lua_State *L, lua_State *L2, int index ) {
    DEEP_PRELUDE **proxy;
    DEEP_PRELUDE *p;

    lua_CFunction idfunc;
    
    idfunc= get_idfunc( L, index );
    if (!idfunc) return NULL;   // not a deep userdata

    // Increment reference count
    //
    proxy= (DEEP_PRELUDE**)lua_touserdata( L, index );
    p= *proxy;

    luaG_push_proxy( L2, idfunc, p );
        //
        // L2 [-1]: proxy userdata

    return idfunc;
}



/*---=== Inter-state copying ===---*/

/*-- Metatable copying --*/

/*
 * 'reg[ REG_MT_KNOWN ]'= {
 *      [ table ]= id_uint,
 *          ...
 *      [ id_uint ]= table,
 *          ...
 * }
 */

/*
* Push a registry subtable (keyed by unique 'token') onto the stack.
* If the subtable does not exist, it is created and chained.
*/
static
void push_registry_subtable( lua_State *L, void *token ) {

    STACK_GROW(L,3);

  STACK_CHECK(L)
    
    lua_pushlightuserdata( L, token );
    lua_rawget( L, LUA_REGISTRYINDEX );
        //
        // [-1]: nil/subtable
    
    if (lua_isnil(L,-1)) {
        lua_pop(L,1);
        lua_newtable(L);                    // value
        lua_pushlightuserdata( L, token );  // key
        lua_pushvalue(L,-2);
            //
            // [-3]: value (2nd ref)
            // [-2]: key
            // [-1]: value

        lua_rawset( L, LUA_REGISTRYINDEX );
    }
  STACK_END(L,1)

    ASSERT_L( lua_istable(L,-1) );
}

#define REG_MTID ( (void*) get_mt_id )

/*
* Get a unique ID for metatable at [i].
*/
static
uint_t get_mt_id( lua_State *L, int i ) {
    static uint_t last_id= 0;
    uint_t id;

    i= STACK_ABS(L,i);

    STACK_GROW(L,3);

  STACK_CHECK(L)
    push_registry_subtable( L, REG_MTID );
    lua_pushvalue(L, i);
    lua_rawget( L, -2 );
        //
        // [-2]: reg[REG_MTID]
        // [-1]: nil/uint
    
    id= lua_tointeger(L,-1);    // 0 for nil
    lua_pop(L,1);
  STACK_MID(L,1)
    
    if (id==0) {
        MUTEX_LOCK( &mtid_lock );
            id= ++last_id;
        MUTEX_UNLOCK( &mtid_lock );

        /* Create two-way references: id_uint <-> table
        */
        lua_pushvalue(L,i);
        lua_pushinteger(L,id);
        lua_rawset( L, -3 );
        
        lua_pushinteger(L,id);
        lua_pushvalue(L,i);
        lua_rawset( L, -3 );
    }
    lua_pop(L,1);     // remove 'reg[REG_MTID]' reference

  STACK_END(L,0)
  
    return id;
}


static int buf_writer( lua_State *L, const void* b, size_t n, void* B ) {
  (void)L;
  luaL_addlstring((luaL_Buffer*) B, (const char *)b, n);
  return 0;
}


/* 
 * Check if we've already copied the same table from 'L', and
 * reuse the old copy. This allows table upvalues shared by multiple
 * local functions to point to the same table, also in the target.
 *
 * Always pushes a table to 'L2'.
 *
 * Returns TRUE if the table was cached (no need to fill it!); FALSE if
 * it's a virgin.
 */
static
bool_t push_cached_table( lua_State *L2, uint_t L2_cache_i, lua_State *L, uint_t i ) {
    bool_t ret;

    ASSERT_L( hijacked_tostring );
    ASSERT_L( L2_cache_i != 0 );

    STACK_GROW(L,2);
    STACK_GROW(L2,3);

    // Create an identity string for table at [i]; it should stay unique at
    // least during copying of the data (then we can clear the caches).
    //
  STACK_CHECK(L)
    lua_pushcfunction( L, hijacked_tostring );
    lua_pushvalue( L, i );
    lua_call( L, 1 /*args*/, 1 /*retvals*/ );
        //
        // [-1]: "table: 0x...."

  STACK_END(L,1)
    ASSERT_L( lua_type(L,-1) == LUA_TSTRING );

    // L2_cache[id_str]= [{...}]
    //
  STACK_CHECK(L2)

    // We don't need to use the from state ('L') in ID since the life span
    // is only for the duration of a copy (both states are locked).
    //
    lua_pushstring( L2, lua_tostring(L,-1) );
    lua_pop(L,1);   // remove the 'tostring(tbl)' value (in L!)

//fprintf( stderr, "<< ID: %s >>\n", lua_tostring(L2,-1) );

    lua_pushvalue( L2, -1 );
    lua_rawget( L2, L2_cache_i );
        //
        // [-2]: identity string ("table: 0x...")
        // [-1]: table|nil

    if (lua_isnil(L2,-1)) {
        lua_pop(L2,1);
        lua_newtable(L2);
        lua_pushvalue(L2,-1);
        lua_insert(L2,-3);
            //
            // [-3]: new table (2nd ref)
            // [-2]: identity string
            // [-1]: new table

        lua_rawset(L2, L2_cache_i);
            //
            // [-1]: new table (tied to 'L2_cache' table')

        ret= FALSE;     // brand new
        
    } else {
        lua_remove(L2,-2);
        ret= TRUE;      // from cache
    }
  STACK_END(L2,1)
    //
    // L2 [-1]: table to use as destination

    ASSERT_L( lua_istable(L2,-1) );
    return ret;
}


/* 
 * Check if we've already copied the same function from 'L', and reuse the old
 * copy.
 *
 * Always pushes a function to 'L2'.
 */
static void inter_copy_func( lua_State *L2, uint_t L2_cache_i, lua_State *L, uint_t i );

static
void push_cached_func( lua_State *L2, uint_t L2_cache_i, lua_State *L, uint_t i ) {
    // TBD: Merge this and same code for tables

    ASSERT_L( hijacked_tostring );
    ASSERT_L( L2_cache_i != 0 );

    STACK_GROW(L,2);
    STACK_GROW(L2,3);

  STACK_CHECK(L)
    lua_pushcfunction( L, hijacked_tostring );
    lua_pushvalue( L, i );
    lua_call( L, 1 /*args*/, 1 /*retvals*/ );
        //
        // [-1]: "function: 0x...."

  STACK_END(L,1)
    ASSERT_L( lua_type(L,-1) == LUA_TSTRING );

    // L2_cache[id_str]= function
    //
  STACK_CHECK(L2)

    // We don't need to use the from state ('L') in ID since the life span
    // is only for the duration of a copy (both states are locked).
    //
    lua_pushstring( L2, lua_tostring(L,-1) );
    lua_pop(L,1);   // remove the 'tostring(tbl)' value (in L!)

//fprintf( stderr, "<< ID: %s >>\n", lua_tostring(L2,-1) );

    lua_pushvalue( L2, -1 );
    lua_rawget( L2, L2_cache_i );
        //
        // [-2]: identity string ("function: 0x...")
        // [-1]: function|nil|true  (true means: we're working on it; recursive)

    if (lua_isnil(L2,-1)) {
        lua_pop(L2,1);
        
        // Set to 'true' for the duration of creation; need to find self-references
        // via upvalues
        //
        lua_pushboolean(L2,TRUE);
        lua_setfield( L2, L2_cache_i, lua_tostring(L2,-2) );        

        inter_copy_func( L2, L2_cache_i, L, i );    // pushes a copy of the func

        lua_pushvalue(L2,-1);
        lua_insert(L2,-3);
            //
            // [-3]: function (2nd ref)
            // [-2]: identity string
            // [-1]: function

        lua_rawset(L2,L2_cache_i);
            //
            // [-1]: function (tied to 'L2_cache' table')
        
    } else if (lua_isboolean(L2,-1)) {
        // Loop in preparing upvalues; either direct or via a table
        // 
        // Note: This excludes the case where a function directly addresses
        //       itself as an upvalue (recursive lane creation).
        //
        luaL_error( L, "Recursive use of upvalues; cannot copy the function" );
    
    } else {
        lua_remove(L2,-2);
    }
  STACK_END(L2,1)
    //
    // L2 [-1]: function

    ASSERT_L( lua_isfunction(L2,-1) );
}


/*
* Copy a function over, which has not been found in the cache.
*/
enum e_vt {
    VT_NORMAL, VT_KEY, VT_METATABLE
};
static bool_t inter_copy_one_( lua_State *L2, uint_t L2_cache_i, lua_State *L, uint_t i, enum e_vt value_type );

static void inter_copy_func( lua_State *L2, uint_t L2_cache_i, lua_State *L, uint_t i ) {

    lua_CFunction cfunc= lua_tocfunction( L,i );
    unsigned n;

    ASSERT_L( L2_cache_i != 0 );

  STACK_GROW(L,2);

  STACK_CHECK(L)
    if (!cfunc) {   // Lua function
        luaL_Buffer b;
        const char *s;
        size_t sz;
        int tmp;
        const char *name= NULL;

#if 0
        // "To get information about a function you push it onto the 
        // stack and start the what string with the character '>'."
        //
        { lua_Debug ar;
        lua_pushvalue( L, i );
        lua_getinfo(L, ">n", &ar);      // fills 'name' and 'namewhat', pops function
        name= ar.namewhat;
        
        fprintf( stderr, "NAME: %s\n", name );  // just gives NULL
        }
#endif 
        // 'lua_dump()' needs the function at top of stack
        //
        if (i!=-1) lua_pushvalue( L, i );

        luaL_buffinit(L,&b);
        tmp= lua_dump(L, buf_writer, &b);
        ASSERT_L(tmp==0);
            //
            // "value returned is the error code returned by the last call 
            // to the writer" (and we only return 0)

        luaL_pushresult(&b);    // pushes dumped string on 'L'
        s= lua_tolstring(L,-1,&sz);
        ASSERT_L( s && sz );

        if (i!=-1) lua_remove( L, -2 );

        // Note: Line numbers seem to be taken precisely from the 
        //       original function. 'name' is not used since the chunk
        //       is precompiled (it seems...). 
        //
        // TBD: Can we get the function's original name through, as well?
        //
        if (luaL_loadbuffer(L2, s, sz, name) != 0) {
            // chunk is precompiled so only LUA_ERRMEM can happen
            // "Otherwise, it pushes an error message"
            //
            STACK_GROW( L,1 );
            luaL_error( L, "%s", lua_tostring(L2,-1) );
        }
        lua_pop(L,1);   // remove the dumped string
  STACK_MID(L,0)
    }

    /* push over any upvalues; references to this function will come from
    * cache so we don't end up in eternal loop.
    */
    for( n=0; lua_getupvalue( L, i, 1+n ) != NULL; n++ ) {
        if ((!cfunc) && lua_equal(L,i,-1)) {
            /* Lua closure that has a (recursive) upvalue to itself
            */
            lua_pushvalue( L2, -((int)n)-1 );
        } else {
            if (!inter_copy_one_( L2, L2_cache_i, L, lua_gettop(L), VT_NORMAL ))
                luaL_error( L, "Cannot copy upvalue type '%s'", luaG_typename(L,-1) );
        }
        lua_pop(L,1);
    }
    // L2: function + 'n' upvalues (>=0)

  STACK_MID(L,0)

    if (cfunc) {
        lua_pushcclosure( L2, cfunc, n );   // eats up upvalues
    } else {
        // Set upvalues (originally set to 'nil' by 'lua_load')
        //
        int func_index= lua_gettop(L2)-n;

        for( ; n>0; n-- ) {
            const char *rc= lua_setupvalue( L2, func_index, n );
                //
                // "assigns the value at the top of the stack to the upvalue and returns its name.
                // It also pops the value from the stack."
            
            ASSERT_L(rc);      // not having enough slots?
        }
    }
  STACK_END(L,0)
}


/*
* Copies a value from 'L' state (at index 'i') to 'L2' state. Does not remove
* the original value.
*
* NOTE: Both the states must be solely in the current OS thread's posession.
*
* 'i' is an absolute index (no -1, ...)
*
* Returns TRUE if value was pushed, FALSE if its type is non-supported.
*/
static bool_t inter_copy_one_( lua_State *L2, uint_t L2_cache_i, lua_State *L, uint_t i, enum e_vt vt )
{
    bool_t ret= TRUE;

    STACK_GROW( L2, 1 );

  STACK_CHECK(L2)

    switch ( lua_type(L,i) ) {
        /* Basic types allowed both as values, and as table keys */

        case LUA_TBOOLEAN:
            lua_pushboolean( L2, lua_toboolean(L, i) );
            break;

        case LUA_TNUMBER:
            /* LNUM patch support (keeping integer accuracy) */
#ifdef LUA_LNUM
            if (lua_isinteger(L,i)) {
                lua_pushinteger( L2, lua_tointeger(L, i) );
                break;
            }
#endif
            lua_pushnumber( L2, lua_tonumber(L, i) ); 
            break;

        case LUA_TSTRING: {
            size_t len; const char *s = lua_tolstring( L, i, &len );
            lua_pushlstring( L2, s, len );
            } break;

        case LUA_TLIGHTUSERDATA:
            lua_pushlightuserdata( L2, lua_touserdata(L, i) );
            break;

        /* The following types are not allowed as table keys */

        case LUA_TUSERDATA: if (vt==VT_KEY) { ret=FALSE; break; }
            /* Allow only deep userdata entities to be copied across
             */
            if (!luaG_copydeep( L, L2, i )) {
                // Cannot copy it full; copy as light userdata
                //
                lua_pushlightuserdata( L2, lua_touserdata(L, i) );
            } break;

        case LUA_TNIL: if (vt==VT_KEY) { ret=FALSE; break; }
            lua_pushnil(L2);
            break;

        case LUA_TFUNCTION: if (vt==VT_KEY) { ret=FALSE; break; } {
            /* 
            * Passing C functions is risky; if they refer to LUA_ENVIRONINDEX
            * and/or LUA_REGISTRYINDEX they might work unintended (not work)
            * at the target.
            *
            * On the other hand, NOT copying them causes many self tests not
            * to work (timer, hangtest, ...)
            *
            * The trouble is, we cannot KNOW if the function at hand is safe
            * or not. We cannot study it's behaviour. We could trust the user,
            * but they might not even know they're sending lua_CFunction over
            * (as upvalues etc.).
            */
#if 0
            if (lua_iscfunction(L,i))
                luaL_error( L, "Copying lua_CFunction between Lua states is risky, and currently disabled." ); 
#endif
          STACK_CHECK(L2)
            push_cached_func( L2, L2_cache_i, L, i );
            ASSERT_L( lua_isfunction(L2,-1) );
          STACK_END(L2,1)
            } break;

        case LUA_TTABLE: if (vt==VT_KEY) { ret=FALSE; break; } {
        
          STACK_CHECK(L)
          STACK_CHECK(L2)

            /* Check if we've already copied the same table from 'L' (during this transmission), and
             * reuse the old copy. This allows table upvalues shared by multiple
             * local functions to point to the same table, also in the target.
             * Also, this takes care of cyclic tables and multiple references
             * to the same subtable.
             *
             * Note: Even metatables need to go through this test; to detect
             *      loops s.a. those in required module tables (getmetatable(lanes).lanes == lanes)
             */
            if (push_cached_table( L2, L2_cache_i, L, i )) {
                ASSERT_L( lua_istable(L2, -1) );    // from cache
                break;
            }
            ASSERT_L( lua_istable(L2,-1) );

            STACK_GROW( L, 2 );
            STACK_GROW( L2, 2 );

            lua_pushnil(L);    // start iteration
            while( lua_next( L, i ) ) {
                uint_t val_i= lua_gettop(L);
                uint_t key_i= val_i-1;

                /* Only basic key types are copied over; others ignored
                 */
                if (inter_copy_one_( L2, 0 /*key*/, L, key_i, VT_KEY )) {
                    /*
                    * Contents of metatables are copied with cache checking;
                    * important to detect loops.
                    */
                    if (inter_copy_one_( L2, L2_cache_i, L, val_i, VT_NORMAL )) {
                        ASSERT_L( lua_istable(L2,-3) );
                        lua_rawset( L2, -3 );    // add to table (pops key & val)
                    } else {
                        luaL_error( L, "Unable to copy over type '%s' (in %s)", 
                                        luaG_typename(L,val_i), 
                                        vt==VT_NORMAL ? "table":"metatable" );
                    }
                }
                lua_pop( L, 1 );    // pop value (next round)
            }
          STACK_MID(L,0)
          STACK_MID(L2,1)
          
            /* Metatables are expected to be immutable, and copied only once.
            */
            if (lua_getmetatable( L, i )) {
                //
                // L [-1]: metatable

                uint_t mt_id= get_mt_id( L, -1 );    // Unique id for the metatable

                STACK_GROW(L2,4);

                push_registry_subtable( L2, REG_MTID );
              STACK_MID(L2,2);
                lua_pushinteger( L2, mt_id );
                lua_rawget( L2, -2 );
                    //
                    // L2 ([-3]: copied table)
                    //    [-2]: reg[REG_MTID]
                    //    [-1]: nil/metatable pre-known in L2

              STACK_MID(L2,3);

                if (lua_isnil(L2,-1)) {   /* L2 did not know the metatable */
                    lua_pop(L2,1);
              STACK_MID(L2,2);
ASSERT_L( lua_istable(L,-1) );
                    if (inter_copy_one_( L2, L2_cache_i /*for function cacheing*/, L, lua_gettop(L) /*[-1]*/, VT_METATABLE )) {
                        //
                        // L2 ([-3]: copied table)
                        //    [-2]: reg[REG_MTID]
                        //    [-1]: metatable (copied from L)

              STACK_MID(L2,3);
                        // mt_id -> metatable
                        //
                        lua_pushinteger(L2,mt_id);
                        lua_pushvalue(L2,-2);
                        lua_rawset(L2,-4);

                        // metatable -> mt_id
                        //
                        lua_pushvalue(L2,-1);
                        lua_pushinteger(L2,mt_id);
                        lua_rawset(L2,-4);
                        
              STACK_MID(L2,3);
                    } else {
                        luaL_error( L, "Error copying a metatable" );
                    }
              STACK_MID(L2,3);
                }
                    // L2 ([-3]: copied table)
                    //    [-2]: reg[REG_MTID]
                    //    [-1]: metatable (pre-known or copied from L)

                lua_remove(L2,-2);   // take away 'reg[REG_MTID]'
                    //
                    // L2: ([-2]: copied table)
                    //     [-1]: metatable for that table

                lua_setmetatable( L2, -2 );
                
                // L2: [-1]: copied table (with metatable set if source had it)

                lua_pop(L,1);   // remove source metatable (L, not L2!)
            }
          STACK_END(L2,1)
          STACK_END(L,0)
            } break;

        /* The following types cannot be copied */

        case LUA_TTHREAD: 
            ret=FALSE; break;
    }

  STACK_END(L2, ret? 1:0)

    return ret;
}


/*
* Akin to 'lua_xmove' but copies values between _any_ Lua states.
*
* NOTE: Both the states must be solely in the current OS thread's posession.
*
* Note: Parameters are in this order ('L' = from first) to be same as 'lua_xmove'.
*/
void luaG_inter_copy( lua_State* L, lua_State *L2, uint_t n )
{
    uint_t top_L= lua_gettop(L);
    uint_t top_L2= lua_gettop(L2);
    uint_t i;

    /* steal Lua library's 'luaB_tostring()' from the first call. Other calls
    * don't have to have access to it.
    *
    * Note: multiple threads won't come here at once; this function will
    *       be called before there can be multiple threads (no locking needed).
    */
    if (!hijacked_tostring) {
        STACK_GROW( L,1 );
        
      STACK_CHECK(L)
        lua_getglobal( L, "tostring" );
            //
            // [-1]: function|nil
            
        hijacked_tostring= lua_tocfunction( L, -1 );
        lua_pop(L,1);
      STACK_END(L,0)
      
        if (!hijacked_tostring) {
            luaL_error( L, "Need to see 'tostring()' once" );
        }
    }

    if (n > top_L) 
        luaL_error( L, "Not enough values: %d < %d", top_L, n );

    STACK_GROW( L2, n+1 );

    /*
    * Make a cache table for the duration of this copy. Collects tables and
    * function entries, avoiding the same entries to be passed on as multiple
    * copies. ESSENTIAL i.e. for handling upvalue tables in the right manner!
    */
    lua_newtable(L2);

    for (i=top_L-n+1; i <= top_L; i++) {
        if (!inter_copy_one_( L2, top_L2+1, L, i, VT_NORMAL )) {
       
            luaL_error( L, "Cannot copy type: %s", luaG_typename(L,i) );
       }
    }

    /*
    * Remove the cache table. Persistant caching would cause i.e. multiple 
    * messages passed in the same table to use the same table also in receiving
    * end.
    */
    lua_remove( L2, top_L2+1 );

    ASSERT_L( (uint_t)lua_gettop(L) == top_L );
    ASSERT_L( (uint_t)lua_gettop(L2) == top_L2+n );
}


void luaG_inter_move( lua_State* L, lua_State *L2, uint_t n )
{
    luaG_inter_copy( L, L2, n );
    lua_pop( L,(int)n );
}
