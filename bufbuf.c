/***********************************************************************/
/* Funkce pro odkladani a zpetne nacitani bufferu do XMS nebo na disk  */
/***********************************************************************/
/*                                             (c) Jan Vlaciha 1994    */

//modified by M.Polak, xChaos software, 1996

#include <conio.h>
#include <dir.h>
#include <time.h>
#include <stdlib.h>
#include <alloc.h>
#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys\stat.h>

#include "ima.h"
#include "bufbuf.h"
#include "messages.h"
#include "a_io.h"

#include "str.h"

void memerr(void);

char *tempdir=NULL;


void tempinit(char *path)
{
 if(!tempdir)
 {
  char *ptr=getenv("ARACHNETEMP");
  char str[64];

  tempdir=farmalloc(64);
#ifndef MINITERM
  if(!tempdir)
   memerr();
#endif
  tempdir[0]='\0';

  if(!ptr)
  {
   ptr=getenv("TEMP");
   if(ptr && strlen(ptr)<48)
   {
    strcpy(str,ptr);
    strcat(str,"\\ARACHNE.TMP");
    mkdir(str);
    ptr=str;
   }
  }

  if(ptr)
  {
   char str[128];
   int f;

   pathstr(tempdir,ptr); //pathcpy is str.h function

   sprintf(str,"%sdonotpan.ic!",tempdir);
   f=a_open(str,O_WRONLY|O_BINARY|O_CREAT|O_TRUNC,S_IWRITE);
   if(f>=0)
   {
    write(f,MSG_DELAY1,strlen(MSG_DELAY1)); // ;-)
    a_close(f);
   }
   f=a_open(str,O_RDONLY|O_BINARY,0);
   if(f<0)
    tempdir[0]='\0';
   else
    a_close(f);
  }
 }
 strcpy(path,tempdir);
}


//#define BUFDEBUG

int h_xmove(XMOVE *p);      // umoznuje odkladat buffery liche delky

//extern char scratchBase[60];  // scratch adresar

/***********************************************************************/
/*              Odlozeni bufferu do XMS nebo na disk   		       */
/***********************************************************************/
// Parametry :  inbuf .... odkladany buffer
//              lenbuf ... delka v bytech
//              bb ..... pointer pro identifikaci (viz getIm)
// Return    :  1 ........ O.K.
//              2 ........ chyba pri alokaci
//              4 ........ chyba pri praci s XMS
//              6 ........ chyba pri psani na disk

// Funkce    :  Odlozi buffer do XMS nebo (kdyz neni XMS) na disk.
//              Soubory na disku se poznaji podle pripony ._$B.
//              Predpoklada se existence a nastaveni globalni promenne
//              scratchBase se jmenem pracovniho adresare.
//              Odkladat lze buffery do maximalni delky 64kB.

char DisableXMS=0;

/***********************************************************************/
/*             Zjisteni delky odlozeneho bufferu    		       */
/***********************************************************************/
// Parametry :  bbuf ..... pointer pro identifikaci
// Return    :  delka odlozeneho bufferu

unsigned int sizeBuf( struct T_buf_buf *bbuf )
{
//   if( *bbuf == NULL ) return(0);
//   return( ((struct T_buf_buf *)*bbuf)->size );
 return( bbuf->size );
}

//!!Bernie: Mar 02, 2007
/*
Below are rewritten functions from bufbuf.c, I've rewritten them so there
are no gotos left. As a side effect the saveBuf function should be
somewhat faster if there isn't enough memory.
*/

