#include "x_lopif.h"

//------ Prepnuti do videoram/XMS ------------
void x_video_XMS(int vidXMS, int bincol)
{
   xg_video_XMS = vidXMS;
   xg_bincol = bincol;
}

//------ Set active virtual screen
#ifdef VIRT_SCR
int xv_set_actvirt(int Index)    //Index: 0..MAX-1
{
    int OldInx;

    //if(Index < 0 || Index > 2) return( -1 );  // takhle ne !!!
    OldInx = xv_act;
    xv_act = Index;
    return( OldInx);
}
#endif