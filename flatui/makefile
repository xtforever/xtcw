dpix=144
dpiy=144
bg="\#ffffff"
VPATH=svg

BTN=btn-on.svg btn-off.svg btn-on-dis.svg btn-off-dis.svg

PIX=check-on.svg $(BTN)

T4=$(PIX:.svg=.xpm)

ALL: $(T4)

check-on.xpm: check-on.svg

%.png: %.svg
	rsvg-convert -d $(dpix) -p $(dpiy) -b $(bg) -f png -o $@ $<

%.pam: %.png
	pngtopnm -background $(bg) $<  | pnmcrop >$@

%.mask.pam: %.pam
	ppmcolormask $(bg) $< >$@

%.xpm: %.mask.pam %.pam
	ppmtoxpm -alphamask=$< $(@:.xpm=.pam) >$@

clean:
	$(RM) *.xpm
