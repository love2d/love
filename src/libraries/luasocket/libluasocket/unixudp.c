/*=========================================================================*\
* Unix domain socket udp submodule
* LuaSocket toolkit
\*=========================================================================*/
#include <string.h>
#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"
#include "compat.h"

#include "auxiliar.h"
#include "socket.h"
#include "options.h"
#include "unix.h"
#include <sys/un.h>

#define UNIXUDP_DATAGRAMSIZE 8192

/*=========================================================================*\
* Internal function prototypes
\*=========================================================================*/
static int global_create(lua_State *L);
static int meth_connect(lua_State *L);
static int meth_bind(lua_State *L);
static int meth_send(lua_State *L);
static int meth_receive(lua_State *L);
static int meth_close(lua_State *L);
static int meth_setoption(lua_State *L);
static int meth_settimeout(lua_State *L);
static int meth_gettimeout(lua_State *L);
static int meth_getfd(lua_State *L);
static int meth_setfd(lua_State *L);
static int meth_dirty(lua_State *L);
static int meth_receivefrom(lua_State *L);
static int meth_sendto(lua_State *L);
static int meth_getsockname(lua_State *L);

static const char *unixudp_tryconnect(p_unix un, const char *path);
static const char *unixudp_trybind(p_unix un, const char *path);

/* unixudp object methods */
static luaL_Reg unixudp_methods[] = {
    {"__gc",        meth_close},
    {"__tostring",  auxiliar_tostring},
    {"bind",        meth_bind},
    {"close",       meth_close},
    {"connect",     meth_connect},
    {"dirty",       meth_dirty},
    {"getfd",       meth_getfd},
    {"send",        meth_send},
    {"sendto",      meth_sendto},
    {"receive",     meth_receive},
    {"receivefrom", meth_receivefrom},
    {"setfd",       meth_setfd},
    {"setoption",   meth_setoption},
    {"setpeername", meth_connect},
    {"setsockname", meth_bind},
    {"getsockname", meth_getsockname},
    {"settimeout",  meth_settimeout},
    {"gettimeout",  meth_gettimeout},
    {NULL,          NULL}
};

/* socket option handlers */
static t_opt optset[] = {
    {"reuseaddr",   opt_set_reuseaddr},
    {NULL,          NULL}
};

/* functions in library namespace */
static luaL_Reg func[] = {
    {"udp", global_create},
    {NULL, NULL}
};

/*-------------------------------------------------------------------------*\
* Initializes module
\*-------------------------------------------------------------------------*/
int unixudp_open(lua_State *L)
{
    /* create classes */
    auxiliar_newclass(L, "unixudp{connected}", unixudp_methods);
    auxiliar_newclass(L, "unixudp{unconnected}", unixudp_methods);
    /* create class groups */
    auxiliar_add2group(L, "unixudp{connected}",   "unixudp{any}");
    auxiliar_add2group(L, "unixudp{unconnected}", "unixudp{any}");
    auxiliar_add2group(L, "unixudp{connected}",   "select{able}");
    auxiliar_add2group(L, "unixudp{unconnected}", "select{able}");

    luaL_setfuncs(L, func, 0);
    return 0;
}

/*=========================================================================*\
* Lua methods
\*=========================================================================*/
static const char *unixudp_strerror(int err)
{
    /* a 'closed' error on an unconnected means the target address was not
     * accepted by the transport layer */
    if (err == IO_CLOSED) return "refused";
    else return socket_strerror(err);
}

static int meth_send(lua_State *L)
{
    p_unix un = (p_unix) auxiliar_checkclass(L, "unixudp{connected}", 1);
    p_timeout tm = &un->tm;
    size_t count, sent = 0;
    int err;
    const char *data = luaL_checklstring(L, 2, &count);
    timeout_markstart(tm);
    err = socket_send(&un->sock, data, count, &sent, tm);
    if (err != IO_DONE) {
        lua_pushnil(L);
        lua_pushstring(L, unixudp_strerror(err));
        return 2;
    }
    lua_pushnumber(L, (lua_Number) sent);
    return 1;
}

