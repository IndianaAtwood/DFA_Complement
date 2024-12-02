#ifndef __MAIN_H__

#define __MAIN_H__

#include <regex.h>
#include "cJSON.h"
#include "charList.h"

void readDFA(cJSON *, cJSON **, char **);
CharList* verify(cJSON **, char **);
void regexCheck(regex_t *, char *);
void complement(CharList *, cJSON **);
void printDFA(FILE *, cJSON **, char **);

#endif
