#include "arachne.h"
#include "internet.h"

#ifdef POSIX
char ARACHNEPICK[80];
#else
char *ARACHNEPICK="arachne.pck";
#endif

#ifndef MINITERM


#ifndef POSIX
int askgraphics(void)
{
//!!glennmcc: Oct 23, 2005 -- always use VGA on 387+
//by using this entire block only for 286 and below
#ifdef XT086
int i;
  puts(MSG_VGASEL);
  puts(MSG_VGAVGA);
  puts(MSG_VGAEGA);
  puts(MSG_VGACGA);
  printf(MSG_VIDEO);

  vga:
  i=getch();
  if (i==27)
   return -1;

  if (i=='1')
  {
   strcpy(arachne.graphics,"EGA");
   return 2;
  }
  else if (i=='2')
  {
   strcpy(arachne.graphics,"BCGA");
   arachne.xSwap=2;
   return 3;
  }
  else if (i!='0' && i!=13)
   goto vga;
  else
#endif
//!!glennmcc: end -- Oct 23, 2005
  {
   strcpy(arachne.graphics,"VGA");
   return 1;
  }

}
#endif

int loadpick( char *exename) //nahrat konfiguraci
		// tr.: load configuration
{
 int f,i,rv=0;
 char *str1;

 strncpy( exepath, exename, 63);   // urceni exepath !!!
                // tr.: definition of exepath!
#ifdef POSIX
 str1= strrchr( exepath, '/');
#else
 str1= strrchr( exepath, '\\');
#endif
 if(str1 == NULL) exepath[0]='\0';
 else                str1[1]='\0';
 strlwr(exepath);

#ifdef POSIX
    // set system directories
    strcpy(sharepath, "/usr/share/arachne/");
    strcpy(helppath, "/usr/share/doc/arachne/");

    // set user directory
    str1 = getenv("HOME");
    if (!str1) str1 = "/";
    snprintf(dotarachne, sizeof(dotarachne), "%s/.arachne/", str1);
    snprintf(cachepath, sizeof(cachepath), "%scache/", dotarachne);
    snprintf(ARACHNEPICK, sizeof(ARACHNEPICK), "%sarachne.pck", dotarachne);
    snprintf(CLIPBOARDNAME, sizeof(CLIPBOARDNAME), "%sclipboard.bin", dotarachne);

    // create user and cache directories
    mkdir(dotarachne, 0700);
    mkdir(cachepath, 0700);

    // open runtime file
    f = a_open(ARACHNEPICK, O_RDONLY, 0);
#else
    // open runtime file
    f = a_open(ARACHNEPICK, O_BINARY | O_RDONLY, 0);
    if (f < 0)
    {
        // try again in exepath
        char str[80];
        sprintf(str,"%s%s", exepath, ARACHNEPICK);
        f = a_open(str, O_BINARY | O_RDONLY, 0);
    }
#endif

 if(f>=0)
  i=a_read(f,&arachne,sizeof(struct ArachnePick));

 if(f<0 || i!=sizeof(struct ArachnePick))
 {
  memset(&arachne,0,sizeof(struct ArachnePick));

#ifndef POSIX
//!!glennmcc: Oct 23, 2005 -- always use XMS on 387+
//by using this entire block only for 286 and below
#ifdef XT086
  puts(MSG_MEMSEL);
  puts(MSG_MEMXMS);
  puts(MSG_MEMEMS);
  puts(MSG_MEMDSK);

  printf(MSG_MEMORY);
  mem:
  i=getch();

  if(i==27)
  {
   printf("\n\n");
   exit(EXIT_ABORT_SETUP);
  }

  if (i=='1')
   arachne.xSwap=1;
  else if (i=='2')
   arachne.xSwap=2;
  else if (i!='0' && i!=13)
   goto mem;

  printf("\n");
#endif
//!!glennmcc: end -- Oct 23, 2005
  rv=askgraphics();

  if(rv==-1)
  {
   printf("\n\n");
   exit(EXIT_ABORT_SETUP);
  }
#endif
 }//endif first start

 p->htmlframe=(struct HTMLframe*)farmalloc(MAXFRAMES*(2+sizeof(struct HTMLframe)));
 AUTHENTICATION=farmalloc(sizeof(struct AUTH_STRUCT)+2);
 if(p->htmlframe && AUTHENTICATION)
 {
  if(f>=0)
   i=a_read(f,p->htmlframe,MAXFRAMES*(1+sizeof(struct HTMLframe)));
  else
   i=0;
  if(i<MAXFRAMES*(1+sizeof(struct HTMLframe)))
   memset(p->htmlframe,0,MAXFRAMES*(1+sizeof(struct HTMLframe)));

  if(f>=0)
   i=a_read(f,AUTHENTICATION,sizeof(struct AUTH_STRUCT));
  else
   i=0;
  if(i<sizeof(struct AUTH_STRUCT))
   memset(AUTHENTICATION,0,sizeof(struct AUTH_STRUCT));
 }
 else
  memerr0();

 a_close(f);

 return rv;

}

