#pragma once

#include <inttypes.h>

#include "kiwi/Counter.h"
#include "kiwi/JobPriority.h"
#include "kiwi/Job.h"

namespace kiwi
{
class SchedulerImpl;

class Scheduler
{
public:
    Scheduler();
    ~Scheduler();

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    // initializes memory and starts worker threads, this function should be called before calling 
    // any other functions on the Scheduler.
    // all the clean up for this function is handled in the decontructor 
    void Init();

    // add a job to the queue, once this function returns it's safe to delete the job object because a copy
    // of the job structure is saved. If a counter is provided it will be incremented for the job added
    void AddJobs(const Job* job, const JobPriority priority = JobPriority::Normal, Counter* counter = nullptr);

    // add multiple jobs to the queue, once this function returns it's safe to delete the job objects because a copy
    // of the job structures is saved. If a counter is provided it will be incremented for each job added
    void AddJobs(const Job* jobs, const uint32_t size, const JobPriority priority = JobPriority::Normal, Counter* counter = nullptr);

    // halt execution on current job until the counter reaches target value, if the counter is deleted before the target is reached
    // the job will be continued anyway.
    // THIS FUNCTION SHOULD ONLY BE CALLED FROM INSIDE JOBS
    void WaitForCounter(Counter* counter, int64_t value = 0);    

    // used internally
    SchedulerImpl* GetImpl() { return m_impl; }

private:
    SchedulerImpl* m_impl = nullptr;
};
}
