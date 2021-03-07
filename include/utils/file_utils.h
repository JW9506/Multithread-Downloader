#pragma once
#include <stdio.h>

void JoinPath(char* destination, const char* path1, const char* path2);

FILE* SmartOpenFileInDirectory(const char* directory, const char* filename,
                               const char* mode);
