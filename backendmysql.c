#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mysql.h>
#include <dlfcn.h>
#include "utilities.h"
#include "backend.h"
#include "schema_mysql.h"

struct connection_struct {
    void* dlhandle;
    int (*library_init)(int,char**,char**);
    int (*library_end)(void);
    MYSQL*(*init)(MYSQL*);
    MYSQL*(*connect)(MYSQL*,const char* host,const char* user,const char* passwd,const char* db,unsigned int port,const char* unix_socket,unsigned long flags);
    void(*close)(MYSQL*);
};
struct session_struct {
    connection_type connection;
    MYSQL* handle;
};

static int
opensession(connection_type connection, session_type* resultSession)
{
    session_type session;
    MYSQL* newhandle;

    *resultSession = NULL;
    session = malloc(sizeof(struct session_struct));
    session->connection = connection;
    session->handle = NULL;

    newhandle = connection->init(session->handle);
    session->handle = connection->connect(newhandle, "localhost","root","poslww","ods",0,NULL,CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS);
    
    return 0;
}

static int
closesession(session_type session)
{
    session->connection->close(session->handle);
    return 0;
}

struct backend
mysqlRegister(void)
{
    int (*threadsafefunc)(void);
    struct connection_struct* connection;
    struct backend backend;
    
    connection = malloc(sizeof(struct connection_struct));
    connection->dlhandle = dlopen("libsqlite3.so", RTLD_LAZY);
    connection->init    = (MYSQL*(*)(MYSQL*))functioncast(dlsym(connection->dlhandle, "mysql_init"));
    connection->connect = (MYSQL*(*)(MYSQL *,const char *,const char *,const char *,const char *,unsigned int,const char*,unsigned long))functioncast(dlsym(connection->dlhandle, "mysql_real_connect"));
    connection->close   = (void (*)(MYSQL*))functioncast(dlsym(connection->dlhandle, "mysql_close"));

    backend.connection = connection;
    backend.opensession  = opensession;
    backend.closesession = closesession;

    threadsafefunc = (int (*)(void))functioncast(dlsym(connection->dlhandle, "sqlite3_threadsafe"));
    if (!threadsafefunc()) {
        fprintf(stderr, "SQLite not thread safe\n");
    }

    return backend;
}
