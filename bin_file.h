

//format of binary files for xChaos basic:

// 10 2 bytes: int n, n bytes: data
// 20 goto 10

#ifndef __BIN_FILE
#define __BIN_FILE

#include "ie.h"

struct bin_file
{
 char filename[80];
 char modified;
 int len; //tital lines 
 int cur;
// int firstswap; //swapy, ktere neobsahuji spolene radky, se smazou hromadne
    // tr.: swaps, that do not contain common lines, are deleted all together
 XSWAP *lineadr; //[IE_MAXLINES+2];
 unsigned short *linesize; //[IE_MAXLINES+2];
 int maxlines;
};

int ie_openbin(struct bin_file *fajl); //load, or open, 1. or 2.
int ie_savebin(struct bin_file *fajl);
void ie_resetbin(struct bin_file *fajl);

#endif
