#include <iostream>

#include <unistd.h>

#include "kiwi/Job.h"
#include "kiwi/Scheduler.h"

void EndApplicationJob(kiwi::Scheduler* scheduler, void* arg)
{
    // convert the arg into a bool so we can change the value
    bool& quit = *reinterpret_cast<bool*>(arg); 

    printf("EndApplicationJob - Started\n");

    printf("EndApplicationJob - Completed\n");

    // we're done running our job, change to true so the application will end
    quit = true;
}

int main(int /*argc*/, char** /*argv*/)
{
    kiwi::Scheduler scheduler;
    scheduler.Init();

    bool quit = false;

    kiwi::Job* job = new kiwi::Job();

    // entry point function for the job, it's expected to take the Scheduler and one argument
    job->m_function = EndApplicationJob;

    // pass pointer to quit variable so we can change it true when our job completes
    // if a pointer is used by a job that pointer needs to remain valid until the job is completed, quit exists
    // on the stack so the memory will be valid.
    job->m_arg = &quit;

    // add job to the scheduler as a normal priority, if it's an important job we can run it as high priority
    // or we can use low if it's minor task not required by the main loop
    scheduler.AddJob(job, kiwi::JobPriority::Normal);

    // once the job has been added to the scheduler it's save to delete, because a copy is kept
    // remember that any pointers, that this job may have access to needs to be valid until the job
    // completes execution
    delete job;

    // loop on quit until it's value is changed inside of EndApplicationJob
    while (!quit)
    {
        sleep(1);
    }

    return 0;
}
