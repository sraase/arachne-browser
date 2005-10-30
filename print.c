
// ========================================================================
// Plain ASCII output module for Arachne
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "html.h"

#define MAXLINES2PRINT 8000
#define PRINTFNAME "_4prt.txt"

//put line of text (radka) at x,y coordinates in ib_editor structure
void virtualtextdraw(struct ib_editor *txt,int x,int y,char *radka,int l)
{
 char *ptr;
 char str[IE_MAXLEN+2];
 int k;

 if(y>MAXLINES2PRINT)
  return;

 if(x>CONSOLEWIDTH)
  return;

 if(x+l>CONSOLEWIDTH)
 {
  l-=x+l-CONSOLEWIDTH;
  if(l<0)
   return;
 }

 if(x+l>=IE_MAXLEN)
 {
  l=IE_MAXLEN-x-1;
 }

 if(y<txt->lines)
 {
  ptr=ie_getline(txt,y);
  strcpy(str,ptr);

  k=strlen(str);
  if(x+l>=k && l<IE_MAXLEN)
   str[x+l]='\0';

  while(k<x)
   str[k++]=' ';

  memcpy(str+x,radka,l);

  ie_putline(txt,y,str);
 }
 else
 {
  while(y>txt->lines)
   ie_insline(txt,txt->lines,"");

  k=0;
  while(k<x)
   str[k++]=' ';

  memcpy(str+x,radka,l);
  str[x+l]='\0';
  ie_insline(txt,y,str);
 }

}

void generateprt(void)
{
 unsigned currentHTMLatom;
 struct ib_editor txt;
 int x,y,l,k;
 char radka[IE_MAXLEN+2], *ptr;
 struct HTMLrecord *atomptr;

/* printf("Console width is: %d\n\n",CONSOLEWIDTH);
#ifndef CLEMENTINE
 fflush(stdout);
#endif*/
 
 ie_openf_lim(&txt,CONTEXT_TMP,MAXLINES2PRINT);
 strcpy(txt.filename,PRINTFNAME);

 currentHTMLatom=p->firstHTMLatom;

 while(currentHTMLatom!=IE_NULL)
 {
//  kbhit();
  atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if(!atomptr)
   MALLOCERR();
  currentHTMLatom=atomptr->next;

  if(atomptr->frameID==p->activeframe &&
     atomptr->type==TEXT ||
     atomptr->type==HR ||
     atomptr->type==LI ||
     atomptr->type==TD_BACKGROUND ||
     (atomptr->type==TD)
      && (atomptr->data1 || atomptr->data2) ||
     (atomptr->type==TABLE)
      && (atomptr->data1) ||
     atomptr->type==INPUT
      && (atomptr->data1==TEXT ||
          atomptr->data1==SUBMIT ||
          atomptr->data1==BUTTON))

  {
   x=atomptr->x/FIXEDFONTX;
   y=(int)((atomptr->y+FIXEDFONTY/2-1)/FIXEDFONTY);

   if(x>=0 && x<IE_MAXLEN && y>=0 && y<MAXLINES2PRINT)
   {
    if(atomptr->type==LI)
    {
     if(x>0)x--;
     virtualtextdraw(&txt,x,y,"*",1);
    }
    else
    if(atomptr->type==HR || atomptr->type==TD ||
       atomptr->type==TD_BACKGROUND || atomptr->type==TABLE)
    {
     char z='-';
     int yy;
     if(atomptr->type==HR)
      z='=';

     l=atomptr->xx/FIXEDFONTX-x;
     if(l<0)l=0;
     if(l>IE_MAXLEN)l=IE_MAXLEN;
     radka[l]='\0';
     k=l;
     while(--k>=0)
      radka[k]=z;

     if(atomptr->type==TD || atomptr->type==TD_BACKGROUND)
     {
      strcat(radka,"+-");
      yy=(int)((atomptr->yy+FIXEDFONTY/2)/FIXEDFONTY);

      while(y<yy)
       virtualtextdraw(&txt,x+l,y++,"|",1);

      l++; //+-
     }
     else
     if(atomptr->type==TABLE)
     {
      radka[0]=',';
      yy=(int)(atomptr->yy/FIXEDFONTY)-1;

      while(yy>y)
       virtualtextdraw(&txt,x,yy--,"|",1);

      l--;
     }

     virtualtextdraw(&txt,x,y,radka,l);
    }
    else
    {
     if(atomptr->type==INPUT)
     {
      editorptr=(struct ib_editor *)ie_getswap(atomptr->ptr);
      if(editorptr)
      {
       memcpy(&tmpeditor,editorptr,sizeof(struct ib_editor));
       ptr=ie_getline(&tmpeditor,0);
      }
      else
       ptr=NULL;
     }
     else
      ptr=ie_getswap(atomptr->ptr);

     if(!ptr)
      memerr();

     l=strlen(ptr);
     if(l+x>IE_MAXLEN)l=IE_MAXLEN-x;
     if(l<0)l=0;
     makestr(radka,ptr,l);
     virtualtextdraw(&txt,x,y,radka,l);
    }
   }
  }
 }
 if(txt.lines+4<txt.maxlines)
 {
  int l;

  ie_insline(&txt,0,"");

  memset(radka,'_',CONSOLEWIDTH);
  radka[CONSOLEWIDTH]='\0';
  ie_insline(&txt,0,radka);

  memset(radka,' ',CONSOLEWIDTH);
  l=strlen(p->htmlframe[p->activeframe].cacheitem.URL);
  if(l>CONSOLEWIDTH-28)
  {
   l=CONSOLEWIDTH-28;
   strncpy(&radka[CONSOLEWIDTH-28],"...",3);
  }
  strncpy(radka,p->htmlframe[p->activeframe].cacheitem.URL,l);
  inettime(&radka[CONSOLEWIDTH-25]);
  ie_insline(&txt,0,radka);

   memset(radka,' ',CONSOLEWIDTH);
  l=strlen(arachne.title);
  if(l>CONSOLEWIDTH-16)
  {
   l=CONSOLEWIDTH-16;
   strncpy(&radka[CONSOLEWIDTH-16],"...",3);
  }
  strncpy(radka,arachne.title,l);
//!!glennmcc: Jan 26, 2005 -- keep aligned regardless of changes to VER
//also added lowercase 'v' in front of version number
  sprintf(&radka[CONSOLEWIDTH-(9+strlen(VER))],"Arachne v%s",VER);
//  sprintf(&radka[CONSOLEWIDTH-17],"Arachne %s",VER);
//!!glennmcc: end
  ie_insline(&txt,0,radka);
 }
 {
  char ffeed[2]=" ";
  ffeed[0]=12; //form feed?

  ie_insline(&txt,txt.lines,ffeed);
 }
 ie_savef(&txt);

 ie_killcontext(CONTEXT_TMP); //clear temporary file
// ie_closef(&txt);

}


void saveastext(void)
{
 GLOBAL.validtables=TABLES_UNKNOWN;
 fixedfont=1;
 arachne.target=p->currentframe;
 mouseoff();
 p->rendering_target=RENDER_PRINTER;
 p->html_source=LOCAL_HTML;
 p->forced_html=0;
 if(renderHTML(p))
 {
  if(GLOBAL.validtables==TABLES_EXPAND)
   renderHTML(p);
  outs(MSG_PRN);
  generateprt();
 }
 else
  unlink(PRINTFNAME);
 fixedfont=0;
}
