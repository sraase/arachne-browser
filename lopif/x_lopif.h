#ifdef CALDERA
#include "link.h"
#endif // CALDERA

/*==============================================================*/
/*   Graficke funkce pro EGA,VGA, a ruzne SVGA                  */
/*==============================================================*/

/* Return codes from function : 1 = O.K.  else = Error          */

// Podmineny preklad pro Hi color mody (1-ano, 0-ne)

#ifdef HICOLOR
#define HI_COLOR  1
#else
#define HI_COLOR  0
#endif

// Xlopif s virtual videoram
#define VIRT_SCR
// Max. pocet virt. obrazovek
#define MAXVIRTUAL 15

// Konstanty pro 2,16,256,HiColor (pouze bity 5..7!!)
#define MM_2      0x20
#define MM_16     0x40
#define MM_256    0x80
#define MM_Hic    0x60

/*##-------------------- x_grf_mod -------- Bios----------------*/
/* Nastavi graficky mod zadany cislem xmod. Zajimave mody :     */
/* Jmena: EGA - 640 x 350 x 16                                  */
/*        VGA - 640 x 480 x 16                                  */
/*        VGA2- 320 x 200 x 256                                 */
/*        A   - 800 x 600 x 16                                  */
/*        B   - 640 x 480 x 256                                 */
/*        C   - 600 x 800 x 256                                 */
/*        D   -1024 x 768 x 16                                  */
/*        E   -1024 x 768 x 256                                 */
/*        BCGA- 640 x 200 x 2                                   */

/*        03 - implicitni textovy mod na EGA/VGA                */

/* Hi-Col Hi16: 16Bit (65536 colors)                            */
/*        Hi15: 15Bit (32768 colors)                            */
/*        I   - 640 x 480 x 15|16                               */
/*        J   - 800 x 600 x 15|16                               */
/*        K   -1024 x 768 x 15|16                               */
/*        L   -1280 x 1024 x 15|16                              */
/*        M   -1600 x 1200 x 15|16                              */

// Priklad:     Hi16.j = 65536 colors x 800 x 600
// Popis fci spec. pro Hi-col pod:  #if HI_COLOR

/*##-------------------- x_grf_mod  ----------------------------*/
int x_grf_mod(int xmod);
/* xmod   - skutecne cislo modu  dle graf. karty                */

/*##-------------------- x_setnomode ---------------------------*/
// pri nomode != 0 doopravdy nenastavi grf. mod
void x_setnomode(int nomode);

/*##-------------------- x_SetXmsEms ---------------------------*/
int SetXmsEms(int Mode);
// Mode - 1-XMS, 2-EMS
// ret    1-OK,  0-ERROR

/*##-------------------- alloc_xmem --------------------------*/
int alloc_xmem(int n); // Alokuje n KB pameti, vraci handle

/*##-------------------- dealloc_xmem ------------------------*/
int dealloc_xmem(int h);

/*##-------------------- mem_all_xmem ------------------------*/
long mem_all_xmem(void);
// vrati ceklove naalokovanych KB XMS pomoci alloc_xmem()

/*##-------------------- hinf_xmem ---------------------------*/
//  K handlu XMS zjisti kolik KB alokuje a volny pocet handlu
int hinf_xmem(int hxms, int *Kb, int *FreeH);
// hxms - handle xms   (in)
//   kb - kolik KB patri k tomuto handlu (out)
// FreeH- zbyvajici pocet handlu XMS

/*##-------------------- x_rea_svga ----------------------------*/
//  Zjisti dle jmena konkretni graf. mod
int x_rea_svga(char *path, char *g_jmeno, int *mod);

/*path   - nema zadny vyznam                                    */
/*g_jmeno- jmeno karty+modu :                                   */
/*         Jmena standartnich graf. modu jsou EGA,VGA,VGA2.     */
/*         Jmena SVGA graf. modu se tvori takto:                */
/*         JmenoSVGA.Jmeno_modu  Napr: TSG3.B , OAK.C atd.      */
/*         viz. tabulka jmen u x_grf_mod()                      */
/*mod    - skutecne cislo graf. modu                            */

/*       return: 1 - O.K.                                       */
/*               2 - soubor neexistuje (nelze otevrit)          */
/*               4 - neznama grf. karta, chybna syntaxe         */
/*               6 - mod u zadane karty neni podporovan         */


/*##-------------------- x_getgrfmode ------ Bios --------------*/
/* Vrati cislo nastaveneho grafickeho modu.                     */

int x_getgrfmode(void);


/*##------------------- x_map_pal------------ Bios --------------*/
/* Namapuje 16 registru z 256 pro 16 barevne VGA mody            */
/* Vola se pouze jednou pred pouzivanim x_palett                 */
/* Po inicializaci nejsou nastaveny zadne registry !!!           */
/* !! Nevolat pro mod EGA.                                       */

void x_map_pal(char *mapreg);

/* mapreg - pole s pal. registry, ktere budeme dale pouzivat     */


/*##------------------- x_palett ---------- Bios ---------------*/
/* Nastavi paletu barev pro dany mod. 16 nebo 256 barev         */
/* Po inicializaci je nastavena implicitni paleta               */
/* Pozn: pro EGA mod je paleta nejprve prevedena do EGA barev   */
/*       metodou zaokrouhlovani. Viz fce x_set_spc_ega().       */

void x_palett(int len, char *paleta);

/* len    - delka palety (16,256)                                */
/* paleta - pole s paletou, ktere obsahuje R,G,B slozky pro      */
/*          kazdou barvu. Delka pole = len * 3 bytu              */


/*##-------------------- x_pal_1 ---------- Bios ---------------*/
/* Nastavi paletu pro 1 barvu v 16/256 barevnych modech         */
/* Pozn: pro EGA mod je paleta nejprve prevedena do EGA barevy  */
/*       metodou zaokrouhlovani. Viz fce x_set_spc_ega().       */

