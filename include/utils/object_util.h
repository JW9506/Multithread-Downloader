#pragma once
#include <stdlib.h>

#define CREATE_OBJECT(type, variable) type* variable = malloc(sizeof(type))
#define CREATE_OBJECT_CLEANED(type, variable)                                  \
    type* variable = calloc(1, sizeof(type))

uint64_t TimeInMillisecond();

void sleepms(unsigned int ms);
