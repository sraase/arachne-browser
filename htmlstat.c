
// ========================================================================
// HTML rendering routines for Arachne WWW browser, statical part
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "html.h"
#include "xanimgif.h"

// ========================================================================
// Fast arguments for HTML tags.
// ========================================================================


// getvar() reads value of variable of given name
int getvar(char *name,char **value)
{
 int i=0,nameidx=0,valueidx=0;

 while(i<argnamecount)
 {
  if(!strcmpi(argnamestr+nameidx+sizeof(int),name))
  {
   *value=argvaluestr+valueidx+sizeof(int);
   return 1;
  }

  nameidx+=*(int*)(&argnamestr[nameidx]);
  valueidx+=*(int*)(&argvaluestr[valueidx]);
  i++;
 }

 return 0;
}

// Alternative to getvar()              [JdS 2004/1/28]
// getarg() reads value of a "variable" of a given name
int getarg(char *name,char **value)
{
 int i=0;

 while(i<argvarcount)
 {
  if(!strcmpi(argnameptr[i],name))
  {
   *value=argvalueptr[i];
   return 1;
  }
  i++;
 }

 return 0;
}

int searchvar(char *name)
{
 int i=0,nameidx=0;
 int l=strlen(name);

 while(i<argnamecount)
 {
  if(!strncmpi(argnamestr+nameidx+sizeof(int),name,l))
   return 1;
  nameidx+=*(int*)(&argnamestr[nameidx]);
  i++;
 }
 return 0;
}

// Alternative to searchvar()            [JdS 2004/1/28]
// searcharg() searches for a "variable" of a given name
int searcharg(char *name)
{
 int i=0, l=strlen(name);

 while(i<argvarcount)
 {
  if(!strncmpi(argnameptr[i],name,l))
   return 1;
  i++;
 }

 return 0;
}

// putvarname() sets new variable name
void putvarname(char *name,int size)
{
 int idx=0,i=argnamecount;

 while(i--)
 {
  idx+=*(int*)(&argnamestr[idx]);
 }//loop

 size+=1+sizeof(int);
 if(idx+size<MAXARGNAMES)
 {
  memcpy(argnamestr+idx,&size,sizeof(int));
  memcpy(argnamestr+idx+sizeof(int),name,size-1-sizeof(int));
  argnamestr[idx+size-1]='\0';
  argnamecount++;
 }
}

// Alternative to putvarname()   [JdS 2004/1/28]
// putargname() sets new "variable" name pointer
// Needs to be followed by putargvalue() to update argvarcount
void putargname(char *name)
{
 if(argvarcount<MAXARGS)
 {
  argnameptr[argvarcount]=name;
 }
}

// putvarvalue() sets new variable value (putvarname must be called first)
void putvarvalue(char *value,int size)
{
 int idx=0,i=argvaluecount;

 while(i--)
 {
  idx+=*(int*)(&argvaluestr[idx]);
 }//loop

 size+=1+sizeof(int);
 if(idx+size<BUF/4)
 {
  memcpy(argvaluestr+idx,&size,sizeof(int));
  memcpy(argvaluestr+idx+sizeof(int),value,size-1-sizeof(int));
  argvaluestr[idx+size-1]='\0';
  argvaluecount++;
 }

}

// Alternative to putvarvalue()    [JdS 2004/1/28]
// putargvalue() sets new "variable" value pointer
void putargvalue(char *value)
{
 if(argvarcount<MAXARGS)
 {
  argvalueptr[argvarcount]=value;
  argvarcount++;
 }
}

//converts hexadecimal character to decimal value
char hexdigit(char c)
{
 if(c>='0' && c<='9') return c-'0';
 if(c>='A' && c<='F') return 10+c-'A';
 return 0;
}

