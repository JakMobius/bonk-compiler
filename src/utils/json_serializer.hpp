#pragma once

#include <cstdio>
#include <sstream>
#include <vector>
#include "streams.hpp"

namespace bonk {

class JSONStringEscaperStream;

struct JsonSerializerState {
    bool is_array;
    bool is_first;
    bool field_name_set;
};

struct JSONSerializer {
    const OutputStream& output_stream;
    mutable std::stringstream ss;
    int depth;
    JsonSerializerState state{};
    std::vector<JsonSerializerState> states;

    explicit JSONSerializer(const OutputStream& output_stream);

    ~JSONSerializer();

    JSONStringEscaperStream block_string_field();

    void block_number_field(long double value);

    void block_start_block();

    void block_start_array();

    void block_add_null();

    void close_block();

    JSONStringEscaperStream array_add_string();

    void array_add_null();

    void array_add_number(long double name);

    void array_add_block();

    void array_add_array();

    void close_array();

    void padding() const;

    void prepare_next_field();

    JSONSerializer& field(std::string_view name);
};

class JSONStringEscaperStream {
  public:
    JSONSerializer& serializer;

    explicit JSONStringEscaperStream(JSONSerializer& serializer);;

    ~JSONStringEscaperStream();

    void flush() const;
};

template <typename T>
const JSONStringEscaperStream& operator<<(const JSONStringEscaperStream& escaper, T&& value) {
    escaper.serializer.ss << value;
    escaper.flush();
    return escaper;
}

} // namespace bonk
