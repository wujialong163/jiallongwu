#ifndef MPIRUN_TIMESTAMP_H
#define MPIRUN_TIMESTAMP_H 1

#include <stdio.h>

int mvp_take_timestamp_mpirun(const char *label);
int mvp_print_timestamps(FILE *fd);

#endif
