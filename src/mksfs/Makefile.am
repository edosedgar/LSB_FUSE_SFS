bin_PROGRAMS = mksfs
mksfs_SOURCES = main.c

mksfs_LDADD = -lm -L$(top_srcdir)/src/lib -L$(top_srcdir)/src/lib/sfs \
	      -lmksfs -lblock -lsfs -ljpeg -lerror_prints
AM_CFLAGS = -Wall -Werror -I$(top_srcdir)/src/include


