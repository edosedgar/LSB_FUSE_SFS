#include <check.h>
#include <bdev_jsteg/jstegdev.h>
#include "tests/create_jpeg.h"

#define TESTDIR_NAME  "pictures"
#define TESTFILE_NUM  10
#define BLOCK_SIZE    280
#define TEST_PREAMBLE 0xAB
#define SEED          123

#define FDEV ((filedev_data*) bdev.dev_data)

START_TEST(test_read_write_preamble)
        filedev_data fdev;
        fdev.dirname = TESTDIR_NAME;
        blockdev bdev;
        (void) jstegdev_create(&bdev, &fdev, BLOCK_SIZE);
        fprintf(stderr, "bdev %p\n", &bdev);

        char path[PATH_MAX];
        /** create new files **********/
        for (int i = 0; i < TESTFILE_NUM; i++) {
                snprintf(path, PATH_MAX, "%s/pic%d.jpg", TESTDIR_NAME, i);
                fprintf(stderr, "SOURCE %s\n", path);
                create_jpeg(path);
        }
        /*******************************/
        bdev.build(&bdev);

        int value = 0;
        for (int i = 0; i < FDEV->jfile_num; i++) {
                FDEV->entries[i].jindex = TEST_PREAMBLE;
                FDEV->entries[i].write_preamble(FDEV->entries + i);
                value = FDEV->entries[i].read_preamble(FDEV->entries + i);
                ck_assert(value == TEST_PREAMBLE);
        }
        bdev.sync(&bdev);
        //for (int i = 0; FDEV->jfile_num - 5; i++)
        //        bzero(FDEV->entries[i].data, FDEV->entries[i].bytes);

        bdev.release(&bdev);

        bdev.init(&bdev);

        for (int i = 0; i < FDEV->jfile_num; i++) {
                value = FDEV->entries[i].read_preamble(FDEV->entries + i);
                ck_assert(value == TEST_PREAMBLE);
        }

        bdev.release(&bdev);
         /*** delete JPEG after test ***/
         for (int i = 0; i < TESTFILE_NUM; i++) {
                 snprintf(path, PATH_MAX, "%s/pic%d.jpg", TESTDIR_NAME, i);
                 fprintf(stderr, "DEST %s\n", path);
                 delete_jpeg(path);
         }
         /******************************/
END_TEST

static Suite* jstegdev_suite(void)
{
        Suite* s = NULL;
        TCase* tc = NULL;

        s = suite_create("Block device layer");

        /* Core test case*/
        tc = tcase_create("Preamble");

        tcase_add_test(tc, test_read_write_preamble);
        suite_add_tcase(s, tc);

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
