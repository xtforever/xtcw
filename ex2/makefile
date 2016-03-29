include ../rules.mk

M=$(CURDIR)/../gui

LOADLIBES+=$(M)/wcl/libvexmo.so.1.0.0 -lrt
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -DMLS_DEBUG -I$(M)/wb
VPATH=$(M)/lib
CFLAGS+=-g -Wall 

all: exbasic2

exbasic2: socket_log.o

clean:
	${RM} exbasic2 *.o



