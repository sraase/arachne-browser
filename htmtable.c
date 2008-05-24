
// ========================================================================
// HTML table optimizing routines for Arachne WWW browser
// (c)1997,1998,1999 Michael Polak, Arachne Labs (xChaos software)
// ========================================================================

#include "arachne.h"
#include "html.h"

#define FUZZYWIDTH 1

// ========================================================================
// reset HTMLtable structure
// ========================================================================

void inittable(struct HTMLtable *tab)
{
 if(!tab)
  return;

 memset(tab,0,sizeof(struct HTMLtable)-sizeof(XSWAP));
}

// ========================================================================
// refloat variable x-sum of columns of HTML table
// ========================================================================

void refloatvarxsum(struct HTMLtable *tab,int where)
{
 int i=where;
 long expandingxsum=tab->xsum[i];

 if(tab->varxsum[i]>expandingxsum)
 {
  tab->varxsum[i]-=expandingxsum;
  return;
 }
 else
 {
  expandingxsum-=tab->varxsum[i];
  tab->varxsum[i]=0;
 }

 i=where-1;
 while(i>0 && (tab->shrink[i]&SHRINK_RIGHT_ALLOWED) && expandingxsum>0)
 {
  if(tab->varxsum[i]>expandingxsum)
  {
   tab->varxsum[i]-=expandingxsum;
   return;
  }
  else
  {
   expandingxsum-=tab->varxsum[i];
   tab->varxsum[i]=0;
  }
  i--;
 }
 i=where+1;
 while(i<=tab->columns && (tab->shrink[i]&SHRINK_LEFT_ALLOWED) && expandingxsum>0)
 {
  if(tab->varxsum[i]>expandingxsum)
  {
   tab->varxsum[i]-=expandingxsum;
   return;
  }
  else
  {
   expandingxsum-=tab->varxsum[i];
   tab->varxsum[i]=0;
  }
  i++;
 }
}

/* ???? defunct

void refloatpercents(struct HTMLtable *tab,int where,int desired)
{
 int i,delta,deltaperc;

 delta=desired-((int)(((long)(tab->maxwidth-tab->cellspacing)*tab->percent[where])/100)-tab->cellspacing);
 if(delta>=0)
  return;
 deltaperc=(int)(100*(long)delta/(tab->maxwidth-tab->cellspacing));

 i=where-1;
 while(i>0 && (tab->shrink[i]&SHRINK_RIGHT_ALLOWED) && deltaperc>0)
 {
  if(tab->percent[i]>deltaperc)
  {
   tab->percent[i]-=deltaperc;
   return;
  }
  else
  {
   deltaperc-=tab->percent[i];
   tab->percent[i]=0;
  }
  i--;
 }

 i=where+1;
 while(i<=tab->columns && (tab->shrink[i]&SHRINK_LEFT_ALLOWED) && deltaperc>0)
 {
  if(tab->percent[i]>deltaperc)
  {
   tab->percent[i]-=deltaperc;
   return;
  }
  else
  {
   deltaperc-=tab->percent[i];
   tab->percent[i]=0;
  }
  i++;
 }

}
*/

// ========================================================================
// find x-position for new cell in HTML table, take care about ROWSPAN !
// ========================================================================

int determine_new_x(struct HTMLtable *tab)
{
 int tabx=tab->x;

 //skip rowspan
 while(tab->rowspan[tabx+1] && tabx<MAXTD-2)
  tabx++;

 return tabx;
}

int determine_new_width(struct HTMLtable *tab, int xspan)
{
 int newwidth=0,i=determine_new_x(tab)+1,j=i+xspan-1;

 newwidth=tab->realtdwidth[i];
 while(i<j && i<tab->columns)
 {
  newwidth+=tab->cellspacing+tab->realtdwidth[++i];
 }
 return newwidth;
}

// ========================================================================
// do tracing of HTML table width progress - for multicolumn cells adjusting
// ========================================================================

void fixtracewidth(struct HTMLtable *tab,int maxx,int x,int xspan)
{
 if(xspan==1) // ..... <TD COLSPAN=1>
 {
  if(x==1)
  {
   if(tab->tracewidth[1]<maxx)
    tab->tracewidth[1]=maxx;
  }
  else        // ..... <TD COLSPAN>1>
  {
   if(tab->tracewidth[x]<tab->tracewidth[x-1]+maxx)
    tab->tracewidth[x]=tab->tracewidth[x-1]+maxx;
  }
 }
 else
 {
  int i=x-xspan+1;
  int minwidth=tab->tracewidth[x-xspan];

  while(i<x) //fix skipped tracewidth records:
  {
   if(tab->tracewidth[i]<minwidth)
    tab->tracewidth[i]=minwidth;
   i++;
  }

  if(xspan==x)
  {
   if(tab->tracewidth[x]<maxx)
    tab->tracewidth[x]=maxx;
  }
  else
  {
   if(tab->tracewidth[x]<maxx+minwidth)
    tab->tracewidth[x]=maxx+minwidth;
  }

 }
}

