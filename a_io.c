#include "posix.h"
#include "a_io.h"

//#define ENABLE_A_IO

/* The following only matter if ENABLE_A_IO is defined: */
#undef PRINT_TO_SERIAL_PORT
#undef PRINT_READS_TO_SERIAL_PORT
#define DONT_PRINT_PATH
#define ENABLE_FILE_CACHE
#define ENABLE_FILE_BUFFER
#define CHANGE_DIRECTORY
#undef EMULATE_TEXT_MODE


#ifndef ENABLE_A_IO
#if 0
int a_open(char *path, int access, unsigned mode)
{
#ifdef POSIX
 return open(path,access,mode);
#else
 return open(path,access,mode);
#endif
}

int a_sopen(char *path, int access, int shflag, int mode)
{
#ifdef POSIX
 return open(path,access,mode);
#else
 return sopen(path,access,shflag,mode);
#endif
}

int a_fast_open(char *path, int access, unsigned mode)
{
 return open(path,access,mode);
}

int a_fast_sopen(char *path, int access, int shflag, int mode)
{
 return sopen(path,access,shflag,mode);
}


//#define a_read(A,B,C) read(A,B,C)

int a_read(int h, void *buf, unsigned len)
{
	return read(h,buf,len);
}

long a_lseek(int h, long pos, int whence)
{
	return lseek(h,pos,whence);
}

/*
int a_eof(int h)
{
	return eof(h);
}
*/
long a_filelength(int h)
{
	return filelength(h);
}

int a_close(int h)
{
	return close(h);
}

void a_chdir_initilaize(void)
{
}

int a_alloccache(void)
{
 return 1;
}
#endif

#else

/*
**----------------------------------------------------------------------
** A set of procedures to speed up file reads for GIF and BMP files:
**----------------------------------------------------------------------
*/
/* Special handle numbers so the program doesn't do read/writes directly: */
#define CACHE_HANDLE_NUM     (12340)
#define BUF_HANDLE_NUM       (dlpBufFileOffset)  /* (5600) */
int dlpBufFileOffset /* = 12350 */ ;

#define CACHE_BUF_SIZE       (300)
#define NUM_CACHE_ENTRIES    (10)

#ifdef ENABLE_FILE_CACHE
	typedef struct fileCache
	{
		struct fileCache near *next;  /* Keep a sorted list for LRU algorithm */
		struct fileCache near *prev;
		int handle;              /* This files handle */
		unsigned char nameLen;   /* Length of the name */
		unsigned char opens;     /* Number of opens on this cache entry */
		int fileLen;             /* Length of the file */
		int pos;                 /* Read position */
		long readCnt;
		unsigned char buf[CACHE_BUF_SIZE];
	} FILE_CACHE;

//mp:
//	FILE_CACHE dlpFileCache[NUM_CACHE_ENTRIES];
	FILE_CACHE near *dlpFileCache;

	FILE_CACHE near *dlpFCHead;
	unsigned char dlpFCInitFlag;

	#ifdef CHANGE_DIRECTORY
		char chdirPathName[MAXPATH];
		char firstPath[MAXPATH];
	#endif

	unsigned long dlpCacheFills;
	unsigned long dlpCacheHits;
	unsigned long dlpCacheMisses;
	unsigned long dlpCacheNonCacheable;
	unsigned long dlpCacheInvalidates;
	unsigned long dlpBytes;
	char *dlpLastOpenType;

int a_alloccache(void)
{
 dlpFileCache=(FILE_CACHE near *)farmalloc(NUM_CACHE_ENTRIES*sizeof(FILE_CACHE));

 if(!dlpFileCache)
  return 0;
 else
  return 1;
}

	FILE_CACHE near *handleToFC( int h )
	{
		FILE_CACHE near *fc;
		if( (h >= CACHE_HANDLE_NUM)
		 && (h < (CACHE_HANDLE_NUM+NUM_CACHE_ENTRIES)) )
		{
			fc = (FILE_CACHE *)&dlpFileCache[h - CACHE_HANDLE_NUM];
			if( fc->handle == h )
			{
				return fc;
			}
		}
		return NULL;
	}
