
// =========================================================================
// Arachne WWW browser - Linux glue code, SVGAlib graphics libraries
// By Michael Polak (Arachne Labs) and Emannuel Marty (Suntech)
// =========================================================================

#include "arachne.h"
#include "pckbrd.h"

#ifdef SVGALIB
#include <vga.h>
#include <vgamouse.h>
#include <vgagl.h>
#include <termios.h>
#include <pthread.h>
#endif

#ifdef SDL2
#include <SDL2/SDL.h>

static SDL_Window   *window   = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Surface  *surface  = NULL;

static unsigned short sdlcolor = 0xFFFF;
static int need_update = 0;
static int key_queue[16];
static int key_queue_len;
static int key_state = 0;
static int mouse_range[4];
static int mouse_state[3];
#endif

#define FOREVER 3600 //60min * 60sec == one hour == close enough to forever..

//#define TXTDEBUG

/* Glue functions for Linux */

int filelength (int handle) {
   off_t curpos;
   int length;
   curpos = lseek (handle, 0, SEEK_CUR);
   lseek (handle, 0, SEEK_END);
   length = lseek (handle, 0, SEEK_CUR);
   lseek (handle, curpos, SEEK_SET);

   return length;
}

int SVGAx=799,SVGAy=599;

// =========================================================================
// bioskey() simulation using stdin, for SVGAlib version.
// =========================================================================

// Interface functions: bioskey_init(), bioskey_close(), bioskey().

#ifdef SVGALIB
//In SVGAlib, there is no need for screen flushing, but we need rather
//sophisticated waitformouse and waitfor keyboard threads....

struct termios old_termios;

//int WaitForKey_pipeline[2];
//int CancelWaitForKey_pipeline[2];
int WaitForMouse_pipeline[2];
volatile int MouseWasUpdatedInThread=0;

#endif

#ifdef SDL2
static int utf8_decode(char byte, unsigned short *out, int *state)
{
    switch (*state) {
    case 0:
        if ((byte & 0x80) == 0x00) {
            *state = 0; *out = byte; return 0;
        } else if ((byte & 0xF8) == 0xF0) {
            *state = 3; *out = (byte & 0x07);
        } else if ((byte & 0xF0) == 0xE0) {
            *state = 2; *out = (byte & 0x0F);
        } else if ((byte & 0xC0) == 0xC0) {
            *state = 1; *out = (byte & 0x1F);
        } else {
            *state = 0; *out = 0xFFFD; return 0;
        }
        break;
    case 1:
    case 2:
        if ((byte & 0xC0) == 0x80) {
            *out = (*out << 6) | (byte & 0x3F);
            if (!--*state) return 0;
        } else {
            *state = 0; *out = 0xFFFD; return 0;
        }
        break;
    case 3:
        if ((byte & 0xC0) != 0x80) {
            *state = 0; *out = 0xFFFD; return 0;
        }
        break;
    }
    return 1;
}

static unsigned short get_color(int index)
{
    unsigned int r = (xg_hipal[3 * index + 0] >> 1) & 0x1F;
    unsigned int g = (xg_hipal[3 * index + 1]     ) & 0x3F;
    unsigned int b = (xg_hipal[3 * index + 2] >> 1) & 0x1F;
    return (r << 11 | g << 5 | b);
}

static int get_button(Uint8 button)
{
    switch (button) {
    case SDL_BUTTON_LEFT:   return 0x01;
    case SDL_BUTTON_RIGHT:  return 0x02;
    case SDL_BUTTON_MIDDLE: return 0x04;
    }
    return 0;
}

static int get_button_mask(Uint32 state)
{
    int ret = 0;
    if (state & SDL_BUTTON_LMASK) ret |= 0x01;
    if (state & SDL_BUTTON_RMASK) ret |= 0x02;
    if (state & SDL_BUTTON_MMASK) ret |= 0x04;
    return ret;
}

