struct HTTPrecord
{
 char URL[URLSIZE];    // Uniform Resource Locator
 int x;                // coordinates where the document was last displayed
 long y;               //
 long size;            // size in bytes
 char knowsize;        // logical - size is valid
 char mime[STRINGSIZE];// mime type
 char locname[80];     // full filename (after conversion to TXT,HTM,GIF,BMP)
 char rawname[80];     // full filename (before conversion)
 int handle;           // file handle
 long lastseen;        // last seen time
 char dynamic;         // document is dynamic
};
