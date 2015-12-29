#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>

#include <dynamic.h>
#include <reactor_core.h>

#include "picohttpparser.h"
#include "reactor_tcp_server.h"
#include "reactor_http_server.h"

void reactor_http_server_init(reactor_http_server *http_server, reactor_user_call *call, void *state)
{
  reactor_user_init(&http_server->user, call, state);
  reactor_tcp_server_init(&http_server->tcp_server, reactor_http_server_event, http_server);
}

int reactor_http_server_open(reactor_http_server *http_server, char *node, char *service)
{
  return reactor_tcp_server_open(&http_server->tcp_server, node ? node : "0.0.0.0", service ? service : "http");
}

void reactor_http_server_event(void *state, int type, void *data)
{
  reactor_http_server *http_server;
  reactor_http_server_session *session;
  reactor_tcp_server_data *client;
  int e;

  http_server = state;
  switch (type)
    {
    case REACTOR_TCP_SERVER_ACCEPT:
      client = data;
      session = malloc(sizeof *session);
      if (!session)
        {
          reactor_http_server_error(http_server);
          break;
        }
      session->server = http_server;
      session->request.base = NULL;
      reactor_stream_init(&session->stream, reactor_http_server_session_event, session);
      e = reactor_stream_open(&session->stream, client->fd);
      if (e == -1)
        {
          (void) close(client->fd);
          free(session);
          break;
        }
      reactor_user_dispatch(&http_server->user, REACTOR_HTTP_SERVER_ACCEPT, session);
      break;
    case REACTOR_TCP_SERVER_CLOSE:
      reactor_user_dispatch(&http_server->user, REACTOR_HTTP_SERVER_CLOSE, NULL);
      break;
    case REACTOR_TCP_SERVER_ERROR:
      reactor_user_dispatch(&http_server->user, REACTOR_HTTP_SERVER_ERROR, data);
      break;
    }
}

void reactor_http_server_error(reactor_http_server *http_server)
{
  reactor_user_dispatch(&http_server->user, REACTOR_HTTP_SERVER_ERROR, http_server);
  reactor_http_server_close(http_server);
}

void reactor_http_server_close(reactor_http_server *http_server)
{
  reactor_tcp_server_close(&http_server->tcp_server);
}

void reactor_http_server_session_event(void *state, int type, void *data)
{
  reactor_http_server_session *session;

  session = state;
  switch (type)
    {
    case REACTOR_STREAM_DATA:
      reactor_http_server_session_data(session, data);
      break;
    case REACTOR_STREAM_END:
    case REACTOR_STREAM_ERROR:
      reactor_stream_close(&session->stream);
      break;
    case REACTOR_STREAM_CLOSE:
      free(session);
      break;
    }
}

void reactor_http_server_session_data(reactor_http_server_session *session, reactor_stream_data *data)
{
  reactor_http_server_request *request;
  struct phr_header headers[REACTOR_HTTP_HEADER_MAX_FIELDS];
  size_t headers_count = REACTOR_HTTP_HEADER_MAX_FIELDS;
  size_t method_size, path_size;
  ssize_t body_size;
  int header_size, i;
  uintptr_t offset;

  request = &session->request;

  while (data->size)
    {
      if (request->base && data->size < request->size)
        return;

      if (!request->base)
        {
          header_size = phr_parse_request(data->base, data->size, (const char **) &request->method, &method_size,
                                          (const char **) &request->path, &path_size, &request->minor_version,
                                          headers, &headers_count, 0);
          if (header_size < 0)
            {
              if (header_size == -1)
                reactor_stream_close(&session->stream);
              return;
            }
          request->base = data->base;
          request->method[method_size] = '\0';
          request->path[path_size] = '\0';
          request->field_count = headers_count;
          body_size = -1;
          for (i = 0; i < (int) headers_count; i ++)
            {
              request->field[i].key = (char *) headers[i].name;
              request->field[i].key[headers[i].name_len] = '\0';
              request->field[i].value = (char *) headers[i].value;
              request->field[i].value[headers[i].value_len] = '\0';
              if (body_size == -1 && strcasecmp(request->field[i].key, "content-length") == 0)
                body_size = strtol(request->field[i].value, NULL, 0);
            }
          if (body_size < 0)
            body_size = 0;
          request->size = header_size + body_size;
          request->body = request->base + header_size;
          request->body_size = body_size;
          if (data->size < request->size)
            return;
        }

      if (request->base != data->base)
        {
          offset = data->base - request->base;
          request->base += offset;
          request->method += offset;
          request->path += offset;
          request->body += offset;
          for (i = 0; i < request->field_count; i ++)
            {
              request->field[i].key += offset;
              request->field[i].value += offset;
            }
        }

      reactor_user_dispatch(&session->server->user, REACTOR_HTTP_SERVER_REQUEST, session);
      reactor_stream_data_consume(data, request->size);
      request->base = NULL;
    }

  reactor_stream_flush(&session->stream);
}

void reactor_http_server_respond(reactor_http_server_session *session, char *code, char *message, char *type,
				 char *body, size_t size)
{
  reactor_stream *stream;

  stream = &session->stream;

  reactor_stream_puts(stream, "HTTP/1.1 ");
  reactor_stream_puts(stream, code);
  reactor_stream_puts(stream, " ");
  reactor_stream_puts(stream, message);
  reactor_stream_puts(stream, "\r\nContent-type: ");
  reactor_stream_puts(stream, type);
  reactor_stream_puts(stream, "\r\nContent-Length: ");
  reactor_stream_putu(stream, size);
  reactor_stream_puts(stream, "\r\n\r\n");

  reactor_stream_write(stream, body, size);
}
