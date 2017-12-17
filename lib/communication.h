
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

int sock_accept(int sockfd);
int sock_init_listen( int portno );
int sock_connect(char *server_ip, int portno );
int sock_connect_service(char *server_ip, char *port );


/* new interface */
int sock_listen_on_port(char *port);
int sock_make_socket_non_blocking (int sfd);
int sock_make_socket_blocking (int sfd);
int sock_enable_blocking (int sfd, int enable );
int sock_create_and_bind (char *port);
int sock_accept_incomming_connection(int sfd);


/** todo: system_error() - log file */
#define system_error(f,a...) \
    do { fprintf(stderr,f, ##a ); perror(__func__); } while(0)

#define system_debug(f,a...) \
    do { fprintf(stderr, "%s: ",__func__); fprintf(stderr,f, ##a ); } while(0)

#endif
