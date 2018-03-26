#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>
#include <pthread.h>
#include "logging.h"

#undef logger_message

typedef logger_result_type (*logger_procedure)(const logger_cls_type*, const logger_ctx_type, const logger_lvl_type, const char*, va_list ap);


struct logger_chain_struct {
    char* name;
    logger_procedure logger;

};

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
struct logger_setup_struct logger_setup;
static logger_cls_type logger = LOGGER_INITIALIZE(__FILE__);

logger_ctx_type logger_noctx = NULL;
logger_ctx_type logger_ctx;
logger_cls_type logger_cls = LOGGER_INITIALIZE("");

int
logger_enabled(logger_cls_type* cls, logger_ctx_type ctx, logger_lvl_type lvl)
{
    (void)ctx;
    if(cls->setupserial != logger_setup.serial)
        logger_resetup(cls);
    if(lvl <= cls->minlvl)
        return 1;
    else
        return 0;
}

void
logger_message(logger_cls_type* cls, logger_ctx_type ctx, logger_lvl_type lvl, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if(!logger_enabled(cls, ctx, lvl))
        return;
    logger_messageinternal(cls,ctx,lvl,fmt,ap);
    va_end(ap);
}

void
logger_vmessage(logger_cls_type* cls, logger_ctx_type ctx, logger_lvl_type lvl, const char* fmt, va_list ap)
{
    if(!logger_enabled(cls, ctx, lvl))
        return;
    logger_messageinternal(cls,ctx,lvl,fmt,ap);
}

void
logger_messageinternal(logger_cls_type* cls, logger_ctx_type ctx, logger_lvl_type lvl, const char* fmt, ...)
{
    va_list ap;
    if(!logger_enabled(cls, ctx, lvl))
        return;
    va_start(ap,fmt);
    vfprintf(stderr,fmt,ap);
    va_end(ap);
}

static pthread_key_t currentctx;

struct logger_ctx_struct {
    struct logger_ctx_struct* prev;
    const char* label;
};

const char*
logger_getcontext(logger_ctx_type ctx)
{
    if(ctx == logger_ctx) {
        ctx = pthread_getspecific(currentctx);
    }
    if(ctx != logger_noctx) {
        return ctx->label;
    } else {
        return NULL;
    }
}

logger_result_type
logger_log_syslog(const logger_cls_type* cls, const logger_ctx_type ctx, const logger_lvl_type lvl, const char* format, va_list ap)
{
    int priority;
    (void)cls;
    (void)ctx;
    switch(lvl) {
        case logger_FATAL:  priority = LOG_ALERT;    break;
        case logger_ERROR:  priority = LOG_ERR;      break;
        case logger_WARN:   priority = LOG_WARNING;  break;
        case logger_INFO:   priority = LOG_NOTICE;   break;
        case logger_DEBUG:  priority = LOG_INFO;     break;
        case logger_DIAG:   priority = LOG_DEBUG;    break;
        default:
            priority = LOG_ERR;
    }
    vsyslog(priority, format, ap);
    return logger_CONT;
}

logger_result_type
logger_log_stderr(const logger_cls_type* cls, const logger_ctx_type ctx, const logger_lvl_type lvl, const char* format, va_list ap)
{
    const char* priority;
    char* message;
    const char* location;
    const char* context;
    switch(lvl) {
        case logger_FATAL:  priority = "fatal error: ";  break;
        case logger_ERROR:  priority = "error: ";        break;
        case logger_WARN:   priority = "warning: ";      break;
        case logger_INFO:   priority = "";               break;
        case logger_DEBUG:  priority = "";               break;
        case logger_DIAG:   priority = "";               break;
        default:
            priority = "unknown problem: ";
    }
    context = logger_getcontext(ctx);
    location = cls->name;
    vasprintf(&message, format, ap);
    fprintf(stderr,"%s%s%s%s%s%s%s%s\n",priority,(location?"[":""),(location?location:""),(location?"] ":""), message, (context?" (":""), (context), (context?")":""));
    fprintf(stderr,"%s",message);
    return logger_CONT;
}

void
logger_log_syslog_open(const char* argv0)
{
    openlog(argv0, LOG_NDELAY, LOG_DAEMON);
}

void
logger_log_syslog_close(void)
{
    closelog();
}

static void
destroyctx(void* arg)
{
    logger_ctx_type ctx = (logger_ctx_type)arg;
    logger_destroycontext(ctx);
}

void
logger_initialize(const char* programname)
{
    logger_setup.serial = 2;
    logger_setup.nchains = 1;
    logger_setup.chains = malloc(sizeof(struct logger_chain_struct) * logger_setup.nchains);
    logger_setup.chains[0].name = "";
    logger_setup.chains[0].logger = logger_log_stderr;
    logger_log_syslog_open(programname);
    //logger_setup.chains[0].logger = logger_log_syslog;
    logger_message(&logger, logger_noctx, logger_INFO, "%s started",programname);
    pthread_key_create(&currentctx, destroyctx);
    logger_ctx = logger_newcontext();
}

