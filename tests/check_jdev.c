#include <check.h>
#include <bdev_jsteg/jstegdev.h>
#include "tests/create_jpeg.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define TESTDIR_NAME "pictures"
#define TESTFILE_NUM 3
#define BLOCK_SIZE 280
#define SEED 123
#define NEW_FILES 3

START_TEST(test_jdev_create)
        filedev_data fdev;
        fdev.dirname = TESTDIR_NAME;
        blockdev bdev;

        ck_assert(jstegdev_create(&bdev, &fdev, BLOCK_SIZE) == &bdev);
        ck_assert(jstegdev_create(NULL, &fdev, BLOCK_SIZE) == NULL);
END_TEST

START_TEST(test_jdev_build)
#define FDEV ((filedev_data*) bdev.dev_data)
        filedev_data fdev;
        fdev.dirname = TESTDIR_NAME;
        blockdev bdev;

        (void) jstegdev_create(&bdev, &fdev, BLOCK_SIZE);
        fprintf(stderr, "bdev %p\n", &bdev);
        bdev.build(&bdev);

        ck_assert_msg(FDEV->jfile_num == TESTFILE_NUM,
                "JNUM %d\n", FDEV->jfile_num);

        int value;
        size_t size = 0;
        for (int i = 0; i < FDEV->jfile_num; i++) {
                value = FDEV->entries[i].read_preamble(FDEV->entries + i);
                ck_assert(value == i);
                ck_assert(value == FDEV->entries[i].jindex);

                addr_space_handler_t* addr_space = FDEV->addr_space;
                size += addr_space->size_rev_transform(FDEV->entries[i].bytes);
        }
        fprintf(stderr, "bdev.size %lu\n", bdev.size);
        fprintf(stderr, "size %lu\n", size);
        ck_assert(size == bdev.size);

        bdev.release(&bdev);
END_TEST

START_TEST(test_jdev_init)
#define FDEV ((filedev_data*) bdev.dev_data)
        filedev_data fdev;
        fdev.dirname = TESTDIR_NAME;
        blockdev bdev;
        (void) jstegdev_create(&bdev, &fdev, BLOCK_SIZE);
        fprintf(stderr, "bdev %p\n", &bdev);

        /** change file name before init **/
        /*
        errno = 0;
        DIR* dir = opendir(TESTDIR_NAME);

        struct dirent* dentry;

        dentry = readdir(dir);
        //  number of files in directory
        char old_path[PATH_MAX];
        char new_path[PATH_MAX];
        for (; dentry != NULL; dentry = readdir(dir)) {
                snprintf(old_path, PATH_MAX, "%s/%s", TESTDIR_NAME,
                         dentry->d_name);
                snprintf(new_path, PATH_MAX, "%s/pic%d.jpg", TESTDIR_NAME,
                         rand());
                rename(old_path, new_path);
        }
        */
        /***********************************/

        /**create new files before init**/
        char path[PATH_MAX];
        for (int i = 0; i < NEW_FILES; i++) {
                snprintf(path, PATH_MAX, "%s/pic%d.jpg", TESTDIR_NAME, i);
                fprintf(stderr, "SOURCE %s\n", path);
                create_jpeg(path);
        }
        /*******************************/
        bdev.init(&bdev);

        ck_assert_msg(FDEV->jfile_num == TESTFILE_NUM,
                "JNUM %d\n", FDEV->jfile_num);
        int value;
        size_t size = 0;
        for (int i = 0; i < FDEV->jfile_num; i++) {
                value = FDEV->entries[i].read_preamble(FDEV->entries + i);
                ck_assert(value == i);
                ck_assert(value == FDEV->entries[i].jindex);

                size += FDEV->entries[i].bytes;
        }
        fprintf(stderr, "bdev.size %lu\n", bdev.size);

        // test initial address
        ck_assert_msg(FDEV->entries[0].start == 0);

        // aligning test

        bdev.release(&bdev);

        /*** delete JPEG after test ***/
        for (int i = 0; i < NEW_FILES; i++) {
                snprintf(path, PATH_MAX, "%s/pic%d.jpg", TESTDIR_NAME, i);
                fprintf(stderr, "DEST %s\n", path);
                delete_jpeg(path);
        }
        /******************************/
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
        tcase_add_test(tc_init, test_jdev_build);
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