#endif

#ifdef ENABLE_FILE_BUFFER
	#define MAX_FILE_BUFS (2)
	#define DLP_BUFSIZE   (4096)
	typedef struct dlpFileBuf
	{
		int handle;       /* Current file handle being buffered */
		long bufPos;  /* Current position of beginniing of buffer */
		long tellPos; /* For debugging */
		int bufI;            /* Current position in buffer to extract */
		int bufBytes;
		#ifdef EMULATE_TEXT_MODE
			int access;
		#endif
		long readCnt;
		int bufFileNameLen;
		char bufFileName[MAXPATH];
		unsigned char buf[DLP_BUFSIZE];
	} DLP_FILE_BUF;
	DLP_FILE_BUF dlpFiles[2];

	DLP_FILE_BUF near *handleToFB(int h)
	{
		int i;
		DLP_FILE_BUF near *fb;

		if( h > 0 )
		{
			h -= BUF_HANDLE_NUM;  /* Offset to the real handle number */
		}
		for( i=0,fb=dlpFiles; i<MAX_FILE_BUFS; i++,fb++ )
		{
			if( fb->handle == h )
			{
				return fb;
			}
		}
		return NULL;
	}
#endif

int a_cache_open(char *path, int access, int shflag, int mode);
FILE_CACHE near *fileNameToFC( char *fname );
void invalidateCacheEntry( FILE_CACHE near *fc );
void dlpUpdateNewCacheEntry( DLP_FILE_BUF near *fb );
void sioPrintf( const char *fmt, ... );
void checkDirectory(char *path, int chFlag);

#ifdef PRINT_TO_SERIAL_PORT
	char *callerProc;
#endif

//Arachne-accelerated I/O operations. write() is not accelerated.

int a_open(char *path, int access, unsigned mode)
{
 #ifdef CHANGE_DIRECTORY
  checkDirectory(path, 0);
 #endif

 #ifdef PRINT_TO_SERIAL_PORT
	callerProc = "a_open";
 #endif
 return a_cache_open(path,access,SH_DENYNO,mode);
}

int a_sopen(char *path, int access, int shflag, int mode)
{
 #ifdef CHANGE_DIRECTORY
  checkDirectory(path, 0);
 #endif

 #ifdef PRINT_TO_SERIAL_PORT
	callerProc = "a_sopen";
 #endif
 return a_cache_open(path,access,shflag,mode);
}

int a_fast_open(char *path, int access, unsigned mode)
{
 #ifdef CHANGE_DIRECTORY
  checkDirectory(path, 1);
 #endif

 #ifdef PRINT_TO_SERIAL_PORT
	callerProc = "a_fast_open";
 #endif
 return a_cache_open(path,access,SH_DENYNO,mode);
}

int a_fast_sopen(char *path, int access, int shflag, int mode)
{
 #ifdef CHANGE_DIRECTORY
  checkDirectory(path, 1);
 #endif

 #ifdef PRINT_TO_SERIAL_PORT
	callerProc = "a_fast_sopen";
 #endif
 return a_cache_open(path,access,shflag,mode);
}

