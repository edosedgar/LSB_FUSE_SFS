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

#ifndef _FOREACH_DCT_
#define _FOREACH_DCT_

#include <bdev_jsteg/jstegentry.h>

/*
 * ci between 0 and number of image component
 * by between 0 and compptr->height_in_blocks
 * bx between 0 and compptr->width_in_blocks
 * bi between 0 and 64 (8x8)
 */

inline LOCAL(JBLOCKROW)
next_row(jdecompress_state* cinfo)
{
	cinfo->by++;
	while (cinfo->by < cinfo->pb.comp_info[cinfo->ci].height_in_blocks) {
		cinfo->row_ptrs[cinfo->ci] = (cinfo->pb.mem->access_virt_barray)((j_common_ptr)cinfo,
								cinfo->coeffs[cinfo->ci],
								cinfo->by,
								(JDIMENSION)1,
								FALSE);

		return cinfo->row_ptrs[cinfo->ci][0];
	}

	return NULL;
}

inline LOCAL(JBLOCKROW)
initial_row(jdecompress_state* cinfo)
{
	cinfo->by = 0;
	cinfo->row_ptrs[cinfo->ci] = (cinfo->pb.mem->access_virt_barray)((j_common_ptr)cinfo,
								cinfo->coeffs[cinfo->ci],
								0,
								(JDIMENSION)1,
								FALSE);

	return cinfo->row_ptrs[cinfo->ci][0];
}

inline LOCAL(JCOEFPTR)
initial_block(jdecompress_state* cinfo, JCOEFPTR* it)
{
	cinfo->bx = 0;
	*it = cinfo->row_ptrs[cinfo->ci][0][0];
	return cinfo->row_ptrs[cinfo->ci][0][0];
}

inline LOCAL(JCOEFPTR)
next_block(jdecompress_state* cinfo, JCOEFPTR* it)
{
	cinfo->bx++;

	while (cinfo->bx < cinfo->pb.comp_info[cinfo->ci].width_in_blocks) {
		*it = cinfo->row_ptrs[cinfo->ci][0][cinfo->bx];
		return cinfo->row_ptrs[cinfo->ci][0][cinfo->bx];
	}

	return NULL;
}


/* FOREACH DCT coefficient */

/* iterate over all coeffecient in DCT table */
#define Foreach_DCT_coeff(it, cinfo) \
for (cinfo.ci = 0; cinfo.ci < cinfo.pb.num_components; cinfo.ci++) \
	for (JBLOCKROW row = initial_row(&cinfo); row != NULL; row = next_row(&cinfo)) \
		for (JCOEFPTR blk = initial_block(&cinfo, &it); blk != NULL; blk = next_block(&cinfo, &it)) \
			for (JDIMENSION bi = 1; bi < DCTSIZE2; bi++, it = &blk[bi])
			 //Manipulate DCT coefficients here


/* iterate over coeffecients in DCT table
   which greater than DCT_MIN_VALUE */
#define Foreach_VDCT_coeff(it, cinfo) \
Foreach_DCT_coeff(it, cinfo)  \
	if (*it >= DCT_MIN_VALUE)

#endif // _FOREACH_DCT_