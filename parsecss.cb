
// ========================================================================
// CSS parsing routines for Arachne WWW browser
// (c)1997-2001 Michael Polak, Arachne Labs
// ========================================================================

#include "arachne.h"
#include "html.h"

#define ALLOWEDCHARS ",_-"

void ProcessSingleCSSItem(char *variable,char *value, char *objmap,struct TMPframedata *sheet)
{
 unsigned char backR,backG,backB;
 unsigned char textR,textG,textB;
// unsigned char linkR,linkG,linkB;
// unsigned char vlinkR,vlinkG,vlinkB;
 int newsize=1;
 char setbits=0,resetbits=0;
// int back=0,text=0,link=0,vlink=0,size=0,align=0;
 int back=0,text=0,size=0;

 if(!strcmpi(variable,"color"))
 {
  try2readHTMLcolor(value,&textR,&textG,&textB);
  text=1;
 }
 else
 if(!strcmpi(variable,"background-color") ||
    !strcmpi(variable,"background"))
 {
  try2readHTMLcolor(value,&backR,&backG,&backB);
  back=1;
 }
 else
 if(!strcmpi(variable,"font-size"))
 {
  char *ptr=strchr(value,'%');
  if(ptr)
   newsize=try2getnum(value,sheet->basefontsize);
  else
  {
   int px;
   ptr=strchr(value,'px');
   if(!ptr)
    ptr=strchr(value,'pt');
   if(ptr)
    *ptr='\0';
   px=atoi(value);
   while(px>fonty(newsize,0) && newsize<6)
    newsize++;
  }
  size=1;
 }
 else
 if(!strcmpi(variable,"font-style"))
 {
  if(toupper(value[0])=='I')
   setbits=ITALIC;
  else
   resetbits=ITALIC;
 }
 else
 if(!strcmpi(variable,"font-weight"))
 {
  if(toupper(value[0])=='B')
   setbits|=BOLD;
  else
   resetbits|=BOLD;
 }
 else
 if(!strcmpi(variable,"text-decoration") ||
    !strcmpi(variable,"font-decoration"))
 {
  if(toupper(value[0])=='U')
   setbits|=UNDERLINE;
  else
   resetbits|=UNDERLINE;
 }

 if(objmap[TAG_BODY] || objmap[TAG_P])
 {
  sheet->basefontstyle|=setbits;
  sheet->basefontstyle-=resetbits&sheet->basefontstyle;
  if(size)sheet->basefontsize=newsize;
  if(text){sheet->textR=textR;sheet->textG=textG;sheet->textB=textB;}
  if(back){sheet->backR=backR;sheet->backG=backG;sheet->backB=backB;}
 }

 if(objmap[TAG_A])
 {
  sheet->ahrefsetbits|=setbits;
  sheet->ahrefresetbits|=resetbits;
  if(size)sheet->ahreffontsize=newsize;
  if(text){sheet->linkR=textR;sheet->linkG=textG;sheet->linkB=textB;}
 }

 if(objmap[TAG_SPECIAL_A_HOVER])
 {
  sheet->usehover=1;
  sheet->hoversetbits|=(setbits&(UNDERLINE|ITALIC));
  sheet->hoverresetbits|=(resetbits&(UNDERLINE|ITALIC));
  if(text){sheet->hoverR=textR;sheet->hoverG=textG;sheet->hoverB=textB;}
 }

// if(objmap[TAG_TABLE])
// {
// sheet->tablefontstyle|=setbits;
// sheet->tablefontstyle&=(resetbits^resetbits);
// if(size)sheet->tablefontsize=newsize;
//  if(text){sheet->tableR=textR;sheet->tableG=textG;sheet->tableB=textB;
//           sheet->usetablecolor=1;/*sheet->usetdcolor=0*/;}
//  if(back){sheet->tablebgR=backR;sheet->tablebgG=backG;sheet->tablebgB=backB;
//           sheet->usetablebgcolor=1;/*sheet->usetdbgcolor=0*/;}
// }

 if(objmap[TAG_TD] || objmap[TAG_TABLE])
 {
  sheet->tdfontstyle|=setbits;
  sheet->tdfontstyle&=(resetbits^resetbits);
  if(size)sheet->tdfontsize=newsize;
  if(text){sheet->tdR=textR;sheet->tdG=textG;sheet->tdB=textB;sheet->usetdcolor=1;}
  if(back){sheet->tdbgR=backR;sheet->tdbgG=backG;sheet->tdbgB=backB;sheet->usetdbgcolor=1;}
 }


}

