#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "utilities.h"

static struct option command_options[] = {
    { "help", no_argument, NULL, 'h'},
    { "config", no_argument, NULL, 'c'},
    { "no-daemon", no_argument, NULL, 'd'},
    { "daemon", no_argument, NULL, 'D'},
    { "verbose", no_argument, NULL, 'v'},
    { "version", no_argument, NULL, 'V'},
    { 0, 0, 0, 0}
};

char* argv0;

static void
usage() {
    fprintf(stderr, "usage: %s [-h]\n", argv0);
}

int
main(int argc, char** argv)
{
    int ch;

    /* Get the name of the program */
    if ((argv0 = strrchr(argv[0], '/')) == NULL)
        argv0 = argv[0];
    else
        ++argv0;

    while ((ch = getopt_long(argc, argv, "+hc", command_options, NULL)) >= 0) {
        switch (ch) {
            case 'h':
                usage(argv[0]);
                return EXIT_SUCCESS;
            case '?':
                return EXIT_FAILURE;
            default:
                usage(argv[0]);
                return EXIT_FAILURE;
        };
    }
    if (optind < argc) {
        fprintf(stderr, "%s: unrecognized command line arguments.\n", argv0);
    }
    if (!strcmp(argv0, "volans")) {
    } else if (!strcmp(argv0, "volansd")) {
    } else {
        fprintf(stderr, "%s: bad command line invocation; executable name %s not recognized.\n", argv[0], argv0);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#include "pkcs11.h"

#ifdef NOTDEFINED
static CK_C_INITIALIZE_ARGS InitArgs = { NULL, NULL, NULL, NULL, CKF_OS_LOCKING_OK, NULL };
static void* libhandle;
static CK_FUNCTION_LIST_PTR pkcs11;
static CK_SESSION_HANDLE session;

int openCryptModule() {
    void* symbol;
    libhandle = dlopen("./libvolans.so", RTLD_NOW | RTLD_LOCAL);
    symbol = dlsym(handle, "C_GetFunctionList");
    ((CK_C_GetFunctionList) symbol)(&table);
    table->C_Initialize(&InitArgs);
    table->C_OpenSession(0, CKF_SERIAL_SESSION | CKF_RW_SESSION, NULL, NULL, &session);
    table->C_Login(session, CKU_USER, (unsigned char *) "1234", strlen((char *) "1234"));
    table->C_Logout(session);
    table->C_CloseSession(session);
    table->C_Finalize(NULL_PTR);
    table = NULL;
    dlclose(handle);
}

#endif