static int get_key(SDL_Keycode code)
{
    if (key_state & (0x04 | 0x08)) { /* LCTRL | RCTRL */
        switch (code) {
        case SDLK_HOME:  return CTRLHOME;
        case SDLK_END:   return CTRLEND;
        case SDLK_LEFT:  return CTRLLEFT;
        case SDLK_RIGHT: return CTRLRIGHT;
        case SDLK_c:     return ASCIICTRLC;
        case SDLK_d:     return ASCIICTRLD;
        case SDLK_k:     return ASCIICTRLK;
        case SDLK_l:     return ASCIICTRLL;
        case SDLK_m:     return ASCIICTRLM;
        case SDLK_n:     return ASCIICTRLN;
        case SDLK_o:     return ASCIICTRLO;
        case SDLK_p:     return ASCIICTRLP;
        case SDLK_q:     return ASCIICTRLQ;
        case SDLK_r:     return ASCIICTRLR;
        case SDLK_s:     return ASCIICTRLS;
        case SDLK_t:     return ASCIICTRLT;
        case SDLK_u:     return ASCIICTRLU;
        case SDLK_v:     return ASCIICTRLV;
        case SDLK_w:     return ASCIICTRLW;
        case SDLK_x:     return ASCIICTRLX;
        case SDLK_y:     return ASCIICTRLY;
        }
    }

    switch (code) {
    case SDLK_BACKSPACE:    return BACKSPACE;
    case SDLK_UP:           return UPARROW;
    case SDLK_DOWN:         return DOWNARROW;
    case SDLK_LEFT:         return LEFTARROW;
    case SDLK_RIGHT:        return RIGHTARROW;
    case SDLK_INSERT:       return INSERT;
    case SDLK_DELETE:       return DELETEKEY;
    case SDLK_HOME:         return HOMEKEY;
    case SDLK_END:          return ENDKEY;
    case SDLK_PAGEUP:       return PAGEUP;
    case SDLK_PAGEDOWN:     return PAGEDOWN;
    case SDLK_PRINTSCREEN:  return PRTSCR;
    case SDLK_F1:           return F1;
    case SDLK_F2:           return F2;
    case SDLK_F3:           return F3;
    case SDLK_F4:           return F4;
    case SDLK_F5:           return F5;
    case SDLK_F6:           return F6;
    case SDLK_F7:           return F7;
    case SDLK_F8:           return F8;
    case SDLK_F9:           return F9;
    case SDLK_F10:          return F10;
    case SDLK_F11:          return F11;
    case SDLK_F12:          return F12;
    default: if (code < 32) return code;
    }

   return 0;
}

static int get_modifiers(SDL_Keycode code)
{
    switch (code) {
    case SDLK_LSHIFT: return 0x01;
    case SDLK_RSHIFT: return 0x02;
    case SDLK_LCTRL:  return 0x04;
    case SDLK_RCTRL:  return 0x08;
    case SDLK_LALT:   return 0x10;
    case SDLK_RALT:   return 0x20;
    }
    return 0;
}

static void add_key(int key)
{
    if (key && (key_queue_len < sizeof(key_queue) / sizeof(key_queue[0]))) {
        key_queue[key_queue_len++] = key;
    }
}

static void add_text(const char *text)
{
    int state = 0;
    unsigned short codepoint = 0;
    while (*text) {
        if (utf8_decode(*text++, &codepoint, &state))
            continue;

        // TODO: handle characters outside ISO-8859-1
        add_key(codepoint < 0x0100 ? codepoint : '?');
    }
}

static void do_events(int timeout)
{
    SDL_Event ev;

    // avoid interface lag when screen update is needed
    // but event queue is empty
    if (need_update && !SDL_PollEvent(NULL))
        goto doupdate;

    SDL_WaitEventTimeout(NULL, timeout);
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            GLOBAL.abort = ABORT_PROGRAM;
            break;

        case SDL_KEYDOWN:
            key_state |= get_modifiers(ev.key.keysym.sym);
            add_key(get_key(ev.key.keysym.sym));
            break;

	case SDL_KEYUP:
            key_state &= ~get_modifiers(ev.key.keysym.sym);
            break;

        case SDL_TEXTINPUT:
            add_text(ev.text.text);
            break;

        case SDL_MOUSEBUTTONDOWN:
            mouse_state[0]  = ev.button.x;
            mouse_state[1]  = ev.button.y;
            mouse_state[2] |= get_button(ev.button.button);
            break;

        case SDL_MOUSEBUTTONUP:
            mouse_state[0]  = ev.button.x;
            mouse_state[1]  = ev.button.y;
            mouse_state[2] &= ~get_button(ev.button.button);
            break;

        case SDL_MOUSEMOTION:
            mouse_state[0]  = ev.button.x;
            mouse_state[1]  = ev.button.y;
            mouse_state[2]  = get_button_mask(ev.motion.state);
            break;

        case SDL_WINDOWEVENT:
            need_update = 1;
            break;
        }
    }

doupdate:
    if (need_update) {
        SDL_RenderClear(renderer);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture)
            SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(texture);
        need_update = 0;
    }
}
#endif


void bioskey_init(void)
{
#ifdef SVGALIB
struct termios new_termios;

/* put the tty into "straight through" mode. */
if (tcgetattr(STDIN_FILENO, &old_termios) < 0)
{
 perror("tcgetattr failed");
}

memcpy(&new_termios,&old_termios,sizeof(struct termios));

new_termios.c_lflag &= ~(ICANON | ECHO	| ISIG);
new_termios.c_iflag &= ~(ISTRIP | IGNCR | ICRNL | INLCR | IXOFF | IXON);
new_termios.c_cc[VMIN]	= 0;
new_termios.c_cc[VTIME] = 0;

if (tcsetattr(STDIN_FILENO, TCSANOW, &new_termios) < 0)
 perror("tcsetattr failed");
else
 printf("Console switched to raw input mode.\n");

//errr, this shouldn't be really located here, but it is SVGAlib/waitfor
//key related, so let's keep it here for now...

/*
 stupid implementation ;)
 if(pipe(WaitForKey_pipeline)<0)
 perror("pipe failed.\n");

 if(pipe(CancelWaitForKey_pipeline)<0)
 perror("pipe failed.\n");
*/

if(pipe(WaitForMouse_pipeline)<0)
 perror("pipe failed.\n");

#endif
#ifdef SDL2
    memset(key_queue, 0, sizeof(key_queue));
    key_queue_len = 0;
    SDL_StartTextInput();
#endif
}

