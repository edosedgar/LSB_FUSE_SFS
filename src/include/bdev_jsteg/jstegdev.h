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

#ifndef _JSTEGDEV_
#define _JSTEGDEV_

//#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include <bdev_jsteg/jstegerr.h>
#include <bdev_jsteg/jstegentry.h>
#include "blockdev.h"

typedef struct filedev_data_t {
		char* dirname;
		jdev_entry* entries;
		DIR* dir;
		size_t jfile_num;
} filedev_data;


blockdev* jstegdev_create(blockdev* bdev, filedev_data* fdev,
                         size_t block_size);
#ifndef FILEDEV_DEBUG
void filedev_dump(blockdev* bdev, size_t block_size);
#endif

uint64_t get_time();

#endif // _JSTEGDEV_