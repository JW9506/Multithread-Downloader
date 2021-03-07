#include "utils/file_utils.h"
#ifdef _WIN32
#include <windows.h>
#endif

static FILE* SmartFOpen(const char* filename, const char* mode) {
    FILE* file;
#ifdef _WIN32
    wchar_t w_output_filename[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, filename, -1, w_output_filename, 1024);
    int mode_length = strlen(mode) * sizeof(wchar_t) + 1;
    wchar_t w_mode[mode_length];
    MultiByteToWideChar(CP_UTF8, 0, mode, -1, w_mode, mode_length);
    file = _wfopen(w_output_filename, w_mode);
#else
    file = fopen(filename, mode);
#endif
    return file;
}

static void JoinPath(char* full_filename, const char* directory,
                     const char* filename) {
    strcpy(full_filename, directory);
    size_t d_len = strlen(directory);
    if (directory[d_len - 1] != '/') { strcat(full_filename, "/"); }
    strcat(full_filename, filename);
}

FILE* SmartOpenFileInDirectory(const char* directory, const char* filename,
                               const char* mode) {
    char full_filename[1024] = { 0 };
    JoinPath(full_filename, directory, filename);
    return SmartFOpen(full_filename, mode);
}
