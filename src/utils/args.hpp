
#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace args {

enum arg_type {

    ARG_TYPE_FLAG,

    ARG_TYPE_KEY
};

struct flag;
struct Argument;
struct ArgumentList;

struct flag {
    arg_type type;
    const char* flag;
    const char* description;
};

struct Argument {
    const flag* flag_type;
    const char* value;
    unsigned char mentioned;
    Argument* next;

    Argument(const flag* argument_flag);

    ~Argument();
};

struct ArgumentList {
    Argument* initial;

    ArgumentList(int argc, const char** argv, const flag* arguments);

    ~ArgumentList();

    Argument* search(const flag* flag_to_search);

    Argument* add(const flag* flag_to_search);

    static void print_help(const flag* arguments);
};

extern const char* HELP_HEADER;
extern const flag TYPES[];

} // namespace args
