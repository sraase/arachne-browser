#include "arachne.h"

char *VER="2.0-dev";

#if defined(LINUX)
char *beta=";Linux";
#elif defined(XT086)
char *beta=";286-";
#else
char *beta=";387+";
#endif

char *copyright="Arachne built " __DATE__ " at " __TIME__;

char *homepage="http://arachne.cz/";
