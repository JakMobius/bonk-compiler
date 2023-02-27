#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>

enum MListOpResult {
    LIST_OP_OK,
    LIST_OP_NOMEM,
    LIST_OP_OVERFLOW,
    LIST_OP_UNDERFLOW,
    LIST_OP_NOTFOUND,
    LIST_OP_SEGFAULT
};

template <typename T> class MList {
    struct Node {
        size_t next;
        size_t previous;
        T value;
        bool valid;
    };

    Node* storage;
    bool optimized;
    constexpr static int INITIAL_INCREASE = 32;

    size_t capacity;
    size_t size;

    size_t freePtr;
    size_t freeSize;

  public:
    MList(size_t initial_size = 16) {
        optimized = true;
        capacity = initial_size;
        size = 0;
        freeSize = 0;
        freePtr = 0;
        this->storage = (Node*)calloc(initial_size + 1, sizeof(Node));

        storage[0].next = 0;
        storage[0].previous = 0;
        storage[0].valid = false;
    }

    ~MList() {
        free(this->storage);
    }

    /**
     * Retrieves next possible free pos at all costs.
     * Reallocates container if needed.
     */
    size_t get_free_pos(bool mutating = false) {
        if (freeSize != 0) {
            size_t newPos = this->freePtr;
            if (mutating) {
                this->freeSize--;
                this->freePtr = this->storage[newPos].next;
            }
            this->storage[newPos].valid = true;
            return newPos;
        }
        if (this->reallocate() != LIST_OP_OK)
            return 0;
        this->storage[this->size + 1].valid = true;
        return this->size + 1;
    }

    /**
     * Reallocates container so that it can hold one more value
     * Reallocation is not performed if some freeSize cells are available.
     */
    MListOpResult reallocate() {
        if (this->freeSize != 0)
            return LIST_OP_OK;
        if (this->size < this->capacity)
            return LIST_OP_OK;
        size_t newCapacity = this->capacity;
        if (this->size >= this->capacity)
            newCapacity = (this->capacity == 0) ? INITIAL_INCREASE : this->capacity * 2;

        if (this->capacity == newCapacity + 2)
            return LIST_OP_OK;

        auto* newStorage = (Node*)realloc(this->storage, (newCapacity + 2) * sizeof(Node));

        if (newStorage == NULL)
            return LIST_OP_NOMEM;
        this->storage = newStorage;
        this->capacity = newCapacity;
        return LIST_OP_OK;
    }

    /**
     * Add released cell to the free poses list
     */
    void add_free_pos(size_t pos) {
        this->storage[pos].valid = false;
        this->storage[pos].previous = pos;
        this->storage[pos].next = pos;
        if (this->freeSize == 0) {
            this->freeSize = 1;
            this->freePtr = pos;
        } else {
            this->freeSize++;
            this->storage[pos].next = this->freePtr;
            this->freePtr = pos;
        }
    }

    /**
     * Convert logic position to the physic one
     */
    [[nodiscard]] size_t logic_to_physic(size_t pos) const {
        if (this->optimized) {
            return pos + 1;
        } else {
            size_t iterator = 0;
            for (size_t i = 0; i <= pos; i++)
                iterator = this->storage[iterator].next;
            return iterator;
        }
    }

    /**
     * Insert an element after pos
     * @param pos - physical pos of considered element
     * @param value - value to be inserted
     * @param physPos - physical position of inserted element
     * @return operation result
     */
    MListOpResult insert_after(size_t pos, T value, size_t* physPos = nullptr) {
        if (pos > this->sum_size()) {
            return LIST_OP_OVERFLOW;
        }
        if (!this->address_valid(pos) && pos != 0) {
            return LIST_OP_SEGFAULT;
        }
        if (pos != this->storage[0].previous)
            this->optimized = false;

        size_t newPos = this->get_free_pos(true);
        if (newPos == 0)
            return LIST_OP_NOMEM;

        if (physPos != nullptr)
            *physPos = newPos;

        this->storage[newPos].value = value;
        this->storage[newPos].previous = pos;
        this->storage[newPos].next = this->storage[pos].next;

        this->storage[this->storage[pos].next].previous = newPos;
        this->storage[pos].next = newPos;

        this->size++;

        return LIST_OP_OK;
    }

    /**
     * Insert an element after pos
     * @param pos - logical pos of considered element
     * @param value - value to be inserted
     * @param physPos - physical position of inserted element
     * @return operation result
     */
    MListOpResult insert_after_logic(size_t pos, T value, size_t* physPos = nullptr) {
        if (pos > this->size)
            return LIST_OP_OVERFLOW;
        return this->insert_after(this->logic_to_physic(pos), value, physPos);
    }

    /**
     * Insert an element before pos
     * @param pos - physical pos of considered element
     * @param value - value to be inserted
     * @param physPos - physical position of inserted element
     * @return operation result
     */
    MListOpResult insert_before(size_t pos, T value, size_t* physPos = nullptr) {
        if (pos > this->sum_size() || (!this->address_valid(pos) && pos != 0))
            return LIST_OP_SEGFAULT;
        pos = this->storage[pos].previous;
        return this->insert_after(pos, value, physPos);
    }

    /**
     * Insert an element before pos
     * @param pos - logical pos of considered element
     * @param value - value to be inserted
     * @param physPos - physical position of inserted element
     * @return operation result
     */
    MListOpResult insert_before_logic(size_t pos, T value, size_t* physPos = nullptr) {
        if (pos > this->size)
            return LIST_OP_OVERFLOW;
        return this->insert_before(this->logic_to_physic(pos), value, physPos);
    }

    /**
     * Insert an element at the first position
     * @param value - value to be inserted
     * @param physPos - physical position of inserted element
     * @return operation result
     */
    MListOpResult insert_head(const T& value, size_t* physPos = nullptr) {
        return this->insert_after(0, value, physPos);
    }

    /**
     * Insert an element at the last position
     * @param value - value to be inserted
     * @param physPos - physical position of inserted element
     * @return operation result
     */
    MListOpResult insert_tail(const T& value, size_t* physPos = nullptr) {
        return this->insert_after(this->storage[0].previous, value, physPos);
    }

    /**
     * Set an element at the physical position pos to the new value
     * @param pos - physical pos of considered element
     * @param value - new value
     * @return operation result
     */
    MListOpResult set(size_t pos, const T& value) {
        if (!this->address_valid(pos))
            return LIST_OP_SEGFAULT;
        this->storage[pos].value = value;
        return LIST_OP_OK;
    }

    /**
     * Set an element at the logical position pos to the new value
     * @param pos - logical pos of considered element
     * @param value - new value
     * @return operation result
     */
    MListOpResult set_logic(size_t pos, const T& value) {
        if (pos > this->size)
            return LIST_OP_OVERFLOW;
        return this->set(this->logic_to_physic(pos), value);
    }

    T get(size_t pos) {
        assert(this->storage[pos].valid);
        return this->storage[pos].value;
    }

    /**
     * Get an element at the physical position pos
     * @param pos - physical pos of considered element
     * @param value - retrieved value
     * @return operation result
     */
    MListOpResult get(size_t pos, T** value) {
        if (value == nullptr || !this->storage[pos].valid)
            return LIST_OP_SEGFAULT;
        *value = &(this->storage[pos].value);
        return LIST_OP_OK;
    }

    /**
     * Get an element at the physical position pos
     * @param pos - physical pos of considered element
     * @param value - retrieved value
     * @return operation result
     */
    MListOpResult get(size_t pos, T* value) {
        if (value == nullptr || !this->storage[pos].valid)
            return LIST_OP_SEGFAULT;
        *value = this->storage[pos].value;
        return LIST_OP_OK;
    }

    const Node* get_storage() const {
        return storage;
    }

    /**
     * Get an element at the logical position pos
     * @param pos - logical pos of considered element
     * @param value - retrieved value
     * @return operation result
     */
    MListOpResult get_logic(size_t pos, T** value = nullptr) {
        if (pos > this->size)
            return LIST_OP_OVERFLOW;
        return this->get(this->logic_to_physic(pos), value);
    }

    /**
     * Get an element at the logical position pos
     * @param pos - logical pos of considered element
     * @param value - retrieved value
     * @return operation result
     */
    MListOpResult get_logic(size_t pos, T* value = nullptr) {
        if (pos > this->size)
            return LIST_OP_OVERFLOW;
        return this->get(this->logic_to_physic(pos), value);
    }

    /**
     * Retrieve an element at the physical position pos and remove it
     * @param pos - physical pos of considered element
     * @param value - retrieved value
     * @return operation result
     */
    MListOpResult pop(size_t pos, T* value = nullptr) {
        if (this->size == 0)
            return LIST_OP_UNDERFLOW;
        if (!this->address_valid(pos))
            return LIST_OP_SEGFAULT;

        if (pos != this->storage[0].previous)
            this->optimized = false;

        if (value != nullptr)
            *value = this->storage[pos].value;

        this->storage[this->storage[pos].next].previous = this->storage[pos].previous;
        this->storage[this->storage[pos].previous].next = this->storage[pos].next;

        this->add_free_pos(pos);
        this->size--;
        return LIST_OP_OK;
    }

    /**
     * Retrieve an element at the beginning and remove it
     * @param value - retrieved value
     * @return operation result
     */
    MListOpResult pop_front(T* value) {
        return this->pop(this->storage[0].next, value);
    }

    /**
     * Retrieve an element at the end and remove it
     * @param value - retrieved value
     * @return operation result
     */
    MListOpResult pop_back(T* value) {
        return this->pop(this->storage[0].previous, value);
    }

    /**
     * Retrieve an element at the logical position pos and remove it
     * @param pos - logical pos of considered element
     * @param value - retrieved value
     * @return operation result
     */
    MListOpResult pop_logic(size_t pos, T* value) {
        return this->pop(this->logic_to_physic(pos), value);
    }

    /**
     * Remove an element at the physical position pos
     * @param pos - physical pos of considered element
     * @return operation result
     */
    MListOpResult remove(size_t pos) {
        return this->pop(pos, nullptr);
    }

    /**
     * Remove an element at the logical position pos
     * @param pos - logical pos of considered element
     * @return operation result
     */
    MListOpResult remove_logic(size_t pos) {
        if (pos > this->size)
            return LIST_OP_OVERFLOW;
        return this->pop(this->logic_to_physic(pos), nullptr);
    }

    /**
     * Clears the list
     * @return operation result
     */
    MListOpResult clear() {
        this->size = 0;
        this->storage[0].next = 0;
        this->storage[0].previous = 0;
        this->freeSize = 0;
        this->freePtr = 0;
        this->reallocate();
        return LIST_OP_OK;
    }

    /**
     * Optimizes the list so that logical access is effective and
     * physical positions are aligned in ascending order in the storage
     * @return operation result
     */
    MListOpResult optimize() {
        auto* newStorage = (Node*)(calloc(this->size + 2, sizeof(Node)));
        if (newStorage == nullptr)
            return LIST_OP_NOMEM;
        newStorage[0] = this->storage[0];
        size_t iterator = this->storage[0].next;
        for (size_t i = 0; i < this->size; i++) {
            newStorage[i + 1] = this->storage[iterator];
            iterator = this->storage[iterator].next;
            newStorage[i + 1].previous = i;
            newStorage[i].next = i + 1;
            newStorage[i].valid = true;
            if (i + 1 == this->size) {
                newStorage[i + 1].next = 0;
            }
        }
        this->optimized = true;
        this->freePtr = 0;
        this->freeSize = 0;
        free(this->storage);
        this->storage = newStorage;
        this->capacity = this->size;
        return LIST_OP_OK;
    }

    /**
     * Moves iterator to the next physical position
     * @param pos
     * @return
     */
    MListOpResult next_iterator(size_t* pos) {
        if (!this->address_valid(*pos) && *pos != 0)
            return LIST_OP_SEGFAULT;
        *pos = this->storage[*pos].next;
        return LIST_OP_OK;
    }

    /**
     * Moves iterator to the next physical position
     * @param pos
     * @return
     */
    size_t next_iterator(size_t pos) {
        if (!this->address_valid(pos) && pos != 0)
            return 0;
        return this->storage[pos].next;
    }

    /**
     * Moves iterator to the next physical position
     * @param pos
     * @return
     */
    size_t prev_iterator(size_t pos) {
        if (!this->address_valid(pos) && pos != 0)
            return 0;
        return this->storage[pos].previous;
    }

    /**
     * Moves iterator to the next physical position
     * @param pos
     * @return
     */
    MListOpResult prev_iterator(size_t* pos) {
        if (!this->address_valid(*pos) && *pos != 0)
            return LIST_OP_SEGFAULT;
        *pos = this->storage[*pos].previous;
        return LIST_OP_OK;
    }

    /**
     * Search an element in the list. Retrieves the physic position
     * @param pos - physic pos of considered element
     * @param value - searched value
     * @return operation result
     */
    MListOpResult search(size_t* pos, const T& value) const {
        if (this->size == 0)
            return LIST_OP_NOTFOUND;
        *pos = this->storage[0].next;
        while (*pos != 0) {
            if (this->storage[*pos].value == value)
                return LIST_OP_OK;
            if (*pos == this->storage[0].previous)
                break;
            *pos = this->storage[*pos].next;
        }
        return LIST_OP_NOTFOUND;
    }

    /**
     * Resizes list to desired number of elements. If elements number is lower than current one,
     * shrinks to fit.
     * @return operation result
     */
    MListOpResult resize(size_t elemNumbers) {
        if (elemNumbers < this->sum_size())
            elemNumbers = this->sum_size();
        elemNumbers++;
        auto newStorage = (Node*)realloc(storage, elemNumbers * sizeof(Node));
        if (newStorage == nullptr)
            return LIST_OP_NOMEM;
        this->storage = newStorage;
        this->capacity = elemNumbers - 1;
        return LIST_OP_OK;
    }

    /**
     * Resizes list to the minimum available space
     * @return operation result
     */
    MListOpResult shrinkToFit() {
        return this->resize(0);
    }

    [[nodiscard]] size_t begin() const {
#ifdef ASMOPT
#pragma message "Asm optimization of mlist::begin\n"
        volatile size_t retVal = 0;
        asm volatile("mov (%1), %0\n"
                     "mov (%0), %0\n"
                     : "=r"(retVal)
                     : "r"(&(this->storage)));
        return retVal;
#endif
#ifndef ASMOPT
        return this->storage[0].next;
#endif
    }

    [[nodiscard]] size_t last() const {
        return this->storage[0].previous;
    }

    [[nodiscard]] size_t end() const {
        return 0;
    }

    [[nodiscard]] size_t get_size() const {
        return this->size;
    }

    [[nodiscard]] size_t get_capacity() const {
        return this->capacity;
    }

    [[nodiscard]] bool is_optimized() const {
        return this->optimized;
    }

    [[nodiscard]] bool is_empty() const {
        return this->size == 0;
    }

    [[nodiscard]] size_t sum_size() const {
        return this->size + this->freeSize;
    }

    [[nodiscard]] bool address_valid(size_t pos) const {
        return this->storage[pos].valid;
    }
};