// ========================================================================
// process cell of HTML table - called by render_html()
// ========================================================================

char processcell(struct HTMLtable *tab,long xsum,int maxx,long y,int *cellx)
{
 char rv=0;
 int i;

 //------------------------------------------------------<TD COLSPAN=1> :
 if(tab->xspan<=1)
 {

  if(tab->realtdwidth[tab->x]<maxx)
  {
   tab->realtdwidth[tab->x]=maxx;

   if(tab->y) //pokud nejsem na prvni radce, musim prekreslovat
     // tr.: if I am not on the first line, I need to redraw
    rv=1;
  }

  *cellx=tab->realtdwidth[tab->x];
  if(xsum>tab->xsum[tab->x])
   tab->xsum[tab->x]=xsum;
  refloatvarxsum(tab,tab->x);
 }
 //------------------------------------------------------<TD COLSPAN>1> :
 else
 {
  long localxsum=0,xsumpercolumn;
  float rate;

  i=tab->x-tab->xspan+1;

  *cellx=0;
  while(i<=tab->x)
  {
   //I am trying to determine real width of table...
   *cellx+=tab->realtdwidth[i];

   //mezera mezi policky
   // tr.: space between fields
   if(i<tab->x)
    *cellx+=tab->cellspacing;

   //let's determine local sum of x-widths:
   localxsum+=tab->xsum[i];
   i++;
  }

  //fix desired table width - xsum[] and varxsum[]
  i=tab->x-tab->xspan+1;
  while(i<=tab->x)
  {
   if(localxsum>0)
   {
    if(tab->xsum[i]>tab->realtdwidth[i])
    {
     rate=(float)tab->xsum[i]/(float)localxsum;  // 0.0 ... 1.0, sum(rate)=1
     xsumpercolumn=rate*(float)xsum;
     if(xsumpercolumn>tab->xsum[i])
      tab->xsum[i]=xsumpercolumn;
    }
    else
     xsumpercolumn=0;
   }
   else
   {
    xsumpercolumn=xsum/tab->xspan;
    if(xsumpercolumn>tab->xsum[i])
    {
     xsumpercolumn-=tab->xsum[i];
     if(xsumpercolumn>tab->varxsum[i])
      tab->varxsum[i]=xsumpercolumn;
    }

   }

   i++;
  }

  //fix actual table width
  if(maxx-*cellx>0)
  {
   if(tab->y) //pokud nejsem na prvni radce, musim prekreslovat
    // tr.: if I am not on the first line I need to redraw
    rv=1;
   *cellx=maxx;
  }
 }
 //----------------------------------------------------end <TD COLSPAN>1> :

 //code for tracing width
 fixtracewidth(tab,maxx,tab->x,tab->xspan);

 if(tab->rowspan[tab->x]==1) //zarovnat radek tabulky ?
    // tr.: align row of the table? 
 {
  if(y>tab->tdend)
   tab->tdend=y;
 }
 else
 {
  if(tab->rowspan[tab->x]==2 && y>tab->nexttdend)
   tab->nexttdend=y;

  if(y>tab->maxtdend)
   tab->maxtdend=y;
 }

 return rv;
}

// ========================================================================
// define new cell in HTML table - called by render_html()
// ========================================================================

