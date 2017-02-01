#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mysql/mysql.h>
#include "schema_mysql.h"
#include "dataset.h"

struct settings;
static dataset_t policies;

struct settings {
    int dataVersion;
};

static MYSQL *mysql;

static int opendatabase()
{
    char *server = "localhost";
    char *user = "test";
    char *password = "test";
    char *database = "test";
    mysql = mysql_init(NULL);
    if (!mysql_real_connect(mysql, server,
            user, password, database, 0, NULL, CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS)) {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        return 1;
    }
    return 0;
}

static int closedatabase()
{
    mysql_close(mysql);
    return 0;
}

int commandModelCreate(void)
{
    int status, count = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;
    
    if (opendatabase()) {
        return 1;
    }

    if (mysql_query(mysql, schema_mysql)) {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        return 1;
    }
    do {
        result = mysql_store_result(mysql);
        if (result) {
            /* were not suppost to have a result */
            fprintf(stderr,"Bad initialization script, expected output");
            /* yes; process rows and free the result set */
            for (count = 0; (row = mysql_fetch_row(result)) != NULL; ++count)
                ;
            printf("rows\t%d\n", count);
            mysql_free_result(result);
        } else {
            if (mysql_field_count(mysql) == 0) {
                printf("%d rows affected\n", (int) mysql_affected_rows(mysql));
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

    if (mysql_query(mysql, "select * from properties;")) {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        return 1;
    }
    do {
        /* result = mysql_use_result(conn); */
        result = mysql_store_result(mysql);
        if (result) {
            /* yes; process rows and free the result set */
            for (count = 0; (row = mysql_fetch_row(result)) != NULL; ++count) {
                while ((field = mysql_fetch_field(result)) != NULL) {
                    /* get the result */
                }
            }
            printf("rows\t%d\n", count);
            mysql_free_result(result);
        } else {
            if (mysql_field_count(mysql) == 0) {
                printf("%d rows affected\n", (int) mysql_affected_rows(mysql));
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
