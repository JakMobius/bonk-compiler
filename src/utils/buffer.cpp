
#include "buffer.hpp"
#include <cstring>

std::string_view bonk::Buffer::get_symbol(std::string_view symbol) {
    // Find the symbol in the set, if it's not there, insert it
    auto it = symbols.find(symbol);
    if (it == symbols.end()) {
        std::vector<char> buffer{symbol.begin(), symbol.end()};
        std::string_view view(buffer.data(), buffer.size());
        data_storage.push_back(std::move(buffer));
        it = symbols.insert(view).first;
        return *it;
    }
    return *it;
}

std::string_view bonk::Buffer::get_symbol(std::vector<char>&& symbol) {
    // Find the symbol in the set, if it's not there, insert it
    std::string_view view(symbol.data(), symbol.size());
    auto it = symbols.find(view);
    if (it == symbols.end()) {
        data_storage.push_back(std::move(symbol));
        it = symbols.insert(view).first;
        return *it;
    }
    return *it;
}

std::string_view bonk::Buffer::store_data(std::string_view data) {
    char* buffer = reserve_data(data.size());
    memcpy(buffer, data.data(), data.size());
    return {buffer, data.size()};
}

char* bonk::Buffer::reserve_data(size_t size) {
    std::vector<char> buffer(size);
    char* result = buffer.data();
    data_storage.push_back(std::move(buffer));
    return result;
}
