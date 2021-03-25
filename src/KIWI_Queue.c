#include "kiwi/KIWI_Queue.h"

#include <malloc.h>

#include "kiwi/KIWI_Std.h"

typedef struct KIWI_Queue
{
	int dummy;
} KIWI_Queue;


struct KIWI_Queue* KIWI_InitializeQueue(int elementSize, int maxCapacity)
{
	(void)elementSize;
	(void)maxCapacity;

	KIWI_Queue* queue = malloc(sizeof(KIWI_Queue));
	return queue;
}
  
void KIWI_FreeQueue(KIWI_Queue* queue)
{
	KIWI_ASSERT(queue);
	free(queue);
}
