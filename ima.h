/* Promenna pro EXT driver. XMOVE je struktura, se kterou         */
/* je nutno pracovat.                                             */
typedef struct
  {
  unsigned long length;         /* velikost prenasene pameti      */
                                /* udava se v nasobcich  B    */
  unsigned int sourceH;         /* handle pro zdroj (0=konvencni  */
                                /* pamet pocitace)                */
  unsigned long sourceOff;      /* offset zdroje pameti           */
  unsigned int destH;           /* handle pro terc (0=konvencni   */
                                /* pamet pocitace)                */
  unsigned long destOff;        /* offset terce pameti            */
  } XMOVE;

/* p = pointer to move structure                                  */
/* returns true if successfull                                    */
/* THIS FUNCITON MOVES EXTENDED MEMORY                            */
int move_xmem(XMOVE *p);

/* int h =  handle to deallocate                                  */
/* returns true if successfull                                    */
/* THIS FUNCTION DEALLOCATES EXTENDED MEMORY                      */
int dealloc_xmem(int h);

/* n = number of kilobytes to allocate                            */
/* returns handle or -1 if error                                  */
/* THIS FUNCTION ALLOCATES EXTENDED MEMORY                        */
int alloc_xmem(int n);

/* THIS FUNCTION CONVERTS A POINTER TO AN INTEL LONG              */
int long ptr2long(char *p);

/* THIS FUNCTION INITIALIZES THE DRIVER                           */
/* returns the version number or -1 if no driver                  */
int get_xmem(void);

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!    */
/* Priklad na pouziti fci getIm, putIm, putStrip, freeIm        */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!    */
/*                                                              */
/* Uchovame oblast x1=25, y1=30, x2=200, y2=250. Vse smazeme.   */
/* Zobrazime druhy pruh o 5 pixlu nahoru oproti puvodnimu       */
/* umisteni. Zobrazime znovu druhy pruh s levym hornim bodem    */
/* do mista 0,0. Zobrazime celou puvodni oblast s levym hornim  */
/* bodem v miste 15,15 - tim uvolnime i vsechny bafry.          */
/*                                                              */
/* V programu musi byt nekde deklarovana globalni promenna      */
/*     char memMore;                                            */
/* ktera rika, ktera pomocna pamet (Extended, Expanded, Disk,   */
/* VDisk) je k dispozici. Nyni nneni implementovano Expanded    */
/* a VDisk. Vyznam promenne:                                    */
/*                                                              */
/* memMore: 0 = default = pamet pocitace. Kdyz dojde, je        */
/*              vyuzivan disk.                                  */
/*          1 = extended memory. Kdyz dojde pammet pocitace,    */
/*              je nejdrive pouzita tato pamet, pak je vyuzivan */
/*              disk. Pro pouziti musi byyt instalovan HIMEM.   */
/*              !!! POZOR !!! Overlay pomoci extended a extended*/
/*              pomoci HIMEM si do sebe kecaji. Program funguje,*/
/*              nespadne ale nakonec zvitezi HIMEM, overlay     */
/*              pomoci extended memory prestane fungovat a      */
/*              overlay se dela pak jiz jen pres diskove operace*/
/*              takze se program zpomali. Pokud nechceme        */
/*              vyuuzivat HIMEM, nesmime volat get_xmem() !!!   */
/*          2 = expanded memory. Neni implementovano.           */
/*          3 = VDisk. Neni implementovano.                     */
/*          4 = disk.                                           */
/*                                                              */


//     char far *scrBuf,          /* adresa bafru pro uchovani */
                                  /* cele puvodni oblasti      */
//              *pomBuf;        /* adresa bafru s pruhem     */
//     int numStrip;              /* pocet pruhu oblasti       */
//     int yps;                   /* pro funkci putStrip       */
//     int stav;                  /* status ukonceni funkci    */
//     char memMore;              /* promenna pro typ pameti   */

       /* zjistime, zda je pritomen driver HIMEM                */
//     stav = get_xmem();
       /* pokud je driver pritomen (stav != -1) a je verze >= 2 */
       /* (stav >= 0x0200), tak to dam vedet pomoci memMore=1   */
       /* jinak ponecham memMore=0                              */
//     if (stav >= 0x0200 && stav != -1) memMore=1;
//     else memMore=0;

       /* Uchovani oblasti. Pocet pruhu je predan v numStrip */
