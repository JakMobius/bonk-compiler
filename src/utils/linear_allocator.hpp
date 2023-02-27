#pragma once

#include <vector>

struct LinearAllocatorPage {
    void* address;
    unsigned long long usage;
    unsigned long long capacity;
};

struct LinearAllocator {
    std::vector<LinearAllocatorPage> pages;
    unsigned long long page_capacity;

    LinearAllocator(unsigned long long page_capacity = 1024);

    void set_page_capacity(unsigned long long page_capacity);

    void* allocate(unsigned long long capacity);

    void clear();

    bool create_page(unsigned long long min_capacity);

    char* strdup(const char* str);
};
