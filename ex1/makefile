include ../rules.mk
M=../
CPPFLAGS+=-I$(M)/lib -I$(M)/vexmo -I$(M)/wcl -DMLS_DEBUG -I$(M)/wb/vexmo
VPATH=$(M)/lib
CFLAGS+=-g -Wall 

all: exbasic1

../wcl/libxtcw.a:
	make -C ../

exbasic1: ../wcl/libxtcw.a

clean:
	${RM} *.o exbasic1