//     numStrip = getIm ( 25, 30, 200, 250, &scrBuf);

       /* smazani obrazovky */
//     stav = x_clearview();

       /* Ziskame bafr s druhym pruhem */
//     yps = 30;       /* puvodni umisteni */
//     pomBuf = scrBuf;
//     stav = putStrip ( &yps, 2, &pomBuf);

       /* zobrazime bafr s pruhem o 5 pixlu vys, nez byl */
//     if (pomBuf!=NULL)
//       x_putimg (25, yps-5, pomBuf, 0);
       /* zobrazime bafr s levym hornim bodem o souradnicich 0, 0 */
//     if (pomBuf!=NULL)
//       x_putimg (0, 0, pomBuf, 0);
       /* zobrazime celou puvodni oblast do mista s levym hornim  */
       /* bodem v miste 15, 15. Tim jsou zaroven uvolneny vsechny */
       /* bafry s puvodni oblasti. Bez zobrazeni jsme mohli pouzit*/
       /* pro uvolneni i funkce freeIm.                           */
//     stav = putIm (15, 15, &scrBuf);
       /* uvolnime bafr s pruhem */
//     if (pomBuf!=NULL) free(pomBuf);


/* 		getIm						*/
/*								*/
/* Funkce: 	uuchova stanovenou oblast v bafru nebo na disku */
/*              nebo v extended memory                          */
/*              Bafr je automaticky alokovan                    */
/*                                                              */
/* Navrat:      0 = neuspesny konec.                            */
/*              Nejlepe je zavolat pak hnned freeIM             */
/*              cislo>0 = uspesny konec                         */
/*              cislo udava pocet vytvorenych pruhu             */
/*                                                              */
/* Parametry:   int x1, y1, x2, y2                              */
/*              Oblast, kterou chceme uchovat (v pixlech)       */
/*                                                              */
/*              char far **scr_buf                              */
/*              getIm vrati do teto prommennne hodnotu pointru  */
/*              prirazeneho bafru, ktery je automaticky funkce  */
/*              alokovan                                        */
/*                                                              */
int getIm (int x1, int y1, int x2, int y2, char far **scr_buf);

/* 		putStrip                                        */
/*								*/
/* Funkce: 	funkce zapise do bafru n-ty vyzadany pruh       */
/*              Bafr je automaticky alokovan                    */
/* 	   	                                                */
/* Navrat:      0 = neuspesny konec                             */
/*		1 = uspesny konec                               */
/*                                                              */
/* Parametry:   int *y                                          */
/*              do promenne zadame hodnotu y-ove souradnice     */
/*              leveho horniho bodu oblasti (v pixlech),        */
/*              kterou jsme uchovali a kam ji chceme umistit,   */
/*              a je vracena hodnota y-ove souradnice           */
/*              leveho horniho bodu vraceneho pruhu             */
/*                                                              */
/*              int n                                           */
/*              cislo pruhu, ktery chceme vratit                */
/*              pocita se od 1                                  */
/*                                                              */
/*              char far **scr_buf                              */
/*              Do promenne se zapise adresa pointru z getIm    */
/*              Funkce sem zapise adresu pointru alokovaneho    */
/*              bafru s pruhem.                                 */
/*                                                              */
int putStrip(int *y, int n, char far **scr_buf);

/* 		freeIm						*/
/*								*/
/* Funkce: 	Uvolneni obrazu z pammeti                       */
/* 	   	                                                */
/* Navrat:      1 = uspesny konec                               */
/*		0 = byl predan pointr na NULL                   */
/*                                                              */
/* Parametry:   char far **scr_buf                              */
/*              Do promenne se zapise adresa pointru z getIm    */
/*                                                              */
int freeIm(char far **scr_buf);

/* 		putIm                                   	*/
/*								*/
/* Funkce: 	Zobrazeni bafru na monitoru a dealokovani       */
/*              bafru.                                          */
/* 	   	                                                */
/* Navrat:      1 = uspesny konec                               */
/*		0 = neuspesny konec                             */
/*                                                              */
/* Parametry:   int x, y                                        */
/*		Souradnice leveho horniho bodu, kam ma byt      */
/*              obraz umisten (v pixlech).                      */
/*                                                              */
/*              char far **scr_buf                              */
/*              Do promenne se zapise adresa pointru z getIm    */
/*                                                              */
int putIm(int x , int y, char far **scr_buf);

int lendIm(int x , int y, char far **scr_buf);

extern char memMore;

