// ========================================================================
// IMPORTANT! This where any new inline image types can be added to Arachne
// In DOS version, this is linked staticaly to accelerate page drawing...
// ========================================================================

#include <string.h>
#include "picinfo.h"

int drawGIF(struct picinfo *gif);
int XCHdrawBMP(struct picinfo *bmp);
#ifdef INLINEJPEGS
int drawJPEG(struct picinfo *jpeg);
#endif
#ifdef INLINEPNGS
int drawPNG(struct picinfo *png);
#endif

int drawanyimage(struct picinfo *image)
{
 char *ext=strrchr(image->filename,'.');

 if(ext && !strcmpi(ext,".bmp")) //JPEGs and PNGs are converted to BMPs
  return XCHdrawBMP(image);
 //======================================================================
#ifdef INLINEJPEGS
 else if(ext && !strcmpi(ext,".jpg"))l // future codec for JPEG
  return drawJPEG(image);
#endif
 //======================================================================
#ifdef INLINEPNGS
 else if(ext && !strcmpi(ext,".png")); // future codec for PNG
  return drawPNG(image);
#endif
 //======================================================================
 else                            //default image type is animated GIF
  return drawGIF(image);
 //======================================================================
}