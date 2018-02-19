#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <ldns/ldns.h>
#include <microhttpd.h>
#include <curl/curl.h>
#include "utilities.h"
#include "backend.h"
#include "cmdhandler.h"
#include "proto.h"
#include "httpd.h"

static struct option command_options[] = {
    { "help", no_argument, NULL, 'h'},
    { "config", no_argument, NULL, 'c'},
    { "no-daemon", no_argument, NULL, 'd'},
    { "daemon", no_argument, NULL, 'D'},
    { "verbose", no_argument, NULL, 'v'},
    { "version", no_argument, NULL, 'V'},
    { 0, 0, 0, 0}
};

char* argv0;

static void
usage(char* argv0path)
{
    (void)argv0path;
    fprintf(stderr, "usage: %s [-h]\n", argv0);
}

int
main(int argc, char** argv)
{
    int exitstatus = EXIT_SUCCESS;
    int ch;
    int optionServer;
    int optionDaemonize;
    struct http_listener_struct listenerconfig;
    struct httpd *httpd;
    char* url = NULL;

    /* Get the name of the program */
    if ((argv0 = strrchr(argv[0], '/')) == NULL)
        argv0 = argv[0];
    else
        ++argv0;

    if (!strcmp(argv0, "volans")) {
        optionServer = 0;
        optionDaemonize = 0;
    } else if (!strcmp(argv0, "volansd")) {
        optionServer = 1;
        optionDaemonize = 1;
    } else {
        optionServer = 0;
        optionDaemonize = 0;
    }
    while ((ch = getopt_long(argc, argv, "+hcdDvV", command_options, NULL)) >= 0) {
        switch (ch) {
            case 'h':
                usage(argv[0]);
                return EXIT_SUCCESS;
            case 'c':
                break;
            case 'd':
                optionServer = 1;
                optionDaemonize = 0;
                break;
            case 'D':
                optionServer = 1;
                optionDaemonize = 1;
                break;
            case 'v':
                break;
            case 'V':
                break;
            case '?':
                fprintf(stderr,"%s unrecognized command line option\n", argv0);
                return EXIT_FAILURE;
            default:
                usage(argv[0]);
                return EXIT_FAILURE;
        };
    }
    /*if (optind < argc) {
        fprintf(stderr, "%s: unrecognized command line arguments.\n", argv0);
    }*/
 
    if(optionServer) {
        if(optionDaemonize)
            daemonize();
        cmdhandler_initialize();
        listenerconfig.count = 0;
        listenerconfig.interfaces = NULL;
        /* IPv4 addesses needs be placed first */
        http_listener_push(&listenerconfig, "0.0.0.0", AF_INET, "8000", NULL, NULL);
        http_listener_push(&listenerconfig, "::0", AF_INET6, "8000", NULL, NULL);
        httpd = httpd_create(&listenerconfig);
        httpd_start(httpd);
        cmdhandler_run();
        httpd_stop(httpd);
        httpd_destroy(httpd);
        cmdhandler_finalize();
    } else {
        CURL *curl = curl_easy_init();
        CURLcode result;
        if(!strcmp(argv[1],"exit")) {
            curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/exit");
        } else if(!strcmp(argv[1],"zone")) {
            switch(argc) {
                case 0:
                case 1:
                case 2:
                    fprintf(stderr,"argument zone apex missing\n");
                    break;
                case 3:
                    asprintf(&url,"http://localhost:8080/zone?apex=%s",argv[2]);
                    break;
                case 4:
                    asprintf(&url,"http://localhost:8080/zone?apex=%s&persist=%s",argv[2],argv[3]);
                    break;
                case 5:
                    asprintf(&url,"http://localhost:8080/zone?apex=%s&persist=%s&input=%s",argv[2],argv[3],argv[4]);
                    break;
                default:
                    fprintf(stderr,"bad number of argument\n");
            }
        } else if(!strcmp(argv[1],"sign")) {
            switch(argc) {
                case 0:
                case 1:
                case 2:
                    fprintf(stderr,"argument new serial missing\n");
                    break;
                case 3:
                    asprintf(&url,"http://localhost:8080/sign?serial=%d",atoi(argv[2]));
                    break;
                case 4:
                    asprintf(&url,"http://localhost:8080/cycle?serial=%d&output=%s",atoi(argv[2]),argv[3]);
                    break;
                default:
                    fprintf(stderr,"bad number of argument\n");
            }
        } else if(!strcmp(argv[1],"cycle")) {
            switch(argc) {
                case 0:
                case 1:
                case 2:
                    asprintf(&url,"http://localhost:8080/cycle");
                    break;
                case 3:
                    asprintf(&url,"http://localhost:8080/cycle?output=%s",argv[2]);
                    break;
                default:
                    fprintf(stderr,"bad number of argument\n");
            }
        } else if(!strcmp(argv[1],"output")) {
            switch(argc) {
                case 0:
                case 1:
                case 2:
                    fprintf(stderr,"argument output missing\n");
                    break;
                case 3:
                    asprintf(&url,"http://localhost:8080/output?output=%s",argv[2]);
                    break;
                default:
                    fprintf(stderr,"bad number of argument\n");
            }
        } else if(!strcmp(argv[1],"persist")) {
            switch(argc) {
                case 0:
                case 1:
                case 2:
                    asprintf(&url,"http://localhost:8080/persist");
                    break;
                default:
                    fprintf(stderr,"bad number of argument\n");
            }
        } else {
            fprintf(stderr,"unknown argument\n");
        }
        if(url)
            curl_easy_setopt(curl, CURLOPT_URL, url);
        result = curl_easy_perform(curl);
        if(result != CURLE_OK) {
            fprintf(stderr,"failed\n");
            exitstatus = EXIT_FAILURE;
        }
        curl_easy_cleanup(curl);
    }
    return exitstatus;
}
