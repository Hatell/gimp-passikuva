.PHONY: all

all: config-2x2.h config-2x3.h config-digi.h digipassikuva.c passikuva-2x2.c passikuva-2x3.c passikuva.c passikuva-digi.c passikuva.h
	gimptool-2.0 --install passikuva-digi.c
	gimptool-2.0 --install passikuva-2x2.c
	gimptool-2.0 --install passikuva-2x3.c
