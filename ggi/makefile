# Makefile for Arachne on Linux

CC=gcc
STRIP=strip

CFLAGS  = -m486 -O2 -fomit-frame-pointer -g -Wall
CFLAGS += -D LINUX -D POSIX -D OVRL -D NOKEY -D CLEMTEST -D HICOLOR -D GGI
OFILES  = linmain.o linglue.o iekernel.o ie_fce.o ie_key.o str.o
OFILES += ie_bin.o fexists.o config.o version.o errors.o outs.o html.o 
OFILES += htmlutil.o htmtable.o htmlread.o htmlstat.o atoms.o scrolbar.o
OFILES += htmldraw.o svgastat.o svgaovrl.o guidraw.o hdrawgif.o hdrawbmp.o 
OFILES += urlovrl.o urlstat.o mime.o misc.o inettime.o v_putimg.o html256c.o
OFILES += entity.o search.o htmlcsim.o putikonw.o gui.o guitick.o protocol.o
OFILES += guieven2.o highligh.o init.o globals.o guievent.o cgiquery.o print.o
OFILES += drawtime.o prtscr.o onmouse.o prespalp.o ipalpal.o tagsort.o main.o
OFILES += bitpix8.o x_char.o x_text.o x_font.o x_txtwh.o x_charmax.o x_cursor.o
OFILES += asockets.o http.o htkernel.o base64.o inetstr.o ps.o anyimage.o

LIBS    = -lggi -lpthread

%.o: ../%.c
	$(CC) $(CFLAGS) -c -o $*.o $<

arachne:	$(OFILES)
	$(CC) -o arachne-ggi $(OFILES) $(LIBS)
#	$(STRIP) arachne

clean:
	@rm -f *.o arachne-ggi