int saveBuf( char *inbuf, unsigned int lenbuf, struct T_buf_buf *bb )
{
   int  ist;
   char mem;

   ist = get_xmem();
   mem = 4;
   if(ist >= 0x0200 && !DisableXMS)
     mem = 1; // HIMEM.SYS O.K.

   if(mem == 1)
   {
     int kby;
//!!Bernie: Mar 04, 2007
//!!glennmcc: Mar 04, 2007 -- this test failed
//     kby  = lenbuf;
   kby  = (lenbuf + 1023) / 1024;                   // from B to kB
//!!Bernie: end
     bb->handle = alloc_xmem(kby);
     if(bb->handle != -1)
     {
       XMOVE             xmove;
       xmove.length    = (long)lenbuf;
       xmove.sourceH   = 0;
       xmove.sourceOff = ptr2long(inbuf);
       xmove.destH     = bb->handle;
       xmove.destOff   = 0;

       if(h_xmove(&xmove) < 1)
       {
	 DisableXMS = 1;
	 mem = 4;
       }
     }
     else
     {
       DisableXMS = 1;
       mem = 4;
     }
   }

//!!Bernie: **** warning *** Do NOT change this into a "else if"
   if(mem == 4)
   {
     char         fname[80], pomstr[20];
     struct ffblk ffblk;
     int h;

     do
     {
       tempinit(fname);

       bb->handle = (int)clock();
       itoa(bb->handle, pomstr, 10);
       strcat(fname, pomstr);
       strcat(fname, "._$B");
     }while(!findfirst( fname, &ffblk, 0));

//!!Bernie: Mar 04, 2007 --- rewite
//old code
/*
     h = a_open(fname, O_CREAT | O_RDWR | O_BINARY, S_IWRITE);
     if(h < 0)
      return(6);

     if(write(h, inbuf, lenbuf) != lenbuf)
      return(6);
*/
//new code
     h = a_open(fname, O_CREAT | O_RDWR | O_BINARY, S_IWRITE);
     if(h < 0)
     {
      bb->handle = NULL;
      return(6);
     }

     if(write(h, inbuf, lenbuf) != lenbuf)
     {
      bb->handle = NULL;
      a_close(h);
      unlink(fname);
      return(6);
     }
//!!Bernie: end

     a_close(h);
   }

   bb->size   = lenbuf;
   bb->medium = mem;
   return(1);
}

int fromBuf( char *outbuf, unsigned int from,
	 unsigned int *lenbuf, struct T_buf_buf *bb )
{
   int               ist;
   unsigned int      d;
   char              mem;

   if( bb->size <= from )
   {
     *lenbuf = 0;
     return(1);
   }

   d = min( *lenbuf, bb->size - from );
   *lenbuf = d;
   mem = bb->medium;

   if( mem == 1 )
   {
     XMOVE             xmove;
     xmove.length    = (long)d;
     xmove.sourceH   = bb->handle;
     xmove.sourceOff = (long)from;
     xmove.destH     = 0;
     xmove.destOff   = ptr2long(outbuf);

     if(h_xmove(&xmove) < 1)
       return(4);
   }
   else
   {
     char fname[80], pomstr[20];
     int h;

     tempinit(fname);
     //strcpy( fname, scratchBase );
     itoa( bb->handle, pomstr, 10);
     strcat( fname, pomstr);
     strcat( fname, "._$B");
     h = a_open( fname, O_RDONLY | O_BINARY,0);
     if ( h < 0 )
       return(6);

     if ( from ) // shift
     {
       if( a_lseek( h, (long)from, SEEK_SET) < 0 )
	 return(6);
     }
     ist = a_read( h, outbuf, d);
     if ( ist != d  )
     {
       if ( ist < 0 )
	 return(6);
       *lenbuf = d;
     }
     a_close( h );
   }
   return(1);
}

//!!Bernie: Mar 04, 2007
//the documentation for delBuf isn't correct. it never returns 2, the
//precondition for the function is that bb isn't NULL. On the other hand
//we never check the result anyway so we should rewrite it (once again) into:
/***********************************************************************/
/*                  Zruseni odlozeneho bufferu                  */
// tr.: Delete saved buffer
/***********************************************************************/
// Parameters:  bb ..... pointer for identification (see getIm), != NULL
void delBuf( struct T_buf_buf *bb )
{
   if(!bb->handle)
    return;
   if( bb->medium == 1 )
    dealloc_xmem( bb->handle );
   else
   {
     char fname[80], pomstr[20];

     tempinit(fname);
     itoa( bb->handle, pomstr, 10);
     strcat( fname, pomstr);
     strcat( fname, "._$B");
     unlink( fname );
   }
}

