
// ========================================================================
// ie_* funtions are "Ibase Editor" functions for texfile manipulations
// (c)1995-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "posix.h"
#include "ie.h"
#include "str.h"
#include "glflag.h"
#include "a_io.h"
#include "pckbrd.h"

//**********  cut block ****************
void ie_cutblock(struct ib_editor *fajl)
{
 if(fajl->blockflag & 2)
 {
  int from,num=0;
  char *ptr,str[IE_MAXLEN+1];

  //move cursor up
  if(fajl->y>fajl->bey)
  {
   fajl->y-=(fajl->bey-fajl->bby);
  }
  else
  if(fajl->y==fajl->bey)
  {
   fajl->y-=(fajl->bey-fajl->bby);
   if(fajl->x>=fajl->bex)
    fajl->x-=(fajl->bex-fajl->bbx);
   else
   if(fajl->x>fajl->bbx)
    fajl->x=fajl->bbx;
  }
  else   //move cursor out of block if necessary

  if((fajl->y>fajl->bby && fajl->y<fajl->bey) ||
     (fajl->y==fajl->bby && fajl->x>fajl->bbx))
  {
   fajl->y=fajl->bby;
   fajl->x=fajl->bbx;
  }

  //single line block ?
  if(fajl->bby==fajl->bey)
  {
   ptr=ie_getline(fajl,fajl->bby);
   if(ptr)
   {
    makestr(str,ptr,IE_MAXLEN);
    makestr(&str[fajl->bbx],&ptr[fajl->bex],strlen(ptr)-fajl->bex);
    ie_putline(fajl,fajl->bby,str);
   }
  }
  else //multiline block
  {
   //truncate 1st line
   if(fajl->bbx>0 || fajl->bex>0)
   {
    ptr=ie_getline(fajl,fajl->bby);
    str[0]='\0';
    if(ptr && fajl->bbx<strlen(ptr))
    {
     makestr(str,ptr,fajl->bbx);
    }

    if(fajl->bex>0)
    {
     ptr=ie_getline(fajl,fajl->bey);
     if(ptr)
      makestr(&str[fajl->bbx],&ptr[fajl->bex],strlen(ptr)-fajl->bex);
    }
    ie_putline(fajl,fajl->bby,str);
    from=1;
   }
   else
   {
    from=0;
   }

   //delete block
   while(num<(fajl->bey-fajl->bby))
   {
    ie_delline(fajl,fajl->bby+from);
    num++;
   }
  }

  fajl->blockflag=0;
  fajl->bex=fajl->bbx;
  fajl->bby=fajl->bby;
 }
}

//********** copy block ****************
void ie_copyblock(struct ib_editor *fajl)
{
 if(fajl->blockflag & 2)
 {
  int i=fajl->bby;
  struct ib_editor clipboard;
  char *ptr,str[IE_MAXLEN+1];
  clipboard.filename[0]='\0'; //do not load clipboard !!!
  ie_openf_lim(&clipboard,CONTEXT_TMP,fajl->maxlines);
  strcpy(clipboard.filename,CLIPBOARDNAME);

  if(fajl->bbx>0 && fajl->bey!=i)
  {
   char str2[IE_MAXLEN+1];
   ptr=ie_getline(fajl,i);
   if(ptr && fajl->bbx<strlen(ptr))
   {
    strcpy(str2,&ptr[fajl->bbx]);
    ie_insline(&clipboard,clipboard.lines,str2);
   }
   i++;
  }

  while(i<fajl->bey)
  {
   ptr=ie_getline(fajl,i);
   if(ptr)
   {
    strcpy(str,ptr);
    ie_insline(&clipboard,clipboard.lines,str);
   }
   i++;
  }

  if(fajl->bex>0)
  {
   ptr=ie_getline(fajl,fajl->bey);
   if(ptr)
   {
    int l=strlen(ptr);
    char str2[IE_MAXLEN+1];
    if((fajl->bex<l || fajl->bby==fajl->bey) && l<IE_MAXLEN)
    {
     int targetx=0;
     if(fajl->bby==fajl->bey)
      targetx=fajl->bbx;

     makestr(str,ptr,targetx);
     strcpy(&str[targetx],&ptr[fajl->bex]);
     makestr(str2,&ptr[targetx],fajl->bex-targetx);
     ie_insline(&clipboard,clipboard.lines,str2);
    }
   }
  }
  ie_savef(&clipboard);
  ie_closef(&clipboard);
  ie_killcontext(CONTEXT_TMP);
 }
}

