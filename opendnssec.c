#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#ifdef NOTDEFINED
#error "never define NOTDEFINED"
#endif

#ifdef __cplusplus
#include <cstdio>
#include <string>
#include <sstream>
#endif
#include <stdarg.h>

#if !defined(__GNUC__) || __GNUC__ < 2 || \
    (__GNUC__ == 2 && __GNUC_MINOR__ < 7) ||\
    defined(NEXT)
#ifndef __attribute__
#define __attribute__(__x)
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern void diagnostic_set(char *file, int line);
extern void diagnostic_print(char *fmt, ...)
     __attribute__ ((__format__ (__printf__, 1, 2)));
#ifdef __cplusplus
}
#endif

#define DIAG(LEVEL,ARG) do { if(DIAGLEVEL >= DIAG##LEVEL) { diagnostic_set(__FILE__,__LINE__); diagnostic_print ARG; } } while(0);
#define DIAGWARN 1
#define DIAGINFO 0
#ifndef DIAGLEVEL
#define DIAGLEVEL DIAGINFO
#endif

#ifdef DEBUG
# define BUG(ARG) ARG
#else
# define BUG(ARG)
#endif

#define CHECK(EX) do { if(EX) { int err = errno; fprintf(stderr, "operation" \
 " \"%s\" failed on line %d: %s (%d)\n", #EX, __LINE__, strerror(err), err); \
  abort(); }} while(0)

#endif

static struct option options[] = {
    { "help", no_argument, NULL, 'h' },
    { 0,0,0,0 }
};

static void
usage(char* argv0)
{
    fprintf(stderr,"usage: %s [-h]\n", argv0);
}

int
main(int argc, char** argv)
{
    char *argv0;
    int ch;
    int i;

    /* Get the name of the program */
    if((argv0 = strrchr(argv[0],'/')) == NULL)
        argv0 = argv[0];
    else
        ++argv0;

    while((ch = getopt_long(argc, argv, "h", options, NULL)) >= 0) {
        switch(ch) {
            case 'h':
                usage(argv[0]);
                break;
            case '?':
            default:
                usage(argv[0]);
                return EXIT_FAILURE;
        };
    }

    for(i=optind; i<argc; i++) {
        printf("%s\n",argv[i]);
    }
    
    return EXIT_SUCCESS;
}

/*
 * for program
 * run it in background
 * obtain its output
 * provide stdin
 * monitor it on exit
 * 
 */