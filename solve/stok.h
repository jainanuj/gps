#ifndef _STOK_H
#define _STOK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TOKEN_INT 1
#define TOKEN_FLOAT 2
#define TOKEN_DOUBLE 3

void reset_tokenizer( void );
int get_token( int type, FILE *fp, void *datum );

#endif
