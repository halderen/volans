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

#pragma GCC optimize ("O0")

void
writezone(names_view_type view, const char* filename, const char* apex, int* defaultttl)
{
    char* s;
    const char* recordname;
    char* recordtype;
    struct item* item;
    char* recorddata;
    char* recordinfo;
    names_iterator domainiter;
    names_iterator rrsetiter;
    names_iterator rriter;
    dictionary domainitem;
    ldns_rdf* origin;
    FILE* fp;

    origin = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_DNAME, apex);
    fp = fopen(filename,"w");

    s = ldns_rdf2str(origin);
    fprintf(fp, "$ORIGIN %s\n", s);
    free(s);
    if(defaultttl)
        fprintf(fp, "$TTL %d\n",*defaultttl);
   
    for (domainiter = names_viewiterator(view, 0); names_iterate(&domainiter, &domainitem); names_advance(&domainiter, NULL)) {
        getset(domainitem, "name", &recordname, NULL);
        for (rrsetiter = names_recordalltypes(domainitem); names_iterate(&rrsetiter, &recordtype); names_advance(&rrsetiter, NULL)) {
            for (rriter = names_recordallvalues(domainitem,recordtype); names_iterate(&rriter, &item); names_advance(&rriter, NULL)) {
                recorddata = item->data;
                recordinfo = item->info;
                fprintf(fp, "%s\t%s\t%s\t%s\n", recordname, recordinfo, recordtype, recorddata);
            }
        }
    }
    fclose(fp);
    ldns_rdf_free(origin);
}
