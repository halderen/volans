#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <stdlib.h>
#include <err.h>
#include <event.h>
#include <evhttp.h>
#include <pthread.h>
#include <ldns/ldns.h>
#include "proto.h"

static struct names_struct* zone = NULL;
static int serial;

struct names_struct* getzone(char* apex)
{
#ifdef NOTDEFINED
    zone_type *zone = zonelist_lookup_zone_by_name(httpd* engine->zonelist, rpc->zone,
        LDNS_RR_CLASS_IN);
    if (!zone) {
        rpc->status = RPC_RESOURCE_NOT_FOUND;
        return 0;
    }
#else
    (void)apex;
    return zone;
#endif
}

static void
handler(struct evhttp_request *req, void *arg)
{
    struct evbuffer *buf;
    (void)arg;
    if((buf = evbuffer_new()) == NULL)
        return;
    evbuffer_add_printf(buf, "Requested: %s\n", evhttp_request_uri(req));
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
}

static void
handler_zone(struct evhttp_request *req, void *arg)
{
    struct evbuffer *buf;
    struct evkeyvalq params;
    const char* apex;
    const char* persist;
    const char* input;
    int status;
    (void)arg;

    if((buf = evbuffer_new()) == NULL)
        return;

    evhttp_parse_query(evhttp_request_get_uri(req), &params);
    apex = evhttp_find_header(&params, "apex");
    persist = evhttp_find_header(&params, "persist");
    input = evhttp_find_header(&params, "input");
    status = names_docreate(&zone, apex, persist, input);
    serial = 1;

    if(status) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Failed", buf);
    } else {
        evhttp_send_reply(req, HTTP_OK, "OK", buf);
    }
}

static void
handler_sign(struct evhttp_request *req, void *arg)
{
    struct evbuffer *buf;
    struct evkeyvalq params;
    const char* newserial;
    (void)arg;

    if((buf = evbuffer_new()) == NULL)
        return;

    evhttp_parse_query(evhttp_request_get_uri(req), &params);
    newserial = evhttp_find_header(&params, "newserial");
    if(newserial) {
        serial = atoi(newserial);
    }

    names_docycle(zone, &serial, NULL);

    evhttp_send_reply(req, HTTP_OK, "OK", buf);
}

static void
handler_output(struct evhttp_request *req, void *arg)
{
    struct evbuffer *buf;
    struct evkeyvalq params;
    const char* output;
    (void)arg;

    if((buf = evbuffer_new()) == NULL)
        return;

    evhttp_parse_query(evhttp_request_get_uri(req), &params);
    output = evhttp_find_header(&params, "output");
    names_docycle(zone, NULL, output);

    evhttp_send_reply(req, HTTP_OK, "OK", buf);
}

static void
handler_cycle(struct evhttp_request *req, void *arg)
{
    struct evbuffer *buf;
    struct evkeyvalq params;
    const char* output;
    const char* newserial;
    (void)arg;

    if((buf = evbuffer_new()) == NULL)
        return;

    evhttp_parse_query(evhttp_request_get_uri(req), &params);
    output = evhttp_find_header(&params, "output");
    newserial = evhttp_find_header(&params, "newserial");
    if(newserial) {
        serial = atoi(newserial);
    }
    names_docycle(zone, &serial, output);

    evhttp_send_reply(req, HTTP_OK, "OK", buf);
}

static void
handler_persist(struct evhttp_request *req, void *arg)
{
    struct evbuffer *buf;
    (void)arg;

    if((buf = evbuffer_new()) == NULL)
        return;

    names_dopersist(zone);

    evhttp_send_reply(req, HTTP_OK, "OK", buf);
}

static void
handler_exit(struct evhttp_request *req, void *arg)
{
    struct evbuffer *buf;
    (void)arg;
    if((buf = evbuffer_new()) == NULL)
        return;

    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    event_loopexit(NULL);
}

static struct evhttp* httpd;

void
cmdhandler_initialize(void)
{
    event_init();
}

void
cmdhandler_run(void)
{
    httpd = evhttp_start("0.0.0.0", 8080);
    evhttp_set_cb(httpd, "/exit", handler_exit, NULL);
    evhttp_set_cb(httpd, "/zone", handler_zone, NULL);
    evhttp_set_cb(httpd, "/sign", handler_sign, NULL);
    evhttp_set_cb(httpd, "/cycle", handler_cycle, NULL);
    evhttp_set_cb(httpd, "/output", handler_output, NULL);
    evhttp_set_cb(httpd, "/persist", handler_persist, NULL);
    evhttp_set_gencb(httpd, handler, NULL);
    event_dispatch();
}
pthread_barrier_t barrier;
void*
routine(void *argument)
{
    /* initialization */
    pthread_barrier_wait(&barrier);
    cmdhandler_run();
    return NULL;
}
void
cmdhandler_run2(void)
{
    pthread_t thread;
    pthread_attr_t attr;
    pthread_barrier_init(&barrier, NULL, 2);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 1);
    pthread_create(&thread, &attr, routine, NULL);
    pthread_attr_destroy(&attr);
    pthread_barrier_wait(&barrier);
    pthread_barrier_destroy(&barrier);
}

void
cmdhandler_finalize(void)
{
    evhttp_free(httpd);
}
