
// ========================================================================
// Search HTML document in memory for search string
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "html.h"
#include "gui.h"

void getTXTprompt(char *str,int lim)
{
 char *ptr;

 editorptr=(struct ib_editor *)ie_getswap(TXTprompt.ptr);
 if(!editorptr)
   MALLOCERR();
 memcpy(&tmpeditor,editorptr,sizeof(struct ib_editor));
 ptr=ie_getline(&tmpeditor,0);
 if(!ptr)
   MALLOCERR();
 makestr(str,ptr,lim);
}

void setTXTprompt(char *str)
{
 SetInputAtom(&TXTprompt,str);
}

//search in HTML document
void SearchString(void)
{
 struct HTMLrecord HTMLatom,foundatom;
 unsigned currentHTMLatom=p->firstonscr,foundHTMLatom,nextHTMLatom;
 struct HTMLframe *frame=&(p->htmlframe[p->activeframe]);
 long minY=0l;
 char *str,*tmp,*foundstr,*ptr,found=0;
 struct HTMLrecord *atomptr;

 str=farmalloc(IE_MAXLEN+2);
 tmp=farmalloc(IE_MAXLEN+2);
 foundstr=farmalloc(IE_MAXLEN+2);
 if(!str || !tmp || !foundstr)
  memerr();

 getTXTprompt(str,IE_MAXLEN);

 if(p->HTMLatomcounter>1000)
 {
  sprintf(tmp,MSG_SRCH1,str);
  outs(tmp);
 }
 strlwr(str);

 while(currentHTMLatom!=IE_NULL)
 {
  kbhit();
  atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if(!atomptr)
   MALLOCERR();
  nextHTMLatom=atomptr->next;
  if(atomptr->type==TEXT && atomptr->frameID==p->activeframe &&
     atomptr->y>=frame->posY)
  {
   memcpy(&HTMLatom,atomptr,sizeof(struct HTMLrecord));
   ptr=ie_getswap(atomptr->ptr);
   if(!ptr)
     MALLOCERR();
   strcpy(tmp,ptr);
   //tmp[IE_MAXLEN]='\0';
   strlwr(tmp);
   if(strstr(tmp,str) && HTMLatom.y>frame->posY && lastfound!=currentHTMLatom &&
      (lastfoundY<HTMLatom.y || lastfoundY==HTMLatom.y  && lastfoundX<HTMLatom.x) &&
      (!found || HTMLatom.y<minY))
   {
    minY=HTMLatom.y;
    memcpy(&foundatom,&HTMLatom,sizeof(struct HTMLrecord));
    makestr(foundstr,ptr,IE_MAXLEN);
    foundHTMLatom=currentHTMLatom;
    found=1;
    if(foundHTMLatom==p->firstHTMLatom)
     break;
   }
  }
  currentHTMLatom=nextHTMLatom;
 }//loop

 if(found)
 {
  if(foundatom.yy>frame->posY+frame->scroll.ysize)
  {
   frame->posY=foundatom.y-FUZZYPIX;
   if(frame->posY+frame->scroll.ysize>frame->scroll.total_y)
       frame->posY=frame->scroll.total_y-frame->scroll.ysize;
   if(frame->posY<0)
    frame->posY=0;
  }

  if(foundatom.xx>frame->posX+frame->scroll.xsize || foundatom.x<frame->posX)
  {
   frame->posX=foundatom.x;
   if(frame->posX+frame->scroll.xsize>frame->scroll.total_x)
       frame->posX=frame->scroll.total_x-frame->scroll.xsize;
  }

  if(arachne.framescount)
  {
   p->activeframe=0;
   redrawHTML(REDRAW_NO_MESSAGE,REDRAW_CREATE_VIRTUAL);
  }
  else
   redrawHTML(REDRAW_NO_MESSAGE,REDRAW_VIRTUAL);


  highlightatom(&foundatom);

  lastfound=foundHTMLatom;
  lastfoundY=foundatom.y;
  lastfoundX=foundatom.x;
  outs(foundstr);
  ie_appendclip(foundstr);
  GLOBAL.clipdel=0;
 }
 else
 {
  redrawHTML(REDRAW_NO_MESSAGE,REDRAW_VIRTUAL);
  outs(MSG_SRCH2);
 }
 farfree(foundstr);
 farfree(tmp);
 farfree(str);

}


