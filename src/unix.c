/*=========================================================================*\
* Unix domain socket
* LuaSocket toolkit
\*=========================================================================*/
#include "luasocket.h"

#include "unixstream.h"
#include "unixdgram.h"

/*-------------------------------------------------------------------------*\
* Modules and functions
\*-------------------------------------------------------------------------*/
static const luaL_Reg mod[] = {
    {"stream", unixstream_open},
    {"dgram", unixdgram_open},
    {NULL, NULL}
};

static void add_alias(lua_State *L, int index, const char *name, const char *target)
{
    lua_getfield(L, index, target);
    lua_setfield(L, index, name);
}

static int compat_socket_unix_call(lua_State *L)
{
    /* Look up socket.unix.stream in the socket.unix table (which is the first
     * argument). */
    lua_getfield(L, 1, "stream");

    /* Replace the stack entry for the socket.unix table with the
     * socket.unix.stream function. */
    lua_replace(L, 1);

    /* Call socket.unix.stream, passing along any arguments. */
    int n = lua_gettop(L);
    lua_call(L, n-1, LUA_MULTRET);

    /* Pass along the return values from socket.unix.stream. */
    n = lua_gettop(L);
    return n;
}

/*-------------------------------------------------------------------------*\
* Initializes module
\*-------------------------------------------------------------------------*/
int luaopen_lanes_unix(lua_State *L) {
    /* create classes */
    auxiliar_newclass(L, "unix{master}", unix_methods);
    auxiliar_newclass(L, "unix{client}", unix_methods);
    auxiliar_newclass(L, "unix{server}", unix_methods);
    /* create class groups */
    auxiliar_add2group(L, "unix{master}", "unix{any}");
    auxiliar_add2group(L, "unix{client}", "unix{any}");
    auxiliar_add2group(L, "unix{server}", "unix{any}");
#if LUA_VERSION_NUM > 501 && !defined(LUA_COMPAT_MODULE)
    lua_pushcfunction(L, global_create);
    (void) func;
#else
    /* set function into socket namespace */
    luaL_openlib(L, "socket", func, 0);
    lua_pushcfunction(L, global_create);
#endif
    /* return the function instead of the 'socket' table */
    return 1;
}

/*=========================================================================*\
* Lua methods
\*=========================================================================*/
/*-------------------------------------------------------------------------*\
* Just call buffered IO methods
\*-------------------------------------------------------------------------*/
static int meth_send(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkclass(L, "unix{client}", 1);
    return buffer_meth_send(L, &un->buf);
}

static int meth_receive(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkclass(L, "unix{client}", 1);
    return buffer_meth_receive(L, &un->buf);
}

static int meth_getstats(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkclass(L, "unix{client}", 1);
    return buffer_meth_getstats(L, &un->buf);
}

static int meth_setstats(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkclass(L, "unix{client}", 1);
    return buffer_meth_setstats(L, &un->buf);
}

/*-------------------------------------------------------------------------*\
* Just call option handler
\*-------------------------------------------------------------------------*/
static int meth_setoption(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unix{any}", 1);
    return opt_meth_setoption(L, optset, &un->sock);
}

/*-------------------------------------------------------------------------*\
* Select support methods
\*-------------------------------------------------------------------------*/
static int meth_getfd(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unix{any}", 1);
    lua_pushnumber(L, (int) un->sock);
    return 1;
}

/* this is very dangerous, but can be handy for those that are brave enough */
static int meth_setfd(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unix{any}", 1);
    un->sock = (t_socket) luaL_checknumber(L, 2); 
    return 0;
}

static int meth_dirty(lua_State *L) {
    p_unix un = (p_unix) auxiliar_checkgroup(L, "unix{any}", 1);
    lua_pushboolean(L, !buffer_isempty(&un->buf));
    return 1;
}

/*-------------------------------------------------------------------------*\
* Waits for and returns a client object attempting connection to the 
* server object 
\*-------------------------------------------------------------------------*/
static int meth_accept(lua_State *L) {
    p_unix server = (p_unix) auxiliar_checkclass(L, "unix{server}", 1);
    p_timeout tm = timeout_markstart(&server->tm);
    t_socket sock;
    int err = socket_accept(&server->sock, &sock, NULL, NULL, tm);
    /* if successful, push client socket */
    if (err == IO_DONE) {
        p_unix clnt = (p_unix) lua_newuserdata(L, sizeof(t_unix));
        auxiliar_setclass(L, "unix{client}", -1);
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
static const char *unix_trybind(p_unix un, const char *path) {
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
    p_unix un = (p_unix) auxiliar_checkclass(L, "unix{master}", 1);
    const char *path =  luaL_checkstring(L, 2);
    const char *err = unix_trybind(un, path);
    if (err) {
        lua_pushnil(L);
        lua_pushstring(L, err);
        return 2;
    }
    lua_pushnumber(L, 1);
    return 1;
}

/*-------------------------------------------------------------------------*\
* Turns a master unix object into a client object.
\*-------------------------------------------------------------------------*/
static const char *unix_tryconnect(p_unix un, const char *path)
{
    int i;
    lua_newtable(L);
    int socket_unix_table = lua_gettop(L);

    for (i = 0; mod[i].name; i++)
        mod[i].func(L);

    /* Add backwards compatibility aliases "tcp" and "udp" for the "stream" and
     * "dgram" functions. */
    add_alias(L, socket_unix_table, "tcp", "stream");
    add_alias(L, socket_unix_table, "udp", "dgram");

    /* Add a backwards compatibility function and a metatable setup to call it
     * for the old socket.unix() interface. */
    lua_pushcfunction(L, compat_socket_unix_call);
    lua_setfield(L, socket_unix_table, "__call");
    lua_pushvalue(L, socket_unix_table);
    lua_setmetatable(L, socket_unix_table);

    return 1;
}
