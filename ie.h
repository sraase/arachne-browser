
// ========================================================================
// XSWAP: ib_editor strucuture, ie_getswap, ie_putswap, etc.
// (c)1995-99 Michael Polak, Arachne Labs (1995-1998: xChaos software)
// ========================================================================

//!!JdS 2004/2/8: Changed from 800 to 742, to reduce memory usage
#define IE_MAXLEN 742  //cookies will now be handled specially
//!!glennmcc: Dec 10, 2002 --- increased to 800 for really looooonnnnggg cookies ;-)
//#define IE_MAXLEN 800  //to handle longer cookies
//#define IE_MAXLEN 600  //to handle longer cookies
#define IE_MAXLINES fajl->maxlines//!!10000 //32000
#define IE_NULL 1023
#define X_NULL IE_NULL
#define IE_MAXSWAP 64   //this value is ultimately: 2^6(=64)*2^10(=1024)
#define IE_MAXLS 1020
#define IE_SWAPSTART (1024*sizeof(unsigned))

#define MAXARGBUF 4096  //!!JdS 2004/2/8
#define CookieCrumbs 6  //!!JdS 2004/3/2
//Limit cookie crumbs to 683 bytes (mustn't exceed IE_MAXLEN)  [JdS 2004/3/6]
#define CrumbSize (MAXARGBUF-1+CookieCrumbs)/CookieCrumbs

#ifdef POSIX
#define IE_MAXSWAPLEN 64000u
#define XSWAP unsigned short
#else //POSIX
#ifdef OVRL
#ifdef XTVERSION
#define IE_MAXSWAPLEN 28000u
#else //XTVERSION
//!!JdS 2004/2/8: Changed from 33000 to 30700, to reduce memory usage
#define IE_MAXSWAPLEN 30700u
//#define IE_MAXSWAPLEN 33000u
#endif //XTVERSION
#else //OVRL
//!!glennmcc: Dec 09, 2002 --- increased to 33000u for really looooonnnnggg cookies ;-)
//#define IE_MAXSWAPLEN 33000u
//!!JdS 2004/2/8: Original low-memory configuration restored (above change reversed)
#define IE_MAXSWAPLEN 28000u
#endif //OVRL
#define XSWAP unsigned
#endif //POSIX

#ifdef POSIX
extern char CLIPBOARDNAME[80];
#else
#define CLIPBOARDNAME "clip.tmp"
#endif


struct ib_editor
{
 char filename[80];
 char modified;
 int x; //0...n
 int y; //0...lines-1
 int lines; //pocet radek (tr.: number of lines)
 int zoomx; //0...od nultyho sloupce (tr.: from column no. 0)
 int zoomy; //0...od prvni radky (tr.: from first line)
 int bbx; //tyka se bloku (tr.: refers to block)
 int bby;
 int bex;
 int bey;
//old: XSWAP *lineadr; //[IE_MAXLINES+2];
//new...
 XSWAP linearray; //XSWAP adr of lineadr[IE_MAXLINES+2]
// int arraycache;
// XSWAP arraycacheadr;
 int swapcontext; //swapy, ktere neobsahuji spolene radky, se smazou hromadne
   // tr.: swaps which do not contain common lines are deleted alltogether

 //support variables
 char blockflag,insert,wordwrap;
 int aktrad;
 char rad[IE_MAXLEN+1];
 char modrad;
 int maxlines;
 char killcomment;
 int cols;
 char killstr[7];
};

//interface pro spravu celeho systemu
// tr.: interface for administration of the entire system 

int ie_initswap(void); // alokace celeho systemu swapobvani
   // tr.: allocation of the whole system of swapping
int ie_optimize(void);
//int ie_freeswap(void); // shutdown of the editor, deleting swaps
long ie_free(void);     // vraci volnou pamet... (tr.: returns free memory)
long ie_used(void);
void ie_destroy(void);
void ie_killcontext(int context);

