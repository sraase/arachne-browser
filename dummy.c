
#include "arachne.h"
#include "html.h"

/*
** ------------------------------------------------------------------------
** readfunc_t: input funtion. Waits for input of any type (eg. network)
** Returns values: <0: error, 0: eof, >0: actual buffer length
** ------------------------------------------------------------------------
*/

typedef int (*readfunc_t) ( void *buf, int len );

/*
** ------------------------------------------------------------------------
** writefunc_t: output funtion. It is expected to be synchronous.
** Returns values: <0: error, >=0: number of bytes actualy written
** ------------------------------------------------------------------------
*/

typedef int (*writefunc_t) ( void *buf, int len );

/*
** ------------------------------------------------------------------------
** ReadStdin is sample input funtion of type readfunc_t - works with STDIN
** ------------------------------------------------------------------------
*/

int ReadStdin(void *buf, int len)
{
 /*select call will be added here...*/
 return read(STDIN_FILENO,buf,len);
}

/*
** ------------------------------------------------------------------------
** WriteStdout is sample output funtion of type writefunc_t - for STDOUT
** ------------------------------------------------------------------------
*/

int WriteStdout(void *buf, int len)
{
 return write(STDOUT_FILENO,buf,len);
}

/*
** ------------------------------------------------------------------------
** Text2Metafile is sample metafile generator - "parses" plain text file
** very simple, just to demonstrate how metafile will look like. Requires
** last line to be terminated with \n character. Not used in final verHTML component.
** ------------------------------------------------------------------------
*/

#define BUFLEN     8000
#define FONT_XSIZE 8
#define FONT_YSIZE 16

int Text2Metafile(readfunc_t in, writefunc_t out)
{
 char buf[BUFLEN];
 char line[BUFLEN];
 int i,l,linelen=0;
 int metafilelen=0;             /*in this case, XSWAP is simply integer... */
 int metafileanchor=0;
 int metafilelastatom=IE_NULL;
 struct HTMLrecord HTMLatom;
 int specialitemmarker=-1;  /*metafile marker for control items - HTMLrecord*/

 /*let's reset HTMLatom template: */
 HTMLatom.type=TEXT;
 HTMLatom.x=0;                 /*top left corner - horizontal*/
 HTMLatom.y=0;                 /*top left corner - vertical*/
 HTMLatom.yy=FONT_YSIZE;
 HTMLatom.R=0;                 /*default RGB text color is black*/
 HTMLatom.G=0;
 HTMLatom.B=0;
 HTMLatom.data1=3;             /*font size 3 - actualy, in this example, font size is forced*/
 HTMLatom.data2=FIXED;         /*fixed font - again, this is not really used in this example*/
 HTMLatom.linkptr=IE_NULL;     /*this atom is not hyperlink*/
 HTMLatom.next=IE_NULL;        /*this probably won't be really used in metafile (?)*/

 /*read plain text and output very simple example of metafile*/ 
 while((l=in(buf,BUFLEN))>0)
 {
  i=0;
  while(i<=l)
  {
   if(buf[i]!='\n')
    line[linelen++]=buf[i];
   else
   {
    line[linelen]='\0';
    metafilelen+=out(&linelen,sizeof(int));  /*every item in metafile is started by length info*/
    metafileanchor=metafilelen;
    metafilelen+=out(line,linelen);
    HTMLatom.ptr=metafileanchor; /*this is important - it determines where to search for data */
    HTMLatom.datalen=linelen;
    HTMLatom.xx=FONT_XSIZE*linelen;
    HTMLatom.prev=metafilelastatom; /*this can be easily set, but it's probably useless.. */
    metafilelen+=out(&specialitemmarker,sizeof(int));  /* announce control item */
    metafilelastatom=metafilelen; /* just to be able to set next HTMLatom.prev ...*/
    metafilelen+=out(&HTMLatom,sizeof(struct HTMLrecord));  /* announce control item */
    HTMLatom.y=HTMLatom.yy;
    HTMLatom.yy+=FONT_XSIZE;
    linelen=0;
   }
   i++;
  }  
 }/*loop*/

 if(l<0)
  return 0; /* error! */
 else
  return 1; /* ok...  */
}

/*
** ------------------------------------------------------------------------
** This is function prototype for future HTML parser. Not yet implemented.
** ------------------------------------------------------------------------
*/

int Html2Metafile(readfunc_t in, writefunc_t out)
{
 return 0;
}


int main(void)
{
 return Text2Metafile(ReadStdin,WriteStdout);
}