void x_pal_1(int n_pal, char *pal_1);

/* n_pal  - ktera barva (index)                                 */
/* pal_1  - pole s paletou, ktere obsahuje R,G,B slozky         */


/*##-------------------- x_getpalette --------------------------*/
/* Vrati nastavenou paletu (delka 16,256) v poli, ktere obsahuje*/
/* pro kazdou polozku palety 3 byty (RGB)                       */
/* Delka palety je dana nastavenym graf. modem.                 */
/* Pozn: V EGA modu jsou cisla barev prevedeny do RGB slozek    */
/*       tak ze bity 0..3 jsou vzdy 0, bity 4,5 jsou nastaveny. */

void x_getpalette(char *pal3);

/* pal3 - pole s paletou                                        */

/*##-------------------- x_set_spc_ega -------------------------*/
/* Ma vyznam jen pro EGA mod. Nastavi zpusob ziskani EGA barvy  */
/* z RGB slozek. Pouzit pred x_palett,x_pal_1.                  */

void x_set_spc_ega(int flag);

/* flag = 0 - ega barva se vybere zaokrouhlovanim z RGB slozek  */
/*            implicitni hodnota                                */
/* flag = 1 - ve VGA palete je umistena spec. EGA paleta        */
/*            ve 2 nejvyssich bitech slozek RGB                 */


/*##-------------------- x_setcolor ---- Bios-------------------*/
/* Nastavi barvu (index) pro dalsi pouziti v kreslicich funkcich*/
/* Pozn: Pro x_bar() se nastavi barva pomoci x_setfill. Pro     */
/*       psani textu je tato barva barvou textu.                */
/* Po x_grf_ini() je nastavena barva 7.                         */

void x_setcolor(int color);

/* color  - ktera barva (index)                                 */


/*##-------------------- x_getmaxcol ---------------------------*/
/* Vrati maximalni cislo barvy (1,15,255).                      */

int x_getmaxcol(void);


/*##-------------------- x_setfill -----------------------------*/
/* Nastavi barvu (index) pro dalsi pouziti ve funkci x_bar a    */
/* pro pozadi fce x_textxy() a x_text_ib()                      */
/* Po x_grf_ini() je nastavena barva 0.                         */

void x_setfill(int vypln, int color);

/* vypln  - zpusob vyplneni obdelnika (ma vyznam jen pro        */
/* standartni mody, pro ostatni se vyplni barvou)               */
/* color  - ktera barva (index)                                 */

/*##-------------------- x_setpattern --------------------------*/
/* Nastavi vzorek a barvu (index) pro dalsi pouziti ve funkci   */
/* x_bar(). Zavolanim fce x_setfill se kresleni uzivatelskym    */
/* vrorem zrusi.                                                */

void x_setpattern(char *patt, int color);

/* patt - pole 8 bytu dlouhe ve kterem je definovan vzor 8x8    */
/* color - barva pro kresleni vzorem  na pozadi barvou 0        */

/*##-------------------- x_setcircf ----------------------------*/
/* Nastavi zda kruznice, ci elipsy (x_circ,x_elipse) se budou   */
/* kreslit nevyplnene nebo vyplnene. Vyplnene se kresli barvou  */
/* nastavenou pres x_setfill. Vyplnovat lze plnou barvou nebo   */
/* nastavenym vzorkem.                                          */

void x_setcircf(int flag);

/* flag - 0 = Nevyplnene kruznice(elipsy), 1 = Vyplnene kruznice */

/*##-------------------- x_getcol -------------------------------*/
/* Vrati cislo nastavene barvy                                   */
int x_getcol(void);

/*##-------------------- x_getfill ------------------------------*/
/* Vrati cislo nastavene barvy  vyplne                           */
int x_getfill(void);

/*##-------------------- x_setlinstyle --------------------------*/
/* Nastavi typ cary, definuje caru a nastavi tloustku cary       */
/* pro dalsi pouziti.                                            */
/* Pozn: Po x_grf_ini() je nastavena plna cara.                  */

void x_setlinstyle(int typ, int user, int width);

/* typ  - preddefinovany typ cary PLNA,CARKOVANA,atd             */
/* user - ve dvou bytech definice cary (16 pixlu)                */
/* width- tloustka cary (zatim jen tenka)                        */


/*##-------------------- x_setratio -----------------------------*/
/* Funkce nastavujici aspectratio, tj pomer velikosti pixlu ve   */
/* smeru X a Y. (Aby kruznice byla vzdy kruznice)                */

void  x_setratio( int xasp, int yasp );


/*##-------------------- x_getratio -----------------------------*/
/* Funkce vraci nastavene aspectratio, tj pomer velikosti        */
/* pixlu ve smeru X a Y. (Aby kruznice byla vzdy kruznice)       */

void  x_getratio( int *xasp, int *yasp );


/*##-------------------- x_setview ------------------------------*/
/* Funkce nastavujici viewport                                   */

void  x_setview(int x1, int y1, int x2, int y2, int clip);

/* x1,y1,x2,y2 - viewport                                        */
/* clip        - 0/1 vypnuti,zapnuti orezavani ve viewportu      */


/*##-------------------- x_cleardev ----- VGA--------------------*/
/* Vycisti obrazovku                                             */

void x_cleardev(void);


/*##-------------------- x_clearview ---- VGA--------------------*/
/* Vycisti viewport                                              */

void x_clearview(void);


/*##------------------- wrt_video ---------- VGA ----------------*/
/* Zapise do videoram bufer s obrazkem                           */

int wrt_video(char *buf, int xz,int yz, int ncol,int nrow, int pix);

