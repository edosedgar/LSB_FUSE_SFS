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

#include <string.h>

#include <bdev_jsteg/addr_space.h>
#include <stdio.h>

// HAMMING_64 address space
size_t hamming_64_size_transform(size_t size) {
        return size / MSG_SIZE * VEC_SIZE * (sizeof(uint64_t) / sizeof(buf_t));
}

size_t hamming_64_size_rev_transform(size_t size) {
        return size / (VEC_SIZE * (sizeof(uint64_t) / sizeof(buf_t))) * MSG_SIZE;
}

int hamming_64_embed(buf_t* dst, buf_t* src, size_t n) {
        uint64_t* x = (uint64_t*)dst;
        return stego_encode(x, n / sizeof(uint64_t),
                                src, hamming_64_size_rev_transform(n));
}

int hamming_64_extract(buf_t* dst, buf_t* src, size_t n) {
        uint64_t* x = (uint64_t*)src;
        return stego_decode(x, n / sizeof(uint64_t),
                        dst, hamming_64_size_rev_transform(n));
}

// SIMPLE address space
size_t simple_size_transform(size_t size) {
        return size;
}

size_t simple_size_rev_transform(size_t size) {
        return size;
}

int simple_embed(buf_t* dst, buf_t* src, size_t n) {
        memcpy(dst, src, n);

        return 0;
}

int simple_extract(buf_t* dst, buf_t* src, size_t n) {
        memcpy(dst, src, n);

        return 0;
}