/*
<FUSE-based implementation of LSB_SFS (Simple File System)>
    Copyright (C) 2017  <Grigoriy Melnikov>

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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h> 
#include <time.h>

#include <bdev_jsteg/jstegdev.h>
#include <sfs/debug.h>

#define FDEV ((filedev_data*) bdev->dev_data)

LOCAL(int)
jstegdev_start_construct(blockdev* bdev)
{
        errno = 0;

	if (FDEV->dirname == NULL) {
		errno = EINVAL;
		return -1;
	}
	fprintf(stderr, "DIRNAME: %s\n", FDEV->dirname);
	FDEV->dir = opendir(FDEV->dirname);
	if (FDEV->dir == NULL)
                return -1;

        struct dirent* dentry;
	long loc = telldir(FDEV->dir);

	errno = 0;
	FDEV->jfile_num = 0;
        struct stat sb;
        dentry = readdir(FDEV->dir);
        //  number of files in directory
        char abs_path[PATH_MAX];
	for (; dentry != NULL; dentry = readdir(FDEV->dir)) {
                snprintf(abs_path, PATH_MAX, "%s/%s", FDEV->dirname, dentry->d_name);
                if (stat(abs_path, &sb) == -1)
                        continue;

                if ((sb.st_mode & S_IFMT) != S_IFREG)
                        continue;

                if (access(abs_path, W_OK | R_OK) == 0)
                        FDEV->jfile_num++;	
	}

        SFS_TRACE("File number after first check:%lu\n", FDEV->jfile_num);

	if (errno != 0)
		return -1;

	FDEV->entries = (jdev_entry*) malloc(FDEV->jfile_num * sizeof(jdev_entry));
	if (FDEV->entries == NULL) {
		errno = ENOMEM;
		return -1;
        }
        
        for (int i = 0; i < FDEV->jfile_num; i++)
                FDEV->entries[i].jindex = -1;

	seekdir(FDEV->dir, loc);

	dentry = readdir(FDEV->dir);
        // for each file in the directory ..
        int idx = 0;
	for (; dentry != NULL; dentry = readdir(FDEV->dir)) {
                snprintf(abs_path, PATH_MAX, "%s/%s", FDEV->dirname, dentry->d_name);
                if (stat(abs_path, &sb) == -1)
                        continue;

                if ((sb.st_mode & S_IFMT) != S_IFREG)
                        continue;

                FILE* ftemp = fopen(abs_path, "r+b");
                jdev_entry* entry = jentry_init(ftemp);
                if (entry == NULL) {
                        FDEV->jfile_num--;
                        continue;
                }
                FDEV->entries[idx] = *entry;
                idx++;
        }

        SFS_TRACE("File number after second check:%lu\n", FDEV->jfile_num);
        if (FDEV->jfile_num <= 0) {
                return -1;
        }

        return 0;
}

LOCAL(int)
jstegdev_finish_construct(blockdev* bdev) 
{
        closedir(FDEV->dir);
        
        if ((bdev->buf = (buf_t*) malloc (bdev->block_size)) == NULL) {
                errno = ENOMEM;
                return -1;
        }

        bdev->buf_num = (bnum_t) -1;

        return 0;
}

METHODDEF(int)
jstegdev_build(blockdev* bdev)
{
        if (jstegdev_start_construct(bdev) != 0)
                return -1;

        // calculate size of the block device
        bdev->size = 0;
	for (int i = 0; i < FDEV->jfile_num; i++) {
                bdev->size += FDEV->entries[i].bytes;
                FDEV->entries[i].jindex = i;
        }

        if (bdev->size < bdev->block_size) {
                errno = EINVAL;
                return -1;
        }

        // calculate start of each entry
        FDEV->entries[0].start = 0;
        for (int i = 1; i < FDEV->jfile_num; i++)
                FDEV->entries[i].start = FDEV->entries[i - 1].start 
                                        + FDEV->entries[i - 1].bytes;


        SFS_TRACE("size_before_align %d\n", (int)bdev->size);
        
        // align size of the block device to block_size
        int rest = bdev->size % bdev->block_size;
        SFS_TRACE("BLOCK SIZE: %lu\n", bdev->block_size);
        int last_jindex = FDEV->jfile_num - 1;
        if (rest) {
                SFS_TRACE("REST %d\n", rest);
                SFS_TRACE("LAST JENTRY SIZE %lu\n", FDEV->entries[last_jindex].bytes);
                SFS_TRACE("FIRST JENTRY SIZE %lu\n", FDEV->entries[0].bytes);
                int i;
                // swap entries
                for (i = last_jindex; rest > FDEV->entries[i].bytes && i >= 0; i--);
                fprintf(stderr, "i num: %d\n", i);
                if (i < 0) {
                        FDEV->entries[last_jindex - 1].bytes 
                                -= (rest - FDEV->entries[last_jindex].bytes);
                        FDEV->jfile_num--;
                } else if (i < last_jindex) {
                        SFS_TRACE("SWAP IDX %d\n", i);

                        jdev_entry swapable_entry = FDEV->entries[i];
                        FDEV->entries[i] = FDEV->entries[last_jindex];
                        FDEV->entries[i].jindex = i;

                        FDEV->entries[last_jindex] = swapable_entry;
                        FDEV->entries[last_jindex].jindex = last_jindex; 
                }

                FDEV->entries[last_jindex].bytes -= rest;
                bdev->size -= rest;
        }

        fprintf(stderr, "FILE NUM: %lu\n", FDEV->jfile_num);
        for (int i = 0; i < FDEV->jfile_num; i++) {
                FDEV->entries[i].write_preamble(FDEV->entries + i);
        }

        if (jstegdev_finish_construct(bdev) != 0)
                return -1;        

        return bdev->size;
}

LOCAL(int)
sort_compare(const void* _lhs, const void* _rhs)
{
        jdev_entry *lhs = (jdev_entry *)_lhs;
        jdev_entry *rhs = (jdev_entry *)_rhs;

        // all negative jindex (==-1) should shift to the end
        if (lhs->jindex < 0)
                return -1;

        if (rhs->jindex < 0)
                return 1;

        return lhs->jindex - rhs->jindex;        
}

METHODDEF(int)
jstegdev_init(blockdev* bdev)
{
        errno = 0;

        if (jstegdev_start_construct(bdev) != 0)
                return -1;

        // check indexing of entries
        for (int i = 0; i < FDEV->jfile_num; i++) {
                FDEV->entries[i].jindex 
                        = FDEV->entries[i].read_preamble(FDEV->entries + i);
        }

        qsort(FDEV->entries, FDEV->jfile_num, sizeof(jdev_entry), sort_compare);

        for (int i = 0; i < FDEV->jfile_num; i++) {
                if (FDEV->entries[i].jindex < 0)
                        FDEV->jfile_num--;

                if (FDEV->entries[i].jindex != i) {
                        errno = EBADFD;
                        return -1;
                }
        }

        if (FDEV->jfile_num < 1) {
                errno = EINVAL;
                return -1;
        }

        // calculate start of each entry
        FDEV->entries[0].start = 0;
        for (int i = 1; i < FDEV->jfile_num; i++)
                FDEV->entries[i].start = FDEV->entries[i - 1].start 
                                        + FDEV->entries[i - 1].bytes;


        // experiment 1
        
        for (int i = 0; i < FDEV->jfile_num; i++)
                FDEV->entries[i].read_data(FDEV->entries + i);
        
        // end of experiment

        // experiment 2
        /*
        for (int i = 0; i < FDEV->jfile_num; i++)
                FDEV->entries[i].is_available = 0;
        */
        // end of experiment

        if (jstegdev_finish_construct(bdev) != 0)
                return -1;

        return 0;
}

