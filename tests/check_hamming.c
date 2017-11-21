#include <stdio.h>
#include <check.h>
#include <stdlib.h>

#include "bdev_jsteg/hamming.h"

#define TEST_VALUE      1255
#define MSG             0x0C
#define MSG_BUF_SIZE    (7 << 1)
#define VEC_BUF_SIZE    (8 << 1)

START_TEST(test_hamming)
        uint8_t msgs[MSG_BUF_SIZE];
        srand(MSG);
        for (int i = 0; i < MSG_BUF_SIZE; i++) {
                msgs[i] = rand() % 127;
        }

        uint64_t x[VEC_BUF_SIZE];
        for (int i = 0; i < VEC_BUF_SIZE; i++) {
                x[i] = lrand48();
        }

        stego_encode(x, VEC_BUF_SIZE, msgs, MSG_BUF_SIZE);
        uint8_t result[MSG_BUF_SIZE];
        stego_decode(x, VEC_BUF_SIZE, result, MSG_BUF_SIZE);
        srand(MSG);
        for (int i = 0; i < MSG_BUF_SIZE; i++) {
                ck_assert(result[i] == rand() % 127);
        }
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