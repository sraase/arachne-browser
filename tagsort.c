/*
#Identification: TAGSORT.FOR
#Revision: B003

#Task:  Sort for PALL
#First issue: 901019
#Author: POLA Ivan Polak
#Changes: %datum: %autor:
#===============================================================*/
#include <stdio.h>

  int TagSort(unsigned long *ival,int *itag,int ip,int ik)
{
/*#parametry:
 ival - i - pole s hodnotami klicu
 itag - o - pole ukazatelu do ival
 ip,ik -i - pocatecni a koncovy index tridene casti ival

 return: 1 = O.K.
	 2 = preplneni vnitr. zasobniku (asi nemuze nastat)
	 4 = kiks (urcite nemuze nastat)

#Funkce:
  V poli ITAG jsou po skonceni prace adresy do pole IVAL a to tak,
  aby posloupnost IVAL(ITAG(i)) byla SESTUPNA (!).
  Trideni metodou QUICKSORT se zasobnikem.
  TAGSORT vznikl z EXTSOR, ktery vznikl upravou MON3SOR_L:QSORT1.FOR
  resp. MON3SOR_L:UMQSOR

tr.: 

 #parameters:
 ival - i - field with values of sorting keys 
 itag - o - field of pointers to ival
 ip,ik -i - initial and final index of sorted parts ival

 return: 1 = O.K.
         2 = overflow if the inner storage (probably cannot happen)
         4 = flop (certainly cannot happen)

#Funkce:
  After having finished the operation, the field ITAG contains the
  addresses into the field IVAL in that way, that the order of
  IVAL(ITAG(i)) will be DESCENDING (!).
  Sorting with method QUICKSORT with storage. 
  TAGSORT emerged from EXTSOR, that was developed from 
  MON3SOR_L:QSORT1.FOR resp. MON3SOR_L:UMQSOR

c ***************************************************************
*/

#define MQSTA 200
      int qsta[MQSTA]; // storage 

      int jx,is, l,r, i,j, ispr,iw;      //auxiliary variable 

// **********************************************************************
//     main:

for(i=ip; i <= ik; i++) itag[i]= i;    // initial setting itag

      is=1;                // whole field into storage
      qsta[is]=ip;
      qsta[is+1]=ik;
//--------------------
L10:  l=qsta[is];          // take from storage 
      r=qsta[is+1];
      is=is-2;
//-------------------
L20:  i=l;                 // sorting section <i,j>
      j=r;
      ispr=((l+r)/2)+1;
      if(ispr > r) ispr=r;
      if(ispr < l) ispr=l;      // central/middle index
      jx=itag[ispr];
//-------------------

//  30 if(a[i]-x[1]) 200,110,300     // 200=.true. 300=.false
//30    if(ival[itag[i]] .lt. ival[jx] ) then
L30:  if( ival[itag[i]]  >  ival[jx] ) //descending!
	goto L200;            // 200=.true. 300=.false
      else
	goto L300;
L200:  i++;
      goto L30;

//  300 if(x[1]-a[j]) 600,410,700     // 600=.true. 700=.false
//300   if( ival[jx] .lt. ival[itag[j]] ) then
L300:  if( ival[jx]  >  ival[itag[j]] ) //descend.
	goto L600;            // 600=.true. 700=.false
      else
	goto L700;
L600: j--;
      goto L300;

L700:  if(i <= j) // prohozeni (tr.: swap)
       { if(i == j) goto L33;

//       call lib$movc3(len,a(i),waa)
//       call lib$movc3(len,a(j),a(i))
//       call lib$movc3(len,waa,a(j))
//-------- super secure - pol 930114 !!!!!!!!!!!!!!1
	if(i<ip || j>ik) return 4;
//-----------------------------------
	iw=itag[i];
	itag[i]=itag[j];
	itag[j]=iw;

L33:    i++;
	j--;
      }
      if(i <= j) goto L30;

      if(i <  r) // save right part into stacku
      {is=is+2;
//p:       if(is >= MQSTA) perror("TagSort-plny zasobnik"); //stop ''
       if(is >= MQSTA) return 2;
       qsta[is]=i;
       qsta[is+1]=r;
      }

      r=j;                 // sorting of the rest of left part
      if(l <  r) goto L20;
      if(is >  0) goto L10;

      return 1;
}
