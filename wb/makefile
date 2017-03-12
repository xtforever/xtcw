# Makefile to generated widgets via wbuild
# register-widgets.sh copies widget headers from ../widgets/
# therefore i need to apply touch after the widget is copied.
# This makefile needs: WBPREFIX,OUTPUT,WBUILD,WFLAGS

include ../rules.mk

BINC=$(OUTPUT)/include/$(WBPREFIX)
BSRC=$(OUTPUT)/src
BDOC=$(OUTPUT)/doc

ALL_WIDGETS:=${notdir $(wildcard *.widget)}
WIDGET_H:=${ALL_WIDGETS:%.widget=$(BINC)/%.h}

ALL: all-widgets $(BINC)/register_wb.h

$(BINC)/%.h: %.widget
	wb_search=../wb $(wb_install)/wb.sh $^ $(WBUILD) $(WFLAGS)

$(BINC)/register_wb.h: $(WIDGET_H)
	./register-widgets.sh $(BINC) $(BSRC)

all-widgets: $(WIDGET_H)

clean:
	-$(RM) -rf $(BDOC) $(BSRC) $(BINC) *~
