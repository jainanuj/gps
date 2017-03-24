
#include "stok.h"

#define SBUF_SIZE 8192

char buf[SBUF_SIZE];
int rem, cur;

void reset_tokenizer( void ) {
  rem = 0;
  cur = 0;
}

int get_token( int type, FILE *fp, void *datum ) {
  int tmp;

  if ( cur == -1 ) {
    fprintf( stderr, "Out of tokens!\n" );
    return 0;
  }

  if ( rem < 20 ) {
    /* move the remainder to the beginning of the buffer */
    memmove( buf, &(buf[cur]), rem );
    cur = 0;

    /* read some new stuff */
    rem += fread( &( buf[rem] ), sizeof(char), SBUF_SIZE-rem, fp );
  }

  switch( type ) {
  case TOKEN_INT:
    *((int *)datum) = atoi( &(buf[cur]) );
    break;

  case TOKEN_FLOAT:
    *((float *)datum) = atof( &(buf[cur]) );
    break;

  case TOKEN_DOUBLE:
    *((double *)datum) = atof( &(buf[cur]) );
    break;
  }

  /* skip to the next token */
  for ( tmp = cur; tmp < cur+rem; tmp++ ) {
    if ( isspace(buf[tmp]) ) {
      break;
    }
  }
  for ( ; tmp < cur+rem; tmp++ ) {
    if ( !isspace(buf[tmp]) ) {
      break;
    }
  }

  if ( tmp == cur+rem ) {
    /* no more tokens */
    cur = -1;

  } else {
    /* set up the next token */
    rem -= (tmp - cur);
    cur = tmp;
  }

  return 1;
}