//old/new function ;-)
/*
int delBuf( struct T_buf_buf *bb )
{
   char              mem;
   mem = bb->medium;
   if( mem == 1 )
   {
     if (dealloc_xmem( bb->handle ) !=1 )
       return(6);
   }
   else
   {
     char fname[80], pomstr[20];
     tempinit(fname);
     itoa( bb->handle, pomstr, 10);
     strcat( fname, pomstr);
     strcat( fname, "._$B");
     if ( unlink( fname ) < 0 )
       return(6);
   }
   return(1);
}
*/
//!!Bernie: end

//old functions retained below.....
/*
//int saveBuf( char *inbuf, unsigned int lenbuf, char far **bbuf )
int saveBuf( char *inbuf, unsigned int lenbuf, struct T_buf_buf *bb )
{
   int               ist, kby, ret, h;
	   char              mem, fname[80], pomstr[20];
   struct ffblk      ffblk;
   XMOVE             xmove;
//   struct T_buf_buf  *bb = NULL;

//   bb = farmalloc( sizeof(struct T_buf_buf) );
//   if ( bb == NULL ) return(2);

Retry:

   mem = 4;
   ist = get_xmem();
   if(ist >= 0x0200 && !DisableXMS) mem = 1; // HIMEM.SYS O.K.

   if( mem == 1 )
   {
     kby  = (lenbuf + 1023) / 1024;                   // z B na kB
     bb->handle = alloc_xmem(kby);
     if ( bb->handle == -1)
     {
      mem = 4;
      DisableXMS=1;
      goto Disk;
     }

     xmove.length    = (long)lenbuf;
     xmove.sourceH   = 0;
     xmove.sourceOff = ptr2long(inbuf);
     xmove.destH     = bb->handle;
     xmove.destOff   = 0;

//     ist = move_xmem(&xmove);
//asm cli;
     ist = h_xmove(&xmove);
//asm sti;

#ifdef BUFDEBUG
     printf("----------------->Handle %d: (written %u) ret=%d)\n",bb->handle,lenbuf,ist);
#endif

     if( ist<1 )
     {
      DisableXMS=1;
      goto Retry;
     }
   }

Disk:
   if( mem == 4 )
   {
     do
     {
       tempinit(fname);

       bb->handle = (int)clock();
       itoa( bb->handle, pomstr, 10);
       strcat( fname, pomstr);
       strcat( fname, "._$B");
     } while ( !findfirst( fname, &ffblk, 0) );

     h = a_open( fname, O_CREAT | O_RDWR | O_BINARY, S_IWRITE);
     if ( h < 0 ) goto ErrWrite;

     ist = write( h, inbuf, lenbuf);
     if ( ist != lenbuf ) goto ErrWrite;

     a_close( h );
   }

   bb->size   = lenbuf;
   bb->medium = mem;
//   *bbuf = (char far *)bb;
   return(1);

ErrWrite:
   ret = 6;
   goto Ko;

Ko:
//   if ( bb != NULL ) farfree(bb);
   return(ret);
}
*/

/***********************************************************************/
/*                  Cetba z odlozeneho bufferu   		       */
/***********************************************************************/
// Parametry :  outbuf ... odkladany buffer
//              from ..... od ktereho bytu cist
//              lenbuf ... vstup  - kolik bytu se ma precist
//                         vystup - kolik bytu se skutecne precetlo
//              bb ..... pointer pro identifikaci (viz getIm)
// Return    :  1 ........ O.K.
//              2 ........ bbuf je NULL
//              4 ........ chyba pri praci s XMS
//              6 ........ chyba pri psani na disk

