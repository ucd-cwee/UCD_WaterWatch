/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

This file is distributed under the BSD License.
Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
http://www.chaiscript.com

 History: RTG	/	2023		1. Modified original source code to use WaterWatch tools, and for better real-time support, including object-typing from parsed code, pre-parsing code without running, real multithreaded code analysis, and more.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include "chaiscript_wrapper.h"

template<std::size_t N, std::size_t alignment = alignof(std::max_align_t)>
class arena {
    alignas(alignment) char buf_[N];
    char* ptr_;

public:
    ~arena() { ptr_ = nullptr; }
    arena() noexcept
        : ptr_(buf_) {
    }
    arena(const arena&) = delete;
    arena& operator=(const arena&) = delete;

    template<std::size_t ReqAlign>
    char* allocate(std::size_t n);
    void deallocate(char* p, std::size_t n) noexcept;

    static constexpr std::size_t size() noexcept { return N; }
    std::size_t used() const noexcept { return static_cast<std::size_t>(ptr_ - buf_); }
    void reset() noexcept { ptr_ = buf_; }

private:
    static std::size_t align_up(std::size_t n) noexcept { return (n + (alignment - 1)) & ~(alignment - 1); }

    bool pointer_in_buffer(char* p) noexcept { return buf_ <= p && p <= buf_ + N; }
};

template<std::size_t N, std::size_t alignment>
template<std::size_t ReqAlign>
char* arena<N, alignment>::allocate(std::size_t n) {
    static_assert(ReqAlign <= alignment, "alignment is too small for this arena");
    assert(pointer_in_buffer(ptr_) && "short_alloc has outlived arena");
    auto const aligned_n = align_up(n);
    if (static_cast<decltype(aligned_n)>(buf_ + N - ptr_) >= aligned_n) {
        char* r = ptr_;
        ptr_ += aligned_n;
        return r;
    }

    static_assert(alignment <= alignof(std::max_align_t),
        "you've chosen an "
        "alignment that is larger than alignof(std::max_align_t), and "
        "cannot be guaranteed by normal operator new");
    return static_cast<char*>(::operator new(n));
}

template<std::size_t N, std::size_t alignment>
void arena<N, alignment>::deallocate(char* p, std::size_t n) noexcept {
    assert(pointer_in_buffer(ptr_) && "short_alloc has outlived arena");
    if (pointer_in_buffer(p)) {
        n = align_up(n);
        if (p + n == ptr_) {
            ptr_ = p;
        }
    }
    else {
        ::operator delete(p);
    }
}

template<class T, std::size_t N, std::size_t Align = alignof(std::max_align_t)>
class short_alloc {
public:
    using value_type = T;
    static auto constexpr alignment = Align;
    static auto constexpr size = N;
    using arena_type = arena<size, alignment>;

private:
    arena_type& a_;

public:
    short_alloc(const short_alloc&) = default;
    short_alloc& operator=(const short_alloc&) = delete;

    explicit short_alloc(arena_type& a) noexcept
        : a_(a) {
        static_assert(size % alignment == 0, "size N needs to be a multiple of alignment Align");
    }
    template<class U>
    explicit short_alloc(const short_alloc<U, N, alignment>& a) noexcept
        : a_(a.a_) {
    }

    template<class _Up>
    struct rebind {
        using other = short_alloc<_Up, N, alignment>;
    };

    T* allocate(std::size_t n) { return reinterpret_cast<T*>(a_.template allocate<alignof(T)>(n * sizeof(T))); }
    void deallocate(T* p, std::size_t n) noexcept { a_.deallocate(reinterpret_cast<char*>(p), n * sizeof(T)); }

    template<class T1, std::size_t N1, std::size_t A1, class U, std::size_t M, std::size_t A2>
    friend bool operator==(const short_alloc<T1, N1, A1>& x, const short_alloc<U, M, A2>& y) noexcept;

    template<class U, std::size_t M, std::size_t A>
    friend class short_alloc;
};

template<class T, std::size_t N, std::size_t A1, class U, std::size_t M, std::size_t A2>
inline bool operator==(const short_alloc<T, N, A1>& x, const short_alloc<U, M, A2>& y) noexcept {
    return N == M && A1 == A2 && &x.a_ == &y.a_;
}

template<class T, std::size_t N, std::size_t A1, class U, std::size_t M, std::size_t A2>
inline bool operator!=(const short_alloc<T, N, A1>& x, const short_alloc<U, M, A2>& y) noexcept {
    return !(x == y);
}