int checkhover(char *variable, char *objmap, char *classfilter,int classenabled){
 char *ptr;

 strlwr(variable);
 if(variable[0]!='.' && classenabled ||
    variable[0]=='.' && classfilter[0] && !strncmpi(classfilter,&variable[1],strlen(classfilter)))
 {
  if(strchr(variable,':')) //just a quick patch...
  {
   if ((ptr=strstr(variable,":link"))!=NULL)  //just quick patch
   {
    objmap[TAG_A]=1;
    if(variable[0]!='.')
     variable[0]='\0';
    else
     *ptr='\0';
   }
   else
   if ((ptr=strstr(variable,":hover"))!=NULL)  //just quick patch
   {
    objmap[TAG_SPECIAL_A_HOVER]=1;
    if(variable[0]!='.')
     variable[0]='\0';
    else
     *ptr='\0';
   }
   else
    variable[0]='\0';

   return 1;
  }
 }
 return 0;
}


int ParseCSS(struct TMPframedata *sheet,XSWAP cssadr, char *classfilter)
{
 char *line;
 char variable[STRINGSIZE];
 char value[IE_MAXLEN];
 int mode=0;
 int quote=0,i;
 int setalltags=0;
 char c;
 char *objmap;
 int classenabled=1;
 struct ib_editor *css;
 int ch;

 if(cssadr==IE_NULL)
  return 0;

 css=(struct ib_editor *)ie_getswap(cssadr);
 if(css)
 {
  if(!css->lines)
   return 0; //empty
  memcpy(&tmpeditor,css,sizeof(struct ib_editor));
  css=&tmpeditor;
 }
 else
  return 0; //we should report error, but CSS are anyway just optional feature

 if(classfilter[0])
  classenabled=0;

 objmap=calloc(1,TAG_LAST); // max 0... TAG_SLASH tags.
 if(!objmap)
  return 0; //no need to crash, just quietly ignore stylesheet ;-)

 memset(objmap,0,TAG_LAST); //reset objmap
 css->y=0;
 while(css->y<css->lines)
 {
  line=ie_getline(css,css->y);
  css->x=0;
  while((c=line[css->x])!='\0')
  {
   if(c<' ' && c>0)c=' ';
   switch(mode)
   {
    case 0: if(!isalnum(c) && c!='.')   //search for class name - starts with alnum()
             break;
            i=0;
            mode=1;
    case 1:
    case 2: if(isalnum(c) || strchr(ALLOWEDCHARS,c) ||
               mode==1 && (c==':' || c=='.' ) )
            {
             if(i<STRINGSIZE)
              variable[i++]=c;
             break;
            }

            if(mode==1 && (c==' ' || c==',') && i)
            {
             variable[i]='\0';

             ch=checkhover(variable,objmap,classfilter,classenabled);
             if(variable[0]=='.')
             {
              if(classfilter[0])
              {
               if(!strcmpi(&variable[1],classfilter))
               {
                if(!classenabled && !ch)
                 setalltags=1;
                classenabled=1;
               }
               else
                classenabled=0;
              }
              else
               classenabled=0;
             }
             else
             if(variable[0]!='/' && classenabled)
             {
              setalltags=0;
              objmap[FastTagDetect(variable)]=1;
             }

             i=0;
            }
            else
            if(mode==1 && c=='{' || mode==2 && c==':')
            {
             variable[i]='\0';
             if(mode==1 && i)
             {
              ch=checkhover(variable,objmap,classfilter,classenabled);
              if(variable[0]=='.' && classfilter[0] && !strcmpi(&variable[1],classfilter))
              {
               if(!ch)
                setalltags=1;
               classenabled=1;
              }
              else
              if(variable[0]!='/')
               objmap[FastTagDetect(variable)]=1;
             }
             i=0;
             mode++;
            }
            else
            if(mode==2 && c=='}')
            {
             i=0;
             memset(objmap,0,TAG_LAST);
             setalltags=0;
             if(classfilter[0])
              classenabled=0;
             else
              classenabled=1; //no class filter applies
             mode=0;
            }

            break;

    case 3: if(c=='\"') quote=1-quote;
            else
            if(c!=';' && (c!=' ' || i) && c!='}' || quote)
            {
             if(i<IE_MAXLEN)
              value[i++]=c;
             break;

            }
            else
            if(c==';' || (c=='}' && !quote))  //end variable
            {
             value[i]='\0';
             if(classenabled)
             {
              if(setalltags)
               memset(objmap,1,TAG_LAST); //active for all tags...
              ProcessSingleCSSItem(variable,value,objmap,sheet);
             }
             i=0;
             if(c=='}')
             {
              memset(objmap,0,TAG_LAST);
              setalltags=0;
              if(classfilter[0])
               classenabled=0;
              else
               classenabled=1; //no class filter applies
              mode=0;
             }
             else
              mode=2;
            }
            break;

   }//end switch
   css->x++;
  }//lopp
  css->y++;
 }//loop
 free(objmap);
 return 1;
}

