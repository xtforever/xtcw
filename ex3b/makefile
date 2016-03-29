include ../rules.mk

LOADLIBES+=../wcl/libxtcw.so.1.0.0 -lrt
CPPFLAGS+=-I../lib -I.. -I../wcl -DMLS_DEBUG -I../wb
VPATH=$(M)/lib
CFLAGS+=-g -Wall

all: ex3

clean:
	${RM} ex3 *.o
