
#include "linear_allocator.hpp"
#include <cstdlib>
#include <cstring>

LinearAllocator::LinearAllocator(unsigned long long the_page_capacity) {
    page_capacity = the_page_capacity;
}

void* LinearAllocator::allocate(unsigned long long capacity) {
    if (!pages.empty()) {
        LinearAllocatorPage& page = pages[pages.size() - 1];
        if (page.capacity - page.usage >= capacity) {
            void* address = (char*)page.address + page.usage;
            page.usage += capacity;
            return address;
        }
    }

    if (!create_page(capacity))
        return nullptr;
    LinearAllocatorPage& page = pages[pages.size() - 1];
    page.usage += capacity;
    return page.address;
}

void LinearAllocator::clear() {
    for (auto & page : pages) {
        free(page.address);
    }
    pages.clear();
}

bool LinearAllocator::create_page(unsigned long long min_capacity) {
    if (min_capacity < page_capacity)
        min_capacity = page_capacity;

    LinearAllocatorPage page = {};
    page.usage = 0;
    page.capacity = min_capacity;

    page.address = calloc(1, min_capacity);
    if (!page.address)
        return false;

    pages.push_back(page);

    return true;
}

void LinearAllocator::set_page_capacity(unsigned long long the_page_capacity) {
    page_capacity = the_page_capacity;
}
