
// ========================================================================
// HTML rendering routines for Arachne - part 1/2 of CSIM implemntation
// (c)1997,1998,1999 Arachne Labs (xChaos software)
// ========================================================================

#include "arachne.h"
#include "html.h"

//This function is required to link IMG type=USEMAP and USEMAP atoms
//with MAP atoms. IMG linkptr will be redirected from USEMAP directly
//to map, to accelerate USEMAP detection

//implementation

//. Clickable map ( a link behind the image )
void LinkUSEMAPs(void)
{
 unsigned nextHTMLatom,pom,nextpom,currentHTMLatom=firstHTMLatom;
 struct HTMLrecord *atomptr;


 outs(MSG_USEMAP);
 while(currentHTMLatom!=IE_NULL)
 {
  atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
  if(atomptr)
  {
   nextHTMLatom=atomptr->next;
   if(atomptr->type==IMG && atomptr->data1==2) //<IMG USEMAP>
   {
    char *mapname,atomptrtype;
    atomptr=(struct HTMLrecord *)ie_getswap(atomptr->linkptr);
    atomptrtype=atomptr->type;
    mapname=ie_getswap(atomptr->ptr);

    if(mapname[0]=='#' && atomptrtype==USEMAP)
    //only relative links to the same document are accepted:
    {
     //we need to update atomptr->linkptr - we will search document for
     //matching MAP tag:
     pom=firstHTMLatom;
     strcpy(text,&mapname[1]);
     while(pom!=IE_NULL)
     {
      atomptr=(struct HTMLrecord *)ie_getswap(pom);
      nextpom=atomptr->next;
      if(atomptr->type==MAP) //<IMG USEMAP>
      {
       mapname=ie_getswap(atomptr->ptr);
       if(!strcmpi(mapname,text))
       {
        atomptr=(struct HTMLrecord *)ie_getswap(currentHTMLatom);
        atomptr->linkptr=pom;
        swapmod=1;
        goto searchnextUSEMAP;
       }
      }
      pom=nextpom;
     }//loop
    }
   }//end if <IMG USEMAP>
  }
  else
   MALLOCERR();
  searchnextUSEMAP:
  currentHTMLatom=nextHTMLatom;
 }//loop
}

void USEMAParea(char basetarget)
{
 char target=findtarget(basetarget);
 unsigned arealink=IE_NULL; // "NOHREF"
 char shape=RECT;
 int *array,maxcoord=4,n=0;
 char *ptr,*comma,*tagarg;
 struct HTMLrecord HTMLatom;


 if(getvar("HREF",&tagarg))
 {
  //vlozit link:
  if(tagarg[0]!='#')
  {
   struct Url *url=farmalloc(sizeof(struct Url));
   if(!url)
    memerr();
   AnalyseURL(tagarg,url,currentframe); //(plne zneni...)
   url2str(url,text);
   tagarg=text;
   farfree(url);
  }

  //vyrobim si pointr na link, a od ted je vsechno link:
  addatom(&HTMLatom,tagarg,strlen(tagarg),HREF,BOTTOM,target,0,IE_NULL,1);
  arealink=lastHTMLatom;
 }

 if(getvar("SHAPE",&tagarg))
 {
  if(!strncmpi(tagarg,"RECT",4))
   shape=RECT;
  else if(!strncmpi(tagarg,"POLY",4))
   shape=POLY;
  else if(!strncmpi(tagarg,"CIRC",4))
   shape=CIRCLE;
  else
   shape=DEFAULT;
 }

 if(getvar("COORDS",&tagarg))
  ptr=tagarg;
 else
 {
  strcpy(text,"-1,-1,-1,-1");
  ptr=text;
 }

 if(shape==POLY)
  maxcoord=1000;

 array=farmalloc((maxcoord+2)*sizeof(int));
 if(!array)
  memerr();

 while(ptr && n<maxcoord)
 {
  while(*ptr==' ')
   ptr++;
  comma=ptr;
  while(*comma && *comma!=',' && *comma!=' ')
   comma++;
  if(*comma)
  {
   *comma='\0';
   comma++;
  }
  else
   comma=NULL;
  array[n++]=atoi(ptr);
  ptr=comma;
 }
 array[n]=-1;
 array[n+1]=-1;

 addatom(&HTMLatom,array,sizeof(int)*(n+2),AREA,BOTTOM,shape,0,arealink,1);
 farfree(array);

}
