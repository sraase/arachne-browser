
//temporary stub for executing Arachne's main function

#ifdef GGI
#include <ggi/ggi.h>
#else
#include <vga.h>
#endif

#ifdef GGI
ggi_visual_t ggiVis;
#endif

#include "arachne.h"

/* Main program for Linux */

int main (int argc, char **argv) {
   int result;

   if(argc==2 && argv[1][0]=='-')
   {
    if(!strcmp(argv[1],"--help"))
    {
     printf("USAGE: arachne [switch] | [path or URL]\n\n");
     printf("Switches:\n-?  start with help screen\n-r  return to last visited URL\n");
     return 0;
    }
    else
    if(!strcmp(argv[1],"--version"))
    {
#ifdef GGI
     printf("Arachne/GGI V%s%s %s\n",VER,beta,copyright);
#else
     printf("Arachne/SVGAlib V%s%s %s\n",VER,beta,copyright);
#endif
     return 0;
    }
   }

#ifdef GGI
   result = ggiInit ();
   if (!result) {
      ggiVis = ggiOpen (NULL);
      if (!ggiVis) {
         printf ("Error opening GGI visual.\n");
         ggiExit ();
         return 100;
      }
   }
#else
   result = vga_init ();
#endif

   if (!result) 
   {
      result = arachne_main (argc, argv);

#ifdef GGI
      ggiClose (ggiVis);
      ggiExit ();
#endif
    return result;
   }
#ifdef GGI
   else printf ("Error initialising GGI.\n");
#else
   else printf ("Error initialising svgalib.\n");
#endif

   return 100;
}