/* buf  - pole obsahujici pixly obrazku, v jednom bytu muze byt i */
/*        vice pixlu (2,4,8)                                      */
/*        Pozn: max. velikost bufru je 64KB.                      */
/* xz   - zacatek obrazku vodorovne v pixlech                     */
/* yz   - zacatek obrazku svisle v pixlech                        */
/* ncol - pocet sloupcu obrazku                                   */
/* nrow - pocet radku obrazku                                     */
/* pix  - pocet pixlu v jednom bytu (1,2,4,8).Pro 256 barev pix=1 */


/*##------------------ x_line ------------------------------------*/
/* Nakresli usecku [x1,y1]-->>[x2,y2]                             */

void x_line(int x1, int y1, int x2, int y2 );


/*##------------------ x_poly ------------------------------------*/
/* Nakresli polygon o nump vrcholech. Souradnice polygonu jsou v  */
/* poli pole[]. Neuzavira se automaticky !                        */

void x_poly(int nump, int *pole );


/*##------------------ x_circle ----------------------------------*/
/* Nakresli kruznici o stredu [x1,y1] a polomeru r.               */

void x_circle(int x1, int y1, int r );


/*##------------------ x_ellipse ---------------------------------*/
/* Nakresli elipsu o stredu [x1,y1] a polomerech rx a ry.         */

void x_ellipse(int x1, int y1, int rx, int ry);


/*##------------------ x_arc -------------------------------------*/
/* Nakresli oblouk o stredu [x1,y1] uhlu od sa do ea o polomeru r */

void x_arc(int x1, int y1, int sa, int ea, int r );


/*##------------------ x_rect ------------------------------------*/
/* Nakresli nevyplneny obdelnik barvou z x_setcolor()             */
/* Pozn: pomoci fce x_wrtmode() lze nastavit psani s XOR          */

void x_rect(int xz, int yz, int xk, int yk);

/* xz   - zacatek obdelnika vodorovne                             */
/* yz   - zacatek obdelnika svisle                                */
/* xk   - konec vodorovne                                         */
/* yk   - konec svisle                                            */


/*##------------------ x_bar -------------------------------------*/
/* Nakresli vyplneny obdelnik  barvou z x_setfill()               */
/* Pozn: Pro standatr. mody pouziva Borland fce bar()             */
/*       a pro Super_VGA mody primo psani do VGA_reg.             */

void x_bar(int xz, int yz, int xk, int yk);

/* xz   - zacatek obdelnika vodorovne                             */
/* yz   - zacatek obdelnika svisle                                */
/* xk   - konec vodorovne                                         */
/* yk   - konec svisle                                            */

/*##------------------ x_fillpoly --------------------------------*/
/* Nakresli polygon vyplneny vzorem.                              */

void x_fillpoly(int nump, int *pole );


/*##------------------ x_floodfill --------------------------------*/
/* Vyplni oblast zadanou barvou v x_setfill().                     */

int  x_floodfill(int x, int y, int flag);

/* x,y  - startovni bod vyplnovani                                 */
/* flag - bit 0: 0->videoram,  1->interni bit. rovina xf_bitmap    */
/*        bit 1: 0->nelze dale pouzit x_flood_2, 1->lze pouzit     */

/* Vyplneni barvou ci vzorem po predchozim volani x_floodfill      */

int  x_flood_2(int dx, int dy);

/* dx, dy - posune vyplnenou oblast o dx,dy                        */

/* Uvolneni pameti : Ma vyznam pouze kdyz param. flag != 0         */

void x_flood_free(void);

/* Kazde nove volani x_floodfill() znovu samo uvolnuje a prideluje */
/* pamet. Ale x_flood_2() ne. Lze je tedy volat nekolikrat po      */
/* jednom x_floodfill().                                           */

/*##----------- x_putpix -----------------------------------------*/
/* Zapise pixel do videoram                                       */

void x_putpix(int x, int y, int color);


/*##------------- x_getpix ---------------------------------------*/
/* Precte pixel z videoram                                        */

int x_getpix(int x, int y);


/*##----------------- x_fnt_load ---------------------------------*/
/* Nahraje font pro psani v grafickem modu pro fce x_textxy()     */
/* a x_text_ib()                                                  */
/* Soubor s fontem je ve tvaru pro program FEDIT, ktery je        */
/* soucasti softwaru pro S_VGA kartu.                             */

int x_fnt_load(char *fnt_file, int num, int mod);

/* fnt_file - soubor s fontem                                     */
/* num      - pocet radku textu na stranku (pro mod 0)            */
/* mod      - 0 - nahrati do BIOSU, pak lze pouzit fci x_textxy() */
/*                Pro tento mod se x_fnt_load() nemusi volat.     */
/*            1 - nahrati jen do vlastniho bufru pro x_text_ib()  */
/*                Pozn: Bufer zabira cca 4-64kB                   */
/*            2 - oboji (0 a 1)                                   */
// Pozn:      Pro proporcni fonty (WINDOWS) se provede vzdy pouze
//            mod 1.  Nelze je vnutit BIOSU !!!

/*##----------------- x_fnt_cls ----------------------------------*/
/* Uvolni font z pameti (xg_fbuf), pripadne uvolni XMS, zavre     */
/* soubor s fontem. Neni-li zadny font v pameti, neprovede se nic */

int x_fnt_cls(void);

// return - 1   - O.K.
//          2,4 - nepovedla se uvolnit XMS, zavrit soubor s fontem


// Naalokuje buffer xg_fbuf pro praci s fontem. Tento buffer se
// bude pouzivat stale dokola pro ruzne fonty. Implicitne se vzdy
// pro kazdy font znovu alokuje novy bufer dle velikosti fontu.

