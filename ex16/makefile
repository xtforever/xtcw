Xawinclude ../rules.mk
TARGET=ex16

M=$(CURDIR)/..
LOADLIBES+=
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -I$(OUTPUT)/include
VPATH=$(M)/lib

LOADLIBES+=$(shell pkg-config taglib_c --libs) -lcairo
CFLAGS+=$(shell pkg-config taglib_c --cflags)

all: $(TARGET)

## $(TARGET): $(M)/wcl/libxtcw.a xt_cairo.o mrb.o pipeshell.o
$(TARGET): $(M)/wcl/libxtcw.so.1.0.0 xt_cairo.o

clean:
	${RM} -rf $(TARGET) *.o src doc xtcw *~
