
#include <string.h>
#include <stdio.h>

void base64code(unsigned char *in,char *out)
{
 char *base64tab ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 char q[5];
 int k;

 while(*in)
 {
  k=strlen((char *)in);
  if(k>3)
   k=3;

   if(!k)
   break;

  if(k<3)
  {
   in[2]=0;
   if(k==1)
    in[1]=0;
  }

  q[0]=base64tab[(in[0]>>2) & 0x3F];
  q[1]=base64tab[((in[0]<<4) & 0x3F)|((in[1]>>4) & 0x3F)];
  q[2]=base64tab[((in[1]<<2) & 0x3F)|((in[2]>>6) & 0x3F)];
  q[3]=base64tab[in[2] & 0x3F];

  if(k<3)
  {
   q[3]='=';
   if(k==1)
    q[2]='=';
  }

  q[4]='\0';
  strcat(out,q);

  in+=k;
 }//loop

}
