/***********************************************************************/
/* Funkce pro odkladani a zpetne nacitani bufferu do XMS nebo na disk  */
/***********************************************************************/
/*                                             (c) Jan Vlaciha 1994    */

//int h_xmove(XMOVE *p);      // umoznuje odkladat buffery liche delky

/***********************************************************************/
/*              Odlozeni bufferu do XMS nebo na disk   		       */
/***********************************************************************/
// Parametry :  inbuf .... odkladany buffer
//              lenbuf ... delka v bytech
//              bbuf ..... pointer pro identifikaci (viz getIm)
// Return    :  1 ........ O.K.
//              2 ........ chyba pri alokaci
//              4 ........ chyba pri praci s XMS
//              6 ........ chyba pri psani na disk

// Funkce    :  Odlozi buffer do XMS nebo (kdyz neni XMS) na disk.
//              Soubory na disku se poznaji podle pripony ._$B.
//              Predpoklada se existence a nastaveni globalni promenne
//              scratchBase se jmenem pracovniho adresare.
//              Odkladat lze buffery do maximalni delky 64kB.

struct T_buf_buf
{
  unsigned int size;          // velikost bufferu
  int          handle;        // handle do XMS, nebo cislo souboru
  char         medium;        // cilove medium ( 1 = XMS, 4 = disk )
};


//int saveBuf( char *inbuf, unsigned int lenbuf, char far **bbuf );
int saveBuf( char *inbuf, unsigned int lenbuf, struct T_buf_buf *bb );

/***********************************************************************/
/*             Zjisteni delky odlozeneho bufferu    		       */
/***********************************************************************/
// Parametry :  bbuf ..... pointer pro identifikaci
// Return    :  delka odlozeneho bufferu

//unsigned int sizeBuf( char **bbuf );
unsigned int sizeBuf( struct T_buf_buf *bbuf );

/***********************************************************************/
/*                  Cetba z odlozeneho bufferu   		       */
/***********************************************************************/
// Parametry :  outbuf ... odkladany buffer
//              from ..... od ktereho bytu cist
//              lenbuf ... vstup  - kolik bytu se ma precist
//                         vystup - kolik bytu se skutecne precetlo
//              bbuf ..... pointer pro identifikaci (viz getIm)
// Return    :  1 ........ O.K.
//              2 ........ bbuf je NULL
//              4 ........ chyba pri praci s XMS
//              6 ........ chyba pri psani na disk

// Funkce    :  Nacte pozadovanou cast odlozeneho bufferu.
//              Nepodari-li se nacist pozadovanou delku,
//              vrati se v lenbuf skutecne nactena delka.

//int fromBuf( char *outbuf, unsigned int from,
//	     unsigned int *lenbuf, char far **bbuf );
int fromBuf( char *outbuf, unsigned int from,
	     unsigned int *lenbuf, struct T_buf_buf *bb );


/***********************************************************************/
/*                  Zruseni odlozeneho bufferu   		       */
/***********************************************************************/
// Parametry :  bbuf ..... pointer pro identifikaci (viz getIm)
// Return    :  1 ........ O.K.
//              2 ........ bbuf je NULL
//              6 ........ nepodarilo se zrusit

//int delBuf( char far **bbuf );
int delBuf( struct T_buf_buf *bb );

