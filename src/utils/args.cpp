
#include "args.hpp"

namespace args {

Argument::Argument(const flag* argument_flag) {
    flag_type = argument_flag;
    value = nullptr;
    next = nullptr;
    mentioned = 0;
}

Argument::~Argument() {
    delete next;
}

const flag* find_argument(const flag* arguments, const char* string) {
    const flag* argument = arguments;
    while (argument->flag) {
        if (strcmp(argument->flag, string) == 0) {
            return argument;
        }

        argument++;
    }

    return nullptr;
}

Argument* ArgumentList::add(const flag* flag_to_search) {
    Argument* current_argument = initial;

    if (current_argument == nullptr) {
        initial = new Argument(flag_to_search);
        return initial;
    }

    while (true) {
        if (current_argument->flag_type != flag_to_search) {
            if (current_argument->next == nullptr) {
                current_argument->next = new Argument(flag_to_search);
                return current_argument->next;
            } else {
                current_argument = current_argument->next;
            }
        } else {
            return current_argument;
        }
    }
}

ArgumentList::ArgumentList(int argc, const char** argv, const flag* arguments) {
    const flag* current_flag = nullptr;

    initial = nullptr;

    for (int i = 0; i < argc; i++) {
        const char* arg = argv[i];

        if (current_flag) {
            Argument* arg_view = add(current_flag);
            if (!arg_view) {
                return;
            }

            arg_view->mentioned++;
            arg_view->value = arg;
            current_flag = nullptr;
            continue;
        }

        if (*arg == '\0')
            continue;
        if (*arg == '-') {
            const flag* found_flag = find_argument(arguments, arg + 1);

            if (found_flag == nullptr) {
                printf("Unknown argument: %s\n", arg);
                continue;
            }

            if (found_flag->type == ARG_TYPE_KEY) {
                current_flag = found_flag;
            } else {
                Argument* new_argument = add(found_flag);
                if (!new_argument) {
                    return;
                }

                new_argument->mentioned++;
            }
        }
    }

    if (current_flag != nullptr) {
        printf("Flag '%s' requires value\n", current_flag->flag);
    }
}

ArgumentList::~ArgumentList() {
    delete initial;
}

Argument* ArgumentList::search(const flag* flag_to_search) {
    Argument* arg_view = initial;

    if (arg_view == nullptr)
        return nullptr;

    while (true) {
        if (arg_view->flag_type == flag_to_search) {
            return arg_view;
        } else {
            if (arg_view->next == nullptr) {
                return nullptr;
            } else {
                arg_view = arg_view->next;
            }
        }
    }
}

void ArgumentList::print_help(const flag* arguments) {
    printf("%s\nCommand line flags:\n", HELP_HEADER);

    int max_flag_length = 0;

    const flag* argument = arguments;
    while (argument->flag) {

        int flag_length = (int)strlen(argument->flag);

        if (flag_length > max_flag_length) {
            max_flag_length = flag_length;
        }

        argument++;
    }

    argument = arguments;

    while (argument->flag) {

        int spacing = (int)(max_flag_length - strlen(argument->flag) + 2);

        printf(" -%s%*c%s\n", argument->flag, spacing, ' ', argument->description);

        argument++;
    }
}

} // namespace args
