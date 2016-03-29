include ../rules.mk

M=$(CURDIR)/..
LOADLIBES+=-lrt 
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -I$(M)/wb
VPATH=$(M)/lib
WFLAGS= -d doc -c src -i vexmo --include-prefix=vexmo $(wb_install)/tex.w $(wb_install)/nroff.w

src/%.c vexmo/%.h: %.widget
	./wb.sh $^ $(WBUILD) $(WFLAGS)

all: ex8

ex8: src/Wdemo.o $(M)/wcl/libxtcw.a

src/Wdemo.o: Wdemo.widget

clean:
	${RM} -rf ex8 *.o src doc vexmo *~

