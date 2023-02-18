#pragma once

#include <cstdio>
#include <vector>

struct json_serializer_state {
    bool is_array;
    bool is_first;
};

struct json_serializer {
    FILE* target;
    int depth;
    json_serializer_state state;
    std::vector<json_serializer_state> states;

    json_serializer(FILE* file);

    ~json_serializer();

    void block_string_field(const char* name, const char* value);

    void block_number_field(const char* name, long double value);

    void block_start_block(const char* name);

    void block_start_array(const char* name);

    void close_block();

    void array_add_string(const char* name);

    void array_add_number(long double name);

    void array_add_block();

    void array_add_array();

    void close_array();

    void padding();

    void escape_string(const char* string);

    void prepare_next_field();
};