int buffered=0,modifiers=0;

int bioskey(int cmd)
{
#ifdef SVGALIB
 fd_set readset;
 struct timeval t={0,0};
 unsigned char	buf[6];

 if(buffered!=0)
 {
  if(cmd==1)
   return 1;
  else
  {
   int ret=buffered;
   buffered=0;
   return ret;
  }
 }

 FD_ZERO(&readset);
 FD_SET(0, &readset);

 if (select(STDIN_FILENO+1, &readset, NULL, NULL, &t) <= 0)
 {
  return 0;
 }

 if(cmd==1)
  return 1;

 read(STDIN_FILENO, buf, 1);

 if ( buf[0] != 27 /* escape */)
 {
  return (int) buf[0];
 }

/* Wait a bit, to see if the character following escape was '['
*  which signals an ANSI key.
*/

 if (select(STDIN_FILENO+1, &readset, NULL, NULL, &t) <= 0)
 {
  usleep(10000l);  /* wait 1/100th of a second */
 }

if (select(STDIN_FILENO+1, &readset, NULL, NULL, &t) <= 0)
{
/* Timed out : must have been plain escape key */
 return (int)buf[0];
}

 read(STDIN_FILENO, buf+1, 1);

 if (buf[1] != '[')
 {
  /* Nope, not an ANSI key sequence */
  buffered=(int)buf[1];
  return (int)buf[0];
 }

/* handle the ANSI key sequences */

 read(STDIN_FILENO, buf+2, 1);

 buf[3] = buf[4] = buf[5] = 0;

 if (isdigit(buf[2]) || (buf[2] == '['))
 {
  read(STDIN_FILENO, buf+3, 1);
 }

 if (isdigit(buf[3]))
 {
  read(STDIN_FILENO, buf+4, 1);
 }

#define CMP_KEY(S,K) if (strcmp(buf+2, (S)) == 0) {return (int)(K);}

	CMP_KEY("A", UPARROW);	  CMP_KEY("B", DOWNARROW);
	CMP_KEY("C", RIGHTARROW); CMP_KEY("D", LEFTARROW);

	CMP_KEY("1~", HOMEKEY);   CMP_KEY("4~", ENDKEY);
	CMP_KEY("2~", INSERT); CMP_KEY("3~", DELETEKEY);
	CMP_KEY("5~", PAGEUP); CMP_KEY("6~", PAGEDOWN);

	CMP_KEY("[A",  F1); CMP_KEY("[B",  F2);
	CMP_KEY("[C",  F3); CMP_KEY("[D",  F4);
	CMP_KEY("[E",  F5); CMP_KEY("17~", F6);
	CMP_KEY("18~", F7); CMP_KEY("19~", F8);
	CMP_KEY("20~", F9); CMP_KEY("21~", F10);
	CMP_KEY("23~", F11); CMP_KEY("24~", F12);

#undef CMP_KEY

 printf("Unrecognized ANSI sequence: %s\n",buf+2);
 return 0;
#endif
#ifdef SDL2
    // handle pending events to update keyboard queue
    do_events(0);
    switch (cmd) {
    case 0:
        // return next key (consumeing)
        if (key_queue[0]) {
            int key = key_queue[0];
            memmove(&key_queue[0], &key_queue[1],
                sizeof(key_queue) - sizeof(int));
            key_queue_len--;
            return key;
        }
        return 0;
    case 1:
        // return next key (non-consuming)
        return key_queue[0];
    case 2:
        // return modifier keys
        {
            int ret = 0;
            if (key_state & 0x01) ret |= LEFTSHIFT;
            if (key_state & 0x02) ret |= RIGHTSHIFT;
            if (key_state & 0x04) ret |= CTRLKEY;
            if (key_state & 0x08) ret |= CTRLKEY;
            if (key_state & 0x10) ret |= ALT;
            if (key_state & 0x20) ret |= ALT;
            return ret;
        }
    }
    return 0;
#endif
}


void bioskey_close(void)
{
#ifdef SVGALIB
 if (tcsetattr(STDIN_FILENO, TCSANOW, &old_termios) < 0)
  perror("tcsetattr failed");
 else
  printf("Console switched back to original mode.\n");
#endif
#ifdef SDL2
    SDL_StopTextInput();
#endif
}


// misc glue functions

int kbhit(void)
{
 //check for keystroke - only to allow Ctrl+C in DOS ....
 return 0;
}

void tempinit(char *path)
{
	// put temporary files in userpath (defaults to ~/.arachne/)
	strcpy(path, userpath);
}

