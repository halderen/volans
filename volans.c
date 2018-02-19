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
    int ch;
    int optionServer;
    int optionDaemonize;
    struct http_listener_struct listenerconfig;
    struct httpd *httpd;

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
    if (optind < argc) {
        fprintf(stderr, "%s: unrecognized command line arguments.\n", argv0);
    }
 
    if(optionServer) {
        mark("cmdhandler_initialize");
        cmdhandler_initialize();
        mark("httpd_create");
        listenerconfig.count = 0;
        listenerconfig.interfaces = NULL;
        /* IPv4 addesses needs be placed first */
        http_listener_push(&listenerconfig, "0.0.0.0", AF_INET, "8000", NULL, NULL);
        http_listener_push(&listenerconfig, "::0", AF_INET6, "8000", NULL, NULL);
        httpd = httpd_create(&listenerconfig);
        mark("httpd_start");
        httpd_start(httpd);
        mark("cmdhandler_run");
        cmdhandler_run();
        mark("httpd_stop");
        httpd_stop(httpd);
        mark("httpd_destroy");
        httpd_destroy(httpd);
        mark("cmdhandler_finalize");
        cmdhandler_finalize();
    } else {
        mark("names_docreate");
        struct names_struct* names = names_docreate("example");
        mark("names_dofull");
        names_dofull(names, 2017101705);
        mark("names_docycle");
        names_docycle(names, 2017101706);
        mark("names_dodestroy");
        names_dodestroy(names);
    }
    return EXIT_SUCCESS;
}

void submittask();
void ensuretask();
void performtask();
void looptasks();
void addzone(struct names_struct* zone);
struct names_struct* getzone(char* apex)
{
#ifdef NOTDEFINED
    zone_type *zone = zonelist_lookup_zone_by_name(httpd* engine->zonelist, rpc->zone,
        LDNS_RR_CLASS_IN);
    if (!zone) {
        rpc->status = RPC_RESOURCE_NOT_FOUND;
        return 0;
    }
#endif
}

#include <curl/curl.h>

docall()
{
    CURLcode result;
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/exit");
    result = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}
