ifeq ($(debug_flag),0)
	CFLAGS += -O -Wall
else
	CFLAGS += -DMLS_DEBUG -D_GNU_SOURCE -g -Wall
endif

prefix      ?= $(HOME)
exec_prefix ?= $(prefix)
bindir      ?= $(exec_prefix)/bin
libdir      ?= $(exec_prefix)/lib
includedir  ?= $(prefix)/include
libexecdir  ?= $(exec_prefix)/libexec

YFLAGS  = -d
YACC    = bison
LEX     = flex
MV      = mv
CP      = cp
MKDIR   = mkdir

#
# WBUILD CONFIGURATION
#
OUTPUT=../build
wb_install=../wbuild
WBUILD=../wbuild/wbuild
WBPREFIX=xtcw
WFLAGS= -d $(OUTPUT)/doc -c $(OUTPUT)/src -i $(OUTPUT)/include/$(WBPREFIX) --include-prefix=$(WBPREFIX) $(wb_install)/tex.w $(wb_install)/nroff.w

XORGLIB=-lXaw -lX11 -lXt -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm
LOADLIBES += $(XORGLIB)
CPPFLAGS  += -I/usr/include/freetype2 -I../lib -I.

%.tab.c %.tab.h: %.y
	$(YACC) $(YFLAGS) $<

%.lex.c %.lex.h: %.l
	$(LEX) -o$*.lex.c --header-file=$*.lex.h $<

%.lo: %.c
	$(CC) -c -fPIC $(CFLAGS) $(CPPFLAGS) $< -o $@

%.c %.h: %.widget
	wb_search=../wb $(wb_install)/wb.sh $^ $(WBUILD) $(WFLAGS)
