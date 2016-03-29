all: tool widgets wclib examples

tool:
	make  -C wbuild PKGDATADIR=${CURDIR}/wbuild

widgets: tool
	make  -C wb

wclib: widgets
	make -C wcl

examples: wclib
	make -C ex11

clean:
	make  -C wbuild clean
	make  -C wb clean
	make  -C wcl clean
	make  -C ex4 clean
	make -C ex11 clean