int ie_blockstart(struct ib_editor *fajl,int modifiers)
{
 if( (modifiers & LEFTSHIFT || modifiers & RIGHTSHIFT)  &&
     (fajl->blockflag==0 ||
      !(fajl->bbx==fajl->x && fajl->bby==fajl->y ||
        fajl->bex==fajl->x && fajl->bey==fajl->y) ))
 {
  fajl->bbx=fajl->x;
  fajl->bex=fajl->x;
  fajl->bby=fajl->y;
  fajl->bey=fajl->y;
  if(fajl->blockflag==2)
   return 3;
  else
  {
   fajl->blockflag=2;
   return 2;
  }
 }
 return 0;
}

void ie_xblockbeginend(struct ib_editor *fajl)
{
 if(fajl->bey<fajl->bby)
 {
  int pom=fajl->bey;
  fajl->bey=fajl->bby;
  fajl->bby=pom;
  pom=fajl->bex;
  fajl->bex=fajl->bbx;
  fajl->bbx=pom;
 }
 if(fajl->bey==fajl->bby && fajl->bex<fajl->bbx)
 {
  int pom=fajl->bex;
  fajl->bex=fajl->bbx;
  fajl->bbx=pom;
 }
}


int ie_blockend(struct ib_editor *fajl,int modifiers)
{
 if((modifiers & LEFTSHIFT || modifiers & RIGHTSHIFT) && fajl->blockflag==2)
 {
  if(fajl->y<fajl->bby ||
     fajl->y==fajl->bey && fajl->y==fajl->bby && fajl->x<fajl->bbx ||
     fajl->y<fajl->bey && fajl->y==fajl->bby)
  {
   fajl->bbx=fajl->x;
   fajl->bby=fajl->y;
  }
  else
  {
   fajl->bex=fajl->x;
   fajl->bey=fajl->y;
  }
  fajl->blockflag=2;
  if(fajl->bby==fajl->bey)
   return 1;
  else
   return 3;
 }
 return 0;
}

char ie_clipstatus=0;

void ie_appendclip(char *ptr)
{
 int f;
 if(!ie_clipstatus)
#ifdef POSIX
  f=a_open(CLIPBOARDNAME,O_CREAT|O_TRUNC|O_WRONLY,S_IREAD|S_IWRITE);
#else
  f=a_open(CLIPBOARDNAME,O_CREAT|O_TRUNC|O_TEXT|O_WRONLY,S_IREAD|S_IWRITE);
#endif
 else
#ifdef POSIX
  f=a_open(CLIPBOARDNAME,O_WRONLY|O_APPEND,0);
#else
  f=a_open(CLIPBOARDNAME,O_WRONLY|O_TEXT|O_APPEND,0);
#endif
 if(f>=0)
 {
  write(f,ptr,strlen(ptr));
  write(f,"\n",1);
  a_close(f);
  ie_clipstatus=1;
 }
}

/***** analyse keystroke and apply it to specified ib_editor structure *******/

int ie_key(struct ib_editor *fajl,int klavesa,int modifiers,int ietxt_max_x,int ietxt_max_y)

