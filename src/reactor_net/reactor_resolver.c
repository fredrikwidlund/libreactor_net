#define _GNU_SOURCE

#include <stdlib.h>
#include <netdb.h>

#include <dynamic.h>
#include <reactor_core.h>

#include "reactor_resolver.h"

void reactor_resolver_init(reactor_resolver *resolver, reactor_user_call *call, void *state)
{
  *resolver = (reactor_resolver) {.state = REACTOR_RESOLVER_CLOSED};
  reactor_user_init(&resolver->user, call, state);
  reactor_signal_dispatcher_init(&resolver->dispatcher, reactor_resolver_dispatcher_event, resolver);
}

int reactor_resolver_open(reactor_resolver *resolver, char *name, char *service, struct addrinfo *hints)
{
  int e;

  if (resolver->state != REACTOR_RESOLVER_CLOSED)
    return -1;

  e = reactor_signal_dispatcher_open(&resolver->dispatcher);
  if (e == -1)
    return -1;

  resolver->hints = hints ? *hints : (struct addrinfo) {.ai_family = AF_INET, .ai_socktype = SOCK_STREAM};
  reactor_signal_dispatcher_sigev(&resolver->dispatcher, &resolver->sigev);
  resolver->gaicb.ar_name = name;
  resolver->gaicb.ar_service = service;
  resolver->gaicb.ar_request = &resolver->hints;
  resolver->gaicb.ar_result = NULL;
  e = getaddrinfo_a(GAI_NOWAIT, (struct gaicb *[1]){&resolver->gaicb}, 1, &resolver->sigev);
  if (e == -1)
    return -1;

  resolver->state = REACTOR_RESOLVER_OPEN;
  return 0;
}

void reactor_resolver_close(reactor_resolver *resolver)
{
  if (resolver->state != REACTOR_RESOLVER_CLOSED)
    {
      (void) gai_cancel(&resolver->gaicb);
      reactor_signal_dispatcher_close(&resolver->dispatcher);
      resolver->state = REACTOR_RESOLVER_CLOSED;
    }
}

void reactor_resolver_dispatcher_event(void *state, int type, void *data)
{
  reactor_resolver *resolver;

  (void) data;
  resolver = state;
  if (type == REACTOR_SIGNAL_DISPATCHER_MESSAGE && resolver->state == REACTOR_RESOLVER_OPEN)
    {
      reactor_user_dispatch(&resolver->user, REACTOR_RESOLVER_REPLY, resolver->gaicb.ar_result);
      reactor_resolver_close(resolver);
    }
}
