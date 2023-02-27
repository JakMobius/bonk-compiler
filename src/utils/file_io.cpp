
#include "file_io.hpp"

const char* read_file(const char* filepath, FileOpResult* result, unsigned long* length) {
    struct stat filedata = {};

    if (filepath == nullptr) {
        if (result != nullptr) {
            *result = FILE_OP_INVALID_ARGUMENT;
        }
        return nullptr;
    }

    if (stat(filepath, &filedata) == -1) {
        if (result != nullptr) {
            *result = FILE_OP_READ_ERROR;
        }
        return nullptr;
    }

    unsigned long file_length = filedata.st_size + 1;

    char* buffer = (char*)malloc(file_length);

    if (!buffer) {
        if (result != nullptr) {
            *result = FILE_OP_NOT_ENOUGH_MEMORY;
        }
        return nullptr;
    }

    if (length != nullptr) {
        *length = file_length;
    }

    FILE* input = fopen(filepath, "rb");
    size_t readen = fread(buffer, 1, file_length, input);
    buffer[readen] = '\0';
    fclose(input);

    if (result != nullptr) {
        *result = FILE_OP_OK;
    }

    return buffer;
}
