#ifndef SRVCONNECTION_H

#define  SRVCONNECTION_H


#include <X11/Intrinsic.h>


typedef void (*srvc_cb)(Widget w, void *user, void *data);
int srvc_connect(Widget w, char *host, char *port, srvc_cb cb, void *user);


extern int SRVC;

#endif
