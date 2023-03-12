
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

    return (uint64_t) (ref_counter + 1);
}

uint64_t $$bonk_object_inc_reference(uint64_t object) {
    // Get the reference counter

    uint64_t* ref_counter = (uint64_t*) object - 1;

    // Increment the reference counter

    (*ref_counter)++;

    // Return the object

    return object;
}

uint32_t $$bonk_object_dec_reference(uint64_t object) {
    // Get the reference counter

    uint64_t* ref_counter = (uint64_t*) object - 1;

    // Decrement the reference counter

    (*ref_counter)--;

    return ref_counter > 0;
}

void $$bonk_object_free(uint64_t object) {
    // Get the actual pointer to the object

    uint64_t* ref_counter = (uint64_t*) object - 1;

    // Free the object

    free(ref_counter);
}