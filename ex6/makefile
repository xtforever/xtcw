include ../rules.mk

M=$(CURDIR)/..
LOADLIBES+=-lrt -lcairo
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -DMLS_DEBUG -I$(M)/wb -I$(OUTPUT)/include
VPATH=$(M)/lib:$(WBPREFIX)
CFLAGS+=-g -Wall
WFLAGS= -d doc -c src -i vexmo --include-prefix=vexmo $(wb_install)/tex.w $(wb_install)/nroff.w
src/%.c vexmo/%.h: %.widget
	./wb.sh $^ $(WBUILD) $(WFLAGS)

all: ex3

UPDATE_WIDGETS:
	make -C ../wb

UPDATE_WCL: UPDATE_WIDGETS
	make -C ../wcl

ex3: src/Woptc.o src/Wcmd2.o xt_cairo.o $(M)/wcl/libxtcw.a

src/Woptc.o: Woptc.widget

src/Wcmd2.o: Wcmd2.widget


clean:
	${RM} ex3 *.o src/* doc/* vexmo/* *~
