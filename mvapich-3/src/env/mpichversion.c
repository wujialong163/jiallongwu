/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const char MPII_Version_string[];
extern const char MPII_Version_date[];
extern const char MPII_Version_ABI[];
extern const char MPII_Version_configure[];
extern const char MPII_Version_device[];
extern const char MPII_Version_CC[];
extern const char MPII_Version_CXX[];
extern const char MPII_Version_F77[];
extern const char MPII_Version_FC[];
extern const char MPII_Version_custom[];

/* FIXME: We really should consider internationalizing the output from this
   program */
/* style: allow:fprintf:1 sig:0 */
/* style: allow:printf:8 sig:0 */

/*
 * This program reports on properties of the MVAPICH library, such as the
 * version, device, and what patches have been applied.
 *
 * The reason that this program doesn't directly include the info is that it
 * can be compiled and then linked with the MVAPICH library to discover
 * the information about the version of the library.  If built with shared
 * libraries, this will give the information about the currently installed
 * shared library.
 */

typedef enum { Version_number = 0, Date = 1,
    Patches = 2, Configure_args = 3, Device = 4,
    Compilers = 5, Custom = 6, LastField
} fields;

/*D
  mpich2version - Report on the MVAPICH version

  Command Line Arguments:
+ -version - Show the version of MVAPICH
. -date    - Show the release date of this version
. -patches - Show the identifiers for any applied patches
. -configure - Show the configure arguments used to build MVAPICH
- -device  - Show the device for which MVAPICH was configured

  Using this program:
  To use this program, link it against 'libmpi.a' (use 'mpicc' or
  the whichever compiler command is used to create MVAPICH programs)
  D*/

int main(int argc, char *argv[])
{
    int i, flags[10];
    char version[MPI_MAX_LIBRARY_VERSION_STRING];
    int versionlen;

    /* FIXME: this is needed to avoid unresolved symbols issues when building with
     * --disable-weak-symbols on some platforms. Ideally, we could use the output
     * from this call instead of accessing internal library variables.
     */
    MPI_Get_library_version(version, &versionlen);

    if (argc <= 1) {
        /* Show all values */
        for (i = 0; i < LastField; i++)
            flags[i] = 1;
    } else {
        /* Show only requested values */
        for (i = 0; i < LastField; i++)
            flags[i] = 0;
        for (i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-version") == 0 ||
                strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0)
                flags[Version_number] = 1;
            else if (strcmp(argv[i], "-date") == 0 ||
                     strcmp(argv[i], "--date") == 0 || strcmp(argv[i], "-D") == 0)
                flags[Date] = 1;
            else if (strcmp(argv[i], "-patches") == 0)
                flags[Patches] = 1;
            else if (strcmp(argv[i], "-configure") == 0 ||
                     strcmp(argv[i], "--configure") == 0 || strcmp(argv[i], "-c") == 0)
                flags[Configure_args] = 1;
            else if (strcmp(argv[i], "-device") == 0 ||
                     strcmp(argv[i], "--device") == 0 || strcmp(argv[i], "-d") == 0)
                flags[Device] = 1;
            else if (strcmp(argv[i], "-compiler") == 0 ||
                     strcmp(argv[i], "--compiler") == 0 || strcmp(argv[i], "-b") == 0)
                flags[Compilers] = 1;
            else if (strcmp(argv[i], "-custom") == 0 ||
                     strcmp(argv[i], "--custom") == 0 || strcmp(argv[i], "-u") == 0)
                flags[Custom] = 1;
            else {
                fprintf(stderr, "Unrecognized argument %s\n", argv[i]);
                exit(1);
            }
        }
    }

    /* Print out the information, one item per line */
    if (flags[Version_number]) {
	    printf( "MVAPICH Version:     \t%s\n", MPII_Version_string );
    }
    if (flags[Date]) {
	    printf( "MVAPICH Release date:\t%s\n", MPII_Version_date );
    }
    if (flags[Device]) {
	    printf( "MVAPICH Device:      \t%s\n", MPII_Version_device );
    }
    if (flags[Configure_args]) {
	    printf( "MVAPICH configure:   \t%s\n", MPII_Version_configure );
    }
    if (flags[Compilers]) {
	    printf( "MVAPICH CC:  \t%s\n", MPII_Version_CC );
	    printf( "MVAPICH CXX: \t%s\n", MPII_Version_CXX );
	    printf( "MVAPICH F77: \t%s\n", MPII_Version_F77 );
	    printf( "MVAPICH FC:  \t%s\n", MPII_Version_FC );
	}
    if (flags[Custom]) {
        printf("MVAPICH Custom Information: \t%s\n", MPII_Version_custom);
    }

    return 0;
}