void
logger_finalize(void)
{
    pthread_key_delete(currentctx);
}

void
logger_resetup(logger_cls_type* cls)
{
    pthread_mutex_lock(&mutex);
    if(cls->setupserial == 1) {
        logger_initialize(NULL);
    }
    cls->setupserial = logger_setup.serial;
    cls->minlvl = logger_WARN;
    cls->chain = &logger_setup.chains[0];
    pthread_mutex_unlock(&mutex);
}

logger_ctx_type
logger_newcontext(void)
{
    logger_ctx_type ctx;
    ctx = malloc(sizeof(struct logger_ctx_struct));
    ctx->prev = NULL;
    ctx->label = NULL;
    return ctx;
}

void
logger_destroycontext(logger_ctx_type ctx)
{
    free((void*)ctx->label);
    free((void*)ctx);
}

void
logger_setcontext(logger_ctx_type ctx)
{
    pthread_setspecific(currentctx, ctx);
}

void
logger_pushcontext(logger_ctx_type ctx)
{
    logger_ctx_type prev;
    if(ctx == logger_noctx) {
        ctx = logger_newcontext();
    } else if (ctx == logger_ctx) {
        ctx = logger_newcontext();
        ctx->label = (ctx->label ? strdup(ctx->label) : NULL);
    }
    prev = pthread_getspecific(currentctx);
    ctx->prev = prev;
    pthread_setspecific(currentctx, ctx);
}

void
logger_popcontext(void)
{
    logger_ctx_type prev, ctx;
    ctx = pthread_getspecific(currentctx);
    prev = ctx->prev;
    logger_destroycontext(ctx);
    pthread_setspecific(currentctx, prev);
}

void
logger_clearcontext(void)
{
    logger_ctx_type prev, ctx;
    for(ctx=pthread_getspecific(currentctx); ctx; ctx=prev) {
        prev = ctx->prev;
        logger_destroycontext(ctx);
    }
    pthread_setspecific(currentctx, NULL);
}

void
logger_putcontext(logger_ctx_type ctx, const char* key, const char* value)
{
    char* newdesc;
    if(ctx->label) {
        if(key) {
            asprintf(&newdesc,"%s,%s=%s",ctx->label,key,value);
        } else {
            asprintf(&newdesc,"%s,%s",ctx->label,value);
        }
    } else {
        if(key) {
            asprintf(&newdesc,"%s=%s",key,value);
        } else {
            asprintf(&newdesc,"%s",value);
        }
    }
    free((void*)ctx->label);
    ctx->label = newdesc;
}

#ifndef DEPRECATE

#ifdef HAVE_SYSLOG_H
#include <strings.h> /* strncasecmp() */
#include <syslog.h> /* openlog(), closelog(), syslog() */
#else /* !HAVE_SYSLOG_H */
#define LOG_EMERG   0 /* ods_fatal_exit */
#define LOG_ALERT   1 /* ods_log_alert */
#define LOG_CRIT    2 /* ods_log_crit */
#define LOG_ERR     3 /* ods_log_error */
#define LOG_WARNING 4 /* ods_log_warning */
#define LOG_NOTICE  5 /* ods_log_info */
#define LOG_INFO    6 /* ods_log_verbose */
#define LOG_DEBUG   7 /* ods_log_debug */
#endif /* HAVE_SYSLOG_H */
#define LOG_DEEEBUG 8 /* ods_log_deeebug */

void
ods_log_deeebug(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    logger_vmessage(&logger_cls, logger_noctx, logger_TRACE, format, ap);
    va_end(ap);
}

void
ods_log_debug(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    logger_vmessage(&logger_cls, logger_noctx, logger_DEBUG, format, ap);
    va_end(ap);
}

void
ods_log_verbose(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    logger_vmessage(&logger_cls, logger_noctx, logger_DEBUG, format, ap);
    va_end(ap);
}

void
ods_log_info(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    logger_vmessage(&logger_cls, logger_noctx, logger_INFO, format, ap);
    va_end(ap);
}

void
ods_log_warning(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    logger_vmessage(&logger_cls, logger_noctx, logger_WARN, format, ap);
    va_end(ap);
}

void
ods_log_error(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    logger_vmessage(&logger_cls, logger_noctx, logger_ERROR, format, ap);
    va_end(ap);
}

void
ods_log_verror(const char *format, va_list ap)
{
    logger_vmessage(&logger_cls, logger_noctx, logger_ERROR, format, ap);
}

void
ods_log_crit(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    logger_vmessage(&logger_cls, logger_noctx, logger_FATAL, format, ap);
    va_end(ap);
}

void
ods_log_alert(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    logger_vmessage(&logger_cls, logger_noctx, logger_ERROR, format, ap);
    va_end(ap);
}

void
ods_fatal_exit(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    logger_vmessage(&logger_cls, logger_noctx, logger_FATAL, format, ap);
    va_end(ap);
    abort();
}

#endif
