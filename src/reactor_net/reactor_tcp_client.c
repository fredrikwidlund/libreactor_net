#define _GNU_SOURCE

#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>

#include <dynamic.h>
#include <reactor_core.h>

#include "reactor_resolver.h"
#include "reactor_tcp_client.h"

void reactor_tcp_client_init(reactor_tcp_client *client, reactor_user_call *call, void *state)
{
  *client = (reactor_tcp_client) {.state = REACTOR_TCP_CLIENT_CLOSED};
  reactor_user_init(&client->user, call, state);
  reactor_resolver_init(&client->resolver, reactor_tcp_client_resolver_event, client);
}

int reactor_tcp_client_open(reactor_tcp_client *client, reactor_stream *stream, char *host, char *service)
{
  int e;

  if (client->state != REACTOR_TCP_CLIENT_CLOSED)
    return -1;

  client->stream = stream;
  e = reactor_resolver_open(&client->resolver, host, service, NULL);
  if (e == -1)
    return -1;

  client->state = REACTOR_TCP_CLIENT_RESOLVING;
  return 0;
}

int reactor_tcp_client_connect(reactor_tcp_client *client, reactor_stream *stream, struct addrinfo *ai)
{
  int e, fd;

  if (client->state != REACTOR_TCP_CLIENT_CLOSED && client->state != REACTOR_TCP_CLIENT_RESOLVING)
    return -1;

  fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
  if (fd == -1)
    return -1;

  e = reactor_stream_open(stream, fd);
  if (e == -1)
    {
      (void) close(fd);
      return -1;
    }

  e = connect(fd, ai->ai_addr, ai->ai_addrlen);
  if (e == -1 && errno != EINPROGRESS)
    return -1;

  return 0;
}

void reactor_tcp_client_close(reactor_tcp_client *client)
{
  if (client->state == REACTOR_TCP_CLIENT_CLOSED)
    return;

  reactor_resolver_close(&client->resolver);
  client->stream = NULL;
  client->state = REACTOR_TCP_CLIENT_CLOSED;
  reactor_user_dispatch(&client->user, REACTOR_TCP_CLIENT_CLOSE, NULL);
}

void reactor_tcp_client_error(reactor_tcp_client *client)
{
  reactor_user_dispatch(&client->user, REACTOR_TCP_CLIENT_ERROR, NULL);
  reactor_tcp_client_close(client);
}

void reactor_tcp_client_resolver_event(void *state, int type, void *data)
{
  reactor_tcp_client *client;
  struct addrinfo *ai;
  int e;

  client = state;
  ai = data;
  if (type != REACTOR_RESOLVER_REPLY || !ai || client->state != REACTOR_TCP_CLIENT_RESOLVING)
    {
      reactor_tcp_client_error(client);
      return;
    }

  e = reactor_tcp_client_connect(client, client->stream, ai);
  freeaddrinfo(ai);
  if (e == -1)
    reactor_tcp_client_error(client);
}
