
// ========================================================================
// Part of Arachne URL/CACHE management/history - overlaid in DOS version.
// (c)1997-2000 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "html.h"
#include "internet.h"

void makehttfilename(char *locname, char *httname)
{
#ifdef POSIX
#ifdef CLEMENTINE
 char *endp;
 
 strcpy(httname,locname);
 endp=strrchr(httname,'.');
 if (endp)
   strcpy (endp,".htt");
 else
   strcat (httname, ".htt");
#else
 strcpy(httname,locname);
 strcat(httname,".http");
#endif
#else
 char drive[MAXDRIVE]="\0";
 char dir[MAXDIR]="\0";
 char file[MAXFILE]="\0";
 char ext[MAXEXT]="\0";

 fnsplit(locname,drive,dir,file,ext);
 strcat(dir,"headers");
 fnmerge(httname,drive,dir,file,".htt");
#endif
}

void removefromcache(struct HTTPrecord *cacheptr)
{
 unlink(cacheptr->locname);
 if(!strncmpi(cacheptr->URL,"http:",5))
 {
  char httname[80];
  makehttfilename(cacheptr->locname,httname);
  unlink(httname);
 }
}

#if defined (POSIX) && !defined (CLEMENTINE)
#define MAX_FNLEN 20
#else
#define MAX_FNLEN 10
#endif

int var=0; //cache items numbering

//create new cache record:
XSWAP Write2Cache(struct Url *absURL,struct HTTPrecord *cacheitem, char ovr,char newfilename)
{
 int overwriteidx=0;
 time_t oldesttime=0;
 struct HTTPrecord *cacheptr;
// struct ffblk ff;
 XSWAP rv;
 char fname[16];
 char *ptr;
 char firstkotva=absURL->kotva[0];
 time_t t;
 char cachefull=0;
 long largest=0;
 long lastseen;
 
 absURL->kotva[0]='\0';
 url2str(absURL,cacheitem->URL);
 absURL->kotva[0]=firstkotva;
 cacheitem->lastseen=time(NULL);

 if(newfilename)
 {
  //resetuju souradnice kde byl dokument zobrazen
  cacheitem->x=0;
  cacheitem->y=0l;

  do
  {
   var+=10;
   t=time(NULL)+var;
   sprintf(fname,"%ld.*",t);
   cacheitem->locname[0]='\0';

   if(user_interface.cache2temp && !strncmpi(cacheitem->URL,"file:",5))
    tempinit(cacheitem->locname);

   if(cacheitem->locname[0]=='\0')
   {
    ptr=configvariable(&ARACHNEcfg,"CachePath",NULL);
    if(!ptr)
     ptr=cachepath;
    strcpy(cacheitem->locname,ptr);
   }

   if(strlen(fname)>MAX_FNLEN)
    strcat(cacheitem->locname,&fname[strlen(fname)-10]);
   else
    strcat(cacheitem->locname,fname);

  }
  while(file_exists(cacheitem->locname));

  strcpy(cacheitem->rawname,cacheitem->locname);
 }//endif newfilename

#ifndef POSIX
 cachefull=lastdiskspace(cacheitem->locname)<user_interface.mindiskspace;
#endif
#ifdef CLEMENTINE
 cachefull=getfreespace("/cache")<0x80000;
#endif

 HTTPcache.cur=0;
 while(HTTPcache.cur<HTTPcache.len)
 {
  cacheptr=(struct HTTPrecord *)ie_getswap(HTTPcache.lineadr[HTTPcache.cur]);
  if(!cacheptr)
    MALLOCERR();
  if(!strcmp(cacheptr->URL,cacheitem->URL))
  {
   if(!cachefull)
   {
    //pokud nejde o NUL, tak pouziju stejne jmeno souboru...
    if(strcmp(cacheptr->locname,"NUL"))
     strcpy(cacheitem->locname,cacheptr->locname);
    rv=HTTPcache.lineadr[HTTPcache.cur];
    goto out;
   }
   else
   {
    if(strcmp(cacheptr->locname,cacheptr->rawname)) // !=
     removefromcache(cacheptr);
    cacheptr->URL[0]='\0';
    swapmod=1;
   }

  }
  HTTPcache.cur++;
 }


 if(HTTPcache.len<HTTPcache.maxlines && !cachefull ||
    HTTPcache.len==0)
 {
  HTTPcache.cur=HTTPcache.len;
  HTTPcache.lineadr[HTTPcache.cur]=ie_putswap((char *)cacheitem,sizeof(struct HTTPrecord),CONTEXT_SYSTEM);
  HTTPcache.linesize[HTTPcache.cur]=sizeof(struct HTTPrecord);
  rv=HTTPcache.lineadr[HTTPcache.cur];
  HTTPcache.len++;
  return rv;
 }
 else
 {
  ovr=1;
  HTTPcache.cur=0;
  while(HTTPcache.cur<HTTPcache.len)
  {
   cacheptr=(struct HTTPrecord *)ie_getswap(HTTPcache.lineadr[HTTPcache.cur]);
   if(!cacheptr)
    MALLOCERR();

   if(cachefull)
   {
    if(cacheptr->URL[0] && largest<cacheptr->size && cacheptr->knowsize)
    {
     largest=cacheptr->size;
     overwriteidx=HTTPcache.cur;
    }
   }
   else
   {
    if(!cacheptr->URL[0])
    {
     overwriteidx=HTTPcache.cur;
     goto done;
    }

    lastseen=cacheptr->lastseen;
    if(cacheptr->dynamic)
     lastseen-=user_interface.expire_dynamic;
    if(oldesttime>lastseen || !oldesttime)
    {
     oldesttime=lastseen;
     overwriteidx=HTTPcache.cur;
     if(!oldesttime)
      goto done;
    }
   }

   HTTPcache.cur++;
  }
 }
 done:
 rv=HTTPcache.lineadr[overwriteidx];
 cacheptr=(struct HTTPrecord *)ie_getswap(rv);

 out:
 if(ovr ||
     strcmp(cacheptr->locname,cacheitem->locname) &&
     strcmp(cacheptr->locname,cacheitem->rawname) )
  removefromcache(cacheptr);

 memcpy(cacheptr,cacheitem,sizeof(struct HTTPrecord));
 swapmod=1;//zapsal jsem do swapu...

 return rv;
}

