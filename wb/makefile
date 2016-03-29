include ../rules.mk

ALL_WIDGETS:=${notdir $(wildcard *.widget)}
WIDGET_C:=${ALL_WIDGETS:%.widget=%.c}

ALL: clean all-widgets register-widgets

register-widgets: all-widgets
	./register-widgets.sh $(OUTPUT)/include/$(WBPREFIX) $(OUTPUT)/src

all-widgets: $(WIDGET_C)

clean:
	-$(RM) -rf $(OUTPUT)/doc $(OUTPUT)/src $(OUTPUT)/include/$(WBPREFIX)
