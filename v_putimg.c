
#include "x_lopif.h"
#include "picinfo.h"


void v_putimg(int xz,int yz, char *bitmap)
{
#ifdef VIRT_SCR
 if(xg_video_XMS)
  xv_int_wrt(xz,yz,bitmap);
 else
#endif
  x_putimg(xz,yz,bitmap,0);

}


void v_getimg(int x1, int y1, int x2, int y2, char *bitmap)
{
#ifdef VIRT_SCR
 if(xg_video_XMS)
  xv_int_rea(x1,y1,x2-x1+1,y2-y1+1,bitmap);
 else
#endif
  x_getimg(x1,y1,x2,y2,bitmap);
}

void mosaic_background(struct picinfo *gif,char *obuf,int yz,int imgx)
{
 int pomx=0;
 int pomy=yz;
 char notskipped=0;
 char pombuf[4096];

 while(pomy<gif->screen_y+gif->draw_y)
 {
  while(pomx+imgx<=gif->stop_x)
  {
   v_putimg(gif->screen_x+pomx,pomy,obuf);
   pomx+=gif->size_x;
  }

  pomx=gif->draw_x%gif->size_x;
  if(!(pomx == gif->draw_x && notskipped || !pomx))
  {
   v_getimg(gif->screen_x ,pomy,gif->screen_x +pomx,pomy,pombuf);
   v_putimg(gif->screen_x +gif->draw_x-pomx,pomy,pombuf);
  }

  pomx=0;
  pomy+=gif->size_y;
  notskipped=1;
 }

}//end if pozadi
