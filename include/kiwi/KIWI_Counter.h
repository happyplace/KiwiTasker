#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct KIWI_Counter;

// Creates a counter, the memory returned needs to be freed using KIWI_FreeCounter when done with it
struct KIWI_Counter* KIWI_CreateCounter();

// frees memory of a spin lock
void KIWI_FreeCounter(struct KIWI_Counter* counter);

// This function is thread-safe
void KIWI_IncrementCounter(struct KIWI_Counter* counter);

// returns the value that this call decremented the counter to, this won't necessarily reflect the current
// value of the counter.
// This function is thread-safe
int KIWI_DecrementCounter(struct KIWI_Counter* counter);

#ifdef __cplusplus
}
#endif
