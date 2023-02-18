
#include "errors.hpp"
#include "compiler.hpp"
#include "parsing/lexic/lexical_analyzer.hpp"

namespace bonk {

void compiler::warning_positioned(parser_position* pos, const char* format, ...) {
    if (config->error_file == nullptr)
        return;
    va_list ap;

    va_start(ap, format);
    fprintf(config->error_file, "%s:%lu:%lu: warning: ", pos->filename, pos->line, pos->ch);
    vfprintf(config->error_file, format, ap);
    va_end(ap);
    fputc('\n', config->error_file);
}

void compiler::warning(const char* format, ...) {
    if (config->error_file == nullptr)
        return;
    va_list ap;

    va_start(ap, format);
    fprintf(config->error_file, "warning: ");
    vfprintf(config->error_file, format, ap);
    va_end(ap);
    fputc('\n', config->error_file);
}

void parser::warning(const char* format, ...) {
    if (linked_compiler->config->error_file == nullptr)
        return;
    va_list ap;

    va_start(ap, format);
    parser_position* pos = (*input)[(int)position].position;
    fprintf(linked_compiler->config->error_file, "%s:%lu:%lu: warning: ", pos->filename, pos->line,
            pos->ch);
    vfprintf(linked_compiler->config->error_file, format, ap);
    va_end(ap);
    fputc('\n', linked_compiler->config->error_file);
}

void compiler::error_positioned(parser_position* pos, const char* format, ...) {
    state = BONK_COMPILER_STATE_ERROR;
    if (config->error_file == nullptr)
        return;
    va_list ap;

    va_start(ap, format);
    if (pos) {
        fprintf(config->error_file, "%s:%lu:%lu: error: ", pos->filename, pos->line, pos->ch);
    } else {
        fprintf(config->error_file, "error: ");
    }
    vfprintf(config->error_file, format, ap);
    va_end(ap);
    fputc('\n', config->error_file);
}

void compiler::error(const char* format, ...) {
    state = BONK_COMPILER_STATE_ERROR;
    if (config->error_file == nullptr)
        return;
    va_list ap;

    va_start(ap, format);
    fprintf(config->error_file, "error: ");
    vfprintf(config->error_file, format, ap);
    va_end(ap);
    fputc('\n', config->error_file);
}

void parser::error(const char* format, ...) {
    linked_compiler->state = BONK_COMPILER_STATE_ERROR;
    if (linked_compiler->config->error_file == nullptr)
        return;
    va_list ap;

    va_start(ap, format);
    parser_position* pos = (*input)[(int)position].position;
    if (pos) {
        fprintf(linked_compiler->config->error_file, "%s:%lu:%lu: error: ", pos->filename,
                pos->line, pos->ch);
    } else {
        fprintf(linked_compiler->config->error_file, "error: ");
    }
    vfprintf(linked_compiler->config->error_file, format, ap);
    va_end(ap);
    fputc('\n', linked_compiler->config->error_file);
}

void lexical_analyzer::error(const char* format, ...) {
    linked_compiler->state = BONK_COMPILER_STATE_ERROR;
    if (linked_compiler->config->error_file == nullptr)
        return;
    va_list ap;

    va_start(ap, format);
    parser_position* pos = &position;
    if (pos) {
        fprintf(linked_compiler->config->error_file, "%s:%lu:%lu: error: ", pos->filename,
                pos->line, pos->ch);
    } else {
        fprintf(linked_compiler->config->error_file, "error: ");
    }
    vfprintf(linked_compiler->config->error_file, format, ap);
    va_end(ap);
    fputc('\n', linked_compiler->config->error_file);
}

void compiler::fatal_error_positioned(parser_position* pos, const char* format, ...) {
    state = BONK_COMPILER_STATE_FATAL_ERROR;
    if (config->error_file == nullptr)
        return;
    va_list ap;

    va_start(ap, format);
    fprintf(config->error_file, "%s:%lu:%lu: fatal error: ", pos->filename, pos->line, pos->ch);
    vfprintf(config->error_file, format, ap);
    va_end(ap);
    fputc('\n', config->error_file);
}

void compiler::fatal_error(const char* format, ...) {
    FILE* output = stderr;

    state = BONK_COMPILER_STATE_FATAL_ERROR;
    if (config->error_file == nullptr)
        return;
    output = config->error_file;

    va_list ap;

    va_start(ap, format);
    fprintf(output, "fatal error: ");
    vfprintf(output, format, ap);
    va_end(ap);
    fputc('\n', output);
}

void parser::fatal_error(const char* format, ...) {
    FILE* output = stderr;
    va_list ap;
    va_start(ap, format);

    linked_compiler->state = BONK_COMPILER_STATE_FATAL_ERROR;
    if (linked_compiler->config->error_file == nullptr)
        return;
    output = linked_compiler->config->error_file;
    parser_position* pos = (*input)[(int)position].position;

    fprintf(output, "%s:%lu:%lu: fatal error: ", pos->filename, pos->line, pos->ch);
    vfprintf(output, format, ap);

    va_end(ap);
    fputc('\n', output);
}

} // namespace bonk