#define _GNU_SOURCE

#include <unistd.h>
#include <netdb.h>
#include <errno.h>

#include <dynamic.h>
#include <reactor_core.h>

#include "reactor_tcp_server.h"

void reactor_tcp_server_init(reactor_tcp_server *tcp_server, reactor_user_call *call, void *state)
{
  reactor_desc_init(&tcp_server->desc, reactor_tcp_server_event, tcp_server);
  reactor_user_init(&tcp_server->user, call, state);
  tcp_server->state = REACTOR_TCP_SERVER_CLOSED;
}

int reactor_tcp_server_open(reactor_tcp_server *tcp_server, char *node, char *service)
{
  struct addrinfo *ai, hints;
  int e, fd;

  if (tcp_server->state != REACTOR_TCP_SERVER_CLOSED)
    return -1;

  hints = (struct addrinfo) {.ai_family = AF_INET, .ai_socktype = SOCK_STREAM, .ai_flags = AI_PASSIVE};
  e = getaddrinfo(node, service, &hints, &ai);
  if (e != 0 || !ai)
    return -1;

  fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
  if (fd == -1)
    {
      freeaddrinfo(ai);
      return -1;
    }

  e = reactor_tcp_server_listen(tcp_server, fd, ai->ai_addr, ai->ai_addrlen);
  freeaddrinfo(ai);
  if (e == -1)
    {
      (void) close(fd);
      return -1;
    }

  tcp_server->state = REACTOR_TCP_SERVER_OPEN;
  return 0;
}

int reactor_tcp_server_listen(reactor_tcp_server *tcp_server, int fd, struct sockaddr *sa, socklen_t sa_len)
{
  int e;

  e = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (int[]){1}, sizeof(int));
  if (e == -1)
    return -1;

  e = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int));
  if (e == -1)
   return -1;

  e = bind(fd, sa, sa_len);
  if (e == -1)
    return -1;

  e = listen(fd, -1);
  if (e == -1)
    return -1;

  e = reactor_desc_open(&tcp_server->desc, fd);
  if (e == -1)
    return -1;

  return 0;
}

void reactor_tcp_server_event(void *state, int type, void *data)
{
  reactor_tcp_server *tcp_server;
  reactor_tcp_server_data client;

  (void) data;
  tcp_server = state;

  switch (type)
    {
    case REACTOR_DESC_CLOSE:
      tcp_server->state = REACTOR_TCP_SERVER_CLOSED;
      reactor_user_dispatch(&tcp_server->user, REACTOR_TCP_SERVER_CLOSE, NULL);
      break;
    case REACTOR_DESC_READ:
      while (1)
        {
          client.addr_len = sizeof(client.addr);
          client.fd = accept(reactor_desc_fd(&tcp_server->desc), (struct sockaddr *) &client.addr, &client.addr_len);
          if (client.fd == -1 && errno != EAGAIN)
            reactor_user_dispatch(&tcp_server->user, REACTOR_TCP_SERVER_ERROR, &errno);
          if (client.fd == -1)
            break;
          reactor_user_dispatch(&tcp_server->user, REACTOR_TCP_SERVER_ACCEPT, &client);
        }
      break;
    default:
      reactor_tcp_server_error(tcp_server);
      break;
    }
}

void reactor_tcp_server_error(reactor_tcp_server *tcp_server)
{
  reactor_user_dispatch(&tcp_server->user, REACTOR_TCP_SERVER_ERROR, tcp_server);
  reactor_tcp_server_close(tcp_server);
}

void reactor_tcp_server_close(reactor_tcp_server *tcp_server)
{
  if (tcp_server->state != REACTOR_TCP_SERVER_OPEN)
    return;

  tcp_server->state = REACTOR_TCP_SERVER_CLOSING;
  reactor_desc_close(&tcp_server->desc);
}