//---------------------------------------------------------------------------
//X_LOPIF bindings:
//---------------------------------------------------------------------------

// this variable indicates graphics mode, is used to select between
// HiColor vs. 256 color modes
int xg_256;
int xg_video_XMS=0; //real screen
unsigned short xg_hival[256]; //hicolor values for palette
int xg_hi16=1;	  // mode : 16bit=1,15bit=0
int xg_hipalmod=0;// x_setcolor(), x_setfill():0=index, 1=direct hicolor
int xg_chrmod=1;
unsigned char xg_hipal[768];  // Pal for HiCol mode (set through x_setpal())

int xg_color;		      /* Set colour (rect,text)      */
int xg_fillc;		      /* Set colour for x_bar()      */
int xg_wrt;		      /* 0=overwrite, 1=XOR	     */
int xg_style;		      /* Definition of line	     */
int xg_xr,xg_yr;	      /* ratio			     */
int xg_view[4]; 	      /* Viewport		     */
int xg_xfnt,xg_yfnt;	      /* Font size in pixels	     */
int xg_tjustx;		      /* Aligning text in X direction*/
int xg_tjusty;		      /* Aligning text in Y direction*/
int xg_clip;		      /* Whether to cut in viewport  */
int xg_fbyt;		      /* Number of bytes in one char of the font */
int xg_flag;		      /* Types: bit 0 - spec EGA pal */
			      /*	bit 1 - aktive patt  */
unsigned char xg_fonlen[256];  // Widths of chars for proportional fonts
int32_t       xg_fonadr[256];  // Beginning of chars for proportional fonts
unsigned char xg_foncon;       // Flag - const/prop font [0/average]
unsigned char xg_fonmem;       // Where is the font: 0-MEM(xg_fbuf),1-XMS,2-DISK
int	      xg_fonhan;       // Handle for XMS/DISK
unsigned int  xg_lbfnt;        // length of buffer with font data

char *xg_fbuf;		      /* font bufffer		      */
int xg_fnt_zoo=1;      /* For zooming text 1 | 2 */
char xg_fnt_akt[64];  // Name of active font

int  xg_fnt_max;       // max number of fonts in XMS
int  xg_fnt_fre;       // first free/available in table
int  xg_fnt_xms;       // handle XMS
long xg_fnt_xlen;      // total length in XMS
long xg_fnt_xoff;      // free space in XMS
struct FNTXTAB *xg_fnt_xtab;   // font table
int   xg_31yn=0;      //ASCII 1 ... ASCII 31 interpreted as 1..31 pixel space

//---------------------------------------------------------------------------

int x_grf_mod(int xmod)
{
 //set text mode here...
 //we will emulate hicolor for now....
 xg_256=MM_Hic;

#ifdef SDL2
 if (xmod == 3) {
  // mode 03h = 80x25 text, called before exit
  SDL_FreeSurface(surface);      surface  = NULL;
  SDL_DestroyRenderer(renderer); renderer = NULL;
  SDL_DestroyWindow(window);     window   = NULL;
  SDL_Quit();
 }
#endif
 return 1;
}


int x_maxx(void)
{
 return SVGAx;
}

int x_maxy(void)
{
 return SVGAy;
}

void x_video_XMS(int vidXMS, int bincol)
{
 //toggle graphics output to real or virtual screen.
}

void x_setcolor(int color)
{
#ifndef TXTDEBUG
 //set foreground color
 xg_color=color;
#ifdef SVGALIB
 vga_setrgbcolor(xg_hipal[3*xg_color]<<2,xg_hipal[3*xg_color+1]<<2,xg_hipal[3*xg_color+2]<<2);
#endif
#ifdef SDL2
    sdlcolor = get_color(xg_color);
#endif
#endif
}

void x_line(int x1, int y1, int x2, int y2 )
{
// draw line
#ifndef TXTDEBUG
#ifdef SVGALIB
 vga_drawline(x1,y1,x2,y2);
#endif
#ifdef SDL2
    // Bresenham line drawing algorithm
    unsigned short *dst = surface->pixels;
    unsigned int pitch = surface->pitch;
    int dx = (x2 > x1) ? x2 - x1 : x1 - x2;
    int sx = x1 < x2 ? 1 : -1;
    int dy = (y2 > y1) ? y1 - y2 : y2 - y1;
    int sy = y1 < y2 ? 1 : -1;
    int e  = dx + dy;
    while (1) {
        if (x1 <= SVGAx && y1 <= SVGAy)
            dst[y1 * pitch/2 + x1] = sdlcolor;
        if (x1 == x2 && y1 == y2)
            break;
        if (dy < 2*e) {
            if (x1 == x2)
                break;
            e  += dy;
            x1 += sx;
        }
        if (dx >= 2*e) {
            if (y1 == y2)
                break;
            e  += dx;
            y1 += sy;
        }
    }
    need_update = 1;
#endif
#endif
}

