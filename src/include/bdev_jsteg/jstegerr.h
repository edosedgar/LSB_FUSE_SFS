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


#ifndef _JSTEGERR_
#define _JSTEGERR_

#include <jpeglib.h>
#include <setjmp.h>

/*
 *  extended error handler struct
 */
struct jerror_mgr {
	struct jpeg_error_mgr pub;

	jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct jerror_mgr* jerror_ptr;

GLOBAL(int) 
jerr_init(j_common_ptr cinfo, jerror_ptr jerr); 

#endif // _JSTEGERR_