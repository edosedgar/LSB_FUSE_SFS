/*
<FUSE-based implementation of SFS (Simple File System)>
    Copyright (C) 2016  <Edgar Kaziahmedov>

 This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "config.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "bdev_jsteg/jstegdev.h"
#include "generic/error_prints.h"
#include "mksfs/mksfs.h"

#ifndef HAVE_PROGRAM_INVOCATION_NAME
char *program_invocation_name;
#endif

void
die(void)
{
        exit(1);
}

/*
 * Print help and extended help
 */
static void usage()
{
        puts("usage: mksfs [OPTIONS] DIR\n"
             "\n"
             "Main:\n"
             "  -m   Metadata size\n"
             "       Default is 5% of file size, but no more than 10M\n"
             "  -b   Block size (in bytes). It should be more than\n"
             "       128B and be power of 2 (default is 512B)\n"
             "  -l   UTF-8 volume name\n"
             "\n"
             "Miscellaneous:\n"
             "  -h   print help message");
        exit(0);
}

/*
 * Convert size suffix to number(B, K, M, G)
 * Number without postfix is handled as B
 */
static size_t convert_size(char* parameter, off_t file_s) 
{
        ssize_t number = 0;
        char err_c = 0;
        char last_sym = 0;
        int ret_code = 0;
        errno = 0;
        /* Try to recognize a number */
        ret_code = sscanf(parameter, "%lu%c%c", &number, &last_sym, &err_c);
        if (err_c != '\0' || ret_code == 0 || number < 0) {
                errno = EINVAL;
                return (size_t)(-1);
        }
        /* Try to recognize a unit of size */
        switch (last_sym) {
        case '%':
                if (file_s != 0)
                        return (size_t)(file_s * number / 100L);
                break;
        case 'B':
        case 'b':
        case '\0':
                return number;
        case 'K':
        case 'k':
                return 1024 * number;
        case 'M':
        case 'm':
                return 1024 * 1024 * number;
        case 'G':
        case 'g':
                return 1024 * 1024 * 1024 * number;
        default:
                errno = EINVAL;
                return (size_t)(-1);
        }
        errno = EINVAL;
        return (size_t)(-1);
}

