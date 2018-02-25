#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <pthread.h>
#include "logging.h"
#include "daemonize.h"

log_cls_t logger = LOG_INITIALIZE(__FILE__);

log_ctx_t log_noctx = NULL;
log_ctx_t log_ctx = NULL;
log_cls_t log_cls = { "" };

int
log_enabled(log_cls_t cls, log_ctx_t ctx, log_lvl_t lvl)
{
    (void)cls;
    (void)ctx;
    (void)lvl;
    return 1;
}

void
log_message(log_cls_t cls, log_ctx_t ctx, log_lvl_t lvl, char* fmt, ...)
{
    va_list ap;
    if(!log_enabled(cls, ctx, lvl))
        return;
    va_start(ap, fmt);
    vfprintf(stderr,fmt,ap);
    va_end(ap);
}

void
mycode(void)
{
    if(log_enabled(logger,log_ctx,log_ERROR)) {
        log_message(log_cls, log_ctx, log_ERROR, "hello %s", "world");
    }
}



/* To use:
 * initialize();
 * drop();
 * daemonize();
 */
__attribute__((__format__(__printf__, 2, 3)))
void
logx(void* class, char *message, ...)
{
    syslog(LOG_ERR, message);
}

void
initialize()
{
    openlog(argv0, LOG_NDELAY, LOG_DAEMON);
}

void
finalize() {
    closelog();
}