void UpdateInCache(XSWAP cacheadr, struct HTTPrecord *store)
{
 struct HTTPrecord *cacheptr;

 if(cacheadr==IE_NULL)
  return;

 cacheptr=(struct HTTPrecord *)ie_getswap(cacheadr);
 if(!cacheptr)
  return;   //nothing to update
//  MALLOCERR();

 store->lastseen=time(NULL);
 memcpy(cacheptr,store,sizeof(struct HTTPrecord));
 swapmod=1;
}

void UpdateFilenameInCache(XSWAP cacheadr, struct HTTPrecord *store)
{
 struct HTTPrecord *cacheptr;

 if (!store) return;

 cacheptr=(struct HTTPrecord *)ie_getswap(cacheadr);
 if(!cacheptr)
   MALLOCERR();

 if ((store->locname) && (cacheptr->locname))
    strcpy(cacheptr->locname,store->locname);
 if ((store->rawname) && (cacheptr->rawname))
    strcpy(cacheptr->rawname,store->rawname);
 if ((store->mime) && (cacheptr->mime))
    strcpy(cacheptr->mime,store->mime);

 cacheptr->size=store->size;
 cacheptr->knowsize=store->knowsize;
 swapmod=1;
}

void DeleteFromCache(XSWAP cacheadr)
{
 struct HTTPrecord *cacheptr;

 cacheptr=(struct HTTPrecord *)ie_getswap(cacheadr);
 if(!cacheptr)
   MALLOCERR();

 cacheptr->URL[0]='\0';
 cacheptr->rawname[0]='\0';
 cacheptr->lastseen=0l;
 unlink(cacheptr->locname);
 strcpy(cacheptr->locname,"NUL");
 swapmod=1;
}

#define MAXCONV 255

// =====================================================================
// Searching HTML page for missing images  ( jpeg  images)
// =====================================================================
//char reload: FIND_MISSING_IMAGE | EXPIRE_ALL_IMAGES

