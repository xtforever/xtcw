include ../rules.mk

M=$(CURDIR)/..
LOADLIBES+=-lrt -lcairo
CPPFLAGS+=-I$(M)/lib -I$(M) -I$(M)/wcl -I$(M)/wb
VPATH=$(M)/lib

all: ex10

ex10: src/Woptc.o $(M)/wcl/libxtcw.a

src/Woptc.o: Woptc.widget

clean:
	${RM} -rf ex10 *.o src doc vexmo *~
