#include <check.h>
#include <bdev_jsteg/jstegdev.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define TESTDIR_NAME "pictures"
#define TESTMSG "Hello, world!"
#define BLOCK_SIZE 255

#define FDEV ((filedev_data*) bdev.dev_data)

START_TEST(test_jentry_write)
        filedev_data fdev;
        fdev.dirname = TESTDIR_NAME;
        blockdev bdev;
        (void) jstegdev_create(&bdev, &fdev, BLOCK_SIZE);
        fprintf(stderr, "bdev %p\n", &bdev);
        bdev.init(&bdev);

        char in_buf[] = "test message";
        memcpy(FDEV->entries[2].data, in_buf, sizeof(in_buf));
        FDEV->entries[2].write_data(FDEV->entries + 2);
        bzero(FDEV->entries[2].data, FDEV->entries[2].bytes);
        fprintf(stderr, "TEST %s\n", (char*)FDEV->entries[2].data);
        FDEV->entries[2].read_data(FDEV->entries + 2);
        //filedev_dump(&bdev, BLOCK_SIZE);

        fprintf(stderr, "TEST %s\n", (char*)FDEV->entries[2].data);
        ck_assert(strncmp((void*) FDEV->entries[2].data,
                (void*) in_buf, sizeof(in_buf)) == 0);


        //ck_abort();
        bdev.release(&bdev);

END_TEST

START_TEST(test_jdev_write)
        filedev_data fdev;
        fdev.dirname = TESTDIR_NAME;
        blockdev bdev;
        (void) jstegdev_create(&bdev, &fdev, BLOCK_SIZE);
        fprintf(stderr, "bdev %p\n", &bdev);
        bdev.init(&bdev);

        // read-write test during one session
        // by one block
        buf_t* buf = (buf_t*) malloc(BLOCK_SIZE * sizeof(buf_t));
        buf_t in_buf[] = TESTMSG;
        buf_t* out_buf = malloc(BLOCK_SIZE * sizeof(buf_t));
        bdev.write(&bdev, in_buf, BLOCK_SIZE, 0);
        bdev.read(&bdev, out_buf, BLOCK_SIZE, 0);
        ck_assert(strncmp((void*) in_buf, (void*) out_buf, BLOCK_SIZE) == 0);

        memset((void*) buf, 0xAB, BLOCK_SIZE);
        bdev.write(&bdev, buf, BLOCK_SIZE, 8);
        bdev.read(&bdev, out_buf, BLOCK_SIZE, 8);
        ck_assert(strncmp((void*) buf, (void*) out_buf, BLOCK_SIZE) == 0);

        // by two block
        buf = realloc(buf, BLOCK_SIZE * 2);
        out_buf = realloc(out_buf, BLOCK_SIZE * 2);

        memset((void*) buf, 0xBA, BLOCK_SIZE * 2);
        bdev.write(&bdev, buf, BLOCK_SIZE * 2, 8);
        fprintf(stderr, "BUFFER %s\n", buf);
        bdev.read(&bdev, out_buf, BLOCK_SIZE * 2, 8);
        fprintf(stderr, "OUT_BUFFER %s\n", out_buf);
        //filedev_dump(&bdev, BLOCK_SIZE);
        ck_assert(strncmp((void*) buf, (void*) out_buf, BLOCK_SIZE * 2) == 0);

        // read-write test within two different session
        bdev.sync(&bdev);

        jdev_entry* entry;
        jdev_entry* end = FDEV->entries + FDEV->jfile_num;
        for (entry = FDEV->entries; entry != end; entry++)
                entry->read_data(entry);

        //filedev_dump(&bdev, BLOCK_SIZE);

        // fsync test
        FDEV->entries[0].is_available = 0;
        bzero(FDEV->entries[0].data, FDEV->entries[0].bytes);
        FDEV->entries[0].read_data(FDEV->entries);
        ck_assert(strncmp((void*) FDEV->entries[0].data, 
                (void*) in_buf, sizeof(in_buf)) == 0);

        bdev.release(&bdev);
        //filedev_dump(&bdev, BLOCK_SIZE);

        filedev_data fdev2;
        fdev2.dirname = TESTDIR_NAME;
        (void) jstegdev_create(&bdev, &fdev2, BLOCK_SIZE);
        fprintf(stderr, "bdev %p\n", &bdev);
        bdev.init(&bdev);


        fprintf(stderr, "jfilenum %lu\n", FDEV->jfile_num);
        bdev.read(&bdev, buf, BLOCK_SIZE * 2, 8);
        fprintf(stderr, "BUF %.*s\n", BLOCK_SIZE * 2, buf);
        fprintf(stderr, "OUT %.*s\n", BLOCK_SIZE * 2, out_buf);
        //filedev_dump(&bdev, BLOCK_SIZE);
        //
        // This test doesn't work
        ck_assert(memcmp((void*) buf, (void*) out_buf, BLOCK_SIZE * 2) == 0);

        //ck_abort();

        free(buf);
        free(out_buf);
        bdev.release(&bdev);
END_TEST

#undef FDEV

static Suite* jstegdev_suite(void)
{
        Suite* s = NULL;
        TCase* tc_write = NULL;

        s = suite_create("Block device layer");

        /* Core test case */
        tc_write = tcase_create("Write");

        tcase_add_test(tc_write, test_jdev_write);
        tcase_add_test(tc_write, test_jentry_write);

        suite_add_tcase(s, tc_write);

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
