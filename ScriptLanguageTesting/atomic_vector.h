#pragma once
#include <vector>
#include <atomic>
#include <array>
#include <ShlDisp.h>
#include <winnt.h>
#include <stdexcept>
#include "basic_atomic_allocator.h"

// Atomic Vector
namespace GL {
    // typedef void* PVOID;

    // equivalent to concurrency::concurrent_vector. Equal performance or slightly faster (~15%) for small number of items, and slightly worse (~20%) performance for many items.
    // buckets increase the number of allocations by *2 each time. Maximum number of buckets must be known at compile-time. 
    // Therefore, there is a maximum size this vector may have. Note that increasing the maximum number of buckets 
    // should only result in a minor increase to the memory use, and little to no impact on performance. Suggest 24 for ~ 33M items, while 64 would handle nearly all use cases possible. 
    template <typename T, size_t max_num_buckets = 32>
    class atomic_vector {
        inline static const short tab64[64] = {
            63,  0, 58,  1, 59, 47, 53,  2,
            60, 39, 48, 27, 54, 33, 42,  3,
            61, 51, 37, 40, 49, 18, 28, 20,
            55, 30, 34, 11, 43, 14, 22,  4,
            62, 57, 46, 52, 38, 26, 32, 41,
            50, 36, 17, 19, 29, 10, 13, 21,
            56, 45, 25, 31, 35, 16,  9, 12,
            44, 24, 15,  8, 23,  7,  6,  5
        };
        static short log2_64(uint64_t value) noexcept {
            value |= value >> 1;
            value |= value >> 2;
            value |= value >> 4;
            value |= value >> 8;
            value |= value >> 16;
            value |= value >> 32;
            return tab64[((uint64_t)((value - (value >> 1)) * 0x07EDD5E59A4E28C2)) >> 58];
        }
        // 0 -> 0, 4 -> 1, 8 -> 2, 16 -> 3, etc.
        static short global_index_to_block(size_t index) noexcept {
            if (index <= 3ull) return 0;
            else return log2_64(index) - 1;
        };
        // 0 -> 0, 3 -> 3, 4 -> 0, 7 -> 3, 8 -> 0, 15 -> 7, 16 -> 0, 31 -> 15, etc.
        static size_t global_index_to_local_index(size_t index) noexcept {
            if (index <= 3ull) return index;
            else return index - (2ull << (log2_64(index) - 1));
        };
        static size_t global_index_to_local_index(size_t index, short blockN) noexcept {
            if (index <= 3ull) return index;
            else return index - (2ull << blockN);
        };

        // 0 -> 4, 1 -> 4, 2 -> 8, 3 -> 16, 4 -> 32, etc.
        static size_t block_to_allocsize(short block_n) noexcept {
            if (block_n <= 1) return 4ull;
            else return 2ull << block_n;
        };

        // 0 -> 4, 1 -> 4, 2 -> 8, 3 -> 16, 4 -> 32, etc.
        static size_t block_to_total_allocsize(short block_n) noexcept {
            if (block_n <= 0) return 4ull;
            else return 2ull << (block_n + 1);
        };

        using element_t = T;
        std::array< element_t*, max_num_buckets >
            blocks{ 0 };
        size_t
            current_pos{ 0 };
        size_t
            valid_pos{ 0 };
        short
            current_blockN{ -1 };

