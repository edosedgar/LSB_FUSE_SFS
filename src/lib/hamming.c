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

#include <stdio.h>
#include <errno.h>

#include "bdev_jsteg/hamming.h"

/* Hamming matrix for (15, 11) */
const uint16_t H_MATRIX[HAMMING_K] = {
        0x00FF,
        0x0F0F,
        0x3333,
        0x5555
};

/* Hamming matrix for (63, 57) */
const uint64_t H_MATRIX64[HAMMING_64_K] = {
        0x00000000FFFFFFFF,
        0x0000FFFF0000FFFF,
        0x00FF00FF00FF00FF,
        0x0F0F0F0F0F0F0F0F,
        0x3333333333333333,
        0x5555555555555555
};

static uint8_t even_odd_parity(uint32_t x) {
        x ^= x >> 16;
        x ^= x >> 8;
        x ^= x >> 4;
        x ^= x >> 2;
        x ^= x >> 1;

        return !((~x) & 1);
}

static uint8_t even_odd_parity_64(uint64_t x) {
        x ^= x >> 32;
        x ^= x >> 16;
        x ^= x >> 8;
        x ^= x >> 4;
        x ^= x >> 2;
        x ^= x >> 1;

        return !((~x) & 1);
}

int stego_encode(uint64_t x[], size_t vec_size,
                 uint8_t msg[], size_t msg_size) {
        if (vec_size % VEC_SIZE) {
                errno = EINVAL;
                return -1;
        }

        if (msg_size % MSG_SIZE) {
                errno = EINVAL;
                return -1;
        }

        if (msg_size / MSG_SIZE != vec_size / VEC_SIZE) {
                errno = EINVAL;
                return -1;
        }
        // in each message only 7 significant bits [6:0]
        uint8_t message;
        uint8_t mask;
        uint8_t prev_bits, next_bits;
        uint8_t val_parity_bit, msg_parity_bit;
        uint8_t parity;
        uint64_t encoded;
        for (int j = 0; j < vec_size / VEC_SIZE; j++) {
                mask = 0;
                prev_bits = 0;
                next_bits = 0;
                val_parity_bit = 0;
                msg_parity_bit = 0;
                parity = 0;
                encoded = 0;

                for (int i = 0; i < VEC_SIZE; i++) {
                        mask |= (1 << i);
                        prev_bits = next_bits;
                        if (i < MSG_SIZE) {
                                message = (msg[i] >> (i + 1));
                                next_bits = msg[i] & mask;
                        } else {
                                message = 0;
                        }
                        message |= prev_bits << (HAMMING_64_K - i + 1);
                        //fprintf(stderr, "message: %u\n", message);

                        val_parity_bit = (x[i] & ((uint64_t)1 << HAMMING_64_LENGTH))
                                                >> HAMMING_64_LENGTH;
                        x[i] &= ~((uint64_t)1 << HAMMING_64_LENGTH);
                        msg_parity_bit = (message & (1 << HAMMING_64_K))
                                                >> HAMMING_64_K;
                        message &= ((1 << HAMMING_64_K) - 1);
                        encoded = hamming_64_encode(x[i], message);
                        parity = (even_odd_parity_64(encoded) ^ val_parity_bit);
                        if (msg_parity_bit != parity) {
                                val_parity_bit ^= 1;
                        }
                        x[i] = 0;
                        x[i] |= ((uint64_t)val_parity_bit << HAMMING_64_LENGTH);
                        x[i] |= encoded;
                }

                x += VEC_SIZE;
                msg += MSG_SIZE;
        }

        return 0;
}

int stego_decode(uint64_t x[], size_t vec_size, uint8_t msg[], size_t msg_size) {
        if (vec_size % VEC_SIZE) {
                errno = EINVAL;
                return -1;
        }

        if (msg_size % MSG_SIZE) {
                errno = EINVAL;
                return -1;
        }

        if (msg_size / MSG_SIZE != vec_size / VEC_SIZE) {
                errno = EINVAL;
                return -1;
        }

        uint8_t msg_parity_bit = 0;
        uint8_t message, prev_message;
        uint8_t mask;
        uint8_t next_message;
        uint64_t tmp_value;
        for (int j = 0; j < vec_size / VEC_SIZE; j++) {
                prev_message = 0;
                mask = 0;
                next_message = 0;
                tmp_value = 0;
                for (int i = 0; i < VEC_SIZE; i++) {
                        msg_parity_bit = even_odd_parity_64(x[i]);
                        tmp_value = (x[i] & ~((uint64_t)1 << HAMMING_64_LENGTH));
                        prev_message = next_message;
                        message = hamming_64_decode(tmp_value);
                        message |= (msg_parity_bit << HAMMING_64_K);
                        next_message = message;
                        //fprintf(stderr, "message_decode: %u\n", message);
                        if (i != 0) {
                                msg[i - 1] = (prev_message << i);
                                mask |= (1 << (HAMMING_64_K - i + 1));
                                msg[i - 1] |= (message & mask) >> (HAMMING_64_K - i + 1);
                        }
                }

                msg += MSG_SIZE;
                x += VEC_SIZE;
        }

        return 0;
}

uint16_t hamming_encode(uint16_t x, uint8_t m) {
        uint32_t value = 0;
        uint8_t  pos = 0;

        for (int i = 0; i < HAMMING_K; i++) {
                value = (H_MATRIX[i] & x);
                pos |= (even_odd_parity(value) << (HAMMING_K - i - 1));
        }
        pos ^= m;
        x ^= (1 << (HAMMING_LENGTH - pos));
        return x;
}


uint8_t hamming_decode(uint16_t x) {
        uint32_t value = 0;
        uint8_t msg = 0;

        for (int i = 0; i < HAMMING_K; i++) {
                value = (H_MATRIX[i] & x);
                msg |= (even_odd_parity(value) << (HAMMING_K - i - 1));
        }

        return msg;
}

uint64_t hamming_64_encode(uint64_t x, uint8_t m) {
        uint64_t value = 0;
        uint8_t  pos = 0;

        for (int i = 0; i < HAMMING_64_K; i++) {
                value = (H_MATRIX64[i] & x);
                pos |= (even_odd_parity_64(value) << (HAMMING_64_K - i - 1));
        }
        pos ^= m;
        if (!pos)
                return x;

        x ^= ((uint64_t)1 << (HAMMING_64_LENGTH - pos));
        return x;
}

uint8_t hamming_64_decode(uint64_t x) {
        uint64_t value = 0;
        uint8_t msg = 0;

        for (int i = 0; i < HAMMING_64_K; i++) {
                value = (H_MATRIX64[i] & x);
                msg |= (even_odd_parity_64(value) << (HAMMING_64_K - i - 1));
        }

        return msg;
}