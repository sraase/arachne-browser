/*----------------------------------------------------------------------------
 * Arachne browser - Clementine startup and glue code
 *----------------------------------------------------------------------------
 *
 * Copyright (c) 1998-2000 Suntech. Written by Emmanuel Marty (core).
 *
 * This file is part of the sourcecode for the Clementine operating system.
 * You can redistribute and/or modify it under the terms and conditions of the
 * Clementine license, described in doc/COPYING-CLEMENTINE of this source tree
 *----------------------------------------------------------------------------
 * $Id:$
 *----------------------------------------------------------------------------
 */

#define NO_GENERIC_STRING_FUNCTIONS
#include <clementine/types.h>
#include <clementine/string.h>
#include <clementine/memory.h>
#include <clementine/event.h>
#include <clementine/file.h>
#include <clementine/errno.h>
#include <clementine/unikeys.h>
#include <clementine/mouse.h>
#include <kgi/kgi.h>
#include <kgi/types.h>
#include <libc/ctype.h>
#include <libc/unistd.h>
#include <stdarg.h>
#include "pckbrd.h"
#include "arachne.dep.h"

/* External main() function in application */
extern int arachne_main (int argc, char **argv);

/* Defined in clemkgi.c */
extern void graphicsdeinit (void);
extern int screenwidth, screenheight;

/* View of our components */
struct components *view;

/* posix errno */
int errno;

/* Dummy, empty argv when argument count = 0 */
static char *dummyArgv[1];

/* Structure describing a key in the buffer */

struct Key {
   int key;             /* Raw unikey */
   long unicode;        /* Corresponding unicode */
};

/* Circular async keyboard buffer */
static struct Key kbdBuf [32];
static volatile int kbdReadIndex, kbdWriteIndex, modifiers;
static semaphore_t kbdSemaphore;

/* Mouse status */
static volatile int mouseX, mouseY, mouseButtons;
static semaphore_t mouseSemaphore;

/* Event sources classes */
#define EVS_COOKEDKEYS  0x1000
#define EVS_MOUSE       0x2000

/*
 * Formatted output
 */

int printf(char *format, ...) {
   va_list arg;
   int done;
   char fmtbuf[2048];

   va_start (arg, format);
   done = view->strlib->vsprintf (fmtbuf, format, arg); 
   va_end (arg);
   
   if (done >= 0) view->stdfile->Write (STDOUT_FILENO, fmtbuf, strlen(fmtbuf));
   return done;
}

int sprintf (char *buffer, const char *fmt, ...) {
   va_list args;
   int result;

   va_start (args, fmt);
   result = view->strlib->vsprintf (buffer, fmt, args);
   va_end (args);
   
   return result;
}

int puts(const char *s) {
   return printf ("%s\n", s);
}

/* Input */

int getchar(void) {
   int result;
   unsigned char buf[1];
   
   result = view->stdfile->Read (STDIN_FILENO, &buf, 1);
   if (!result) return -1;
   return (int) buf[0];
}

int putchar (int c) {
   int result;
   unsigned char buf[1];
   
   buf[0] = (char) c;
   result = view->stdfile->Write (STDIN_FILENO, &buf, 1);
   if (!result) return -1;
   return c;
}

/* String to integer conversion */

int atoi (const char *s) {
   int i=0;
   int sign = 1;

   switch (*s) {
   case '+': s++; break;
   case '-': sign = -1; s++;
   }
   while (isdigit((unsigned char) *s))
      i = i*10 + *(s++) - '0';
   return sign == 1 ? i : -i;
}

int atol (const char *s) {
   long i=0;
   int sign = 1;

   switch (*s) {
   case '+': s++; break;
   case '-': sign = -1; s++;
   }
   while (isdigit((unsigned char) *s))
      i = i*10 + *(s++) - '0';
   return sign == 1 ? i : -i;
}

/*
 * File access
 */

int open(char* path, int oflags, ...) {
  va_list arg;
  int result;

  va_start (arg, oflags);
  result = view->stdfile->Open(path, oflags, arg);
  va_end (arg);
  
  if (result >= 0) {
    return result;
  } else {
    errno = -result;
    return -1;
  }
}

ssize_t read(int fildes, void *buf, size_t nbyte) {
  int result;
  if ((result = view->stdfile->Read(fildes, buf, nbyte)) >= 0) {
    return result;
  } else {
    errno = -result;
    return -1;
  }
}

