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

#ifndef __JSTEG_DEBUG__
#define __JSTEG_DEBUG__

#include <stdio.h>

#include <generic/error_prints.h>

#define js_error(fmt, ...) \
        print_on_level(ERROR, fmt, ##__VA_ARGS__)

#define js_debug(fmt, ...) \
        print_on_level(DEBUG, fmt, ##__VA_ARGS__)

#define js_info(fmt, ...) \
        print_on_level(INFO, fmt, ##__VA_ARGS__)

#endif