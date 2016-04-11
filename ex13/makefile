include ../rules.mk
TARGET=ex13

M=$(CURDIR)/..
LOADLIBES+=
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -I$(OUTPUT)/include
VPATH=$(M)/lib

LOADLIBES+= -lcairo
CFLAGS+=

all: $(TARGET)

$(TARGET): $(M)/wcl/libxtcw.a xt_cairo.o mrb.o pipeshell.o

clean:
	${RM} -rf $(TARGET) *.o src doc xtcw *~
