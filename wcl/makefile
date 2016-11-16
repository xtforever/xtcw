#  Xt Library Core Widget Set (xcws)
#
# Based on the Xt Core Widget and the
# great Xt Library
#
#
#
# Erstellen der XTCW library
#
# wcl/     - wcl quellen und die lib selber
# wb/      - widgets fuer den wbuilder
#            wbuilder speichert die erzeugten
#            widgets in den Verzeichnissen
#              wb/src, wb/xtcw
# widgets/ - alle widgets die ohne wbuilder auskommen
# lib/     - zusatzliche funktionen
#
#
# install: die lib wird nach $libdir kopiert
#          die includes landen in $includedir/xtcw
#
#
#
include ../rules.mk

STATIC_LIB=libxtcw.a
SL_BASE_NAME=xtcw
SL_MAJOR=1
SL_MINOR=0
SL_REV=0

SL_VERSION=$(SL_MAJOR).$(SL_MINOR).$(SL_REV)
SL_NAME=lib$(SL_BASE_NAME).so.$(SL_VERSION)
SL=lib$(SL_BASE_NAME).so

VPATH=../lib:../build/src:../widgets
CPPFLAGS+=-I../lib -I../build/include -I../widgets -I.. -I../build/include/xtcw
INC=${includedir}/xtcw

WIDGETS=$(wildcard ../wb/*.widget)
WIDGET_OBJS:=${WIDGETS:../wb/%.widget=%.o}
WIDGET_C:=${WIDGETS:../wb/%.widget=src/%.c}

OBJS=focus-group.o Gridbox.o converters-xft.o xutil.o mls.o
OBJS+=ini_parse.tab.o ini_lex.lex.o ini_read2.o
OBJS+=converters.o util.o xlayout.o wcreg2.o xprint2.o
OBJS+=micro_vars.o parser_util.o communication.o
OBJS+=XCC.o long.o icon.o choosecol.o strarray.o StrToPmap.o
OBJS+=Pen.o str2color.o
OBJS+=$(WIDGET_OBJS)
OBJS+=MapAg.o TableVec.o WcCreate.o WcLoadRes.o WcRegXt.o WcWarn.o
OBJS+=Table.o WcActCB.o WcInvoke.o WcName.o WcSetValue.o Xp.o
OBJS+=TableLoc.o WcConvert.o WcLateBind.o WcReg.o WcTemplate.o
OBJS+=register_wb.o
OBJS+=font.o

ALL_LO=${OBJS:%.o=%.lo}

ALL: $(SL_NAME) $(STATIC_LIB)

ini_lex.lex.o: ini_parse.tab.h

ini_read2.o: ini_lex.lex.o


$(SL_NAME): ${ALL_LO}
	$(LINK.c) $^ $(LOADLIBES) -shared -Wl,-soname,$@ -o $@

$(STATIC_LIB): ${OBJS}
	ar rcs $@ $^

clean:
	-${RM} *.o *.lo *.so* *.a *.lex.h *.tab.h

install: $(SL_NAME)
	-${MKDIR} -p $(INC)
	cp *.h $(INC)
	cp ../vexmo/*.h $(INC)
	cp ../wb/vexmo/*.h $(INC)
	cp ../lib/*.h $(INC)
	cp *.so* $(libdir)
	ln -sf $(SL_NAME) $(libdir)/$(SL).$(SL_MAJOR).$(SL_MINOR)
	ln -sf $(SL_NAME) $(libdir)/$(SL).$(SL_MAJOR)
	ln -sf $(SL_NAME) $(libdir)/$(SL)
