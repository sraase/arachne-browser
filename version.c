#include "arachne.h"

#ifndef CUSTOMER
char *VER="1.99";

//!!glennmcc: Begin Apr 06, 2003--- NOKEY==GPL now
#ifndef NOKEY

#ifdef MSDOS
char *beta=";UE14"; //MS-DOS version is stable
#else
char *beta=";beta";    //for other platforms, not even beta....
#endif//MSDOS

#else
#if defined(LINUX)
char *beta=";Linux";
#elif defined(XT086)
char *beta=";Final,286-";
#else
char *beta=";Final,387+";
#endif
#endif//ifndef NOKEY


#ifdef NOKEY//!!glennmcc: Apr 06,2003 --- NOKEY==GPL now
#ifdef BEAR
char *ident=" (Bear)";
#else
#ifdef ERIC
char *ident=" (Eric)";
#else
#ifdef GREGY
char *ident=" (Gregy)";
#else
char *ident="";
#endif//ifdef Gregy
#endif//ifdef Eric
#endif//ifdef Bear
char *copyright="";//ifdef NOKEY
#else
char *copyright="Copyright (c)1996-2002 Michael Polak, Arachne Labs";
#endif//ifdef NOKEY

char *homepage="http://arachne.cz/";

#endif//ifndef CUSTOMER
