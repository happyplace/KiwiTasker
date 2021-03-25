#pragma once

#include "kiwi/KIWI_std.h"

#ifdef __cplusplus
extern "C" {
#endif

	struct KIWI_Queue;

	// creates a queue with a max capacity of maxCapacity and a element size of elementSize. returns NULL on error.
	// The pointer returned by this function needs a call to KIWI_FreeQueue to free the memory
	extern DECLSPEC struct KIWI_Queue* KIWI_CreateQueue(int elementSize, int maxCapacity);

	// This frees all the memory associated with a queue created by KIWI_InitializeQueue
	extern DECLSPEC void KIWI_FreeQueue(struct KIWI_Queue* queue);

#ifdef __cplusplus
}
#endif