LOCAL(int)
comparator(const void* _key, const void* _item) {
	int* key = (int*) _key;
	jdev_entry* item = (jdev_entry*) _item;

	if (*key < item->start)
		return -1;

	if (*key >= item->start + item->bytes)
		return 1;
	else
		return 0;
}

METHODDEF(size_t)
jstegdev_write(blockdev* bdev, buf_t* buf, size_t buf_size, bnum_t block_num) 
{
	int start_pos = 0;
        errno = 0;

        if (bdev == NULL) {
                errno = EFAULT;
                return -1;
        }

        if (buf == NULL) {
                errno = EFAULT;
                return -1;
        }

        if (FDEV->dir == NULL) {
                errno = EINVAL;
                return -1;
        }

        if (buf_size % bdev->block_size) {
                errno = EINVAL;
                return -1;
        }

        start_pos = block_num * bdev->block_size;

        jdev_entry* cur_entry = bsearch(&start_pos, 
        				FDEV->entries, 
        				FDEV->jfile_num,
        				sizeof(jdev_entry),
        				comparator);

        if (cur_entry == NULL)
        	return -1;

        SFS_TRACE("FIRST ENTRY: %d\n", cur_entry->jindex);
        SFS_TRACE("END: %ld\n", cur_entry->start + cur_entry->bytes);

        jdev_entry* end = FDEV->entries + FDEV->jfile_num;
        int offset = start_pos - cur_entry->start;
        SFS_TRACE("OFFSET: %d\n", offset);
        SFS_TRACE("END OFFSET: %ld\n", offset + buf_size);


        int written_bytes = 0;
        int remained_bytes = buf_size;
        while (remained_bytes > 0) {
        	if (cur_entry == end) {
        		errno = EINVAL;
        		return -1;
        	}

                // it's needed to read before write
                if (!cur_entry->is_available) {
                        cur_entry->read_data(cur_entry);
                        //fprintf(stderr, "AVAILABLE WRITE %d\n", cur_entry->jindex);
                        cur_entry->is_available = 1;
                }

        	written_bytes = remained_bytes < cur_entry->bytes - offset 
        			? remained_bytes : cur_entry->bytes - offset;

        	memcpy(cur_entry->data + offset, buf + buf_size - remained_bytes, written_bytes);

        	remained_bytes -= written_bytes;

        	cur_entry++;
        	offset = 0;
        }

        SFS_TRACE("LAST ENTRY: %d\n", cur_entry->jindex);
        return (size_t) buf_size;
}

