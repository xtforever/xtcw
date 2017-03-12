include ../rules.mk

M=$(CURDIR)/..
LOADLIBES+=-lrt -lcairo
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -I$(M)/wb
VPATH=$(M)/lib
WFLAGS= -d doc -c src -i vexmo --include-prefix=vexmo $(wb_install)/tex.w $(wb_install)/nroff.w

src/%.c vexmo/%.h: %.widget
	$(wb_install)/wb.sh $^ $(WBUILD) $(WFLAGS)

all: vdemo

vdemo: src/Wdemo.o $(M)/wcl/libxtcw.a xt_cairo.o

src/Wdemo.o: Wdemo.widget

clean:
	${RM} -rf ex12 *.o src doc vexmo *~ vdemo
