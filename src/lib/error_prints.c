/*
<FUSE-based implementation of SFS (Simple File System)>

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

#include "config.h"

#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "generic/error_prints.h"

#ifndef HAVE_PROGRAM_INVOCATION_NAME
extern char *program_invocation_name;
#endif

static void
verror_msg(int err_no, const char *fmt, va_list p)
{
        char *msg;

        fflush(NULL);

        msg = NULL;
        if (vasprintf(&msg, fmt, p) >= 0) {
                if (err_no)
                        fprintf(stderr, "%s: %s: %s\n",
                                program_invocation_name, msg,
                                strerror(err_no));
                else
                        fprintf(stderr, "%s: %s\n",
                                program_invocation_name, msg);
                free(msg);
        } else {
                /* malloc in vasprintf failed, try it without malloc */
                fprintf(stderr, "%s: ", program_invocation_name);
                vfprintf(stderr, fmt, p);
                if (err_no)
                        fprintf(stderr, ": %s\n", strerror(err_no));
                else
                        putc('\n', stderr);
        }
}

void
error_msg(const char *fmt, ...)
{
        va_list p;
        va_start(p, fmt);
        verror_msg(0, fmt, p);
        va_end(p);
}

void
error_msg_and_die(const char *fmt, ...)
{
        va_list p;
        va_start(p, fmt);
        verror_msg(0, fmt, p);
        die();
}

void
error_msg_and_help(const char *fmt, ...)
{
        if (fmt != NULL) {
                va_list p;
                va_start(p, fmt);
                verror_msg(0, fmt, p);
        }
        fprintf(stderr, "Try '%s -h' for more information.\n",
                program_invocation_name);
        die();
}

void
perror_msg(const char *fmt, ...)
{
        va_list p;
        va_start(p, fmt);
        verror_msg(errno, fmt, p);
        va_end(p);
}

void
perror_msg_and_die(const char *fmt, ...)
{
        va_list p;
        va_start(p, fmt);
        verror_msg(errno, fmt, p);
        die();
}
