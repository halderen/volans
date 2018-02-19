#define _LARGEFILE64_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ldns/ldns.h>
#include "uthash.h"
#include "proto.h"
#include "utilities.h"

#pragma GCC optimize ("O0")

struct names_struct*
names_docreate(char* name)
{
    const char* baseviewkeys[] = { "namerevision", NULL};
    const char* inputviewkeys[] = { "nameupcoming", "namehierarchy", NULL};
    const char* prepareviewkeys[] = { "namerevision", "namenoserial", "namenewserial", NULL};
    const char* signviewkeys[] = { "nameready", "expiry", "denialname", NULL};
    const char* outputviewkeys[] = { "validnow", NULL};
    int status;
    struct names_struct* names;

    names = malloc(sizeof(struct names_struct));
    names->basefd = open(".", O_PATH, 07777);
    names->source = strdup(name);
    names->apex = strdup(name);

    names->baseview = names_viewcreate(NULL, "  base    ", baseviewkeys);
    mark("created base view");

    status = names_viewrestore(names->baseview, names->apex, names->basefd, "storage");
    mark("restored base view");

    names->inputview = names_viewcreate(names->baseview,   "  input   ", inputviewkeys);
    names->prepareview = names_viewcreate(names->baseview, "  prepare ", prepareviewkeys);
    names->signview = names_viewcreate(names->baseview,    "  sign    ", signviewkeys);
    names->outputview = names_viewcreate(names->baseview,  "  output  ", outputviewkeys);
    mark("created other view");


    if(status) {
        names->apex = NULL;
        readzone(names->inputview, PLAIN, names->source, &names->apex, NULL);
        mark("read zone");
        if (names_viewcommit(names->inputview)) {
            abort();
        }
        mark("commit read");
    }
    return names;
}

void
names_dodestroy(struct names_struct* names)
{
    names_viewreset(names->baseview);
    mark("base updated");
    names_viewpersist(names->baseview, names->basefd, "storage");
    mark("base persisted");

    names_viewdestroy(names->inputview);
    names_viewdestroy(names->prepareview);
    names_viewdestroy(names->signview);
    names_viewdestroy(names->outputview);
    names_viewdestroy(names->baseview);

    close(names->basefd);
}

void
names_dofull(struct names_struct* names, int serial)
{
    names_viewreset(names->prepareview);
    prepare(names->prepareview, serial);
    if (names_viewcommit(names->prepareview)) {
        abort();
    }
    mark("prepared");
    mark("signing");
    names_viewreset(names->signview);
    mark("sign updated");
    sign(names->signview);
    mark("sign signed");
    if (names_viewcommit(names->signview)) {
        abort();
    }
    mark("sign committed");
    names_viewreset(names->outputview);
    mark("persist view");
    writezone(names->outputview, "example.2", names->apex, NULL);
    names_viewpersist(names->baseview, names->basefd, "storage");
}

void
names_docycle(struct names_struct* names, int serial)
{
    names_viewreset(names->inputview); /* FIXME not having this causes a conflict which is not necessary */
    readzone(names->inputview, DELTAMINUS, "example.delta", &names->apex, NULL);
    mark("read delta");
    if (names_viewcommit(names->inputview)) {
        abort();
    }
    mark("commit delta");
    names_viewreset(names->prepareview);
    prepare(names->prepareview, serial);
    if (names_viewcommit(names->prepareview)) {
        abort();
    }    
    mark("signing");
    names_viewreset(names->signview);
    mark("sign updated");
    sign(names->signview);
    mark("sign signed");
    if (names_viewcommit(names->signview)) {
        abort();
    }
    mark("sign committed");
    names_viewreset(names->outputview);

    mark("persist view");
    writezone(names->outputview, "example.2", names->apex, NULL);
}
