/* Promenna pro EXT driver. XMOVE je struktura, se kterou         */
/* je nutno pracovat.                                             */
/* tr.: variable for pro EXT driver. XMOVE is the structure       */
/*      you will have to work with.                               */

typedef struct
  {
  unsigned long length;         /* velikost prenasene pameti      */
                                /* udava se v nasobcich  B        */
    /* tr.: size of transferred memory, given in multiples of B   */
  unsigned int sourceH;         /* handle pro zdroj (0=konvencni  */
                                /* pamet pocitace)                */
    /* tr.: handle for source (0=conventional memory of computer) */
  unsigned long sourceOff;      /* offset zdroje pameti           */
    /* tr.: offset of memory source */
  unsigned int destH;           /* handle pro terc (0=konvencni   */
                                /* pamet pocitace)                */
    /* tr.: handle for target (0=conventional memory of computer) */
  unsigned long destOff;        /* offset terce pameti            */
    /* tr.: offset of memory target */
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
/* Example how to use fci getIm, putIm, putStrip, freeIm        */
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
/* tr.: We save the area x1=25, y1=30, x2=200, y2=250. We       */
/*      delete everything. We display another stripe 5 pixels   */
/*      above the original location. We display again another   */
/*      stripe with the left upper point to location 0,0. We    */
/*      display the whole original area with the left upper     */
/*      point at location at 15,15. Thus we also release all    */
/*      buffers.                                                */
/*                                                              */
/*      It is also necessary to declare somewhere in the        */
/*      program the global variable char memMore, that tells    */
/*      which kind of auxiliary memory (Extended, Expanded,     */
/*      Disk, VDisk) is available. VDisk and Expanded are not   */
/*      yet implemented. The meaning of the variables:          */
/*                                                              */
/*      memMore: 0 = default = computer memory. When it         */
/*                   expires, disk will be used                 */
/*               1 = extended memory. When computer memory      */
/*                   expires, first this memory will be used,   */
/*                   then disk be used. In order to do so,      */
/*                   HIMEM must be installed. !!! ATTENTION !!! */
/*                   Overlay with help of extended and extended */
/*                   with help of HIMEM talk to each other.     */
/*                   The program wor, does not crash/fall, but  */
/*                   in the end wins HIMEM, the overlay with    */
/*                   extended memory will stop to work and the  */
/*                   overlay will then be done only through     */
/*                   disk operation, and the program will slow  */
/*                   down. If we do not want to use HIMEM, we   */
/*                   must not call get_xmem() !!!               */
/*               2 = expanded memory. not implemented yet       */
/*               3 = VDisk. not implemented yet                 */
/*               4 = disk.                                      */
/*                                                              */


//     char far *scrBuf,          /* address of buffer for saving  */
                                  /* whole original area           */
//              *pomBuf;          /* address of buffer with stripe */
//     int numStrip;              /* number of stripes of area     */
//     int yps;                   /* for function putStrip         */
//     int stav;                  /* status of ending of functions */
//     char memMore;              /* variable for memory type      */

       /* zjistime, zda je pritomen driver HIMEM                   */
       /* tr.: we will find out, if the driver HIMEM is present    */
//     stav = get_xmem();
       /* pokud je driver pritomen (stav != -1) a je verze >= 2 */
       /* (stav >= 0x0200), tak to dam vedet pomoci memMore=1   */
       /* jinak ponecham memMore=0                              */
       /* tr.: if the driver is present (stav != -1) and its    */
       /*      version >= 2 (stav >= 0x0200), I will let you    */
       /*      know with memMore=1; otherwise I leave memMore=0 */
//     if (stav >= 0x0200 && stav != -1) memMore=1;
//     else memMore=0;

       /* Uchovani oblasti. Pocet pruhu je predan v numStrip        */
       /* tr.: Saving area. Number of stripes is passed in numStrip */
//     numStrip = getIm ( 25, 30, 200, 250, &scrBuf);

       /* smazani obrazovky */
       /* tr.: erasing screen */
//     stav = x_clearview();

       /* Ziskame bafr s druhym pruhem */
       /* tr.: We get buffer with another/second stripe */
//     yps = 30;       /* original location */
//     pomBuf = scrBuf;
//     stav = putStrip ( &yps, 2, &pomBuf);

       /* zobrazime bafr s pruhem o 5 pixlu vys, nez byl */
       /* tr.: we display buffer with stripe 5 pixels    */
       /*      above its former location                 */
//     if (pomBuf!=NULL)
//       x_putimg (25, yps-5, pomBuf, 0);
       /* zobrazime bafr s levym hornim bodem o souradnicich 0, 0 */
       /* tr.: we display buffer with left upper point  */
       /*      at coordinates 0, 0 */
//     if (pomBuf!=NULL)
//       x_putimg (0, 0, pomBuf, 0);
       /* zobrazime celou puvodni oblast do mista s levym hornim  */
       /* bodem v miste 15, 15. Tim jsou zaroven uvolneny vsechny */
       /* bafry s puvodni oblasti. Bez zobrazeni jsme mohli pouzit*/
       /* pro uvolneni i funkce freeIm.                           */
       /* tr.: we display the whole original area at the location */
       /*      with left upper point at 15,15. This way all       */
       /*      buffers with the original area are released, too.  */
       /*      Without displaying we would have also be able to   */
       /*      free the memory through function freeIm.           */
//     stav = putIm (15, 15, &scrBuf);
       /* uvolnime bafr s pruhem */
       /* tr.: we release buffer with the stripe */
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
/* tr.: Function:   saves given/declared area in the buffer or  */
/*                  on disk or in extended memory               */
/*                  Buffer is allocated automatically           */
/*                                                              */
/*      Return:     0 = fail; then it will be the best to call  */
/*                   freeIM immediately                         */
/*                  number>0 = successful; the value gives the  */
/*                   number of stripes created                  */
/*                                                              */
/*      Parameters: int x1, y1, x2, y2                          */
/*                  are that we want to save (in pixels)        */
/*                                                              */
/*                  char far **scr_buf                          */
/*                  into this variable getIm returns the value  */
/*                  of the pointer of corresponding buffer,     */
/*                  automatically allocated to/by the function  */
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
/* tr.: Function:   function writes into buffer the nth         */
/*                  requested stripe.                           */
/*                  The buffer is allocated automatically       */
/* 	   	                                                */
/*      Return:     0 = fail                                    */
/*                  1 = success                                 */
/*                                                              */
/*      Parameters: int *y                                      */
/*                  into the variable we put the value of the   */
/*                  y-coordinate of the left upper point to the */
/*                  area (in pixels), that we have saved, and   */
/*                  where we want to locate it; and the value   */
/*                  of the y-coordinate of the left upper point */
/*                  of the returned stripe will be returned     */
/*                                                              */
/*                  int n                                       */
/*                  number of stripes we want to return         */
/*                  counting starts at 1                        */
/*                                                              */
/*                  char far **scr_buf                          */
/*                  into the variable we write the address of   */
/*                  the pointer from getIm; here, the function  */
/*                  writes the address of the pointer of the    */
/*                  buffer that has been allocated with the     */
/*                  stripe                                      */
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
/*								*/
/* tr.: Function:   Releasing the picture from memory           */
/* 	   	                                                */
/*      Return:     1 = successful                              */
/*                  0 = a pointer to NULL has been passed       */
/*                                                              */
/*      Parameters: char far **scr_buf                          */
/*                  into the variable we write the address      */
/*                  of the pointer from getIm                   */
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
/* tr.: Function:  Displaying buffers on monitor and            */
/*                 deallocating buffers.                        */
/* 	   	                                                */
/*      Return:     1 = success                                 */
/*                  0 = fail                                    */
/*                                                              */
/*      Parameters: int x, y                                    */
/*                  Cooridinates of left upper point, where     */
/*                  the picture shall be located (in pixels).   */
/*                                                              */
/*                  char far **scr_buf                          */
/*                  Into the variable we write the address      */
/*                  of the pointer from getIm                   */
/*                                                              */
int putIm(int x , int y, char far **scr_buf);

int lendIm(int x , int y, char far **scr_buf);

extern char memMore;

