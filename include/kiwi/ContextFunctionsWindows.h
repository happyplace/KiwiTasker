#include "kiwi/Context.h"

namespace kiwi
{
    void SetupContext(kiwi::Context* context, void* instructionPointer, void* stackPointer, void* parameter)
    {
        context->rip = instructionPointer;
        context->rsp = stackPointer;
        context->rcx = parameter;
    }
}

extern "C" void get_context(kiwi::Context*);
extern "C" void set_context(kiwi::Context*);
extern "C" void swap_context(kiwi::Context*, kiwi::Context*);
