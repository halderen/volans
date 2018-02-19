#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef HAVE_SQLITE3
#include <sqlite3.h>
#endif
#include <dlfcn.h>
#include "utilities.h"
#include "backend.h"
#ifdef HAVE_SQLITE3
#include "schema_sqlite.h"
#endif

#ifdef HAVE_SQLITE3

struct connection_struct {
    void* dlhandle;
    int (*open)(const char *filename, sqlite3 **ppDb);
    int (*close)(sqlite3*);
    int (*free)(void*);
    const char* (*errmsg)(sqlite3*);
    int (*errcode)(sqlite3*);
    int (*exec)(sqlite3*,const char*,int (*)(void*,int,char**,char**),void*,char**);
    int (*prepare)(sqlite3*,const char*,int,sqlite3_stmt**,const char**);
    int (*step)(sqlite3_stmt*);
    int (*reset)(sqlite3_stmt *);
    int (*bind_int)(sqlite3_stmt*, int, int);
    int (*bind_text)(sqlite3_stmt*,int,const char*,int,void(*)(void*));
    const unsigned char *(*column_text)(sqlite3_stmt*, int iCol);
    int (*column_int)(sqlite3_stmt*, int iCol);
    int (*changes)(sqlite3*);
    int (*finalize)(sqlite3_stmt *);
};

struct session_struct {
    connection_type connection;
    sqlite3* handle;
    int returnCode;
    int errorCode;
    char* errorMessage;
    sqlite3_stmt* getpropStmt;
    sqlite3_stmt* insertStmt;
    sqlite3_stmt* maximumStmt;
    sqlite3_stmt* forwardStmt;
    sqlite3_stmt* purgeStmt;
    sqlite3_stmt* beginStmt;
    sqlite3_stmt* commitStmt;
    sqlite3_stmt* testStmt;
};

static const char* getpropStatementString = "SELECT propertyValue FROM properties WHERE propertyName = 'version'";
static const char* insertStatementString = "REPLACE INTO rrsets ( dname, dcontent ) VALUES ( ?, ? )";
static const char* maximumStatementString = "SELECT MAX(revision) FROM rrsets";
static const char* forwardStatementString = "UPDATE rrsets SET revision = ? WHERE revision = 0";
static const char* beginStatementString = "BEGIN TRANSACTION";
static const char* commitStatementString = "COMMIT";
/*static const char* purgeStatementString = "DELETE FROM rrsets WHERE ROWID IN ( WITH cte AS ( SELECT MAX(revision) AS m, dname AS n FROM rrsets GROUP BY dname ) SELECT rrsets.ROWID FROM rrsets INNER JOIN cte ON cte.n = dname WHERE revision < cte.m AND revision < ? AND revision > 0 LIMIT 10000 )";*/
static const char* purgeStatementString   = "DELETE FROM rrsets WHERE ROWID IN ( SELECT rrsets.ROWID FROM rrsets LEFT JOIN currrsets ON rrsets.dname = currrsets.dname WHERE rrsets.revision < currrsets.revision AND rrsets.revision < ? AND rrsets.revision > 0 LIMIT 10000 )";
static const char* testStatementString = "WITH cte AS ( SELECT MAX(revision) AS m, dname AS n FROM rrsets GROUP BY dname ) SELECT COUNT(*) FROM rrsets INNER JOIN cte ON cte.n = dname WHERE revision < cte.m AND revision < ? AND revision > 0";

static int
isuptodate(session_type session)
{
    int status;
    if(session->getpropStmt == NULL)
        return 0;
    if((status = session->connection->reset(session->getpropStmt)))
        return 0;
    do {
        switch ((status = session->connection->step(session->getpropStmt))) {
            case SQLITE_ROW:
                session->connection->column_text(session->getpropStmt, 0);
                return 1;
            case SQLITE_DONE:
                return 0;
            case SQLITE_BUSY:
                sleep(1);
                break;
            case SQLITE_ERROR:
            case SQLITE_MISUSE:
                fprintf(stderr, "sql %d error: %s (%d)\n",status,session->connection->errmsg(session->handle),session->connection->errcode(session->handle));
                return 0;
            default:
                break;
        }
    } while(status == SQLITE_BUSY);
    return 0;
}

