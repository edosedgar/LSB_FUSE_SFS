#include <check.h>
#include <bdev_jsteg/hamming.h>

#define TEST_VALUE 300
#define MSG         7

START_TEST(test_hamming)
    uint16_t test_value = TEST_VALUE;
    uint8_t pos = hamming_encode(test_value, (uint8_t)MSG);
    fprintf(stderr, "value %ud\n", pos);
    ck_assert(0);
END_TEST

static Suite* jstegdev_suite(void)
{
        Suite* s = NULL;
        TCase* tc_hamming = NULL;

        s = suite_create("Hamming code (15, 11)");

        /* Core test case */
        tc_hamming = tcase_create("Hamming");

        tcase_add_test(tc_hamming, test_hamming);

        suite_add_tcase(s, tc_hamming);

        return s;
}

int main(void)
{
        int number_failed = 0;
        Suite* jdev_suite = NULL;
        SRunner* sr = NULL;

        jdev_suite = jstegdev_suite();
        sr = srunner_create(jdev_suite);

        srunner_run_all(sr, CK_VERBOSE);
        number_failed = srunner_ntests_failed(sr);
        srunner_free(sr);

        //unlink(TESTFILE_NAME);

        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}