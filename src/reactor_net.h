#ifndef MAIN_REACTOR_NET_H_INCLUDED
#define MAIN_REACTOR_NET_H_INCLUDED

#define REACTOR_NET_VERSION "0.1.0"
#define REACTOR_NET_VERSION_MAJOR 0
#define REACTOR_NET_VERSION_MINOR 1
#define REACTOR_NET_VERSION_PATCH 0

#ifdef __cplusplus
extern "C" {
#endif

#include "reactor_net/reactor_resolver.h"
#include "reactor_net/reactor_tcp_client.h"
#include "reactor_net/reactor_tcp_server.h"
#include "reactor_net/reactor_http_server.h"
#include "reactor_net/reactor_rest_server.h"

#ifdef __cplusplus
}
#endif

#endif /* MAIN_REACTOR_NET_H_INCLUDED */
