/*=========================================================================*\
* Unix domain socket tcp sub module
* LuaSocket toolkit
\*=========================================================================*/
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "compat.h"

#include "auxiliar.h"
#include "socket.h"
#include "options.h"
#include "unixtcp.h"
#include <sys/un.h>

/*=========================================================================*\
* Internal function prototypes
\*=========================================================================*/
static int global_create(lua_State *L);
static int meth_connect(lua_State *L);
static int meth_listen(lua_State *L);
static int meth_bind(lua_State *L);
static int meth_send(lua_State *L);
static int meth_shutdown(lua_State *L);
static int meth_receive(lua_State *L);
static int meth_accept(lua_State *L);
static int meth_close(lua_State *L);
static int meth_setoption(lua_State *L);
static int meth_settimeout(lua_State *L);
static int meth_getfd(lua_State *L);
static int meth_setfd(lua_State *L);
static int meth_dirty(lua_State *L);
static int meth_getstats(lua_State *L);
static int meth_setstats(lua_State *L);
static int meth_getsockname(lua_State *L);

static const char *unixtcp_tryconnect(p_unix un, const char *path);
static const char *unixtcp_trybind(p_unix un, const char *path);

/* unixtcp object methods */
static luaL_Reg unixtcp_methods[] = {
    {"__gc",        meth_close},
    {"__tostring",  auxiliar_tostring},
    {"accept",      meth_accept},
    {"bind",        meth_bind},
    {"close",       meth_close},
    {"connect",     meth_connect},
    {"dirty",       meth_dirty},
    {"getfd",       meth_getfd},
    {"getstats",    meth_getstats},
    {"setstats",    meth_setstats},
    {"listen",      meth_listen},
    {"receive",     meth_receive},
    {"send",        meth_send},
    {"setfd",       meth_setfd},
    {"setoption",   meth_setoption},
    {"setpeername", meth_connect},
    {"setsockname", meth_bind},
    {"getsockname", meth_getsockname},
    {"settimeout",  meth_settimeout},
    {"shutdown",    meth_shutdown},
    {NULL,          NULL}
};

/* socket option handlers */
static t_opt optset[] = {
    {"keepalive",   opt_set_keepalive},
    {"reuseaddr",   opt_set_reuseaddr},
    {"linger",      opt_set_linger},
    {NULL,          NULL}
};

/* functions in library namespace */
static luaL_Reg func[] = {
    {"tcp", global_create},
    {NULL, NULL}
};

/*-------------------------------------------------------------------------*\
* Initializes module
\*-------------------------------------------------------------------------*/
int unixtcp_open(lua_State *L)
{
    /* create classes */
    auxiliar_newclass(L, "unixtcp{master}", unixtcp_methods);
    auxiliar_newclass(L, "unixtcp{client}", unixtcp_methods);
    auxiliar_newclass(L, "unixtcp{server}", unixtcp_methods);

    /* create class groups */
    auxiliar_add2group(L, "unixtcp{master}", "unixtcp{any}");
    auxiliar_add2group(L, "unixtcp{client}", "unixtcp{any}");
    auxiliar_add2group(L, "unixtcp{server}", "unixtcp{any}");

    luaL_setfuncs(L, func, 0);
    return 0;
}

/*=========================================================================*\
* Lua methods
\*=========================================================================*/
/*-------------------------------------------------------------------------*\
* Just call buffered IO methods
\*-------------------------------------------------------------------------*/
static int meth_send(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkclass(L, "unixtcp{client}", 1);
    return buffer_meth_send(L, &un->buf);
}

static int meth_receive(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkclass(L, "unixtcp{client}", 1);
    return buffer_meth_receive(L, &un->buf);
}

static int meth_getstats(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkclass(L, "unixtcp{client}", 1);
    return buffer_meth_getstats(L, &un->buf);
}

static int meth_setstats(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkclass(L, "unixtcp{client}", 1);
    return buffer_meth_setstats(L, &un->buf);
}

/*-------------------------------------------------------------------------*\
* Just call option handler
\*-------------------------------------------------------------------------*/
static int meth_setoption(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixtcp{any}", 1);
    return opt_meth_setoption(L, optset, &un->sock);
}

/*-------------------------------------------------------------------------*\
* Select support methods
\*-------------------------------------------------------------------------*/
static int meth_getfd(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixtcp{any}", 1);
    lua_pushnumber(L, (int) un->sock);
    return 1;
}

/* this is very dangerous, but can be handy for those that are brave enough */
static int meth_setfd(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixtcp{any}", 1);
    un->sock = (t_socket) luaL_checknumber(L, 2);
    return 0;
}

static int meth_dirty(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixtcp{any}", 1);
    lua_pushboolean(L, !buffer_isempty(&un->buf));
    return 1;
}