ssize_t write(int fildes, void *buf, size_t nbyte) {
  int result;
  if ((result = view->stdfile->Write(fildes, buf, nbyte)) >= 0) {
    return result;
  } else {
    errno = -result;
    return -1;
  }
}

off_t lseek(int fildes, off_t offset, int whence) {
  off_t result;
  if ((result = view->stdfile->LSeek(fildes, offset, whence)) != (off_t)-1) {
    return result;
  } else {
    errno = -result;
    return (off_t)-1;
  }
}

int filelength (int handle) {
   off_t curpos;
   int length;
   
   curpos = lseek (handle, 0, SEEK_CUR);
   lseek (handle, 0, SEEK_END);
   length = lseek (handle, 0, SEEK_CUR);
   lseek (handle, curpos, SEEK_SET);
   
   return length;
}

int close(int fildes) {
  int result;  
  if ((result = view->stdfile->Close(fildes)) == 0) {
    return 0;
  } else {
    errno = result;
    return -1;
  }
}

int unlink (const char *path) {
  int result;
  if ((result = view->stdfile->Unlink(path)) >= 0) {
    return result;
  } else {
    errno = -result;
    return -1;
  }
}

/*
 * Memory allocation
 */

void *malloc (size_t size) {
   return view->memory->malloc (size);
}

void free (void *ptr) {
   view->memory->free (ptr);
}

/* Time management */

time_t time (time_t *t) {
   long long seconds;
   long usecs;
   
   if (!view->systime->GetCurrentTime (&seconds, &usecs)) {
      time_t timesecs = (long) seconds;
      
      if (t) *t = timesecs;
      return timesecs;
   }
   
   return (time_t) -1;
}

/* Convert number of seconds since epoch, into a human-readable date.
 * Taken from VSTA, will go away to be rewritten into a proper external
 * component soon; just for testing. */

#define  SECSPERMIN     60
#define  SECSPERHOUR    (60*SECSPERMIN)
#define  SECSPERDAY     (24*SECSPERHOUR)
#define  DAYSPERWEEK    7
#define  EPOCH_YEAR     1970
#define  EPOCH_WDAY     3
#define  TM_YEAR_BASE   0
#define  isleap(y)      (((!(y%4)) && (y % 100)) || (!(y % 400)))