void x_bar(int xz, int yz, int xk, int yk)
{
#ifndef TXTDEBUG
#ifdef SVGALIB
 unsigned char *palptr = xg_hipal + 3*xg_fillc;
 int r, g, b;

 r = ((int) (*palptr++)) << 2;
 g = ((int) (*palptr++)) << 2;
 b = ((int) (*palptr)) << 2;

 //printf ("box (%d): %d %d %d\n", xg_fillc, r, g, b);
 // rectangle filled with background color
 gl_fillbox(xz,yz,xk-xz+1,yk-yz+1,
   gl_rgbcolor(r,g,b));
#endif
#ifdef SDL2
    unsigned short *dst = surface->pixels;
    unsigned int pitch = surface->pitch;
    unsigned short c = get_color(xg_fillc);
    for (int y = yz; y < yk; y++) {
        for (int x = xz; x < xk; x++) {
            if (x <= SVGAx && y <= SVGAy) {
                dst[y * pitch/2 + x] = c;
            }
        }
    }
    need_update = 1;
#endif
#endif
}

void x_rect(int xz, int yz, int xk, int yk)
{
 // draw empty rectangle
#ifndef TXTDEBUG
#ifdef SVGALIB
 vga_drawline(xz,yz,xk,yz);
 vga_drawline(xk,yz,xk,yk);
 vga_drawline(xz,yk,xk,yk);
 vga_drawline(xz,yz,xz,yk);
#endif
#ifdef SDL2
    x_line(xz, yz, xk, yz);
    x_line(xk, yz, xk, yk);
    x_line(xz, yk, xk, yk);
    x_line(xz, yz, xz, yk);
#endif
#endif
}

void x_setfill(int vypln, int color)
{
 // set fill color - first arg is dummy
 xg_fillc=color;
}

void x_settextjusty(int horiz, int vert)
{
  xg_tjustx = horiz;
  xg_tjusty = vert;
}

void x_charmod(int chrmod)
{
 //transparent or solid text blocks...
 xg_chrmod=chrmod;
}

//get image (real screen only)
void x_getimg(int x1, int y1, int x2, int y2, char *bitmap)
{
 short int w=(short int)(x2-x1)+1;
 short int h=(short int)(y2-y1)+1;
 memcpy(bitmap,&w,sizeof(short int));
 memcpy(&bitmap[sizeof(short int)],&h,sizeof(short int));

// if(x1>=0 && x2<=x_maxx() && y1>=0 && y2<=x_maxy())
#ifdef SVGALIB
 gl_getbox(x1, y1, w, h,&bitmap[2*sizeof(short int)]);
#endif
#ifdef SDL2
    unsigned short *src = surface->pixels;
    unsigned int pitch = surface->pitch;
    unsigned short *dst = (unsigned short *)&bitmap[2 * sizeof(short int)];
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            dst[y * w + x] = src[(y + y1) * pitch/2 + (x + x1)];
        }
    }
#endif
}

//put image (real screen only)
void x_putimg(int xz,int yz, char *bitmap, int op)
{
 int w;
 int h;

 w = *(short int *)bitmap;
 h = *(short int *)&bitmap[sizeof(short int)];
// if(xz>=0 && xz+w<=x_maxx() && yz>=0 && yz+h<=x_maxy())
#ifdef SVGALIB
 gl_putbox(xz,yz,w,h,&bitmap[2*sizeof(short int)]);
#endif
#ifdef SDL2
    unsigned short *src = (unsigned short *)&bitmap[2 * sizeof(short int)];
    unsigned short *dst = surface->pixels;
    unsigned int pitch = surface->pitch;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if ((x + xz) <= SVGAx && (y + yz) <= SVGAy) {
                dst[(y + yz) * pitch/2 + (x + xz)] = src[y * w + x];
            }
        }
    }
    need_update = 1;
#endif
}

int x_getmaxcol(void)
{
 //return max. number of colors
 return 256;
}

void x_palett(int len, char *paleta)
{
 //set palette to array or (3 R-G-B bytes)*len
  if(xg_256 == MM_Hic)		 // only a copy into xg_hipal
  {
   int i;

     memcpy(xg_hipal, paleta, 3*len);
     for(i=0; i<len; i++)
       { xg_hival[i] = xh_RgbHiPal(xg_hipal[3*i], xg_hipal[3*i+1],
				   xg_hipal[3*i+2]);
       }
  }
  //else real palette mapping.... later....

}

/*
void x_yncurs(int on, int x, int y, int col)
{
 //enable/disable cursor at certain position, using certain color
}
*/

void x_cleardev(void)
{
#ifdef SVGALIB
 vga_clear();
#endif
#ifdef SDL2
    if (surface) {
        memset(surface->pixels, 0, surface->pitch * (SVGAy + 1));
        need_update = 1;
    }
#endif
}

int x_detect(char *svga, int *kby)
{
 //detect graphics mode... will be probably unused with SVGAlibg
 return 0;
}

void x_pal_1(int n_pal, char *pal_1)
{
 //set one color in palette
 memcpy(&xg_hipal[3*n_pal],pal_1,3);
 xg_hival[n_pal] = xh_RgbHiPal(xg_hipal[3*n_pal], xg_hipal[3*n_pal+1], xg_hipal[3*n_pal+2]);
}

