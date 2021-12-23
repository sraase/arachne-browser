#include <stdio.h>

//-------- Hledani retezce v retezci ----------------
//  Pozn:  Retezce mohou obsahovat i znak "0"
//  Funkce hleda v bufru "buf" podretezec "str"

char far *x_strcmp(char far *buf, int lenb, char far *str, int lens)
//  buf - string ve kterem hledame, lenb- jeho delka
//  str - hledany retezec, lens- jeho delka
//  return - NULL - nenalezen, jinak - adresa str v buf
{
    int  ib,j,num;

    if(lens > lenb) return( NULL );

    num = lenb-lens+1;      // Pocet porovnani
    for(ib=0; ib<num; ib++)
     {
	for(j=0; j<lens; j++)
	 {  if(buf[ib+j] != str[j]) goto Krok;
	 }
	return( buf+ib );   // Nalezen

	Krok:;
     }
    return( NULL );        // Nenalezen
}