int x_fnt_alloc(long Size);
// size - max. velikost fontu v Bytech. Urci se tak, ze vezmeme nejdelsi
// soubor s fontem a od nej odecteme 1280.
// Zadame-li Size=0, bude se alokovat pro kazdy font znovu pamet.
// Pozn: Volani zrusi jiz nacteny font.
// Pozn: Max. velikost Size je 64kb !!!

// ret : 1 - O.K.,  0-Error (neni pamet)

/*##----------------- x_fnt_initxms -----------------------------*/
// Inicializuje tabulky a XMS pro uchovavani fontu v XMS.
// Funkce x_fnt_load() pak umisti nacteny font do XMS, a pri dalsim volani
// x_fnt_load() se stejmym jmenem ho necte z disku ale z XMS. Dojde-li
// misto v tabulce fontu, nebo XMS pak se tabulka vyprazdni a zacne se
// znova plnit pri volani x_fnt_load().
// Pozn: max. delka jmena pro font je 31 znaku !!!
//       volat nekde pri init xlopifu po x_fnt_alloc() !!! (pouziva-li se)
// Jeden font umisteny v XMS zabira navic 38B v konvencni pameti, tabulka
// se alokuje v x_fnt_initxms(). XMS se alokuje take zde a ma velikost
// Maxfnt * SIZE_XMS_FNT bytu (prumerne 10kB na font viz #define SIZE...).
// Pro uvolneni XMS je nutno na konci programu voloat x_fnt_cls().

int  x_fnt_initxms(int Maxfnt);
// Maxfnt - max. pocet fontu soucasne v XMS. 0,-1 znamena zadne fonty
//          v XMS(jako drive).

/*##------------------ x_textxy -------------- Bios --------------*/
/* Napise text v grafickem modu. Tvar textu lze menit zmenou      */
/* fontu pres fci x_fnt_load. Pise pouze vodorovne. Zarovnani ve  */
/* smeru x se nastavi pres fci x_settextjusty(). Pise se barvou   */
/* nastavenou v x_setcolor() a na pozadi nastavene pres funkci    */
/* x_setffill().  Souradnice se zadavaji v pixlech a jsou pred    */
/* psanim zaokrouhleny na radky a sloupce textu dle velikosti     */
/* fontu. Pocet radku a sloupcu na obrazovce  zavisi na velikosti */
/* velikosti fontu. Typicky znak je velky 8x8,8x14,8x16 pixlu.    */
/* Pozn: Tato funkce vyuziva pro psani textu BIOS.                */

void x_textxy(int xp, int yp, char *text);

/* xp,yp - souradnice pocatku textu v pixlech                     */
/* text  - psany text                                             */


/*##------------------ x_text_ib ------------- VGA reg -----------*/
/* Napise text v grafickem modu. Tvar textu lze menit zmenou      */
/* fontu pres fci x_fnt_load. Pise pouze vodorovne. Zarovnani ve  */
/* smeru x,y se nastavi pres fci x_settextjusty(). Pise se barvou */
/* nastavenou v x_setcolor() a na pozadi nastavene pres funkci    */
/* x_setffill().  Souradnice se zadavaji v pixlech a text lze     */
/* na rozdil od fce x_textxy() umistit od libovolneho pixlu.      */
/* Pozn: Pro 16 bar. mody je x_text_ib() asi 2x pomalejsi nez     */
/*       x_textxy(). Pro 256 bar. mody je asi 2x rychlejsi.       */

int x_text_ib(int xp, int yp, unsigned char *text);

/* xp,yp - souradnice pocatku textu v pixlech                     */
/* text  - psany text                                             */
/* return  1 - text O.K., 2-nenacetl se znak z XMS|DISKu          */

/*##----------------- x_settextjusty -----------------------------*/
/* Nastavi zpusob zarovnani textu                                 */
/* Po inicializaci je nastaveno : left,top                        */

void x_settextjusty(int horiz, int vert);

/* horiz - 0=left, 1=center, 2=right                              */
/* vert  - 0=bott, 1=center, 2=top                                */


/*##----------------- x_txwidth,x_txheight -----------------------*/
/* Vrati delku a vysku textu v pixlech (dle akt. fontu)           */

int x_txwidth (char *string);
int x_txheight(char *string);

/* string - pro x_txheight() nema vyznam                          */

/*##----------------- x_charmax ----------------------------------*/
/*  Urci kolik znaku se vejde do zadane sirky v pixlech           */

int x_charmax(unsigned char *string, int dxpixlu);

//  string - string s texten
//  dxpixlu- sirka okna v pixlech
//  ret    - max. pocet znaku do dane sire (hlavne pro prop. fonty)
//           a je-li mensi pak delku stringu ve znacich

/*##----------------- x_charmod ----------------------------------*/
/*  Nastavi, zda se u textu bude kreslit i pozadi (pusvitny text) */

void x_charmod(int chrmod);

// chrmod - 0 - bude se kreslit pozadi (implicitni po startu)
//          1 - nebude se kreslit pozadi - prusvitny text

int x_getcharmod(void);   // vrati typ pozadi

/*##----------------- x_text_zoom --------------------------------*/
/* Nastavi zvetseni textu pro x_text_ib(), Pouze: 2x nebo 4x      */

void x_text_zoom(int zoom);

/* zoom - velikost textu (pouze 1,2,4)                             */

/*##---------------- x_getsize -----------------------------------*/
/* Zjisti velikost bufru pro uschovu obrazku                      */
/* Je buffer >= 64K vrati 0xFFFF, jinak potrebnou velikost        */

unsigned int x_getsize(int x1, int y1, int x2, int y2);

/* x1,y1,x2,y2 - okno v pixlech                                   */


/*##---------------- x_set_planes ---------------------------------*/
/* Nasatvi pocet bitovych rovin pro GETIMG,PUTIMG (16 barevne mody)*/
/* Implicitne 4 bit. roviny, rozsah 1..4 roviny                    */

