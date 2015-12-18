#define _GNU_SOURCE

#include <stdlib.h>
#include <netdb.h>

#include <dynamic.h>
#include <reactor_core.h>

#include "reactor_resolver.h"

void reactor_resolver_init(reactor_resolver *resolver, reactor_user_call *call, void *state)
{
  reactor_user_init(&resolver->user, call, state);
  reactor_signal_dispatcher_init(&resolver->dispatcher, reactor_resolver_event, resolver);
  resolver->state = REACTOR_RESOLVER_READY;
}

int reactor_resolver_lookup(reactor_resolver *resolver, char *name, char *service, struct addrinfo *hints)
{
  struct sigevent sigev;
  int e;

  if (resolver->state != REACTOR_RESOLVER_READY)
    return -1;

  reactor_signal_dispatcher_sigev(&resolver->dispatcher, &sigev);
  resolver->gaicb.ar_name = name;
  resolver->gaicb.ar_service = service;
  resolver->gaicb.ar_request = hints ? hints : (struct addrinfo[]) {{.ai_family = AF_INET, .ai_socktype = SOCK_STREAM}};
  resolver->gaicb.ar_result = NULL;

  e = reactor_signal_dispatcher_hold();
  if (e == -1)
    return -1;

  e = getaddrinfo_a(GAI_NOWAIT, (struct gaicb *[1]){&resolver->gaicb}, 1, &sigev);
  if (e == -1)
    return -1;

  resolver->state = REACTOR_RESOLVER_RESOLVING;
  return 0;
}

void reactor_resolver_event(void *state, int type, void *data)
{
  reactor_resolver *resolver;

  (void) data;
  resolver = state;
  if (type == REACTOR_SIGNAL_DISPATCHER_MESSAGE && resolver->state == REACTOR_RESOLVER_RESOLVING)
    {
      (void) gai_cancel(&resolver->gaicb);
      reactor_signal_dispatcher_release();
      resolver->state = REACTOR_RESOLVER_READY;
      reactor_user_dispatch(&resolver->user, REACTOR_RESOLVER_REPLY, resolver->gaicb.ar_result);
    }
}
