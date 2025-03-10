# Makefile for Arachne on Linux (SDL2)

CC=gcc
STRIP=strip

CFLAGS  = -O2 -fomit-frame-pointer -Wall -std=gnu99
CFLAGS += -D LINUX -D POSIX -D OVRL -D HICOLOR -D SDL2
OFILES  = linglue.o iekernel.o ie_fce.o ie_key.o str.o
OFILES += ie_bin.o fexists.o config.o version.o errors.o outs.o html.o
OFILES += htmlutil.o htmtable.o htmlread.o htmlstat.o atoms.o scrolbar.o
OFILES += htmldraw.o svgastat.o svgaovrl.o guidraw.o hdrawgif.o hdrawbmp.o
OFILES += urlovrl.o urlstat.o mime.o misc.o inettime.o v_putimg.o html256c.o
OFILES += entity.o search.o htmlcsim.o putikonw.o gui.o guitick.o protocol.o
OFILES += guieven2.o highligh.o init.o globals.o guievent.o cgiquery.o print.o
OFILES += drawtime.o prtscr.o onmouse.o prespalp.o ipalpal.o tagsort.o main.o
OFILES += bitpix8.o x_char.o x_text.o x_font.o x_txtwh.o x_charma.o x_cursor.o
OFILES += http.o htkernel.o base64.o inetstr.o ps.o anyimage.o
OFILES += wheelmou.o parsecss.o atcpip.o

LIBS    = -lSDL2 -lpthread

%.o: ../%.c
	$(CC) $(CFLAGS) -c -o $*.o $<

arachne:	$(OFILES)
	$(CC) -o arachne-sdl2 $(OFILES) $(LIBS)
# $(STRIP) arachne-sdl2


clean:
	@rm -f *.o arachne-sdl2
