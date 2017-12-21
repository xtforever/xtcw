#include "slop4.h"
#include "crc16-generic.h"
#include "mls.h"


#define SLOP_CACHE (100*1024)
#define SLOP_MSG 1024
typedef uint16_t u16;
typedef uint8_t u8;



/* SLOP special character codes
 */
#define END             10    /* indicates end of packet */
#define ESC             92    /* indicates byte stuffing */
#define ESC_END         110   /* ESC ESC_END means END data byte */
#define ESC_ESC         95    /* ESC ESC_ESC means ESC data byte */
#define ESC_CRC         91    /* END CHUNK with CRC CHECK        */

static uint16_t asctohex(uint16_t val, uint8_t asc) {
    if( asc >= 'a' ) asc -= 'a' - 10; else asc -= '0';
    return ((uint16_t)(val << 4)) | asc;
}

static uint8_t hextoasc(uint16_t val) {
    uint8_t asc = val >> 12;
    return ( asc > 9 ) ? asc + 'a' - 10 : asc + '0';
}

static u16 crc_update_byte(u16 crc, u8 d)
{
    return crc_update(crc,&d,1);
}

static void slop_free_message(int buf )
 {
    int p; 
    int *cur;
    if( !buf ) return;
    m_foreach(buf,p,cur) {
	if( *cur ) m_free(*cur);
    }
    m_clear(buf);
}

static int slop_add_msg(int msg)
{
    int *m = m_add(msg);
    *m = m_create(SLOP_CACHE, 1);
    // m_putc(*m,'+');
    return *m;
}

void slop_init(struct slop_state *slop)
{
    slop->state=0;
    slop->msg=m_create(5,sizeof(int));

}

void slop_free(struct slop_state *slop)
{
    slop_free_message(slop->msg); m_free(slop->msg);
}


static int slop_current_msg(struct slop_state *slop)
{
    
    if( m_len(slop->msg) == 0 )
	return slop_add_msg(slop->msg);
    return INT(slop->msg, m_len(slop->msg)-1 );    
}

void slop_put( struct slop_state *slop, int ch )
{
    int m = slop_current_msg(slop);

    if( ch == END ) {
        TRACE(1, "END" );
	if( m_len(m) || m_len(slop->msg) > 1 ) {
	    slop->error += slop->state; 	    
            /* send message */
	    
	    slop->cb(slop->error, slop->msg, slop->ctx);
	    slop_free_message(slop->msg);
	}
	slop->crc = crc_init();
	slop->error = 0;
	slop->state = 0;
	return;
    }


    switch( slop->state ) {
    default:
	slop->state=0;
	return;
	
    case 0: /* wait for start/end of message */	
	if( ch == ESC ) {     TRACE(1, "ESC" );
	    slop->state=5;
	    return;
	}
	break; /* store byte */
		
    case 5: /* wait for escaped code */
	if( ch == ESC_CRC ) {    TRACE(1, "ESC CRC" );
	    slop->state=1; /* recive crc */
	    slop->cs=0;
	    return;
	}
	if( ch == ESC_END ) ch=END; 
	if( ch == ESC_ESC ) ch=ESC;
	slop->state=0;
	break; /* store byte */

	/* escape crc byte 1..4 received */
    case 1: /* byte 1 */
    case 2: /* byte 2 */
    case 3: /* byte 3 */
    case 4: /* byte 4 */
	slop->cs = asctohex(slop->cs,ch);
	if( slop->state < 4 ) {
	    slop->state++;
	    return;
	}

	/* end of message */
	slop->crc = crc_finalize(slop->crc);
	TRACE(1, "CRC %4x %4x", slop->cs, slop->crc);
	if( slop->crc != slop->cs ) {
	    slop->error=1;
	}
	slop_add_msg(slop->msg);
	slop->state=0;
	slop->crc=crc_init();
	return;
    }

    if( m_len(m) < SLOP_CACHE ) {
	m_putc(m, ch);
	TRACE(1,"store byte %c", ch);
    }
    slop->crc = crc_update_byte(slop->crc,ch);
}


void slop_encode_msg(int m,
		     const char *p, int len, int with_crc )
{
    
    crc_t crc = crc_init();
    /* send an initial END character to flush out any data that may
     * have accumulated in the receiver due to line noise
     */
    m_putc(m, END);
    /* for each byte in the packet, send the appropriate character
     * sequence
     */
    while(len--) {

        switch(*p) {
            /* if it's the same code as an END character, we send a
             * special two character code so as not to make the
             * receiver think we sent an END
             */
        case END:
	    m_putc(m, ESC);
	    m_putc(m, ESC_END);
            break;

            /* if it's the same code as an ESC character,
             * we send a special two character code so as not
             * to make the receiver think we sent an ESC
             */
        case ESC:
	    m_putc(m, ESC);
	    m_putc(m, ESC_ESC);
            break;
            /* otherwise, we just send the character
             */
        default:
	    m_putc(m, *p);
        }
        crc = crc_update(crc, p, 1 );
        p++;
    }

    if( with_crc ) {
	m_putc(m, ESC);
	m_putc(m, ESC_CRC);
	crc = crc_finalize(crc);
        for(int i=0;i<4;i++, crc <<= 4 )
	    m_putc(m, hextoasc(crc) );
    }

    /* tell the receiver that we're done sending the packet
     */
    m_putc(m, END);
}






#define UC_OUT(c) m_putc(buf,c)

#define UC_OUT16X(crc) do {			\
    u8 i; for(i=0;i<4;i++, crc <<= 4 )		\
	      UC_OUT( hextoasc(crc) );		\
    } while(0)


/* add ch to slop encoded buf,
   if ch < 0 a crc checksum is created
*/
int slop_encode( int buf, int crc, int ch )
{
    if( ch < 0 ) { /* footer */
	UC_OUT( ESC );		
	UC_OUT( ESC_CRC );
	UC_OUT16X(crc);		
	UC_OUT(END);		
	return -1;
    }
    
    switch( ch ) {
    default:			/* payload */
	UC_OUT(ch);
	break;
    case END:			/* escaped char */
	UC_OUT(ESC);
	UC_OUT(ESC_END);
	break;
    case ESC:			/* escaped char */
	UC_OUT(ESC);
	UC_OUT(ESC_ESC);
	break;
    }
    return crc = crc_update_byte(crc,ch);    
}


int slop_encode_str( int buf, int crc, char *s )
{
    if(s==0) ERR("");
    while(*s) 
	crc = slop_encode(buf,crc, *s++ );
    return crc;
}