int main(int argc, char* argv[]) {
        extern int opterr, optopt;
        opterr = 0;
        int opt = 0;
        struct sfs_options sfs_opts;
        /* Service variables */
        size_t rsrvd_size       = MBR_SIZE;
        size_t index_size       = 0;
        size_t block_size       = 0;
        size_t total_blocks     = 0;
        size_t index_sz_perblk  = 0;
        off_t  file_size        = 0;
        char*  index_size_s     = NULL;
        char*  block_size_s     = NULL;
        char*  label            = NULL;
        filedev_data tmp_fdev;
        blockdev tmp_bdev;

        if (!program_invocation_name || !*program_invocation_name) {
                static char name[] = "mksfs";
                program_invocation_name =
                        (argv[0] && *argv[0]) ? argv[0] : name;
        }

        if (argc == 1)
                error_msg_and_help("must have OPTIONS");

        /* Get user options */
        while ((opt = getopt(argc, argv, "hm:b:l:")) != -1)
                switch (opt) {
                case 'h':
                        usage();
                        break;
                case 'm':
                        index_size_s = optarg; 
                        break;
                case 'b':
                        block_size_s = optarg;
                        break;
                case 'l':
                        label = optarg;
                        break;
                default:
                        error_msg_and_help("unrecognized option '%c'", optopt);
                }
        /* 
         * Try to open directory
         */
        DIR* dir = opendir(argv[argc - 1]);
        if (dir == NULL)
                error_msg(strerror(errno));
        /*
         * File size calculate and check it
         */
        //file_size = lseek(fd, 0, SEEK_END);
        //close(fd);

        /*
         * File size calculate and check it
         */

        if (jstegdev_create(&tmp_bdev, &tmp_fdev, DEFAULT_BLOCK_SIZE) != &tmp_bdev)
                exit(EXIT_INPFILE);
        tmp_fdev.dirname = argv[argc - 1];
        //fprintf(stderr, "%s\n", tmp_fdev.dirname);
        if (tmp_bdev.init(&tmp_bdev) != 0)
                exit(EXIT_INPFILE);
        file_size = tmp_bdev.size;
        //fprintf(stderr, "file_size %lu\n", file_size);
        if (file_size < (MBR_SIZE + INDEX_MIN_SIZE))
                error_msg_and_die("image size %luB is less than %luB",
                                  file_size, MBR_SIZE + INDEX_MIN_SIZE);
        /* 
         * Handler blocksize data 
         */ 
        if (block_size_s == NULL) 
                block_size = DEFAULT_BLOCK_SIZE;
        else {
                block_size = convert_size(block_size_s, 0);
                /* block size must be greater than 128B */
                if (block_size <= DEFAULT_MIN_BLOCK/2 || errno == EINVAL)
                        error_msg_and_help("invalid block size");
                long int divisor = DEFAULT_MIN_BLOCK;
                /* Check on the power of two */
                while (divisor > 0 && (divisor != block_size)) 
                        divisor <<= 1;

                if (divisor < 0)
                        error_msg_and_die("block size isn't the power "
                                          "of 2");
        }
        if (block_size > file_size)
                error_msg_and_die("block size cannot be more than %luB "
                                  "(image size)", file_size);
        if (file_size % block_size != 0)
                error_msg_and_die("block size isn't a divisor of %luB "
                                  "(image size)", file_size);
        /*
         * Calculate reserved area size in bytes
         */
        if (block_size <= MBR_SIZE)
                rsrvd_size = MBR_SIZE;
        else
                rsrvd_size = block_size;
        /* 
         * Handle index size 
         */
        if (index_size_s == NULL) {
                double buf = DEFAULT_INDEX_PERCENT * file_size / 100L;
                index_size = (size_t)round(buf);
                /* If index_size > 10M */
                if (index_size > (10 * 1024 * 1024))
                        index_size = 10 * 1024 * 1024;

        } else {
                index_size = convert_size(index_size_s, file_size);
                if (index_size == 0 || errno == EINVAL)
                        error_msg_and_die("invalid metadata size");
        }
        /* Auto align to BLOCK_SIZE (up) */
        if (index_size % block_size != 0) {
                index_size += block_size - index_size % block_size; 
                error_msg("metadata size was aligned to %luB", index_size);
        }
        /* Check index size(maybe file size too small) */
        if (index_size > (file_size - rsrvd_size))
                error_msg_and_die("metadata size cannot be more or equal "
                                  "than image size %luB",
                                  file_size - rsrvd_size);
        /* Check size of index area */
        if (index_size < INDEX_MIN_SIZE)
                error_msg_and_die("metadata size cannot be smaller than "
                                  "%lu", INDEX_MIN_SIZE);
        /*
         * Handler label name
         */
        unsigned length = 0;
        int i = 0;
        if (label != NULL && (length = strlen(label)) >= VOLUME_NAME_SIZE)
                error_msg_and_die("label cannot be longer than %ld symbols",
                                  VOLUME_NAME_SIZE - 1);
        /* Check on unsupported symbols */ 
        for (i = 0; i < length; i++)
                if (label[i] < 0x20   || 
                   (label[i] >= 0x80  && label[i] <= 0x9F) ||
                    label[i] == '"'   || label[i] == '*'   ||
                    label[i] == ':'   || label[i] == '<'   ||
                    label[i] == '>'   || label[i] == '?'   ||
                    label[i] == '\\'  || label[i] == 0x5C  ||
                    label[i] == 0x7F  || label[i] == 0xA0)
                        error_msg_and_die("unsupported symbol \'%c\' "
                                          "in volume name", label[i]);
        /*
         * Start to flll fields of options struct
         */
        total_blocks = file_size / block_size;
        /* Convert reserved area size to size in blocks */
        rsrvd_size /= block_size;
        /* Convert index area size to align size per block size */
        if (index_size % block_size == 0) 
                index_sz_perblk = index_size / block_size;
        else
                index_sz_perblk = index_size / block_size + 1;
        /* Fill struct */
        sfs_opts.time_stamp = time(NULL);
        sfs_opts.data_size = total_blocks - rsrvd_size - index_sz_perblk; 
        sfs_opts.index_size = index_size;
        sfs_opts.total_block = total_blocks;
        sfs_opts.reserved_size = rsrvd_size;
        sfs_opts.block_size = (size_t)log2(block_size) - BEGIN_POWER_OF_BS;
        if (label != NULL)
                strcpy(sfs_opts.label, label);
        else 
                sfs_opts.label[0] = '\0';
        sfs_opts.file_name = calloc((strlen(argv[argc - 1]) + 1),
                                    sizeof(char));
        strcpy(sfs_opts.file_name, argv[argc - 1]);
        /*
         * Create empty SFS image 
         */
        if (image_create(sfs_opts) != 0) {
                free(sfs_opts.file_name);
                return EXIT_FAILURE;
        }
        free(sfs_opts.file_name);
        return EXIT_SUCCESS;
}
