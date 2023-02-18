#pragma once

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>
#include <mach/mach.h>
#include <mach/machine.h>

namespace bonk::macho {

enum relocation_type { RELOCATION_TYPE_CALL, RELOCATION_TYPE_CONSTANT };

enum symbol_section { SYMBOL_SECTION_TEXT, SYMBOL_SECTION_DATA };

struct macho_file {
    mach_header_64 header;
    segment_command_64 segment;
    section_64 section_data;
    section_64 section_text;
    symtab_command symtab;
    dysymtab_command dysymtab;

    std::unordered_map<std::string, int> local_symbol_index_table;
    std::unordered_map<std::string, int> external_symbol_index_table;

    std::vector<relocation_info> relocations;
    std::vector<uint32_t> local_symbol_string_indices;
    std::vector<uint32_t> external_symbol_string_indices;
    std::vector<nlist_64> local_symbols;
    std::vector<nlist_64> external_symbols;
    std::vector<std::string> string_table;
    std::vector<std::string> text_fragments;
    std::vector<std::string> data_fragments;

    macho_file();

    void flush(FILE* file);

    bool add_relocation(uint32_t symbol, int32_t address, bool pc_rel, uint8_t data_length);

    void add_code(const std::string& code);

    void add_data(const std::string& data);

    void section_init_data();

    void section_init_text();

    void header_init();

    void symtab_init();

    void dysymtab_init();

    void segment_init();

    void declare_external_symbol(const std::string& symbol);

    void declare_internal_symbol(const std::string& symbol);

    void add_external_symbol(const std::string& symbol);

    void add_internal_symbol(const std::string& symbol, symbol_section section, uint32_t offset);

    std::string get_symbol_name(uint32_t symbol);

    uint32_t get_symbol_from_name(const std::string& symbol);
};

} // namespace bonk::macho
