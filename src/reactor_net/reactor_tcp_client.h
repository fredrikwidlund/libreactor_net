#ifndef REACTOR_TCP_CLIENT_H_INCLUDED
#define REACTOR_TCP_CLIENT_H_INCLUDED

enum reactor_tcp_client_event
{
  REACTOR_TCP_CLIENT_ERROR = 0x01,
  REACTOR_TCP_CLIENT_CLOSE = 0x02
};

enum reactor_tcp_client_state
{
  REACTOR_TCP_CLIENT_CLOSED,
  REACTOR_TCP_CLIENT_RESOLVING,
  REACTOR_TCP_CLIENT_CONNECTING,
  REACTOR_TCP_CLIENT_CONNECTED
};

typedef struct reactor_tcp_client reactor_tcp_client;
struct reactor_tcp_client
{
  int                    state;
  reactor_user           user;
  reactor_stream        *stream;
  reactor_resolver       resolver;
};

void reactor_tcp_client_init(reactor_tcp_client *, reactor_user_call *, void *);
int  reactor_tcp_client_open(reactor_tcp_client *, reactor_stream *, char *, char *);
int  reactor_tcp_client_connect(reactor_tcp_client *, reactor_stream *, struct addrinfo *);
void reactor_tcp_client_close(reactor_tcp_client *);
void reactor_tcp_client_error(reactor_tcp_client *);
void reactor_tcp_client_resolver_event(void *, int, void *);

#endif /* REACTOR_TCP_CLIENT_H_INCLUDED */
