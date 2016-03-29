include ../rules.mk
TARGET=ex11

M=$(CURDIR)/..
LOADLIBES+=
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -I$(OUTPUT)/include
VPATH=$(M)/lib

LOADLIBES+=$(shell pkg-config taglib_c --libs) -lcairo
CFLAGS+=$(shell pkg-config taglib_c --cflags)

all: make-db $(TARGET)

$(TARGET): $(M)/wcl/libxtcw.a db.o communication.o if_mpg123.o mrb.o xt_cairo.o

make-db: test-id3read

test-id3read: id3read.o mls.o

clean:
	${RM} -rf $(TARGET) *.o src doc xtcw *~ test-id3read
