//. This module is not included in Arachne.prj

// Funkce pro diakritiku byla prevzata z IBTEXTU, (c)1994 Ivan Polak
 //
int MIbkey(int key)
{

int znak, i, hakcarka;//, lastkbd=0;
char *ptr1;

char klav5[50]="zZyY1234567890-=\[];',./~!@#$%^&*()_+{}:\"<>?";
char klav2[50]="zZyY+à®á©ëò†°Ç-`\[];',./~!@#$%^&*()_~{}:\"<>?";
char klav9[50]="zZyY!à®á©ëò†°Ç-`\£=ñ;,./~1234567890_~():\"<>?";
char klav1[50]="yYzZ+à®á©ëò†°Ç=`\£)ñ≠,.-;1234567890%~/(\"!?:_";
char klav3[50]="yYzZ+å®áüëò†°Ç=`\£Ñì),.-~1234567890%~/(\"!?:_";
char klav6[50]="yYzZ1234567890µ=\Å+îÑ,.-~!\"≠$%&/()=?+ö*ôé;:_";


Start:
znak= key & 0xFF;
if(znak==0)
{
//osetreni Alt/1,2,3,5,6,9 :
//  lastkbd=kbd;
  switch(key)
  {
  case 0x7800: kbd=1; break; //<Alt>1
  case 0x7900: kbd=2; break; //<Alt>2
  case 0x7A00: kbd=3; break; //<Alt>3
  case 0x7C00: kbd=5; break; //<Alt>5
  case 0x7D00: kbd=6; break; //<Alt>6
  case 0x8000: kbd=9; break; //<Alt>9
  }
//  if (lastkbd!=kbd) redraw(0);
  return key;
}

Ostatni:
if(kbd == 5) return key;  //klav "IBM"

if((key & 0xFF00) == 0) return key;  //ctrl/...

if(kbd != 6)
{
switch(znak)
{
  case '+': if(key==0x4E2B) break;  //Keypad '+'
  /*case '~':*/
//  message("h†áek:","e->à s->® c->á r->© z->ë t->ü u->ñ o->ì a->Ñ d->É l->å n->§");
  hakcarka=1; goto Ctidalsi;     //Hacek
  case'=':
  /*case'`'*/
  hakcarka=0;
//  message("á†rka:","e->Ç r->™ y->ò u->£ i->° o->¢ a->† l->ç");
Ctidalsi:
// _setcursortype(_NOCURSOR);
//  mouseoff();
//  key= bioskey(0);

  do key= bioskey(1); while(key==0);
  key=bioskey(0);
//  *status=bioskey(2);
   znak= key & 0xFF;
//upozorneni "hacek/carka"  !!??----------
//--------- hackovani --------------
//  redraw(0);
//  mouseon();
  if((znak= MKamenik( znak, hakcarka)) == 0)
  {
//  _setcursortype(_NOCURSOR);
   putch(7);
   return(0);
  }
//  znak= (znak & 0xFF) | 0x01FF;  //???!!!
 return znak;
default:;
}
}
//---------- osetreni nestandardnich klaves predvolene klavesnice:
//?   if(znak>='0' && znak=<'9' && key > 0x4700) return key;//Keypad cislice
   if(key > 0x3500) return key;//Keypad znaky
   ptr1= strchr(klav5, znak);
   if(ptr1 != NULL)
   {
    i=(int)(ptr1 - klav5);
    switch(kbd)
    {
case 1:  ptr1=klav1; break;
case 2:  ptr1=klav2; break;
case 3:  ptr1=klav3; break;
case 6:  ptr1=klav6; break;
case 9:  ptr1=klav9; break;
    }
    key= ptr1[i];
//    key= (key & 0xFF) | 0x01FF;  //???!!!
   }
   return key;
}
/***********************************************************************/
/*                  Funkce pro diakritiku klavesnice                   */
/***********************************************************************/
/*  znak= vstupni znak
 hakcarka= 0/1 : carka/hacek
 return: novy znak, dle Kamenickych
         0 - pri chybe (znak, nad ktery nelze dat hacek/carku)
*/
int MKamenik( int znak, int hakcarka)
{
  const char pcar[2*POCCAR] =
{ 'e','y','u','i','o','a','E','Y','U','I','O','A','l','r','L','R',
  'Ç','ò','£','°','¢','†','ê','ù','ó','ã','ï','è','ç','™','ä','´' };
  const char phak[2*POCHAK] =
{ 'e','r','t','u','s','d','z','c','n','E','R','T','U','S','D','Z','C','N',
  'a','l','o','A','L','O',
  'à','©','ü','ñ','®','É','ë','á','§','â','û','Ü','¶','õ','Ö','í','Ä','•',
  'Ñ','å','ì','é','ú','ß'};
  int  i;

Pismeno:
//  key = bioskey(0);
//  i = bioskey(2);
  if ( !hakcarka)  //carka
  {
    for ( i = 0; i < POCCAR; i++ )
      if ( pcar[i] == znak ) return( (int) pcar[i+POCCAR] );
    goto Chyba;
  }
  else
  {
    for ( i = 0; i < POCHAK; i++ )
      if ( phak[i] == znak) return( (int) phak[i+POCHAK] );
  }
Chyba: return 0;
}