        __declspec(noinline) bool EnsureBlockExists(short block_n) noexcept {
            bool out = false;
            if (blocks[block_n]) return out;
            for (short blockN = 0; blockN <= block_n; ++blockN) {
                if (!blocks[blockN]) {
                    element_t* new_ptr = (element_t*)(::_aligned_malloc(block_to_allocsize(blockN) * sizeof(element_t), 16));
                    if constexpr (!std::is_pod_v<T>) {
                        for (int i = 0; i < block_to_allocsize(blockN); ++i) {
                            new (new_ptr + i) element_t();
                        }
                    }
                    else {
                        // some users expect the POD-types to be zero'd when the requested index has been initialized.
                        std::memset(new_ptr, 0, block_to_allocsize(blockN) * sizeof(element_t));
                    }
                    if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&blocks[blockN]), new_ptr, nullptr) == nullptr) {
                        out = true;
                    }
                    else {
                        if constexpr (!std::is_pod_v<element_t>) {
                            for (int i = 0; i < block_to_allocsize(blockN); ++i) {
                                (new_ptr + i)->~element_t();
                            }
                        }
                        ::_aligned_free(new_ptr);
                    }
                }
            }
            return out;
        };
        bool grow_to_at_least_blocksN(short blockN) noexcept { return EnsureBlockExists(blockN); };
    public:
        atomic_vector() noexcept = default;
        atomic_vector(atomic_vector const&) = delete;
        atomic_vector(atomic_vector &&) = delete;
        atomic_vector& operator=(atomic_vector const&) = delete;
        atomic_vector& operator=(atomic_vector&&) = delete;
        __declspec(noinline) ~atomic_vector() noexcept {
            for (int blockN = 0; blockN < max_num_buckets; ++blockN) {
                if (blocks[blockN]) {
                    if constexpr (!std::is_pod_v<T>) {
                        for (int i = 0; i < block_to_allocsize(blockN); ++i) {
                            (blocks[blockN] + i)->~element_t();
                        }
                    }
                    ::_aligned_free(blocks[blockN]);
                }
            }
        };

        element_t& at(size_t index) noexcept {
            auto block_i = global_index_to_block(index);
            auto block_j = global_index_to_local_index(index, block_i);
            return blocks[block_i][block_j];
        };
        element_t& operator[](size_t index) noexcept { return at(index); };
        const element_t& at(size_t index) const noexcept {
            auto block_i = global_index_to_block(index);
            auto block_j = global_index_to_local_index(index, block_i);
            return blocks[block_i][block_j];
        };
        const element_t& operator[](size_t index) const noexcept { return at(index); };
        bool grow_to_at_least(size_t index) noexcept {
            bool did_grow = EnsureBlockExists(global_index_to_block(index));
            while (true) {
                size_t prevValid = valid_pos;
                if (prevValid < index) {
                    if (InterlockedCompareExchange(reinterpret_cast<volatile size_t*>(&valid_pos), index, prevValid) == prevValid) {
                        break;
                    }
                }
                else {
                    break;
                }
            }
            return did_grow;
        };
        size_t push_back(element_t const& srce) noexcept {
            size_t position;
            short blockN;

            position = InterlockedIncrementNoFence(reinterpret_cast<volatile size_t*>(&current_pos)) - 1;
            blockN = global_index_to_block(position);
            if (current_blockN < blockN) {
                grow_to_at_least_blocksN(blockN);
                InterlockedExchange16(reinterpret_cast<volatile short*>(&current_blockN), blockN);
            }
            blocks[blockN][global_index_to_local_index(position, blockN)] = srce;
            return position;
        };
        size_t push_back(element_t&& srce) noexcept {
            size_t position;
            short blockN;

            position = InterlockedIncrementNoFence(reinterpret_cast<volatile size_t*>(&current_pos)) - 1;
            blockN = global_index_to_block(position);
            if (current_blockN < blockN) {
                grow_to_at_least_blocksN(blockN);
                InterlockedExchange16(reinterpret_cast<volatile short*>(&current_blockN), blockN);
            }
            blocks[blockN][global_index_to_local_index(position, blockN)] = std::move(srce);
            return position;
        };
        size_t size() const {
            return valid_pos;
        };

        class Iterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = element_t;
            using difference_type = long long;
            using pointer = element_t*;
            using reference = element_t&;

            void recalc_position() {
                _block_inner_n = global_index_to_local_index(_ptr);
                _block_inner_size_n = block_to_allocsize(_block_outer_n = global_index_to_block(_ptr));
            };

            Iterator(const atomic_vector* p = nullptr, difference_type pos = 0) 
                : _ptr(pos), parent(const_cast<atomic_vector*>(p)) {
                recalc_position();
            }
            Iterator(const Iterator& rhs) 
                : _ptr(rhs._ptr)
                , parent(rhs.parent)  
                , _block_outer_n(rhs._block_outer_n)
                , _block_inner_n(rhs._block_inner_n)
                , _block_inner_size_n(rhs._block_inner_size_n)          
            {}

            /*inline*/ Iterator& operator+=(difference_type rhs) { 
                if (rhs < 0) return operator-=(-rhs);

                if ((_block_inner_n + rhs) >= _block_inner_size_n) {
                    _ptr += rhs;
                    recalc_position();
                }
                else {
                    _block_inner_n += rhs;
                    _ptr += rhs;
                }
                return *this; 
            }
            /*inline*/ Iterator& operator-=(difference_type rhs) {
                if (rhs < 0) return operator+=(-rhs);
                if (rhs > _block_inner_n) {
                    _ptr -= rhs;
                    recalc_position();
                }
                else {
                    _block_inner_n -= rhs;
                    _ptr -= rhs;
                }
                return *this;
            }
            /*inline*/ reference operator*() {
                return parent->blocks[_block_outer_n][_block_inner_n];
            }
            /*inline*/ pointer operator->() {
                return &parent->blocks[_block_outer_n][_block_inner_n];
            }
            /*inline*/ reference operator[](difference_type rhs) {
                return parent->at(rhs); 
            }
            /*inline*/ const reference operator*() const {
                return parent->blocks[_block_outer_n][_block_inner_n];
            }
            /*inline*/ const pointer operator->() const {
                return &parent->blocks[_block_outer_n][_block_inner_n];
            }
            /*inline*/ const reference operator[](difference_type rhs) const { return parent->at(rhs); }

            /*inline*/ Iterator& operator++() { return operator+=(1); }
            /*inline*/ Iterator& operator--() { return operator-=(1); }
            /*inline*/ Iterator operator++(int) { Iterator tmp(*this); this->operator++(); return tmp; }
            /*inline*/ Iterator operator--(int) { Iterator tmp(*this); this->operator--(); return tmp; }

            /*inline*/ difference_type operator-(const Iterator& rhs) const { return (_ptr - rhs._ptr); }
            /*inline*/ Iterator operator+(difference_type rhs) const { Iterator tmp(*this); tmp.operator+=(rhs); return tmp; }
            /*inline*/ Iterator operator-(difference_type rhs) const { Iterator tmp(*this); tmp.operator-=(rhs); return tmp; }
            friend /*inline*/ Iterator operator+(difference_type lhs, const Iterator& rhs) { Iterator tmp(rhs); tmp._ptr = lhs + rhs._ptr; tmp.recalc_position(); return tmp; }
            friend /*inline*/ Iterator operator-(difference_type lhs, const Iterator& rhs) { Iterator tmp(rhs); tmp._ptr = lhs - rhs._ptr; tmp.recalc_position(); return tmp; }

            /*inline*/ bool operator==(const Iterator& rhs) const { return _ptr == rhs._ptr; }
            /*inline*/ bool operator!=(const Iterator& rhs) const { return _ptr != rhs._ptr; }
            /*inline*/ bool operator>(const Iterator& rhs) const { return _ptr > rhs._ptr; }
            /*inline*/ bool operator<(const Iterator& rhs) const { return _ptr < rhs._ptr; }
            /*inline*/ bool operator>=(const Iterator& rhs) const { return _ptr >= rhs._ptr; }
            /*inline*/ bool operator<=(const Iterator& rhs) const { return _ptr <= rhs._ptr; }

            difference_type _ptr;
        protected:
            short _block_outer_n; // [0..max_num_buckets)
            difference_type _block_inner_n; // [0..max_num_buckets)
            difference_type _block_inner_size_n; // [0..             
            atomic_vector* parent;

        };

        using iterator = Iterator;
        using const_iterator = iterator;

        auto begin() { return Iterator(this, 0); };
        auto end() { return Iterator(this, valid_pos); };
        auto cbegin() const { return iterator(this, 0); };
        auto cend() const { return iterator(this, valid_pos); };
        auto begin() const { return iterator(this, 0); };
        auto end() const { return iterator(this, valid_pos); };

        friend class atomic_vector::Iterator;
    };
};