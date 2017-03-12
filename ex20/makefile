include ../rules.mk
TARGET=batt-test

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

xtcw/Wbatt.h src/Wbatt.c: Wbatt.widget


$(TARGET): src/Wbatt.o $(M)/wcl/libxtcw.a xt_cairo.o

clean:
	${RM} -rf $(TARGET) *.o src doc xtcw *~