/* list of named colors */
static const struct color {
	char *name;
	unsigned char r;
	unsigned char g;
	unsigned char b;
} far colors[] = {
	/* standard 16 colors + orange */
	{ "aqua",    0x00, 0xFF, 0xFF, },
	{ "black",   0x00, 0x00, 0x00, },
	{ "blue",    0x00, 0x00, 0xFF, },
	{ "fuchsia", 0xFF, 0x00, 0xFF, },
	{ "gray",    0x80, 0x80, 0x80, },
	{ "green",   0x00, 0x80, 0x00, },
	{ "grey",    0x80, 0x80, 0x80, },
	{ "lime",    0x00, 0xFF, 0x00, },
	{ "maroon",  0x80, 0x00, 0x00, },
	{ "navy",    0x00, 0x00, 0x80, },
	{ "olive",   0x80, 0x80, 0x00, },
	{ "orange",  0xFF, 0xA5, 0x00, },
	{ "purple",  0x80, 0x00, 0x80, },
	{ "red",     0xFF, 0x00, 0x00, },
	{ "silver",  0xC0, 0xC0, 0xC0, },
	{ "teal",    0x00, 0x80, 0x80, },
	{ "white",   0xFF, 0xFF, 0xFF, },
	{ "yellow",  0xFF, 0xFF, 0x00, },

	/* remaining extended colors */
	{ "aliceblue",            0xF0, 0xF8, 0xFF, },
	{ "antiquewhite",         0xFA, 0xEB, 0xD7, },
	{ "aquamarine",           0x7F, 0xFF, 0xD4, },
	{ "azure",                0xF0, 0xFF, 0xFF, },
	{ "beige",                0xF5, 0xF5, 0xDC, },
	{ "bisque",               0xFF, 0xE4, 0xC4, },
	{ "blanchedalmond",       0xFF, 0xEB, 0xCD, },
	{ "blueviolet",           0x8A, 0x2B, 0xE2, },
	{ "brown",                0xA5, 0x2A, 0x2A, },
	{ "burlywood",            0xDE, 0xB8, 0x87, },
	{ "cadetblue",            0x5F, 0x9E, 0xA0, },
	{ "chartreuse",           0x7F, 0xFF, 0x00, },
	{ "chocolate",            0xD2, 0x69, 0x1E, },
	{ "coral",                0xFF, 0x7F, 0x50, },
	{ "cornflowerblue",       0x64, 0x95, 0xED, },
	{ "cornsilk",             0xFF, 0xF8, 0xDC, },
	{ "crimson",              0xDC, 0x14, 0x3C, },
	{ "cyan",                 0x00, 0xFF, 0xFF, },
	{ "darkblue",             0x00, 0x00, 0x8B, },
	{ "darkcyan",             0x00, 0x8B, 0x8B, },
	{ "darkgoldenrod",        0xB8, 0x86, 0x0B, },
	{ "darkgray",             0xA9, 0xA9, 0xA9, },
	{ "darkgreen",            0x00, 0x64, 0x00, },
	{ "darkkhaki",            0xBD, 0xB7, 0x6B, },
	{ "darkmagenta",          0x8B, 0x00, 0x8B, },
	{ "darkolivegreen",       0x55, 0x6B, 0x2F, },
	{ "darkorange",           0xFF, 0x8C, 0x00, },
	{ "darkorchid",           0x99, 0x32, 0xCC, },
	{ "darkred",              0x8B, 0x00, 0x00, },
	{ "darksalmon",           0xE9, 0x96, 0x7A, },
	{ "darkseagreen",         0x8F, 0xBC, 0x8F, },
	{ "darkslateblue",        0x48, 0x3D, 0x8B, },
	{ "darkslategray",        0x2F, 0x4F, 0x4F, },
	{ "darkturquoise",        0x00, 0xCE, 0xD1, },
	{ "darkviolet",           0x94, 0x00, 0xD3, },
	{ "deeppink",             0xFF, 0x14, 0x93, },
	{ "deepskyblue",          0x00, 0xBF, 0xFF, },
	{ "dimgray",              0x69, 0x69, 0x69, },
	{ "dodgerblue",           0x1E, 0x90, 0xFF, },
	{ "firebrick",            0xB2, 0x22, 0x22, },
	{ "floralwhite",          0xFF, 0xFA, 0xF0, },
	{ "forestgreen",          0x22, 0x8B, 0x22, },
	{ "gainsboro",            0xDC, 0xDC, 0xDC, },
	{ "ghostwhite",           0xF8, 0xF8, 0xFF, },
	{ "gold",                 0xFF, 0xD7, 0x00, },
	{ "goldenrod",            0xDA, 0xA5, 0x20, },
	{ "greenyellow",          0xAD, 0xFF, 0x2F, },
	{ "honeydew",             0xF0, 0xFF, 0xF0, },
	{ "hotpink",              0xFF, 0x69, 0xB4, },
	{ "indianred",            0xCD, 0x5C, 0x5C, },
	{ "indigo",               0x4B, 0x00, 0x82, },
	{ "ivory",                0xFF, 0xFF, 0xF0, },
	{ "khaki",                0xF0, 0xE6, 0x8C, },
	{ "lavender",             0xE6, 0xE6, 0xFA, },
	{ "lavenderblush",        0xFF, 0xF0, 0xF5, },
	{ "lawngreen",            0x7C, 0xFC, 0x00, },
	{ "lemonchiffon",         0xFF, 0xFA, 0xCD, },
	{ "lightblue",            0xAD, 0xD8, 0xE6, },
	{ "lightcoral",           0xF0, 0x80, 0x80, },
	{ "lightcyan",            0xE0, 0xFF, 0xFF, },
	{ "lightgoldenrodyellow", 0xFA, 0xFA, 0xD2, },
	{ "lightgray",            0xD3, 0xD3, 0xD3, },
	{ "lightgreen",           0x90, 0xEE, 0x90, },
	{ "lightpink",            0xFF, 0xB6, 0xC1, },
	{ "lightsalmon",          0xFF, 0xA0, 0x7A, },
	{ "lightseagreen",        0x20, 0xB2, 0xAA, },
	{ "lightskyblue",         0x87, 0xCE, 0xFA, },
	{ "lightslategray",       0x77, 0x88, 0x99, },
	{ "lightsteelblue",       0xB0, 0xC4, 0xDE, },
	{ "lightyellow",          0xFF, 0xFF, 0xE0, },
	{ "limegreen",            0x32, 0xCD, 0x32, },
	{ "linen",                0xFA, 0xF0, 0xE6, },
	{ "magenta",              0xFF, 0x00, 0xFF, },
	{ "mediumaquamarine",     0x66, 0xCD, 0xAA, },
	{ "mediumblue",           0x00, 0x00, 0xCD, },
	{ "mediumorchid",         0xBA, 0x55, 0xD3, },
	{ "mediumpurple",         0x93, 0x70, 0xDB, },
	{ "mediumseagreen",       0x3C, 0xB3, 0x71, },
	{ "mediumslateblue",      0x7B, 0x68, 0xEE, },
	{ "mediumspringgreen",    0x00, 0xFA, 0x9A, },
	{ "mediumturquoise",      0x48, 0xD1, 0xCC, },
	{ "mediumvioletred",      0xC7, 0x15, 0x85, },
	{ "midnightblue",         0x19, 0x19, 0x70, },
	{ "mintcream",            0xF5, 0xFF, 0xFA, },
	{ "mistyrose",            0xFF, 0xE4, 0xE1, },
	{ "moccasin",             0xFF, 0xE4, 0xB5, },
	{ "navajowhite",          0xFF, 0xDE, 0xAD, },
	{ "oldlace",              0xFD, 0xF5, 0xE6, },
	{ "olivedrab",            0x6B, 0x8E, 0x23, },
	{ "orangered",            0xFF, 0x45, 0x00, },
	{ "orchid",               0xDA, 0x70, 0xD6, },
	{ "palegoldenrod",        0xEE, 0xE8, 0xAA, },
	{ "palegreen",            0x98, 0xFB, 0x98, },
	{ "paleturquoise",        0xAF, 0xEE, 0xEE, },
	{ "palevioletred",        0xDB, 0x70, 0x93, },
	{ "papayawhip",           0xFF, 0xEF, 0xD5, },
	{ "peachpuff",            0xFF, 0xDA, 0xB9, },
	{ "peru",                 0xCD, 0x85, 0x3F, },
	{ "pink",                 0xFF, 0xC0, 0xCB, },
	{ "plum",                 0xDD, 0xA0, 0xDD, },
	{ "powderblue",           0xB0, 0xE0, 0xE6, },
	{ "rosybrown",            0xBC, 0x8F, 0x8F, },
	{ "royalblue",            0x41, 0x69, 0xE1, },
	{ "saddlebrown",          0x8B, 0x45, 0x13, },
	{ "salmon",               0xFA, 0x80, 0x72, },
	{ "sandybrown",           0xF4, 0xA4, 0x60, },
	{ "seagreen",             0x2E, 0x8B, 0x57, },
	{ "seashell",             0xFF, 0xF5, 0xEE, },
	{ "sienna",               0xA0, 0x52, 0x2D, },
	{ "skyblue",              0x87, 0xCE, 0xEB, },
	{ "slateblue",            0x6A, 0x5A, 0xCD, },
	{ "slategray",            0x70, 0x80, 0x90, },
	{ "snow",                 0xFF, 0xFA, 0xFA, },
	{ "springgreen",          0x00, 0xFF, 0x7F, },
	{ "steelblue",            0x46, 0x82, 0xB4, },
	{ "tan",                  0xD2, 0xB4, 0x8C, },
	{ "thistle",              0xD8, 0xBF, 0xD8, },
	{ "tomato",               0xFF, 0x63, 0x47, },
	{ "turquoise",            0x40, 0xE0, 0xD0, },
	{ "violet",               0xEE, 0x82, 0xEE, },
	{ "wheat",                0xF5, 0xDE, 0xB3, },
	{ "whitesmoke",           0xF5, 0xF5, 0xF5, },
	{ "yellowgreen",          0x9A, 0xCD, 0x32, },

	/* end-of-list */
	{ NULL, 0x00, 0x00, 0x00 },
};