METHODDEF(size_t)
jstegdev_read(blockdev* bdev, buf_t* buf, size_t buf_size, bnum_t block_num)
{
	int start_pos = 0;
        errno = 0;

        if (bdev == NULL) {
                errno = EFAULT;
                return -1;
        }

        if (buf == NULL) {
                errno = EFAULT;
                return -1;
        }

        if (FDEV->dir == NULL) {
                errno = EINVAL;
                return -1;
        }

        if (buf_size % bdev->block_size) {
                errno = EINVAL;
                return -1;
        }

        start_pos = block_num * bdev->block_size;
        jdev_entry* cur_entry = bsearch(&start_pos, 
        				FDEV->entries, 
        				FDEV->jfile_num,
        				sizeof(jdev_entry),
        				comparator);

        SFS_TRACE("FIRST ENTRY: %d\n", cur_entry->jindex);
        SFS_TRACE("END: %ld\n", cur_entry->start + cur_entry->bytes);

        
        jdev_entry* end = FDEV->entries + FDEV->jfile_num;
        int offset = start_pos - cur_entry->start;
        SFS_TRACE("Read OFF %d\n", offset);
        int read_bytes = 0;
        int remained_bytes = buf_size;
        while (remained_bytes > 0) {
        	if (cur_entry == end) {
        		errno = EINVAL;
        		return -1;
        	}

                if (!cur_entry->is_available) {
                        cur_entry->read_data(cur_entry);
                        SFS_TRACE("AVAILABLE READ %d\n", cur_entry->jindex);
                        cur_entry->is_available = 1;
                }

        	read_bytes = remained_bytes < cur_entry->bytes - offset 
        			? remained_bytes : cur_entry->bytes - offset;

        	memcpy(buf + buf_size - remained_bytes, cur_entry->data + offset, read_bytes);

        	remained_bytes -= read_bytes;
        	cur_entry++;
        	offset = 0;
        }

        SFS_TRACE("LAST ENTRY: %d\n", cur_entry->jindex);
        return buf_size;
}

