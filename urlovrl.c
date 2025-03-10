
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
 strcpy(httname,locname);
 strcat(httname,".http");
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

#if defined (POSIX)
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
  // tr.: I reset coordinate where the document was displayed
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
    strcpy(cacheitem->locname, cachepath);
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
//!!glennmcc: Mar 18, 2006 -- check the drive containing 'cachepath'
 cachefull=lastdiskspace(cachepath)<user_interface.mindiskspace;
// cachefull=lastdiskspace(cacheitem->locname)<user_interface.mindiskspace;
#endif

 HTTPcache.cur=0;
 while(HTTPcache.cur<HTTPcache.len)
 {
  cacheptr=(struct HTTPrecord *)ie_getswap(HTTPcache.lineadr[HTTPcache.cur]);
  if(!cacheptr)
//!!glennmcc: Mar 03, 2007 -- return instead of crashing
return 0;
//    MALLOCERR();
//!!glennmcc: end

  if(!strcmp(cacheptr->URL,cacheitem->URL))
  {
   if(!cachefull)
   {
    //pokud nejde o NUL, tak pouziju stejne jmeno souboru...
    // tr.: in case that it is not NUL, I use the same file name...
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
//!!glennmcc: Mar 03, 2007 -- return instead of crashing
return 0;
//    MALLOCERR();
//!!glennmcc: end

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
 swapmod=1;//zapsal jsem do swapu... (tr.: I have written to swap)

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
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return;
//   MALLOCERR();
//!!glennmcc: end

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
//!!glennmcc: Mar 03, 2007 -- return instead of crashing
return;
//   MALLOCERR();
//!!glennmcc: end

 cacheptr->URL[0]='\0';
 cacheptr->rawname[0]='\0';
 cacheptr->lastseen=0l;
//!!glennmcc: Feb 14, 2008 -- decrement counter when deleting from cache
if(unlink(cacheptr->locname)==0)
   HTTPcache.len--;
//unlink(cacheptr->locname);//original line
//!!glennmcc: end
 strcpy(cacheptr->locname,"NUL");
 swapmod=1;
}

#define MAXCONV 624

// =====================================================================
// Searching HTML page for missing images  ( jpeg  images)
// =====================================================================
//char reload: FIND_MISSING_IMAGE | EXPIRE_ALL_IMAGES

//!!JdS 2005/08/15 : Rewritten so as to use a temporary heap buffer
//instead of p->buf, since under some circumstances, the latter did
//not allow sufficient space for "$roura$.bat" for the conversion of
//all JPG and/or PNG images on a web page. The size required for the
//"$roura$.bat" buffer depends both on the number of images requiring
//conversion, and the path name length of the Arachne directory and
//the Temporary directory ...

char NeedImage(char reload, XSWAP *from)
{
 struct Url url;
 struct picinfo *img;
 unsigned status;
 XSWAP uptr, currentHTMLatom = p->firstHTMLatom, IMGatom;
 XSWAP imageptr[MAXCONV];
 int found, converting = 0, willconvert = 0;
 int maxmem = 1; //,frameID;
// struct ffblk ff;
// char pushact;
 char command[120], ext[5], *cmdbuf;  //JdS
 char type;
 char *URLptr;
 struct HTMLrecord *atomptr;
 struct HTTPrecord HTTPdoc;

/* if(arachne_will_download_images_now)
 {
  maybe later...

 }*/


 if (from && *from!=IE_NULL && reload==FIND_MISSING_IMAGE)
  currentHTMLatom = *from;

 if (reload==EXPIRE_ALL_IMAGES)
  outs(MSG_DELAY0);
 else
 {
  if (!GLOBAL.nowimages && !config_get_bool("LoadImages", 1))
   return 0;
  outs(MSG_VERIFY);
 }

 //pushact=p->activeframe;
 while (currentHTMLatom!=IE_NULL)
 {
// doesn't help anyway  kbhit();
#ifndef NOTCPIP
  Backgroundhttp();
#endif
  atomptr = (struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if (!atomptr)
  {
   if (from)
    return NeedImage(FIND_MISSING_IMAGE,NULL);
   else
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return 0;
//    MALLOCERR();
//!!glennmcc: end
  }

  IMGatom = currentHTMLatom;
  if (currentHTMLatom==atomptr->next)
   currentHTMLatom = IE_NULL; //avoid looping
  else
   currentHTMLatom = atomptr->next;
  type = atomptr->type;

//!!glennmcc: Jan 19, 2003 -- added support for 'BGSOUND'
//!!glennmcc: AUG 05, 2011 -- added support for HTML5 'AUDIO' & 'VIDEO'
  if (type==IMG || type==EMBED || type==BGSOUND || type==BACKGROUND ||
      type==TD_BACKGROUND || type==STYLESHEET || type==AUDIO || type==VIDEO)
  {
   if (atomptr->yy>p->htmlframe[atomptr->frameID].posY &&
       atomptr->xx>p->htmlframe[atomptr->frameID].posX &&
       atomptr->y<p->htmlframe[atomptr->frameID].posY+p->htmlframe[atomptr->frameID].scroll.ysize &&
       atomptr->x<p->htmlframe[atomptr->frameID].posX+p->htmlframe[atomptr->frameID].scroll.xsize)
    GLOBAL.imagevisible = 1;
   else
    GLOBAL.imagevisible = 0;
//   frameID=atomptr->frameID;

   if (type==STYLESHEET)
   {
    URLptr = ie_getswap(atomptr->ptr);
    if (!URLptr)
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return 0;
//     MALLOCERR();
//!!glennmcc: end
   }
   else
   {
    img = (struct picinfo *)ie_getswap(atomptr->ptr);
    if (img)
     URLptr = img->URL;
    else
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return 0;
//     MALLOCERR();
//!!glennmcc: end
   }

   if (!URLptr[0])
   {
//    printf("[object URL is void]");
    continue; //continue with loop
   }
//!!glennmcc: Nov 02, 2005 -- if CSS is enable, grab CSS file if it exists
//!!glennmcc: Nov 22, 2005 -- the experiment did not work correctly :(
//I'll leave this here but commented-out for others to play with ;-)
/*
if(type!=STYLESHEET &&
   configvariable(&ARACHNEcfg,"LoadImages",NULL)[0]=='N' &&
   GLOBAL.nowimages!=IMAGES_LOAD
  ) break;
*/
//!!glennmcc: end

   AnalyseURL(URLptr,&url,IGNORE_PARENT_FRAME);

//!!glennmcc: Jan 06, 2006 -- do not auto-download embeded .SWf files
if(type==EMBED && !strncmpi(&URLptr[strlen(URLptr)-4],".SWF",4)) found=1; else
//!!glennmcc: end

   found = SearchInCache(&url,&HTTPdoc,&uptr,&status);

   if (reload==EXPIRE_ALL_IMAGES && found && status!=LOCAL)
   {
    if (!ie_getswap(uptr))
     uptr = Write2Cache(&url,&HTTPdoc,1,0);
    DeleteFromCache(uptr);
   }
   else
    if (reload==FIND_MISSING_IMAGE)
    {
     if (!found && status==REMOTE && GLOBAL.nowimages!=IMAGES_SEEKCACHE)
     {
      strcpy(GLOBAL.location,HTTPdoc.URL);
      GLOBAL.isimage = 1;
      mouseon();
//       p->activeframe=pushact;
      if (from && currentHTMLatom!=IE_NULL)
       *from = currentHTMLatom;
      return 1;

      /* maybe later ...
      arachne_will_download_images_now++;
      if(arachne_will_download_images_now>LIM_DOWNLOAD)
      {
       return NeedImage(FIND_MISSING_IMAGE,NULL);
      }
      */

     }
     else //conversion only
      if (GLOBAL.nowimages!=IMAGES_SEEKCACHE && type!=STYLESHEET &&
	  (search_mime_cfg(HTTPdoc.mime, ext,command)==1 ||
	  type==EMBED && search_mime_cfg(HTTPdoc.mime, ext, command)==2
//!!glennmcc: Jan 19, 2003 --- added support for 'BGSOUND'
	  || type==BGSOUND && search_mime_cfg(HTTPdoc.mime, ext, command)==2
//!!glennmcc: AUG 05, 2011 -- added support for HTML5 'AUDIO' & 'VIDEO'
	  || type==AUDIO && search_mime_cfg(HTTPdoc.mime, ext, command)==2
	  || type==VIDEO && search_mime_cfg(HTTPdoc.mime, ext, command)==2)

	  && willconvert<MAXCONV)
       imageptr[willconvert++] = IMGatom;
    } //endif

  } //endif
 } //loop


 if (from && *from!=IE_NULL) //we have to verify if we don't need to convert images...
  return NeedImage(FIND_MISSING_IMAGE,NULL);

/*
 maybe later
 if (arachne_will_download_images_now)
  return NeedImage(FIND_MISSING_IMAGE,NULL);
*/

 mouseoff();

//JdS {
#ifdef EXPMAX
#define CMD_BUF_SIZE 49152L
#else
#define CMD_BUF_SIZE 32768L
#endif
#ifdef POSIX
 cmdbuf = malloc(CMD_BUF_SIZE);
#else
 cmdbuf = farmalloc(CMD_BUF_SIZE);
#endif
 if (cmdbuf == NULL)
  memerr();
//JdS }

#ifdef LINUX //JdS (was POSIX)
 cmdbuf[0] = '\0';
#else
 strcpy(cmdbuf,"@echo off\n");
#endif


 if (willconvert) //prepare for conversion
 {
  int mode, f = -1, i = 0;
  #ifdef EXPMAX
  unsigned int cmdlen = 0;
  #else
  int cmdlen = 0;
  #endif
  char *pom;

  while (i<willconvert)
  {
   atomptr = (struct HTMLrecord *)ie_getswap(imageptr[i]);
   if (!atomptr)
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return 0;
//    MALLOCERR();
//!!glennmcc: end

//   frameID=atomptr->frameID;
   uptr = atomptr->ptr;
   type = atomptr->type;
   img = (struct picinfo *)ie_getswap(uptr);
   if (img)
   {
    AnalyseURL(img->URL,&url,IGNORE_PARENT_FRAME);
    SearchInCache(&url,&HTTPdoc,&uptr,&status);

    // if raw=.JPG AND loc=.JPG then ...
    if (!strcmp(HTTPdoc.locname,HTTPdoc.rawname) &&
        search_mime_cfg(HTTPdoc.mime, ext, command)==1
        || type==EMBED //)
//!!glennmcc: Jan 19, 2003 --- added support for 'BGSOUND'
	|| type==BGSOUND
//!!glennmcc: AUG 05, 2011 -- added support for HTML5 'AUDIO' & 'VIDEO'
	|| type==AUDIO || type==VIDEO)
    {
     if (!ie_getswap(uptr))
     {
      char tmpstr[80];
      makestr(tmpstr,HTTPdoc.rawname,79);
      uptr = Write2Cache(&url,&HTTPdoc,0,1); //create filename
      makestr(HTTPdoc.rawname,tmpstr,79);
     }

     pom = strrchr(HTTPdoc.locname,'.');
     if (pom)
     {
      strcpy(&pom[1],ext);
      sprintf(HTTPdoc.mime,"file/.%s",ext);

      if (file_exists(HTTPdoc.rawname) && !strstr(cmdbuf,HTTPdoc.rawname) && //exclude dupes...
          cmdlen<CMD_BUF_SIZE-300)  //JdS (was BUF-IE_MAXLEN)
      {
        //we will try to convert image, if there is enough space
        //in $roura$.bat (means "pipe"), and if the image is already not
        //in converting queue:
        mode = make_cmd(command, &(cmdbuf[cmdlen]), HTTPdoc.URL, "\0","\0",
                        HTTPdoc.rawname, HTTPdoc.locname);

        strcat(&(cmdbuf[cmdlen]),"\n");
        cmdlen += strlen(&(cmdbuf[cmdlen]));

        //touch(HTTPdoc.locname) - mainly for Write2Cache
        f = a_fast_open(HTTPdoc.locname,O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
        if (f>=0)
   	 a_close(f);

        if (maxmem && mode>maxmem || mode<=0)
         maxmem = mode;
        converting++;
      } //if (file_exists ...)
     } //if (pom) nothing to convert

     UpdateInCache(uptr,&HTTPdoc);
    } //if (!strcmp ...)
    else
     if (GLOBAL_justrestarted)
     {
      //we just attempted to convert, we were seeking cache for
      //converted image, but nothing was found. We won't convert
      //the picture again...

      if (!ie_getswap(uptr))
       uptr = Write2Cache(&url,&HTTPdoc,0,0);
      strcpy(HTTPdoc.locname,"NUL");
      HTTPdoc.rawname[0] = '\0';
      UpdateInCache(uptr,&HTTPdoc);
     }
   } //if (img)
   else
//!!glennmcc: Mar 03, 2007 -- too many atoms, return instead of crashing
//"Page too long !" message will then be displayed
//and the incomplete page can be viewed.
return 0;
//    MALLOCERR();
//!!glennmcc: end

   i++;
  }//loop
 }//endif prepare for conversion


 if (converting)
 {
  char msg[128];
  sprintf(msg,MSG_CONVI,converting,MSG_DELAY1,ctrlbreak);
  outs(msg);

  MemInfo(NORMAL);
  arachne.target = 0;
#ifndef POSIX
  //JdS : The command buffer memory will be freed shortly, so ...
  if (maxmem > 0 && farcoreleft()+CMD_BUF_SIZE > ((long)maxmem+10L)*1024L)
  {
   willexecute(cmdbuf);
   farfree(cmdbuf);  //JdS
#endif
#ifndef NOTCPIP
   FinishBackground(BG_FINISH);
#endif

   outs(msg);
#ifdef POSIX
   printf("Executing command:\n%s\n",cmdbuf);
   system(cmdbuf);
#else
   rouraname(command);
   system(command);
#endif

   GLOBAL.nowimages = IMAGES_SEEKCACHE;
   GLOBAL.needrender = 1;

#ifndef POSIX
  } //if (maxmem ...)
  else
  {
   if (maxmem==-1)
    closebat(cmdbuf,RESTART_REDRAW);
   else
    closebat(cmdbuf,RESTART_KEEP_GRAPHICS);
   GLOBAL.willexecute = willexecute(cmdbuf);
   farfree(cmdbuf);  //JdS
#ifndef NOTCPIP
   FinishBackground(BG_FINISH);
#endif
   outs(msg);
   gotoxy(1,8);
   mouseoff();
   if (maxmem==-2)
   {
    x_setfill(0,0);
    x_bar(p->htscrn_xtop, p->htscrn_ytop,
          p->htscrn_xtop+p->htscrn_xsize,
          p->htscrn_ysize+p->htscrn_ytop);
   }
   else
    if (maxmem==-1)
    {
     x_grf_mod(3);
     return 0;
    }
  } //else (if (maxmem ...))
 } //if (converting)
//JdS {
 else
   farfree(cmdbuf);
#else //ifndef POSIX
 } //if (converting)
 free(cmdbuf);
#endif //ifndef POSIX
//JdS }

 mouseon();
 //p->activeframe=pushact;
 return 0; //nic nepotrebuju... (tr.: I do not need anything)
} //NeedImage()

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

#endif

*/