#ifdef ENABLE_FILE_CACHE
/*
**----------------------------------------------------------------------
** Cached version of open:
**----------------------------------------------------------------------
*/
int a_cache_open(char *path, int access, int shflag, int mode)
{
 DLP_FILE_BUF near *fb;
 FILE_CACHE near *fc;
 int h;

 fc = fileNameToFC( path );

 if( (access == (O_RDONLY | (int)O_BINARY))
  || (access == (O_RDONLY |      O_TEXT)) )
 {
	if( fc )
	{
		if( fc->opens )
		{
			#ifdef PRINT_TO_SERIAL_PORT
				sioPrintf( "**Double file open! [%s] (%x,%x,%x).\n",
					path, access, shflag, mode );
			#endif
			h = -1; /* Don't let it open twice! */
		}
		else
		{
			fc->pos = 0;
			fc->readCnt = 0;
			fc->opens++;
			dlpCacheHits++;
			h = fc->handle;
		}
	}
	else
	{
		fb = handleToFB(0); /* Get a buffer if one is available */

		#ifdef EMULATE_TEXT_MODE
			if( fb )
			{
				h = sopen( path, O_RDONLY | O_BINARY, shflag, mode );
			}
			else
			{
				h = sopen( path, access, shflag, mode );
			}
		#else
			h = sopen( path, access, shflag, mode );
		#endif

		/* Check to see if we have an available buffer for this file */
		if( (h>0) && fb )
		{
			#ifdef EMULATE_TEXT_MODE
				fb->access = access;
			#endif
			fb->handle   = h;
			fb->bufPos   = 0;
			fb->tellPos  = 0;
			fb->bufBytes = 0;
			fb->bufI     = 0;
			fb->readCnt  = 0;
			fb->bufFileNameLen = strlen(path);
			/* Save the name for a possible new cache entry */
			if( fb->bufFileNameLen < sizeof(fb->bufFileName) )
			{
				memcpy( fb->bufFileName, path, fb->bufFileNameLen+1 );
			}
			h += BUF_HANDLE_NUM;
		}

		#ifdef PRINT_TO_SERIAL_PORT
			if( h <= 0 )
			{
				sioPrintf( "Failure to open file [%s] (%x,%x,%x).\n",
					path, access, shflag, mode );
			}
		#endif
	}
 }
 else
 {
   if( fc )
   {
	#ifdef PRINT_TO_SERIAL_PORT
		sioPrintf( "Invalidating cache entry [%s] (%x,%x,%x).",
			path, access, shflag, mode );
	#endif
   	invalidateCacheEntry(fc);
   }
   dlpCacheNonCacheable++;
   h = sopen(path,access,shflag,mode);
 }
 #ifdef PRINT_TO_SERIAL_PORT
	sioPrintf( "%d=%s(%s,%x,%x,%x);\n",
		h, callerProc, path, access, shflag, mode );
 #endif
 return h;
}
#endif

#if defined(PRINT_TO_SERIAL_PORT) && defined(PRINT_READS_TO_SERIAL_PORT)
  void sioPrintRead( char rtyp, int h, unsigned char *buf, unsigned len, int i )
  {
	int j;
	sioPrintf( "a_read(%c-%d,%p,%d);[%d=",
		rtyp, h, buf, len, i );
	for( j=0; j<i && j<20; j++ )
	{
		sioPrintf( "%02x", buf[j] );
		if( (j & 3) == 3 )
		{
			sioPrintf( "." );
		}
	}
	sioPrintf( "\n" );
  }
#endif