/*-------------------------------------------------------------------------*\
* Waits for and returns a client object attempting connection to the
* server object
\*-------------------------------------------------------------------------*/
static int meth_accept(lua_State *L) {
    p_unix server = (p_unix) auxiliar_checkclass(L, "unixtcp{server}", 1);
    p_timeout tm = timeout_markstart(&server->tm);
    t_socket sock;
    int err = socket_accept(&server->sock, &sock, NULL, NULL, tm);
    /* if successful, push client socket */
    if (err == IO_DONE) {
        p_unix clnt = (p_unix) lua_newuserdata(L, sizeof(t_unix));
        auxiliar_setclass(L, "unixtcp{client}", -1);
        /* initialize structure fields */
        socket_setnonblocking(&sock);
        clnt->sock = sock;
        io_init(&clnt->io, (p_send)socket_send, (p_recv)socket_recv,
                (p_error) socket_ioerror, &clnt->sock);
        timeout_init(&clnt->tm, -1, -1);
        buffer_init(&clnt->buf, &clnt->io, &clnt->tm);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushstring(L, socket_strerror(err));
        return 2;
    }
}

/*-------------------------------------------------------------------------*\
* Binds an object to an address
\*-------------------------------------------------------------------------*/
static const char *unixtcp_trybind(p_unix un, const char *path) {
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

static int meth_bind(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkclass(L, "unixtcp{master}", 1);
    const char *path =  luaL_checkstring(L, 2);
    const char *err = unixtcp_trybind(un, path);
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
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixtcp{any}", 1);
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
* Turns a master unixtcp object into a client object.
\*-------------------------------------------------------------------------*/
static const char *unixtcp_tryconnect(p_unix un, const char *path)
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
    p_unix un = (p_unix) auxiliar_checkclass(L, "unixtcp{master}", 1);
    const char *path =  luaL_checkstring(L, 2);
    const char *err = unixtcp_tryconnect(un, path);
    if (err) {
        lua_pushnil(L);
        lua_pushstring(L, err);
        return 2;
    }
    /* turn master object into a client object */
    auxiliar_setclass(L, "unixtcp{client}", 1);
    lua_pushnumber(L, 1);
    return 1;
}

/*-------------------------------------------------------------------------*\
* Closes socket used by object
\*-------------------------------------------------------------------------*/
static int meth_close(lua_State *L)
{
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixtcp{any}", 1);
    socket_destroy(&un->sock);
    lua_pushnumber(L, 1);
    return 1;
}

/*-------------------------------------------------------------------------*\
* Puts the sockt in listen mode
\*-------------------------------------------------------------------------*/
static int meth_listen(lua_State *L)
{
    p_unix un = (p_unix) auxiliar_checkclass(L, "unixtcp{master}", 1);
    int backlog = (int) luaL_optnumber(L, 2, 32);
    int err = socket_listen(&un->sock, backlog);
    if (err != IO_DONE) {
        lua_pushnil(L);
        lua_pushstring(L, socket_strerror(err));
        return 2;
    }
    /* turn master object into a server object */
    auxiliar_setclass(L, "unixtcp{server}", 1);
    lua_pushnumber(L, 1);
    return 1;
}

/*-------------------------------------------------------------------------*\
* Shuts the connection down partially
\*-------------------------------------------------------------------------*/
static int meth_shutdown(lua_State *L)
{
    /* SHUT_RD,  SHUT_WR,  SHUT_RDWR  have  the value 0, 1, 2, so we can use method index directly */
    static const char* methods[] = { "receive", "send", "both", NULL };
    p_unix tcp = (p_unix) auxiliar_checkclass(L, "unixtcp{client}", 1);
    int how = luaL_checkoption(L, 2, "both", methods);
    socket_shutdown(&tcp->sock, how);
    lua_pushnumber(L, 1);
    return 1;
}

/*-------------------------------------------------------------------------*\
* Just call tm methods
\*-------------------------------------------------------------------------*/
static int meth_settimeout(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unixtcp{any}", 1);
    return timeout_meth_settimeout(L, &un->tm);
}

/*=========================================================================*\
* Library functions
\*=========================================================================*/
/*-------------------------------------------------------------------------*\
* Creates a master unixtcp object
\*-------------------------------------------------------------------------*/
static int global_create(lua_State *L) {
    t_socket sock;
    int err = socket_create(&sock, AF_UNIX, SOCK_STREAM, 0);
    /* try to allocate a system socket */
    if (err == IO_DONE) {
        /* allocate unixtcp object */
        p_unix un = (p_unix) lua_newuserdata(L, sizeof(t_unix));
        /* set its type as master object */
        auxiliar_setclass(L, "unixtcp{master}", -1);
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