static void eatResult(session_type session, sqlite3_stmt* stmt)
{
    int status;
    do {
        switch ((status = session->connection->step(stmt))) {
            case SQLITE_ROW:
                break;
            case SQLITE_DONE:
                return;
            case SQLITE_BUSY:
                sleep(1);
                break;
            case SQLITE_ERROR:
                fprintf(stderr, "sql1 %d error: %s (%d)\n",status,session->connection->errmsg(session->handle),session->connection->errcode(session->handle));
                return;
            case SQLITE_MISUSE:
                fprintf(stderr, "sql2 misuse: %s (%d)\n",session->connection->errmsg(session->handle),session->connection->errcode(session->handle));
                return;
            default:
                break;
        }
    } while(status == SQLITE_BUSY || status == SQLITE_ROW);
}

static int gobbleResult(void* a, int b, char**c, char**d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return 0;
}

static int
registerchange(session_type session, int count, char** names, char** values)
{
    int i;
    session->connection->reset(session->beginStmt);
    eatResult(session, session->beginStmt);
    for(i=0; i<count; i++) {
        session->connection->reset(session->insertStmt);
        session->connection->bind_text(session->insertStmt, 1, names[i], -1, SQLITE_STATIC);
        session->connection->bind_text(session->insertStmt, 2, values[i], -1, SQLITE_STATIC);
        eatResult(session, session->insertStmt);
    }
    session->connection->reset(session->commitStmt);
    eatResult(session, session->commitStmt);
    return 0;
}

static int getInteger(session_type session, sqlite3_stmt* stmt, int* valuePtr)
{
    int status;
    int value;
    do {
        switch ((status = session->connection->step(stmt))) {
            case SQLITE_ROW:
                value = session->connection->column_int(session->maximumStmt, 0);
                *valuePtr = value;
                break;
            case SQLITE_DONE:
                return 0;
            case SQLITE_BUSY:
                sleep(1);
                break;
            case SQLITE_ERROR:
                fprintf(stderr, "sql %d error: %s (%d)\n",status,session->connection->errmsg(session->handle),session->connection->errcode(session->handle));
                return 1;
            case SQLITE_MISUSE:
                fprintf(stderr, "sql %d misuse: %s (%d)\n",status,session->connection->errmsg(session->handle),session->connection->errcode(session->handle));
                return 1;
            default:
                break;
        }
    } while(status == SQLITE_BUSY);
    return 0;
}

static int
newrevision(session_type session, long* revisionPtr)
{
    int revision = -1;
    session->connection->reset(session->beginStmt);
    eatResult(session, session->beginStmt);
    session->connection->reset(session->maximumStmt);
    getInteger(session, session->maximumStmt, &revision);
    if (revision <= 0) {
        revision = 1;
    } else {
        revision = revision + 1;
    }
    session->connection->reset(session->forwardStmt);
    session->connection->bind_int(session->forwardStmt, 1, revision);
    *revisionPtr = revision;
    eatResult(session, session->forwardStmt);
    session->connection->reset(session->commitStmt);
    eatResult(session, session->commitStmt);
    return 0;
}

static int
purgerevision(session_type session, long revision)
{
    int changed;
    do {
        session->connection->reset(session->beginStmt);
        eatResult(session, session->beginStmt);
        session->connection->reset(session->purgeStmt);
        session->connection->bind_int(session->purgeStmt, 1, revision);
        eatResult(session, session->purgeStmt);
        changed = session->connection->changes(session->handle);
        session->connection->reset(session->commitStmt);
        eatResult(session, session->commitStmt);
        printf("dropped %d\n",changed);
    } while(changed > 0);
    return 0;
}

