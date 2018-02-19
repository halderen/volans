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
handler_sign(struct evhttp_request *req, void *arg)
{
    struct evbuffer *buf;
    (void)arg;
    if((buf = evbuffer_new()) == NULL)
        return;

    evbuffer_add_printf(buf, "Requested: %s\n", evhttp_request_uri(req));
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
}

static void
handler_exit(struct evhttp_request *req, void *arg)
{
    struct evbuffer *buf;
    (void)arg;
    if((buf = evbuffer_new()) == NULL)
        return;

    evbuffer_add_printf(buf, "Requested: %s\n", evhttp_request_uri(req));
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    event_loopbreak();
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
    evhttp_set_cb(httpd, "/sign", handler_sign, NULL);
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
