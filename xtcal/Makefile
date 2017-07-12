wb_install=../wbuild
WBUILD=../wbuild/wbuild
CFLAGS +=-Wall -std=gnu99
CPPFLAGS+=-I. -D_GNU_SOURCE
WFLAGS= -d doc -c src -i xtcw --include-prefix=xtcw $(wb_install)/tex.w $(wb_install)/nroff.w
LOADLIBES=-lX11 -lXaw -lXft -lXt -lXext -lXmu -lXi -lXxf86vm

ifeq ($(debug),1)
CFLAGS+=-g -O0 -DDEBUG
else
CFLAGS+=-Os
endif


%.h %c: %.widget
	wb_search=../wb $(wb_install)/wb.sh $^ $(WBUILD) $(WFLAGS)

all: xtcal

xtcw/Calib.h src/Calib.c: Calib.widget
	wb_search=../wb $(wb_install)/wb.sh $^ $(WBUILD) $(WFLAGS)

src/Calib.o: src/Calib.c xtcw/Calib.h

xtcal: src/Calib.o xborderless.o solver.o xinput-util.o xfullscreen.o

clean:
	-${RM} -r *.o xtcal

mrproper:
	-${RM} -r xtcw *.o xtcal src doc
