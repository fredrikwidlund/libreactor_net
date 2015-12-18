#ifndef REACTOR_HTTP_SERVER_H_INCLUDED
#define REACTOR_HTTP_SERVER_H_INCLUDED

#ifndef REACTOR_HTTP_HEADER_MAX_FIELDS
#define REACTOR_HTTP_HEADER_MAX_FIELDS 32
#endif

enum REACTOR_HTTP_SERVER_EVENT
{
  REACTOR_HTTP_SERVER_ERROR   = 0x01,
  REACTOR_HTTP_SERVER_ACCEPT  = 0x02,
  REACTOR_HTTP_SERVER_REQUEST = 0x04,
  REACTOR_HTTP_SERVER_CLOSE   = 0x08
};

typedef struct reactor_http_server reactor_http_server;
struct reactor_http_server
{
  reactor_user                    user;
  reactor_tcp_server              tcp_server;
};

typedef struct reactor_http_header_field reactor_http_header_field;
struct reactor_http_header_field
{
  char                           *key;
  char                           *value;
};

typedef struct reactor_http_server_request reactor_http_server_request;
struct reactor_http_server_request
{
  char                           *base;
  size_t                          size;
  char                           *method;
  char                           *path;
  char                           *body;
  size_t                          body_size;
  int                             minor_version;
  int                             field_count;
  reactor_http_header_field       field[REACTOR_HTTP_HEADER_MAX_FIELDS];
};

typedef struct reactor_http_server_session reactor_http_server_session;
struct reactor_http_server_session
{
  reactor_http_server            *server;
  reactor_stream                  stream;
  reactor_http_server_request     request;
};


void            reactor_http_server_init(reactor_http_server *, reactor_user_call *, void *);
int             reactor_http_server_open(reactor_http_server *, char *, char *);
void            reactor_http_server_event(void *, int, void *);
void            reactor_http_server_error(reactor_http_server *);
void            reactor_http_server_close(reactor_http_server *);

void            reactor_http_server_session_init(reactor_http_server_session *, reactor_user_call *, void *);
int             reactor_http_server_session_open(reactor_http_server_session *, int);
void            reactor_http_server_session_event(void *, int, void *);
void            reactor_http_server_session_data(reactor_http_server_session *, reactor_stream_data *);

#endif /* REACTOR_HTTP_SERVER_H_INCLUDED */
