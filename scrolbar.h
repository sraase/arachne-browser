
// ========================================================================
// Arachne scrollbars
// (c)1996-1999 Michael Polak, Arachne Labs
// ========================================================================

struct ScrollBar
{
 int max_xscrsz,max_yscrsz,xscr,yscr,xscrsz,yscrsz;
 int xsize,ysize,ymax,xtop,ytop;
 int total_x;
 long total_y;
 char xvisible;
 char yvisible;
 int x_decrease_gap; //for scrollbuttons
 int x_increase_gap;
 int y_decrease_gap;
 int y_increase_gap;
 int gap;
 char onscrollx,onscrolly;
 char scrollbarstyle; // \0...no buttons,A..Arachne,X..Aracne,W..Windows
};

void ScrollInit(struct ScrollBar *scroll,int draw_x,int draw_y,
      int max_ysize,int xtop, int ytop, int total_x, long total_y);
void ScrollButtons(struct ScrollBar *scroll);
void ScrollDraw(struct ScrollBar *scroll,int fromx,long fromy);
void ScrollInit(struct ScrollBar *scroll,int draw_x,int draw_y,
      int max_ysize,int xtop, int ytop, int total_x, long total_y);

int OnScrollButtons(struct ScrollBar *scroll);
int OnBlackZone(struct ScrollBar *scroll);
int ScrollBarTICK(struct ScrollBar *scroll,int *X, long *Y);

extern int lx,ly,lmouse;

