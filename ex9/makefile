include ../rules.mk

M=$(CURDIR)/..
LOADLIBES+=-lrt -lcairo
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -I$(M)/wb
VPATH=$(M)/lib
WFLAGS= -d doc -c src -i vexmo --include-prefix=vexmo $(wb_install)/tex.w $(wb_install)/nroff.w

src/%.c vexmo/%.h: %.widget
	$(wb_install)/wb.sh $^ $(WBUILD) $(WFLAGS)

all: ex9

ex9: src/Wedit.o $(M)/wcl/libxtcw.a xt_cairo.o

src/Wedit.o: Wedit.widget

clean:
	${RM} -rf ex9 *.o src doc vexmo *~