void x_set_planes(int planes);

/* planes - pocet bitovych rovin (1..4) */

/*##---------------- x_getimg ---------------------- VGA_reg-------*/
/* Uschova cast obrazovky zadane oknem do bufru                    */
/* Pozn: Pro 16 bar.mody je obraz ve tvaru bit. rovin po radcich.  */
/*       Pro 256 bar. mody primo ctenim videoram. Bufer na zacatku */
/*       obsahuje pocet_slopcu(2B) a pocet_radku(2B). Dale pixly   */
/*       obrazu ulozene po radcich. 1 pixel = 1B.                  */

void x_getimg(int x1, int y1, int x2, int y2, char *bitmap);

/* x1,y1,x2,y2 - okno na obrazovce                                 */
/* bitmap      - bufer s obrazem                                   */


/*##---------------- x_putimg ---------------------- VGA_reg-------*/
/* Zobrazi v danem miste cast obrazovky, ktera byla uschovana pres */
/* funkci x_getimg.                                                */
/* Pozn: Pro 16 bar. mody je obraz zapsan pres Borland fci putimage*/
/*       Pro 256 bar. mody zapisem primo do videoram.              */

void x_putimg(int xz,int yz, char *bitmap, int op);

/* xz,yz - zacatek obrazu                                           */
/* bitmap- bufer s obrazem                                          */
/* op    - viz putimg(), jen 16 bar. mody                           */


/*##------------- x_maxx, x_maxy -----------------------------------*/
/* Zjisti maximalni velikost souradnic pro aktivni graf. mod        */

int x_maxx(void);       /* Ve smeru X */
#ifdef CALDERA
#define x_maxy() my_x_maxy()    // Screen size excludes S/W keyboard
#else
int x_maxy(void);       /* Ve smeru Y */
#endif // CALDERA


/*##------------- x_yncurs -----------------------------------------*/
/* Zapne (vypne) zobrazovani kursoru, zobrazi kursor v pocatecnim   */
/* bode a nastavi jeho barvu.                                       */

void x_yncurs(int on, int x, int y, int col);

/* on     = 1 -> zapnuti kursoru, = 0 -> vypnuti kursoru            */
/* Pozn: Vsechny dalsi parametry maji vyznam pouze pro zapnuti !!   */
/* x,y  - misto pocatecniho zobrazeni kursoru                       */
/* col  - barva kursoru                                             */


/*##------------- x_cursor -----------------------------------------*/
/* Simulace kurzoru 16x16 pixlu pro 256 barevne mody                */
/* Tvar kursoru se definuje vyplnenim tzv. screen a cursor masky.   */
/* Jde o bitova pole 16 * 16 bitu (viz. popis GMOUSE )              */
/* viz. fce x_defcurs()                                             */
/* Na kazde zavolani kurzor v starem miste zmizi a objevi se v      */
/* novem. Pri zadavani stejnych souradnic fce. x_cursor nedela nic. */

void x_cursor(int x, int y);

/* x,y - souradnice pro zobrazeni kursoru                           */


/*##------------- x_defcurs ----------------------------------------*/
/* Definuje tvar kursoru pomoci screen a cursor poli (jako v GMOUSE)*/

void x_defcurs(int *screen, int *cursor, int color);

/* int screen[16], cursor[16] - bitove masky 16x16 definice kurzoru */


/*##-------------------- x_wrtmode -----------------------------*/
/* Nastavi zpusob psani(prepis/XOR) do videoram pro funkce:     */
/* x_rect(), x_line(); ...                                      */

void x_wrtmode(int wrtmode);

/*wrtmode   -  0=prepis(implicitni), 1=XOR                      */

/*##-------------------- x_id_ellip ----------------------------*/
/* Identifikace hranice elipsy(kruznice) oknem                  */

int  x_id_ellip( int okno[4],       // identifikacni okno (I/O)
                 int x1, int y1,    // stred
                 int rx, int ry );  // polomery

/* Return : 0/1 - Idetifikovano NE/ANO                          */
/*          pro ANO je v okno[0],[1] identif. bod               */

/*##--------------------- img_to_file --------------------------*/
/* Vytvori soubor .OBR uschovanim casti obrazovky               */

int img_to_file(int x1, int y1, int x2, int y2, char *file);

// x1..y2 - okno na obrazovce
// file   - jmeno souboru
// Predpoklada se volani pouze v graf. modu
// Pozn: Zaokrouhluje se na nasobek 8, Pro setreni pameti pise
//       po radcich (neni moc rychly!)
//       4kB lokalnich promennych (STACK !)

/*##-------------------- x_detect ------------------------------*/
/* Detekce super VGA karty                                      */

int x_detect(char *svga, int *kby);

//  svag - retezec dle IBASE (vystup, TSG3,TSG4,OAK,TRIDENT...3)
//  kby  - pamet SVGA v kB   (vystup) - pouze 64..256kB
//  ret  - 0 - zadna znama nebyla detekovana
//         1 - byla detekovana

/*##-------------------- x_strcmp ------------------------------*/
/* Hledani substringu ve stringu (stringy mohou obsahovat bin 0)*/

char *x_strcmp(char *buf, int lenb, char *str, int lens);

//  buf - string ve kterem hledame, lenb- jeho delka
//  str - hledany retezec, lens- jeho delka
//  return - NULL - nenalezen, jinak - adresa str v buf


//##--------- Nastaveni param. pro zalamovani textu -----------
void x_set_zalom(int istyle, int ostyle, float MaxMez, float MaxRozt);