void newcell(struct HTMLtable *tab,int xspan,int yspan,int *tdx,long *tdy,int *width, char perc,int tdwidth)
{
 int i;
 int currentwidth=0;

 *tdx=0;
 *tdy=tab->tdstart;

 //preskocit rowspan
 // tr.: skip rowspan
 tab->x=determine_new_x(tab);

 if(xspan>1) //for multicolumn cells, reset shrink bits
 {
  if(tab->shrink[tab->x+1]&SHRINK_LEFT_ALLOWED)
   tab->shrink[tab->x+1]-=SHRINK_LEFT_ALLOWED;

  if(tab->shrink[tab->x+xspan]&SHRINK_RIGHT_ALLOWED)
   tab->shrink[tab->x+xspan]-=SHRINK_RIGHT_ALLOWED;

  tab->shrink[tab->x+1]|=SHRINK_LEFT_FORBIDDEN;
  tab->shrink[tab->x+xspan]|=SHRINK_RIGHT_FORBIDDEN;

  if(!(tab->shrink[tab->x+xspan]&SHRINK_LEFT_FORBIDDEN))
   tab->shrink[tab->x+xspan]|=SHRINK_LEFT_ALLOWED;

  i=tab->x+1;
  while(i<tab->x+xspan)
  {
   //set shrink bits
   if(!(tab->shrink[i]&SHRINK_RIGHT_FORBIDDEN))
    tab->shrink[i]|=SHRINK_RIGHT_ALLOWED;
   if(i>tab->x+1 && !(tab->shrink[i]&SHRINK_LEFT_FORBIDDEN))
    tab->shrink[i]|=SHRINK_LEFT_ALLOWED;
   i++;
  }

  perc/=xspan;
 }
 else if(tdwidth)
 {
  tab->shrink[tab->x+1]|=PIXEL_RESTRICTED_TD;
 // refloatpercents(tab,tab->x+1,*width);
 }

 if(perc)  // ....... is width defined using percents ?
 {
  i=tab->x+1;

  //for all columns
  while(i<=tab->x+xspan)
  {
   if(perc>tab->percent[i])
    tab->percent[i]=perc;
   i++;
  }//loop
 }

 if(*width) // ........ is width defined ?
 {
  i=tab->x+1;

  //for all columns
  while(i<=tab->x+xspan)
  {
   //add actual real columnt width
   currentwidth+=tab->realtdwidth[i];

   //for multicolumn cells....
   if(i<tab->x+xspan)
   {
    //add cellspacing
    currentwidth+=tab->cellspacing;
   }

   i++;
  }//loop

  //new cell will expand current column
  if(*width-currentwidth>0 && xspan==1)
   tab->realtdwidth[tab->x+1]=*width;
 }

 //code for tracing width
 fixtracewidth(tab,*width,tab->x+xspan,xspan);

 *width=0;

 if(tab->x+xspan>MAXTD-1)
  xspan=0;

 i=0;
 while(i<tab->x+xspan)
 {
  i++;
  if(i>tab->x)
  {
   tab->rowspan[i]=yspan;
   if(i<MAXROWSPANTD)
    tab->closerowspan[i]=IE_NULL;
   *width+=tab->realtdwidth[i];
  }
  else
   *tdx+=tab->cellspacing+tab->realtdwidth[i];
 }


 tab->x+=xspan;
 tab->xspan=xspan;
 if(tab->x>tab->columns)
  tab->columns=tab->x;
 *tdx+=tab->cellspacing;
 *width+=(xspan-1)*tab->cellspacing;

 if(tab->x-xspan>0 && *tdx<tab->tracewidth[tab->x-xspan]+((tab->x-xspan)*tab->cellspacing))
  *tdx=tab->tracewidth[tab->x-xspan]+((tab->x-xspan)*tab->cellspacing);

}

// ========================================================================
// recalculate table width - phase 1
// ========================================================================

char calcwidth(struct HTMLtable *tab)
//return 0 - we don't have to expand the table
//       1 - we must expand the table - second parsing is needed
{
 int i=1,willexpand=0;

 tab->totalxsum=0l;
 tab->realwidth=tab->cellspacing;

 while(i<=tab->columns)
 {
  tab->realwidth+=tab->cellspacing;
  tab->realwidth+=tab->realtdwidth[i];

  tab->xsum[i]+=tab->varxsum[i];

  tab->xsum[i]-=tab->realtdwidth[i]-2*tab->cellpadding;
  if(tab->xsum[i]<0)
   tab->xsum[i]=0; //only overhead
  tab->totalxsum+=tab->xsum[i];

  if (tab->realtdwidth[i]>tab->tracewidth[i]-tab->tracewidth[i-1])
  {
   tab->tracewidth[i]=tab->tracewidth[i-1]+tab->realtdwidth[i];
   willexpand=1;
  }

  i++;
 }

 if(willexpand)
  return 1;

 //table expansion test:
 if(tab->realwidth<tab->maxwidth && (tab->totalxsum || tab->fixedmax))
 {
  if(tab->totalxsum+tab->realwidth<tab->maxwidth &&
     (tab->totalxsum>0 || tab->fixedmax))
   return 1;

  i=1;
  while(i<=tab->columns)
  {
   if(tab->xsum[i]>tab->realtdwidth[i] && tab->xsum[i])
    return 1;
   i++;
  }
 }
 return 0;
}

// ========================================================================
// expand (optimize) columns of HTML table; called by render_html()
// ========================================================================