int a_read(int h, void *buf, unsigned len)
{
	int i, togo, totalCopy;
	DLP_FILE_BUF near *fb;

	#ifdef ENABLE_FILE_CACHE
		FILE_CACHE near *fc;
		if( (fc=handleToFC(h)) != NULL )
		{
			i = fc->fileLen - fc->pos;   /* Bytes in buffer */
			if( i > len ) { i = len; }
			memcpy( buf, &fc->buf[fc->nameLen+fc->pos], i );
			fc->pos += i;
			fc->readCnt += i;
			dlpBytes += i;
#if defined(PRINT_TO_SERIAL_PORT) && defined(PRINT_READS_TO_SERIAL_PORT)
	sioPrintRead( 'C', h-CACHE_HANDLE_NUM, buf, len, i );
#endif
			return i;
		}
	#endif

	if( (fb = handleToFB(h)) == NULL )
	{
		i = read( h, buf, len );
#if defined(PRINT_TO_SERIAL_PORT) && defined(PRINT_READS_TO_SERIAL_PORT)
	sioPrintRead( 'R', h, buf, len, i );
#endif
		return i;
	}

#ifdef PRINT_TO_SERIAL_PORT
	/* A debug check to make sure no one else has read or written this file: */
	if( tell(fb->handle) != fb->tellPos )
	{
		sioPrintf( "\n\n***** Error, buffered file (%d) position changed!\n",
			h );
		sioPrintf( "   tell(h)=%ld,  we are at %ld.\n\n",
			tell( fb->handle), fb->tellPos );
	}
#endif

	totalCopy = 0;
	togo = len;
	while( togo > 0 )
	{
		i = fb->bufBytes - fb->bufI;   /* Bytes in buffer */

		/* If buffer is empty, fill it now */
		if( i == 0 )
		{
			/* Read in the next chunk of data */
			/* Update currend position in the file */
			fb->bufPos += fb->bufBytes;
			fb->bufI = 0;
			fb->bufBytes = read( fb->handle, fb->buf, DLP_BUFSIZE );
			fb->tellPos = tell( fb->handle );
			if( fb->bufBytes <= 0 )
			{
				fb->bufBytes = 0;
				break;
			}
			i = fb->bufBytes;   /* Bytes in buffer */
			#ifdef ENABLE_FILE_CACHE
				if( (fb->bufPos == 0)
				 && ((i + fb->bufFileNameLen) <= CACHE_BUF_SIZE) )
				{
					dlpUpdateNewCacheEntry(fb);
				}
			#endif
		}

		if( i >= togo ) { i = togo; }

		memcpy( &((char *)buf)[totalCopy], &fb->buf[fb->bufI], i );
		fb->bufI += i;
		totalCopy += i;
		togo -= i;
	}
	fb->readCnt += totalCopy;
	dlpBytes += totalCopy;

	#ifdef EMULATE_TEXT_MODE
		if( fb->access == (O_RDONLY | O_TEXT) )
		{
			int dstI;

			for( i=0; i<totalCopy; i++ )
			{
				if( ((char *)buf)[i] == '\r' )
				{
					dstI = i;
					for( ++i; i<totalCopy; i++ )
					{
						if( ((char *)buf)[i] != '\r' )
						{
							((char *)buf)[dstI] = ((char *)buf)[i];
							dstI++;
						}
					}
					totalCopy = dstI;
					break;
				}
			}
		}
	#endif

#if defined(PRINT_TO_SERIAL_PORT) && defined(PRINT_READS_TO_SERIAL_PORT)
	sioPrintRead( 'B', h-BUF_HANDLE_NUM, buf, len, totalCopy );
#endif
	return totalCopy;
}

long a_lseek(int h, long pos, int whence)
{
	DLP_FILE_BUF near *fb;
	FILE_CACHE near *fc;

#ifdef PRINT_TO_SERIAL_PORT
	sioPrintf( "a_lseek(%d,%ld,%d)", h, pos, whence );
#endif
	/* First check to see if it is a cache entry */
	#ifdef ENABLE_FILE_CACHE
		if( (fc=handleToFC(h)) != NULL )
		{

			if( whence == SEEK_CUR )
			{
				pos += fc->pos;  /* Calculate relative position */
			}

			if( pos < 0 )
			{
				pos = 0;
			}
			else if( pos > fc->fileLen )
			{
				pos = fc->fileLen;
			}

			fc->pos = (int)pos;
#ifdef PRINT_TO_SERIAL_PORT
	sioPrintf( "=%ld\n", pos );
#endif
			return pos;
		}
	#endif

	if( (fb = handleToFB(h)) == NULL )
	{
		pos = lseek( h, pos, whence );
#ifdef PRINT_TO_SERIAL_PORT
	sioPrintf( "=%ld\n", pos );
#endif
		return pos;
	}

	if( whence == SEEK_CUR )
	{
		pos = fb->bufPos + fb->bufI + pos;
	}

	if( (pos >= fb->bufPos) && (pos < (fb->bufPos + fb->bufBytes)) )
	{
		fb->bufI = (int)(pos - fb->bufPos);
	}
	else
	{
		fb->bufI = 0;
		fb->bufBytes = 0;
		fb->bufPos = pos = lseek( fb->handle, pos, SEEK_SET );
		fb->tellPos = pos;
	}
#ifdef PRINT_TO_SERIAL_PORT
	sioPrintf( "=%ld\n", pos );
#endif
	return pos;
}

