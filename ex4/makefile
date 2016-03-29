include ../rules.mk

M=$(CURDIR)/..
LOADLIBES+=-lrt
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -DMLS_DEBUG -I$(M)/wb
VPATH=$(M)/lib
CFLAGS+=-g -Wall

all:  ex3

ex3: src/Woptc.o $(M)/wcl/libxtcw.a

clean:
	${RM} ex3 *.o src/* doc/* xtcw/*