void expand(struct HTMLtable *tab)
{
 int i=1;
 int delta;
 int prebytek;//,left;
 int sum=0;

 if(tab->columns==0)
  return;

 //let's expand columns proportionaly
 if(tab->realwidth+FUZZYWIDTH<tab->maxwidth && (tab->fixedmax || tab->totalxsum))
 {
  //let's expand table!
  prebytek=(int)(tab->maxwidth-tab->realwidth);

  //special case - no columns will wrap
  if(tab->fixedmax && tab->totalxsum==0)
  {
   delta=prebytek/(int)(tab->columns);
   //printf("[delta - %d]",delta);
   if(delta>0)
   {
    while(i<=tab->columns)
    {
     if(!(tab->shrink[i]&PIXEL_RESTRICTED_TD))
     {
      tab->realtdwidth[i]+=(int)tab->xsum[i]+delta;
      tab->realwidth+=(int)tab->xsum[i]+delta;
     }
     i++;
    }
   }
   return;
  }//endif not wrapped columns

  //to avoid divide by zero:
  if(tab->totalxsum==0)
   return;

  while(i<=tab->columns)
  {
   if(/* ?? tab->xsum[i]>tab->realtdwidth[i]&&*/
      !(tab->shrink[i]&PIXEL_RESTRICTED_TD) && tab->xsum[i] || tab->percent[i])
   {
    delta=(int)((long)((long)prebytek*tab->xsum[i])/tab->totalxsum);

    if(tab->percent[i])
    {
     int realpercents=(int)(((long)(tab->maxwidth-tab->cellspacing)*tab->percent[i])/100)-tab->cellspacing;

     if(realpercents<tab->realtdwidth[i]+delta)
      delta=realpercents-tab->realtdwidth[i];

     if (realpercents>tab->realtdwidth[i]+delta)
     {
      int zbyva=(int)(tab->maxwidth-tab->realwidth);
      delta=realpercents-tab->realtdwidth[i];
      if(delta>zbyva)
       delta=zbyva;
     }

    }

    if(delta>prebytek)
     delta=prebytek;

//!!Ray & !!glennmcc: Sep 21, 2007 -- fix TD width problem
if(delta<0) delta=0;
if(delta+tab->realwidth>tab->maxwidth+FUZZYPIX)
   delta=tab->maxwidth+FUZZYPIX-tab->realwidth;
//    if(delta<0  || delta+tab->realwidth>tab->maxwidth+FUZZYPIX)
//     delta=0;
// original 2 lines of code above this comment
//!!Ray & !!glennmcc: end

    //for tables with undefined width, where delta is bigger than necessary
    if(tab->fixedmax==0 && delta>tab->xsum[i])
     delta=(int)tab->xsum[i];

    tab->realtdwidth[i]+=delta;
    tab->realwidth+=delta;
   }

   sum+=tab->realtdwidth[i];
   if(sum<tab->tracewidth[i])
   {
    tab->realwidth+=tab->tracewidth[i]-sum;
    tab->realtdwidth[i]+=tab->tracewidth[i]-sum;
    prebytek-=tab->tracewidth[i]-sum;
    if(prebytek<0)
     prebytek=0;
   }
   else if(tab->percent[i]) // table was distorted by percent expansion ?
   {
    int j=i,shft=sum-tab->tracewidth[i];
    while(j<=tab->columns)
     tab->tracewidth[j++]+=shft;
   }
   i++;
  }//loop
 }
 else //do not expand
 {
  while(i<=tab->columns)
  {
   sum+=tab->realtdwidth[i];
   if(sum<tab->tracewidth[i])
   {
    tab->realwidth+=tab->tracewidth[i]-sum;
    tab->realtdwidth[i]+=tab->tracewidth[i]-sum;
   }
   i++;
  }//loop
 }//endif
}

// ========================================================================
// fix <TD ROWSPAN>1>; called by render_html()
// ========================================================================

void fixrowspan(struct HTMLtable *tab,int closetable, XSWAP *closeptrs)
{
 int i=0,nospan=1,closecount=0;

 while(i<=tab->columns)
 {
  if(tab->rowspan[i])
  {
   tab->rowspan[i]--;
   if(tab->rowspan[i])
    nospan=0;
   else //!rowspan!
    if(i<MAXROWSPANTD && tab->closerowspan[i]!=IE_NULL && closecount<MAXROWSPANTD)
     closeptrs[closecount++]=tab->closerowspan[i];
  }
  i++;
 }
 if((nospan || closetable) && tab->maxtdend>tab->tdend)
  tab->tdend=tab->maxtdend;

 closeptrs[closecount]=IE_NULL;
}

void fixrowspan_y(XSWAP *closeptrs,long y,int padding)
{
 int i=0;

 while(i<MAXROWSPANTD && closeptrs[i]!=IE_NULL)
  closeatom_y(closeptrs[i++],y,padding);
}