long a_filelength(int h)
{
	DLP_FILE_BUF near *fb;

	#ifdef ENABLE_FILE_CACHE
		FILE_CACHE near *fc;
		if( (fc=handleToFC(h)) != NULL )
		{
			return (long)(fc->fileLen);
		}
	#endif

	if( (fb = handleToFB(h)) != NULL )
	{
		return filelength(fb->handle);
	}

	return filelength(h);
}
/*
int a_eof(int h)
{
	DLP_FILE_BUF near *fb;

	#ifdef ENABLE_FILE_CACHE
		FILE_CACHE near *fc;
		if( (fc=handleToFC(h)) != NULL )
		{
			if( fc->pos >= fc->fileLen )
			{
				return 1;
			}
			return 0;
		}
	#endif

	if( (fb = handleToFB(h)) != NULL )
	{
		if( fb->bufI < fb->bufBytes )
		{
			return 0;
		}
		return eof(fb->handle);
	}

	return eof(h);
}
*/
int a_close(int h)
{
	DLP_FILE_BUF near *fb;
	int rval;

	/* First check to see if it is a cache entry */
	#ifdef ENABLE_FILE_CACHE
		FILE_CACHE near *fc;

		if( (fc=handleToFC(h)) != NULL )
		{
#ifdef PRINT_TO_SERIAL_PORT
	sioPrintf( "a_close(%d): cached, opens=%d, readCnt=%ld.\n",
		h, fc->opens, fc->readCnt );
#endif
			if( fc->opens )
			{
				fc->opens--;
			}
			return 0;
		}
	#endif

	if( (fb = handleToFB(h)) != NULL )
	{
#ifdef PRINT_TO_SERIAL_PORT
	sioPrintf( "a_close(%d): buffered(%d), readCnt=%ld.\n",
		h, fb->handle, fb->readCnt );
#endif
		rval = close( fb->handle );
		fb->handle = 0;
		return rval;
	}

#ifdef PRINT_TO_SERIAL_PORT
	sioPrintf( "a_close(%d): raw.\n", h );
#endif
	return close( h );
}

//will include atexit() registration for deinitialization:
void a_chdir_initilaize(void)
{
}

#ifdef PRINT_TO_SERIAL_PORT
#define SIO_ADDR    (0x3f8)
#define TIMER55MS   (*(volatile long int *)0x46c)
char sioPortInitialized;
void initSioPort( void )
{
	int i, j;
	if( sioPortInitialized ) { return; }
	sioPortInitialized = 1;

	for( j=0; j<10; j++ )
	{
		outportb( SIO_ADDR+3, 0x83 );
		outportb( SIO_ADDR+0, 0x03 );  /* 38400 BAUD */
		outportb( SIO_ADDR+1, 0x00 );
		outportb( SIO_ADDR+3, 0x03 );
		outportb( SIO_ADDR+4, 0x03 );
		for( i=0; i<16; i++ )
		{
			inportb( SIO_ADDR + 5 );
			inportb( SIO_ADDR + 0 );
		}
	}
}

void sioPutchar( int c )
{
	long int t;
	int to;
	initSioPort();  /* returns if already done */
	for( t=TIMER55MS,to=0; to<2; )
	{
		if( (inportb(SIO_ADDR+5) & 0x20) != 0 )
		{
			outportb(SIO_ADDR,c);
			break;
		}
		if( t != TIMER55MS )
		{
			to++;
			t = TIMER55MS;
		}
	}
}