/*
istyle -   0 => respektovat vicenasobne mezery
           1 => vicenas.mezera = 1 mezera
ostyle -   bit 0:  0 => bez zarovnavani (="na praporek")
                   1 => zarovnavani na pravy okraj
           bit 1:  0 => nenechavat "sirotky" na konci radku
                   1 => nechavat sirotky (sirotek= 1 osamele pismeno:
                       a..z,A..Z,nebo jakakoliv otviraci zavorka)
MaxMez -   maximalni pripustna sirka mezery - zadava se jako nasobek
           sirky jedne mezery, musi byt >= 1.0; hodne velke => zarovnava
           se jen rozsirovanim mezer mezi slovy; doporucena hodnota:
           <2.0 .. 3.0>
MaxRozt -  maximalni pripustna pomocna mezera mezi znaky - nasobek
           prumerne sirky znaku. Je-li <= 0.0 => zadne mezery se
           nebudou vkladat, jinak se roztec vzdy zaokrouhlli alespon
           na 1 pixel; doporucena hodnota: <0.1 .. 0.4>
*/
//##------- Nastaveni psani do vide/XMS pro vektorove fce (x_line...)
void x_video_XMS(int vidXMS, int bincol);

//vidXMS - 0-psani na obrazovku(default), 1-do XMS(disk)
//bincol - barva pro psani do XMS pro binar (0-cerne,1-bile)

/*====================== Interni funkce ============================*/
void x_text_1(int col, int xz, int yz, char *text, int xr);
/* col  - barva textu                                             */
/* xz   - zacatek textu vodorovne                                 */
/* yz   - zacatek textu svisle                                    */
/* text - psany text                                              */
/* xr   - pro 16 barevne mody 0/1 = prepis/XOR                    */
/*        pro 256 barevne mody - barva pozadi                     */

void x_putpix_v(int x, int y, int color);
int  x_getpix_v(int x, int y);

int z_xmsputpix(int x, int y, int color);
int z_xmsgetpix(int x, int y);
int z_xmsbar(int x1, int y1, int x2, int y2, int col);

void rea_w256(char  *buf, int xz, int yz, int dx, int dy);
void rea_bincga(char  *buf, int xz, int yz, int dx, int dy);
void x_bar256(int col, int xz, int yz, int dx, int dy);
void x_putpix_s(int x, int y, int color);
int  x_getpix_s(int x, int y);
void x_rect_s(int col, int xz, int yz, int dx, int dy, int xr);
void wrt_chr(char  *buf, int xz, int yz, int dx, int dy,
             int col1, int col2);
int  wrt_video1(char *buf, int xz,int yz, int ncol,int nrow, int pix);
void wrt_bincga(char  *buf, int xz, int yz, int dx, int dy,
                int LxMask, int First);
int  x_viewline(int *x1, int *y1, int *x2, int *y2 );
void pal_vga_ega(char *pal1, int *ega_col, int jak); /* Prevody EGA <--> VGA */
void pal_ega_vga(int ega_col, char *pal1);
void x_bar_patt(int x1,int y1,int dx,int dy);
void x_gimg16(int x1,int y1,int x2,int y2,char *buf,int plan);  //GETIMAGE
void x_pimg16(int x1,int y1,char *buf, int op, int plan);       //PUTIMAGE
void x_b16(int x1,int y1,int x2,int y2,int col,int mask);       //16BAR

/*====================== Externi promenne lopifu ===================*/

extern int xg_mod;                   /* Graf. mod   */
extern int xg_256;                   /* 0/1 nastven 256 barevny mod */
extern int xg_color;                 /* Nastavena barva (rect,text) */
extern int xg_fillc;                 /* Nastavena barva pro x_bar() */
extern int xg_wrt;                   /* 0=prepis, 1=XOR             */
extern int xg_style;                 /* Definice cary               */
extern int xg_xr,xg_yr;              /* ratio                       */
extern int xg_view[4];               /* Viewport                    */
extern int xg_xfnt,xg_yfnt;          /* Velikost fontu v pixlech    */
extern int xg_tjustx;                /* Zarovnani textu ve smeru X  */
extern int xg_tjusty;                /* Zarovnani ve smeru Y        */
extern int xg_clip;                  /* Zda orezavat ve viewportu   */
extern int xg_fbyt;                  /* Pocet bytu na znak fontu    */
extern int xg_flag;                  /* Priznaky: bit 0 - spec EGA pal */
                                     /*           bit 1 - aktivni patt */
extern unsigned char xg_upatt[8];    /* Vzorek 8x8 pro vyplnovani      */
extern char *xg_fbuf;                /* bufffer na font                */

#if HI_COLOR
extern char xg_curs[520];            /* Image pod kursorem          */
#else
extern char xg_curs[260];            /* Image pod kursorem          */
#endif

extern char xg_f_cur;
extern unsigned int xg_c_col;       /* On/Off,barva kurzoru     */
extern int  xg_x_cur,xg_y_cur;      /* Posledni souradnice      */
extern int xg_s1[16];   /* Screen mask  */
extern int xg_s2[16];   /* Cursor mask  */
extern int xg_svga;     /* Bit. mapa pro SVGA mody       */
                        /* nizsi byte : bit 0..3 delka radku 320..1024 */
                        /*              bit 7    0=16/1=256 barev      */
                        /* vyssi byte : 0..6 typ graf karty (7=VESA)   */
extern int xg_intern;   /* Interni cislo graf. modu 0..7 */
extern char xg_egapal[17];  /* Paleta pro EGU */
extern unsigned int xg_and16[68];   // Masky pro 16 bar. cursor.
extern unsigned int xg_or16[68];
extern int xg_fnt_zoo;      /* Pro zooming textu 1 | 2 */
extern int xg_col_plan;     // Pocet. bit. rovin
extern int xg_notview;      // 0-do vieportu, 1-absolutne (jen text)

