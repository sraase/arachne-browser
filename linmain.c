
//temporary stub for executing Arachne's main function

#ifdef SVGALIB
#include <vga.h>
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
#ifdef SVGALIB
     printf("Arachne/SVGAlib V%s%s %s\n",VER,beta,copyright);
#endif
#ifdef SDL2
     printf("Arachne/SDL2 V%s%s %s\n",VER,beta,copyright);
#endif
     return 0;
    }
   }

#ifdef SVGALIB
   result = vga_init ();
#endif

   if (!result) 
   {
      result = arachne_main (argc, argv);

    return result;
   }
#ifdef SVGALIB
   else printf ("Error initialising svgalib.\n");
#endif

   return 100;
}