/*-------------------------------------------------------------------------*\
* Send data through unconnected unixudp socket
\*-------------------------------------------------------------------------*/
static int meth_sendto(lua_State *L)
{
    p_unix un = (p_unix) auxiliar_checkclass(L, "unixudp{unconnected}", 1);
    size_t count, sent = 0;
    const char *data = luaL_checklstring(L, 2, &count);
    const char *path = luaL_checkstring(L, 3);
    p_timeout tm = &un->tm;
    int err;
    struct sockaddr_un remote;
    size_t len = strlen(path);

    if (len >= sizeof(remote.sun_path)) {
		lua_pushnil(L);
		lua_pushstring(L, "path too long");
		return 2;
	}

    memset(&remote, 0, sizeof(remote));
    strcpy(remote.sun_path, path);
    remote.sun_family = AF_UNIX;
    timeout_markstart(tm);
#ifdef UNIX_HAS_SUN_LEN
    remote.sun_len = sizeof(remote.sun_family) + sizeof(remote.sun_len)
        + len + 1;
    err = socket_sendto(&un->sock, data, count, &sent, (SA *) &remote, remote.sun_len, tm);
#else
    err = socket_sendto(&un->sock, data, count, &sent, (SA *) &remote,
		   	sizeof(remote.sun_family) + len, tm);
#endif
    if (err != IO_DONE) {
        lua_pushnil(L);
        lua_pushstring(L, unixudp_strerror(err));
        return 2;
    }
    lua_pushnumber(L, (lua_Number) sent);
    return 1;
}

static int meth_receive(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixudp{any}", 1);
    char buf[UNIXUDP_DATAGRAMSIZE];
    size_t got, wanted = (size_t) luaL_optnumber(L, 2, sizeof(buf));
    char *dgram = wanted > sizeof(buf)? (char *) malloc(wanted): buf;
    int err;
    p_timeout tm = &un->tm;
    timeout_markstart(tm);
    if (!dgram) {
        lua_pushnil(L);
        lua_pushliteral(L, "out of memory");
        return 2;
    }
    err = socket_recv(&un->sock, dgram, wanted, &got, tm);
    /* Unlike TCP, recv() of zero is not closed, but a zero-length packet. */
    if (err != IO_DONE && err != IO_CLOSED) {
        lua_pushnil(L);
        lua_pushstring(L, unixudp_strerror(err));
        if (wanted > sizeof(buf)) free(dgram);
        return 2;
    }
    lua_pushlstring(L, dgram, got);
    if (wanted > sizeof(buf)) free(dgram);
    return 1;
}

/*-------------------------------------------------------------------------*\
* Receives data and sender from a UDP socket
\*-------------------------------------------------------------------------*/
static int meth_receivefrom(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkclass(L, "unixudp{unconnected}", 1);
    char buf[UNIXUDP_DATAGRAMSIZE];
    size_t got, wanted = (size_t) luaL_optnumber(L, 2, sizeof(buf));
    char *dgram = wanted > sizeof(buf)? (char *) malloc(wanted): buf;
    struct sockaddr_un addr;
    socklen_t addr_len = sizeof(addr);
    int err;
    p_timeout tm = &un->tm;
    timeout_markstart(tm);
    if (!dgram) {
        lua_pushnil(L);
        lua_pushliteral(L, "out of memory");
        return 2;
    }
    err = socket_recvfrom(&un->sock, dgram, wanted, &got, (SA *) &addr,
            &addr_len, tm);
    /* Unlike TCP, recv() of zero is not closed, but a zero-length packet. */
    if (err != IO_DONE && err != IO_CLOSED) {
        lua_pushnil(L);
        lua_pushstring(L, unixudp_strerror(err));
        if (wanted > sizeof(buf)) free(dgram);
        return 2;
    }

    lua_pushlstring(L, dgram, got);
	/* the path may be empty, when client send without bind */
    lua_pushstring(L, addr.sun_path);
    if (wanted > sizeof(buf)) free(dgram);
    return 2;
}

/*-------------------------------------------------------------------------*\
* Just call option handler
\*-------------------------------------------------------------------------*/
static int meth_setoption(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixudp{any}", 1);
    return opt_meth_setoption(L, optset, &un->sock);
}

/*-------------------------------------------------------------------------*\
* Select support methods
\*-------------------------------------------------------------------------*/
static int meth_getfd(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixudp{any}", 1);
    lua_pushnumber(L, (int) un->sock);
    return 1;
}

/* this is very dangerous, but can be handy for those that are brave enough */
static int meth_setfd(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixudp{any}", 1);
    un->sock = (t_socket) luaL_checknumber(L, 2);
    return 0;
}

static int meth_dirty(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixudp{any}", 1);
    (void) un;
    lua_pushboolean(L, 0);
    return 1;
}