/* read HTML color in #RGB or #RRGGBB format,
   or from the list of named colors */
void try2readHTMLcolor(const char *str,
	unsigned char *r,unsigned char *g,unsigned char *b)
{
	static char buf[8];
	const struct color far *c = colors;
	char *ptr = buf;

	/* initialize parse buffer */
	memset(buf, 0, sizeof(buf));
	strncpy(buf, str, 7);
	strupr(buf);

	/* designated hex string */
	if (*ptr == '#') {
		ptr++;
		goto number;
	}

	/* named color */
	while (c->name) {
		if (!strcmpi(str, c->name)) {
			*r = c->r;
			*g = c->g;
			*b = c->b;
			return;
		}
		c++;
	}

	/* fix for bad html */
	if (ptr[0] > 'F' || ptr[1] > 'F' || ptr[2] > 'F' ||
	    ptr[3] > 'F' || ptr[4] > 'F' || ptr[5] > 'F') {
		strcpy(ptr, "777777");
	}

number:
	/* hex string, either #RGB or #RRGGBB */
	if (ptr[3] < '0') {
		*r = 16 * hexdigit(ptr[0]);
		*g = 16 * hexdigit(ptr[1]);
		*b = 16 * hexdigit(ptr[2]);
	} else {
		*r = 16 * hexdigit(ptr[0]) + hexdigit(ptr[1]);
		*g = 16 * hexdigit(ptr[2]) + hexdigit(ptr[3]);
		*b = 16 * hexdigit(ptr[4]) + hexdigit(ptr[5]);
	}
	return;
}

