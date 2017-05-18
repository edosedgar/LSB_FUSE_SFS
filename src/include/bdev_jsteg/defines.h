/*
<FUSE-based implementation of SFS (Simple File System)>
    Copyright (C) 2016  <Grigoriy Melnikov>

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

#ifndef _BDEV_DEFINES_
#define _BDEV_DEFINES_

#include <stdint.h>
#include <stddef.h>

typedef uint8_t  buf_t;
typedef size_t   bnum_t;


#define JMAX_FILE 	(1<<16) // max number of file
#define DCT_MIN_VALUE 	(1<<3) // min value of DCT coeff 

#define LSB 		(1<<1) // number of mutable bits, 1 or 2
#define LSBF 		(8 / LSB) // LSB frequency
#define SB_BITMASK 	((1<<LSB)-1) // 0x3 - 2 significant bits, 
				// 0x1 - 1 bit
#define PREAMBLE_SIZE 	(32 / LSB) // preamble size = 4 x 4 (2 bits)

#define START_PR 	0xFF
#define END_PR 		0x00

#endif // _BDEV_DEFINES_