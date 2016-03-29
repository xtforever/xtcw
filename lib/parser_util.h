#ifndef PARSER_UTIL_H
#define PARSER_UTIL_H

#include "mls.h"

/** kopiert den String |src| in das marray |m| 
    bis zum ersten WS. Führende WS werden ausgelassen.
    Falls |m| == 0, wird |m| erzeugt, ansonsten
    nur geleert.
    returns: |m| marray of char
*/
int parser_remove_whitespace(int m, char *src);

/** returns: -1 keine weiteren zeichen, 
            ansonsten das nächste byte
*/ 
int parser_next_char( int buf, int *p );

/** returns: 0 - |*p| zeigt auf das erste zeichen das 
                kein WS ist.
           -1 - keine weiteren zeichen
*/
int parser_skip_whitespace(int buf, int *p);


/* lese ein signed int aus buf, überspringe WS
   returns: -1 kein integer gefunden oder keine weiteren
               zeichen 
             0 sonst
*/
int parser_int(int buf, int *p, int *ret );

/** suche und überspringe skip_char, überspringe führende leerzeichen
 */
int parser_skip(int buf, int *p, int skip_char );

/** erzeuge eine kopie des strings im marray |buf| ab |*p| 
    und übergebe den zeiger in |s|. 
    returns: -1 falls der string leer wäre.
              0 sonst
*/  
int parser_strdup(int buf, int *p, char **s );

/* kein gleichzeichen und kein space erlaubt */
int parser_id(int buf, int *p, int ret_id );

#endif

