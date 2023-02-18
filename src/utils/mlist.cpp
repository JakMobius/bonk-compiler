
#include "mlist.hpp"

template <typename T> struct mlist {
    T* element_array;
    int* prev_indices;
    int* next_indices;
    int head;
    int tail;
    int capacity;
    int size;

    mlist(int capacity);

    ~mlist();

    void insert(int index, T element);

    void insert_head(T element);

    void insert_tail(T element);

    void delete_index(int index, T* target);

    void delete_head(T* target);

    void delete_tail(T* target);

    void linear_address_search(int index, int* address) const;

    void shrink_to_fit();

    void resize(int size);

    void order_swap(int a, int b);

    void clear();
};

template <typename T> mlist<T>::mlist(int new_capacity) {
    tail = 1;
    head = 1;
    size = 0;
    capacity = 0;

    element_array = nullptr;
    prev_indices = nullptr;
    next_indices = nullptr;

    resize(new_capacity);

    next_indices[0] = 1;
    prev_indices[0] = new_capacity;
    prev_indices[1] = 0;
    next_indices[new_capacity] = 0;
}

template <typename T> void mlist<T>::insert(int index, T element) {

    // index = 0 is valid when root_list is full
    // and user wants to insert element to end
    if (index == 0 && tail != 0)
        return;

    // If index = 0 is invalid, it's already has been checked.
    if (index != 0 && index - 1 > size)
        return;

    if (tail == 0) {
        resize(capacity * 2);

        // If we were adding elements to tail,
        // resetting it to tail as it was zero

        if (index == 0)
            index = tail;
    }

    int free_element = tail;

    element_array[free_element] = element;
    tail = next_indices[free_element];
    size++;

    if (index == free_element) {
        return;
    }

    next_indices[prev_indices[free_element]] = next_indices[free_element];
    prev_indices[next_indices[free_element]] = prev_indices[free_element];

    prev_indices[free_element] = prev_indices[index];
    next_indices[free_element] = index;
    prev_indices[index] = free_element;

    next_indices[prev_indices[free_element]] = free_element;
    prev_indices[next_indices[index]] = index;

    if (head == index) {
        head = free_element;
    }

    return;
}

template <typename T> void mlist<T>::insert_head(T element) {
    return insert(head, element);
}

template <typename T> void mlist<T>::insert_tail(T element) {
    return insert(tail, element);
}

template <typename T> void mlist<T>::delete_index(int index, T* target) {
    if (index == 0)
        return;
    if (index > size)
        return;

    if (target != nullptr)
        *target = element_array[index];

    if (head == index && size != 1)
        head = next_indices[head];

    order_swap(index, size);

    // Deleting item from old position

    next_indices[prev_indices[size]] = next_indices[size];
    prev_indices[next_indices[size]] = prev_indices[size];

    // Inserting item to end

    prev_indices[size] = prev_indices[tail];
    prev_indices[tail] = size;

    if (prev_indices[tail] == size) {
        next_indices[size] = tail;
        next_indices[prev_indices[size]] = size;
    } else {
        next_indices[size] = tail;
        next_indices[prev_indices[tail]] = size;
    }

    tail = prev_indices[tail];

    size--;

    //    if(size < capacity / 4 && capacity > min_size * 2) {
    //        resize(capacity / 2);
    //    }

    return;
}

template <typename T> void mlist<T>::delete_head(T* target) {
    return delete_index(head, target);
}

template <typename T> void mlist<T>::delete_tail(T* target) {
    return delete_index(prev_indices[tail], target);
}

template <typename T> void mlist<T>::order_swap(int a, int b) {

    if (head == a)
        head = b;
    else if (head == b)
        head = a;

    if (tail == a)
        tail = b;
    else if (tail == b)
        tail = a;

    T buffer = element_array[a];
    element_array[a] = element_array[b];
    element_array[b] = buffer;

    int prev_a = prev_indices[a];
    int next_a = next_indices[a];

    int prev_b = prev_indices[b];
    int next_b = next_indices[b];

    if (next_a == b) {
        prev_indices[a] = b;
        next_indices[b] = a;
    } else {
        prev_indices[a] = prev_b;
        next_indices[b] = next_a;
    }

    if (prev_a == b) {
        next_indices[a] = b;
        prev_indices[b] = a;
    } else {
        next_indices[a] = next_b;
        prev_indices[b] = prev_a;
    }

    if (next_a != b) {
        prev_indices[next_a] = b;
        next_indices[prev_b] = a;
    }

    if (prev_a != b) {
        next_indices[prev_a] = b;
        prev_indices[next_b] = a;
    }
}

template <typename T> void mlist<T>::shrink_to_fit() {
    return resize(size);
}

template <typename T> void mlist<T>::resize(int new_capacity) {
    new_capacity++;

    int* new_element_array = (T*)realloc(this->element_array, new_capacity * sizeof(T));
    if (!new_element_array)
        return;
    element_array = new_element_array;

    int* new_next_indices = (int*)realloc(this->next_indices, new_capacity * sizeof(int));
    if (!new_next_indices)
        return;
    next_indices = new_next_indices;

    int* new_prev_indices = (int*)realloc(this->prev_indices, new_capacity * sizeof(int));
    if (!new_prev_indices)
        return;
    prev_indices = new_prev_indices;

    if (new_capacity > capacity) {

        int old_capacity = capacity;

        for (int index = old_capacity; index < new_capacity; index++) {
            new_next_indices[index] = index + 1;
            if (index > capacity)
                prev_indices[index] = index - 1;
        }

        int old_last_element = prev_indices[tail];

        next_indices[prev_indices[tail]] = capacity;
        prev_indices[capacity] = prev_indices[tail];

        next_indices[new_capacity - 1] = 0;
        prev_indices[0] = new_capacity - 1;
        next_indices[0] = head;

        tail = next_indices[old_last_element];
    } else {
        if (tail == new_capacity) {
            tail = 0;
        }
        next_indices[capacity - 1] = 0;
        prev_indices[0] = capacity - 1;
    }

    capacity = new_capacity;
}

template <typename T> void mlist<T>::linear_address_search(int index, int* target) const {
    if (index < 0 || index > size)
        return;

    for (int address = head; address != tail; address = next_indices[address]) {
        if (index-- == 0) {
            *target = address;
            return;
        }
    }

    if (index == 0) {
        *target = tail;
    }
}

template <typename T> mlist<T>::~mlist() {
    free(element_array);
    free(next_indices);
    free(prev_indices);

    element_array = nullptr;
    next_indices = nullptr;
    prev_indices = nullptr;
}

template <typename T> void mlist<T>::clear() {
    tail = 1;
    head = 1;
    size = 0;
}