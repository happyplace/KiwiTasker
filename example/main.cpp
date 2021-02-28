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

struct NumberData
{
    int num = 0;
};

void PrintNumberJob(kiwi::Scheduler* scheduler, void* arg)
{
    NumberData* data = reinterpret_cast<NumberData*>(arg);

    printf("Number: %i\n", data->num);
}

void WaitExampleJob(kiwi::Scheduler* scheduler, void* arg)
{
    printf("WaitExampleJob - Start\n");

    kiwi::Job* jobs = new kiwi::Job[25];
    NumberData* data = new NumberData[25];
    for (int i = 0; i < 25; ++i)
    {
        jobs[i].m_function = PrintNumberJob;
        data[i].num = i;
        jobs[i].m_arg = &data[i];
    }

    // we can keep the counter on our stack because our stack will remain until our fiber completes execution
    kiwi::Counter counter(scheduler);

    scheduler->AddJob(jobs, 25, kiwi::JobPriority::Normal, &counter);
    // once jobs are added to the scheduler they're safe to delete, because the scheduler keeps it's own copy of them
    delete[] jobs;  

    scheduler->WaitForCounter(&counter);

    printf("WaitExampleJob - End\n");

    // once we know all the jobs have finished running it's safe for us to delete the data used by the jobs
    delete[] data;
}

int main(int /*argc*/, char** /*argv*/)
{
    kiwi::Scheduler scheduler;
    scheduler.Init();

    ExampleData jobData;
    jobData.secretNumber = 8001;
    kiwi::Job jobs[2];
    jobs[0] = { ExampleJob, &jobData };
    jobs[1] = { WaitExampleJob };
    scheduler.AddJob(jobs, 2);
    
    // we need to wait for all the jobs to finish running because they'll all be run on different threads
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    return 0;
}