//try2getnum converts HTML object metrics to number: pixels or percents
int try2getnum(char *str,unsigned proczaklad)
{
 int l;

 l=strlen(str);
 if(str[l-1]=='%')
 {
  str[l-1]='\0';
  return (int)( (long)((long)atoi(str)*(long)proczaklad) /100 );
 }
 else
 {
  return atoi(str);
 }
}

// ========================================================================
// fast HTML tag analysis
// ========================================================================

int FastTagDetect(char *tagname)
{
 char *ptr=&tagname[1];

 tagname[0]=toupper(tagname[0]);
 switch(tagname[0])
 {
  case '/':
  return 1000+FastTagDetect(ptr);

  case 'A':   //most frequent tag: <A HREF=...>
  if(!*ptr)
   return TAG_A;
  strupr(ptr);
  if(!strcmp(ptr,"REA"))
   return TAG_AREA;
  if(!strcmp(ptr,"DDRESS"))
   return TAG_I;
  if(!strcmp(ptr,"RACHNE"))
   return TAG_ARACHNE_BONUS;
//!!glennmcc: Aug 05, 2011 --- added support for HTML5 'AUDIO' & 'VIDEO'
  if(!strcmp(ptr,"UDIO"))
   return TAG_AUDIO;
  break;

  case 'T':   //second most frequend tag: <TABLE>, <TD>, etc.
  strupr(ptr);
  if(!strcmp(ptr,"D"))
   return TAG_TD;
  if(!strcmp(ptr,"R"))
   return TAG_TR;
  if(!strcmp(ptr,"H"))
   return TAG_TH;
  if(!strcmp(ptr,"T"))
   return TAG_TT;
  if(!strcmp(ptr,"ABLE"))
   return TAG_TABLE;
  if(!strcmp(ptr,"ITLE"))
   return TAG_TITLE;
  if(!strcmp(ptr,"EXTAREA"))
   return TAG_TEXTAREA;
  break;

  case 'B':
  if(!*ptr)
   return TAG_B;
  strupr(ptr);
  if(!strcmp(ptr,"R"))
   return TAG_BR;
  if(!strcmp(ptr,"IG"))
   return TAG_BIG;
  if(!strcmp(ptr,"ODY"))
   return TAG_BODY;
  if(!strcmp(ptr,"ASE"))
   return TAG_BASE;
  if(!strcmp(ptr,"ASEFONT"))
   return TAG_BASEFONT;
  if(!strcmp(ptr,"LOCKQUOTE"))
   return TAG_BLOCKQUOTE;
  if(!strcmp(ptr,"UTTON"))
   return TAG_BUTTON;
//!!glennmcc: Jan 19, 2003 --- added support for 'BGSOUND'
  if(!strcmp(ptr,"GSOUND"))
   return TAG_BGSOUND;
  break;

  case 'C':
  strupr(ptr);
  if(!strcmp(ptr,"ENTER"))
   return TAG_CENTER;
  if(!strcmp(ptr,"APTION"))
   return TAG_CAPTION;
  if(!strcmp(ptr,"ODE"))
   return TAG_CODE;
  if(!strcmp(ptr,"ITE"))
   return TAG_I;
  break;

  case 'D':
  strupr(ptr);
  if(!strcmp(ptr,"T"))
   return TAG_LI;
  if(!strcmp(ptr,"D"))
   return TAG_DD;
  if(!strcmp(ptr,"L") || !strcmp(ptr,"IR"))
   return TAG_OL;
  if(!strcmp(ptr,"IV"))
   return TAG_DIV;
//!!glennmcc: Feb 27, 2007 -- <S> has been depreciated to <DEL>
//therefore, we can treat <DEL> with the same action as <S> here
//<S> is actually 'strike'
//since our font set does not support 'strike',
//we should italicise <S> instead of treating it as <B>
//!!JDS: Feb 28, 2007 -- STRIKE now implemented (see code in htmldraw.c)
  if(!strcmp(ptr,"EL"))
   return TAG_S;
//!!glennmcc: end
  break;

  case 'E':
  strupr(ptr);
  if(!strcmp(ptr,"M"))
   return TAG_B;
  if(!strcmp(ptr,"MBED"))
   return TAG_EMBED;
  break;

  case 'F':
  strupr(ptr);
  if(!strcmp(ptr,"ONT"))
   return TAG_FONT;
  if(!strcmp(ptr,"ORM"))
   return TAG_FORM;
  if(!strcmp(ptr,"RAME"))
   return TAG_FRAME;
  if(!strcmp(ptr,"RAMESET"))
   return TAG_FRAMESET;
  break;

  case 'H':
  if(*ptr<'7' && *ptr>'0')
   return TAG_H1+*ptr-'1';
  strupr(ptr);
  if(!strcmp(ptr,"R"))
   return TAG_HR;
  if(!strcmp(ptr,"EAD"))
   return TAG_HEAD;
  break;

  case 'I':
  if(!*ptr)
   return TAG_I;
  strupr(ptr);
  if(!strcmp(ptr,"MG"))
   return TAG_IMG;
  if(!strcmp(ptr,"NPUT"))
   return TAG_INPUT;
//!!glennmcc: Feb 27, 2007 -- <U> has been depreciated to <INS>
//therefore, we can treat <INS> with the same action as <U> here
  if(!strcmp(ptr,"NS"))
   return TAG_U;
//!!glennmcc: end
//!!glennmcc: Mar 12, 2007 -- quick-n-dirty hack to support <iframe
//test page .... http://www.auschess.org.au/
  if(!strcmp(ptr,"FRAME"))
   return TAG_IFRAME;
//!!glennmcc: end
  break;

  case 'K':
  strupr(ptr);
  if(!strcmp(ptr,"BD"))
   return TAG_CODE;
  break;

  case 'L':
  strupr(ptr);
  if(!strcmp(ptr,"I"))
   return TAG_LI;
  if(!strcmp(ptr,"INK"))
   return TAG_LINK;
  break;

  case 'M':
  strupr(ptr);
  if(!strcmp(ptr,"AP"))
   return TAG_MAP;
  if(!strcmp(ptr,"ENU"))
   return TAG_OL;
  if(!strcmp(ptr,"ETA"))
   return TAG_META;
  break;

  case 'N':
  strupr(ptr);
  if(!strcmp(ptr,"OBR"))
   return TAG_NOBR;
  if(!strcmp(ptr,"OFRAMES"))
   return TAG_NOFRAMES;
  if(!strcmp(ptr,"OSCRIPT"))
   return TAG_NOSCRIPT;
  break;

  case 'O':
  strupr(ptr);
  if(!strcmp(ptr,"L"))
   return TAG_OL;
  if(!strcmp(ptr,"PTION"))
   return TAG_OPTION;
  break;

  case 'P':
  if(!*ptr)
   return TAG_P;
  strupr(ptr);
  if(!strcmp(ptr,"RE"))
   return TAG_PRE;
  break;

  case 'S':
//!!glennmcc: Feb 27, 2007 -- <S> is actually 'strike'
//since our font set does not support 'strike',
//we should italicise <S> instead of treating it as <B>
//!!JDS: Feb 28, 2007 -- STRIKE now implemented (see code in htmldraw.c)
  if(!*ptr)
   return TAG_S;
// return TAG_B;//original line
//!!glennmcc: end
  strupr(ptr);
//!!glennmcc: Aug 13, 2007 -- support <strike> in addition to <s>
  if(!strcmp(ptr,"TRIKE"))
   return TAG_S;
//!!glennmcc: end
  if(!strcmp(ptr,"ELECT"))
   return TAG_SELECT;
  if(!strcmp(ptr,"MALL"))
   return TAG_SMALL;
  if(!strcmp(ptr,"UP"))
   return TAG_SUP;
  if(!strcmp(ptr,"UB"))
   return TAG_SUB;
  if(!strcmp(ptr,"CRIPT"))
   return TAG_SCRIPT;
  if(!strcmp(ptr,"TYLE"))
   return TAG_STYLE;
  if(!strcmp(ptr,"TRONG"))
   return TAG_B;
  break;

  case 'U':
  if(!*ptr)
   return TAG_U;
  strupr(ptr);
  if(!strcmp(ptr,"L"))
   return TAG_UL;
  break;

//!!glennmcc: Aug 05, 2011 --- added support for HTML5 'AUDIO' & 'VIDEO'
  case 'V':
  strupr(ptr);
  if(!strcmp(ptr,"IDEO"))
   return TAG_VIDEO;
  break;
//!!glennmcc: end
 }
 return 0;
}

