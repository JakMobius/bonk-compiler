
#pragma once

#include <cstdlib>
#include <vector>

template <typename T> struct bin_heap {
    std::vector<T> buffer {};
    long long length = 0;

    bin_heap();

    T pop();

    void push(T value);

    void sift_down(long long index);

    void sift_up(long long index);

    T get_max();

    T get_min();
};

template <typename T> bin_heap<T>::bin_heap() {
}

template <typename T> T bin_heap<T>::pop() {
    assert(length > 0);
    if (length == 1) {
        length--;
        return buffer[0];
    }

    T saved_value = buffer[0];
    buffer[0] = buffer[length - 1];
    length--;

    sift_down(0);

    return saved_value;
}

template <typename T> void bin_heap<T>::push(T value) {
    if (length == 0) {
        if (buffer.size() <= length)
            buffer.push_back(value);
        else
            buffer[0] = value;
        length = 1;
        return;
    }
    long long index = length;
    if (buffer.size() <= length)
        buffer.push_back(value);
    else
        buffer[index] = value;

    length++;

    sift_up(index);
}

template <typename T> T bin_heap<T>::get_max() {
    return buffer.get(length - 1);
}

template <typename T> T bin_heap<T>::get_min() {
    return buffer[0];
}

template <typename T> void bin_heap<T>::sift_up(long long index) {
    if (index == 0) {
        return;
    }

    while (true) {
        long long parent_index = (index + 1) / 2 - 1;
        if (parent_index == index)
            break;
        if (buffer[index] < buffer[parent_index]) {
            T temp = buffer[parent_index];
            buffer[parent_index] = buffer[index];
            buffer[index] = temp;
        } else {
            break;
        }

        if (parent_index == 0)
            return;

        index = parent_index;
    }
}

template <typename T> void bin_heap<T>::sift_down(long long index) {

    while (true) {
        long long child_index = (index + 1) * 2;
        if (child_index > length) {
            break;
        } else if (child_index == length || buffer[child_index - 1] < buffer[child_index]) {
            child_index--;
        }

        if (child_index == index) {
            return;
        }

        if (buffer[child_index] < buffer[index]) {
            T temp = buffer[index];
            buffer[index] = buffer[child_index];
            buffer[child_index] = temp;
            index = child_index;
        } else {
            break;
        }
    }
}
