
#include "odcd.h"

/* By setting this, nothing actually gets kicked out to disk.  We act
   as if we really were paging things in and out, though, so all of
   the statistics are accurate. */

/* #define ODCD_JUST_PROFILE */

/*
 * ----------------------------------------------------------------------------
 */

odcd_cache_t *odcd_cache_alloc( void ) {
  odcd_cache_t *o;

  o = (odcd_cache_t *)malloc( sizeof(odcd_cache_t) );
  if ( o == NULL ) {
    wlog( 1, "Couldn't allocate odcd_cache_t!\n" );
    return NULL;
  }

  memset( o, 0, sizeof(odcd_cache_t) );

  return o;
}

void odcd_cache_free( odcd_cache_t *o ) {
  free( o );
}

/*
 * ----------------------------------------------------------------------------
 */

int odcd_cache_init( odcd_cache_t *o, int cache_size, char *cache_file ) {

/*   wlog( 1, "Initializing ODCD cache [%s] w/size %d\n", */
/* 	cache_file, cache_size ); */

  o->cache_file = fopen( cache_file, "w+b" );
  if ( o->cache_file == NULL ) {
    wlog( 1, "Couldn't open odcd cache file %s!\n", cache_file );
    return 0;
  }

  o->total_in_cache = 0;
  o->total_cache_size = cache_size;
  o->cache_head = NULL;
  o->cache_tail = NULL;
  o->max_data_location = 0;
  o->cache_file_name = strdup( cache_file );

  return 1;
}

void odcd_cache_destroy( odcd_cache_t *o ) {
  if ( o->cache_file != NULL ) {
    fclose( o->cache_file );
  }
  unlink( o->cache_file_name );
}

/*
 * ----------------------------------------------------------------------------
 */

void odcd_elem_init( odcd_cache_t *o, odcd_cache_elem_t *e,
		     int data_size, char *data ) {

  assert( o != NULL );
  assert( e != NULL );

/*   wlog( 1, "ODCD: adding element %d (%d)\n", */
/* 	o->max_data_location, data_size ); */

  e->data_location = o->max_data_location;
  o->max_data_location += data_size;
  e->data_size = data_size;

  e->data = data;
  e->prev = NULL;
  e->next = NULL;

  /* set this object as the MRU (this adds it to the cache) */
  odcd_insert_as_mru( o, e );

  /* since we've added this element, we need to increment the total
     in our cache */
  odcd_inc_total( o );
}

/*
 * ----------------------------------------------------------------------------
 */

void odcd_load_data( odcd_cache_t *o, odcd_cache_elem_t *e ) {
#ifndef ODCD_JUST_PROFILE
/*   wlog( 1, "ODCD: loading element %d (%d)\n", */
/* 	e->data_location, e->data_size ); */
  e->data = (char *)malloc( e->data_size );
  if ( e->data == NULL ) {
    wlog( 1, "Couldn't allocate cache object (size=%d)!\n", e->data_size );
    exit( 0 );
  }
  fseek( o->cache_file, e->data_location, SEEK_SET );
  fread( e->data, e->data_size, 1, o->cache_file );
#endif
}

void odcd_store_data( odcd_cache_t *o, odcd_cache_elem_t *e ) {
#ifndef ODCD_JUST_PROFILE
/*   wlog( 1, "ODCD: storing element %d (%d)\n", */
/* 	e->data_location, e->data_size ); */
  fseek( o->cache_file, e->data_location, SEEK_SET );
  fwrite( e->data, e->data_size, 1, o->cache_file );
  free( e->data );
  e->data = NULL;
#endif
}

/*
 * ----------------------------------------------------------------------------
 */