//close HTML atom == set coordinates of right bottom corner
char closeatom(XSWAP adr,int deltax,long absy)
{
 char expand=0;
 struct HTMLrecord *atomptr;


 if(adr!=IE_NULL)
 {
  atomptr=(struct HTMLrecord *)ie_getswap(adr);
  if(atomptr)
  {
   if(abs((int)atomptr->x+deltax-(int)atomptr->xx)>FUZZYPIX)
    expand=1;
   p->xsum+=(atomptr->x+deltax)-atomptr->xx;
   atomptr->xx=atomptr->x+deltax;
   if(absy) //0...don't overwrite!
    atomptr->yy=absy;
   swapmod=1;
  }
  else
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return NULL;
//   MALLOCERR();
//!!glennmcc: end
 }
 return expand;
}//end if


// find target frame for any HTML tag with TARGET attribute
// <A HREF=... TARGET=...>
int findtarget(int basetarget)
{
 int target=basetarget;
 char *tagarg;

 if(arachne.framescount && getvar("TARGET",&tagarg))
 {
  int fr=0;

  if(!strcmpi(tagarg,"_parent"))
   return p->htmlframe[p->currentframe].parent;

  do
  {
   if(!strncmpi(p->htmlframe[fr].framename,tagarg,39))
    target=fr;
  }
  while(fr++<arachne.framescount);
 }
 return target;
}

