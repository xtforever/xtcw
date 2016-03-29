#ifndef SOCKET_LOG
#define SOCKET_LOG


#include <X11/Intrinsic.h>
#include <stdio.h>
#include <stdarg.h>

void slog_init(Widget top);

void slog_mputs( int buf );
void slog_write( const char *s, ... );
void slog_write_va(const char *s, va_list ap );

#endif