static int test(session_type session, long revision)
{
    int count;
    session->connection->reset(session->beginStmt);
    eatResult(session, session->beginStmt);
    session->connection->reset(session->testStmt);
    session->connection->bind_int(session->testStmt, 1, revision);
    getInteger(session, session->testStmt, &count);
    printf("TEST %ld %d\n",revision,count);
    /* eatResult(session, session->testStmt); */
    session->connection->reset(session->commitStmt);
    eatResult(session, session->commitStmt);
    return 0;
}

static int
opensession(connection_type connection, session_type* resultSession)
{
    session_type session;

    *resultSession = NULL;
    session = malloc(sizeof(struct session_struct));
    session->connection = connection;
    session->handle = NULL;
    session->returnCode = 0;
    session->errorCode = 0;
    session->errorMessage = NULL;
    if (SQLITE_OK != (session->returnCode = connection->open(":memory:", &session->handle))) {
        fprintf(stderr, "Error opening database %s (%d)\n",connection->errmsg(session->handle),connection->errcode(session->handle));
    }
    if (SQLITE_OK != (session->returnCode = connection->prepare(session->handle, getpropStatementString, strlen(getpropStatementString)+1, &session->getpropStmt, NULL))) {
        if((session->returnCode = connection->finalize(session->getpropStmt)))
            fprintf(stderr, "sql error: %s (%d)\n",connection->errmsg(session->handle),connection->errcode(session->handle));
        session->getpropStmt = NULL;
    }
    if(!isuptodate(session)) {
        if (SQLITE_OK != (connection->exec(session->handle, schema_sqlite, gobbleResult, NULL, &session->errorMessage))) {
            fprintf(stderr, "sql error: %s (%d)\n",connection->errmsg(session->handle),connection->errcode(session->handle));
        }
        if (SQLITE_OK != (session->returnCode = connection->prepare(session->handle, getpropStatementString, strlen(getpropStatementString)+1, &session->getpropStmt, NULL))) {
            session->getpropStmt = NULL;
            fprintf(stderr, "sql error: %s (%d)\n",connection->errmsg(session->handle),connection->errcode(session->handle));
        }
        isuptodate(session);
    }
    if (SQLITE_OK != (session->returnCode = connection->prepare(session->handle, insertStatementString, strlen(insertStatementString)+1, &session->insertStmt, NULL))) {
        fprintf(stderr, "sql error: %s (%d)\n",connection->errmsg(session->handle),connection->errcode(session->handle));
    }
    if (SQLITE_OK != (session->returnCode = connection->prepare(session->handle, maximumStatementString, strlen(maximumStatementString)+1, &session->maximumStmt, NULL))) {
        fprintf(stderr, "sql error: %s (%d)\n",connection->errmsg(session->handle),connection->errcode(session->handle));
    }
    if (SQLITE_OK != (session->returnCode = connection->prepare(session->handle, forwardStatementString, strlen(forwardStatementString)+1, &session->forwardStmt, NULL))) {
        fprintf(stderr, "sql error: %s (%d)\n",connection->errmsg(session->handle),connection->errcode(session->handle));
    }
    if (SQLITE_OK != (session->returnCode = connection->prepare(session->handle, purgeStatementString, strlen(purgeStatementString)+1, &session->purgeStmt, NULL))) {
        fprintf(stderr, "sql error: %s (%d)\n",connection->errmsg(session->handle),connection->errcode(session->handle));
    }
    if (SQLITE_OK != (session->returnCode = connection->prepare(session->handle, testStatementString, strlen(testStatementString)+1, &session->testStmt, NULL))) {
        fprintf(stderr, "sql error: %s (%d)\n",connection->errmsg(session->handle),connection->errcode(session->handle));
    }
    if (SQLITE_OK != (session->returnCode = connection->prepare(session->handle, beginStatementString, strlen(beginStatementString)+1, &session->beginStmt, NULL))) {
        fprintf(stderr, "sql error: %s (%d)\n",connection->errmsg(session->handle),connection->errcode(session->handle));
    }
    if (SQLITE_OK != (session->returnCode = connection->prepare(session->handle, commitStatementString, strlen(commitStatementString)+1, &session->commitStmt, NULL))) {
        fprintf(stderr, "sql error: %s (%d)\n",connection->errmsg(session->handle),connection->errcode(session->handle));
    }

    *resultSession = session;
    return 0;
}

