/**
 * @file mypopen.h
 * BES - header file for the mypopen.c function
 * Projekt 2
 *
 * Gruppe 13
 *
 * @author Binder Patrik         <ic19b030@technikum-wien.at>
 * @author Pittner Stefan        <ic19b003@technikum-wien.at>
 * @author Ferchenbauer Nikolaus <ic19b013@technikum-wien.at>
 * @date 2020/05/4
 *
 * @version 1.x
 *
 */

/**
 * -------------------------------------------------------------- includes --
 */

#ifndef MYPOPEN_H
#define MYPOPEN_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * -------------------------------------------------------------- functions --
 */

FILE *mypopen(const char *const command, const char *const type);
int mypclose(FILE *stream);
void child_process(int *fd, const char *const type, const char *const command);
void parent_process(int *fd, const char *const type);
void reset(void);


#endif

/**
 * end
 */
