#ifndef MYPOPEN_H
#define MYPOPEN_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


FILE *mypopen(const char *const command, const char *const type);
int mypclose(FILE *stream);
void child_process(int *fd, const char *const type, const char *const command);
void parent_process(int *fd, const char *const type);


#endif