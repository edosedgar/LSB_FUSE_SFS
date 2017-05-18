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

/*
 *      JEDECOMPRESS STATE extension 
 */
GLOBAL(void)
jdecompress_create(jdecompress_state* cinfo_ptr, FILE* file)
{
        cinfo_ptr->ci = 0;
        cinfo_ptr->by = 0;
        fseek(file, 0, SEEK_SET);

        jpeg_create_decompress((j_decompress_ptr) cinfo_ptr);

        jpeg_stdio_src((j_decompress_ptr) cinfo_ptr, file);
        (void) jpeg_read_header((j_decompress_ptr) cinfo_ptr, TRUE);

        cinfo_ptr->coeffs = jpeg_read_coefficients((j_decompress_ptr) cinfo_ptr);
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
        		//if (i % 4 == 3) printf("%c", entry->data[i / 4]);
        		//printf("%d ", value);
        		i++;
        	}
        }
/*
        printf("\n");
        for (int j = 0; j < entry->bytes; j++)
        	if (entry->data[j]) printf("%c %d", entry->data[j], j);
*/
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

        // write preamble    	
    	struct preamble_t pr;
    	pr.bytes[0] = START_PR;
    	pr.bytes[3] = END_PR;
    	*(uint16_t*)(pr.bytes + 1) = entry->jindex;

    	int pos = 0;
    	int i = 0;

    	JCOEFPTR it;
    	uint16_t wmask = ~SB_BITMASK;
    	Foreach_VDCT_coeff(it, dinfo) {
    		if (i >= entry->bytes * LSBF) break;

		if (pos < PREAMBLE_SIZE) {
			// write preamble
			*it &= wmask; 
			*it |= ((pr.preamble >> ((pos % PREAMBLE_SIZE) * LSB)) & SB_BITMASK);

        		pos++;
		} else {
			// write data
			*it &= wmask;
			*it |= ((entry->data[i / LSBF] >> ((i % LSBF) * LSB)) & SB_BITMASK);
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
jentry_init(FILE* file, uint16_t* jindex)
{
	jdev_entry* entry = (jdev_entry*) malloc(sizeof(jdev_entry));
    	entry->file = file;

	jdecompress_state cinfo;
    	struct jpeg_error_mgr jerr;
    	cinfo.pb.err = jpeg_std_error(&jerr);

    	jdecompress_create(&cinfo, file);
	
	int size = 0;
    	int pos = 0;
    	struct preamble_t pr;
    	pr.preamble = 0;

    	/* Format of preamble:
 	 * [START_PR][uint16_t jindex][END_PR]
 	 */
    	JCOEFPTR it;
    	Foreach_VDCT_coeff(it, cinfo) {
    		//fprintf(stderr, "%d\n", *it);

    		if (pos < PREAMBLE_SIZE) {
        		pr.preamble |= (*it & SB_BITMASK) 
        			    	<< ((pos % PREAMBLE_SIZE) * LSB);
        		pos++; 
		} else
        		size++;        	
        }

        if (pr.bytes[0] == START_PR && pr.bytes[3] == END_PR) {
        	entry->jindex = *(uint16_t*)(pr.bytes + 1);
                //fprintf(stderr, "jentryinit_count %d\n", entry->jindex);
        } else {
        	entry->jindex = *jindex;
        	(*jindex)--;
        }	
        

        entry->bytes = size / LSBF;
        entry->data = (byte_t*) calloc(entry->bytes, sizeof(byte_t));
        entry->start = 0;

        entry->is_available = 0;

        // methods
        entry->read_data = read_file;
        entry->write_data = write_file;
        entry->jentry_release = jentry_destroy;

        jdecompress_destroy(&cinfo);

	return entry;    	
}