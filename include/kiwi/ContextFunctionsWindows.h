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