static int
closesession(session_type session)
{
    session->connection->finalize(session->getpropStmt);
    session->connection->finalize(session->insertStmt);
    session->connection->finalize(session->maximumStmt);
    session->connection->finalize(session->forwardStmt);
    session->connection->finalize(session->beginStmt);
    session->connection->finalize(session->commitStmt);
    session->connection->close(session->handle);
    return 0;
}

struct backend
sqliteRegister(void)
{
    int (*threadsafefunc)(void);
    struct connection_struct* connection;
    struct backend backend;
    
    connection = malloc(sizeof(struct connection_struct));
    connection->dlhandle = dlopen("libsqlite3.so", RTLD_LAZY);
    connection->open = (int(*)(const char *, sqlite3 **))functioncast(dlsym(connection->dlhandle, "sqlite3_open"));
    connection->close       = (int (*)(sqlite3*))functioncast(dlsym(connection->dlhandle, "sqlite3_close"));
    connection->free        = (int (*)(void*))functioncast(dlsym(connection->dlhandle, "sqlite3_free"));
    connection->errmsg      = (const char* (*)(sqlite3*))functioncast(dlsym(connection->dlhandle, "sqlite3_errmsg"));
    connection->errcode     = (int (*)(sqlite3*))functioncast(dlsym(connection->dlhandle, "sqlite3_errcode"));
    connection->exec        = (int (*)(sqlite3*,const char*,int (*)(void*,int,char**,char**),void*,char**))functioncast(dlsym(connection->dlhandle, "sqlite3_exec"));
    connection->prepare     = (int (*)(sqlite3*,const char*,int,sqlite3_stmt**,const char**))functioncast(dlsym(connection->dlhandle, "sqlite3_prepare"));
    connection->step        = (int (*)(sqlite3_stmt*))functioncast(dlsym(connection->dlhandle, "sqlite3_step"));
    connection->reset       = (int (*)(sqlite3_stmt *))functioncast(dlsym(connection->dlhandle, "sqlite3_reset"));
    connection->bind_int    = (int (*)(sqlite3_stmt*, int, int))functioncast(dlsym(connection->dlhandle, "sqlite3_bind_int"));
    connection->bind_text   = (int (*)(sqlite3_stmt*,int,const char*,int,void(*)(void*)))functioncast(dlsym(connection->dlhandle, "sqlite3_bind_text"));
    connection->column_text = (const unsigned char *(*)(sqlite3_stmt*,int))functioncast(dlsym(connection->dlhandle, "sqlite3_column_text"));
    connection->column_int  = (int(*)(sqlite3_stmt*,int))functioncast(dlsym(connection->dlhandle, "sqlite3_column_int"));
    connection->changes     = (int(*)(sqlite3*))functioncast(dlsym(connection->dlhandle, "sqlite3_changes"));
    connection->finalize   = (int (*)(sqlite3_stmt *))functioncast(dlsym(connection->dlhandle, "sqlite3_finalize"));

    backend.connection = connection;
    backend.opensession  = opensession;
    backend.closesession = closesession;
    backend.registerchange = registerchange;
    backend.newrevision = newrevision;
    backend.purgerevision = purgerevision;
    backend.test = test;

    threadsafefunc = (int (*)(void))functioncast(dlsym(connection->dlhandle, "sqlite3_threadsafe"));
    if (!threadsafefunc()) {
        fprintf(stderr, "SQLite not thread safe\n");
    }

    return backend;
}

#else

struct backend
sqliteRegister(void)
{
    struct backend backend;
    memset(backend, 0, sizeof(struct backend));
    return backend;
}

#endif
