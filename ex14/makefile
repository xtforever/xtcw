include ../rules.mk

M=$(CURDIR)/..
LOADLIBES+=-lrt -lcairo
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -I$(M)/wb
VPATH=$(M)/lib
WFLAGS= -d doc -c src -i vexmo --include-prefix=vexmo $(wb_install)/tex.w $(wb_install)/nroff.w

src/%.c vexmo/%.h: %.widget
	$(wb_install)/wb.sh $^ $(WBUILD) $(WFLAGS)

all: ex14

ex14: $(M)/wcl/libxtcw.a font.c xt_cairo.c

## src/Wtxt.o: Wtxt.widget

clean:
	${RM} -rf ex14 *.o src doc vexmo *~
