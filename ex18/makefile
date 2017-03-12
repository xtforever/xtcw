include ../rules.mk
TARGET=ex18

M=$(CURDIR)/..
LOADLIBES+=
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -I$(OUTPUT)/include
VPATH=$(M)/lib

LOADLIBES+=$(shell pkg-config taglib_c --libs) -lcairo -lXxf86vm
CFLAGS+=$(shell pkg-config taglib_c --cflags)
WFLAGS= -d doc -c src -i xtcw --include-prefix=xtcw $(wb_install)/tex.w $(wb_install)/nroff.w

all: $(TARGET) xcal xtcal

src/%.c vexmo/%.h: %.widget
	wb_search=../wb ../wbuild/wb.sh $^ $(WBUILD) $(WFLAGS)

xtcw/LightTxt.h src/LightTxt.c: LightTxt.widget

xtcw/Calib.h src/Calib.c: Calib.widget

xtcw/WviewQV.h src/WviewQV.c: WviewQV.widget


$(TARGET): $(M)/wcl/libxtcw.a xt_cairo.o lfont.o src/LightTxt.o src/WviewQV.o src/Calib.o

xcal: $(M)/wcl/libxtcw.a xt_cairo.o src/Calib.o xborderless.o fullscreen.o

clean:
	${RM} -rf $(TARGET) *.o src doc xtcw *~ xcal xtcal

xtcal: src/Calib.o xborderless.o fullscreen.o
