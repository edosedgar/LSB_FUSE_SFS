/*
<FUSE-based implementation of SFS (Simple File System)>
    Copyright (C) 2016  <Klim Kireev>

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

#include <bdev_jsteg/blockdev.h>
#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/unit.h>
#include <sfs/callback.h>
#include <sfs/fsutils.h>
#include <sfs/utils.h>
#include <sfs/alloc.h>

#define AS_FILE(entr) ((file_entry*) (entr))

static inline uint8_t count_entry(size_t len)
{
        if (len <= FIRST_FILE_NAME_SIZE) return 1;

        return 1 + (((len - FIRST_FILE_NAME_SIZE) / (INDEX_ENTRY_SIZE)) + 
               !!(len - FIRST_FILE_NAME_SIZE) % (INDEX_ENTRY_SIZE));
}

off_t sfs_rename(sfs_unit* fs, off_t file, const char* newpath)
{
        entry entr;
        off_t new_off = 0;
        off_t cur = 0;
        size_t len = 0;
        uint8_t n = 0;
        uint64_t start = 0;
        uint64_t end = 0;
        uint64_t time = 0;
        size_t size = 0;

        if (is_correct_filepath(newpath) != 0) {
                SFS_TRACE("Incorrect filename %s", newpath);
                SET_ERRNO(EINVAL);
                return 0;
        }
        
        if ((new_off = check_dirs(fs, (char*) newpath, &entr)) != 0) {
                SFS_TRACE("File dirs %s exist. Offset: %lu", newpath, new_off);
                SET_ERRNO(EEXIST);
                return 0;
        }

        if ((new_off = search_dir(fs, (char*) newpath, &entr)) != 0) {
                SFS_TRACE("Dir %s exist. Offset: %lu", newpath, new_off);
                SET_ERRNO(EEXIST);
                return 0;
        }

        if ((new_off = search_file(fs, (char*) newpath, &entr)) != 0) {
                SFS_TRACE("File %s exist. Offset: %lu", newpath, new_off);
                SET_ERRNO(EEXIST);
                return 0;
        }

        len = strlen(newpath) + 1; /* With '\0' */
        memset(&entr, 0, INDEX_ENTRY_SIZE);
        n = count_entry(len);

        if ((new_off = alloc_entry(fs, &entr, n)) == 0) {
                SFS_TRACE("Not enough space for file %s %d", newpath, n);
                SET_ERRNO(ENOSPC);
                return 0;
        }
        SFS_TRACE("Start: %lX %s %d", new_off, newpath, n);

        if (read_entry(fs->bdev, file, &entr) == -1) {
                SET_ERRNO(EIO);
                return 0;
        }

        if (entr.entry_type != FILE_ENTRY) {
                SET_ERRNO(EINVAL);
                return 0;
        }

        time = AS_FILE(&entr)->time_stamp;
        size = AS_FILE(&entr)->size;
        start = AS_FILE(&entr)->start_block;
        end = AS_FILE(&entr)->end_block;
         
        if (sfs_delete(fs, file) == -1) {
                return 0;
        }

        if (read_entry(fs->bdev , new_off, &entr) == -1) {
                SET_ERRNO(EIO);
                return 0;
        }

        AS_FILE(&entr)->time_stamp = time;
        AS_FILE(&entr)->size = size;
        AS_FILE(&entr)->start_block = start;
        AS_FILE(&entr)->end_block = end;
        AS_FILE(&entr)->entry_type = FILE_ENTRY;
        n--;
        AS_FILE(&entr)->cont_entries = n;
        cur = new_off;
        cur += INDEX_ENTRY_SIZE;
        if (n == 0) {
                strcpy((char*) AS_FILE(&entr)->name, newpath);
        } else {
                memcpy(AS_FILE(&entr)->name, (uint8_t*) newpath, 
                       FIRST_FILE_NAME_SIZE);
                len -= FIRST_FILE_NAME_SIZE;
                newpath += FIRST_FILE_NAME_SIZE;
        }
        write_entry(fs->bdev, new_off, &entr);
        while (n--) {
                SFS_TRACE("Write cont entry %s", newpath);
                strncpy((char*) &entr, newpath, INDEX_ENTRY_SIZE);
                len -= INDEX_ENTRY_SIZE;
                newpath += INDEX_ENTRY_SIZE;
                write_entry(fs->bdev, cur, &entr);
                cur += INDEX_ENTRY_SIZE;
        }

        update(fs);
        SET_ERRNO(0);
        return new_off;
}
