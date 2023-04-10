
#include <stdlib.h>
#include <stdio.h>

uint64_t $$bonk_create_object(uint32_t size) {
    // Allocate memory for the object and for the reference counter

    void* ptr = calloc(sizeof(uint64_t) + size, 1);
    if(ptr == NULL) {
        fprintf(stderr, "bonk: failed to allocate memory for new object");
        exit(1);
    }

    // Set the reference counter to 1

    uint64_t* ref_counter = (uint64_t*) ptr;
    *ref_counter = 1;

    // Return the pointer to the object

    // If environment variable BONK_DEBUG is set, print the address of the allocated object

    if(getenv("BONK_DEBUG_ALLOC") != NULL) {
        printf("Allocated object of size %d at %p\n", size, ptr + sizeof(uint64_t));
        fflush(stdout);
    }

    return (uint64_t) (ref_counter + 1);
}

void $$bonk_object_free(uint64_t object) {
    // Get the actual pointer to the object

    uint64_t* ref_counter = (uint64_t*) object - 1;

    if(getenv("BONK_DEBUG_ALLOC") != NULL) {
        printf("Freed object at %p\n", (void*)object);
        fflush(stdout);
    }

    // Free the object
    free(ref_counter);
}