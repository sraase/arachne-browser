//*******************************************************************
//*                      PresspalP
//*    slouceni mnoziny vst. palet a zkraceni na dannou delku
//* puv PresspalN = verze pro velmi dlouhou celkovou delku vst.palet
// 970415 - rozsireni pro npalout==2 (...CGA)
// 970421 - uprava pripadu 2 palet: pokus o zatrideni 2.pal. do prvni
//*******************************************************************
/*
Parametry:
 multip  - pocet vstupnich palet ( >= 1 )
*Palin[] - pole pointruu na vstupni palety
*Npalin  - pole delek vstupnich palet stejnolehle s *Palin
           Pozn.: palety jsou na zacatku okopirovany do pomocnych
           poli v poradi v nemz jsou jejich pointry v Palin a dale se
           pracuje de facto s 1 paletou "palin" delky npalin=suma(Npalin[i])
!!!**** POZOR: POMOCNA POLE: zabiraji celkem  (npalin * 20) bytu !!! *******
*palout  - "vystupni" paleta
*npalout - pozadovana delka zkracene palety (*palout) => pozor, vstupni i
           vystupni par.!
*mapio   - prevodni mapa - vystupni parametr
          ==> j = mapio[i] ... i= vstupni barva, j = vystupni barva
          mapio musi byt dimensovano na delku npalin (* 2B)
*Mmapio[]-pole pointruu do mapio pro jednotlive vst. palety - pole
          je stejnolehle s *Palin a *Npalin a musi mit delku <multip>
          Je-li Mmapio == NULL s polem se nepracuje.
Swinout- prepinac realizace cinnosti:
        (= 1  -> *palout se nevyuziva, mapio zobrazuje relokaci v *palin)
         !!!! zruseno - u PresspalN musi byt Swinout==2 !!!!!!!!
              ponechano jen kvuli kompatibilite s Presspal...
         = 2  -> zkracena paleta je zapsana do *palout, mapio zobrazuje
                 vstupni barvy do *palout;
TypFuse- typ slucovani vybranych dvojic barev:
         = 0 -> za vyslednou barvu bude zvolena barva s nizsim por. cislem
         = 2 -> za vyslednou barvu bude zvolena barva s vyssim por. cislem
         = 1 -> do *palout bude vlozena nova barva, vznikla jako prumer
                puvodnich dvou barev. Ma smysl jen pri Swinout=2, jinak
                jako TypFuse= 2;
         = 3 -> vysledna barva bude zvolena nahodne (0 a 2 nema smysl, je-li
                vstupnich palet vic!)
Tolerance- max. vzdalenost dvou barev, ktere maji byt automaticky slouceny:
           Tolerance < 0  ==> bez ucinku,
           Tolerance == 0 ==> vsechny shodne dvojice barev jsou slouceny
                              i za cenu zmenseni zadaneho *npalout
           Tolerance > 0  ==> vsechny dvojice barev jejichz vzdalenost
                     je <= Tolerance jsou slouceny, ev. i se zmensenim
                     *npalout; Vzdalenost = soucet diferenci v R/G/B.
*Savecols- oznaceni "chranenych" barev, ktere maji zustat ve vystupu
           zachovany; Savecols je stejnolehle pole s palin, je-li
           (Savecols[i] != 0) zustane i-ta barva zarucene v palete.
           Pri kopirovani chranenych barev do *palout se postupuje takto:
           Pokud je to mozne, zustanou chranene barvy na stejnem indexu
           jako byly v palin. Do vzniklych "mezer" jsou nejprve ulozeny
           ostatni barvy, setridene podle intenzity, potom ev.zbyvajici
           chranene barvy (to je ty, jejichz index byl vetsi nez npalout).
           -> Zjevne musi byt ( (pocet chranenych barev) <= npalout ) !
           -> Pri slucovani dvojice {chranena,nechranena} se samozrejme
              neuplatni TypFuse (ani prumerovani!), vysledkem slouceni
              je v tomto pripade vzdy chranena barva.
           *** Savecols==NULL => zadne chranene barvy: uvnitr funkce se
               vygeneruje pomocne pole Savecols a ochrani se nejtmavsi a
               nejsvetlejsi barva ze vsech vstupnich barev.
   *** pri Savecols==NULL  je vzdy vysledna paleta setridena podle intenzity

Pozor! u vsech slozek R/G/B se uvazuje vsech 8 bitu, tzv. EGA bity
       ze vstupnich dat musi byt vymaskovany pred Presspal.. ! (funkce
       je naopak zarucena i pro R/G/B z intervalu <0..255> )

return: 1 = O.K.
        2 = nesmyslne parametry
        4 = nedostatek pameti
        6,.. = selhani algoritmu (nemelo by nastat)

Poznamka: "Vystupni" paleta muze byt i kratsi nez vstupni hodnota *npalout,
          v pripade, ze nektere barvy v palin jsou shodne. V takovem pripade
          je npalout prepsano (zkraceno)
(c) ivan polak, 1992-1997
*/


