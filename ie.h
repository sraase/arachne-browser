
// ========================================================================
// XSWAP: ib_editor strucuture, ie_getswap, ie_putswap, etc.
// (c)1995-99 Michael Polak, Arachne Labs (1995-1998: xChaos software)
// ========================================================================

#define IE_MAXLEN 600  //to handle longer cookies
#define IE_MAXLINES fajl->maxlines//!!10000 //32000
#define IE_NULL 1023
#define X_NULL IE_NULL
#define IE_MAXSWAP 64   //this value is ultimate: 2^6(=64)*2^10(=1024)
#define IE_MAXLS 1020
#define IE_SWAPSTART (1024*sizeof(unsigned))

#ifdef POSIX
#define IE_MAXSWAPLEN 64000u
#define XSWAP unsigned short
#else
#ifdef OVRL
#ifdef XTVERSION
#define IE_MAXSWAPLEN 28000u
#else
#define IE_MAXSWAPLEN 33000u
#endif
#else
#define IE_MAXSWAPLEN 28000u
#endif
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
 int lines; //pocet radek
 int zoomx; //0...od nultyho sloupce
 int zoomy; //0...od prvni radky
 int bbx; //tyka se bloku
 int bby;
 int bex;
 int bey;
//old: XSWAP *lineadr; //[IE_MAXLINES+2];
//new...
 XSWAP linearray; //XSWAP adr of lineadr[IE_MAXLINES+2]
// int arraycache;
// XSWAP arraycacheadr;
 int swapcontext; //swapy, ktere neobsahuji spolene radky, se smazou hromadne

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

int ie_initswap(void); // alokace celeho systemu swapobvani
int ie_optimize(void);
//int ie_freeswap(void); // shutdown editoru, smazani swapu
long ie_free(void);     // vraci volnou pamet...
long ie_used(void);
void ie_destroy(void);
void ie_killcontext(int context);

//interface pro praci s otevrenym souborem
//----------------------------------------
char *ie_getline(struct ib_editor *fajl, int i);
int ie_putline(struct ib_editor *fajl, int i, char *str);
int ie_insline(struct ib_editor *fajl, int i, char *str);
int ie_delline(struct ib_editor *fajl, int i);

//pokud se neco stane spatne, pak return value _neni_ 1, pripadne pointer
//ukazuje na NULL. To si musi hlidat aplikace.


//interface pro otvirani/ zavirani souboru
//----------------------------------------
//vymazani
void ie_clearf(struct ib_editor *fajl,char all);
void ie_closef(struct ib_editor *fajl);
//load/save souboru, do fajl.filename se musi dat filename, kolikaty je 1/2
int ie_openf(struct ib_editor *fajl,int context);
int ie_openf_lim(struct ib_editor *fajl,int context,int max);
int ie_savef(struct ib_editor *fajl);
int ie_insblock(struct ib_editor *fajl,char *filename);

//funkce pro analyzu vyznamu stisknute klavesy
int ie_key(struct ib_editor *fajl,int klavesa,int modifiers,
           int ietxt_max_x,int ietxt_max_y);
//vraci 0:nic se nedeje,
//      1:nastav kurzor
//      2:prekresli radku
//      3:prekresli vsechno
//      4:roluj o radku nahoru
//      5:roluj o radku dolu
//      6:prekresli cele okno i s ramem
//     27:stisknuto Esc
//101-110:stisknuto F1-F10


//fce vyrizne slovo na pozici kurzoru
int ie_cutword(struct ib_editor *fajl,char *slovo);

void ie_bak(char* filename);//prejmenuje na bak
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
extern char *swapstr; //od adresy swapstr[IE_SWAPSTART] zacina vlastni odkladani radku
extern int swaplen[IE_MAXSWAP]; //delka vsech swapu v radkach
extern unsigned swapsize[IE_MAXSWAP]; //delka vsech swapu v bytech
extern int swapcontext[IE_MAXSWAP]; //swap context

extern char ie_clipstatus;

