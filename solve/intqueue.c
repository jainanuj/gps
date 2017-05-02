
#include "intqueue.h"



int check_obj_present_in_q(queue* q, int obj)
{
    int items_scanned = 0;
    int i = q->start_item_ptr;
    for (items_scanned = 0; items_scanned < q->numitems; items_scanned++)
    {
        if (q->items[i] == obj)
            return i;
        else
            i = ((i + 1)  % q->maxitems);
    }
    
    return -1;
}

//
// ----------------------------------------------------------------------------
//

queue *queue_create( int maxitems ) {
  queue *q;

  q = (queue *)malloc( sizeof(queue) );
  if (q == NULL) {
    fprintf(stderr, "Out of memory!\n");
    exit(0);
  }

  q->maxitems = maxitems;
  q->numitems = 0;

  q->items = (int *)malloc( maxitems * sizeof(int) );
  if (q->items == NULL) {
    fprintf(stderr, "Out of memory!\n");
    exit(0);
  }
    q->end_item_ptr = 0;
    q->start_item_ptr = 0;

  return q;
}

//
// ----------------------------------------------------------------------------
//
// dump the object at the end of the array.
//

int queue_add( queue *q, int obj )
{
    int pos = 0;

    pos = check_obj_present_in_q(q, obj);
    if (pos != -1)
        return 1;     //Item already present at position pos.
    
    if ( q->numitems >= q->maxitems )
    {
        fprintf(stderr, "Hey!  Queue's full!\n");
        return 0;
    }

    // tack the new object onto the end of the queue.
    pos = q->end_item_ptr;
    q->items[ q->end_item_ptr ] = obj;
    q->end_item_ptr = ((q->end_item_ptr + 1 ) % q->maxitems);
    q->numitems++;
    return 1;
}

//
// ----------------------------------------------------------------------------
//

int queue_pop(queue *q, int *result )
{
    if ( (q->numitems <= 0) || (q->start_item_ptr < 0) )
    {
        fprintf( stderr, "Hey! queue's empty!\n" );
        return 0;
    }

    *result = q->items[q->start_item_ptr];
    q->numitems--;
    q->start_item_ptr = ((q->start_item_ptr + 1 ) % q->maxitems);

    return 1;
}
