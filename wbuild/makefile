# makefile wbuild
# include ../../rules.mk
# ACHTUNG:
# das verzeichnis $(prefix)/libexec/ muss vor dem kompilieren
# erstellt werden.
#
# PKGDATADIR:=$(shell readlink -f $(libexecdir)/wbuild)
# PKGDATADIR:=$(shell readlink -f $(prefix)/libexec/wbuild)
# PKGDATADIR:=$(shell readlink -f ./)
PKGDATADIR ?= $(CURDIR)

YFLAGS  = -d
YACC    = bison
LEX     = flex

OBJS=declarefunc.o  generatedoc.o parse.tab.c scan.o util.o generatec.o generateh.o printbody.o  symbol.o
CPPFLAGS+=-DPKGDATADIR=\"$(PKGDATADIR)\" -DYYDEBUG -I.

%.tab.c %.tab.h: %.y
	$(YACC) $(YFLAGS) $<

%.lex.c %.lex.h: %.l
	$(LEX) -o$*.lex.c --header-file=$*.lex.h $<


ALL: wbuild
extra: clean ALL

scan.o: parse.tab.h

wbuild: $(OBJS)

install: wbuild
	-${MKDIR} -p ${PKGDATADIR}
	-${MKDIR} -p ${bindir}
	-${CP} *.w *.tex ${PKGDATADIR}
	-${CP} wbuild ${bindir}

clean:
	${RM} wbuild $(OBJS) parse.c parse.tab.c parse.tab.h scan.c *~
