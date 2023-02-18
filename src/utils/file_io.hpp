
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

enum file_op_result {
    FILE_OP_READ_ERROR,
    FILE_OP_NOT_ENOUGH_MEMORY,
    FILE_OP_INVALID_ARGUMENT,
    FILE_OP_OK
};

const char* read_file(const char* filepath, file_op_result* result, unsigned long* length);
