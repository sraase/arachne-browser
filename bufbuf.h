/***********************************************************************/
/* Function for saving and reloading buffers into XMS or to disk       */
/***********************************************************************/
/*                                             (c) Jan Vlaciha 1994    */

//int h_xmove(XMOVE *p);      // umoznuje odkladat buffery liche delky
   // tr.: makes it possible to save buffers of odd length

/***********************************************************************/
/*              Saving buffers into XMS or to disk                     */
/***********************************************************************/
// Parameters:  inbuf .... saved buffer
//              lenbuf ... length in bytes
//              bbuf ..... pointer for identification (see getIm)
// Return    :  1 ........ O.K.
//              2 ........ error while allocating
//              4 ........ error while working with XMS
//              6 ........ error while writing to disk

// Funkce    :  Odlozi buffer do XMS nebo (kdyz neni XMS) na disk.
//              Soubory na disku se poznaji podle pripony ._$B.
//              Predpoklada se existence a nastaveni globalni promenne
//              scratchBase se jmenem pracovniho adresare.
//              Odkladat lze buffery do maximalni delky 64kB.
// tr.: Buffer is saved into XMS or (if there is no XMS) to disk.
//      Files on disk are marked with the extension ._$B.
//      Program supposes the existence and setting of a global
//      variable scratchBase with the name of the working directory.
//      Buffers can be saved up to a maximum length of 64kB.

struct T_buf_buf
{
  unsigned int size;          // buffer size
  int          handle;        // handle into XMS, or number of file 
  char         medium;        // target medium ( 1 = XMS, 4 = disk )
};


//int saveBuf( char *inbuf, unsigned int lenbuf, char far **bbuf );
int saveBuf( char *inbuf, unsigned int lenbuf, struct T_buf_buf *bb );

/***********************************************************************/
/*             Get length of saved buffer                              */
/***********************************************************************/
// Parameters:  bbuf ..... pointer for identification
// Return    :  length of saved buffer

//unsigned int sizeBuf( char **bbuf );
unsigned int sizeBuf( struct T_buf_buf *bbuf );

/***********************************************************************/
/*                  Read from saved buffer                             */
/***********************************************************************/
// Parameters:  outbuf ... saved buffer
//              from ..... from which byte to read
//              lenbuf ... input  - how many bytes to read 
//                         output - how many bytes have actually been read
//              bbuf ..... pointer for identification (see getIm)
// Return    :  1 ........ O.K.
//              2 ........ bbuf is NULL
//              4 ........ error while working with XMS
//              6 ........ error while writing to disk

// Funkce    :  Nacte pozadovanou cast odlozeneho bufferu.
//              Nepodari-li se nacist pozadovanou delku,
//              vrati se v lenbuf skutecne nactena delka.
// tr.:  Reads requested part of the saved buffer. If it is not
//       possible to read the requested length, lenbuf returns
//       the length that has actually been read. 

//int fromBuf( char *outbuf, unsigned int from,
//	     unsigned int *lenbuf, char far **bbuf );
int fromBuf( char *outbuf, unsigned int from,
	     unsigned int *lenbuf, struct T_buf_buf *bb );


/***********************************************************************/
/*                  Delete saved buffer                                */
/***********************************************************************/
// Parameters:  bbuf ..... pointer for identification (see getIm)
// Return    :  1 ........ O.K.
//              2 ........ bbuf is NULL
//              6 ........ failed to delete

//int delBuf( char far **bbuf );
int delBuf( struct T_buf_buf *bb );

