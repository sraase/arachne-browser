
//(c)1998 xChaos software

#ifdef NOKEY
#define URLSIZE 1024 //(GPL version)
#else
#define URLSIZE 1024 //(UE version)
#endif
#define MAXPALMIX 20

struct picinfo
{
 char URL[URLSIZE]; //url souboru s obrazkem
 char filename[80]; //jmeno souboru na lokalnim disku:
		    //prazdne - je treba analyzovat URL
		    //NUL - obrazek neni dostupny
 char alt[80];      //alternativni popis, pokud obrazek neni v pameti
 int from_x,from_y; //odkud kreslit na _obrazovce_
 int pic_x,pic_y;   //odkud kreslit v ramci obrazku (v souradnicich _obrazovky_)
 int stop_x,stop_y; //zatazit se o (souradnice na _obrazovce_)
 int size_x,size_y; //velikost _obrazku_
 int resize_x,resize_y;//zmenena velikost _obrazku_
 char palonly;      //vrat mi JEN paletu !!!
 char sizeonly;     //vrat mi JEN velikost obrazku !!!
 char palismap;     //paleta je pole indexu barev primo na obrazovce
 unsigned char pal[768];     //data palety
 int  npal;         //if palonly:     delka vracene palety
		    //if not palonly: kolik barev smis zabrat
 char is_background;//ma obrazek vyplnit pozadi ? (pokud se bude kreslit)
 char is_animation; //je GIF animovany ?
 char sequence;     //ktery frame animovaneho gifu kreslit ?
 unsigned char bgindex;   //pro background GIFy
 int draw_x,draw_y,screen_x,screen_y; //pozor - plati pro virt. obrazovku!
 int html_x;
// int unknownsize;   //size of this image is unknown
 long html_y;
 char picinfo_frameID;
 long filesize;

 // v XMS : 4B-delka img, 4B-next adr. pro anim.,nebo 0, nasleduje img.
 int  IsInXms;     // Je gif uz v XMS
 long BegImg;      // Adresa prvniho obrazku v XMS
 long NextImg;     // Adresa nasledujiciho obr v XMS (animovane)
 int  offx1, offy1;// posun, doplnek size_x,y, vetsinou 0,0
 int  x1gif, dxgif;// vyrez v gifu (pracovni promenne)
 long BckImg;      // >0 : pointr na ulozeny bacground pod gifem

 unsigned hPicInfo;// xSwap pointer na tuhle celou struktru
};
