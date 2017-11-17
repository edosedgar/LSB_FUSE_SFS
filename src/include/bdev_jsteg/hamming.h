/*
<FUSE-based implementation of SFS (Simple File System)>
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

#ifndef _HAMMING_H_
#define _HAMMING_H_

#include <stdint.h>

// Hamming code (15, 11)
#define HAMMING_LENGTH 15
#define HAMMING_K      4
uint16_t hamming_encode(uint16_t x, uint8_t m);
uint8_t  hamming_decode(uint16_t x);

// Hamming code (63, 57)
#define HAMMING_64_LENGTH 63
#define HAMMING_64_K      6
uint64_t hamming_64_encode(uint64_t x, uint8_t m);
uint8_t  hamming_64_decode(uint64_t x);

// There are two embeding chanels.
// 1) hamming codes
// 2) xor of all bits
#define VEC_SIZE 8
#define MSG_SIZE 7
int stego_encode(uint64_t x[], size_t vec_size, uint8_t msg[], size_t msg_size);
int stego_decode(uint64_t x[], size_t vec_size, uint8_t msg[], size_t msg_size);

#endif