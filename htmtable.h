
// Header file for HTML table rendering structures & functions.
// Used in file "htmtable.c"

#define MAXTD 60
#define MAXROWSPANTD 16
#define MAXTABLEDEPTH 16
#define SHORTSTR 15

struct HTMLtable
{
 int columns;
 int realwidth;
 int maxwidth;
 int realtdwidth[MAXTD];
 char rowspan[MAXTD];
 XSWAP closerowspan[MAXROWSPANTD];
 long xsum[MAXTD];
 long varxsum[MAXTD];
 int tracewidth[MAXTD];
 char shrink[MAXTD]; //bits SHRINK_LEFT_... | SHRINK_RIGHT_... | ....
 char percent[MAXTD];
 int cellspacing;
 int cellpadding;
 long tdend;
 long tdstart;
 long maxtdend;
 long nexttdend;
 char fixedmax;  //PERCENTS_FIXED_TABLE || PIXELS_FIXED_TABLE
 int x,y,xspan;
 char tablebg[SHORTSTR+1],rowbg[SHORTSTR+1];
 long totalxsum;
 char valignrow;
 char depth;
 XSWAP nextHTMLtable;
};

#define PERCENTS_FIXED_TABLE 1
#define PIXELS_FIXED_TABLE   2
                                  //in multicolum cells:
#define SHRINK_RIGHT_FORBIDDEN 1  //|        |   ..   |    ->|
#define SHRINK_LEFT_FORBIDDEN  2  //|<-      |   ..   |      |
#define SHRINK_RIGHT_ALLOWED   4  //|      ->|   .. ->|      |
#define SHRINK_LEFT_ALLOWED    8  //|        |<- ..   |<-    |
#define PIXEL_RESTRICTED_TD    16

void inittable(struct HTMLtable *tab);
char processcell(struct HTMLtable *tab,long xsum,int maxx,long y,int *cellx);
void newcell(struct HTMLtable *tab,int xspan,int yspan,int *tdx,long *tdy,int *width, char perc,int tdwidth);
char calcwidth(struct HTMLtable *tab);
void expand(struct HTMLtable *tab);
void fixrowspan(struct HTMLtable *tab,int closetable, XSWAP *closeptrs);
void fixrowspan_y(XSWAP *closeptrs,long y);
void closeatom_y(XSWAP adr,long absy);
int determine_new_x(struct HTMLtable *tab);
int determine_new_width(struct HTMLtable *tab,int xspan);

extern struct HTMLtable *thistable;

