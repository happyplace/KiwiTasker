#include "kiwi/KIWI_Counter.h"

#include <malloc.h>

#include "kiwi/KIWI_Std.h"
#include "kiwi/KIWI_SpinLock.h"

typedef struct KIWI_CounterData
{
	int value;
	struct KIWI_SpinLock* lock;
} KIWI_CounterData;

struct KIWI_Counter* KIWI_CreateCounter()
{
	KIWI_Counter* counter = malloc(sizeof(KIWI_Counter));
	if (counter == NULL)
	{
		KIWI_ASSERT(!"are we out of memory???");
		return NULL;
	}

	KIWI_PrepareCounter(counter);

	return counter;
}

void KIWI_PrepareCounter(KIWI_Counter* counter)
{
	KIWI_ASSERT(counter);

	counter->data = malloc(sizeof(KIWI_CounterData));
	if (counter->data == NULL)
	{
		KIWI_ASSERT(!"are we out of memory???");
		return;
	}

	KIWI_CounterData* data = counter->data;
	data->value = 0;
	data->lock = KIWI_CreateSpinLock();
}

void KIWI_FreeCounter(KIWI_Counter* counter)
{
	KIWI_ASSERT(counter);
	KIWI_ASSERT(counter->data);

	KIWI_FreePrepareCounter(counter);

	free(counter);
}

void KIWI_FreePrepareCounter(KIWI_Counter* counter)
{
	KIWI_ASSERT(counter);

	KIWI_CounterData* data = counter->data;
	KIWI_FreeSpinLock(data->lock);

	free(counter->data);
}

void KIWI_IncrementCounter(KIWI_Counter* counter)
{
	KIWI_ASSERT(counter);
	KIWI_ASSERT(counter->data);
	
	KIWI_CounterData* data = counter->data;

	KIWI_LockSpinLock(data->lock);
	data->value++;
	KIWI_UnlockSpinLock(data->lock);
}

int KIWI_DecrementCounter(KIWI_Counter* counter)
{
	KIWI_ASSERT(counter);
	KIWI_ASSERT(counter->data);

	KIWI_CounterData* data = counter->data;

	KIWI_LockSpinLock(data->lock);
	data->value--;
	int value = data->value;
	KIWI_UnlockSpinLock(data->lock);

	return value;
}

int KIWI_CounterLockAndGetValue(KIWI_Counter* counter)
{
	KIWI_ASSERT(counter);
	KIWI_ASSERT(counter->data);

	KIWI_CounterData* data = counter->data;

	KIWI_LockSpinLock(data->lock);
	return data->value;
}

void KIWI_CounterUnlock(KIWI_Counter* counter)
{
	KIWI_ASSERT(counter);
	KIWI_ASSERT(counter->data);

	KIWI_CounterData* data = counter->data;

	KIWI_UnlockSpinLock(data->lock);
}

void KIWI_CounterResetToZero(KIWI_Counter* counter)
{
	KIWI_ASSERT(counter);
	KIWI_ASSERT(counter->data);

	KIWI_CounterData* data = counter->data;
	data->value = 0;
}
