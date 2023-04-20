#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <variant>
#include <vector>

namespace bonk {

class DynamicBitSet {
  public:
    explicit DynamicBitSet(size_t size, bool value = false) : bit_size(size) {
        // If someone wants to use a bitset larger than 2GB, they are probably doing something
        // wrong.
        assert(size < (1ULL << 32) * 8);
        if (num_chunks() > inline_chunks) {
            chunks.emplace<std::vector<uint64_t>>(num_chunks(), value ? ~0ULL : 0);
        } else {
            chunks.emplace<std::array<uint64_t, inline_chunks>>();
            std::get<std::array<uint64_t, inline_chunks>>(chunks).fill(value ? ~0ULL : 0);
        }
    }

    DynamicBitSet(std::initializer_list<bool> values) : DynamicBitSet(values.size()) {
        size_t i = 0;
        for (auto value : values) {
            (*this)[i++] = value;
        }
    }

    DynamicBitSet(const DynamicBitSet& other) = default;

    DynamicBitSet(DynamicBitSet&& other) noexcept
        : bit_size(other.bit_size), chunks(std::move(other.chunks)) {
    }

    DynamicBitSet& operator=(const DynamicBitSet& other) = default;

    DynamicBitSet& operator=(DynamicBitSet&& other) noexcept {
        bit_size = other.bit_size;
        chunks = std::move(other.chunks);
        return *this;
    }

    class BitProxy {
      public:
        BitProxy(uint64_t& chunk, uint64_t bit_mask) : chunk(chunk), bit_mask(bit_mask) {
        }

        BitProxy& operator=(bool value) {
            if (value) {
                chunk |= bit_mask;
            } else {
                chunk &= ~bit_mask;
            }
            return *this;
        }

        explicit operator bool() const {
            return (chunk & bit_mask) != 0;
        }

      private:
        uint64_t& chunk;
        uint64_t bit_mask;
    };

    BitProxy operator[](size_t index) {
        if (index >= bit_size) {
            throw std::out_of_range("Index out of range");
        }
        return {data()[index / bits_per_chunk], 1ULL << (index % bits_per_chunk)};
    }

    bool operator[](size_t index) const {
        if (index >= bit_size) {
            throw std::out_of_range("Index out of range");
        }
        return (data()[index / bits_per_chunk] & (1ULL << (index % bits_per_chunk))) != 0;
    }

    DynamicBitSet& operator&=(const DynamicBitSet& other) {
        if (bit_size != other.bit_size) {
            throw std::invalid_argument("Bitsets must have the same size");
        }
        for (auto i = (ssize_t)num_chunks() - 1; i >= 0; --i) {
            data()[i] &= other.data()[i];
        }
        return *this;
    }

    DynamicBitSet& operator|=(const DynamicBitSet& other) {
        if (bit_size != other.bit_size) {
            throw std::invalid_argument("Bitsets must have the same size");
        }
        for (auto i = (ssize_t)num_chunks() - 1; i >= 0; --i) {
            data()[i] |= other.data()[i];
        }
        return *this;
    }

    DynamicBitSet& operator^=(const DynamicBitSet& other) {
        if (bit_size != other.bit_size) {
            throw std::invalid_argument("Bitsets must have the same size");
        }
        for (auto i = (ssize_t)num_chunks() - 1; i >= 0; --i) {
            data()[i] ^= other.data()[i];
        }
        return *this;
    }

    DynamicBitSet& operator-=(const DynamicBitSet& other) {
        if (bit_size != other.bit_size) {
            throw std::invalid_argument("Bitsets must have the same size");
        }
        for (auto i = (ssize_t)num_chunks() - 1; i >= 0; --i) {
            data()[i] &= ~other.data()[i];
        }
        return *this;
    }

    DynamicBitSet& negate() {
        for (ssize_t i = (ssize_t)num_chunks() - 1; i >= 0; --i) {
            data()[i] = ~data()[i];
        }
        return *this;
    }

    DynamicBitSet& reset() {
        for (ssize_t i = (ssize_t)num_chunks() - 1; i >= 0; --i) {
            data()[i] = 0;
        }
        return *this;
    }

    DynamicBitSet& set() {
        for (ssize_t i = (ssize_t)num_chunks() - 1; i >= 0; --i) {
            data()[i] = ~0ULL;
        }
        return *this;
    }

    DynamicBitSet operator&(const DynamicBitSet& other) const {
        DynamicBitSet result(*this);
        result &= other;
        return result;
    }

    DynamicBitSet operator|(const DynamicBitSet& other) const {
        DynamicBitSet result(*this);
        result |= other;
        return result;
    }

    DynamicBitSet operator^(const DynamicBitSet& other) const {
        DynamicBitSet result(*this);
        result ^= other;
        return result;
    }

    DynamicBitSet operator-(const DynamicBitSet& other) const {
        DynamicBitSet result(*this);
        result -= other;
        return result;
    }

    DynamicBitSet operator~() const {
        DynamicBitSet result(*this);
        result.negate();
        return result;
    }

    bool operator==(const DynamicBitSet& other) const {
        if (bit_size != other.bit_size) {
            return false;
        }
        for (auto i = (ssize_t)num_chunks() - 1; i >= 0; --i) {
            if (data()[i] != other.data()[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const DynamicBitSet& other) const {
        return !(*this == other);
    }

    [[nodiscard]] size_t size() const {
        return bit_size;
    }

    uint64_t* data() {
        return std::visit([](auto& data) { return data.data(); }, chunks);
    }

    [[nodiscard]] const uint64_t* data() const {
        return std::visit([](const auto& data) { return data.data(); }, chunks);
    }

    [[nodiscard]] size_t num_chunks() const {
        return (bit_size + bits_per_chunk - 1) / bits_per_chunk;
    }

    bool any() {
        for (auto i = (ssize_t)num_chunks() - 1; i >= 0; --i) {
            if (data()[i] != 0) {
                return true;
            }
        }
        return false;
    }

  private:
    static constexpr size_t inline_chunks = 2;
    static constexpr size_t bits_per_chunk = sizeof(uint64_t) * 8;
    std::variant<std::array<uint64_t, inline_chunks>, std::vector<uint64_t>> chunks;
    size_t bit_size;
};

} // namespace bonk