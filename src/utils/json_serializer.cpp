
#include <assert.h>
#include "json_serializer.hpp"

namespace bonk {

JSONSerializer::JSONSerializer(const OutputStream& output_stream) : output_stream(output_stream) {
    state.is_first = true;
    state.is_array = false;
    state.field_name_set = false;
    output_stream.get_stream() << "{";
    depth = 1;
}

JSONStringEscaperStream JSONSerializer::block_string_field() {
    assert(state.field_name_set);
    state.field_name_set = false;
    output_stream.get_stream() << ": ";
    return JSONStringEscaperStream(*this);
}

void JSONSerializer::block_number_field(long double value) {
    assert(state.field_name_set);
    state.field_name_set = false;
    output_stream.get_stream() << ": " << value;
}

void JSONSerializer::block_start_block() {
    assert(state.field_name_set);
    state.field_name_set = false;
    output_stream.get_stream() << ": {";

    states.push_back(state);
    state.is_first = true;
    state.is_array = false;
    depth++;
}

void JSONSerializer::block_add_null() {
    assert(state.field_name_set);
    state.field_name_set = false;
    output_stream.get_stream() << ": null";
}

void JSONSerializer::close_block() {
    assert(!state.field_name_set);
    depth--;

    if (!state.is_first) {
        output_stream.get_stream() << '\n';
        padding();
    }

    state = states.back();
    states.pop_back();

    output_stream.get_stream() << '}';
}

void JSONSerializer::block_start_array() {
    assert(state.field_name_set);
    state.field_name_set = false;
    output_stream.get_stream() << ": [";

    states.push_back(state);
    state.is_first = true;
    state.is_array = true;
    state.field_name_set = false;
    depth++;
}

JSONStringEscaperStream JSONSerializer::array_add_string() {
    assert(state.is_array);
    return JSONStringEscaperStream(*this);
}

void JSONSerializer::array_add_null() {
    assert(state.is_array);
    prepare_next_field();
    padding();
    output_stream.get_stream() << "null";
}

void JSONSerializer::array_add_number(long double value) {
    assert(state.is_array);
    prepare_next_field();
    padding();
    output_stream.get_stream() << value;
}

void JSONSerializer::array_add_block() {
    assert(state.is_array);
    prepare_next_field();
    output_stream.get_stream() << '{';

    states.push_back(state);
    state.is_first = true;
    state.is_array = false;
    depth++;
}

void JSONSerializer::array_add_array() {
    assert(state.is_array);
    prepare_next_field();
    output_stream.get_stream() << '[';

    states.push_back(state);
    state.is_first = true;
    state.is_array = true;
    depth++;
}

void JSONSerializer::close_array() {
    assert(state.is_array);
    depth--;

    if (!state.is_first) {
        output_stream.get_stream() << '\n';
        padding();
    }

    state = states[states.size() - 1];
    states.pop_back();

    output_stream.get_stream() << ']';
}

void JSONSerializer::prepare_next_field() {
    assert(!state.field_name_set);
    if (!state.is_first)
        output_stream.get_stream() << ",\n";
    else
        output_stream.get_stream() << '\n';

    padding();
    state.is_first = false;
}

void JSONSerializer::padding() const {
    for (int i = 0; i < depth; i++) {
        output_stream.get_stream() << "  ";
    }
}

JSONSerializer::~JSONSerializer() {
    output_stream.get_stream() << '\n';
}

JSONSerializer& JSONSerializer::field(std::string_view name) {
    prepare_next_field();
    state.field_name_set = true;
    JSONStringEscaperStream(*this) << name;
    return *this;
}

void JSONStringEscaperStream::flush() const {
    while (true) {
        int c = serializer.ss.get();
        if (c == EOF)
            break;
        if (c == '\n')
            serializer.output_stream.get_stream() << "\\n";
        else if (c == '"')
            serializer.output_stream.get_stream() << "\\\"";
        else
            serializer.output_stream.get_stream() << (char)c;
    }
    serializer.ss.clear();
}

JSONStringEscaperStream::JSONStringEscaperStream(JSONSerializer& serializer)
    : serializer(serializer) {
    serializer.output_stream.get_stream() << '"';
}

JSONStringEscaperStream::~JSONStringEscaperStream() {
    flush();
    serializer.output_stream.get_stream() << '"';
}

} // namespace bonk