//---- Novinky 930319 pro WINDOWS fonty ------------------------
extern unsigned char xg_fonlen[256];  // Sirky znaku pro proporcni fonty
extern long  int     xg_fonadr[256];  // Zacatky znaku pro prop. fonty
extern unsigned char xg_foncon;       // Flag - konst/prop font [0/prumer]
extern unsigned char xg_fonmem;       // Kde je font: 0-MEM(xg_fbuf),1-XMS,2-DISK
extern int           xg_fonhan;       // Handle pro XMS/DISK
extern unsigned int  xg_lbfnt;        // delka bufru s daty fontu
extern char          xg_fnt_akt[64];  // Jmeno akt. fontu
// Pozn: pro fonty v XMS|DISK xv_fbuf 4K pro jeden znak
extern long          xg_fntalloc;     // Fixni delka bufru pro font

//---- Vice fontu v XMS 971001
#define  SIZE_XMS_FNT  10000   // prum velikost fontu v XMS

struct FNTXTAB             // pro jeden font v pameti
{
   long         Offset;    // offset bufru s fontem v XMS
   unsigned int Size;      // delka fontu v XMS
//mp:!!! updated!!!
   char         Name[80];  // jmeno fontu
};

struct FNTMBUF             // buffer pro zapis/cteni fontu XMS
{ unsigned int  lbfnt;
  int           xfnt;
  int           yfnt;
  int           fbyt;
  int           foncon;
  int           fonmem;
  int           fonhan;
  unsigned char fonlen[256];
  long     int  fonadr[256];
};

extern int  xg_fnt_max;       // max pocet fontu v XMS
extern int  xg_fnt_fre;       // prvni volny v tabulce
extern int  xg_fnt_xms;       // handle XMS
extern long xg_fnt_xlen;      // celkova delka v XMS
extern long xg_fnt_xoff;      // volne misto v XMS
extern struct FNTXTAB *xg_fnt_xtab;   // tabulka fontu

//---- Zalamovani textu ----------------------------------------
//extern int   xg_istyle;               // Parametry funkce IZALOM(...)
//extern int   xg_ostyle;
//extern float xg_MaxMez;
//extern float xg_MaxRozt;

extern int   xg_31yn;                 // Zda znaky <32 kreslit/nebo jen posun
extern int   xg_chrmod;               // Kresleni pozadi textu
extern int   xg_no_mode;              // !=0-> doopravdy se grf.mod nenastavi
//---- pro VIRT video -----------------------------
extern int   xg_video_XMS;            // 0-kresli se do video, 1-do VIRT
extern int   xg_bincol;               // barva pro kresleni do binarni VIRT
//---- Psani barevneho textu (alokovat pole a naplnit barvami)
extern unsigned char *xg_chr1c;       // Popredi
extern unsigned char *xg_chr2c;       // Pozadi


//---- Hi color mody ------------------------------
#if HI_COLOR
// Prvod do hi-col z palety (0..63)
#define RGBHI15(R,G,B)  (((unsigned)R>>1)<<10)|(((unsigned)G>>1)<<5)|((unsigned)B>>1)
#define RGBHI16(R,G,B)  (((unsigned)R>>1)<<11)|((unsigned)G<<5)|((unsigned)B>>1)

// Fce "C":
//##---- Prevod bufru s RGB triplety do Hi-col (2B)
void xh_RgbToHi(unsigned char *Rgb, unsigned char *Hi,
            int Pixs, int Rows, int LenLin);
//Rgb   - in - Buffer se vstupnimi radky obrazu v RGB (0..255)
//Hi    - out- Buffer s vystupnim obrazem v Hi-col
//Pixs  - in - Pocet pixlu na radek
//Rows  - in - Pocet radku obrazu (Rows < 0 : zrcadleni radku)
//LenLin- in - Delka vstupniho radku v bytech

//##---- Prevod bufru s bytovym (paletovym) obrazem do Hi-col
void xh_ByteToHi(unsigned char *Ib1, unsigned char *Hi,
            int Pixs, int Rows, int LenLine);
//Ib1   - in - Buffer se vstupnimi radky obrazu: pixel=byte(0..255)
//Hi    - out- Buffer s vystupnim obrazem v Hi-col
//Pixs  - in - Pocet pixlu na radek
//Rows  - in - Pocet radku obrazu (Rows < 0 : zrcadleni radku)
//LenLin- in - Delka vstupniho radku v bytech

//##---- Prevod R,G,B (paleta 0..63) do Hi-col
unsigned xh_RgbHiPal(unsigned char R, unsigned char G, unsigned char B);
// R,G,B - in - polozky palety R,G,B v rozsahu 0..63
// return  :    Hi-color pixel

//##---- Prevod Hi-col pixlu na R,G,B slozky (0..63)
void     xh_HiPalRgb(unsigned int Hi, unsigned char *rgb);
//Hi   - in - pixel v Hi-color (napr. z x_getpix() )
//rgb  - out- slozky R,G,B pixlu
//Pozn:  xh_RgbHiPal() a xh_HiPalRgb() nejsou invezni fce. Pri
//       prevodu dochazi ke ztrate presnosti v jednom bitu na slozku.

//##---- Nastavi zpusob ziskani delky radku pri kresleni fci wrt_video()
int      xh_SetRounding(int Round);
//round - in - Jak ziskat delku radky v bytech:
//    0 - delka radky se spocte prostym nasobenim pixlu a bitu na pixel
//    1 - delka radky se zaokrouhli nahoru na nasobek 4 bytu (napr.BMP)
//    return : puvodni round
//    implicitni nastaveni : 0

