#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <mysql/mysql.h>

struct zone;
struct keyclass;
struct keyinstance;
struct policy;

struct catalog;
struct collection;

struct catalog* zones;
struct catalog* policies;

struct zone {
    char name[64];
    char* label;
    int inadapttype;
    char* inadapturi;
    int outadapttype;
    int f1;
    int f2;
    int f3;
    int f4;
    time_t t1;
    time_t t2;
    time_t t3;
    time_t t4;
    time_t t5;
    time_t t6;
    time_t t7;
    time_t t8;
    struct policy* policy;
    struct collection* instances;
};

struct keyinstance {
    int id;
    char* locator;
    struct zone* zone;
    struct keyclass* keyclass;
    time_t inception;
    int state;
    int dsstate;
    time_t dsstatesince;
    int ksstate;
    time_t ksstatesince;
    int rrsigstate;
    time_t rrsigstatesince;
    int kssigstate;
    time_t kssigstatesince;
};

struct keyclass {
    int id;
    struct policy* policy;
    int nbyt;
    int algorithm;
    int role;
    time_t lifetime;
};

struct policy {
    char name[64];
    char *label;
};

static MYSQL *mysql;

static int opendatabase()
{
    char *server = "localhost";
    char *user = "opendnssec";
    char *password = "iuap";
    char *database = "ods";
    mysql = mysql_init(NULL);
    if (!mysql_real_connect(mysql, server,
            user, password, database, 0, NULL, CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS)) {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        return 1;
    }
}

static int closedatabase()
{
    mysql_close(mysql);
}

int commandModelCreate(void)
{
    int status, count = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;
    MYSQL_FIELD* field;
    char *cmdsequence = "";
    
    if (opendatabase())
        return 1;

    if (mysql_query(mysql, cmdsequence)) {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        return 1;
    }
    do {
        result = mysql_store_result(mysql);
        if (result) {
            // were not suppost to have a result
            fprintf(stderr,"Bad initialization script, expected output");
            /* yes; process rows and free the result set */
            for (count = 0; (row = mysql_fetch_row(result)) != NULL; ++count)
                ;
            printf("rows\t%d\n", count);
            mysql_free_result(result);
        } else {
            if (mysql_field_count(mysql) == 0) {
                printf("%lld rows affected\n", mysql_affected_rows(mysql));
            } else {
                fprintf(stderr, "Could not retrieve result set\n");
                break;
            }
        }
        /* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
        if ((status = mysql_next_result(mysql)) > 0)
            printf("Could not execute statement\n");
    } while (status == 0);

    closedatabase();
    return 0;
}

int commandModelRead(void)
{
    int status, count = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;
    MYSQL_FIELD* field;

    if (opendatabase())
        return 1;

    if (mysql_query(mysql, "select * from zones; select * from keyinstances; select * from keyclasses; select * from policies;")) {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        return 1;
    }
    do {
        // result = mysql_use_result(conn);
        result = mysql_store_result(mysql);
        if (result) {
            /* yes; process rows and free the result set */
            for (count = 0; (row = mysql_fetch_row(result)) != NULL; ++count) {
                while ((field = mysql_fetch_field(result)) != NULL) {
                    malloc(sizeof (struct zone));
                }
            }
            printf("rows\t%d\n", count);
            mysql_free_result(result);
        } else {
            if (mysql_field_count(mysql) == 0) {
                printf("%lld rows affected\n", mysql_affected_rows(mysql));
            } else {
                printf("Could not retrieve result set\n");
                break;
            }
        }
        /* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
        if ((status = mysql_next_result(mysql)) > 0)
            printf("Could not execute statement\n");
    } while (status == 0);

    closedatabase();
    return 0;
}
