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

#ifndef _JSTEGENTRY_
#define _JSTEGENTRY_

#include <stdio.h>

#include <jpeglib.h>
#include <bdev_jsteg/defines.h>

typedef uint8_t byte_t;

struct preamble_t {
	union {
		uint32_t preamble;
		byte_t bytes[4];
	};
};


// JDECOMPRESS STATE extension

typedef struct jdecompress_state_t {
	struct jpeg_decompress_struct pb;

	JDIMENSION ci;
	JDIMENSION by;
	JDIMENSION bx;
	jvirt_barray_ptr* coeffs;
	JBLOCKARRAY row_ptrs[MAX_COMPONENTS];
} jdecompress_state;

GLOBAL(int)
jdecompress_create(jdecompress_state* cinfo_ptr, FILE* file);

GLOBAL(void)
jdecompress_destroy(jdecompress_state* cinfo_ptr);


typedef struct jsteg_entry_t {
        int32_t jindex;
        FILE* file;

        size_t start;
        size_t bytes;

        int is_available;

        byte_t* data;

        void (*read_data) (struct jsteg_entry_t* jentry);
        void (*write_data) (struct jsteg_entry_t* jentry);
        void (*jentry_release) (struct jsteg_entry_t* jentry);
        void (*write_preamble) (struct jsteg_entry_t* jentry);
        int (*read_preamble) (struct jsteg_entry_t* jentry);
} jdev_entry;

GLOBAL(jdev_entry*)
jentry_init(FILE* file);

#endif // _JSTEGENTRY_