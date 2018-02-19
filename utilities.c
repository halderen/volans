#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "utilities.h"

functioncast_t
functioncast(void*generic) {
    functioncast_t* function = (functioncast_t*)&generic;
    return *function;
}

#ifndef NOTDEFINED
unsigned long long int rnd(void)
{
  unsigned long long int foo;
  int cf_error_status;

  asm("rdrand %%rax; \
        mov $1,%%edx; \
        cmovae %%rax,%%rdx; \
        mov %%edx,%1; \
        mov %%rax, %0;":"=r"(foo),"=r"(cf_error_status)::"%rax","%rdx");
  return  (!cf_error_status ? 0 : foo);
}
#else
unsigned long long int rnd(void)
{
    uint64_t rand = 0;
    unsigned char ok;
    asm volatile ("rdrand %0; setc %1"
                : "=r" (rand), "=qm" (ok));
    return (ok ? rand : 0);
}
#endif

static int markcount = 0;
static int marktime;
static intptr_t markbrk;

int
mark(char* message)
{
#ifdef NOTDEFINED
    time_t t;
    intptr_t b;
    if(markcount == 0) {
        t = marktime = time(NULL);
        b = markbrk = (intptr_t) sbrk(0);
    } else {
        t = time(NULL);
        b = (intptr_t) sbrk(0);
    }
    fprintf(stderr, "MARK#%02d %2ld %4ld %s\n", markcount, t-marktime, (b-markbrk+1048576/2)/1048576, message);
    ++markcount;
#endif
    return 0;
}
