#pragma once

#include <vector>
#include <string>
#include <unordered_set>

namespace bonk {

class Buffer {
  public:
    std::vector<std::vector<char>> data_storage;
    std::unordered_set<std::string_view> symbols;

    std::string_view get_symbol(std::vector<char>&& symbol);
    std::string_view get_symbol(std::string_view symbol);
    std::string_view store_data(std::string_view data);
    char* reserve_data(size_t size);
};

}