
#include "intqueue.h"
#include <sys/time.h>

/* don't use floats with this function -- you'll run into precision
 problems!*/
double whenq( void ) {
    struct timeval tv;
    gettimeofday( &tv, NULL );
    return (double)(tv.tv_sec) + (double)(tv.tv_usec)*(double)(1e-6);
}



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

#ifdef BITQ
  bit_queue *bq;
#endif

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
#ifdef BITQ
    bq = create_bit_queue(maxitems);
    q->bitqueue = bq;
#endif
    q->add_time = 0.0;
    q->pop_time = 0.0;

  return q;
}

//
// ----------------------------------------------------------------------------
//
// dump the object at the end of the array.
//

int queue_add( queue *q, int obj )
{
    double t_start, t_end;
    t_start = whenq();
    int pos = 0;

#ifndef BITQ
    pos = check_obj_present_in_q(q, obj);
    if (pos != -1)
        return 1;     //Item already present at position pos.
#else
    if (check_bit_obj_present(q->bitqueue, obj))
        return 1;
#endif
    
    if ( q->numitems >= q->maxitems )
    {
        fprintf(stderr, "Hey!  Queue's full!\n");
        return 0;
    }

    // tack the new object onto the end of the queue.
//    pos = q->end_item_ptr;
    q->items[ q->end_item_ptr ] = obj;
    q->end_item_ptr = ((q->end_item_ptr + 1 ) % q->maxitems);
    q->numitems++;
#ifdef BITQ
    queue_add_bit(q->bitqueue, obj);
#endif
    t_end = whenq();
    
    q->add_time += (t_end - t_start);
    
    return 1;
}

//
// ----------------------------------------------------------------------------
//

int queue_pop(queue *q, int *result )
{
    double t_start, t_end;
    t_start = whenq();
    if ( (q->numitems <= 0) || (q->start_item_ptr < 0) )
    {
        fprintf( stderr, "Hey! queue's empty!\n" );
        return 0;
    }
    
    *result = q->items[q->start_item_ptr];
    q->numitems--;
    q->start_item_ptr = ((q->start_item_ptr + 1 ) % q->maxitems);

#ifdef BITQ
    bit_queue_pop(q->bitqueue, *result);
#endif
    
    t_end = whenq();
    q->pop_time += (t_end - t_start);
    return 1;
}



int queue_has_items(queue *q)
{
    return q->numitems;
}

/*********----------------------------------------------------**************/

bit_queue *create_bit_queue( int max_items)
{
    bit_queue *bq;
    int num_bit_arrays, i;
    bq = (bit_queue *)malloc(sizeof(bit_queue));
    if (bq == NULL) {
        fprintf(stderr, "Out of memory!\n");
        exit(0);
    }
    
    //q->maxitems = max_items;
    
    num_bit_arrays = (max_items / BIT_ARRAY_SIZE) + 1;
    bq->bit_arrays = (unsigned long *)malloc(sizeof(unsigned long) * (num_bit_arrays) );
    if (bq->bit_arrays == NULL) {
        fprintf(stderr, "Out of memory!\n");
        exit(0);
    }
    for (i = 0; i < num_bit_arrays; i++)
        bq->bit_arrays[i] = 0;
    bq->max_bit_arrays = num_bit_arrays;
    return bq;
    
}

int check_bit_obj_present( bit_queue *bq, int obj )
{
    int index_bit_array = obj/BIT_ARRAY_SIZE;       //index of the bit array to use.
    int set_bit = obj - (index_bit_array * BIT_ARRAY_SIZE);     //The index of bit to be set in the chosen bit array
    unsigned long number_bitset = 0x1 << set_bit;        //The number with the required bit set.
    if (set_bit > BIT_ARRAY_SIZE)
    {
        fprintf(stderr, "Bit manip problem!\n");
        exit(0);
    }
    //will return non-zero if the bit is already set else zero.
    return (bq->bit_arrays[index_bit_array] & number_bitset);
}

int bit_queue_pop( bit_queue *bq, int obj )
{
    int index_bit_array = obj/BIT_ARRAY_SIZE;       //index of the bit array to use.
    int set_bit = obj - (index_bit_array * BIT_ARRAY_SIZE);     //The index of bit to be set in the chosen bit array
    unsigned long number_bit_unset = 0x1 << set_bit;        //The number with the required bit set.
    number_bit_unset = (~number_bit_unset) & MAX_DEB_SEQ_SIZE;
    if (set_bit > BIT_ARRAY_SIZE)
    {
        fprintf(stderr, "Bit manip problem!\n");
        exit(0);
    }
    bq->bit_arrays[index_bit_array] &= number_bit_unset;  //set the bit corresponding to obj as 0
    return 1;
    
}




int queue_add_bit( bit_queue *bq, int obj )
{
    int index_bit_array = obj/BIT_ARRAY_SIZE;       //index of the bit array to use.
    int set_bit = obj - (index_bit_array * BIT_ARRAY_SIZE);     //The index of bit to be set in the chosen bit array
    unsigned long number_bitset = 0x1 << set_bit;        //The number with the required bit set.
    if (set_bit > BIT_ARRAY_SIZE)
    {
        fprintf(stderr, "Bit manip problem!\n");
        exit(0);
    }
    //Setting the required bit in the appropriate bit array.
    bq->bit_arrays[index_bit_array] |= number_bitset;
    return 1;
}

/*
//Returns the partition number in result for which bit is popped.
int queue_pop_bit( bit_queue *q, int *result )
{
    int bit_array_index;
    unsigned long bit_array;
    int intermed_result;
    
    for (bit_array_index =0; bit_array_index < q->max_bit_arrays; bit_array_index++)
    {
        if (q->bit_arrays[bit_array_index] != 0)
        {
            bit_array = q->bit_arrays[bit_array_index];
            q->bit_arrays[bit_array_index] = bit_array & (bit_array - 1);  //Removing LSB - so popped the last partition.

            bit_array = bit_array - (bit_array & (bit_array-1));  //Only the bit indicating the partition number is set now.
            intermed_result = DEBRUIJNBITPOS[ ((bit_array * DEBSEQ) & MAX_DEB_SEQ_SIZE) >> DEB_SEQ_REM_WINDOW];
            
            *result = intermed_result + BIT_ARRAY_SIZE * bit_array_index;    //Adjusting result as per the bit array used.
            return 1;
        }
    }

    fprintf( stderr, "Hey! queue's empty!\n" );
    return 0;
    
}


int bit_queue_has_items(bit_queue *q)
{
    int bit_array_index = 0;
    for (bit_array_index =0; bit_array_index < q->max_bit_arrays; bit_array_index++)
    {
        if (q->bit_arrays[bit_array_index] != 0)
            return 1;
    }
    return 0;
}
*/

