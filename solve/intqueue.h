
#include <stdio.h>
#include <stdlib.h>

#ifndef _PART_QUEUE_H
#define _PART_QUEUE_H

typedef struct {
  int *items;
  int numitems, maxitems;
  int start_item_ptr, end_item_ptr;
} queue;

queue* queue_create( int maxitems );

int queue_add( queue *q, int a );
int queue_pop( queue *q, int *result );
int check_obj_present_in_q( queue *q, int obj);
int queue_has_items(queue *q);

#endif
