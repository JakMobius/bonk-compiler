
#include "errors.hpp"
#include "compiler.hpp"
#include "parsing/lexic/lexical_analyzer.hpp"

namespace bonk {

void Compiler::warning_positioned(ParserPosition* pos, const char* format, ...) const {
    if (config->error_file == nullptr)
        return;
    va_list ap;

    va_start(ap, format);
    fprintf(config->error_file, "%s:%lu:%lu: warning: ", pos->filename, pos->line, pos->ch);
    vfprintf(config->error_file, format, ap);
    va_end(ap);
    fputc('\n', config->error_file);
}

void Compiler::warning(const char* format, ...) const {
    if (config->error_file == nullptr)
        return;
    va_list ap;

    va_start(ap, format);
    fprintf(config->error_file, "warning: ");
    vfprintf(config->error_file, format, ap);
    va_end(ap);
    fputc('\n', config->error_file);
}

void Parser::warning(const char* format, ...) const {
    if (linked_compiler->config->error_file == nullptr)
        return;
    va_list ap;

    va_start(ap, format);
    ParserPosition* pos = (*input)[(int)position].position;
    fprintf(linked_compiler->config->error_file, "%s:%lu:%lu: warning: ", pos->filename, pos->line,
            pos->ch);
    vfprintf(linked_compiler->config->error_file, format, ap);
    va_end(ap);
    fputc('\n', linked_compiler->config->error_file);
}

void Compiler::error_positioned(ParserPosition* pos, const char* format, ...) {
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

void Compiler::error(const char* format, ...) {
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

void Parser::error(const char* format, ...) const {
    linked_compiler->state = BONK_COMPILER_STATE_ERROR;
    if (linked_compiler->config->error_file == nullptr)
        return;
    va_list ap;

    va_start(ap, format);
    ParserPosition* pos = (*input)[(int)position].position;
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

void LexicalAnalyzer::error(const char* format, ...) {
    linked_compiler->state = BONK_COMPILER_STATE_ERROR;
    if (linked_compiler->config->error_file == nullptr)
        return;
    va_list ap;

    va_start(ap, format);
    ParserPosition* pos = &position;
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

void Compiler::fatal_error_positioned(ParserPosition* pos, const char* format, ...) {
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

void Compiler::fatal_error(const char* format, ...) {
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

void Parser::fatal_error(const char* format, ...) const {
    FILE* output = stderr;
    va_list ap;
    va_start(ap, format);

    linked_compiler->state = BONK_COMPILER_STATE_FATAL_ERROR;
    if (linked_compiler->config->error_file == nullptr)
        return;
    output = linked_compiler->config->error_file;
    ParserPosition* pos = (*input)[(int)position].position;

    fprintf(output, "%s:%lu:%lu: fatal error: ", pos->filename, pos->line, pos->ch);
    vfprintf(output, format, ap);

    va_end(ap);
    fputc('\n', output);
}

} // namespace bonk