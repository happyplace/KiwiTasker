#include <iostream>

#include <chrono>
#include <thread>

#include "kiwi/Scheduler.h"

struct ExampleData
{
    int secretNumber = 0;
};

void ExampleJob(kiwi::Scheduler* schduler, void* arg)
{
    ExampleData* data = reinterpret_cast<ExampleData*>(arg);
    printf("The Secret Number is %i\n", data->secretNumber);
} 

int main(int /*argc*/, char** /*argv*/)
{
    kiwi::Scheduler scheduler;
    scheduler.Init();

    ExampleData jobData;
    kiwi::Job job { ExampleJob, &jobData };
    scheduler.AddJob(&job);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    return 0;
}
