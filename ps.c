
// ========================================================================
// PostScript output module for Arachne WWW Browser
// (c)1998-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "html.h"

#define POSTSCRIPT "_4prt.ps"
#define PSPTBORDER 72

#define PT2PIX(pt) (long)((pt)*2)
#define PIX2PT(pix) ((pix)/2)
#define MM2PT(mm) (float)((mm)*2.8)

int kbhit(void);

void generateps(void)
{
 float page_pt=MM2PT(user_interface.postscript_y);
 float page_xpt=MM2PT(user_interface.postscript_x);
 long page_pix=PT2PIX(page_pt-2*PSPTBORDER);
 long currentpagestart_pix=0l;
 float pt_x,pt_y,pt_xx,pt_yy;
 int pagecount=0;
 int totalpages=(int)(p->htmlframe[p->activeframe].scroll.total_y/page_pix)+1;
 unsigned currentHTMLatom;
 char str[256];
 char text[2*IE_MAXLEN+2];
 char fntstr[128];
 char *fntptr=NULL;
 char currentfontsize;
 unsigned char currentfontstyle;
 int pspropfontsize[7]={0,6,7,8,9,10,14};  //in future, we will use Adobe Font Metrics instead !!!!!!
 int psfixedfontsize[7]={0,6,7,8,9,10,12};
 int realfontsize;
 struct HTMLrecord *atomptr;
#ifdef POSIX
 int f = a_open( POSTSCRIPT, O_RDWR | O_TEXT | O_CREAT | O_TRUNC, S_IREAD|S_IWRITE );
#else
 int f = a_sopen( POSTSCRIPT, O_RDWR | O_TEXT | O_CREAT | O_TRUNC,
		  SH_COMPAT | SH_DENYNONE, S_IREAD|S_IWRITE );
#endif
 if(f<0)
  return;

 sprintf(str,"%%!PS-Adobe-3.0\n%%%%Creator: Arachne %s%s %s\n",VER,beta,copyright);
 write(f,str,strlen(str));
 sprintf(str,"\
%%%%Title: %s\n\
%%%%BoundingBox: 0 0 %d %d\n\
%%%%Pages: %d\n\
%%%%EndComments\n\
\n\n",
       arachne.title,  (int)page_xpt,(int)page_pt,totalpages);
 write(f,str,strlen(str));

 //for all PostScript pages...
 while(currentpagestart_pix<p->htmlframe[p->activeframe].scroll.total_y)
 {
  pagecount++;
  sprintf(str,MSG_PS,pagecount,totalpages);
  outs(str);

  sprintf(str,"%%%%Page: %d %d\n",pagecount,pagecount);
  write(f,str,strlen(str));

  //for all font sizes and styles
  currentfontstyle=0;
  while(currentfontstyle<=(BOLD|ITALIC|FIXED)) //8 basic HTML font styles...
  {
   currentfontsize=1;
   while(currentfontsize<7) //six basic HTML font sizes...
   {
    switch(currentfontstyle)
    {
     case 0:                  fntptr="Helvetica";break;
     case BOLD:               fntptr="Helvetica-Bold";break;
     case ITALIC:             fntptr="Helvetica-Oblique";break;
     case BOLD|ITALIC:        fntptr="Helvetica-BoldOblique";break;
     case FIXED:              fntptr="Courier";break;
     case FIXED|BOLD:         fntptr="Courier-Bold";break;
     case FIXED|ITALIC:       fntptr="Courier-Oblique";break;
     case FIXED|BOLD|ITALIC:  fntptr="Courier-BoldOblique";
    }
    if(currentfontstyle & FIXED)
     realfontsize=psfixedfontsize[currentfontsize];
    else
     realfontsize=pspropfontsize[currentfontsize];

    sprintf(fntstr,"/%s findfont %d scalefont setfont\n",fntptr,realfontsize);
    fntptr=fntstr;

    //for all HTML atoms on this page...
    currentHTMLatom=p->firstHTMLatom;
    while(currentHTMLatom!=IE_NULL)
    {
     kbhit();
     atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
     if(!atomptr)
      MALLOCERR();
     currentHTMLatom=atomptr->next;

     if((atomptr->type==TEXT || currentfontsize==1 && currentfontstyle==0) &&
        atomptr->frameID==p->activeframe &&
        (atomptr->yy>currentpagestart_pix &&
         atomptr->yy<currentpagestart_pix+page_pix ||
         atomptr->y<currentpagestart_pix+page_pix) &&
         atomptr->y>currentpagestart_pix)
     {
      if((atomptr->type==TD && (atomptr->data1 || atomptr->data2) ||
          atomptr->type==TABLE && atomptr->data1) ||
          atomptr->type==HR || atomptr->type==LI || atomptr->type==TD_BACKGROUND)
      {//draw a box:
       pt_x=PSPTBORDER+PIX2PT(atomptr->x);
       pt_xx=PSPTBORDER+PIX2PT(atomptr->xx);
       pt_y=PSPTBORDER+PIX2PT((page_pix-(atomptr->y-currentpagestart_pix)));
       pt_yy=PSPTBORDER+PIX2PT((page_pix-(atomptr->yy-currentpagestart_pix)));
       if(pt_x<PSPTBORDER)pt_x=PSPTBORDER;
       if(pt_xx>page_xpt-PSPTBORDER)pt_xx=page_xpt-PSPTBORDER;
       if(pt_yy<PSPTBORDER)pt_yy=PSPTBORDER;
       if(pt_y>page_pt-PSPTBORDER)pt_y=page_pt-PSPTBORDER;

       sprintf(str,"\
newpath\n\
%.1f %.1f moveto %.1f %.1f lineto %.1f %.1f lineto %.1f %.1f lineto\n\
closepath\n\
stroke\n",pt_x,pt_y,pt_xx,pt_y,pt_xx,pt_yy,pt_x,pt_yy);
       write(f,str,strlen(str));

      }
      else if(atomptr->type==TEXT &&
              (atomptr->data1==currentfontsize ||
               currentfontsize==6 && atomptr->data1>6) &&
              (atomptr->data2 & (FIXED|BOLD|ITALIC))==currentfontstyle)
      {
       char *p,*q=text;
       int show=0;

       pt_x=PSPTBORDER+PIX2PT(atomptr->x);
       pt_y=PSPTBORDER+PIX2PT((page_pix-(atomptr->yy-currentpagestart_pix)+2));

       p=ie_getswap(atomptr->ptr);
       if(!p)
        MALLOCERR();

       //docasne!!! (tr.: provisional)
       while(*p)
       {
        if(!show && *p!=' ')
         show=1;
        if(*p=='(' || *p==')' || *p=='\\')
        {
         *q='\\';
         q++;
        }
        else if ((unsigned char)*p==160)
         *p=' ';

        *q=*p;
        p++;
        q++;
       }
       *q='\0';

       if(show)
       {
        if(fntptr)
        {
         write(f,fntptr,strlen(fntptr));
         fntptr=NULL;
        }

        sprintf(str,"%.1f %.1f moveto (%s) show\n",pt_x,pt_y,text);
        write(f,str,strlen(str));
       }
      }
     }//endif
    }//loop
    currentfontsize++;
   }//loop
   currentfontstyle++;
  }//loop

  currentpagestart_pix+=page_pix;
  write(f,"\nshowpage\n",10);
 }//loop
 strcpy(str,"%%Trailer\n%%EOF\n");
 write(f,str,strlen(str));
 a_close(f);
}

void saveasps(void)
{
 GLOBAL.validtables=TABLES_UNKNOWN;
 arachne.target=p->currentframe;
 mouseoff();
 p->rendering_target=RENDER_PRINTER;
 p->html_source=LOCAL_HTML;
 p->forced_html=0;
 if(renderHTML(p))
 {
  if(GLOBAL.validtables==TABLES_EXPAND)
   renderHTML(p);
  generateps();
 }
 else
  unlink(POSTSCRIPT);
}
