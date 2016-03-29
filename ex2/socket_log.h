#ifndef SOCKET_LOG
#define SOCKET_LOG


#include <X11/Intrinsic.h>
#include <stdio.h>
#include <stdarg.h>

typedef void (*slog_cb_t) (int sfd, char *buf, int n); 
#define MAX_CLIENT_SOCKETS 100

struct socket_log_state {
  XtAppContext app;
  int client_socket[MAX_CLIENT_SOCKETS*2];
  int buffer;
  int n_connections;
  slog_cb_t cb;
};

extern struct socket_log_state SLOG;
extern int trace_slog;

int slog_init(Widget top, char *port, slog_cb_t cb);

void slog_sendto_all(char *buf, int len);
void slog_mputs( int buf );
void slog_write( const char *s, ... );
void slog_write_va(const char *s, va_list ap );

#endif
