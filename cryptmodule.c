#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#include "pkcs11.h"

static CK_C_INITIALIZE_ARGS InitArgs = { NULL, NULL, NULL, NULL, CKF_OS_LOCKING_OK, NULL };
static void* handle;
static CK_FUNCTION_LIST_PTR table;
static CK_SESSION_HANDLE session;

int openCryptModule() {
    void* symbol;
    handle = dlopen("/usr/local/lib/softhsm/libsofthsm2.so", RTLD_NOW | RTLD_LOCAL);
    symbol = dlsym(handle, "C_GetFunctionList");
    ((CK_C_GetFunctionList) symbol)(&table);
    table->C_Initialize(&InitArgs);
    table->C_OpenSession(0, CKF_SERIAL_SESSION | CKF_RW_SESSION, NULL, NULL, &session);
    table->C_Login(session, CKU_USER, (unsigned char *) "1234", strlen((char *) "1234"));
}

int closeCryptModule() {
    table->C_Logout(session);
    table->C_CloseSession(session);
    table->C_Finalize(NULL_PTR);
    table = NULL;
    dlclose(handle);
    handle = NULL;
}

int createKey() {
}

int encryptData() {
}