int x_rea_svga(char *path, char *g_jmeno, int *mod)
{
 //convert mode name to mode number
 //probably won't be used
 return 0;
}

void z_bitbyte(unsigned char *buf1, unsigned char *buf2, int delka)
{
 //converts bit planes to bytes
}

// Prevod bufru s bytovym obrazem na Hi-color
// Pozn: Predpoklada se, ze byla volana fce x_palett() s paletou obrazu.
// tr.: Convert buffer with bitmap to Hi-color
//	Note: provides that fce x_palett() has been called with palette of picture
void xh_ByteToHi(unsigned char *Ibuf, unsigned char *Hi,
		int Pixs, int Rows, int LenLine)
{
// Ibuf- (in) buffer with rows of palette picture (1B/pixel)
// Hi  - (out)buffer with rows HiCol
// Pixs- pixels in a line
// Rows- number of rows (-Rows => Rows in Hi in reverse order)
// LenLine - length Ibuf of row/rows in Bytes
   int	 i,j, Rows2;
   unsigned char *Ibufx;
   unsigned short  *Hix;

   Hix = (unsigned short *)Hi;
   Rows2 = abs(Rows);

   for(i=0; i<Rows2; i++)
   {
     if(Rows > 0)
      Ibufx = Ibuf + (LenLine * i);
     else
      Ibufx = Ibuf + (LenLine * (Rows2-i-1));

     for(j=0; j<Pixs; j++)
     {
	(*Hix) = xg_hival[(*Ibufx)];
	Ibufx++;
	Hix++;
     }

   }
}

// Conversion RGB (0..63) to Hi-color
unsigned short xh_RgbHiPal(unsigned char R, unsigned char G, unsigned char B)
{
   if(xg_hi16 == 1)
    return(RGBHI16(R,G,B));
   else
    return(RGBHI15(R,G,B));
}

//---------------------------------------------------------------------------
// virtual screens
//---------------------------------------------------------------------------

int xv_set_actvirt(int Index)
{
 //set active virtual screen
 return 1;
};

int  xv_to_scr(int xs, int ys, int xo, int yo, int dx, int dy)
{
 //dump virtual screen to real screen
 return 1;
}

int  xv_cls_virt(int Xwrt,	  // 0-only free XMS, 1-write XMS on disk
		int Index)	  // number of virtual videoram
{
 //clear virt. screen
 return 0;
}

int xv_new_virt(char *filenam, // File name for disk file
	      int dx, int dy,  // Size in pixels
	      int col,	       // Default color
	      int bitpix,      // 3-1bit/pixel, 0-8bit/pixel, -1-16bit/pixel
	      int npal,        // Length of palette
	      char *pal,       // Palette, range RGB 0..63, max. 256 entries
	      int Index,       // Index 0..5, number of virtual videoram
	      int Typ)	      // 0-XMS or DISK, 1-XMS, 2-DISK
{
 return 1;
}

int  xv_int_wrt(int xs, int ys, char *buf)
{
 //putimage to virt. screen
 return 1;
}

int  xv_int_rea(int xs, int ys, int dx, int dy, char *buf)
{
 //getimage from virt. screen
 return 1;
}


//---------------------------------------------------------------------------
// GIF animations - hanimgif.c, needs to rework memory management....
//---------------------------------------------------------------------------

int  g_NumAnim=0;	// number of animated GIFs

int  XInitAnimGIF(int XmsKby)
{
 return 0;
 //call once at start
}

void XCloseAnimGIF(void)
{
 //call once at end
}

int  XResetAnimGif(void)
{
 return 0;
 // when redrawing page
}

int  XAnimateGifs(void)
{
 return 0;
 //called from loop to animage GIFs
}

void XSetAllAnim1(void)
{
 //rewind animations
}

void XSetAnim1(void)
{
 //rewind animation (?)
}

//---------------------------------------------------------------------------
//mouse interface
//---------------------------------------------------------------------------

int xg_mousebutton=0;

 //initialize mouse
int ImouseIni( int xmin, int ymin, int xmax, int ymax,
		int xstart, int ystart)
{
#ifdef SVGALIB
 mouse_init("/dev/mouse",vga_getmousetype(),10);

 {
  int mscale = config_get_int("SVGAlib_MouseScale", 60);
  mouse_setscale(mscale);
 }

 mouse_setxrange(xmin,xmax);
 mouse_setyrange(ymin,ymax);
 mouse_setposition(xstart,ystart);
 return 1;
#endif
#ifdef SDL2
    // set mouse range and position
    mouse_range[0] = xmin;
    mouse_range[1] = ymin;
    mouse_range[2] = xmax;
    mouse_range[3] = ymax;
    mouse_state[0] = xstart;
    mouse_state[1] = ystart;
    mouse_state[2] = 0;

    // hide system cursor, we draw our own
    SDL_ShowCursor(SDL_DISABLE);
    return 1;
#endif
}


