// ========================================================================
// Management of MIME types according to MIME.CFG
// (c)1997-2000 Michael Polak, Arachne Labs (xChaos software)
// ========================================================================

// when compiling 16 bit, should be always linked STATICALY

#include "arachne.h"

int search_mime_cfg(char *rawmime, char *ext, char *cmd)
{
 char *extptr=NULL,line[IE_MAXLEN],*outext=NULL,*pom;
 int viewer=0;
 char mime[STRINGSIZE],*ptr;

 strcpy(ext,"TMP");
 if(!rawmime || !*rawmime)
  return 0; 
  
  //rawmime is lowecase - 1) strlwr() when reading http, 2) strlwr() when searching for local file
  makestr(mime,rawmime,STRINGSIZE-1);
  ptr=strchr(mime,' ');
  if(ptr)
   *ptr='\0';
  ptr=strchr(mime,';');
  if(ptr)
   *ptr='\0';

  if(!strncmpi(mime,"file/",5) && !strchr(mime,'*'))
  {
   //speed up built in etensions
   extptr=strrchr(mime,'.'); //napr. mime=file/../blabla.htm
   if(extptr && strstr("HTM TXT GIF BMP IKN htm txt gif bmp ikn",&extptr[1]) )
    goto ret;
  }

  MIMEcfg.y=0;
  while(MIMEcfg.y<MIMEcfg.lines)
  {
   strcpy(line,ie_getline(&MIMEcfg,MIMEcfg.y));

   outext=strchr(line,' ');
   if(outext)
   {
    *outext='\0';
    do
    {
     outext++;
    }
    while(*outext==' ');
   }
   if(!strcmpi(mime,line))
   {
    extptr=strchr(outext,'>');
    if(extptr && strchr(extptr,'|'))
     *extptr='\0';
    else
    {
     extptr=strchr(outext,'|');
     if(extptr)
     {
      *extptr='\0';
      viewer=1;
     }
     else
      extptr=outext; //v nouzi za prikaz povazuji priponu > will be err....
    }
    pom=extptr;
    extptr=outext; //ukazauju na priponu
    outext=&pom[1]; // preskocim | pred cmd
    goto mamji; //napr. mime=text/html
   }

   if((!strncmpi(line,"file",4)||
       !strncmpi(line,"ftp",3)/*||
       !strncmpi(line,"fastfile",8)*/) &&
       !strncmpi(line,mime,3)) //pseudomime
   {
    extptr=strchr(line,'/');
    if(extptr)
    {
     extptr++;
     strlwr(extptr);
     strlwr(mime);
     if(strstr(mime,extptr)) //napr. line=file/.jpg,extptr=.jpg,mime=file/xx.jpg
     {
      if(*outext=='|')
      {
       viewer=1;
       outext++; //preskoci | pred |cmd,
      }
      else
      if( *outext=='>')
       outext++; //preskoci > pred >EXT|cmd..
      else
      if(!cmd)
      {
       extptr=outext;
       goto mamji;
      }
      pom=strrchr(extptr,'.');
      if(pom)
       extptr=pom+1;
      goto mamji;
     }
    }
   }//end if pseudomime

   MIMEcfg.y++;
  }


 if(cmd) // zadny konverzni prikaz neni k dispozici, extenzi nechci...!
  return 0;

 extptr=strrchr(rawmime,'.'); //napr. mime=file/../blabla.htm
 if(extptr)
 {
  ret:
  makestr(ext,&extptr[1],3);
  strupr(ext);
 }
#ifdef POSIX
 //in Unix-like systems, text files are usually without extension:
 else
  strcpy(ext,"TXT");
#endif
 return 0;


 mamji:
 makestr(ext,extptr,3);

 if(cmd)
 {
  if(!viewer)
  {
   extptr=strchr(outext,'|');
   if(extptr)
   {
    *extptr='\0';
    makestr(ext,outext,3);
    outext=&extptr[1];
   }
   else
    return 0; //nothing to do
  }
  strcpy(cmd,outext); //strcpy() is safe, because outtext is null-terminated
  return 1+viewer;
 }
 return 0;

}

void get_extension(char *mime, char *ext)
{
 search_mime_cfg(mime,ext, NULL);
}

int call_plugin(char *mime, char *command, char *newext)
{
 return search_mime_cfg(mime, newext, command);
}

