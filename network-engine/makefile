include ../rules.mk
TARGET=cwnet
OBJS=xt_cairo.o slopnet.o slop4.o mrb.o  table256-crc16.o command-parser.o

M=$(CURDIR)/..
LOADLIBES+=
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -I$(OUTPUT)/include
VPATH=$(M)/lib src

LOADLIBES+=-lcairo
## CFLAGS+=$(shell pkg-config taglib_c --cflags)
WFLAGS= -d doc -c src -i xtcw --include-prefix=xtcw $(wb_install)/tex.w $(wb_install)/nroff.w

xtcw/%.h src/%.c: %.widget
	wb_search=../wb ../wbuild/wb.sh $^ $(WBUILD) $(WFLAGS)

all: $(TARGET)  lua-server lua-client

$(TARGET): $(OBJS) $(M)/wcl/libxtcw.a

$(TARGET)-dyn: $(OBJS) $(TARGET).o
	gcc -o $@ $(TARGET).o $(OBJS) $(M)/wcl/libxtcw.so.1.0.0 $(LOADLIBES)  

## slop-server: $(M)/wcl/libxtcw.a  $(OBJS)
## slop-server: CFLAGS+=-DTEST_MAIN

lua-server: CFLAGS+=$(shell pkg-config lua5.3 --cflags)
lua-server: LOADLIBES+=$(shell pkg-config lua5.3 --libs)
lua-server: $(OBJS) slop-server.o command-parser.o  luaexec.o $(M)/wcl/libxtcw.a

lua-client: $(OBJS) slop-server.o command-parser.o  $(M)/wcl/libxtcw.a

lua-test: CFLAGS+=$(shell pkg-config lua5.3 --cflags)
lua-test: LOADLIBES+=$(shell pkg-config lua5.3 --libs)

clean:
	${RM} -rf $(TARGET) *.o src doc xtcw *~ lua-client lua-server lua-test cwnet-dyn slop-server
