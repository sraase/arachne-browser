/******************************************************************/
/*   Funkce pro cteni polohy myssi s vypnutym kursorem, fungujici */
/*   i v SVGA gr. modech                                          */
/******************************************************************/
/* (C) Ivan Polak, Jan Vlaciha, 1992 */

/***********************************************************************/
/*         ImouseIni - Inicializace snimani polohy                     */
/***********************************************************************/
/* Parametry :  xmin, ymin - levy horni roh pomyslneho okna, v nemz
			     se ma kursor pohybovat
		xmax, ymax - pravy dolni roh okna kursoru
		xstart, ystart - pocatecni poloha kursoru
Poznamky  : -zadane okno je "namapovano" na celou obrazovku pri
	     prave nastavenem grafickem modu;
	    -pred ImouseIni musi byt inicializovana grafika,
	     uvnitr je vyresestovana myss (a tudiz vypnut kursor myssi).
*/
/* Return    :  0 ...  neni instalovana Genius Mouse                    */
/*             -1 ... je instalovana      - " -                        */

int ImouseIni( int xmin, int ymin, int xmax, int ymax,
		int xstart, int ystart);
/***********************************************************************/
/*         ImouseRead - Snimani polohy  myssi                          */
/***********************************************************************/
/* Parametry :  xcurs, ycurs - okamzita poloha kursoru

Po zavolani ImouseIni a mezi opakovanim volani ImouseRead neni radno
volat gmouse driver, zejmena "Read cursor position" nebo "Set cursor
position"!

Return: "button status" myssi, tj.: bit 0 = leve tlacitko
				    bit 1 = prave tlacitko
				    bit 2 = prostredni tlac.
*/
int  ImouseRead( int *xcurs, int *ycurs);

/***********************************************************************/
/*         ImouseSet - Nastaveni polohy  myssi                         */
/***********************************************************************/
/* Parametry :   xstart, ystart - pozadovana poloha kursoru            */
void ImouseSet( int xstart, int ystart);

/***********************************************************************/
/*           ImouseFix - Fixovani jedne souradnice myssi               */
/***********************************************************************/
/* Parametr  :  fix ... 0 kurzor nefixovan                             */
/*                  ... 1 fixni x                                      */
/*                  ... 2 fixni y                                      */
/* Fixovana je jen ImouseRead, nikoliv ImouseSet.                      */
void  ImouseFix( int fix );

/***********************************************************************/
/*         ImouseBut - Indikace stisknuti tlacitka myssi               */
/***********************************************************************/
/* Pouze ceka na stisknuti tlacitka mysi. Pohyby mysi jsou zatim igno- */
/* rovany. Vraci stejne hodnoty jako ImouseRead.                       */
int  ImouseBut( void );
/***********************************************************************/
/*         ImouseWait - cekani na "uklidneni" mysi         */
/***********************************************************************/
/*  Fce ceka na uvolneni tlacitek mysi, pokud je nektere stisknuto.
    Potom udela reset. Ev.pohyby mysi do uvolneni tlacitka se tudiz
    ztrati.
    Fci je vhodne volat vzdy po ukonceni urcite akce vyvolane stiskem
    tlacitka mysi.                                                     */
void ImouseWait(void);

/***********************************************************************/
/*         ImouseLimit - zmena okna pro snimani polohy myssi           */
/***********************************************************************/
/* Parametry :  xmin, ymin - levy horni roh pomyslneho okna, v nemz
			     se ma kursor pohybovat
		xmax, ymax - pravy dolni roh okna kursoru

  Return    :  vzdy 1
*/

int ImouseLimit( int xmin, int ymin, int xmax, int ymax);
