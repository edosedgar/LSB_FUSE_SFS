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

#include <stdlib.h>
#include <strings.h>

#include <bdev_jsteg/jstegentry.h>
#include <bdev_jsteg/foreach.h>
#include <bdev_jsteg/jstegerr.h>

/*
 *      JEDECOMPRESS STATE extension
 */
GLOBAL(int)
jdecompress_create(jdecompress_state* cinfo_ptr, FILE* file)
{
        cinfo_ptr->ci = 0;
        cinfo_ptr->by = 0;
        fseek(file, 0, SEEK_SET);

        struct jerror_mgr jerr;
        (void) jerr_init((j_common_ptr) cinfo_ptr, &jerr);

        jpeg_create_decompress((j_decompress_ptr) cinfo_ptr);

        jpeg_stdio_src((j_decompress_ptr) cinfo_ptr, file);
        if (jpeg_read_header((j_decompress_ptr) cinfo_ptr, TRUE) ==
                JPEG_ERROR) {
                        jpeg_destroy_decompress((j_decompress_ptr) cinfo_ptr);
                        return -1;
                }

        cinfo_ptr->coeffs = jpeg_read_coefficients(
                                (j_decompress_ptr) cinfo_ptr);

        return 0;
}

GLOBAL(void)
jdecompress_destroy(jdecompress_state* cinfo_ptr)
{
        jpeg_finish_decompress((j_decompress_ptr) cinfo_ptr);
        jpeg_destroy_decompress((j_decompress_ptr) cinfo_ptr);
}

/*
 *      jsteg_entry_t methods
 */

METHODDEF(int)
read_preamble(jdev_entry* entry)
{
        // initialize decompressor
        jdecompress_state dinfo;
        jdecompress_create(&dinfo, entry->file);

        // only last LSB bits of DCT coeff are used
        int pos = 0;
        struct preamble_t pr;
        pr.preamble = 0;

        JCOEFPTR it;
        Foreach_VDCT_coeff(it, dinfo) {
                if (pos < PREAMBLE_SIZE) {
                        pr.preamble |= (*it & SB_BITMASK)
                                        << ((pos % PREAMBLE_SIZE) * LSB);
                        pos++;
                }
                break;
        }

        int jindex = PREAMBLE_INDEX(pr);
        fprintf(stderr, "start %d end %d\n", pr.bytes[0], pr.bytes[3]);
        fprintf(stderr, "read_preamble %d\n", *(uint16_t*)((pr).bytes + 1));

        jdecompress_destroy(&dinfo);

        return jindex;
}

METHODDEF(void)
read_file(jdev_entry* entry)
{
        struct jpeg_error_mgr jerr;

        // initialize decompressor
        jdecompress_state dinfo;
        dinfo.pb.err = jpeg_std_error(&jerr);
        jdecompress_create(&dinfo, entry->file);

        // only last LSB bits of DCT coeff are used
        int i = 0;      
        int pos = 0;
        byte_t value;
        JCOEFPTR it;
        bzero(entry->data, entry->bytes);
        Foreach_VDCT_coeff(it, dinfo) {
                if (i >= entry->bytes * LSBF)
                        break;

                if (pos < PREAMBLE_SIZE)
                //skip preamble
                        pos++;
                else {
                        value = *it & SB_BITMASK;
                        entry->data[i / LSBF] |= value << ((i % LSBF) * LSB);
                        i++;
                }
        }
        jdecompress_destroy(&dinfo);
}

METHODDEF(void)
write_preamble(jdev_entry* entry)
{
        struct jpeg_error_mgr jerr;

        // initialize compressor
        struct jpeg_compress_struct cinfo;

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);
        jpeg_stdio_dest(&cinfo, entry->file);

        // initialize decompressor
        jdecompress_state dinfo;
        dinfo.pb.err = jpeg_std_error(&jerr);
        jdecompress_create(&dinfo, entry->file);

        // write preamble
        struct preamble_t pr;
        PREAMBLE_INIT(pr, entry->jindex);
        fprintf(stderr, "PREAMBLE_INIT: %d\n", (int)pr.bytes[0]);

        int pos = 0;

        JCOEFPTR it;
        uint16_t wmask = ~SB_BITMASK;
        // write preamble
        Foreach_VDCT_coeff(it, dinfo) {
                if (pos < PREAMBLE_SIZE) {
                        *it &= wmask;
                        *it |= ((pr.preamble >> ((pos % PREAMBLE_SIZE) * LSB))
                                & SB_BITMASK);

                        pos++;
                }

                break;
        }

        jpeg_copy_critical_parameters((j_decompress_ptr)&dinfo, &cinfo);

        fseek(entry->file, 0, SEEK_SET);
        jpeg_write_coefficients(&cinfo, dinfo.coeffs);

        // destroy compressor
        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);

        // destroy decompressor
        jdecompress_destroy(&dinfo);
}

METHODDEF(void)
write_file(jdev_entry* entry)
{
        struct jpeg_error_mgr jerr;

        // initialize compressor
        struct jpeg_compress_struct cinfo;

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);
        jpeg_stdio_dest(&cinfo, entry->file);

        // initialize decompressor
        jdecompress_state dinfo;
        dinfo.pb.err = jpeg_std_error(&jerr);
        jdecompress_create(&dinfo, entry->file);

        int pos = 0;
        int i = 0;

        JCOEFPTR it;
        uint16_t wmask = ~SB_BITMASK;
        Foreach_VDCT_coeff(it, dinfo) {
                if (i >= entry->bytes * LSBF) break;

                if (pos < PREAMBLE_SIZE) {
                        // skip preamble
                        pos++;
                } else {
                        // write data
                        *it &= wmask;
                        *it |= ((entry->data[i / LSBF] >> ((i % LSBF) * LSB))
                                & SB_BITMASK);
                        i++;
                }
        }

        jpeg_copy_critical_parameters((j_decompress_ptr)&dinfo, &cinfo);

        fseek(entry->file, 0, SEEK_SET);
        jpeg_write_coefficients(&cinfo, dinfo.coeffs);

        // destroy compressor
        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);

        // destroy decompressor
        jdecompress_destroy(&dinfo);
}

METHODDEF(void)
jentry_destroy(jdev_entry* jentry) {
        free(jentry->data);
        fclose(jentry->file);
}

GLOBAL(jdev_entry*)
jentry_init(FILE* file)
{
        jdev_entry* entry = (jdev_entry*) malloc(sizeof(jdev_entry));
        entry->file = file;

        jdecompress_state cinfo;

        if (jdecompress_create(&cinfo, file) == -1) {
                return NULL;
        }

        int size = 0;
        int pos = 0;

        JCOEFPTR it;
        Foreach_VDCT_coeff(it, cinfo) {
                if (pos < PREAMBLE_SIZE) {
                        // skip preamble
                        pos++;
                } else
                        size++;
        }

        entry->bytes = size / LSBF;
        entry->data = (byte_t*) malloc(entry->bytes * sizeof(byte_t));
        entry->start = 0;

        entry->is_available = 0;

        // methods
        entry->read_data      = read_file;
        entry->read_preamble  = read_preamble;
        entry->write_data     = write_file;
        entry->write_preamble = write_preamble;
        entry->jentry_release = jentry_destroy;

        jdecompress_destroy(&cinfo);

        return entry;
}
