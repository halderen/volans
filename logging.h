struct log_cls_struct {
    const char* name;
};
        
typedef void* log_ctx_t;
typedef struct log_cls_struct log_cls_t;
typedef enum { log_FATAL, /*log_ALERT,*/ log_ERROR, log_WARN, /*log_NOTICE,*/ log_INFO, log_DEBUG, log_DIAG } log_lvl_t;

#define LOG_INITIALIZE(N) { N }


extern log_ctx_t log_noctx;
extern log_ctx_t log_ctx;
extern log_cls_t log_cls;

void log_initialize(char* programname);
log_ctx_t log_setnewcontext();
void log_setcontext(log_ctx_t);
void log_clearcontext();
log_ctx_t log_newcontext();
void log_destroycontext(log_ctx_t);

int log_enabled(log_cls_t cls, log_ctx_t ctx, log_lvl_t lvl);

void
log_message(log_cls_t cls, log_ctx_t ctx, log_lvl_t lvl, char* fmt,...)
#ifdef HAVE___ATTRIBUTE__
     __attribute__ ((format (printf, 4, 5)))
#endif
;
