#ifndef UNIXTCP_H
#define UNIXTCP_H
/*=========================================================================*\
* UNIX TCP object
* LuaSocket toolkit
*
* The unixtcp.h module is basicly a glue that puts together modules buffer.h,
* timeout.h socket.h and inet.h to provide the LuaSocket UNIX TCP (AF_UNIX,
* SOCK_STREAM) support.
*
* Three classes are defined: master, client and server. The master class is
* a newly created unixtcp object, that has not been bound or connected. Server
* objects are unixtcp objects bound to some local address. Client objects are
* unixtcp objects either connected to some address or returned by the accept
* method of a server object.
\*=========================================================================*/
#include "unix.h"

int unixtcp_open(lua_State *L);

#endif /* UNIXTCP_H */
