#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

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
