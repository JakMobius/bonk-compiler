
#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>

enum FileOpResult {
    FILE_OP_READ_ERROR,
    FILE_OP_NOT_ENOUGH_MEMORY,
    FILE_OP_INVALID_ARGUMENT,
    FILE_OP_OK
};

const char* read_file(const char* filepath, FileOpResult* result, unsigned long* length);
