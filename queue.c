//SSOO-P3 2022-2023

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"



//To create a queue
queue* queue_init(int size){

	queue * q = (queue *)malloc(sizeof(queue));
    q->inicio = 0;
    q->fin = 0;
    q->size = size;
    q->array_elements = (struct element**)malloc(sizeof(struct element*) * size); 
    q->last_acc = 0;
	return q;
}


// To Enqueue an element
// Esta función devuelve 0 en caso de éxito y 1 en caso de fracaso
int queue_put(queue *q, struct element* x) {
    if (queue_full(q) == 0)
    {
        q->array_elements[q->fin] = (struct element*)malloc(sizeof(struct element));
        *(q->array_elements[q->fin]) = *x;
        if(q->fin +1 != q->size)
        {
            q->fin = q->fin + 1;
        }
        else
        {
            q->fin = 0;
        }
        q->last_acc = 1;
        return 0;
    }
    else
    {
    return 1;
    }

}


// To Dequeue an element.
struct element* queue_get(queue *q) {
	struct element* element = (struct element*)malloc(sizeof(struct element));
	if(queue_empty(q) == 0)
	{
	    *element = *q->array_elements[q->inicio];
	    q->array_elements[q->inicio] = NULL;
	    if(q->inicio +1 != q->size)
        {
            q->inicio = q->inicio + 1;
        }
        else
        {
            q->inicio = 0;
        }
	    q->last_acc = 0;
	    return element;
	}
	    return NULL;
}


//To check queue state
int queue_empty(queue *q){
	if (q->fin == q->inicio && q->last_acc == 0)
	{
	    return 1;
	}
	else
	{
	    
	    return 0;
	}
}


int queue_full(queue *q){
	if (q->fin == q->inicio && q->last_acc == 1)
	{
	    return 1;
	}
	else
	{
	    
	    return 0;
    }
}

//To destroy the queue and free the resources
int queue_destroy(queue *q){
    
    for(int i = 0;i< q->size;i++)
    {
        free((void *)q->array_elements[i]);
           
    }
    free((void*) q->array_elements);
    
    free((void *)q);
    
    
    
}