char NeedImage(char reload, XSWAP *from)
{
 struct Url url;
 struct picinfo *img;
 unsigned status;
 XSWAP uptr,currentHTMLatom=p->firstHTMLatom,IMGatom;
 XSWAP imageptr[MAXCONV];
 int found,converting=0,willconvert=0;
 int maxmem=1;//,frameID;
// struct ffblk ff;
// char pushact;
 char command[120],ext[5];
 char type;
 char *URLptr;
 struct HTMLrecord *atomptr;
 struct HTTPrecord HTTPdoc;

/* if(arachne_will_download_images_now)
 {
  maybe later...

 }*/


 if(from && *from!=IE_NULL && reload==FIND_MISSING_IMAGE)
  currentHTMLatom=*from;

 if(reload==EXPIRE_ALL_IMAGES)
  outs(MSG_DELAY0);
 else
 {
  char *value=configvariable(&ARACHNEcfg,"LoadImages",NULL);
  if(!GLOBAL.nowimages && value && (*value=='n' || *value=='N'))
   return 0;
  outs(MSG_VERIFY);
 }

 //pushact=p->activeframe;
 while(currentHTMLatom!=IE_NULL)
 {
// doesn't help anyway  kbhit();
#ifndef NOTCPIP
  Backgroundhttp();
#endif
  atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if(!atomptr)
  {
   if(from)
    return NeedImage(FIND_MISSING_IMAGE,NULL);
   else
    MALLOCERR();
  }

  IMGatom=currentHTMLatom;
  if( currentHTMLatom==atomptr->next)
   currentHTMLatom=IE_NULL; //avoid looping
   else
   currentHTMLatom=atomptr->next;
  type=atomptr->type;

//!!glennmcc: Jan 19, 2003 -- added support for 'BGSOUND'
  if(type==IMG || type==EMBED || type==BGSOUND || type==BACKGROUND ||
     type==TD_BACKGROUND || type==STYLESHEET)
  {
   if(atomptr->yy>p->htmlframe[atomptr->frameID].posY &&
      atomptr->xx>p->htmlframe[atomptr->frameID].posX &&
      atomptr->y<p->htmlframe[atomptr->frameID].posY+p->htmlframe[atomptr->frameID].scroll.ysize &&
      atomptr->x<p->htmlframe[atomptr->frameID].posX+p->htmlframe[atomptr->frameID].scroll.xsize)
    GLOBAL.imagevisible=1;
   else
    GLOBAL.imagevisible=0;
//   frameID=atomptr->frameID;

   if(type==STYLESHEET)
   {
    URLptr=ie_getswap(atomptr->ptr);
    if(!URLptr)
     MALLOCERR();
   }
   else
   {
    img=(struct picinfo *)ie_getswap(atomptr->ptr);
    if(img)
     URLptr=img->URL;
    else
     MALLOCERR();
   }

   if(!URLptr[0])
   {
//    printf("[object URL is void]");
    continue; //continue with loop
   }

   AnalyseURL(URLptr,&url,IGNORE_PARENT_FRAME);
   found=SearchInCache(&url,&HTTPdoc,&uptr,&status);

   if(reload==EXPIRE_ALL_IMAGES && found && status!=LOCAL)
   {
    del:
    if(!ie_getswap(uptr))
     uptr=Write2Cache(&url,&HTTPdoc,1,0);
    DeleteFromCache(uptr);
   }
   else
   if(reload==FIND_MISSING_IMAGE)
   {
    if(!found && status==REMOTE && GLOBAL.nowimages!=IMAGES_SEEKCACHE)
    {
     strcpy(GLOBAL.location,HTTPdoc.URL);
     GLOBAL.isimage=1;
     mouseon();
//       p->activeframe=pushact;
     if(from && currentHTMLatom!=IE_NULL)
      *from=currentHTMLatom;
     return 1;

     /* maybe later ...
     arachne_will_download_images_now++;
     if(arachne_will_download_images_now>LIM_DOWNLOAD)
     {
      return NeedImage(FIND_MISSING_IMAGE,NULL);
     }
     */

    }
    else//conversion only
    if(GLOBAL.nowimages!=IMAGES_SEEKCACHE && type!=STYLESHEET &&
	(search_mime_cfg(HTTPdoc.mime, ext,command)==1 ||
	 type==EMBED && search_mime_cfg(HTTPdoc.mime, ext, command)==2 //)
//!!glennmcc: Jan 19, 2003 --- added support for 'BGSOUND'
	|| type==BGSOUND && search_mime_cfg(HTTPdoc.mime, ext, command)==2)

	&& willconvert<MAXCONV)
     imageptr[willconvert++]=IMGatom;
   }//endif

  }
 }//loop


 if(from && *from!=IE_NULL) //we have to verify if we don't need to convert images...
  return NeedImage(FIND_MISSING_IMAGE,NULL);

/*
 maybe later
 if(arachne_will_download_images_now)
  return NeedImage(FIND_MISSING_IMAGE,NULL);
*/

 mouseoff();
#ifdef POSIX
 p->buf[0]='\0';
#else
 strcpy(p->buf,"@echo off\n");
#endif


 if(willconvert) //prepare for conversion
 {
  int mode,f=-1,i=0;
  int cmdlen=0;
  char *pom;

  while(i<willconvert)
  {
   atomptr=(struct HTMLrecord *)ie_getswap(imageptr[i]);
   if(!atomptr)
    MALLOCERR();

//   frameID=atomptr->frameID;
   uptr=atomptr->ptr;
   type=atomptr->type;
   img=(struct picinfo *)ie_getswap(uptr);
   if(img)
   {
    AnalyseURL(img->URL,&url,IGNORE_PARENT_FRAME);
    SearchInCache(&url,&HTTPdoc,&uptr,&status);

    // if raw=.JPG AND loc=.JPG then ...
    if(!strcmp(HTTPdoc.locname,HTTPdoc.rawname) &&
       search_mime_cfg(HTTPdoc.mime, ext, command)==1
       || type==EMBED //)
//!!glennmcc: Jan 19, 2003 --- added support for 'BGSOUND'
       || type==BGSOUND)
    {
     if(!ie_getswap(uptr))
     {
      char tmpstr[80];
      makestr(tmpstr,HTTPdoc.rawname,79);
      uptr=Write2Cache(&url,&HTTPdoc,0,1); //create filename
      makestr(HTTPdoc.rawname,tmpstr,79);
     }

     pom=strrchr(HTTPdoc.locname,'.');
     if(pom)
     {
      strcpy(&pom[1],ext);
      sprintf(HTTPdoc.mime,"file/.%s",ext);

      if(file_exists(HTTPdoc.rawname) && !strstr(p->buf,HTTPdoc.rawname) && //exclude dupes...
         cmdlen<BUF-IE_MAXLEN)
      {
#ifdef CLEMENTINE
       if (!strcmp (command, "djpeg $j -outfile $2 $1")) {
          jpeg2bmp (HTTPdoc.rawname, HTTPdoc.locname);
       }
       else {
#endif
          //we will try to convert image, if there is enough space
	  //in $roura$.bat, (means "pipe") and if the image is already not
          //in converting queue:
          mode=make_cmd(command, &(p->buf[cmdlen]), HTTPdoc.URL, "\0","\0",
                        HTTPdoc.rawname, HTTPdoc.locname);

          strcat(&(p->buf[cmdlen]),"\n");
          cmdlen+=strlen(&(p->buf[cmdlen]));

          //touch(HTTPdoc.locname) - mainly for Write2Cache
          f=a_fast_open(HTTPdoc.locname,O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
          if(f>=0)
   	  a_close(f);

	  if(maxmem && mode>maxmem || mode<=0)
          maxmem=mode;
          converting++;
         }
#ifdef CLEMENTINE
      }
#endif
     }//endif nothing to convert

     UpdateInCache(uptr,&HTTPdoc);
    }
    else
    if(GLOBAL_justrestarted)
    {
     //we just attempted to convert, we were seeking cache for
     //converted image, but nothing was found. We won't convert
     //the picture again...

     if(!ie_getswap(uptr))
      uptr=Write2Cache(&url,&HTTPdoc,0,0);
	  strcpy(HTTPdoc.locname,"NUL");
     HTTPdoc.rawname[0]='\0';
  	  UpdateInCache(uptr,&HTTPdoc);
    }
   }//endif
   else
    MALLOCERR();

   i++;
  }//loop
 }//endif prepare for conversion


 if(converting)
 {
  char msg[128];
  sprintf(msg,MSG_CONVI,converting,MSG_DELAY1,ctrlbreak);
  outs(msg);

  MemInfo(NORMAL);
  arachne.target=0;
#ifndef POSIX
  if(maxmem>0 && farcoreleft()>(long)((long)maxmem+10l)*1024)
  {
   willexecute(p->buf);
#endif
#ifndef NOTCPIP
   FinishBackground(BG_FINISH);
#endif

   outs(msg);
#ifdef POSIX
   printf("Executing command:\n%s\n",p->buf);
   system(p->buf);
#else
   rouraname(command);
   system(command);
#endif

   GLOBAL.nowimages=IMAGES_SEEKCACHE;
   GLOBAL.needrender=1;

#ifndef POSIX
  }
  else
  {
   if(maxmem==-1)
    closebat(/*cmd*/p->buf,RESTART_REDRAW);
   else
    closebat(/*cmd*/p->buf,RESTART_KEEP_GRAPHICS);
   GLOBAL.willexecute=willexecute(/*cmd*/p->buf);
#ifndef NOTCPIP
   FinishBackground(BG_FINISH);
#endif
   outs(msg);
   gotoxy(1,8);
   mouseoff();
   if(maxmem==-2)
   {
    x_setfill(0,0);
    x_bar(p->htscrn_xtop,p->htscrn_ytop,p->htscrn_xtop+p->htscrn_xsize,p->htscrn_ysize+p->htscrn_ytop);
   }
   else
   if(maxmem==-1)
   {
    x_grf_mod(3);
    return 0;
   }
  }
#endif //POSIX  
 }

 mouseon();
 //p->activeframe=pushact;
 return 0; //nic nepotrebuju...
}

/* Obsolete - moved to DGI script ....

//clear cache:
void gumujcache(char dukladne)
{
 char *ptr,str[80],httname[80],path[65];
#ifndef POSIX
 struct ffblk ff;
#endif 
 int done,i=0,j=HTTPcache.len,headers=0;

 if(dukladne<2)
  ie_resetbin(&HTTPcache);
 if(!dukladne)
  return;
 outs(MSG_KILL);
 if(!j)
  j=1;
 unlink(HTTPcache.filename);
 ptr=configvariable(&ARACHNEcfg,"CachePath",NULL);
 if(!ptr)
  ptr=cachepath;
 makestr(path,ptr,64);
 strcpy(str,path);
 if(dukladne==2)
 {
  strcat(str,"*.htm");
  j/=2;
 }
 else
 {
  strcat(str,"*.*");
  j*=2;
 }

killheaders:
 done=findfirst(str,&ff,0);
 while(!done)
 {
  if(GUITICK())
   break;
  strcpy(str,path);
  strcat(str,ff.ff_name);
  unlink(str);
  if(dukladne==2) //kill only selected headers
  {
   makehttfilename(str,httname);
   unlink(httname);
  }
  done=findnext(&ff);
  if( (!(i%10) || j<100) && i<=j)
  {
   long prc=(long)(100*i)/j;

   outs(MSG_KILL);
   if(prc<100l && prc>=0l)
    percentbar((int)prc);
  }
  i++;
 }//loop

 if(dukladne<2 && !headers) //kill remaining .htt files from "cache\headers\"
 {
  headers=1;
  strcat(path,"headers\\*.*");
  strcpy(str,path);
  goto killheaders;
 }

 //?
 arachne.cachesize=0;

 if(GLOBAL.abort)
  outs(copyright);
 else
 {
  if(dukladne==2)
  {
   struct HTTPrecord *cacheitem;
   i=0;
   while(i<HTTPcache.len)
   {
    cacheitem=(struct HTTPrecord *)ie_getswap(HTTPcache.lineadr[i]);
    if(cacheitem && findfirst(cacheitem->locname,&ff,0))
    {
     memmove(&(HTTPcache.lineadr[i]),&(HTTPcache.lineadr[i+1]),sizeof(int)*(HTTPcache.len-i-1));
     HTTPcache.len--;
    }
    else
     i++;
   }
  }
  outs(MSG_DEAD);
 }
}

#endif //CLEMTEST

*/
