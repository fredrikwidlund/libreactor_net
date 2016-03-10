#ifndef REACTOR_RESOLVER_H_INCLUDED
#define REACTOR_RESOLVER_H_INCLUDED

enum reactor_resolver_event
{
  REACTOR_RESOLVER_ERROR = 0x00,
  REACTOR_RESOLVER_REPLY = 0x01
};

enum reactor_resolver_state
{
  REACTOR_RESOLVER_CLOSED,
  REACTOR_RESOLVER_OPEN
};

typedef struct reactor_resolver reactor_resolver;
struct reactor_resolver
{
  int                        state;
  reactor_user               user;
  reactor_signal_dispatcher  dispatcher;
  struct gaicb               gaicb;
};

void reactor_resolver_init(reactor_resolver *, reactor_user_call *, void *);
int  reactor_resolver_open(reactor_resolver *, char *, char *, struct addrinfo *);
void reactor_resolver_close(reactor_resolver *);
void reactor_resolver_dispatcher_event(void *, int, void *);

#endif /* REACTOR_RESOLVER_H_INCLUDED */
