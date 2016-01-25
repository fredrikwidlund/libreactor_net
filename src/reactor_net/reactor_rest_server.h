#ifndef REACTOR_REST_SERVER_H_INCLUDED
#define REACTOR_REST_SERVER_H_INCLUDED

enum REACTOR_REST_SERVER_EVENT
{
  REACTOR_REST_SERVER_ERROR   = 0x01,
  REACTOR_REST_SERVER_REQUEST = 0x02,
  REACTOR_REST_SERVER_CLOSE   = 0x04
};

typedef struct reactor_rest_server_options reactor_rest_server_options;
struct reactor_rest_server_options
{
  reactor_http_server_options  http_server_options;
};

typedef struct reactor_rest_server reactor_rest_server;
struct reactor_rest_server
{
  reactor_user                 user;
  reactor_http_server          http_server;
  vector                       maps;
};

typedef struct reactor_rest_server_map reactor_rest_server_map;
struct reactor_rest_server_map
{
  char                        *method;
  char                        *path;
  void                        *state;
};

typedef struct reactor_rest_server_request reactor_rest_server_request;
struct reactor_rest_server_request
{
  reactor_http_server_session *session;
  void                        *state;
};

void reactor_rest_server_init(reactor_rest_server *, reactor_user_call *, void *);
int  reactor_rest_server_open(reactor_rest_server *, char *, char *, reactor_rest_server_options *);
void reactor_rest_server_event(void *, int, void *);
int  reactor_rest_server_add(reactor_rest_server *, char *, char *, void *);
int  reactor_rest_server_match(reactor_rest_server_map *, reactor_http_server_request *);
void reactor_rest_server_respond(reactor_rest_server_request *, int, char *, char *, size_t);
void reactor_rest_server_respond_text(reactor_rest_server_request *, char *);
void reactor_rest_server_respond_clo(reactor_rest_server_request *, int, clo *);
void reactor_rest_server_respond_error(reactor_rest_server_request *, int, char *);

#endif /* REACTOR_REST_SERVER_H_INCLUDED */
