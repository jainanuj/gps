#ifndef _ODCD_H
#define _ODCD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "logger.h"

typedef struct odcd_cache_elem_t {
  int in_cache, data_size;
  long data_location;
  char *data;
  struct odcd_cache_elem_t *next, *prev;
} odcd_cache_elem_t;

typedef struct odcd_cache_t {
  int total_in_cache, total_cache_size;
  int cache_tries, cache_hits;
  odcd_cache_elem_t *cache_head, *cache_tail;
  long max_data_location;
  char *cache_file_name;
  FILE *cache_file;
} odcd_cache_t;

odcd_cache_t *odcd_cache_alloc( void );
void odcd_cache_free( odcd_cache_t *o );

int odcd_cache_init( odcd_cache_t *o, int cache_size, char *cache_file );
void odcd_cache_destroy( odcd_cache_t *o );

void odcd_elem_init( odcd_cache_t *o, odcd_cache_elem_t *e,
		     int data_size, char *data );

void odcd_load_data( odcd_cache_t *o, odcd_cache_elem_t *e );
void odcd_store_data( odcd_cache_t *o, odcd_cache_elem_t *e );

void odcd_inc_total( odcd_cache_t *o );

void odcd_pull_out_of_list( odcd_cache_t *o, odcd_cache_elem_t *e );
void odcd_insert_as_mru( odcd_cache_t *o, odcd_cache_elem_t *e );

char *odcd_cache_pull_in( odcd_cache_t *o, odcd_cache_elem_t *e );
void odcd_cache_kick_out( odcd_cache_t *o, odcd_cache_elem_t *e );

odcd_cache_elem_t *odcd_get_lru( odcd_cache_t *o );
float odcd_hit_ratio( odcd_cache_t *o );
float odcd_miss_ratio( odcd_cache_t *o );

#endif /* ndef _ODCD_H */
