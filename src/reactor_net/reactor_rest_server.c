#define _GNU_SOURCE

#include <stdint.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>

#include <dynamic.h>
#include <clo.h>
#include <reactor_core.h>

#include "reactor_tcp_server.h"
#include "reactor_http_server.h"
#include "reactor_rest_server.h"

void reactor_rest_server_init(reactor_rest_server *server, reactor_user_call *call, void *state)
{
  reactor_user_init(&server->user, call, state);
  reactor_http_server_init(&server->http_server, reactor_rest_server_event, server);
  vector_init(&server->maps, sizeof(reactor_rest_server_map));
}

int reactor_rest_server_open(reactor_rest_server *server, char *node, char *service,
                             reactor_rest_server_options *options)
{
  return reactor_http_server_open(&server->http_server, node, service, &options->http_server_options);
}

void reactor_rest_server_event(void *state, int type, void *data)
{
  reactor_rest_server *server;
  reactor_rest_server_map *map;
  reactor_rest_server_request request;
  reactor_http_server_session *session;
  size_t i;

  server = state;
  switch (type)
    {
    case REACTOR_HTTP_SERVER_REQUEST:
      session = data;
      for (i = 0; i < vector_size(&server->maps); i ++)
        {
          map = vector_at(&server->maps, i);
          if (reactor_rest_server_match(map, &session->request))
            {
              request = (reactor_rest_server_request) {.session = data, .state = map->state};
              reactor_user_dispatch(&server->user, REACTOR_REST_SERVER_REQUEST, &request);
              return;
            }
        }
      reactor_rest_server_respond_error((reactor_rest_server_request[]){{.session = session}}, 404, "not found");
      break;
    case REACTOR_HTTP_SERVER_ERROR:
      reactor_user_dispatch(&server->user, REACTOR_REST_SERVER_ERROR, data);
      break;
    }
}

int reactor_rest_server_add(reactor_rest_server *server, char *method, char *path, void *state)
{
  reactor_rest_server_map map = {.method = strdup(method), .path = strdup(path), state = state};

  return vector_push_back(&server->maps, &map);
}

int reactor_rest_server_match(reactor_rest_server_map *map, reactor_http_server_request *request)
{
  return ((!map->method || strcasecmp(map->method, request->method) == 0) &&
          (!map->path || strcasecmp(map->path, request->path) == 0));
}

void reactor_rest_server_respond(reactor_rest_server_request *request, int code, char *content_type, char *body, size_t size)
{
  reactor_http_server_respond(request->session, code, content_type, body, size);
}

void reactor_rest_server_respond_text(reactor_rest_server_request *request, char *body)
{
  reactor_rest_server_respond(request, 200, "text/plain", body, strlen(body));
}

void reactor_rest_server_respond_clo(reactor_rest_server_request *request, int code, clo *clo)
{
  buffer b;
  int e;

  buffer_init(&b);
  e = 0;
  clo_encode(clo, &b, &e);
  if (e == 0)
    reactor_rest_server_respond(request, code, "application/json", buffer_data(&b), buffer_size(&b));
  buffer_clear(&b);
}

void reactor_rest_server_respond_error(reactor_rest_server_request *request, int code, char *message)
{
  reactor_rest_server_respond_clo(request, code, (clo[]) {clo_object({"status", clo_string("error")},
                                                                     {"code", clo_number(code)},
                                                                     {"reason", clo_string(message)})});
}