/*

void defaultGUIstyle(void)
{

  if(x_maxx()<640)
   arachne.GUIstyle = STYLE_SMALL2;
  else
  if(x_maxx()<800)
   arachne.GUIstyle = STYLE_SMALL1;

}

*/
#ifndef POSIX
void meminit(char arg)
{
 switch(arg)
 {
  case 1: //EMS
  SetXmsEms(2);
  break;
  case 2: //disk
  DisableXMS=1;
  break;
  default://like previous versions
  SetXmsEms(1);
 }
}
#endif

void savepick() //save configuration
{
 int f;

#ifdef POSIX
 f=a_open(ARACHNEPICK,O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
#else
 f=a_open(ARACHNEPICK,O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
#endif
 if(f>=0)
 {
  write(f,&arachne,sizeof(struct ArachnePick));
  write(f,p->htmlframe,MAXFRAMES*(1+sizeof(struct HTMLframe)));
  write(f,AUTHENTICATION,sizeof(struct AUTH_STRUCT));
  a_close(f);
 }
}

//in case newvalue!=NULL
//keystring must be allocated at the length keystring+newvalue together!

#endif //MINITERM

static char *configvariable(struct ib_editor *fajl,char *keystring,char *newvalue)
{
 int l;
 char *line,*ptr;

 l=strlen(keystring);
 fajl->y=0;
 while(fajl->y<fajl->lines)
 {
reaquire://!!glennmcc: Mar 08, 2008 -- case sensitive for entity.cfg
  line=ie_getline(fajl,fajl->y);
  if(line && strlen(line)>l && !strncmpi(line,keystring,l))//original line
  {
//!!glennmcc: Mar 08, 2008 -- case sensitive for entity.cfg
   char *filename=fajl->filename;
   if(strstr(filename,"entity.cfg") && strncmp(line,keystring,l))
     {
      fajl->y++;
      goto reaquire;
     }
//!!glennmcc: end
   if(newvalue)
   {
    newval:
    line=farmalloc(IE_MAXLEN);
    if(line)
    {
     strcpy(line,keystring);
     strcat(line," ");
     strcat(line,newvalue);
     ie_putline(fajl,fajl->y,line);
     farfree(line);
    }
    return NULL;
   }
   else
   {
    line+=l;                 //ted ukazuju na prvni mezeru
      // tr.: now I point at first space
    while(*line==' ')line++; //uriznu mezeru
      // tr.: I cut off space
    ptr=strchr(line,';');
    if(ptr)                  //je za hodnotou komentar ?
      // tr.: is after value a comment?
    {
     while(*(ptr-1)==' ')ptr--;//uriznu mezeru pred komentarem
      // tr.: I cut off space before comment
     *ptr='\0';
    }
    return line;             //vracim pointer na hodnotu promenne.
      // tr.: I reset pointer to the value of the variable
   }
  }
  fajl->y++;
 }
 if(newvalue && *newvalue && fajl->y<fajl->maxlines)
 {
  ie_insline(fajl,fajl->y,"");
  goto newval;
 }

 return NULL;
}

/**
 * read boolean option with default
 *   returns 1 for "Yes" / "True"  / "Enable"  / "On"
 *   returns 0 for "No"  / "False" / "Disable" / "Off"
 */
int config_get_bool(char *key, int defval)
{
	char *ptr = configvariable(&ARACHNEcfg, key, NULL);
	if (ptr) {
		switch (toupper(ptr[0])) {
		case 'Y': /* Yes    */
		case 'T': /* True   */
		case 'E': /* Enable */
			return 1;

		case 'N': /* No      */
		case 'F': /* False   */
		case 'D': /* Disable */
			return 0;
		}

		/* On / Off */
		if (!strcmpi(ptr, "on"))
			return 1;
		if (!strcmpi(ptr, "off"))
			return 0;
	}

	/* return default */
	return defval ? 1 : 0;
}

/**
 * read int option with default
 *   supports decimal, octal and hexadecimal
 */
int config_get_int(char *key, int defval)
{
	long val = config_get_long(key, defval);
	return (int)val;
}

/**
 * read long option with default
 *   supports decimal, octal and hexadecimal
 */
long config_get_long(char *key, long defval)
{
	char *ptr = configvariable(&ARACHNEcfg, key, NULL);
	if (ptr) {
		char *eptr;
		long val = strtol(ptr, &eptr, 0);
		if (val || ptr != eptr) {
			return val;
		}
	}
	return defval;
}

/**
 * read string option with default
 */
char *config_get_str(char *key, char *defstr)
{
	char *ptr = configvariable(&ARACHNEcfg, key, NULL);
	if (ptr) {
		return ptr;
	}
	return defstr;
}

/**
 * update string option with new value
 */
void config_set_str(char *key, char *newval)
{
	configvariable(&ARACHNEcfg, key, newval);
}

/**
 * get entity value or NULL
 */
char *config_get_entity(char *entity)
{
	char *ptr = configvariable(&ENTITYcfg, entity, NULL);
	return ptr;
}

/**
 * get toolbar string or NULL
 */
char *config_get_toolbar(char *name)
{
	char *ptr = configvariable(&TOOLBARcfg, name, NULL);
	return ptr;
}

#ifndef WWWMAN
#ifndef MINITERM
#ifndef POSIX

//. Roura : name of a batch file.  Allow us to use external executable.  Like Pipe in UNIX
void rouraname(char *fname)
{
 tempinit(fname);
 strcat(fname,"$roura$.bat");
}

// create a batch file executing cmd , uses name created by rouraname( ..)
int willexecute(char *cmd) //vykonat nejakou hloupost:
  // tr.: to do foolish things
{
 int f;
 char bat[80];

 rouraname(bat);
 f=a_fast_open(bat,O_TEXT|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
 if(f>=0)
 {
  write(f,cmd,strlen(cmd));
  a_close(f);
  if(tempdir[0])
   return EXIT_FAST_EXECUTE; // %ARACHNETEMP%\$roura$.bat
  else
   return EXIT_EXECUTE;      // .\$roura$.bat
 }
 else
 {
  return 0;
 }
}
#endif

#ifdef MINITERM
struct uiface user_interface;
#endif

extern struct ib_editor ARACHNEcfg;// main configuration

void configure_user_interface(void)
{
 char *value;
 long numval;

 /* Hand cursor */
 static const unsigned short cur[32] = {
  0x9FFF, 0x0FFF, 0x07FF, 0x83FF, 0xC1FF, 0xE0FF, 0xF067, 0xF003,
  0xF001, 0xF000, 0xF800, 0xF800, 0xF800, 0xFC00, 0xFC00, 0xFC00,
  0x0000, 0x6000, 0x7000, 0x3800, 0x1C00, 0x0E00, 0x0700, 0x0018,
  0x07EC, 0x07EE, 0x001E, 0x03EE, 0x03EE, 0x001E, 0x00EC, 0x0002
 };

 /* Cross cursor */
 static const unsigned short cur1[32] = {
  0xFC7F, 0xFC7F, 0xFC7F, 0xFC7F, 0xFC7F, 0xFC7F, 0x0001, 0x0101,
  0x0001, 0xFC7F, 0xFC7F, 0xFC7F, 0xFC7F, 0xFC7F, 0xFC7F, 0xFFFF,
  0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0xFEFE,
  0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0000
 };

 /* Arrow cursor */
 static const unsigned short cur2[32] = {
  0x1FFF, 0x0FFF, 0x07FF, 0x03FF, 0x01FF, 0x00FF, 0x007F, 0x003F,
  0x001F, 0x01FF, 0x10FF, 0x30FF, 0x787F, 0xF87F, 0xFC3F, 0xFFFF,
  0x4000, 0x6000, 0x7000, 0x7800, 0x7C00, 0x7E00, 0x7F00, 0x7F80,
  0x7C00, 0x6C00, 0x4600, 0x0600, 0x0300, 0x0300, 0x0180, 0x0000
 };

 value = config_get_str("CursorType", "Hand");
 switch (toupper(*value)) {
 case 'A':
  x_defcurs(cur2, &cur2[16], 15); // 'Arrow' cursor
  break;
 case 'C':
  x_defcurs(cur1, &cur1[16], 15); // 'Cross' cursor
  break;
 case 'H':
 default:
  x_defcurs(cur, &cur[16], 15); // 'Hand' cursor
  break;
 }

 user_interface.iconsoff = !config_get_bool("SmallIcons", 1);
 user_interface.hotkeys = config_get_bool("Hotkeys", 1);

 numval = config_get_long("AltSysFont", 0);
 if (numval < 0) numval = 0;
 if (numval > 2) numval = 2; /* prevent lock up in 640x480 */
 if (numval > 0) {
  user_interface.bigfnum  = numval;
  user_interface.bigfont  = 1;
  user_interface.bigstyle = FIXED;
 }

 if (config_get_bool("BigFont", 0)) {
  user_interface.bigfont  = 1;
  user_interface.bigfnum  = 5;
  user_interface.bigstyle = BOLD|FIXED;
 }

 user_interface.scrollbarsize = config_get_long("ScrollBarSize", 10);
 if(user_interface.scrollbarsize > 80)
  user_interface.scrollbarsize=80;

 value = config_get_str("Colors", "11 0"); /* green-on-black */
 if(value)
 {
  char *newvalue;

  user_interface.ink = (int)strtol (value, &newvalue, 10);
  if (newvalue != value) {
    value = newvalue;
    user_interface.paper = (int)strtol (value, &newvalue, 10);
    if (newvalue == value)
       user_interface.paper=0; //black
  }
  else user_interface.ink=11; //green
 }

 value = config_get_str("MouseColors", "7 15"); /* grey or white */
 if(value)
 {
  char *newvalue;

  user_interface.darkmouse = (int)strtol (value, &newvalue, 10);
  if (newvalue != value) {
    value = newvalue;
    user_interface.brightmouse = (int)strtol (value, &newvalue, 10);
    if (newvalue == value)
       user_interface.brightmouse=15; //black
  }
  else user_interface.darkmouse=7; //white
 }

 user_interface.frames = config_get_bool("Frames", 0);
 user_interface.quickanddirty = config_get_bool("QADT", 0);
 user_interface.cachefonts = config_get_bool("CacheFonts", 1);
 user_interface.cache2temp = config_get_bool("Cache2TEMP", 0);
 user_interface.killadds = config_get_bool("Kill468x60", 0);

#ifndef POSIX
 if (config_get_bool("AltKeys", 0) ||
     config_get_bool("AltTab", 0)) {
  InstalAltTab();
  atexit(ReleaseAltTab);
 }

 InstalPrtScr();
 atexit(ReleasePrtScr);
#endif

 user_interface.mailisdesktop = config_get_bool("MailIsDesktop", 1);
 user_interface.edithotlistentry = config_get_bool("EditHotlistEntry", 1);
 user_interface.autodial = config_get_bool("Autodial", 0);

#if defined(MSDOS) && !defined(XTVERSION)
 user_interface.vfat = config_get_bool("VFAT", 0);
#endif

 user_interface.keephtt = config_get_bool("KeepHTT", 1);
 user_interface.virtualysize = 0;
 user_interface.smooth = 0;

#ifdef VIRT_SCR
 user_interface.virtualysize = config_get_int("VirtualScreen", 0);
 user_interface.smooth = config_get_bool("SmoothScroll", 0);
 if (user_interface.smooth) {
  user_interface.step = config_get_int("ScrollStep", 1000);
 }

 /* [A]uto, [S]mart, [N]ice */
 value = config_get_str("ScreenMode", "Auto");
 switch (toupper(*value)) {
 case 'S':
 case 'N':
  user_interface.screenmode = toupper(*value);
  break;
 case 'A':
 default:
  user_interface.screenmode = 0;
  break;
 }
#endif // VIRT_SCR

 value = config_get_str("ScrollBarStyle", "Arachne");
 switch (toupper(*value)) {
 case 'A':
  user_interface.scrollbarstyle = 0; /* [A]rachne */
  break;
 case 'C':
  user_interface.scrollbarstyle = 'C'; /* [C]larence */
  user_interface.scrollbarsize = 0;    /* no scrollbars */
  break;
 default:
  /* [W]indoze, [N]extstep, [X]perimental, ... */
  user_interface.scrollbarstyle = toupper(*value);
  break;
 }

 value = config_get_str("ESC", "X");
 switch (toupper(*value)) {
 case 'I':
  user_interface.esc = ESC_IGNORE;
  break;
 case 'B':
  user_interface.esc = ESC_BACK;
  break;
 default:
  user_interface.esc = ESC_EXIT;
  break;
 }

 value = config_get_str("Multitasking", "X");
 if (toupper(*value) == 'S') {
  user_interface.multitasking = MULTI_SAFE;
 } else {
  int val = config_get_bool("Multitasking", 1);
  user_interface.multitasking = val ? MULTI_YES : MULTI_NO;
 }

 user_interface.logoiddle = config_get_int("Logo", 2000);
 user_interface.mindiskspace = config_get_long("MinDiskSpace", 128) * 1024;
 user_interface.xms4allgifs = config_get_int("XMS4allgifs", 256);
 user_interface.xms4onegif = config_get_long("XMS4onegif", 32) * 1024;
 user_interface.expire_dynamic = config_get_long("ExpireDynamic", 3600);
 user_interface.expire_static = config_get_long("ExpireStatic", 86400L);

 if (cgamode) {
  user_interface.fontshift = -2;
 } else {
  user_interface.fontshift = config_get_long("FontShift", 0);
  if (user_interface.fontshift < -2) user_interface.fontshift = -2;
  if (user_interface.fontshift >  1) user_interface.fontshift =  1;
 }

 user_interface.printerwidth = config_get_long("ASCIIwidth", 78);
 if (user_interface.printerwidth <  40) user_interface.printerwidth = 40;
 if (user_interface.printerwidth > 254) user_interface.printerwidth = 255;

#ifndef NOPS
 user_interface.postscript_x = config_get_int("PostScriptWidth",  210); /* A4 */
 if (user_interface.postscript_x <  100) user_interface.postscript_x = 100;
 if (user_interface.postscript_x > 1000) user_interface.postscript_x = 1000;

 user_interface.postscript_y = config_get_int("PostScriptHeight", 297); /* A4 */
 if (user_interface.postscript_y <  100) user_interface.postscript_y = 100;
 if (user_interface.postscript_y > 1000) user_interface.postscript_y = 1000;
#endif

 user_interface.keymap = NULL;
 if (config_get_bool("UseKeyMap", 0)) {
  value=configvariable(&ARACHNEcfg,"Keymap1",NULL);
  if(value && *value)
  {
   user_interface.keymap=farmalloc(96);
   if(user_interface.keymap)
   {
    strncpy((char *)user_interface.keymap,value,48);
    value=configvariable(&ARACHNEcfg,"Keymap2",NULL);
    if(value && *value)
     strncpy((char *)&user_interface.keymap[48],value,48);
   }
  }
 }

 ScreenSaver = config_get_long("ScreenSaver", 0);
 user_interface.refresh = config_get_int("RefreshDelay", 20);

 user_interface.css = config_get_bool("CSS", 1);

 // HTTP parameters --------------------------------------------------

 http_parameters.referer = config_get_bool("HTTPreferer", 1);
 http_parameters.keepalive = config_get_bool("HTTPKeepAlive", 1);
 http_parameters.useproxy = config_get_bool("UseProxy", 0);
 http_parameters.acceptcookies = config_get_bool("Cookies", 1);
 http_parameters.https2http = config_get_bool("HTTPS2HTTP", 1);
 user_interface.ignorejs = config_get_bool("IGNOREJS", 0);
 http_parameters.ignorebasehref = config_get_bool("IGNOREBASEHREF", 0);
 user_interface.alwaysusecfgcolors = config_get_bool("AlwaysUseCFGcolors", 0);
}

#endif
#endif
