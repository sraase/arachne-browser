
#include "security\xchreg.h"
#include "arachne.h"


int rot31(int z);

int loadkey(char *filename,char *reg)
{
 int f,i,j,keylen;
 int buf[4*KEYLEN];
 char in[16*KEYLEN];
 char str[10];
 int a,b,c;

 char kwd[12][41]=
{
 "TfYlVOEBQJdbnSEtPVtuQJDIGVDltQfSNBqsEfM",
 "gFMBdfNBTuvsCbdfpObojfQfepgJMJfOflspGJM",
 "dvoojmjohvtBOBMOJTFYcjtftfyvbmjubmftcpT",
 "npuifsGVDLjohbtiPMfpjevQVtGsfvETjhnVoE`",
 "TBepNbTpNpupspQjmpLswfIfsfUJLP[wsbulpef",
 "TbeFLPOFDofwZWbovujDSBDLFSJKTPVEFNFOUJ\"",
 "qvSFMPWFgffmjohjtfwfSZuijohJxboobipme%%",
 "zpvsIBOEJxbouZpvsLjttupmbtugpsfwfsJMPWF",
 "VJMPWFVjmPWFvJxbOOBlopxIpxVgffmJoffe\`\`\`",
 "tpnfPOFuptbwfnfJGFFMtpmpofmzJGFFMtpTBE\"",
 "MtEuIdYudzMtEuIdYudzMtEuIdYudzMtEFFFFFF",
 "KftvtxbtuifpomztpoufmmnfJ(nUifDiptfoPof"
};

 f=open(filename,O_RDONLY|O_TEXT);
 if(f<0)return 0;
 keylen=read(f,in,16*KEYLEN);
 in[keylen]='\0';
 j=0;
 i=0;
 strcpy(str,"0x0000");
 while(j<keylen && i<4*KEYLEN)
 {
  while(in[j]=='\n')j++;
  strncpy(&str[2],&in[j],4);
  j+=4;
  sscanf(str,"%x",&buf[i]);
  buf[i]-=1973;
  i++;
 }//loop

 close(f);
 keylen=i-1;

 i=0;
 while(i<12)
 {
  j=0;
  while(j<41)
  {
//   kwd[i][j]++;
//   printf("%c",kwd[i][j]);
   kwd[i][j]--;
   j++;
  }
//  printf("\n");
  i++;
 }
 i=0;
 j=0;
 while(i<keylen && j<KEYLEN)
 {
  a=kwd[buf[i]-'M'][j];
  b=kwd[buf[i+2]-'P'][j];

  if(a+b+buf[i+3]==42)
  {
   c=kwd[buf[i]-'M'][j];
   reg[j]=buf[i+1]-c;
  }
  else
  {
   printf("\nInvalid registration key. Please delete %s\n",filename);
   exit(EXIT_TO_DOS);
  }
  j++;
  i+=4;
 }//loop
 reg[j]='\0';
 return 1;
}
#define CHECKBUF 16000

int exeisok(char *exename)
{
 long offset,checksum,x=0;
 int i=0,step[5]={7,3,1,2,7},j=0;
 int f=sopen(exename,O_RDONLY|O_BINARY, SH_DENYNO, S_IREAD);
 char *buf=farmalloc(CHECKBUF+1);
 int rv=0;

 if(buf)
 {
  if(f)
  {
   lseek(f, -8L, SEEK_END);
   read(f,&offset,4);
   read(f,&checksum,4);

   lseek(f, offset, SEEK_SET);
   read(f,buf,CHECKBUF);

   while(j<CHECKBUF)
   {
    x+=buf[j];
    j+=step[i++];
    if(i==4)i=0;
   }

   if(x==checksum)
    rv=1;
   close(f);
  }
  farfree(buf);
 }

 return rv;
}

