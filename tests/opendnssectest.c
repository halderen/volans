#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>

/*
 * CUnit Test Suite
 */

int init_suite(void) {
    printf("init_suite\n");
    return 0;
}

int clean_suite(void) {
    printf("clean_suite\n");
    return 0;
}

void
testGood()
{
    printf("test_good\n");
    CU_ASSERT(2 * 2 == 4);
}

void
testFail()
{
    printf("test_bad\n");
    CU_ASSERT(2 * 2 == 4);
}

extern int commandModelCreate(void);
void
testDatabaseCreate()
{
    CU_ASSERT(!commandModelCreate());
}

int
main()
{
    int fails;
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("Basic", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "testGood", testGood)) ||
            (NULL == CU_add_test(pSuite, "testFail", testFail))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!(CU_add_test(pSuite, "testDatabaseCreate", testDatabaseCreate))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

#ifdef NOTDEFINED
    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
#endif

    CU_automated_run_tests();

    fails = CU_get_number_of_tests_failed();
    CU_cleanup_registry();
    return fails;
}