/*-------------------------------------------------------------------------*\
* Binds an object to an address
\*-------------------------------------------------------------------------*/
static const char *unixudp_trybind(p_unix un, const char *path) {
    struct sockaddr_un local;
    size_t len = strlen(path);
    int err;
    if (len >= sizeof(local.sun_path)) return "path too long";
    memset(&local, 0, sizeof(local));
    strcpy(local.sun_path, path);
    local.sun_family = AF_UNIX;
#ifdef UNIX_HAS_SUN_LEN
    local.sun_len = sizeof(local.sun_family) + sizeof(local.sun_len)
        + len + 1;
    err = socket_bind(&un->sock, (SA *) &local, local.sun_len);

#else
    err = socket_bind(&un->sock, (SA *) &local,
            sizeof(local.sun_family) + len);
#endif
    if (err != IO_DONE) socket_destroy(&un->sock);
    return socket_strerror(err);
}

static int meth_bind(lua_State *L)
{
    p_unix un = (p_unix) auxiliar_checkclass(L, "unixudp{unconnected}", 1);
    const char *path =  luaL_checkstring(L, 2);
    const char *err = unixudp_trybind(un, path);
    if (err) {
        lua_pushnil(L);
        lua_pushstring(L, err);
        return 2;
    }
    lua_pushnumber(L, 1);
    return 1;
}

static int meth_getsockname(lua_State *L)
{
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixudp{any}", 1);
    struct sockaddr_un peer = {0};
    socklen_t peer_len = sizeof(peer);

    if (getsockname(un->sock, (SA *) &peer, &peer_len) < 0) {
        lua_pushnil(L);
        lua_pushstring(L, socket_strerror(errno));
        return 2;
    }

    lua_pushstring(L, peer.sun_path);
    return 1;
}

/*-------------------------------------------------------------------------*\
* Turns a master unixudp object into a client object.
\*-------------------------------------------------------------------------*/
static const char *unixudp_tryconnect(p_unix un, const char *path)
{
    struct sockaddr_un remote;
    int err;
    size_t len = strlen(path);
    if (len >= sizeof(remote.sun_path)) return "path too long";
    memset(&remote, 0, sizeof(remote));
    strcpy(remote.sun_path, path);
    remote.sun_family = AF_UNIX;
    timeout_markstart(&un->tm);
#ifdef UNIX_HAS_SUN_LEN
    remote.sun_len = sizeof(remote.sun_family) + sizeof(remote.sun_len)
        + len + 1;
    err = socket_connect(&un->sock, (SA *) &remote, remote.sun_len, &un->tm);
#else
    err = socket_connect(&un->sock, (SA *) &remote,
            sizeof(remote.sun_family) + len, &un->tm);
#endif
    if (err != IO_DONE) socket_destroy(&un->sock);
    return socket_strerror(err);
}

static int meth_connect(lua_State *L)
{
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixudp{any}", 1);
    const char *path =  luaL_checkstring(L, 2);
    const char *err = unixudp_tryconnect(un, path);
    if (err) {
        lua_pushnil(L);
        lua_pushstring(L, err);
        return 2;
    }
    /* turn unconnected object into a connected object */
    auxiliar_setclass(L, "unixudp{connected}", 1);
    lua_pushnumber(L, 1);
    return 1;
}

/*-------------------------------------------------------------------------*\
* Closes socket used by object
\*-------------------------------------------------------------------------*/
static int meth_close(lua_State *L)
{
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixudp{any}", 1);
    socket_destroy(&un->sock);
    lua_pushnumber(L, 1);
    return 1;
}

/*-------------------------------------------------------------------------*\
* Just call tm methods
\*-------------------------------------------------------------------------*/
static int meth_settimeout(lua_State *L)
{
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixudp{any}", 1);
    return timeout_meth_settimeout(L, &un->tm);
}

static int meth_gettimeout(lua_State *L)
{
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixudp{any}", 1);
    return timeout_meth_gettimeout(L, &un->tm);
}

/*=========================================================================*\
* Library functions
\*=========================================================================*/
/*-------------------------------------------------------------------------*\
* Creates a master unixudp object
\*-------------------------------------------------------------------------*/
static int global_create(lua_State *L)
{
    t_socket sock;
    int err = socket_create(&sock, AF_UNIX, SOCK_DGRAM, 0);
    /* try to allocate a system socket */
    if (err == IO_DONE) {
        /* allocate unixudp object */
        p_unix un = (p_unix) lua_newuserdata(L, sizeof(t_unix));
        /* set its type as master object */
        auxiliar_setclass(L, "unixudp{unconnected}", -1);
        /* initialize remaining structure fields */
        socket_setnonblocking(&sock);
        un->sock = sock;
        io_init(&un->io, (p_send) socket_send, (p_recv) socket_recv,
                (p_error) socket_ioerror, &un->sock);
        timeout_init(&un->tm, -1, -1);
        buffer_init(&un->buf, &un->io, &un->tm);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushstring(L, socket_strerror(err));
        return 2;
    }
}
