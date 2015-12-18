#ifndef REACTOR_TCP_SERVER_H_INCLUDED
#define REACTOR_TCP_SERVER_H_INCLUDED

enum REACTOR_TCP_SERVER_EVENTS
{
  REACTOR_TCP_SERVER_ERROR  = 0x01,
  REACTOR_TCP_SERVER_ACCEPT = 0x02,
  REACTOR_TCP_SERVER_CLOSE  = 0x04
};

enum REACTOR_TCP_SERVER_STATE
{
  REACTOR_TCP_SERVER_CLOSED = 0,
  REACTOR_TCP_SERVER_OPEN,
  REACTOR_TCP_SERVER_CLOSING
};

typedef struct reactor_tcp_server reactor_tcp_server;
struct reactor_tcp_server
{
  int                     state;
  reactor_user            user;
  reactor_desc            desc;
};

typedef struct reactor_tcp_server_data reactor_tcp_server_data;
struct reactor_tcp_server_data
{
  int                     fd;
  struct sockaddr_storage addr;
  socklen_t               addr_len;
};

void reactor_tcp_server_init(reactor_tcp_server *, reactor_user_call *, void *);
int  reactor_tcp_server_open(reactor_tcp_server *, char *, char *);
void reactor_tcp_server_event(void *, int, void *);
int  reactor_tcp_server_listen(reactor_tcp_server *, int, struct sockaddr *, socklen_t);
void reactor_tcp_server_error(reactor_tcp_server *);
void reactor_tcp_server_close(reactor_tcp_server *);

#endif /* REACTOR_TCP_SERVER_H_INCLUDED */
