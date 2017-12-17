#ifndef SLOPNET_H
#define SLOPNET_H

#include <stdio.h>

/* called if a message has arrived */
typedef void (*sln_input_t) (int error, int msg, int sln, void *ctx); 

int sln_init(void);
void sln_destruct(void);
void sln_free(int sln);

/* returns fd for socket */
int sln_listen(int sln, int port);

/* returns fd for socket */
int sln_connect(int sln, char *host, char *service, sln_input_t cb, void *ctx );
		
int sln_input_cb(int n);	       

int sln_printf(int n, char *format, ... );

int sln_vaprintf(int n, char *format, va_list va );

int sln_accept(int sln, int fd, sln_input_t cb, void *ctx );		

int sln_lookup_fd(int fd);
int sln_get_fd(int sln);

int sock_write(int fd, int msg);

void sln_set_exit_flag(int flag);
void sln_server_select(int listener, sln_input_t cb, void *ctx);
void sln_client_select(int sln);
int sln_server_loop(char *port,  sln_input_t cb, void *ctx );

#endif
		