//##---- Nastavi zpusob prace s paletou v Hi-color modech
int      xh_SetPalMode(int Mode);
//Mode - in - Jak pracovat s paletou
//   0 - Vsechny barvy x_setcol(),... se zadavaji jako index do plalety
//       kterou je ovsem treba predem nastavit
//   1 - Barvy se zadavaji jako Hi-col RGB, paleta nemusi byt nastavena
//   return : puvodni mode
//   implicitni nastaveni : 0

// Fce "ASM" -> internal
int  xh_getpix(int x, int y);
void xh_putpix(int x, int y, int color);
int  xh_read(unsigned char *buf,int xpix, int ypix, int ncol, int nrow);
void xh_barx(unsigned int col, int xz, int yz, int dx, int dy);
int  xh_wrtchr(char *buf,int xpix, int ypix, int ncol, int nrow, int col1, int col2);
int  xh_write(unsigned char *Buf, int x, int y, int dx, int dy);

// Externy
extern int xg_hi16;    // mode : 16bit=1,15bit=0
extern int xg_xgr;     // poradi RGB v tripletu pro true color, 0=BGR,1=RGB
extern int xg_hipalmod;// x_setcolor(), x_setfill():0=index, 1=primo hicolor
extern int xg_round;   // pro vypocet delky radku v B (0=dle ncol, 1-nas. ctyr(BMP))
extern unsigned char xg_hipal[768];  // Pal pro HiCol mode (nastavit pres x_setpal())
extern unsigned int  xg_hival[256];  // Hi-col hodnoty k palete
#endif

#ifdef VIRT_SCR
//  *******  Fce for managing with virtual videoram: ********
//  Virtual videoram is in XMS(EMS) or disk file and simulate
//  normal videoram.
//  Supported types : 1bit/pixel, 8bit/pixel,
//  Max number of virtual videoram in program : 3

//  Using:
//  1. Create or open virtual videoram(s), set active virt.videoram
//  2. Call x_video_XMS(1,...), All output from xlopif->virt.mem.
//  3. Call xlopif function, x_bar(), x_text_ib(),...
//  4. Copy virt. videoram to screen
//  5. Close (delete) virt. videoram

//##--- Create new virtual videoram
int xv_new_virt(char *filenam, // File name for disk file
              int dx, int dy,  // Size in pixels
              int col,         // Default color
              int bitpix,      // 3-1bit/pixel, 0-8bit/pixel, -1-16bit/pixel
              int npal,        // Length of palette
              char *pal,       // Palette, range RGB 0..63, max. 256 entries
              int Index,       // Index 0..5, number of virtual videoram
              int Typ);        // 0-XMS or DISK, 1-XMS, 2-DISK

//  return   1 - O.K. XMS,
//           3 - O.K. Disk, else error

//##--- Open file .OBR as virtual videoram
int  xv_opn_virt(char *filnam,    // File name of .OBR
                int Index,        // number of virtual videoram
                int Typ);         // 0-XMS or DISK, 1-XMS, 2-DISK
// return   1 = O.K XMS
//          3 = O.K disk
//          2 = Open error .OBR,   4 = Read error
//          6 = Memory DOS error,  8 = Write error (XMS)

//##---- Close virtual videoram .OBR vytvoreny xv_opn_file,xv_new_obr
int  xv_cls_virt(int Xwrt,        // 0-only free XMS, 1-write XMS on disk
                int Index);       // number of virtual videoram
// return   1 = O.K. else error

//##---- Set active virtual videoram for next operation
int xv_set_actvirt(int Index);    // 0..2, default 0

//##---- Copy rect from active virt.videoram to screen
int  xv_to_scr(int xs, int ys, int xo, int yo, int dx, int dy);
// xs,ys - left upper point in virt.videoram
// xo,yo - left upper point in screen
// dx,dy - size in pixels
// return - 1 - O.K. else error

//##---- Copy rect from screen to active virt. vieoram
int  xv_to_virt(int xo, int yo, int xs, int ys, int dx, int dy);
// xo,yo - left upper point in screen
// xs,ys - left upper point in virt.videoram
// dx,dy - size in pixels
// return - 1 - O.K. else error

//##--- Read rect to buffer from virt.videoram in format getimage()
int  xv_int_rea(int xs, int ys, int dx, int dy, char *buf);
//   xs,ys - upper left point
//   dx,dy - size in pixels
//   buf   - output buffer with format getimage(), max. 64KB !!!

//##--- Write rect to virt mem. in format putimage() -----
int  xv_int_wrt(int xs, int ys, char *buf);
//   xs,ys - upper left point
//   buf   - input buffer with format getimage(), max. 64KB !!!

//##--- Write text to virt. screen
int xv_text_virt(int xp, int yp, char *text);
//   xp,yp - point in pixels
//   text  - buffer with text;

//##--- Write rect to virtual screen
int xv_wrt_virt(char *Buf, int xz, int yz, int ncol, int nrow, int pix);

// WARNING:
// getimage,putimage,cursor working always with real videoram

// RESTRICTION:
// 1. Not implemented XOR, only COPY for putpixel, line, rect, bar ...
// 2. Not implemented fill pattern for bar, only solid fill

//-------------------- Global var. for vv -------------------
typedef struct tagXV_VV
{
int  xv_XMS;           // Pro extended memory ( -1 = disk)
int  xv_file;          // Otevreny soubor .OBR (handle)
int  xv_bits;          // Pro shift 8bit=0,4bit=1,1bit=3
int  xv_len_col;       // Pocet bytu radku
int  xv_rows;          // Pocet radku
int  xv_zmap;          // Zacatek obrazu v souboru (512,1024)
//!!mp: reduced
char xv_File[16];
} XV_VV;

extern XV_VV xv_vv[MAXVIRTUAL];
extern int   xv_act;    // Actual vv
extern int   xv_NoDisk; // 1-only XMS, 0-Disk|XMS
#endif
/*==================================================================*/