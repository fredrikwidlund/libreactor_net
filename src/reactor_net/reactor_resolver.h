#ifndef REACTOR_RESOLVER_H_INCLUDED
#define REACTOR_RESOLVER_H_INCLUDED

enum REACTOR_RESOLVER_EVENT
{
  REACTOR_RESOLVER_ERROR = 0x00,
  REACTOR_RESOLVER_REPLY = 0x01
};

enum REACTOR_RESOLVER_STATE
{
  REACTOR_RESOLVER_UNDEFINED = 0,
  REACTOR_RESOLVER_READY,
  REACTOR_RESOLVER_RESOLVING,
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
int  reactor_resolver_lookup(reactor_resolver *, char *, char *, struct addrinfo *);
void reactor_resolver_close(reactor_resolver *);
void reactor_resolver_event(void *, int, void *);

#endif /* REACTOR_RESOLVER_H_INCLUDED */
