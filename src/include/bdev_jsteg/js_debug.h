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

// log levels
#define UNSET (-1)
#define ERROR (0)
#define DEBUG (1)
#define INFO  (2)

#define DEFAULT_LEVEL UNSET

extern int current_level;

#define js_error(fmt, ...) do {                         \
        if (ERROR <= current_level)                     \
                fprintf(stderr, fmt, ##__VA_ARGS__);    \
        } while(0);

#define js_debug(fmt, ...) do {                         \
        if (DEBUG <= current_level)                     \
                fprintf(stderr, fmt, ##__VA_ARGS__);    \
        } while(0);

#define js_info(fmt, ...) do {                          \
        if (INFO <= current_level)                      \
                fprintf(stderr, fmt, ##__VA_ARGS__);    \
        } while(0);

#endif
