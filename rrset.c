/*
 * Copyright (c) 2009 NLNet Labs. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * RRset.
 *
 */

#include "config.h"
#include "rrset.h"

#include "utilities.h"
#include "proto.h"

int b64_pton(char const *src, uint8_t *target, size_t targsize);

/**
 * Get the string-format of RRtype.
 *
 */
const char*
rrset_type2str(ldns_rr_type type)
{
    if (type == LDNS_RR_TYPE_IXFR) {
        return "IXFR";
    } else if (type == LDNS_RR_TYPE_AXFR) {
        return "AXFR";
    } else if (type == LDNS_RR_TYPE_MAILB) {
        return "MAILB";
    } else if (type == LDNS_RR_TYPE_MAILA) {
        return "MAILA";
    } else if (type == LDNS_RR_TYPE_ANY) {
        return "ANY";
    } else {
        const ldns_rr_descriptor* descriptor = ldns_rr_descript(type);
        if (descriptor && descriptor->_name) {
            return descriptor->_name;
        }
    }
    return "TYPE???";
}

/**
 * Print an LDNS RR, check status.
 *
 */
ods_status
util_rr_print(FILE* fd, const ldns_rr* rr)
{
    char* result = NULL;
    ldns_buffer* tmp_buffer = NULL;
    ods_status status = ODS_STATUS_OK;

    if (!fd || !rr) {
        return ODS_STATUS_ASSERT_ERR;
    }

    tmp_buffer = ldns_buffer_new(LDNS_MAX_PACKETLEN);
    if (!tmp_buffer) {
            return ODS_STATUS_MALLOC_ERR;
    }
    if (ldns_rr2buffer_str_fmt(tmp_buffer, NULL, rr)
                    == LDNS_STATUS_OK) {
            /* export and return string, destroy rest */
            result = ldns_buffer2str(tmp_buffer);
            if (result) {
                fprintf(fd, "%s", result);
                status = ODS_STATUS_OK;
                LDNS_FREE(result);
            } else {
                fprintf(fd, "; Unable to convert rr to string\n");
                status = ODS_STATUS_FWRITE_ERR;
            }
    } else {
            status = ODS_STATUS_FWRITE_ERR;
    }
    ldns_buffer_free(tmp_buffer);
    return status;
}


typedef struct item* resourcerecord_t;
struct itemsig {
    char* signature;
    char* keylocator;
    int keyflags;
};
struct itemset {
    char* itemname;
    int nitems;
    struct item* items;
    int nsignatures;
    struct itemsig* signatures;
};
struct dictionary_struct {
    char* name;
    int revision;
    int marker;
    char* spanhash;
    int nspansignatures;
    struct itemsig* spansignatures;
    int* validupto;
    int* validfrom;
    int* expiry;
    int nitemsets;
    struct itemset* itemsets;
    char* tmpRevision;
    char* tmpNameSerial;
    char* tmpValidFrom;
    char* tmpValidUpto;
    char* tmpExpiry;
};


/**
 * Add RR to RRset.
 *
 */
rr_type*
rrset_add_rr(rrset_type* rrset, ldns_rr* rr)
{
    rr_type* rrs_old = NULL;

    rrs_old = rrset->rrs;
    CHECKALLOC(rrset->rrs = (rr_type*) malloc((rrset->rr_count + 1) * sizeof(rr_type)));
    if (rrs_old) {
        memcpy(rrset->rrs, rrs_old, (rrset->rr_count) * sizeof(rr_type));
    }
    free(rrs_old);
    rrset->rr_count++;
    rrset->rrs[rrset->rr_count - 1].rr = rr;
    return &rrset->rrs[rrset->rr_count -1];
}


/**
 * Delete RR from RRset.
 *
 */
void
rrset_del_rr(rrset_type* rrset, uint16_t rrnum)
{
    rr_type* rrs_orig = NULL;

    ldns_rr_free(rrset->rrs[rrnum].rr);
    while (rrnum < rrset->rr_count-1) {
        rrset->rrs[rrnum] = rrset->rrs[rrnum+1];
        rrnum++;
    }
    memset(&rrset->rrs[rrset->rr_count-1], 0, sizeof(rr_type));
    rrs_orig = rrset->rrs;
    CHECKALLOC(rrset->rrs = (rr_type*) malloc((rrset->rr_count - 1) * sizeof(rr_type)));
    memcpy(rrset->rrs, rrs_orig, (rrset->rr_count -1) * sizeof(rr_type));
    free(rrs_orig);
    rrset->rr_count--;
}

