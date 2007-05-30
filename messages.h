#ifdef AGB
#include "agb_msg.h"
#else

#ifdef TELEMED
#include "telemed.h"
#else

/*
Message file for ARACHNE WWW browser

Notes for translators:

1) Please translate only quoted messages (text between " .... ")
2) Please keep symbols like %s, %d, etc. They will be replaced by program.
3) Symbol \n means line break
4) For German message file, there is one big inconvenience: text mode messages
   must be displayed in DOS code page, while graphics mode messages in Windows
   code page. This is because of characters with "umlauts".

*/

//Messages with (almost) unlimited size (one line of text):
//!!glennmcc: June 22, 2002 ... spelling correction "succesfuly->successfully" ;-)

#define MSG_START  "\nArachne V%s%s is taking off...\n%s\n"
#ifdef NOKEY
#define MSG_END    "\nArachne V%s%s%s\n%s has just successfully collapsed to DOS...\n%s\n\n"
#else
#define MSG_END    "\nArachne V%s%s%s has just successfully collapsed to DOS...\n%s\n\n"
#endif
#define MSG_ENDX   "Arachne has just successfully collapsed...\n\n"
#define MSG_MEM    "Arachne is out of conventional (low) DOS memory"
#define MSG_MEMERR "Arachne is out of memory - malloc() failed.\n"
#define MSG_BYTES  ": (%lu bytes missing)\n"
#define MSG_XSWAP  "Illegal xSwap operation"
#define MSG_XDEL   "cannot delete from xSwap"
#define MSG_DELAY1 "Would you like to have some coffee ?"
#define MSG_DELAY2 "Just a moment, please..."
#define MSG_BREAK  "<Ctrl+Pause> aborts."
#define MSG_ESC    "<Esc> aborts..."
#define MSG_ANYKEY " press any key "
#define MSG_ALLOC  "Memory allocation error - %s at line %d of file %s!\nTrying to deallocate xSwap..."
#define MSG_DNSERR "Cannot locate computer %s - please check your setup !"
#define MSG_BADEXE "Error in EXE file."
#define MSG_TCPIP  "Initializing TCP/IP..."
#define MSG_FONT   "Updating font information..."
#define MSG_RETURN "Type 'arachne -r' to return to World Wide Web.\n"
#define MSG_THIS   "> This is NON-COMMERCIAL (unregistered) version of Arachne,%s<\n"
#define MSG_HANGUP "Terminating dial-up connection..."
#define MSG_ERRIMG "Cannot load image..."
#define MSG_CONV   "Processing '%s' [%s]. %s %s"
#define MSG_PLUGIN "Processing '%s'. %s"
#define MSG_ERROR  "Error!"
#define MSG_NOTMEM "Page is too long !"
#define MSG_ABORT  "Transfer interrupted !"
#define MSG_ASKDNS "Asking domain name server for %s ..."
#define MSG_ERRCON "Cannot connect to %s - please check your setup !"
#define MSG_CON    "Connecting to %s, port %u ..."
#define MSG_REQ    "Connected to %s - requesting object '%s' ..."
#define MSG_ALIVE  "Alive to %s, requesting object %s ..."
#define MSG_POST   "Posting data..."
#define MSG_READ   "%d bytes read.\n"
#define MSG_HTTP   "Page information for"
#define MSG_REDIR  "Redirection..."
#define MSG_CLOSE  "Closing connection..."
#define MSG_CLOSED "Connection closed."
#define MSG_DOWNLD "Downloading file ("
//!!glennmcc: Nov 17, 2004 -- include bytes/sec rate
#define MSG_X_OF_Y_byte "%s%ld of %ld bytes, %ld bytes/sec)"
//!!glennmcc: Aug 19, 2005 -- restore original MSG_X_of_Y for use by 2nd image
//during parallel image download & use MSG_X_of_Y_byte for the 1st image only
#define MSG_X_OF_Y "%s%ld of %ld bytes ) ..."
#define MSG_BYTESR "%s%ld bytes read) ..."
#define MSG_MISOBJ "%d objects missing, "
#define MSG_DISK   "Loading page from disk"
#define MSG_ADJUST "Adjusting frames, images and tables"
#define MSG_DLPERC "Downloading page (%ld of %ld bytes) ..."
#define MSG_DLBYTE "Downloading page (%ld bytes read) ..."
#define MSG_PARALL "Waiting for parallel image download to finish ..."
#define MSG_REDRAW "Refreshing screen..."
#define MSG_RENDER "Generating virtual screen..."
#define MSG_FORM   "Processing form..."
#define MSG_FRAMES "This is multiple document:"
#define MSG_DELAY0 "Meditating..."
#define MSG_VERIFY "Verifying images..."
#define MSG_CFGERR "Error reading %s:\nFile not found, file is longer than %d lines or out of memory\n"
#define MSG_KILL   "Killing disk cache..."
#define MSG_DEAD   "Cache is now dead."
#define MSG_URL    "Enter the URL (internet address) or search phrase and press <Enter> ..."
#define MSG_TCPERR "ERROR: %s\n"
#define MSG_TCPILL "Illegal TCP/IP status: %d\n"
#define MSG_NOIP   "Undefined IP adress !"
#define MSG_CONFIG "Updating configuration file..."
#define MSG_WRITE  "Writing file..."
#define MSG_COPY   "Copying..."
#define MSG_HOTLST "URL '%.40s' was added to hotlist."
#define MSG_ERRHOT "Cannot write to hotlist !"
#define MSG_FNTERR "Error loading font file: "
#define MSG_MEMLFT "(free memory left = %lu)\n"
#define MSG_F5ZOOM "\n\nIn this mode you can use <F5> or <*> key to toggle full-screen view.\n\nPlease%s..."
#define MSG_VERR1  "\nUnable to initialize video mode - please run setup.bat !"
#define MSG_APCK   "\nUnable to load arachne.pck - please run setup.bat !"
#define MSG_GIF    "Original pallette=%d, GIF pallette=%d, Total colors=%d."
#define MSG_LDPAL  "Loading %d color pallettes ..."
#define MSG_MIXPAL "Mixing pallette of %d colors ..."
#define MSG_BMP    "Original palette=%d, BMP=Truecolor, Total colors=%d."
#define MSG_ASKING "Connected. Asking for '%s'..."
#define MSG_WTRPL  "Connected. Now waiting for reply..."
#define MSG_RDRPL  "Reading reply... (%ld bytes read)"
#define MSG_CONVI  "Processing %d embeded objects. %s %s"
#define MSG_LOGIN  "Logging in..."
#define MSG_DETECT "Detected %lu messages (%lu bytes in mailbox)\n"
#define MSG_GET1   "Getting message #%lu of %lu (%lu%s"
#define MSG_GET2   "Getting message #%lu (%lu bytes) of %lu (%lu%s"
#define MSG_GET3   " bytes total)\n"
//#define MSG_GET3   " bytes in mailbox)\n"
#define MSG_DELE   "Marking message #%lu for deletion"
//#define MSG_DELE   "Deleting message # %lu of %lu"
#define MSG_SKIP   "Skipping message #%lu, too big for disk space available\n"
#define MSG_ERROPN "Cannot open file!"
#define MSG_SMTP   "Connecting to SMTP server..."
#define MSG_ICON1  "Previous visited page"
#define MSG_ICON2  "Next visited page"
#define MSG_ICON3  "Homepage"
#define MSG_ICON4  "Reload"
#define MSG_ICON5  "Add to hotlist"
#define MSG_ICON6  "Hotlist"
#define MSG_ICON7  "Abort transfer"
#define MSG_ICON8  "Search engine"
#define MSG_ICON9  "Help"
#define MSG_ICONH  "History"
#define MSG_ZOOM   "Resize"
#define MSG_EXIT   "Exit Arachne"
#define MSG_INFO2  "More information"
#define MSG_DIAL2  "Connection control page"
#define MSG_SEND   "Sending message, %ld bytes of %ld..."
#define MSG_UPLOAD "Uploading file, %ld bytes of %ld..."
#define MSG_SENT   "%d bytes sent, waiting for response..."
#define MSG_MAILUP "SMTP upload"
#define MSG_MAILDL "POP3 download"
#define MSG_REMOVE "Deleted %s"
#define MSG_DIAL   "Just a moment, dialing! %s"
#define MSG_USEMAP "Linking client side imagemaps..."
#define MSG_PRN    "Converting to plain text..."
#define MSG_SRCH1  "Searching for '%s'..."
#define MSG_SRCH2  "Not found !"
#define MSG_IMAGE  "%s image %dx%d"
#define MSG_COLORS ", %d colors"
#define MSG_LINECOL "Line:%04d/%04d Column:%03d"
#define MSG_SRCH4  "Search"
#define MSG_ENTER  "Enter search string and press <Enter>"
#define MSG_PS     "Printing PostScript page # %d of %d ..."
#define MSG_CLPDEL "You can remove selected object using <Delete> key..."
#define MSG_CLPADD "You can add selected link to Hotlist..."
#define MSG_EDTADD "Title"
#define MSG_AENTER "Edit name and press <Enter>"
#define MSG_BACKGR "Parallel image download ("
#define MSG_IDENT  "Ignoring IDENT request at port 113..."
#define MSG_EFNAME "Enter file name and press <Enter>"
#define MSG_REFRSH "Arachne will fly to '%.40s' in %d seconds. %s"
#define MSG_DOCDON "%s (load time=%ld:%02d)"
#define MSG_BLOCK  "^B begin block, ^K end block, ^C copy block, ^M move block, ^Y cut block"
#define MSG_NOVIRT "No virtual screen to export !"
#define MSG_NOVGA  "Unsupported graphics mode !"
#define MSG_BLCUT  "Cut block to clipboard"
#define MSG_BLCLIP "Copy block to clipboard"
#define MSG_BLPAST "Paste block from clipboard"
#define MSG_MISLNK "Missing links: %d"
#define MSG_ERRIKN "Error while processing icon file. Can't draw user interface!\n"
#define MSG_BLCKWR "Block written."

