#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

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
    CU_ASSERT(2 * 2 == 4);
}

void
testFail()
{
    CU_ASSERT(2 * 2 == 5);
}

int
main()
{
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("basic suite", init_suite, clean_suite);
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

#ifdef NOTDEFINED
    /* Run all tests using the CUnit Basic interface */
    /CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
#endif

    CU_automated_run_tests();

    CU_cleanup_registry();
    return CU_get_error();
}