//returns 0:shit happens            / nic se nedeje,
//        1:set cursor+show status  / nastav kurzor
//        2:redraw line             / prekresli radku
//        3:redraw all              / prekresli vsechno
//      4:roluj o radku nahoru
//      5:roluj o radku dolu
//      6:prekresli cele okno i s ramem
//      7:"error"
//     27:stisknuto Esc
//101-110:stisknuto F1-F10
//201-210:stisknuto ctrl+F1-F10 (jen kombinace, ktere maji vyznam)
//301-...:special messages
{
 char znak=klavesa & 0xFF;
 int l;
 char *ptr;
 int rv=1;

 if(fajl->aktrad!=fajl->y)//zapamatovani posledni opracovane radky
 {
  if(fajl->aktrad>=0 && fajl->modrad) ie_putline(fajl,fajl->aktrad,fajl->rad);
  fajl->modrad=0;
  if(fajl->y<fajl->lines)strcpy(fajl->rad,ie_getswap(fajl->lineadr[fajl->y]));
  else fajl->rad[0]='\0';
  fajl->aktrad=fajl->y;
 }//endif
 l=strlen(fajl->rad);

 //block operation - copy, cut, paste
 if(fajl->blockflag & 1)  //^K is active
 {
  if(znak<27)
   znak+=64;
  fajl->blockflag-=1;
  if(fajl->aktrad>=0 && fajl->modrad) ie_putline(fajl,fajl->aktrad,fajl->rad);
  fajl->aktrad=-1;
  switch(toupper(znak))
  {
   //***** end block *****
   case 'K':
   fajl->bey=fajl->y;
   fajl->bex=fajl->x;
   if(fajl->bey>=fajl->bby && (fajl->bey>fajl->bby || fajl->bex>=fajl->bbx))
    fajl->blockflag=2; //visible
   else
    fajl->blockflag=0; //invisible
   return 3;

   //***** begin block *****
   case 'B':
   fajl->bby=fajl->y;
   fajl->bbx=fajl->x;
   fajl->blockflag=2; //visible
   if(fajl->bey>=fajl->bby && (fajl->bey>fajl->bby || fajl->bex>=fajl->bbx))
    fajl->blockflag=2; //visible
   else
    fajl->blockflag=0; //invisible
   return 3;

   //***** hide block *****
   case 'H':
   if(fajl->blockflag & 2)
   {
    rv=3;
    fajl->blockflag=0; //invisible
   }
   else
   {
    if(!fajl->bey<fajl->bby && (fajl->bex>fajl->bbx || fajl->bey>fajl->bby))
    {
     fajl->blockflag=2; //visible
     rv=3;
    }
    else
     rv=0;
   }
   return rv;

   //***** del block *****
   case 'Y':
   ie_cutblock(fajl);
   rv=3;
   goto zoom_synchro;

   //***** move block *****
   case 'M':
   case 'V':
   ie_copyblock(fajl);
   ie_cutblock(fajl);
   ie_insblock(fajl,CLIPBOARDNAME);
   rv=3;
   goto zoom_synchro;

   //***** copy block *****
   case 'C':
   ie_copyblock(fajl);
   ie_insblock(fajl,CLIPBOARDNAME);
   return 3;

   case 'R':
   return 103;
  }
  return 0;
 }

 if (klavesa==LEFTARROW) //doleva
 {
  rv|=ie_blockstart(fajl,modifiers);
  if(fajl->x>0) fajl->x--;

  bacha_nalevo: //sem se skace z Backspace
  if (fajl->x-fajl->zoomx<0) {fajl->zoomx=fajl->x;rv=3;}
  rv|=ie_blockend(fajl,modifiers);
  return rv;
 }
 else if (klavesa==RIGHTARROW) //doprava
 {
  rv|=ie_blockstart(fajl,modifiers);
  if(fajl->x<IE_MAXLEN) fajl->x++;

  bacha_napravo: //sem se skace z Backspace
  if (fajl->x-fajl->zoomx>ietxt_max_x) {fajl->zoomx=fajl->x-ietxt_max_x;rv=3;}
  rv|=ie_blockend(fajl,modifiers);
  return rv;
 }
 else
 if (klavesa==UPARROW) //nahoru
 {
  rv=1|ie_blockstart(fajl,modifiers);
  if (fajl->y>0)
  {
   fajl->y--;
   if (fajl->y-fajl->zoomy<0)
   {
    fajl->zoomy--;
    rv=3;
   }//endif
  }//endif
  rv|=ie_blockend(fajl,modifiers);
  return rv;
 }
 else
 if (klavesa==DOWNARROW) //dolu
 {
  rv=1|ie_blockstart(fajl,modifiers);
  if (fajl->y<fajl->lines-1)
  {
   fajl->y++;
   if (fajl->y-fajl->zoomy>ietxt_max_y)
   {
    fajl->zoomy++;
    rv=3;
   }//endif
  }//endif
  rv|=ie_blockend(fajl,modifiers);
  return rv;
 }
 else if (klavesa==CURSOR_SYNCHRO)
 {
  rv=0;
  if (fajl->x>l)
  {
   fajl->x=l;
   if(fajl->blockflag==4)
    fajl->blockflag=0;
   if(!rv)rv=1;
  }
  if (fajl->x<fajl->zoomx) {fajl->x=fajl->zoomx;rv=3;}
  if (fajl->y<fajl->zoomy) {fajl->y=fajl->zoomy;rv=3;}
  if (fajl->y>fajl->lines) {fajl->y=fajl->lines;if(!rv)rv=1;}
  if (fajl->y>fajl->zoomy+ietxt_max_y) {fajl->y=fajl->zoomy+ietxt_max_y;rv=3;}
  //? l=strlen(ie_getline(fajl,fajl->y));
  if (fajl->x>fajl->zoomx+ietxt_max_x) {fajl->x=fajl->zoomx+ietxt_max_x;rv=3;}
  return rv;
 }
 else if (klavesa==ZOOM_SYNCHRO)
 {
  rv=0;
  zoom_synchro:
  if (fajl->x<fajl->zoomx) {fajl->zoomx=fajl->x;rv=3;}
  if (fajl->y<fajl->zoomy) {fajl->zoomy=fajl->y;rv=3;}
  if (fajl->y>fajl->zoomy+ietxt_max_y) {fajl->zoomy=fajl->y;rv=3;}
  if (fajl->x>fajl->zoomx+ietxt_max_x) {fajl->zoomx=fajl->x;rv=3;}
  if(rv==3)
  {
   if (fajl->zoomy+ietxt_max_y>fajl->lines && fajl->lines>ietxt_max_y+1 )
    {fajl->zoomy=fajl->lines-ietxt_max_y-1;}
   if (l>ietxt_max_x+1 && fajl->zoomx+ietxt_max_x>l)
    {fajl->zoomx=l-ietxt_max_x-1;}
  }
  return rv;
 }
 else if (klavesa==PAGEUP) //PgUp
 {
  ie_blockstart(fajl,modifiers);
  fajl->y-=ietxt_max_y;
  if(fajl->y<0)fajl->y=0;
  fajl->zoomy-=ietxt_max_y;
  retpg:
  if(fajl->zoomy<0)fajl->zoomy=0;
  ie_blockend(fajl,modifiers);
  return 3;
 }
 else if (klavesa==CTRLHOME) //PgUp
 {
  fajl->x-=ietxt_max_x;
  if(fajl->x<0)fajl->x=0;
  fajl->zoomx-=ietxt_max_x;
  retpg2:
  if(fajl->zoomx<0)fajl->zoomx=0;
  return 3;
 }
 else if (klavesa==PAGEDOWN)
 {
  ie_blockstart(fajl,modifiers);
  fajl->y+=ietxt_max_y;
  if(fajl->y>=fajl->lines)fajl->y=fajl->lines-1;
  fajl->zoomy+=ietxt_max_y;
  if(fajl->zoomy>fajl->lines-ietxt_max_y)fajl->zoomy=fajl->lines-ietxt_max_y;
  goto retpg;
 }
 else if (klavesa==CTRLEND)
 {
  fajl->x+=ietxt_max_x;
  if(fajl->x>=fajl->cols)fajl->x=fajl->cols;
  fajl->zoomx+=ietxt_max_x;
  if(fajl->zoomx>fajl->cols-ietxt_max_x)fajl->zoomx=fajl->cols-ietxt_max_x;
  goto retpg2;
 }
 else if (klavesa==(int)0x8400) //Ctrl+PgUp
 {
  ie_blockstart(fajl,modifiers);
  fajl->y=0;
  fajl->zoomy=0;
  fajl->x=0;
  fajl->zoomx=0;
  ie_blockend(fajl,modifiers);
  return 3;
 }
 else if (klavesa==0x7600) //Ctrl+PgDn
 {
  ie_blockstart(fajl,modifiers);
  fajl->y=fajl->lines-1;
  fajl->zoomy=fajl->lines-ietxt_max_y-1;
  fajl->x=0;
  fajl->zoomx=0;
  goto retpg;
 }
 else if (klavesa==F1) //F1
  return 101;
 else if (klavesa==F2) //F2
  return 102;
 else if (klavesa==F3 || znak==ASCIICTRLR) //F3, ctrl+R
  return 103;
 else if (klavesa==F7 || znak==ASCIICTRLQ) //ctrl+q
  return 107;
 else if (znak==ASCIICTRLL) //ctrl+q
  return 127;
 /*
 else if (klavesa==0x3f00) //F5
  return 105;
 else if (klavesa==0x5e00) //Ctrl+F1
  return 201;
 */
 if (znak==13) //enter - pridavani radky
 {
  if(fajl->lines>=IE_MAXLINES) return 7; //beee, moc radek!
  if(fajl->aktrad>=0 && fajl->modrad) ie_putline(fajl,fajl->aktrad,fajl->rad);
  if(fajl->x<=l) //...uvnitr radky
  {
   if(fajl->y==fajl->lines)
    //pridani radky, kdyz stojim na prazdne posledni radce
    goto appendline;
   else
    //rozdeleni radky
    ie_insline(fajl,fajl->y+1,&(fajl->rad[fajl->x]));
  }
  else
  {
    //pridani prazdne radky
   if(fajl->y==fajl->lines)
   {
    appendline:
    //pridani radky kdyz jsem na/za koncem fajlu
    fajl->lines++;
    ie_putline(fajl,fajl->y+1,"\0");
   }
   else
    //pridani radky kdyz jsem uvnitr fajlu za koncem radky
    ie_insline(fajl,fajl->y+1,"\0");
  }//endif
  fajl->rad[fajl->x]='\0';
  if(fajl->bby>fajl->y)
  {
   fajl->bby++;
   fajl->bey++;
  }
  else
  if(fajl->bby<=fajl->y && fajl->bey>=fajl->y)
   fajl->blockflag=0; //hide block
  fajl->y++;
  if(fajl->y>=IE_MAXLINES)
   fajl->y=IE_MAXLINES-1;
  fajl->x=0;
  fajl->modrad=1;
  fajl->zoomx=0;
  if(fajl->y>fajl->zoomy+ietxt_max_y)fajl->zoomy++;
  return 3;//prekresli vsechno
 }
 else
 if (klavesa==INSERT) //Insert
 {
  fajl->insert=1-fajl->insert;
  return 1;//redraw status line
 }
 else
 if (klavesa==DELETEKEY) //Delete
 {
  if(fajl->x<l)
  {
   fajl->x++;
   return ie_key(fajl,BACKSPACE,modifiers,ietxt_max_x,ietxt_max_y); //finta
  }
  else
  if(fajl->x==l && fajl->y<fajl->lines-1)
  {
   fajl->x=0;
   fajl->y++;
   return ie_key(fajl,BACKSPACE,modifiers,ietxt_max_x,ietxt_max_y);
  }//endif
 }
 else
 if (klavesa==BACKSPACE) //Backspace
 {
  if(fajl->x>l)
   fajl->x--;
  else
  if(fajl->x>0)
  {
   memmove(&(fajl->rad[fajl->x-1]),&(fajl->rad[fajl->x]),l-fajl->x);
   fajl->rad[l-1]='\0';
   fajl->x--;
   fajl->modrad=1;
   fajl->modified=1;
   if(fajl->cols==l)fajl->cols--;
   rv=2; //redraw line
   goto bacha_nalevo;
  }
  else if(fajl->y>0)
  {
   char str[IE_MAXLEN];
   int pomx;

   ptr=ie_getline(fajl,fajl->y-1);
   if(strlen(ptr)+l>=IE_MAXLEN) return 9; //moc dlouha radka

   pomx=strlen(ptr);
   strcpy(str,ptr);
   strcat(str,fajl->rad);
   ie_delline(fajl,fajl->y);
   if(fajl->bby>=fajl->y)
    fajl->bby--;
   if(fajl->bey>=fajl->y)
    fajl->bey--;
   fajl->y--;
   ie_putline(fajl,fajl->y,str);
   if (fajl->y-fajl->zoomy<0) fajl->zoomy--;
   ptr=ie_getline(fajl,fajl->y);
   if(ptr!=NULL)strcpy(fajl->rad,ptr);else fajl->rad[0]='\0';
   fajl->aktrad=fajl->y;
   fajl->x=pomx;
   rv=3; //all
   goto bacha_napravo;
  }

  return rv;
 }
 if (znak==ASCIICTRLX && (fajl->blockflag & 2)) //Ctrl+X  .... cut to clipboard
 {
  if(fajl->aktrad>=0 && fajl->modrad) ie_putline(fajl,fajl->aktrad,fajl->rad);
  fajl->aktrad=-1;
  ie_copyblock(fajl);
  ie_cutblock(fajl);
  rv=3;
  goto zoom_synchro;
 }
 else
 if (znak==ASCIICTRLD && (fajl->blockflag & 2)) //Ctrl+D  .... cut to clipboard
 {
  if(fajl->aktrad>=0 && fajl->modrad) ie_putline(fajl,fajl->aktrad,fajl->rad);
  fajl->aktrad=-1;
  ie_cutblock(fajl);
  rv=3;
  goto zoom_synchro;
 }
 else
 if (znak==ASCIICTRLV) //Ctrl+V .... paste
 {
  if(fajl->aktrad>=0 && fajl->modrad) ie_putline(fajl,fajl->aktrad,fajl->rad);
  fajl->aktrad=-1;
  ie_insblock(fajl,CLIPBOARDNAME);
  return 3;
 }
 else
 if (znak==ASCIICTRLC)  //Ctrl+C  .... copy to clipboard
 {
  char unmark=0;
  if(!(fajl->blockflag & 2))
  {
   fajl->bbx=0;
   fajl->bex=0;
   fajl->bby=fajl->y;
   fajl->bey=fajl->y+1;
   fajl->blockflag=2;
   unmark=1;
  }
  if(fajl->aktrad>=0 && fajl->modrad) ie_putline(fajl,fajl->aktrad,fajl->rad);
  fajl->aktrad=-1;
  ie_copyblock(fajl);
  if(unmark)
   fajl->blockflag=0;
  return 301; //message "block written"
 }
 else
 if (znak==ASCIICTRLY) //Ctrl+Y  .... delete line
 {
  /*
  ptr=ie_getline(fajl,fajl->y);
  if(ptr)
  {
    .... store undelete info
  }
  */
  if(fajl->bby>fajl->y)
   fajl->bby--;
  if(fajl->bey>fajl->y)
   fajl->bey--;
  ie_delline(fajl,fajl->y);
  fajl->x=0;
  fajl->zoomx=0;
  ptr=ie_getline(fajl,fajl->y);
  if(ptr!=NULL)strcpy(fajl->rad,ptr);else fajl->rad[0]='\0';
  return 3;
 }
 /*
 else
 if (znak==ASCIICTRLU) //Ctrl+U,V
 {
  .... restore undelete info
 }
 */
 else
 if(klavesa==CTRLLEFT)// previous word
 {
  if(fajl->x)
  {
   fajl->x--;
   while(fajl->x &&
         !(fajl->x>1 && !isalnum(fajl->rad[fajl->x-1]) && isalnum(fajl->rad[fajl->x])))
    fajl->x--;
   goto bacha_nalevo;
  }
  else
  {
   if(fajl->y)
   {
    rv=1;
    fajl->x=strlen(ie_getline(fajl,--(fajl->y)));
    if (fajl->x-fajl->zoomx>ietxt_max_x) {fajl->zoomx=fajl->x-ietxt_max_x;rv=3;}
    if (fajl->y-fajl->zoomy<0){fajl->zoomy--;rv=3;}
   }
   return rv;
  }
 }
 else
 if(klavesa==CTRLRIGHT)// next word
 {
  if(fajl->x<l)
  {
   fajl->x++;
   while(fajl->x<l &&
         !(fajl->x>1 && !isalnum(fajl->rad[fajl->x-1]) && isalnum(fajl->rad[fajl->x])))
    fajl->x++;
   goto bacha_napravo;
  }
  else
  {
   if(fajl->y<fajl->lines-1)
   {
    rv=1;
    fajl->y++;
    fajl->x=0;
    if (fajl->zoomx>0) {fajl->zoomx=0;rv=3;}
    if (fajl->y-fajl->zoomy>ietxt_max_y) {fajl->zoomy++;rv=3;}
   }//endif
   return rv;
  }
 }
 else
 if(klavesa==HOMEKEY)//home
 {
  rv=1|ie_blockstart(fajl,modifiers);
  fajl->x=0;
  if(fajl->zoomx>0){fajl->zoomx=0;rv=3;}
  rv|=ie_blockend(fajl,modifiers);
  return rv;
 }
 else
 if(klavesa==ENDKEY)//end
 {
  rv=1|ie_blockstart(fajl,modifiers);
  fajl->x=l;
  if (fajl->x-fajl->zoomx>ietxt_max_x) {fajl->zoomx=fajl->x-ietxt_max_x;rv=3;}
  rv|=ie_blockend(fajl,modifiers);
  return rv;
 }
 else
 if (znak==20) //ctrl+T = toggle wordwrap
 {
  fajl->wordwrap=1-fajl->wordwrap;
  return 1;
 }
 if (znak==11) //ctrl+K = block prefix
 {
  fajl->blockflag|=1; //^K was pressed
  return 1;
 }
 else
 if (znak==23) return 123;//ctrl+W = write file
 else
 if (znak==27) return 27; //esc
 else
 if (znak==10 || znak==24) return 999; //Ctrl+Enter, Ctrl+X

 if (znak<32 && znak>=0) return 0; //nedefinovana klavesa

 if(fajl->wordwrap && fajl->x>=ietxt_max_x-1 && znak!=' ' && fajl->x>=l && fajl->maxlines>1)
 {
  int newx=l;
  while (newx>0 && fajl->rad[newx]!=' ')
   newx--;

  if(newx)
  {
   int dx=l-newx-1;
   fajl->x=newx;
   ie_key(fajl,DELETEKEY,0,ietxt_max_x,ietxt_max_y);
   ie_key(fajl,'\r',0,ietxt_max_x,ietxt_max_y);
   if(dx>0)
    fajl->x=dx;
   ie_key(fajl,klavesa,0,ietxt_max_x,ietxt_max_y);
   return 3;
  }
 }

 //pridani radky, pokud se nachazim za koncem souboru
 if(fajl->y==fajl->lines) fajl->lines++;
 if(l<IE_MAXLEN-1 && fajl->x<l && fajl->insert)
  memmove(&(fajl->rad[fajl->x+1]),&(fajl->rad[fajl->x]),l-fajl->x+1);
 while(fajl->x>l)fajl->rad[l++]=' ';
 fajl->rad[(fajl->x)++]=znak;
 fajl->rad[++l]='\0';
 if(l>fajl->cols)fajl->cols=l;
 fajl->modified=1;
 fajl->modrad=1;
 if (fajl->x-fajl->zoomx>ietxt_max_x) {fajl->zoomx=fajl->x-ietxt_max_x;return 3;}
 else return 2;//redraw line
}//end sub

