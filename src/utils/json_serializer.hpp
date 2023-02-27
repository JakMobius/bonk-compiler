#pragma once

#include <cstdio>
#include <vector>

struct JsonSerializerState {
    bool is_array;
    bool is_first;
};

struct JsonSerializer {
    FILE* target;
    int depth;
    JsonSerializerState state{};
    std::vector<JsonSerializerState> states;

    explicit JsonSerializer(FILE* file);

    ~JsonSerializer();

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

    void padding() const;

    void escape_string(const char* string) const;

    void prepare_next_field();
};