/**
 * Remove signatures, deallocate storage and add then to the outgoing IFXR for that zone.
 *
 */
void
rrset_drop_rrsigs(rrset_type* rrset)
{
    int i;
    for(i=0; i<rrset->nrrsigs; i++) {
        rrset_destroyrrsig(&rrset->rrsigs[i]);
    }
    free(rrset->rrsigs);
    rrset->rrsigs = NULL;
    rrset->nrrsigs = 0;
}

/**
 * Add RRSIG to RRset.
 *
 */
void
rrset_add_rrsig(rrset_type* rrset, ldns_rr* rr,
    const char* locator, uint32_t flags)
{
    rrsig_type rrsig;
    rrsig.rr = rr;
    rrsig.key_locator = locator;
    rrsig.key_flags = flags;
    rrset->rrsigs = realloc(rrset->rrsigs, sizeof(rrsig_type)*rrset->nrrsigs);
    rrset->rrsigs[rrset->nrrsigs] = rrsig;
    rrset->nrrsigs += 1;
}

/**
 * Transmogrify the RRset to a RRlist.
 *
 */
static ldns_rr_list*
rrset2rrlist(rrset_type* rrset)
{
    ldns_rr_list* rr_list = NULL;
    int ret = 0;
    size_t i = 0;
    rr_list = ldns_rr_list_new();
    for (i=0; i < rrset->rr_count; i++) {
        ret = (int) ldns_rr_list_push_rr(rr_list, rrset->rrs[i].rr);
        if (!ret) {
            ldns_rr_list_free(rr_list);
            return NULL;
        }
        if (rrset->rrtype == LDNS_RR_TYPE_CNAME ||
            rrset->rrtype == LDNS_RR_TYPE_DNAME) {
            /* singleton types */
            return rr_list;
        }
    }
    ldns_rr_list_sort(rr_list);
    return rr_list;
}


ods_status
rrset_getliteralrr(ldns_rr** dnskey, const char *resourcerecord, uint32_t ttl, ldns_rdf* apex)
{
    uint8_t dnskeystring[4096];
    ldns_status ldnsstatus;
    int len;
    if ((len = b64_pton(resourcerecord, dnskeystring, sizeof (dnskeystring) - 2)) < 0) {
        return ODS_STATUS_PARSE_ERR;
    }
    dnskeystring[len] = '\0';
    if ((ldnsstatus = ldns_rr_new_frm_str(dnskey, (const char*) dnskeystring, ttl, apex, NULL)) != LDNS_STATUS_OK) {
        return ODS_STATUS_PARSE_ERR;
    }
    return ODS_STATUS_OK;
}
/**
 * Print RRset.
 *
 */
void
rrset_print(FILE* fd, rrset_type* rrset, int skip_rrsigs,
    ods_status* status)
{
    rrsig_type* rrsig;
    uint16_t i = 0;
    ods_status result = ODS_STATUS_OK;

    if (!rrset || !fd) {
        if (status) {
            *status = ODS_STATUS_ASSERT_ERR;
        }
    } else {
        for (i=0; i < rrset->rr_count; i++) {
                result = util_rr_print(fd, rrset->rrs[i].rr);
                if (rrset->rrtype == LDNS_RR_TYPE_CNAME ||
                    rrset->rrtype == LDNS_RR_TYPE_DNAME) {
                    /* singleton types */
                    break;
                }
        }
        if (! skip_rrsigs) {
            result = ODS_STATUS_OK;
            for(i=0; i<rrset->nrrsigs; i++) {
                rrsig = &rrset->rrsigs[i];
                if (result == ODS_STATUS_OK) {
                    result = util_rr_print(fd, rrsig->rr);
                }
            }
        }
        if (status) {
            *status = result;
        }
    }
}
