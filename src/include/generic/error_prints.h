/*
<FUSE-based implementation of SFS (Simple File System)>
This file contains error printing functions.
These functions can be used by various binaries included in the strace
package.  Variable 'program_invocation_name' and function 'die()'
have to be defined globally.

    Copyright (C) 2017  <Edgar Kaziakhmedov>

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

#ifndef SFS_ERROR_PRINTS_H
#define SFS_ERROR_PRINTS_H

void die(void);

void error_msg(const char *fmt, ...);
void perror_msg(const char *fmt, ...);
void perror_msg_and_die(const char *fmt, ...);
void error_msg_and_help(const char *fmt, ...);
void error_msg_and_die(const char *fmt, ...);

#endif
