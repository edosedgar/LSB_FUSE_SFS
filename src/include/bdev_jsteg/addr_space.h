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

#ifndef _ADDR_SPACE_
#define _ADDR_SPACE_

#include <bdev_jsteg/defines.h>
#include <bdev_jsteg/hamming.h>

typedef enum {
    SIMPLE_ADDR,
    HAMMING_64_ADDR,
    NONE_ADDR
} addr_transform_t;

typedef size_t (*size_transform_t)(size_t size);
typedef size_t (*size_rev_transform_t)(size_t size);
typedef int    (*mem_embed_t)(buf_t* dst, buf_t* src, size_t n);
typedef int    (*mem_extract_t)(buf_t* dst, buf_t* src, size_t n);

typedef struct {
    addr_transform_t type;
    size_transform_t size_transform;
    size_rev_transform_t size_rev_transform;
    mem_embed_t      mem_embed;
    mem_extract_t    mem_extract;
    size_t           grain_size;
} addr_space_handler_t;

// HAMMING_64 address space
#define HAMMING_GRAIN_SIZE  (VEC_SIZE * sizeof(uint64_t))
size_t hamming_64_size_transform(size_t size);
size_t hamming_64_size_rev_transform(size_t size);
int hamming_64_embed(buf_t* dst, buf_t* src, size_t n);
int hamming_64_extract(buf_t* dst, buf_t* src, size_t n);

// SIMPLE address space
#define SIMPLE_GRAIN_SIZE 1
size_t simple_size_transform(size_t size);
size_t simple_size_rev_transform(size_t size);
int simple_embed(buf_t* dst, buf_t* src, size_t n);
int simple_extract(buf_t* dst, buf_t* src, size_t n);

#endif // _ADDR_SPACE_