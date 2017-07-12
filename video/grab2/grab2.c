-*- mode: compilation; default-directory: "~/S/xtcw/video/" -*-
Compilation started at Wed Mar 15 12:30:18

make -k 
wb_search=../wb ../wbuild/wb.sh Wcap.widget ../wbuild/wbuild -d doc -c src -i xtcw --include-prefix=xtcw ../wbuild/tex.w ../wbuild/nroff.w
cc -DMLS_DEBUG -D_GNU_SOURCE -g -Wall -I/usr/include/freetype2 -I../lib -I. -I/home/jens/S/xtcw/video/../lib -I/home/jens/S/xtcw/video/.. -I/home/jens/S/xtcw/video/../wcl -I../build/include   video.c src/Wcap.c /home/jens/S/xtcw/video/../wcl/libxtcw.a xt_cairo.o xv4l2.o -lXaw -lX11 -lXt -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm  -lcairo -lv4l2  -o video

Compilation finished at Wed Mar 15 12:30:19