//interface pro praci s otevrenym souborem
// tr.: interface for the work with an open file
//----------------------------------------
char *ie_getline(struct ib_editor *fajl, int i);
void ie_getcookie(char *cookie, struct ib_editor *jar, int start); //!!JdS 2004/3/6
int ie_putline(struct ib_editor *fajl, int i, char *str);
int ie_putcookie(struct ib_editor *jar, int start, char *cookie); //!!JdS 2004/3/6
int ie_insline(struct ib_editor *fajl, int i, char *str);
int ie_inscookie(struct ib_editor *jar, int start, char *cookie); //!!JdS 2004/3/6
int ie_delline(struct ib_editor *fajl, int i);
int ie_delcookie(struct ib_editor *jar, int posn); //!!JdS 2004/3/6

//pokud se neco stane spatne, pak return value _neni_ 1, pripadne pointer
//ukazuje na NULL. To si musi hlidat aplikace.
// tr.: if something goes wrong, return the value _notexist_ 1,
//   it may happen that the pointer points to NULL.
//   This has to be checked by the application. 

//interface pro otvirani/ zavirani souboru
// tr.: interface to open/close files
//----------------------------------------
//vymazani (tr.: deleting)
void ie_clearf(struct ib_editor *fajl,char all);
void ie_closef(struct ib_editor *fajl);
//load/save files, do fajl.filename se musi dat filename, kolikaty je 1/2
// tr.: load/save files, into variable fajl.filename must be entered
//      filename, which one in seuqence it is 1/2
int ie_openf(struct ib_editor *fajl,int context);
int ie_openf_lim(struct ib_editor *fajl,int context,int max);
int ie_savef(struct ib_editor *fajl);
int ie_insblock(struct ib_editor *fajl,char *filename);

//funkce pro analyzu vyznamu stisknute klavesy
// tr.: function to analyse the meaning of the pressed key
int ie_key(struct ib_editor *fajl,int klavesa,int modifiers,
           int ietxt_max_x,int ietxt_max_y);
//vraci 0:nic se nedeje, (tr.: returns 0: nothing happens,)
//      1:nastav kurzor (tr.: 1:set cursor)
//      2:prekresli radku (tr.: 2:overwrite line)
//      3:prekresli vsechno (tr.: 3:overwrite all)
//      4:roluj o radku nahoru (tr.: 4:scroll one line up)
//      5:roluj o radku dolu (tr.: 5:scroll one line down)
//      6:prekresli cele okno i s ramem (tr.: 6:overwrite entire window incl. frame)
//     27:stisknuto Esc (tr.: 27:pressed Esc)
//101-110:stisknuto F1-F10 (tr.: pressed F1-F10)


//fce vyrizne slovo na pozici kurzoru
// tr.: cuts word out at cursor position 
int ie_cutword(struct ib_editor *fajl,char *slovo);

void ie_bak(char* filename);//rename to .bak
int ie_which(int i);

void ie_appendclip(char *ptr);
void ie_xblockbeginend(struct ib_editor *fajl);

//private IE functions:
//write structure to swap
XSWAP ie_putswap(char *line, unsigned l, int context);
char *ie_getswap(XSWAP adr);
int ie_delswap(XSWAP adr, int l);
XSWAP getXSWAPlineadr(struct ib_editor *fajl, int i);

extern int swapmod;

extern unsigned *swapidx; //uznavam i=0-MAXLS; 1023=error flag, empty line
  // tr.: I accept i=0-MAXLS; 1023=error flag, empty line
extern char *swapstr; //od adresy swapstr[IE_SWAPSTART] zacina vlastni odkladani radku
  // tr.: from address swapstr[IE_SWAPSTART] begins the direct/actual
  //      deferring/depositing of lines/of the line
extern int swaplen[IE_MAXSWAP]; //length of all swaps in lines
extern unsigned swapsize[IE_MAXSWAP]; //length of all swaps in Bytes
extern int swapcontext[IE_MAXSWAP]; //swap context

extern char ie_clipstatus;

