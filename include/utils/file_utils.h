#pragma once
#include <stdio.h>

FILE* SmartOpenFileInDirectory(const char* directory, const char* filename,
                               const char* mode);