//read mouse coordinates and return mouse buttons status 0,1,2,4...


int ImouseRead( int *xcurs, int *ycurs)
{
#ifdef SVGALIB
 if(MouseWasUpdatedInThread || mouse_update())
 {
  MouseWasUpdatedInThread=0;
  xg_mousebutton=mouse_getbutton();
  *xcurs=mouse_getx();
  *ycurs=mouse_gety();
//  if(button!=0)
//   printf("[mouse=%d]",button);

  switch(xg_mousebutton) //?? SVGAlib mouse events ....
  {
   case 4:
   xg_mousebutton=1;
   break;
   case 1:
   xg_mousebutton=2;
   break;
   case 2:
   xg_mousebutton=4; //?
   break;
  }
 }
 return xg_mousebutton;
#endif
#ifdef SDL2
    // handle all pending events to update mouse state
    do_events(0);
    *xcurs = min(mouse_range[2], max(mouse_range[0], mouse_state[0]));
    *ycurs = min(mouse_range[3], max(mouse_range[1], mouse_state[1]));
    return mouse_state[2];
#endif
}

//set mouse coordinates
void ImouseSet( int xstart, int ystart)
{
#ifdef SVGALIB
 mouse_setposition(xstart,ystart);
#endif
}

//wait for user to release mouse key
void ImouseWait(void)
{
#ifdef SVGALIB
 int dummy;
 while (ImouseRead(&dummy,&dummy))
  usleep(10000); //don't let it eat all CPU time...
#endif
#ifdef SDL2
    // handle all events until mouse buttons released
    while (mouse_state[2])
        do_events(-1);
#endif
}

#ifdef SVGALIB


void *WaitForMouse_thread(void *arg)
{
 mouse_waitforupdate();
 MouseWasUpdatedInThread=1;
 write(WaitForMouse_pipeline[1],"!",1);
 return NULL;
}

/* This worked, but it was stupid!!!

void WaitForKey_thread(void)
{
 pthread_t mousethread;
 int term_thread=0,waitpipes;
 fd_set readset;
 struct timeval forever={FOREVER,0};
 struct timeval now={0,0};
 char dummy[2];

 FD_ZERO(&readset);
 FD_SET(STDIN_FILENO, &readset);
 FD_SET(WaitForMouse_pipeline[0], &readset);

 if(select(max(WaitForMouse_pipeline[0],STDIN_FILENO)+1, &readset, NULL, NULL, &now)<=0)
 {
  if(pthread_create(&mousethread,NULL,WaitForMouse_thread,NULL)!=0)
  {
   printf("Can't create mouse thread!\n");
   return;
  }
  term_thread=1;

  FD_ZERO(&readset);
  FD_SET(STDIN_FILENO, &readset);
  FD_SET(WaitForMouse_pipeline[0], &readset);
  FD_SET(CancelWaitForKey_pipeline[0], &readset);

  waitpipes=max(WaitForMouse_pipeline[0],CancelWaitForKey_pipeline[0]);
  select(max(waitpipes,STDIN_FILENO)+1, &readset, NULL, NULL, &forever);
 }

 if(FD_ISSET(WaitForMouse_pipeline[0],&readset))
  read(WaitForMouse_pipeline[0],&dummy,1);

 if(FD_ISSET(CancelWaitForKey_pipeline[0],&readset))
  read(CancelWaitForKey_pipeline[0],&dummy,1);
 else
  write(WaitForKey_pipeline[1],"!",1);

 if(term_thread)
  pthread_cancel(mousethread);
}
*/




#endif

/*
//---------------------------------------------------------------------------
//This is the main CPU saving function in Arachne.
//It is not used at all in DOS.
//---------------------------------------------------------------------------
*/

