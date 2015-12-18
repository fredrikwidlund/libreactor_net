#define _GNU_SOURCE

#include <stdint.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>

#include <dynamic.h>
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

int reactor_rest_server_open(reactor_rest_server *server, char *node, char *service)
{
  return reactor_http_server_open(&server->http_server, node, service);
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
      reactor_rest_server_return_not_found((reactor_rest_server_request[]){{.session = session}});
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

void reactor_rest_server_return(reactor_rest_server_request *request, int code, char *reason, char *content_type, char *body, size_t size)
{
  reactor_stream_printf(&request->session->stream,
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Type: %s\r\n"
                        "Content-Length: %llu\r\n\r\n",
                        code, reason, content_type, size, body);
  if (size)
    reactor_stream_write(&request->session->stream, body, size);
}

void reactor_rest_server_return_not_found(reactor_rest_server_request *request)
{
  char *not_found_body = "{\"status\":\"error\", \"code\":404, \"reason\":\"not found\"}";
  reactor_rest_server_return(request, 404, "Not Found", "application/json", not_found_body, strlen(not_found_body));
}

void reactor_rest_server_return_text(reactor_rest_server_request *request, char *body)
{
  reactor_rest_server_return(request, 200, "OK", "text/html", body, strlen(body));
}