void putString( char far *p )
{
	while( *p )
	{
		if( *p == '\n' )
		{
			sioPutchar( '\r' );
		}
		sioPutchar( *p++ );
	}
}

char siobuf[256];
void sioPrintf( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	vsprintf( siobuf, fmt, ap );
	va_end(ap);

	putString( siobuf );
}

unsigned long int dlpStartT, dlpTotalT, dlpFirstT;
void dlpStartTime( void )
{
	dlpStartT = TIMER55MS;
	if( dlpFirstT == 0UL )
	{
		dlpFirstT = dlpStartT;
	}
	dlpBytes = 0;
}

void dlpReportTime( char *name )
{
  #ifdef DONT_PRINT_PATH
	char *b1, *b2;
	/* Print only the base and one directory back */
	for( b1=b2=name; *b2; b2++ )
	{
		if( *b2 == '\\' )
		{
			b1 = b2 + 1;
		}
	}
  #else
  	char *b1 = name;
  #endif
	dlpStartT = TIMER55MS - dlpStartT;
	dlpTotalT += dlpStartT;
	#ifdef ENABLE_FILE_CACHE
	  sioPrintf( "%s=%lu, tot=%lu, elap=%lu H/N/M=%lu,%lu,%lu (%s) %lu bytes.\n",
		b1, dlpStartT, dlpTotalT, TIMER55MS - dlpFirstT,
		dlpCacheHits, dlpCacheFills, dlpCacheMisses, dlpLastOpenType, dlpBytes );
	#else
	  sioPrintf( "%s=%lu, tot=%lu, elap=%lu.\n",
		b1, dlpStartT, dlpTotalT, TIMER55MS - dlpFirstT );
	#endif
}
#endif


#ifdef ENABLE_FILE_CACHE
/*
**----------------------------------------------------------------------
** Initialize the cache.
**----------------------------------------------------------------------
*/
void initDlpCache( void )
{
	FILE_CACHE near *fc;
	FILE_CACHE near *fcLast;
	int i;

	dlpFCInitFlag = 1;
	fc = (FILE_CACHE *)dlpFileCache;
	dlpFCHead = fc;  /* Point the head at the first one */
	fcLast = &fc[NUM_CACHE_ENTRIES-1];

	/* Make a complete circular list */
	for( i=0; i<NUM_CACHE_ENTRIES; i++ )
	{
		fc->handle = CACHE_HANDLE_NUM + i;
		fc->prev = fcLast;
		fc->nameLen = 0;  /* This is a null entry */
		fcLast->next = fc;

		fcLast = fc;
		fc++;
	}
	dlpLastOpenType = "";
}

/*
** The currently buffered file is OK to put in a cache entry.
** Find the Least-Recently-Used one, put this one in it, and make it
** the Most-Recently-Used one.
*/
void dlpUpdateNewCacheEntry( DLP_FILE_BUF near *fb )
{
	FILE_CACHE near *fc;

	fc = dlpFCHead->prev;   /* This is the oldest one */
	dlpFCHead = fc;         /* It is now the newest! */

	/* Update cache info fields */
	fc->nameLen = fb->bufFileNameLen;
	memcpy( fc->buf, fb->bufFileName, fb->bufFileNameLen );
	fc->fileLen = fb->bufBytes;   /* Size of the file */

	/* Put in the file data */
	memcpy( &fc->buf[fb->bufFileNameLen], fb->buf, fb->bufBytes );
	dlpCacheFills++;
	dlpLastOpenType = "FILL";
}