// Funkce    :  Nacte pozadovanou cast odlozeneho bufferu.
//              Nepodari-li se nacist pozadovanou delku,
//              vrati se v lenbuf skutecne nactena delka.

/*
int fromBuf( char *outbuf, unsigned int from,
	     unsigned int *lenbuf, struct T_buf_buf *bb )
{
   int               ist, /*ret,*/ /* h;
___________________________________^^<-- added for commenting-out old functions
   unsigned int      d;
   char              mem, fname[80], pomstr[20];
   XMOVE             xmove;
//   struct T_buf_buf  *bb;

//   bb = (struct T_buf_buf *)*bbuf;
//   if( bb == NULL ) return(2);

   if( bb->size <= from )
   {
     *lenbuf = 0;
     goto Ok;
   }

   d = min( *lenbuf, bb->size - from );
   *lenbuf = d;
   mem = bb->medium;

   if( mem == 1 )
   {
     xmove.length    = (long)d;
     xmove.sourceH   = bb->handle;
     xmove.sourceOff = (long)from;
     xmove.destH     = 0;
     xmove.destOff   = ptr2long(outbuf);

//     ist = move_xmem(&xmove);
#ifdef BUFDEBUG
     printf("<--Handle=%d, reading %u",bb->handle,d);
#endif
//asm cli;
     ist = h_xmove(&xmove);
//asm sti;
#ifdef BUFDEBUG
     printf("Ret=%d\n",ist);
#endif


     if( ist<1 ) goto ErrMove;
   }
   else
   {
     tempinit(fname);
     //strcpy( fname, scratchBase );
     itoa( bb->handle, pomstr, 10);
     strcat( fname, pomstr);
     strcat( fname, "._$B");
     h = a_open( fname, O_RDONLY | O_BINARY,0);
     if ( h < 0 )
     {
//	 printf("1.fromBuf: %s, %d", fname, from);
       goto ErrRead;
     }

     if ( from ) // posun
     {
       if( a_lseek( h, (long)from, SEEK_SET) < 0 )
       {
//	 printf("2.fromBuf: %s, %d", fname, from);
	 goto ErrRead;
       }
     }
     ist = a_read( h, outbuf, d);
     if ( ist != d  )
     {
       if ( ist < 0 ) goto ErrRead;
       *lenbuf = d;
     }
     a_close( h );
   }
Ok:
   return(1);

ErrMove:
   return(4);

ErrRead:
   return(6);


//Ko:
//   if ( bb != NULL ) farfree(bb);
//   return(ret);

}
*/

/***********************************************************************/
/*                  Zruseni odlozeneho bufferu   		       */
/***********************************************************************/
// Parametry :  bb ..... pointer pro identifikaci (viz getIm)
// Return    :  1 ........ O.K.
//              2 ........ bb je NULL
//              6 ........ nepodarilo se zrusit

/*
int delBuf( struct T_buf_buf *bb )
{
   int               ist, ret;
   char              mem, fname[80], pomstr[20];
//   struct T_buf_buf  *bb;

//   bb = (struct T_buf_buf *)*bbuf;
//   if( bb == NULL ) return(2);

   mem = bb->medium;

   if( mem == 1 )
   {
     ist = dealloc_xmem( bb->handle );
//     printf("* Dealocating handle %d, ret=%d",bb->handle,ist);
     if ( ist !=1 )  goto ErrDel;
   }
   else
   {
     tempinit(fname);
     //strcpy( fname, scratchBase );
     itoa( bb->handle, pomstr, 10);
     strcat( fname, pomstr);
     strcat( fname, "._$B");
     ist = unlink( fname );
     if ( ist < 0 ) goto ErrDel;
   }

//   if(bb)farfree( bb );
//   *bbuf = NULL;
   return(1);

ErrDel:
   ret = 6;
   goto Ko;

Ko:
//   if ( bb != NULL ) farfree(bb);
   return(ret);
}
*/
