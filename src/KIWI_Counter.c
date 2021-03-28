#include "kiwi/KIWI_Counter.h"

#include <malloc.h>

#include "kiwi/KIWI_Std.h"
#include "kiwi/KIWI_SpinLock.h"

typedef struct KIWI_Counter
{
	int value;
	struct KIWI_SpinLock* lock;
} KIWI_Counter;

struct KIWI_Counter* KIWI_CreateCounter()
{
	KIWI_Counter* counter = malloc(sizeof(KIWI_Counter));
	if (counter == NULL)
	{
		KIWI_ASSERT(!"are we out of memory???");
		return NULL;
	}

	counter->value = 0;
	counter->lock = KIWI_CreateSpinLock();

	return counter;
}

void KIWI_FreeCounter(struct KIWI_Counter* counter)
{
	KIWI_ASSERT(counter);

	KIWI_FreeSpinLock(counter->lock);
	free(counter);
}

void KIWI_IncrementCounter(struct KIWI_Counter* counter)
{
	KIWI_ASSERT(counter);
	
	KIWI_LockSpinLock(counter->lock);
	counter->value++;
	KIWI_UnlockSpinLock(counter->lock);
}

int KIWI_DecrementCounter(struct KIWI_Counter* counter)
{
	KIWI_ASSERT(counter);

	KIWI_LockSpinLock(counter->lock);
	counter->value--;
	int value = counter->value;
	KIWI_UnlockSpinLock(counter->lock);

	return value;
}

int KIWI_CounterLockAndGetValue(struct KIWI_Counter* counter)
{
	KIWI_ASSERT(counter);

	KIWI_LockSpinLock(counter->lock);
	return counter->value;
}

void KIWI_CounterUnlock(struct KIWI_Counter* counter)
{
	KIWI_ASSERT(counter);

	KIWI_UnlockSpinLock(counter->lock);
}