//add atom to metafile
void addatom(struct HTMLrecord *atom,void *ptr,int len,char t, char align,
           char d1, unsigned char d2,unsigned currentlink,char norightedge)
{
 unsigned dataptr,prevHTMLatom=p->lastHTMLatom;

 // printf("Adding atom of type %d...\n",t);

 if(len==0 && t==TEXT) return;


 if(len)
  dataptr=ie_putswap((char *)ptr,len,CONTEXT_HTML);
 else
  dataptr=IE_NULL;
 atom->type=t;
 atom->align=align;
 atom->data1=d1;
 atom->data2=d2;
// atom->R=r;   //this comes pre-defined in HTML atom...
// atom->G=g;
// atom->B=b;
 atom->ptr=dataptr;
 atom->datalen=len;
 atom->linkptr=currentlink;
 atom->next=IE_NULL;
 atom->prev=prevHTMLatom;
 atom->frameID=p->currentframe;

 p->lastHTMLatom=ie_putswap((char *)atom,sizeof(struct HTMLrecord),CONTEXT_HTML);
 if(p->firstHTMLatom==IE_NULL)
  p->firstHTMLatom=p->lastHTMLatom;
 else
 {
  struct HTMLrecord *atomptr=(struct HTMLrecord *)ie_getswap(prevHTMLatom);
  if(atomptr)
  {
   atomptr->next=p->lastHTMLatom;
   swapmod=1;
  }
  else
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return;
//   MALLOCERR();
//!!glennmcc: end
 }

 if(p->lastHTMLatom==IE_NULL)
 {
  p->memory_overflow=1;
  return;
 }
 else
  p->HTMLatomcounter++;

 if(!norightedge)
 {
  if(atom->xx>p->docRight)
   p->docRight=atom->xx;
  if(atom->xx>p->docRightEdge)
   p->docRightEdge=atom->xx;
  if(atom->xx+HTMLBORDER>p->htmlframe[p->currentframe].scroll.total_x)
   p->htmlframe[p->currentframe].scroll.total_x=atom->xx+HTMLBORDER;

  p->xsum+=(long)(atom->xx-atom->x);
 }

}

