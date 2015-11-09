#include <stdio.h>
#include <stdlib.h>

typedef struct job_struct* job_t;
typedef struct trigger_struct* trigger_t;

trigger_t schedule_create_flagtrigger(flag_t);

int schedule_submit(job_t job, trigger_t trigger);
int schedule_cancel(job_t job);
int schedule_run(/*indicate if more, single, until idle, until done*/) {
    /* find a matching trigger */
    /* none found, raise idle flag */
}

void
idlejob(trigger_t satisfied)
{
    terminate=true;
}

void
idlejob(trigger_t satisfied)
{
    prevtime = schedule__time
    currenttime = schedule_gettime()
    select any flags type=time and prevtime<param.time<=currenttime
    activate those flags
    if there were no such flags
    select flags type=time param.time>currenttime && param.time=min()
    sle
}

/*
 

 
 */
