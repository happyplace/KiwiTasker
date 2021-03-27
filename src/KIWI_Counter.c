#include "kiwi/KIWI_Counter.h"

#include <malloc.h>

#include "kiwi/KIWI_Std.h"
#include "kiwi/KIWI_Atomics.h"

typedef struct KIWI_Counter
{
	atomic_int value;
} KIWI_Counter;

struct KIWI_Counter* KIWI_CreateCounter()
{
	KIWI_Counter* counter = malloc(sizeof(KIWI_Counter));
	if (counter == NULL)
	{
		KIWI_ASSERT(!"are we out of memory???");
		return NULL;
	}

	atomic_store(&counter->value, 0);

	return counter;
}

void KIWI_FreeCounter(struct KIWI_Counter* counter)
{
	KIWI_ASSERT(counter);

	free(counter);
}

void KIWI_IncrementCounter(struct KIWI_Counter* counter)
{
	KIWI_ASSERT(counter);
	
	atomic_fetch_add(&counter->value, 1);
}

int KIWI_DecrementCounter(struct KIWI_Counter* counter)
{
	KIWI_ASSERT(counter);

	int value = atomic_fetch_sub(&counter->value, 1);

	// we subtract one to the value because it returns the value it was BEFORE it was decremented
	return value - 1;
}
