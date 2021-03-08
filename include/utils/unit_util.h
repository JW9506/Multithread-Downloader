#pragma once
#include <stdint.h>

void FormatUnit(char* dest, const char* unit, int64_t bytes);

void FormatByte(char* dest, int64_t byte_count);

void FormatByteRate(char* dest, int64_t byte_rate);

void FormatTime(char* dest, int total_seconds);
