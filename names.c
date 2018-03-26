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

const char* baseviewkeys[] = { "namerevision", NULL};
const char* inputviewkeys[] = { "nameupcoming", "namehierarchy", NULL};
const char* prepareviewkeys[] = { "namerevision", "namenoserial", "namenewserial", NULL};
const char* signviewkeys[] = { "nameready", "expiry", "denialname", NULL};
const char* outputviewkeys[] = { "validnow", NULL};

int
names_docreate(struct names_struct** zoneptr, const char* apex, const char* persist, const char* input)
{
    struct names_struct* zone = *zoneptr;
    int status = 0;

    if(!*zoneptr) {
        zone = malloc(sizeof(struct names_struct));
        zone->basefd = open(".", O_PATH, 07777);
        zone->apex = strdup(apex);
        zone->source = NULL;
        zone->persist = strdup(persist);
    }
    if (input) {
        zone->source = strdup(input);
    }

    if(!*zoneptr) {
        zone->baseview = names_viewcreate(NULL, "  base    ", baseviewkeys);
        status = names_viewrestore(zone->baseview, zone->apex, zone->basefd, persist);
        zone->inputview = names_viewcreate(zone->baseview,   "  input   ", inputviewkeys);
        zone->prepareview = names_viewcreate(zone->baseview, "  prepare ", prepareviewkeys);
        zone->signview = names_viewcreate(zone->baseview,    "  sign    ", signviewkeys);
        zone->outputview = names_viewcreate(zone->baseview,  "  output  ", outputviewkeys);
    }

    if(status && zone->source != NULL) {
        if(zone->apex != NULL) {
            free(zone->apex);
            zone->apex = NULL;
        }
        readzone(zone->inputview, PLAIN, zone->source, &zone->apex, NULL);
        if (names_viewcommit(zone->inputview)) {
            return -1;
        }
        if(zone->apex == NULL) {
            zone->apex = strdup(apex);
        }
    }
  
    *zoneptr = zone;
    return 0;
}

void
names_dodestroy(struct names_struct* names)
{
    names_viewreset(names->baseview);
    names_viewpersist(names->baseview, names->basefd, names->persist);

    names_viewdestroy(names->inputview);
    names_viewdestroy(names->prepareview);
    names_viewdestroy(names->signview);
    names_viewdestroy(names->outputview);
    names_viewdestroy(names->baseview);

    close(names->basefd);
}

void
names_docycle(struct names_struct* names, int* serial, const char* filename)
{
    if(serial) {
        names_viewreset(names->prepareview);
        prepare(names->prepareview, *serial);
        if (names_viewcommit(names->prepareview)) {
            abort(); // FIXME
        }
        names_viewreset(names->signview);
        sign(names->signview, names->apex);
        if (names_viewcommit(names->signview)) {
            abort(); // FIXME
        }
    }
    if(filename) {
        names_viewreset(names->outputview);
        writezone(names->outputview, filename, names->apex, NULL);
    }
}

void
names_dopersist(struct names_struct* names)
{
    names_viewreset(names->baseview);
    names_viewpersist(names->baseview, names->basefd, names->persist);
}