static int __months_len[2][12] =
	{{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	 {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};
static int __years_len[2] = {365, 366};

static char *weekday[7] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
static char *month[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

static char timebuf[64];

void timeconv (long time, char *buf) {
	long days, rem;
        int tm_hour, tm_min, tm_sec, tm_wday, tm_year;
        int tm_yday, tm_mday, tm_mon;
	int y, yleap;
	const int *ip;

	days = time / SECSPERDAY;
	rem = time % SECSPERDAY;

	while (rem < 0) {
		rem += SECSPERDAY;
		--days;
	}
	while (rem >= SECSPERDAY) {
		rem -= SECSPERDAY;
		++days;
	}
        tm_hour = (int)(rem / SECSPERHOUR);
	rem = rem % SECSPERHOUR;
	tm_min = (int)(rem / SECSPERMIN);
	tm_sec = (int)(rem % SECSPERMIN);

	tm_wday = (int)((EPOCH_WDAY + days) % DAYSPERWEEK);
	if (tm_wday < 0) tm_wday += DAYSPERWEEK;

	y = EPOCH_YEAR;
	if (days >= 0) {
		for (;;) {
			yleap = isleap(y);
			if (days < (long)__years_len[yleap])
				break;
			++y;
			days = days - (long)__years_len[yleap];
		}
	} else do {
		--y;
		yleap = isleap(y);
		days = days + (long) __years_len[yleap];
	} while (days < 0);
	tm_year = y - TM_YEAR_BASE;
	tm_yday = (int) days;
	ip = __months_len[yleap];
	for (tm_mon = 0; days >= (long)ip[tm_mon]; ++(tm_mon)) {
		days = days - (long) ip[tm_mon];
	}
	tm_mday = (int)(days + 1);
        
        sprintf (buf, "%s %d %s %d %02d:%02d:%02d",
                 weekday[tm_wday], tm_mday, month[tm_mon], tm_year,
                 tm_hour, tm_min, tm_sec);
}

char *ctime (const time_t *timep) {
   timeconv ((long) *timep, timebuf);
   return timebuf;
}

/* Glue for memset() */

void *memset(void *s, int c, size_t n) {
   return __arch_memset (s, c, n);
}

/* Process exit */

typedef void (*exit_noret_t)(int status) __attribute__ ((noreturn));

void exit(int status) {
   ((exit_noret_t) view->scheduler->Exit) (status);
}

/* Keyboard handling */

static int translatekey (int key) {
   switch (key) {
      case UK_F1: return F1;
      case UK_F2: return F2;
      case UK_F3: return F3;
      case UK_F4: return F4;
      case UK_F5: return F5;
      case UK_F6: return F6;
      case UK_F7: return F7;
      case UK_F8: return F8;
      case UK_INSERT: return INSERT;
      case UK_DELETE: return DELETEKEY;
      case UK_PRIOR: return PAGEUP;
      case UK_NEXT: return PAGEDOWN;
      case UK_CURS_LEFT: return LEFTARROW;
      case UK_CURS_DOWN: return RIGHTARROW;
      default: return 0;
   }
}

int bioskey(int mode) {
   int result;

   switch (mode) {
      case 0:     /* Dequeue and return next key */
         view->scheduler->SemaphoreDown (&kbdSemaphore, 0);
         if (kbdReadIndex != kbdWriteIndex) {
            result = translatekey (kbdBuf[kbdReadIndex].key);
            if (!result) {
               result = (int) kbdBuf[kbdReadIndex].unicode;
               if (result > 255) result = 0;
            }
            if (result)
               kbdReadIndex = (kbdReadIndex + 1) & 31;
         }
         else result = 0;
         view->scheduler->SemaphoreUp (&kbdSemaphore);
         
         return result;

      case 1:     /* Return next key but do not dequeue */
         view->scheduler->SemaphoreDown (&kbdSemaphore, 0);
         if (kbdReadIndex != kbdWriteIndex) {
            result = translatekey (kbdBuf[kbdReadIndex].key);
            if (!result) {
               result = (int) kbdBuf[kbdReadIndex].unicode;
               if (result > 255) result = 0;
            }
         }
         else result = 0;
         view->scheduler->SemaphoreUp (&kbdSemaphore);
         
         return result;

      case 2:     /* Return shift state */
         view->scheduler->SemaphoreDown (&kbdSemaphore, 0);
         result = modifiers;
         view->scheduler->SemaphoreUp (&kbdSemaphore);
         return result;
  
      default:
         return 0;
  }
}

int kbhit(void) {
  //check for keystroke - only to allow Ctrl+C in DOS ....
  return 0;
}

/* Get path to dir for temporary files */

void tempinit (char *path) {
  path[0] = '\0';
}

/* modf() */

#define MAX ((double)(1l << 30) * (1l << 22))

double modf (double fp,double *fint) {
   double temp;

   temp = fp < 0 ? -fp : fp;

   if (temp >=  MAX)
      *fint = fp;
   else {
      *fint = temp + MAX;
      for (*fint = *fint - MAX ; *fint > temp ; --*fint) ;
      if (fp < 0)
         *fint = -*fint;
   }

   return fp - *fint;
}

/* stub for system() */

int system (const char * string) {
   return 0;
}

/* strtol() */

static int getprefix (const char *p, char **outp, int base, int *outbase,
      int *neg) {

   while (isspace(*p)) p++;

   if (*p == '-') {
      *neg = TRUE;
      p++;
   }
   else if (*p == '+')
      p++;
   
   if (!base) {
      base = 10;

      if (*p == '0') {
         p++;
         base = 8;
         
         if (*p == 'x') {
            p++;
            base = 16;
         }
      }
   }
   else if ((base < 2) || (base > 36)) return -EINVAL;

   *outp = (char *) p;
   *outbase = base;
   return 0;
}

static int getnumber (const char *p, char **outp, unsigned long base,
      unsigned long *outvalue) {
   unsigned long value, limit = ULONG_MAX / base;
   int retval = 0;
   char maxnumeric = '0' + ((base > 10) ? 10 : base);
   char maxalpha = ((base > 10) ? ('A' + base - 10) : 'A');

   value = 0;
   
   while (1) {
      char c = *p, majc = c & ~0x20;
      unsigned long digit;

      if ((c >= '0') && (c < maxnumeric))
         digit = ((unsigned long) (c - '0'));
      else if ((majc >= 'A') && (majc < maxalpha))
            digit = ((unsigned long) (majc - 'A')) + 10;
      else break;
      
      if (!retval) {
         if (value > limit) retval = -ERANGE;
         else value *= base;

         if (value > (ULONG_MAX-digit)) retval = -ERANGE;
         else value += digit;
      }

      p++;
   }

   if (outp) *outp = (char *) p;
   *outvalue = value;
   return retval;
}
   
long strtol (const char *nptr, char **endptr, int base) {
   const char *p = nptr;
   char *curp;
   int negate = FALSE, retval;
   long value;
   unsigned long nvalue;

   if (!p) return LONG_MAX;

   retval = getprefix (p, &curp, base, &base, &negate);
   if (retval < 0) return retval;
   
   retval = getnumber (curp, endptr, (unsigned long) base, &nvalue);

   if ((retval) || (nvalue > LONG_MAX)) {
     if (negate)
	 value = LONG_MIN;
       else value = LONG_MAX;
   }
   else {
      value = (long) nvalue;
      if (negate)
         value = -value;
   }

   return value;
}

/* Process external events */

int ProcessEvent (int classCode, int *typePtr, int *objIdPtr,
      int *triggerIdPtr, long *dataPtr, long *data2Ptr) {
   /* Cooked keyboard event? */
   if (classCode == EVS_COOKEDKEYS) {
      int key = *triggerIdPtr, code = key & 0x7FFF;

      /* Yes. Modifier?. */

      if ((code >= UK_MOD_LCTRL) && (code <= UK_MOD_RMENU4)) {
         /* It is. Translate it. */

         switch (code) {
            case UK_MOD_LCTRL:
            case UK_MOD_RCTRL:   code = CTRLKEY; break;
            case UK_MOD_LALT:
            case UK_MOD_RALT:    code = ALT; break;
            case UK_MOD_LSHIFT:  code = LEFTSHIFT; break;
            case UK_MOD_RSHIFT:  code = RIGHTSHIFT; break;
            default: return EVH_OK;
         }

         /* Now acquire semaphore on keys circular buffer */
         view->scheduler->SemaphoreDown (&kbdSemaphore, 0);
         
         /* Alter modifiers state */
         if (key & 0x8000)
            modifiers &= ~code;
         else
            modifiers |= code;

         /* Release semaphore on buffer */
         view->scheduler->SemaphoreUp (&kbdSemaphore);         
      }
      else if (!(key & 0x8000)) {
         /* Regular key. Acquire semaphore on keys circular buffer */
         view->scheduler->SemaphoreDown (&kbdSemaphore, 0);
   
         /* Insert key in buffer */
         kbdBuf [kbdWriteIndex].key = key;
         kbdBuf [kbdWriteIndex].unicode = *dataPtr;
         kbdWriteIndex = (kbdWriteIndex + 1) & 31;
   
         /* Release semaphore on buffer */
         view->scheduler->SemaphoreUp (&kbdSemaphore);
      }

      /* Report success */
      return EVH_OK;
   }

   return EVH_OK;
   /* Mouse event? */
   if (classCode == EVS_MOUSE) {
      int button;

      switch (*typePtr) {
         case MOUSE_EV_RELMOVE:     /* Relative movement. */
            view->scheduler->SemaphoreDown (&mouseSemaphore, 0);

            mouseX += (int) *dataPtr;
            mouseY += (int) *data2Ptr;

            if (mouseX < 0) mouseX = 0;
            else if (mouseX >= screenwidth) mouseX = screenwidth-1;
            if (mouseY < 0) mouseY = 0;
            else if (mouseY >= screenheight) mouseY = screenheight-1;
            
            view->scheduler->SemaphoreUp (&mouseSemaphore);            
            break;
            
         case MOUSE_EV_ABSMOVE:     /* Absolute movement. */
            view->scheduler->SemaphoreDown (&mouseSemaphore, 0);

            mouseX = (int) *dataPtr;
            mouseY = (int) *data2Ptr;

            if (mouseX < 0) mouseX = 0;
            else if (mouseX >= screenwidth) mouseX = screenwidth-1;
            if (mouseY < 0) mouseY = 0;
            else if (mouseY >= screenheight) mouseY = screenheight-1;
            
            view->scheduler->SemaphoreUp (&mouseSemaphore);
            break;
            

         case MOUSE_EV_BUTTON:      /* Button state transition */
            button = *triggerIdPtr;
            if ((button <= MOUSE_BUTTON_LEFT) &&
                (button >= MOUSE_BUTTON_SEVENTH)) {
               button--;

               view->scheduler->SemaphoreDown (&mouseSemaphore, 0);

               switch (*dataPtr) {
                  case MOUSE_BUTTONEV_DOWN:
                     mouseButtons |= 1 << button;
                     break;

                  case MOUSE_BUTTONEV_UP:
                     mouseButtons &= ~(1 << button);
                     break;
               }
               view->scheduler->SemaphoreUp (&mouseSemaphore);
            }
            break;
      }
      
      /* Report success. */
      return EVH_OK;
   }

   /* Unknown source */
   return -ENOSYS;
}

/*
 * Build argv[] from argument string
 *
 * argCount: number of arguments
 * argString: argument string
 *
 * Returns pointer to created argument vector array (argv), or NULL for
 * failure.
 */

char **BuildArgVector (int argCount, char *argString) {
   char **argv;
   
   /* Any arguments? */
   if (argCount) {
      /* Yes, allocate array */
      argv = malloc ((argCount + 1) * sizeof (char *));
   
      /* Success? */
      if (argv) {
         char **curVectorPtr = argv;
         char *argStringPtr = argString, *prevStringPtr = argString;
         int i = 0;
         char c;

         /* Yes, fill vectors and fix argument string, replacing
          * end-of-argument markers by \0. */
      
         while ((c = *argStringPtr++)) {
            if (c == 0x1E) {
               argStringPtr[-1] = '\0';
               *curVectorPtr++ = prevStringPtr;
               prevStringPtr = argStringPtr;

               i++;
               if (i >= argCount) return argv;
            }
         }
         
         *curVectorPtr++ = prevStringPtr; i++;

         while (i < (argCount+1)) {
            i++;
            *curVectorPtr++ = NULL;
         }

         return argv;
      }
   }
   
   /* No arguments, return a valid one-empty-entry array */
   dummyArgv[0] = NULL;
   return dummyArgv;
}

/*
 * Application start
 */

int Start (int argCount, string argString) {
   char **argv;
   int result;

   errno = 0;
   result = -1;

   argv = BuildArgVector (argCount, argString);

   if (STDIN_FILENO==open("/dyn/os/display/terminal/xterm",O_RDONLY,NULL)) {
      if (STDOUT_FILENO==open("/dyn/os/display/terminal/xterm",O_WRONLY,NULL)) {
         if (STDERR_FILENO==open("/dyn/os/display/terminal/xterm",O_WRONLY,NULL)) {
            kbdReadIndex = kbdWriteIndex = modifiers = 0;
            mouseX = mouseY = mouseButtons = 0;
            view->scheduler->InitSemaphore (&kbdSemaphore, 1);
            view->scheduler->InitSemaphore (&mouseSemaphore, 1);

            view->cookedkeys->StartEvents (view->cookedkeys->this,
               EVS_COOKEDKEYS, 0, 0);
            if (view->mouse)
               view->mouse->StartEvents (view->mouse->this,
                  EVS_MOUSE, 0, 0);

            printf ("Arachne browser for Clementine starting.\n");
            result = arachne_main (argCount, argv);

            if (view->mouse)
               view->mouse->StopEvents (view->mouse->this);
            view->cookedkeys->StopEvents (view->cookedkeys->this);
            graphicsdeinit ();       
            close (STDERR_FILENO);
         }
         close (STDOUT_FILENO);
      }
      close (STDIN_FILENO);
   }
         
   return result;
}

/* Instance management */

handle_t Register (handle_t myHandle, void *viewPtr, handle_t callerHandle,
      int exportIndex) {
   return 1;
}

void Unregister (handle_t myHandle, void *viewPtr, handle_t callerHandle,
   int exportIndex, handle_t instance) {
}

/* Initialise browser */

boolean Init (handle_t myHandle, void *viewPtr, long dynFsNode) {
  view = (struct components *) viewPtr;
  return TRUE;
}

/* Prepare to destroy */

void Destroy (handle_t myHandle, void *viewPtr) {
}