/*
**----------------------------------------------------------------------
** Check to see if a file is in cache, and return its handle if so.
**----------------------------------------------------------------------
*/
FILE_CACHE near *fileNameToFC( char *fname )
{
	/* See if the file is in the cache first */
	FILE_CACHE near *fc;
	int i;
	unsigned char nl;

	/* Initialize the cache if it the first time here */
	if( !dlpFCInitFlag )
	{
		initDlpCache();
	}

	nl = strlen(fname);

	for( i=0,fc=(FILE_CACHE *)dlpFileCache; i<NUM_CACHE_ENTRIES; fc++,i++ )
	{
		if( (fc->nameLen == nl) && (memcmp(fc->buf,fname,nl) == 0) )
		{
			/* We found it!  Use the cache entry! */
			dlpLastOpenType = "HIT";

			/* Now make this the newest cache entry */
			if( fc == dlpFCHead )
			{
				/* Already newest, nothing to do! */
			}
			else if( fc == dlpFCHead->prev )
			{
				dlpFCHead = fc;  /* Just move the pointer back one */
			}
			else
			{
				/* First unlink the new one */
				fc->prev->next = fc->next;
				fc->next->prev = fc->prev;

				/* Now link us in front */
				fc->next = dlpFCHead;
				fc->prev = dlpFCHead->prev;
				dlpFCHead->prev->next = fc;
				dlpFCHead->prev = fc;
				dlpFCHead = fc;  /* We are now in front! */
			}

			return fc;
		}
	}

	dlpLastOpenType = "MISS";
	dlpCacheMisses++;
	return NULL;
}

/*
**----------------------------------------------------------------------
** Remove a cache entry.
**----------------------------------------------------------------------
*/
void invalidateCacheEntry( FILE_CACHE near *fc )
{
	dlpCacheInvalidates++;
	fc->nameLen = 0;

	/* Now make this the oldest cache entry */
	if( fc == dlpFCHead )
	{
		fc = fc->next;  /* fc is now the oldest */
	}
	else if( fc == dlpFCHead->prev )
	{
		/* Nothing to do, we are already the oldest! */
	}
	else
	{
		/* First unlink this one */
		fc->prev->next = fc->next;
		fc->next->prev = fc->prev;

		/* Now link us in back */
		fc->next = dlpFCHead;
		fc->prev = dlpFCHead->prev;
		fc->prev->next = fc;
		dlpFCHead->prev = fc;
	}
}
#endif

#ifdef CHANGE_DIRECTORY
void checkDirectory(char *path, int chFlag)
{
 /*
 ** Check the directory here, and if not the current one, set the current
 ** one to here...
 */
 char *p;
 int i, absPath;
 char pbuf[MAXPATH];

 if( firstPath[0] == 0 )
 {
	/* Need to fill in the directory we started in */
	getcwd( firstPath, sizeof(firstPath) );
 }

 /* First check to see if we have an absolute path */
 absPath = 0;
 if( path[0] == '\\' )
 {
   absPath = 1;
 }
 else if( (path[1] == ':') && (path[2] == '\\') )
 {
   absPath = 1;
 }

 if( !absPath )
 {
	/* We need to do a relative path for this one, just go back to firstPath first */
	if( strcmp(firstPath, chdirPathName) != 0 )
	{
		#ifdef PRINT_TO_SERIAL_PORT
			sioPrintf( "Changing dir to firstPath:%s\n", firstPath );
		#endif
		chdir( firstPath );
		strcpy( chdirPathName, firstPath );
	}
 }
 else if( chFlag )
 {
	/* We have a full path... */
 	p = strrchr( path, '\\' );
 	if( p && ((i=(int)(p-path)) < sizeof(pbuf)) )
 	{
 		memcpy( pbuf, path, i );
		pbuf[i] = 0;
		if( strcmp( chdirPathName, pbuf ) != 0 )
		{
			strcpy( chdirPathName, pbuf );
			chdir( chdirPathName );
			#ifdef PRINT_TO_SERIAL_PORT
				sioPrintf( "Changing dir to:%s\n", chdirPathName );
			#endif
		}
 	}
 }
}
#endif
#endif
