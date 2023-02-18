
#include "linear_allocator.hpp"
#include <cstring>

linear_allocator::linear_allocator(unsigned long long the_page_capacity) {
    page_capacity = the_page_capacity;
}

void* linear_allocator::allocate(unsigned long long capacity) {
    if (pages.size() > 0) {
        linear_allocator_page& page = pages[pages.size() - 1];
        if (page.capacity - page.usage >= capacity) {
            void* address = (char*)page.address + page.usage;
            page.usage += capacity;
            return address;
        }
    }

    if (!create_page(capacity))
        return nullptr;
    linear_allocator_page& page = pages[pages.size() - 1];
    page.usage += capacity;
    return page.address;
}

void linear_allocator::clear() {
    for (int i = 0; i < pages.size(); i++) {
        free(pages[i].address);
    }
    pages.clear();
}

bool linear_allocator::create_page(unsigned long long min_capacity) {
    if (min_capacity < page_capacity)
        min_capacity = page_capacity;

    linear_allocator_page page = {};
    page.usage = 0;
    page.capacity = min_capacity;

    page.address = calloc(1, min_capacity);
    if (!page.address)
        return false;

    pages.push_back(page);

    return true;
}

char* linear_allocator::strdup(const char* str) {
    char* duplicate = (char*)allocate(strlen(str) + 1);
    strcpy(duplicate, str);
    return duplicate;
}

void linear_allocator::set_page_capacity(unsigned long long the_page_capacity) {
    page_capacity = the_page_capacity;
}