// move HTML document "cursor" d pixels to the right
void xshift(int *x,int d)
{
 *x+=d;
 if(*x>p->docRight)
  p->docRight=*x;
 if(*x>p->docRightEdge)
  p->docRightEdge=*x;
 if(*x+HTMLBORDER>p->htmlframe[p->currentframe].scroll.total_x)
  p->htmlframe[p->currentframe].scroll.total_x=*x+HTMLBORDER;

 p->xsum+=(long)d;
}

//called after the current line has been processed
void alignrow(int x,long y,int islist)
{
 int xhop;
 //int idx;
 long vsize;
 unsigned currentHTMLatom=p->lastHTMLatom;
 struct HTMLrecord *atomptr;

 if(p->HTMLatomcounter==0)
  return;

 if(!p->sizeTextRow)p->sizeTextRow=p->sizeRow;

 alignloop:
 atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
 if(!atomptr)
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return;
//  MALLOCERR();
//!!glennmcc: end
 if(atomptr->y==y && //eliminate unclosed buttons:
   !(atomptr->type==INPUT && (atomptr->data2 & 2) && atomptr->x==atomptr->xx) &&
    !(atomptr->type==TD || atomptr->type==TD_BACKGROUND ||
      atomptr->type==TABLE || atomptr->type==NAME))
 {
  if(atomptr->align & BOTTOM) //ALIGN=BOTTOM
  {
   if(atomptr->type==TEXT) //text
   {
    atomptr->y+=p->sizeTextRow;
    atomptr->yy=atomptr->y;
    atomptr->y-=fonty((int)atomptr->data1,atomptr->data2);
   }
   else if(atomptr->type!=INPUT || ! (atomptr->data2 & 2))
        //^^^^^^^^^^^^^^^^^ this is for <BUTTON> tags
   {
    vsize=atomptr->yy-atomptr->y;
    atomptr->y+=p->sizeRow;
    atomptr->yy=atomptr->y;
    atomptr->y-=vsize;
   }
   swapmod=1; //I have written to swapped memory!
  }

  if(atomptr->align & CENTER) //ALIGN=CENTER
  {
   xhop=(p->docRight-x)/2;
   atomptr->x+=xhop;
   atomptr->xx+=xhop;
   swapmod=1; //I have written to swapped memory!
  }
  else
  if(atomptr->align & RIGHT) //ALIGN=RIGHT
  {
   xhop=(p->docRight-x);
   atomptr->x+=xhop;
   atomptr->xx+=xhop;
   swapmod=1; //I have written to swapped memory!
  }

  if(atomptr->align & SUP) //ALIGN=SUP
  {
   vsize=fonty((int)atomptr->data1,atomptr->data2)/3;
   atomptr->y-=vsize;
   atomptr->yy-=vsize;
   swapmod=1; //I have written to swapped memory!
  }

  if(atomptr->align & SUB) //ALIGN=SUB
  {
   vsize=fonty((int)atomptr->data1,atomptr->data2)/3;
   atomptr->y+=vsize;
   atomptr->yy+=vsize;
   swapmod=1; //I have written to swapped memory!
  }

  atomptr->align=0;
  if(atomptr->prev!=IE_NULL && atomptr->yy>y)
  {
   currentHTMLatom=atomptr->prev;
   goto alignloop;
  }
 }
 if(p->docClearRight && y+p->sizeRow>=p->docClearRight)
 {
  p->docRight=p->docRightEdge;
  p->docClearRight=0;
 }
 if(p->docClearLeft && y+p->sizeRow>=p->docClearLeft)
 {
  if(islist==0)p->docLeft=p->docLeftEdge;
  p->docClearLeft=0;
 }

 p->sizeTextRow=0;
}//end sub

