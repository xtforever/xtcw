all: tool widgets wclib 

tool:
	make  -C wbuild PKGDATADIR=${CURDIR}/wbuild

widgets: tool
	make  -C wb

wclib: widgets
	make -C wcl

clean:
	make  -C wbuild clean
	make  -C wb clean
	make  -C wcl clean