void WaitForEvent(struct timeval *tv) //waits for user input or whatever...
{
#ifdef SVGALIB
 struct timeval forever={FOREVER,0};  //one hour is like forever, for most CPUs
 if(!tv)
  tv=&forever;
/* old version of event polling:
 {
  struct timeval now={0,0};
  pthread_t keythread;
  int term_thread=0;
  fd_set readset;
  char dummy[2];

  FD_ZERO(&readset);
  FD_SET(WaitForKey_pipeline[0], &readset);
  if(select(WaitForKey_pipeline[0]+1, &readset, NULL, NULL, &now)<=0)
  {
   if(pthread_create(&keythread,NULL,WaitForKey_thread,NULL)!=0)
   {
    printf("Can't create keyboard thread!\n");
    return;
   }
   term_thread=1;

   FD_ZERO(&readset);
   FD_SET(WaitForKey_pipeline[0], &readset);
   select(WaitForKey_pipeline[0]+1, &readset, NULL, NULL, tv);
  }

  if(FD_ISSET(WaitForKey_pipeline[0],&readset))
  {
   read(WaitForKey_pipeline[0],&dummy,1);
  }
  else if(term_thread)
   write(CancelWaitForKey_pipeline[1],"!",1);
 }
*/
 {
  struct timeval now={0,0};
  pthread_t mousethread;
  int term_thread=0;
  fd_set readset;
  char dummy[2];

  FD_ZERO(&readset);
  FD_SET(WaitForMouse_pipeline[0], &readset);
  FD_SET(STDIN_FILENO, &readset);
  if(select(max(STDIN_FILENO,WaitForMouse_pipeline[0])+1, &readset, NULL, NULL, &now)<=0)
  {
   if(pthread_create(&mousethread,NULL,WaitForMouse_thread,NULL)!=0)
   {
    printf("Can't create mouse thread!\n");
    return;
   }
   term_thread=1;

   FD_ZERO(&readset);
   FD_SET(WaitForMouse_pipeline[0], &readset);
   FD_SET(STDIN_FILENO, &readset);
   select(max(STDIN_FILENO,WaitForMouse_pipeline[0])+1, &readset, NULL, NULL, tv);
  }

  if(FD_ISSET(WaitForMouse_pipeline[0],&readset))
   read(WaitForMouse_pipeline[0],&dummy,1);
  else if(term_thread)
   pthread_cancel(mousethread);
 }
#endif
#ifdef SDL2
    // wait for events and handle them
    int timeout = -1;
    if (tv)
        timeout = tv->tv_sec * 1000 + tv->tv_usec / 1000;
    do_events(timeout);
#endif
}

/*
//---------------------------------------------------------------------------
// graphicsinit() is graphics initialization function.
// argument will be string describing resolution
// graphics mode is read by x_maxx(), x_maxy(), x_getmaxcoll(). Arachne also
// accesses "private" variable xg_256 because public API for detecting HiColor
// modes is missing..
//---------------------------------------------------------------------------
*/

void graphicsinit(char *svgamode) //initialization of graphics, mode according to X_LOPIF
{
const unsigned short cur[32] =
	 { 0x9FFF, 0x0FFF, 0x07FF, 0x83FF, 0xC1FF, 0xE0FF, 0xF067, 0xF003,
	   0xF001, 0xF000, 0xF800, 0xF800, 0xF800, 0xFC00, 0xFC00, 0xFC00,
	   0x0000, 0x6000, 0x7000, 0x3800, 0x1C00, 0x0E00, 0x0700, 0x0018,
	   0x07EC, 0x07EE, 0x001E, 0x03EE, 0x03EE, 0x001E, 0x00EC, 0x0002 };


  xg_256=MM_Hic; //set Hicolor flag...
  initpalette();
  x_settextjusty(0,2);	// always write text from upper left corner

#ifdef SVGALIB
 strupr(svgamode);
// printf("Console switched to graphics mode.\n");
 if(strstr(svgamode,".I")) {
  vga_setmode(G640x480x64K);
  gl_setcontextvga(G640x480x64K);
  SVGAx=639;
  SVGAy=479;
 } else if(strstr(svgamode,".K")) {
  vga_setmode(G1024x768x64K);
  gl_setcontextvga(G1024x768x64K);
  SVGAx=1023;
  SVGAy=767;
 } else if(strstr(svgamode,".L")) {
  vga_setmode(G1280x1024x64K);
  gl_setcontextvga(G1280x1024x64K);
  SVGAx=1279;
  SVGAy=1023;
 } else if(strstr(svgamode,".M")) {
  vga_setmode(G1600x1200x64K);
  gl_setcontextvga(G1600x1200x64K);
  SVGAx=1599;
  SVGAy=1199;
 } else {
  vga_setmode(G800x600x64K);
  gl_setcontextvga(G800x600x64K);
  SVGAx=799;
  SVGAy=599;
 }
 vga_runinbackground(1);
 vga_oktowrite();
 gl_setwritemode(FONT_COMPRESSED|WRITEMODE_MASKED);
 gl_setfontcolors(0,vga_white());
 //gl_setfont(8,8,gl_font8x8);
 gl_setrgbpalette();
 gl_enableclipping();
#endif
#ifdef SDL2
    char *ptr;

    // parse mode string (e.g. "800x600") to window size
    if (svgamode && (ptr = strchr(svgamode, 'x'))) {
        SVGAx = atoi(svgamode) - 1;
        SVGAy = atoi(ptr + 1)  - 1;
    }
    if (SVGAx < 319 || SVGAy < 199) {
        // invalid or too small
        SVGAx = 799;
        SVGAy = 599;
    }

    // initialize SDL2 and open window
    SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    window = SDL_CreateWindow("Arachne",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SVGAx + 1, SVGAy + 1, 0);
    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_PRESENTVSYNC);
    surface = SDL_CreateRGBSurfaceWithFormat(0,
        SVGAx + 1, SVGAy + 1, 16, SDL_PIXELFORMAT_RGB565);
    if (!window || !renderer || !surface) {
        fprintf(stderr, "Failed to initialize SDL2.\n");
        exit(EXIT_TO_DOS);
    }
#endif
 x_defcurs( cur, &cur[16], 15); //mouse kursor
}