void odcd_inc_total( odcd_cache_t *o ) {
  odcd_cache_elem_t *lru_elem;

  assert( o != NULL );

  o->total_in_cache++;

/*   wlog( 1, "ODCD: inc total (now %d)\n", o->total_in_cache ); */

  /* have we exceeded the cache size? */
  if ( o->total_in_cache > o->total_cache_size ) {
    /* by pulling something into cache, we've exceeded our cache size.
       Kick out the LRU element. */
    lru_elem = odcd_get_lru( o );
    assert( lru_elem != NULL );
    odcd_cache_kick_out( o, lru_elem );
  }

}

/*
 * ----------------------------------------------------------------------------
 */

void odcd_pull_out_of_list( odcd_cache_t *o, odcd_cache_elem_t *e ) {

  /* unchain this element from the lru list */

  e->in_cache = 0;

  /* is it both the head and the tail? */
  if ( o->cache_head == e && o->cache_tail == e ) {
    o->cache_head = NULL;
    o->cache_tail = NULL;
    return;
  }

  /* is it the head? */
  if ( o->cache_head == e ) {
    o->cache_head = e->next;
    assert( e->next != NULL );
    e->next->prev = NULL;
    return;
  }

  /* is it the tail? */
  if ( o->cache_tail == e ) {
    o->cache_tail = e->prev;
    assert( e->prev != NULL );
    e->prev->next = NULL;
    return;
  }

  /* must be in the middle */
  assert( e->prev != NULL );
  assert( e->next != NULL );

  e->prev->next = e->next;
  e->next->prev = e->prev;
}

void odcd_insert_as_mru( odcd_cache_t *o, odcd_cache_elem_t *e ) {

  assert( o != NULL );
  assert( e != NULL );

/*   wlog( 1, "ODCD: setting as mru %d\n", e->data_location ); */

  /* chain it back in at the bottom of the list */
  if ( o->cache_tail == NULL ) {
    assert( o->cache_head == NULL );
    o->cache_head = e;
    e->prev = NULL;

  } else {
    assert( o->cache_tail->next == NULL );
    o->cache_tail->next = e;
    e->prev = o->cache_tail;

  }

  e->in_cache = 1;
  e->next = NULL;
  o->cache_tail = e;
}

/*
 * ----------------------------------------------------------------------------
 */

char *odcd_cache_pull_in( odcd_cache_t *o, odcd_cache_elem_t *e ) {

  assert( o != NULL );
  assert( e != NULL );

/*   wlog( 1, "ODCD: pulling in %d\n", e->data_location ); */

  o->cache_tries++;

  if ( o->total_cache_size == 0 ) {
    odcd_load_data( o, e );
    return e->data;
  }

  if ( e->in_cache ) {
    /* requested object is already in cache */
    o->cache_hits++;
    odcd_pull_out_of_list( o, e );
    odcd_insert_as_mru( o, e );

  } else {

    /* pull the object into cache */
    odcd_load_data( o, e );

    odcd_insert_as_mru( o, e );
    odcd_inc_total( o );
  }

  return e->data;
}

/*
 * ----------------------------------------------------------------------------
 */

void odcd_cache_kick_out( odcd_cache_t *o, odcd_cache_elem_t *e ) {

  assert( o != NULL );
  assert( e != NULL );

/*   wlog( 1, "ODCD: kicking out %d\n", e->data_location ); */

  odcd_store_data( o, e );

  odcd_pull_out_of_list( o, e );

  e->next = NULL;
  e->prev = NULL;

  o->total_in_cache--;
}

/*
 * ----------------------------------------------------------------------------
 */

odcd_cache_elem_t *odcd_get_lru( odcd_cache_t *o ) {
  assert( o != NULL );
  return o->cache_head;
}

float odcd_hit_ratio( odcd_cache_t *o ) {
  return (float)(o->cache_hits)/(float)(o->cache_tries);
} 

float odcd_miss_ratio( odcd_cache_t *o ) {
  return (float)(o->cache_tries - o->cache_hits) /
    (float)(o->cache_tries);
}

/*
 * ----------------------------------------------------------------------------
 */
