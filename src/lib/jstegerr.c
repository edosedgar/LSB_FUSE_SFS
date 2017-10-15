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

//#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

#include <bdev_jsteg/jstegerr.h>

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
	// cinfo->err really points to a jerror_mgr struct
	jerror_ptr myerr = (jerror_ptr) cinfo->err;

	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
  	longjmp(myerr->setjmp_buffer, 1);
}


GLOBAL(int) 
jerr_init(j_common_ptr cinfo, jerror_ptr jerr)
{
	cinfo->err = jpeg_std_error(&(jerr->pub));
	jerr->pub.error_exit = my_error_exit;

  	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr->setjmp_buffer)) {
		return JPEG_ERROR;
	}

	return 0;
}