#define MSG_MEMSEL "\nAvailable memory types for swapping:\n"
#define MSG_MEMXMS "0. XMS (recommended) [Enter]"
#define MSG_MEMEMS "1. EMS (for XT/AT)"
#define MSG_MEMDSK "2. Disk (last choice)"
#define MSG_MEMORY "\nSelect memory type [Esc to abort]: "

#define MSG_VGASEL "\nAvailable video card categories:\n"
#define MSG_VGAVGA "0. VGA (recommended) [Enter]"
#define MSG_VGAEGA "1. EGA (historical)"
#define MSG_VGACGA "2. CGA (experimental)"
#define MSG_VIDEO  "\nSelect video type [Esc to abort]: "

//Messages with limited size:

//                 >------------<
#define MSG_UNREG  "Unregistered"

//                 >-------|----|-----<
#define MSG_INFMSG " device free  used"

#define MSG_SAVE   "Save to disk, send by e-mail, FTP upload"
#define MSG_MAIL   "Your e-mail"

//(aproximately)   >-------------<
#define MSG_OPEN   "Load/open URL"
#define MSG_PRINT  "Print/export"
#define MSG_SEARCH "Search in page"
#define MSG_EDIT   "Local edit"
#define MSG_SOURCE "Page source"
#define MSG_INFO   "Page information"
#define MSG_IMAGES "Load images"
#define MSG_HOME   "Desktop"
#define MSG_WRITEF "Write file"
#define MSG_READF  "Read file"
#define MSG_SRCHTX "Search in text"
#define MSG_PRT    "Print"
#define MSG_BLKCUT "Cut"
#define MSG_BLKCLP "To clipboard"
#define MSG_BLKPST "Paste"
#define MSG_BLKCOP "Copy"
#define MSG_BLKMOV "Move"
#define MSG_BLKDEL "Delete"

//(aproximately)   >------<
#define MSG_TITLE  "Title"

#endif
#endif