#include "posix.h"

// ===== prototypy ======================================
int TagSort(unsigned long *ival,int *itag,int ip,int ik);
//p:void palpal (char *palin, int npalin, char  *palout, int npalout,
//             int *mapio, char metoda, int VahaB, int VahaS, int VahaI );
void Ipalpal (char *palin, int npalin, char  *palout, int npalout,
               int *mapio );
int INearCol( char* pal, int npal, char colr, char colg, char colb);

//=====================================================================
int PresspalO (int multip, char *Palin[], int *Npalin, char  *palout,
               int *npalout, int *mapio, int *Mmapio[],
               int Swinout, int TypFuse, int Tolerance,
               char *Savecols )
{
  int iin,jin,iout,jout,ir,ig,ib, npal1,npal2,i,j,ireturn,
      jmin, kmaxmin, ksummin, kmax, ksum, kr2,kg2,kb2,
      multi0,Tol1,j1,j2,i1,ndelta,npalin2,jkonec,n1stop,
      nsavecols, s1, iblack, iwhite,jlev,jprav;
  long Qcol1,Qcol2;
  char *palin, *pal2;
  int npalin;
  int *IR, *IG, *IB, *Order, *Soused, *Multi;
  unsigned long *Dist;
  char *Origpalin, *savecols;
#define Moc 32767
  long LOWLIM = 150l;  //#define ?
  long HIGHLIM= 500l;
//=====main==============================================
  if(multip < 1) return 2; //test pripustnosti
  IR=IG=IB=Order=Soused=Multi=NULL;
  Dist=NULL; Origpalin=savecols=NULL;
 if(multip==2)
  //? if(Npalin[1] <= Npalin[0])
 {  //Pokus o jednoduche zatrideni 2. palety do prvni >>>>>
     //(na Savecols se v tom pripade kasle!)
  npal1= Npalin[0];
  npal2= Npalin[1];
  palin= Palin[0];
  pal2=  Palin[1];
  Qcol1=Qcol2= 0l;
  i=iin=0;//indexy do 1. palety
  memcpy(&Qcol1,palin,3);//1.barva
  j2= npal1;//index do mapio
  for(j=jin=0; j < npal2; j++,jin+=3)  // cykl pres 2.paletu >>>
  { memcpy(&Qcol2,&pal2[jin],3);
    n1stop=i;
Test:
    if(Qcol2==Qcol1)
	 { mapio[j2]=i; j2++;//?npalin2++;
	   if(i==j)//uplne totozne palety?
	   { i++; iin +=3;
	     if(i>=npal1)
                         i=iin=0; //ach jo, cykl dokolecka, dokola
	     memcpy(&Qcol1,&palin[iin],3);
	     n1stop=i;
	   }
	 }
    else { i++; iin +=3;
//p:       if(i>=npal1) goto Normalne; //jednoduse to nejde!
	   if(i>=npal1)
                         i=iin=0; //ach jo, cykl dokolecka, dokola
	   if(i==n1stop) goto Normalne; //jednoduse to nejde!
/*//start misa 13.5.
	   if(i>=n1stop) goto Normalne; //<- tyhle dve podminky
	   if(i>=npal1) i=iin=0;        //<- bylo nutne prohodit!
//end misa 13.5.
*/
	   memcpy(&Qcol1,&palin[iin],3);
           goto Test;
         }
  }//end for...
// Uspech, povedlo se!!
  *npalout= npal1;
  for(i=0;i<npal1;i++) mapio[i]=i; //doplneni mapio
  if(Mmapio != NULL)
  {  Mmapio[0]= mapio;
     Mmapio[1]= &mapio[npal1];
  }
  return 1;
 }//end (multip==2)
Normalne:
  for(npalin=0, j=0; j<multip; j++) npalin += Npalin[j];

  if(npalin < 2 || *npalout < 2 ) return 2; //test pripustnosti
//p:  if(npalin < 3 || *npalout < 2 ||
//p:               *npalout > npalin) return 2; //test pripustnosti

  if(Swinout != 2) return 2;
  if(*npalout==2) goto BlackWhite;

  ireturn = 4;
  IR= farmalloc( (unsigned long)(sizeof(int) *npalin) ); //pomocna pole
  if(IR == NULL) return 4;
  IG= farmalloc( (unsigned long)(sizeof(int) *npalin) ); //pomocna pole
  if(IG == NULL) goto freeIR;
  IB= farmalloc( (unsigned long)(sizeof(int) *npalin) ); //pomocna pole
  if(IB == NULL) goto freeIG;
  Origpalin= farmalloc( (unsigned long)(3 *npalin) ); //pomocna pole
  if(Origpalin== NULL) goto freeIB;
  Dist= farmalloc( (unsigned long)(sizeof(unsigned long) *npalin) ); //pomocna pole
  if(Dist == NULL) goto freeOrig;
  Soused= farmalloc( (unsigned long)(sizeof(int) *npalin) ); //pomocna pole
  if(Soused == NULL) goto freeDist;
  Order= farmalloc( (unsigned long)(sizeof(int) *npalin) ); //pomocna pole
  if(Order == NULL) goto freeSoused;
  Multi= farmalloc( (unsigned long)(sizeof(int) *npalin) ); //pomocna pole
  if(Multi== NULL) goto freeOrder;
  savecols= farmalloc( (unsigned long) npalin ); //pomocna pole
  if(savecols== NULL) goto freeMulti;

kmax=-1; kmaxmin= 9999;
for(jin=0,iout=0, j=0; j < multip; j++)
{
  npal1= Npalin[j];
  palin= Palin[j];
  for(iin=0, i1=0; i1 < npal1; i1++)  // cykl pres 1 vstupni paletu >>>
  {
    ir=IR[jin]= Origpalin[iout++]= palin[iin++];  //pocatecni nastaveni poli
    ig=IG[jin]= Origpalin[iout++]= palin[iin++];  //"stejnolehlych" s palin
    ib=IB[jin]= Origpalin[iout++]= palin[iin++];

    Dist[jin]= (ig<<16)|(ir<<8)|ib; //pro pomocne trideni
    ksummin= ir+ig+ib;//urcovani nejtmavsi a nejsvetlejsi barvy:
    if(ksummin < kmaxmin){kmaxmin=ksummin;iblack=jin;}
    if(ksummin > kmax)   {kmax=ksummin;   iwhite=jin;}

    jin++;
  }  //konec cyklu pres 1 vstupni paletu
}   //konec cyklu pres <multi> vstupnich palet
memset(Multi,0,(sizeof(int) * npalin));

  npal1= npalin2= npalin; //npalin2= delka pracovni (zkracovane) palety
  multi0=0; Tol1=1;       //pomocne, ridici promenne
//======= Osetreni nejtmavsi a nejsvetlejsi barvy >>>
//p:  iblack=INearCol( Origpalin, npalin, '\0','\0','\0');
//p:  iwhite=INearCol( Origpalin, npalin, '\63','\63','\63'); //63?
  nsavecols=0;
  if(Savecols == NULL)
  {
    memset(savecols,0,npalin);  //nahradni savecols
    savecols[iblack]= 1;
    savecols[iwhite]= 1;
  }
  else    //ohledani ex. Savecols
  {
    memcpy(savecols, Savecols, npalin);
    for(jin=0; jin < npalin; jin++)    // cykl pres vstupni paletu >>>
    {
      if(Savecols[jin] != 0) nsavecols++;
    }
    if(nsavecols > *npalout) return 2; //test pripustnosti
  }
 if(multip==2) //spec.pripad
 { //npal2= Npalin[1];
   if(nsavecols==Npalin[0] && npal2==1 && Npalin[0]<= *npalout) goto Merge1;
 }
Mereni:  //=========================================== Mereni vzdalenostii >>>
//pomocne poradi pro hledani nejblizsich sousedu:
  j1=TagSort( Dist, Order, 0, npalin2-1);//
  if(j1 != 1){ireturn=10+j1;goto Konec;}  //!! test?
  for(jin=0; jin < npalin2; jin++)    // cykl pres pracovni paletu >>>
  {
    Dist[jin]=Moc;
  }
//P:  for(jin=0; jin < npalin2; jin++)    // cykl pres pracovni paletu >>>
  for(jin=1; jin < npalin2-1; jin++)    // cykl pres pracovni paletu >>>
  {
    iin = Order[jin];
    kmaxmin=(int)Dist[iin];
    if(kmaxmin == 0) continue;
    ksummin=kmaxmin;
    jmin= Soused[iin];
    ir= IR[iin];
    ig= IG[iin];
    ib= IB[iin];
    s1=savecols[iin];
// ========================= hledani nejblizsi barvy ve vst. palete >>>>
//P: for(jout=0; jout < npalin2; jout++)  //cykl pro jin+1 .. npalin
    jlev=jprav=jin;
Doleva:
    if(--jlev<0) goto Doprava;
    jout=  Order[jlev]; //"levy soused"
    if(s1 && (savecols[jout] != 0) ) goto Doleva; //obe chranene
    kr2= abs(ir - IR[jout]);  // diference slozek
    if(kr2 > kmaxmin) goto Doprava;
    kg2= abs(ig - IG[jout]);
    if(kg2 > kmaxmin) goto Doprava;
    kb2= abs(ib - IB[jout]);
    if(kb2 > kmaxmin) goto Doprava;

    kmax= max( kr2, max(kg2, kb2) );    // 1.metrika
    if(kmax == 0)
           {jmin=jout; ksummin=0; goto Hotovo;}
    if(kmax == kmaxmin)
    {
       ksum= kr2 + kg2 + kb2;   // 2.metrika
       if(ksum >= ksummin) goto Doprava;
       jmin=jout;
       kmaxmin=kmax;
       ksummin= ksum;
    }
    else
    {
       jmin=jout;
       kmaxmin=kmax;
       ksummin= kr2 + kg2 + kb2;   // 2.metrika
       goto Doleva;
    }
Doprava:
    if(++jprav>=npalin2) goto Hotovo;
    jout=  Order[jprav]; //"levy soused"
    if(s1 && (savecols[jout] != 0) ) goto Doprava; //obe chranene
    kr2= abs(ir - IR[jout]);  // diference slozek
    if(kr2 > kmaxmin) goto Hotovo;
    kg2= abs(ig - IG[jout]);
    if(kg2 > kmaxmin) goto Hotovo;
    kb2= abs(ib - IB[jout]);
    if(kb2 > kmaxmin) goto Hotovo;

    kmax= max( kr2, max(kg2, kb2) );    // 1.metrika
    if(kmax == 0)
           {jmin=jout; ksummin=0; goto Hotovo;}
    if(kmax == kmaxmin)
    {
       ksum= kr2 + kg2 + kb2;   // 2.metrika
       if(ksum >= ksummin) goto Hotovo;
       jmin=jout;
       kmaxmin=kmax;
       ksummin= ksum;
    }
    else
    {
       jmin=jout;
       kmaxmin=kmax;
       ksummin= kr2 + kg2 + kb2;   // 2.metrika
       goto Doprava;
    }
/*P: for(jout=0; jout < npalin2; jout++)  //cykl pro jin+1 .. npalin
    {
      if(jout == jin) continue;
      if(s1 && (savecols[jout] != 0) ) continue; //obe chranene

      kr2= abs(ir - IR[jout]);  // diference slozek
      if(kr2 > kmaxmin) continue;
      kg2= abs(ig - IG[jout]);
      if(kg2 > kmaxmin) continue;
      kb2= abs(ib - IB[jout]);
      if(kb2 > kmaxmin) continue;

      kmax= max( kr2, max(kg2, kb2) );  // 1.metrika
//p:      if(kmax > kmaxmin) continue;
      if(kmax == 0)
        {jmin=jout; ksummin=0; goto Hotovo;}
      if(kmax == kmaxmin)
      {
        ksum= kr2 + kg2 + kb2;  // 2.metrika
        if(ksum >= ksummin) continue;
        jmin=jout;
        kmaxmin=kmax;
        ksummin= ksum;
      }
      else
      {
        jmin=jout;
        kmaxmin=kmax;
        ksummin= kr2 + kg2 + kb2;   // 2.metrika
      }
    } //konec 2.cyklu pres pracovni paletu  :P*/
Hotovo:
    Soused[iin]= jmin;
    Dist  [iin]= ksummin;
    if((int)Dist[jmin] > ksummin)
    { Soused[jmin]= iin;
      Dist  [jmin]= ksummin;
    }
kon1:;
  } //konec cyklu pres pracovni paletu

  j1=TagSort( Dist, Order, 0, npalin2-1);
  if(j1 != 1){ireturn=10+j1;goto Konec;}  //!! test?

//========================= Zkracovani palety =======>>>
  n1stop=npal1 - (npalin2 / 2);
Pruchod:
  ndelta=0;
  jin  = npalin2; //ukazovatko do Order
//p:?  jkonec= npalin-npalin2;
  jkonec=0;
Dalsi:                    //======= zac. cyklu pres Order >>
  jin--;
  if(jin < jkonec)
     goto Zaver; //konec jednoho pruchodu
  iin = Order[jin];
//  if(iin < 0) goto Dalsi; //pouzita dvojice
  if(Dist[iin] > Tolerance && npal1 <= *npalout)
     goto Zaver;
  if(Multi[iin] > multi0) goto Dalsi;
  if(Dist[iin] > Tol1)
     goto Zaver;
  iout= Soused[iin];
  if(Multi[iout] > multi0) goto Dalsi; //Novinka PresspalN !
// ================== vlastni slouceni jedne dvojice ========
  if(savecols[iout] != 0) goto Sluc; // {chranena,nechranena}
  if(savecols[iin] != 0)
    {j=iout; iout=iin; iin=j; goto Sluc;}
  switch(TypFuse )
  {
case 0: if(iout > iin) {j=iout; iout=iin; iin=j;}  //iout= nizsi i
        break;
case 1: IR[iout]= (IR[iout] + IR[iin] +1) / 2;    //prumer 2 barev
        IG[iout]= (IG[iout] + IG[iin] +1) / 2;
        IB[iout]= (IB[iout] + IB[iin] +1) / 2;
        break;
case 2:
        if(iout < iin) {j=iout; iout=iin; iin=j;}  //iout= vyssi i
default:
        break;
  }
Sluc:
//puv:  mapio[iin]= iout;
  Multi[iin]   = Moc;
  Multi[iout] += 1;
  npal1--; ndelta++;
//  Order[jin]= -1;  //priznak pouzite dvojice
  if(npal1 >2) goto Dalsi;
Zaver:     //======================= Zaver =================
//---------- vyhodnoceni uspesnosti ----->
  if(npal1 > *npalout)
   {
     multi0 +=1; if(Tol1<189) Tol1 +=3;  //p:Tol1 *=2;
     if(ndelta > 0 && npal1 > n1stop)
          goto Pruchod; //dalsi pruchod!
   }
Setres:   //---- setreseni palety --->
  jin=0;
  while(Multi[jin] < Moc && jin < npalin2) jin++;
  for(iout=jin; jin < npalin2; jin++)    // cykl pres pracovni paletu >>>
  {
    if(Multi[jin] == Moc) continue;
    IR[iout]= IR[jin];
    IG[iout]= IG[jin];
    IB[iout]= IB[jin];
    Multi[iout]= Multi[jin];
    savecols[iout]= savecols[jin];
    iout++;
  }
  if( iout != npal1 ) return 8; //pro jistotu?
  npalin2= npal1;
  if(npal1 > *npalout) goto Mereni; //nove mereni vzdalenosti!

Zakonceni:         //============================ ZAKONCENI =========
  *npalout= npal1;
  ireturn= 1;  //dobry konec!!

//==================== Setrideni palout ===================>>>
  if(Savecols == NULL)
  {
    savecols[iblack]= 0;  //zruseni pomocne masky!
    savecols[iwhite]= 0;
  }
  npalin2=0;  //zde pocet "nechranenych" barev
  for(jin=0; jin < npal1; jin++)    // cykl pres pracovni paletu >>>
  {
    if(savecols[jin] != 0)
     { Dist[jin]= 0;  continue;} //Dist: unsigned long!!
    npalin2++;
    Dist[jin]  = IR[jin] + IG[jin] + IB[jin] +1; //intenzita pro SORT
  }
  if(npalin2>0)
  {
    j1=TagSort( Dist, Order, 0, npal1-1);
    if(j1 != 1){ireturn=20+j1;goto Konec;}  //!! test?
  }
                    //================= presun nove palety do palout >>>
 if(Savecols == NULL)
 {
  for(jin=npalin2-1,iout=0; jin >= 0; jin--)    // cykl pres pracovni paletu >>>
  {
    iin= Order[jin];
    palout[iout++]= IR[iin];
    palout[iout++]= IG[iin];
    palout[iout++]= IB[iin];
  }
 }
 else  //Chranene barvy:
 {
// 1. presun chranenych barev ktere mohou zustat na svem miste:
  memset(savecols,0,npal1);  // nove savecols
  iin=0;
  for(jin=0; jin < npal1; jin++)    // cykl pres pracovni paletu >>>
  {
    if(Savecols[jin] != 0)
    {
       palout[iin]= Origpalin[iin];
       palout[iin+1]= Origpalin[iin+1];
       palout[iin+2]= Origpalin[iin+2];
       savecols[jin]=1;
       nsavecols--;
    }
    iin +=3;
  }
  jkonec= npal1;//?p: jin;
// 2. presun nechranenych barev:
  jout=0; iout=0;
  for(jin=npalin2-1; jin >= 0; jin--)    // cykl pres pracovni paletu >>>
  {
    while(savecols[jout] != 0) {jout++; iout +=3;}
    iin= Order[jin];
    palout[iout  ]= IR[iin];
    palout[iout+1]= IG[iin];
    palout[iout+2]= IB[iin];
    jout++; iout+=3;
  }
//3. presun zbyvajicich chranenych barev:
  iin= 3 * jkonec;
  while(nsavecols > 0)
  {
    if(Savecols[jkonec] != 0)
    {
      while(savecols[jout] != 0) {jout++; iout +=3;}
      palout[iout++]= Origpalin[iin];
      palout[iout++]= Origpalin[iin+1];
      palout[iout++]= Origpalin[iin+2];
      jout++;
      nsavecols--;
    }
    jkonec++; iin +=3;
  }
  if(jkonec > npalin || jout > npal1) return 6; //pro jistotu
 } //konec vytvareni palout pri chranenych barvach
//-------- uvolneni dale nepotrebnych poli -->
farfree(savecols);   savecols=NULL;
farfree(Multi);   Multi=NULL;
farfree(Order);   Order=NULL;
farfree(Soused);  Soused=NULL;
farfree(Dist);    Dist=NULL;
farfree(IB);      IB=NULL;
farfree(IG);      IG=NULL;
farfree(IR);      IR=NULL;
/* ===================  naplneni mapio ================
*mapio  - prevodni mapa - vystupni parametr
          ==> j = mapio[i] ... i= vstupni barva, j = vystupni barva
 metoda - metoda porovnavani barev: 'R' = metoda RGB
                                    'B' = metoda BSI
*/
Ipalpal (Origpalin, npalin, palout, *npalout, mapio);

// ===================  naplneni Mmapio ================
Mmap:
  if(Mmapio == NULL) goto Konec;
  for(i1=0, j=0; j<multip; j++)
  {
     Mmapio[j]= &mapio[i1];
     npal1 = Npalin[j];
     i1 += npal1;
  }

Konec:
            if(savecols!=NULL) farfree(savecols);
freeMulti:  if(Multi!=NULL) farfree(Multi);
freeOrder:  if(Order!=NULL) farfree(Order);
freeSoused: if(Soused!=NULL) farfree(Soused);
freeDist:   if(Dist!=NULL) farfree(Dist);
freeOrig:   if(Origpalin!=NULL) farfree(Origpalin);
freeIB:     if(IB!=NULL) farfree(IB);
freeIG:     if(IG!=NULL) farfree(IG);
freeIR:     if(IR!=NULL) farfree(IR);

  return ireturn;
//----------------- Osetreni c/b palety ---- 970416...---------------
BlackWhite:
 IR=IG=IB=Multi=Soused=NULL;
 savecols=Origpalin= NULL;
 Dist= farmalloc( (unsigned long)(4 *npalin) ); //pomocna pole
 if(Dist == NULL) return 4;
 Order= farmalloc( (unsigned long)(2 *npalin) ); //pomocna pole
 if(Order == NULL) { farfree(Dist); return 4;
                   }
 for(jin=0,iout=0, j=0; j < multip; j++)
{
  npal1= Npalin[j];
  palin= Palin[j];
  for(iin=0, i1=0; i1 < npal1; i1++)  // cykl pres 1 vstupni paletu >>>
  {
    ir= palin[iin++];  //pocatecni nastaveni poli
    ig= palin[iin++];  //"stejnolehlych" s palin
    ib= palin[iin++];

    Dist[jin]= 5 * ig + 3 *ir + 2 * ib; //Vazene intezity
    jin++;
  }  //konec cyklu pres 1 vstupni paletu
}   //konec cyklu pres <multi> vstupnich palet
//pomocne poradi dle intenzit: (sestupne!)
  j1=TagSort( Dist, Order, 0, npalin-1);//
  if(j1 != 1){ireturn=10+j1;goto Konec;}  //!! test?
// Vyst. paleta:
  palout[0]= palout[1]= palout[2]= 0;
  palout[3]= palout[4]= palout[5]= 63;

  iin = Order[npalin-1];
  mapio[iin]= 0; //1. a nejtmavsi barva!
  jin=npalin-2;     // cykl pres vst.paletu >>>
  while( jin >0 )    // povinne cerne barvy>
  {
    iin = Order[jin];
    if(Dist[iin] > LOWLIM) break;
    mapio[iin]= 0;
    jin--;
  }
  j=0;//"oscilujici barva"
  while( jin >=0 )
  {
    iin = Order[jin];
    if(Dist[iin] > HIGHLIM) j=1;// povinne bile barvy>
    else j= 1 - j;
    mapio[iin]= j;
    jin--;
  }
  mapio[iin]= 1;//posledni, nejsvetlejsi barva!
  ireturn=1;
  goto Mmap;
Merge1://----------- spec. pripad - pridavani 1 barvy!! -----------------
  npal1= Npalin[0];
  palin= Palin[1];
  ir = palin[0];
  ig = palin[1];
  ib = palin[2];
  palin= Palin[0];
  j = 0;
  kmaxmin = 100;
  ksummin = 200;   //? unsigned? !
  for( i1 = 0; i1 < npal1; i1++ )//cykl pres 1. paletu
  {
    mapio[i1]=i1;
    kr2 = ir - IR[i1];  // diference slozek
    kg2 = ig - IG[i1];
    kb2 = ib - IB[i1];
    if (kr2  < 0) kr2 = -kr2;
    if (kg2  < 0) kg2 = -kg2;
    if (kb2  < 0) kb2 = -kb2;
/*   KMAX= MAX( KR2, KG2, KB2 ) ! 1.metrika */
    kmax = kr2 > kg2 ? kr2 : kg2;
    kmax = kmax > kb2 ? kmax : kb2;
//jiz nebude:    if( kmax == 0) {jmin = i1; break;}
    if( kmax > kmaxmin ) continue;
    ksum = kr2 + kg2 + kb2;
    if( kmax == kmaxmin )
    {
      if( ksum >= ksummin ) continue;
    }
    jmin = i1; kmaxmin = kmax; ksummin=ksum;
    if(ksum<=Tolerance) break;//!! nepatlame se s tim!
  }//end cykl pres npal1

  while( i1 < npal1)
    { mapio[i1]=i1++;}//dokonceni mapio
//================= presun 1.palety do palout >>>
  jout= 3 * npal1;
  memmove(palout, palin, jout);

  if(ksummin<=Tolerance || npal1 >= *npalout)
  { mapio[npal1]= jmin;  //zarazeni dovnitr 1.palety
    *npalout= npal1;
  }
  else
  { mapio[i1]= npal1;  //pridani za 1.paletu
    palout[jout++]= ir;
    palout[jout++]= ig;
    palout[jout]= ib;
    *npalout= npal1+1;
  }
  ireturn=1;
  goto Mmap;
}
