#include <check.h>
#include <bdev_jsteg/jstegdev.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define TESTDIR_NAME "pictures"
#define TESTFILE_NUM 3
#define BLOCK_SIZE 255
#define FREE_SPACE_0 2373

START_TEST(test_jdev_create)
        filedev_data fdev;
        fdev.dirname = TESTDIR_NAME;
        blockdev bdev;

        ck_assert(jstegdev_create(&bdev, &fdev, BLOCK_SIZE) == &bdev);
        ck_assert(jstegdev_create(NULL, &fdev, BLOCK_SIZE) == NULL);
END_TEST

START_TEST(test_jdev_init)
#define FDEV ((filedev_data*) bdev.dev_data)
        filedev_data fdev;
        fdev.dirname = TESTDIR_NAME;
        blockdev bdev;
        (void) jstegdev_create(&bdev, &fdev, BLOCK_SIZE);
        fprintf(stderr, "bdev %p\n", &bdev);
        bdev.init(&bdev);

        ck_assert_msg(FDEV->jfile_num == TESTFILE_NUM,
                "JNUM %d\n", FDEV->jfile_num);
        ck_assert(FDEV->entries[0].jindex == 0);
        ck_assert(FDEV->entries[0].bytes == FREE_SPACE_0);

        // test initial address
        ck_assert_msg(FDEV->entries[0].start == 0);

        // aligning test

        bdev.release(&bdev);
#undef FDEV
END_TEST

static Suite* jstegdev_suite(void)
{
	Suite* s = NULL;
        TCase* tc_init = NULL;

        s = suite_create("Block device layer");

        /* Core test case */
        tc_init = tcase_create("Init");

        tcase_add_test(tc_init, test_jdev_create);
        tcase_add_test(tc_init, test_jdev_init);

        suite_add_tcase(s, tc_init);

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