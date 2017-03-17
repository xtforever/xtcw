include ../rules.mk
TARGET=video

M=$(CURDIR)/..
LOADLIBES+=
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -I$(OUTPUT)/include
VPATH=$(M)/lib src
CFLAGS+=-O3
LOADLIBES+=-lcairo -lv4l2
WFLAGS= -d doc -c src -i xtcw --include-prefix=xtcw $(wb_install)/tex.w $(wb_install)/nroff.w

xtcw/%.h src/%.c: %.widget
	wb_search=../wb ../wbuild/wb.sh $^ $(WBUILD) $(WFLAGS)

all: $(TARGET)

xtcw/Wcap.h src/Wcap.c: Wcap.widget

$(TARGET):  src/Wcap.c $(M)/wcl/libxtcw.a xt_cairo.o xv4l2.o xshm-util.o

clean:
	${RM} -rf $(TARGET) *.o src doc xtcw *~
