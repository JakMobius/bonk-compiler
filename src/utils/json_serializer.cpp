
#include "json_serializer.hpp"

JsonSerializer::JsonSerializer(FILE* file) {
    target = file;
    state.is_first = true;
    state.is_array = false;
    fprintf(target, "{");
    depth = 1;
}

void JsonSerializer::block_string_field(const char* name, const char* value) {
    prepare_next_field();
    escape_string(name);
    fprintf(target, ": ");
    escape_string(value);
}

void JsonSerializer::block_number_field(const char* name, long double value) {
    prepare_next_field();
    escape_string(name);
    fprintf(target, ": %.17Lg", value);
}

void JsonSerializer::block_start_block(const char* name) {
    prepare_next_field();
    escape_string(name);
    fprintf(target, ": {");

    states.push_back(state);
    state.is_first = true;
    state.is_array = false;
    depth++;
}

void JsonSerializer::close_block() {
    depth--;

    if (!state.is_first) {
        fputc('\n', target);
        padding();
    }

    state = states[states.size() - 1];
    states.pop_back();

    fputc('}', target);
}

void JsonSerializer::block_start_array(const char* name) {
    prepare_next_field();
    escape_string(name);
    fprintf(target, ": [");

    states.push_back(state);
    state.is_first = true;
    state.is_array = true;
    depth++;
}

void JsonSerializer::array_add_string(const char* name) {
    prepare_next_field();
    padding();
    escape_string(name);
}

void JsonSerializer::array_add_number(long double value) {
    prepare_next_field();
    padding();
    fprintf(target, "%.17Lg", value);
}

void JsonSerializer::array_add_block() {
    prepare_next_field();
    fprintf(target, "{");

    states.push_back(state);
    state.is_first = true;
    state.is_array = false;
    depth++;
}

void JsonSerializer::array_add_array() {
    prepare_next_field();
    fprintf(target, "[");

    states.push_back(state);
    state.is_first = true;
    state.is_array = true;
    depth++;
}

void JsonSerializer::close_array() {
    depth--;

    if (!state.is_first) {
        fputc('\n', target);
        padding();
    }

    state = states[states.size() - 1];
    states.pop_back();

    fputc(']', target);
}

void JsonSerializer::prepare_next_field() {
    if (!state.is_first)
        fputc(',', target);
    fputc('\n', target);
    padding();
    state.is_first = false;
}

void JsonSerializer::padding() {
    for (int i = 0; i < depth; i++) {
        fputc(' ', target);
        fputc(' ', target);
    }
}

void JsonSerializer::escape_string(const char* string) {
    if (!string)
        fprintf(target, "null");
    else {
        fputc('"', target);
        char c = '\0';
        while ((c = *(string++))) {
            if (c == '\n')
                fputs("\\n", target);
            else if (c == '"')
                fputs("\\", target);
            else
                fputc(c, target);
        }
        fputc('"', target);
    }
}

JsonSerializer::~JsonSerializer() {
    fprintf(target, "}");
    target = nullptr;
}
