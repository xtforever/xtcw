include ../rules.mk

M=$(CURDIR)/..
LOADLIBES+=-lrt -lcairo
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -DMLS_DEBUG -I$(M)/wb
VPATH=$(M)/lib:$(WBPREFIX)
CFLAGS+=-g -Wall

all: ex3

UPDATE_WIDGETS:
	make -C ../wb

UPDATE_WCL: UPDATE_WIDGETS
	make -C ../wcl

ex3: src/Woptc.o src/Wcmd2.o xt_cairo.o $(M)/wcl/libxtcw.a


clean:
	${RM} ex3 *.o src/* doc/* vexmo/* *~
