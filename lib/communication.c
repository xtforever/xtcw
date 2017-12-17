#include "communication.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include "mls.h"



/** returns: socket fd */
int sock_create_and_bind (char *port)
{
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int s, sfd;

  memset (&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
  hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
  hints.ai_flags = AI_PASSIVE;     /* All interfaces */

  s = getaddrinfo (NULL, port, &hints, &result);
  if (s != 0)
    {
        system_error( "getaddrinfo: %s\n", gai_strerror (s));
        return -1;
    }

  for (rp = result; rp != NULL; rp = rp->ai_next)
    {
      sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sfd == -1)
        continue;

      s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
      if (s == 0)
        {
          /* We managed to bind successfully! */
          break;
        }

      close (sfd);
    }

  if (rp == NULL)
    {
        system_error( "Could not bind to port %s\n", port);
        return -1;
    }

  freeaddrinfo (result);
  return sfd;
}


/** returns: 0 if successfull */
int sock_make_socket_non_blocking (int sfd)
{
    return sock_enable_blocking(sfd, 0);
}

/** returns: 0 if successfull */
int sock_make_socket_blocking (int sfd)
{
    return sock_enable_blocking(sfd, 1);
}



/** returns: 0 if successfull */
int sock_enable_blocking (int sfd, int enable )
{
  int flags, s;

  flags = fcntl (sfd, F_GETFL, 0);
  if (flags == -1)
    {
      perror ("fcntl");
      return -1;
    }

  if( enable ) flags &= ~ O_NONBLOCK; else flags |= O_NONBLOCK;
  s = fcntl (sfd, F_SETFL, flags);
  if (s == -1)
    {
      perror ("fcntl");
      return -1;
    }

  return 0;
}


/** create a socket, bind socket to |port| and listen for incomming conn.
    returns: sfd - socket file descriptor or -1 - if error
*/
int sock_listen_on_port(char *port)
{
    int sfd, s;

    sfd = sock_create_and_bind ( port );
    if (sfd <=0 ) return -1;

    s = sock_make_socket_non_blocking (sfd);
    if (s < 0 ) { close(sfd); return -1; }

    s = listen (sfd, SOMAXCONN);
    if (s < 0) {
        system_error("listen");
        close(sfd);
        return -1;
    }

    return sfd;
}

/** returns: -1 - no more connections pending,
    >0 - file descriptor of newly connected socket
         the socket is NON BLOCKING.
*/
int sock_accept_incomming_connection(int sfd)
{
    struct sockaddr in_addr;
    socklen_t in_len;
    int s,infd;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

    in_len = sizeof in_addr;
    infd = accept (sfd, &in_addr, &in_len);
    if (infd == -1)
        {
            if ((errno == EAGAIN) ||
                (errno == EWOULDBLOCK))
                {
                    /* We have processed all incoming
                       connections. */
                    return -1;
                }
            else
                {
                    system_error("accept");
                    return -1;
                }
        }

    s = getnameinfo (&in_addr, in_len,
                     hbuf, sizeof hbuf,
                     sbuf, sizeof sbuf,
                     NI_NUMERICHOST | NI_NUMERICSERV);
    if (s == 0)
        {
            system_debug("Accepted connection on descriptor %d "
                   "(host=%s, port=%s)\n", infd, hbuf, sbuf);
        }

    /* Make the incoming socket non-blocking */
    s = sock_make_socket_non_blocking (infd);
    if (s == -1) { close(infd); return -1; }
    return infd;
}


int sock_init_listen( int portno )
{
  int sockfd;
  struct sockaddr_in serv_addr;


  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) ERR("ERROR opening socket");


  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
	   sizeof(serv_addr)) < 0)
    ERR("ERROR on binding");

  listen(sockfd,5);
  return sockfd;
}

int sock_accept(int sockfd)
{
  struct sockaddr_in cli_addr;
  uint clilen = sizeof(cli_addr);
  int newsockfd = accept(sockfd,
		     (struct sockaddr *) &cli_addr,
		     &clilen);
  if (newsockfd < 0)
    ERR("ERROR on accept");

  char *s; int p;
  s = inet_ntoa( cli_addr.sin_addr );
  p = ntohs( cli_addr.sin_port );
  TRACE(2,"connect from %s:%u", s,p );
  return newsockfd;
}

int sock_connect_service(char *server_ip, char *port )
{
  struct servent *servinfo;
  int portno;

  if( !port || !*port ) {
    system_error("you must provide a service port\n");
    exit(1);
  }

  if( isdigit(*port) ) portno = atoi(port);
  else {
    servinfo = getservbyname( port, "tcp");
    if(!servinfo) {
      system_error("no %s service\n", port);
      exit(1);
    }
    portno = ntohs( servinfo->s_port );
  }
  return sock_connect( server_ip, portno );

}

int sock_connect(char *server_ip, int portno )
{
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
	return -1;
    }
    server = gethostbyname( server_ip );
    if (server == NULL) {
        perror( "ERROR no such host");
	close(sockfd);
	return -1;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
	int err = errno;
	if( err == EINPROGRESS )
	    return sockfd;

	system_error( "ERROR %s\ncould not connect to port %d\n",
		      strerror(errno), portno );

	close(sockfd);
	return -1;
    }

    return sockfd;
}