//implementatino of HTML tag <BR CLEAR=ALL>
void clearall(long *y)
{
 if(p->docClearLeft && p->docClearLeft>=p->docClearRight)
  *y=p->docClearLeft;
 else if (p->docClearRight)
  *y=p->docClearRight;

 if(p->docClearLeft)
 {
  p->docClearLeft=0;
  p->docLeft=p->docLeftEdge;
 }

 if(p->docClearRight)
 {
  p->docClearRight=0;
  p->docRight=p->docRightEdge;
 }
}

void fixrowsize(int font,char style)
{
     if(fonty(font,style)>p->sizeRow)
      p->sizeRow=fonty(font,style);
     if(fonty(font,style)>p->sizeTextRow)
      p->sizeTextRow=fonty(font,style);
}

//save current font information to font stack
void pushfont(int font,char style, struct HTMLrecord *atom,struct Fontstack *fontstack)
{
 if(fontstack->depth<MAXFONTSTACK)
 {
  if(fontstack->depth<0) //default value is -1
   fontstack->depth=0;
  else
   fontstack->depth++; //default value is -1
  fontstack->font[fontstack->depth]=font;
  fontstack->style[fontstack->depth]=style;
  fontstack->rgb[3*fontstack->depth]=atom->R;
  fontstack->rgb[3*fontstack->depth+1]=atom->G;
  fontstack->rgb[3*fontstack->depth+2]=atom->B;
 }
}

//restore current font information to font stack
int popfont(int *font,char *style, struct HTMLrecord *atom, struct Fontstack *fontstack)
{
 if(fontstack->depth>=0)
 {
  *font=fontstack->font[fontstack->depth];
  *style=fontstack->style[fontstack->depth];
  atom->R=fontstack->rgb[3*fontstack->depth];
  atom->G=fontstack->rgb[3*fontstack->depth+1];
  atom->B=fontstack->rgb[3*fontstack->depth+2];
  fontstack->depth--; //margin value is -1
  return 1;
 }
 else
  return 0;
}

int RGB256(unsigned char r,unsigned char g,unsigned char b);

int RGB(unsigned char r,unsigned char g,unsigned char b)
{
#ifdef HICOLOR
 if(xg_256==MM_Hic)
 {
  char pal[3];

  pal[0]=r>>2;
  pal[1]=g>>2;
  pal[2]=b>>2;
  x_pal_1(17, pal);
  return 17;
 }
 else
#endif
  return RGB256(r,g,b);
}


//returns width of space (ASCI 32)
int space(char font)
{
 return fontx(font,0,' ');
}


//initialization of color cache, which speeds up color lookup
void resetcolorcache(void)
{
 memset(cacher,0,16);
 memset(cacheg,0,16);
 memset(cacheb,0,16);
 memset(coloridx,0,16);
 cacher[1]=255;
 cacheg[1]=255;
 cacheb[1]=255;
 coloridx[1]=15;
 rgbcacheidx=2;
}

int InitInput(struct ib_editor *fajl,char *name,char *str,int lines,int context)
{
 int rc;

 fajl->filename[0]='\0';
 rc=ie_openf_lim(fajl,context,lines);
 strcpy(fajl->filename,name);
 if(rc==1 && str)
  ie_insline(fajl,0,str);

 return rc;
}

struct TMPframedata *locatesheet(struct TMPframedata *rootsheet, struct TMPframedata *tmpsheet,XSWAP stylesheetadr)
{
 if(stylesheetadr==IE_NULL)
  return rootsheet;
 else
  return locatesheet_ovrl(rootsheet,tmpsheet,stylesheetadr);
}
