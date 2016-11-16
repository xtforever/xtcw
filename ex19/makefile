include ../rules.mk
TARGET=ex19

M=$(CURDIR)/..
LOADLIBES+=
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -I$(OUTPUT)/include
VPATH=$(M)/lib src

LOADLIBES+=$(shell pkg-config taglib_c --libs) -lcairo
CFLAGS+=$(shell pkg-config taglib_c --cflags)
WFLAGS= -d doc -c src -i xtcw --include-prefix=xtcw $(wb_install)/tex.w $(wb_install)/nroff.w

xtcw/%.h src/%.c: %.widget
	wb_search=../wb ../wbuild/wb.sh $^ $(WBUILD) $(WFLAGS)


all: $(TARGET)

xtcw/WviewQV.h src/WviewQV.c: WviewQV.widget
xtcw/Wlab.h src/Wlab.c: Wlab.widget
xtcw/Wsel.h src/Wsel.c: Wsel.widget
xtcw/Wsel2.h src/Wsel2.c: Wsel2.widget
xtcw/KaroEd.h src/KaroEd.c: KaroEd.widget
xtcw/TermEd.h src/TermEd.c: TermEd.widget

$(TARGET):  src/TermEd.o src/KaroEd.o src/Wsel2.o src/Wsel.o src/Wmvar.o src/WviewQV.o src/Wlab.o src/DigTab.o $(M)/wcl/libxtcw.a xt_cairo.o mrb.o subshell.o


clean:
	${RM} -rf $(TARGET) *.o src doc xtcw *~