void ReadWriteTextarea(char sw)
{
 struct HTMLrecord *aptr=(struct HTMLrecord *)ie_getswap(focusedatom);
 char fname[80];
 if(aptr)
 {
  memcpy(&activeatom,aptr,sizeof(struct HTMLrecord));
  activeatomptr=&activeatom;
  activeadr=focusedatom;
  getTXTprompt(fname,79);
  editorptr=(struct ib_editor *)ie_getswap(activeatomptr->ptr);
  if(editorptr)
  {
   memcpy(&tmpeditor,editorptr,sizeof(struct ib_editor));
   if(sw==INPUT_READFILE)
   {
    ie_insblock(&tmpeditor,fname);
    editorptr=(struct ib_editor *)ie_getswap(activeatomptr->ptr);
    if(editorptr)
    {
     memcpy(editorptr,&tmpeditor,sizeof(struct ib_editor));
     swapmod=1;
    }
    else
     MALLOCERR();
   }
   else
   {
    strcpy(tmpeditor.filename,fname);
    outs(MSG_WRITE);
    ie_savef(&tmpeditor);
   }
  }
  else
   MALLOCERR();
  redrawHTML(REDRAW_NO_MESSAGE,REDRAW_VIRTUAL);
  activeatomptr=&activeatom;
  activeadr=focusedatom;
  activeatomcursor(1);
 }//end if pointer exists - else "nonfatal error"
}


//search in active TEXTAREA...
void SearchInTextarea(char cont)
{
 int i=0;
 struct HTMLrecord *aptr=(struct HTMLrecord *)ie_getswap(focusedatom);
 if(aptr)
 {
  char searchstring[IE_MAXLEN+1],str[IE_MAXLEN+80],*ptr;
  int l1,l2,x;

  memcpy(&activeatom,aptr,sizeof(struct HTMLrecord));
  activeatomptr=&activeatom;
  activeadr=focusedatom;
  getTXTprompt(searchstring,IE_MAXLEN);
  sprintf(str,MSG_SRCH1,searchstring);
  outs(str);
  l1=strlen(searchstring);
  if(l1==0)
   goto redraw_and_exit;

  editorptr=(struct ib_editor *)ie_getswap(activeatomptr->ptr);
  if(editorptr)
  {
   memcpy(&tmpeditor,editorptr,sizeof(struct ib_editor));
   if(cont)
    i=tmpeditor.y;
   while(i<tmpeditor.lines)
   {
    ptr=ie_getline(&tmpeditor,i);
    if(cont)
    {
     x=tmpeditor.x+l1;
     ptr+=x;
     cont=0;
    }
    else
     x=0;
    l2=strlen(ptr);
    while(l2>=l1)
    {
     if(!strncmpi(searchstring,ptr,l1))
     {
      tmpeditor.y=i;
      tmpeditor.x=x;
      editorptr=(struct ib_editor *)ie_getswap(activeatomptr->ptr);
      if(editorptr)
      {
       memcpy(editorptr,&tmpeditor,sizeof(struct ib_editor));
       swapmod=1;
      }
      activeatomtick(ZOOM_SYNCHRO,TEXTAREA_NOREFRESH); //found
      goto redraw_and_exit;
     }
     ptr++;
     l2--;
     x++;
    }//loop
    i++;
   }//loop
  }
  else
   MALLOCERR();
  outs(MSG_SRCH2); //not found

  redraw_and_exit:
  redrawHTML(REDRAW_NO_MESSAGE,REDRAW_VIRTUAL);
  activeatomptr=&activeatom;
  activeadr=focusedatom;
  activeatomcursor(1);
 }//end if pointer exists - else "nonfatal error"
}
