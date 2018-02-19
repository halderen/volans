#ifndef BACKEND_H
#define BACKEND_H

#ifdef __cplusplus
extern "C" {
#endif

struct connection_struct;
typedef struct connection_struct* connection_type;
struct session_struct;
typedef struct session_struct* session_type;

struct backend {
    connection_type connection;
    int (*opensession)(connection_type, session_type* resultSession);

    /** register count number of changes */
    int (*registerchange)(session_type, int count, char**, char**);
    int (*purgerevision)(session_type, long revision);
    int (*newrevision)(session_type, long* revision);
    int (*changeset)(session_type, int maxcount, int *actualcount, const char**);
    int (*getzoneinfo)(session_type, const char**);
    int (*closesession)(session_type);
    int (*test)(session_type, long what);
 };

struct backend sqliteRegister(void);
struct backend mysqlRegister(void);

#ifdef __cplusplus
}
#endif

#endif
