include ../rules.mk

M=$(CURDIR)/..
LOADLIBES+=-lrt -lImlib2
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -DMLS_DEBUG -I$(M)/wb
VPATH=$(M)/lib
CFLAGS+=-g -Wall
WFLAGS= -d doc -c src -i vexmo --include-prefix=vexmo $(wb_install)/tex.w $(wb_install)/nroff.w

src/%.c vexmo/%.h: %.widget
	./wb.sh $^ $(WBUILD) $(WFLAGS)

all: ex3

UPDATE_WIDGETS:
	make -C ../wb

UPDATE_WCL: UPDATE_WIDGETS
	make -C ../wcl

ex3: src/Wim2.o $(M)/wcl/libxtcw.a

clean:
	${RM} ex3 *.o src/*