METHODDEF(int)
jstegdev_release(blockdev* bdev)
{
        SFS_TRACE("RELEASE\n");
        jdev_entry* entry;

	for (int i = 0; i < FDEV->jfile_num; i++) {
                entry = FDEV->entries + i;
                entry->jentry_release(entry);
        }

        free(FDEV->entries);

        return 0;
}

METHODDEF(int)
jstegdev_sync(blockdev* bdev)
{
        //fprintf(stderr, "SYNC\n");
        jdev_entry* entry;
        jdev_entry* end = FDEV->entries + FDEV->jfile_num;


        errno = 0;
        int fd;
        for (entry = FDEV->entries; entry != end; entry++) {
                //if (entry->is_available) {
                        //fprintf(stderr, "is_available %d\n", entry->jindex);
                        entry->write_data(entry);
                        fd = fileno(entry->file);
                        if (fd == -1 || errno == EBADF)
                                return -1;

                        fsync(fd);
                //}
        }

        fd = dirfd(FDEV->dir);
        if (fd == -1)
                return -1;

        fsync(fd);

        return 0;
}


GLOBAL(blockdev*)
jstegdev_create(blockdev* bdev, filedev_data* fdev, size_t block_size) 
{
        errno = 0;

        if (bdev == NULL) {
                errno = EFAULT;
                return NULL;
        }

        bdev->block_size = block_size;
        bdev->dev_data = fdev;
        bdev->init = jstegdev_init;
        bdev->read = jstegdev_read;
        bdev->write = jstegdev_write;
        bdev->release = jstegdev_release;
        bdev->sync = jstegdev_sync;
        bdev->build = jstegdev_build;

        return bdev;
}

#ifndef FILEDEV_DEBUG
void filedev_dump(blockdev* bdev, const size_t block_size)
{
        if (bdev == NULL || FDEV == NULL) {
                fprintf(stderr, "Invalid pointers\n");
        }
        fprintf(stderr, "<--FILEDEV DATA DUMP begin-->:\n");

        int pos = 0;
        buf_t* buffer = malloc(block_size * sizeof(buf_t));

        jdev_entry* cur_entry = FDEV->entries;
        int end = FDEV->entries[FDEV->jfile_num - 1].start
                        + FDEV->entries[FDEV->jfile_num - 1].bytes;

        //fprintf(stderr, "END %d\n", end);
        //int rest = cur_entry->start + cur_entry->bytes - pos;

        int rest;
        int offt;
        while (pos < end) 
        {
                offt = pos - cur_entry->start;
                rest = cur_entry->bytes - offt;
                //fprintf(stderr, "rest %d\n", rest);

                memcpy(buffer, cur_entry->data + offt, 
                        block_size > rest ? rest : block_size);

                fprintf(stderr, "==========================\n");
                fprintf(stderr, "BLOCK_NUM: %ld\n", pos / block_size);
                fprintf(stderr, "POS %d\n", pos);
                fprintf(stderr, "==========================\n");

                if (block_size > rest) {
                        cur_entry++;
                        memcpy(buffer + rest, cur_entry->data, block_size - rest);
                        fprintf(stderr, "TAIL %ld\n", block_size - rest);
                }

                pos += block_size;

                for (int i = 0; i < block_size; i++) {
                        fprintf(stderr, "%c", buffer[i]);        
                }


                fprintf(stderr, "\n");
        }


        free(buffer);

        fprintf(stderr, "<--FILEDEV DATA DUMP end-->\n");
}

uint64_t get_time() {
        return (uint64_t) time(NULL);
}

#endif