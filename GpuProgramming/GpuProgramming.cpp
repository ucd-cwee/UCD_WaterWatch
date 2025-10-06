// GpuProgramming.cpp : Defines the functions for the static library.

#include <cstdarg>
#include <type_traits>
#include <tuple>
#include <ShlDisp.h>
#include <winnt.h>
#include <thread>
#include <execution>
#include <vector>
#include <iostream>
#include <map>
#include <boost/math/distributions/students_t.hpp>

#pragma region "Convenience implementation of CPU parallel computing for the conditions where GPU parallel compute is not available or not convenient."
namespace parallel {
    /// <summary>
    /// Iterator that steps through a list, without needing to instance the whole list. 
    /// </summary>
    /// <typeparam name="Type"></typeparam>
    template<typename Type = size_t>
    class sequence {
    private:
        Type min;
        Type max;
        Type step;

        static std::tuple<Type, Type, Type> DetermineSteps(Type N0, Type N1, Type Step) {
            if (Step >= 0) {
                // want to go from small to large
                if (N1 >= N0) {
                    return { N0, N1, Step };
                }
                else {
                    return { N1, N0, Step };
                }
            }
            else {
                // want to go from large to small
                if (N1 >= N0) {
                    return { N1, N0, Step };
                }
                else {
                    return { N0, N1, Step };
                }
            }
        };

    public:
        sequence(Type N0, Type N1, Type Step) {
            std::tie(min, max, step) = DetermineSteps(std::move(N0), std::move(N1), std::move(Step));
        };
        sequence() : sequence(0, 0, 1) {};
        sequence(Type N) : sequence(0, N, 1) {};
        sequence(Type N0, Type N1) : sequence(N0, N1, 1) {};

        class Iterator { // : public std::iterator<std::random_access_iterator_tag, Type>
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = Type;
            using difference_type = ptrdiff_t;
            using pointer = Type*;
            using reference = Type&;

            Iterator() : _ptr(0), _min(0), _step(1) {}
            Iterator(Type rhs, Type min, Type step) : _ptr(rhs), _min(min), _step(step) {}
            Iterator(const Iterator& rhs) : _ptr(rhs._ptr), _min(rhs._min), _step(rhs._step) {}

            inline Iterator& operator+=(difference_type rhs) { _ptr += static_cast<Type>(rhs) * _step; return *this; }
            inline Iterator& operator-=(difference_type rhs) { _ptr -= static_cast<Type>(rhs) * _step; return *this; }
            inline Type& operator*() { return _ptr; }
            inline Type* operator->() { return &_ptr; }
            inline Type operator[](difference_type rhs) { return static_cast<Type>(_min + static_cast<Type>(rhs) * _step); }
            inline const Type& operator*() const { return _ptr; }
            inline const Type* operator->() const { return &_ptr; }
            inline const Type operator[](difference_type rhs) const { return static_cast<Type>(_min + static_cast<Type>(rhs) * _step); }

            inline Iterator& operator++() { _ptr += _step; return *this; }
            inline Iterator& operator--() { _ptr -= _step; return *this; }
            inline Iterator operator++(int) { Iterator tmp(*this); _ptr += _step; return tmp; }
            inline Iterator operator--(int) { Iterator tmp(*this); _ptr -= _step; return tmp; }
            inline difference_type operator-(const Iterator& rhs) const { return (_ptr - rhs._ptr) / _step; }
            inline Iterator operator+(difference_type rhs) const { return Iterator(_ptr + static_cast<Type>(rhs) * _step, _min, _step); }
            inline Iterator operator-(difference_type rhs) const { return Iterator(_ptr - static_cast<Type>(rhs) * _step, _min, _step); }
            friend inline Iterator operator+(difference_type lhs, const Iterator& rhs) { return Iterator((static_cast<Type>(lhs) * rhs._step) + rhs._ptr, rhs._min, rhs._step); }
            friend inline Iterator operator-(difference_type lhs, const Iterator& rhs) { return Iterator((static_cast<Type>(lhs) * rhs._step) - rhs._ptr, rhs._min, rhs._step); }

            inline bool operator==(const Iterator& rhs) const { return _ptr == rhs._ptr; }
            inline bool operator!=(const Iterator& rhs) const { return _ptr != rhs._ptr; }
            inline bool operator>(const Iterator& rhs) const { return _ptr > rhs._ptr; }
            inline bool operator<(const Iterator& rhs) const { return _ptr < rhs._ptr; }
            inline bool operator>=(const Iterator& rhs) const { return _ptr >= rhs._ptr; }
            inline bool operator<=(const Iterator& rhs) const { return _ptr <= rhs._ptr; }

        protected:
            Type _min;
            Type _ptr;
            Type _step;
        };

        using iterator = Iterator;
        using const_iterator = iterator;

        auto begin() { return Iterator(min, min, step); };
        auto end() { return Iterator(max, min, step); };
        auto cbegin() const { return iterator(min, min, step); };
        auto cend() const { return iterator(max, min, step); };
        auto begin() const { return iterator(min, min, step); };
        auto end() const { return iterator(max, min, step); };
    };

    /* parallel_for (auto i = start; i < end; i++){ todo(i); }
    If the todo(i) returns anything, it will be collected into a vector at the end. */
    template<typename iteratorType, class F> decltype(auto) Std_For(iteratorType start, iteratorType end, F const& ToDo) {
        sequence<iteratorType> seq(start, end); // 0..999
        std::exception_ptr* e{ nullptr };

        std::for_each(
            std::execution::par,
            seq.begin(),
            seq.end(),
            [&](auto& x) { // copies are safer, and the resulting code will be as quick.
                try {
                    if (!e) ToDo(x);
                }
                catch (...) {
                    if (!e) {
                        auto ptr = new std::exception_ptr(std::current_exception());
                        if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&e), ptr, nullptr) != nullptr) {
                            delete ptr;
                        }
                    }
                }
            }
        );
        if (e) {
            std::exception_ptr copy{ *e };
            delete e;
            std::rethrow_exception(std::move(copy));
        }
    };
};
#pragma endregion

#pragma region "Includes and Defines"
#include "GpuProgramming.h"
#define CL_HPP_ENABLE_EXCEPTIONS
#include "../arrayfire/include/CL/opencl.hpp"
#include "opencl.hpp"

#define print(a) std::cout << a << std::endl
#define EXPECT_EQ(a, b) if (a != b){ std::cout << "FAILURE AT LINE " << __LINE__ << std::endl; }
#define EXPECT_NE(a, b) if (a == b){ std::cout << "FAILURE AT LINE " << __LINE__ << std::endl; }
#pragma endregion

class ArrayTasks {
public:
    ArrayTasks() = default;
    ArrayTasks(ArrayTasks const& rhs) = default; /*{
        if (rhs.tasks) {
            tasks = std::make_shared<std::vector<Event>>(rhs.get());            
        }
    };*/
    ArrayTasks(ArrayTasks &&) = default;
    ArrayTasks& operator=(ArrayTasks const& rhs) = default; /*{
        if (rhs.tasks) {
            if (!tasks) {
                tasks = std::make_shared<std::vector<Event>>(rhs.get());
            }
            else {
                get().insert(get().end(), rhs.tasks->begin(), rhs.tasks->end());
            }
        }
        return *this;
    };*/
    ArrayTasks& operator=(ArrayTasks&&) = default;
    ~ArrayTasks() = default;

    std::vector<Event>& get() const {
        if (!tasks) tasks = std::make_shared<std::vector<Event>>();
        return *tasks;
    };
    friend ArrayTasks operator+(ArrayTasks const& lhs, ArrayTasks const& rhs) {
        if (!lhs.tasks && !rhs.tasks) return ArrayTasks();
        else if (lhs.tasks && !rhs.tasks) return lhs;
        else if (!lhs.tasks && rhs.tasks) return rhs;
        else {
            ArrayTasks out;
            out.get() = *lhs.tasks;            
            out.get().insert(out.get().end(), rhs.tasks->begin(), rhs.tasks->end());
            return out;
        }
    };
    void wait() const {
        if (tasks) Event::waitForEvents(*tasks);
        tasks = nullptr;
    };

    ArrayTasks copy() const {
        ArrayTasks out;
        if (this->tasks) {
            out.tasks = std::make_shared<std::vector<Event>>(this->get());
        }
        return *this;
    };

    mutable std::shared_ptr<std::vector<Event>> tasks;
};

template<typename T> class Array {
public:
    template <typename G> friend class Array;

    static Device& GetDevice() {
        static Device device(select_device_with_most_flops(), get_opencl_c_code<T>(true));
        return device;
    };

public:
    std::vector<Event> events;
    std::shared_ptr<Memory<T>> data;
    ArrayTasks tasks;
    mutable bool working = false;
    mutable bool local = false;
    size_t LenX;
    size_t LenY;
    size_t LenZ;
    size_t Dim;

    Array()
        : data{ nullptr }
        , LenX{ 0ull }
        , LenY{ 0ull }
        , LenZ{ 0ull }
        , Dim{ 0ull }
        , tasks{}
        , working{ false }
        , local{ true }
    {};
    explicit Array(size_t lenX)
        : data{ std::make_shared<Memory<T>>(GetDevice(), lenX, 1) }
        , LenX{ lenX }
        , LenY{ 1ull }
        , LenZ{ 1ull }
        , Dim{ 1ull }
        , tasks{}
        , working{ false }
        , local{ true }
    {};
    explicit Array(size_t lenX, size_t lenY)
        : data{ std::make_shared<Memory<T>>(GetDevice(), lenX * lenY, 2ull) }
        , LenX{ lenX }
        , LenY{ lenY }
        , LenZ{ 1ull }
        , Dim{ std::max<size_t>(1ull, (size_t)(lenY > 1ull) + (size_t)(lenX > 1ull)) }
        , tasks{}
        , working{ false }
        , local{ true }
    {};
    explicit Array(size_t lenX, size_t lenY, size_t lenZ, bool GPU_Only = false)
        : data{ std::make_shared<Memory<T>>(GetDevice(), lenX * lenY * lenZ, 3ull, !GPU_Only, true) }
        , LenX{ lenX }
        , LenY{ lenY }
        , LenZ{ lenZ }
        , Dim{ std::max<size_t>(1ull, (size_t)(lenZ > 1ull) + (size_t)(lenY > 1ull) + (size_t)(lenX > 1ull)) }
        , tasks{}
        , working{ false }
        , local{ true }
    {};
    Array(Array const& rhs)
        : data{ rhs.data }
        , LenX{ rhs.LenX }
        , LenY{ rhs.LenY }
        , LenZ{ rhs.LenZ }
        , Dim{ rhs.Dim }
        , tasks{ rhs.tasks }
        , working{ rhs.working }
        , local{ rhs.local }
    {};
    void stop_work(bool read_from_device = true) const {
        if (this->working) {
            this->tasks.wait();
            this->working = false;
            if (read_from_device) {
                if (!local) {
                    local = true;
                    this->data->add_host_buffer();
                }
                this->data->read_from_device();
            }
        }
    }
    // call this if this vector has been manually updated on the host, or if you need the results to be finished. Slow on repeat calls.
    void sync() {
        stop_work();
        if (data) data->write_to_device();
    }
    ~Array() {
        stop_work(false);
    };

    Array copy() const {
        Array out;

        stop_work(true);

        if (this->data) {
            this->data->read_from_device();
            size_t cnt = LenX * LenY * LenZ;
            out.data = std::make_shared<Memory<T>>(GetDevice(), cnt, Dim);
            for (size_t n = 0u; n < cnt; n++) {
                out.data->operator[](n) = data->operator[](n);
            }
        }

        {
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.working = false;
            out.local = true;
        }

        out.sync();

        return out;
    };

    T& operator()(size_t x, size_t y = 0, size_t z = 0) {
        stop_work();
        return data->operator[]((z * LenY * LenX) + (y * LenX) + x);
    };
    const T& operator()(size_t x, size_t y = 0, size_t z = 0) const {
        stop_work();
        return data->operator[]((z * LenY * LenX) + (y * LenX) + x);
    };
    size_t size() const {
        return LenZ * LenY * LenX;
    }
    size_t size(unsigned int dim) const {
        if (dim == 0) return LenX;
        if (dim == 1) return LenY;
        if (dim == 2) return LenZ;
        else throw std::runtime_error("Array does not support more than 3 dimensions yet");
    }

    T& operator[](size_t n) {
        stop_work();
        return data->operator[](n);
    };
    const T& operator[](size_t n) const {
        stop_work();
        return data->operator[](n);
    };

protected:
    template<class... T> static inline void enqueue_kernel(const ulong N, const string& name, const T&... parameters) { // accepts Memory<T> objects and fundamental data type constants        
        Kernel kernel(GetDevice(), N, name, parameters...); // kernel that runs on the device
        kernel.enqueue_run();
    }
    template<class... T> static inline void enqueue_kernel(const ulong N, const unsigned int workgroup_size, const string& name, const T&... parameters) { // accepts Memory<T> objects and fundamental data type constants
        Kernel kernel(GetDevice(), N, workgroup_size, name, parameters...); // kernel that runs on the device
        kernel.enqueue_run();
    }
    static void complete_kernels() {
        GetDevice().get_cl_queue().finish();
    };
    template<class... T> inline void work(const string& name, const T&... parameters) { // accepts Memory<T> objects and fundamental data type constants
        Kernel kernel(GetDevice(), LenX * LenY * LenZ, name, parameters...);
        Event this_event;
        kernel.enqueue_run(1, &tasks.get(), &this_event);
        tasks.get().push_back(this_event);
        working = true;
    }
    template<class... T> inline void work(const unsigned int workgroup_size, const string& name, const T&... parameters) { // accepts Memory<T> objects and fundamental data type constants
        Kernel kernel(GetDevice(), LenX * LenY * LenZ, workgroup_size, name, parameters...);
        Event this_event;
        kernel.enqueue_run(1, &tasks.get(), &this_event);
        tasks.get().push_back(this_event);
        working = true;
    }

public:
    friend Array operator+(Array const& lhs, Array const& rhs) {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks + rhs.tasks;
            out.working = true;
            out.local = false;
        }
        out.work("add", *lhs.data, *rhs.data, *out.data);
        return out;
    };
    Array& operator+=(Array const& rhs) {
        this->work("add_inplace", *data, *rhs.data);
        return *this;
    };
    friend Array operator-(Array const& lhs, Array const& rhs) {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks + rhs.tasks;
            out.working = true;
            out.local = false;
        }
        out.work("sub", *lhs.data, *rhs.data, *out.data);
        return out;
    };
    Array& operator-=(Array const& rhs) {
        this->work("sub_inplace", *data, *rhs.data);
        return *this;
    };
    friend Array operator*(Array const& lhs, Array const& rhs) {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks + rhs.tasks;
            out.working = true;
            out.local = false;
        }
        out.work("mult", *lhs.data, *rhs.data, *out.data);
        return out;
    };
    Array& operator*=(Array const& rhs) {
        this->work("mult_inplace", *data, *rhs.data);
        return *this;
    };
    friend Array operator/(Array const& lhs, Array const& rhs) {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks + rhs.tasks;
            out.working = true;
            out.local = false;
        }
        out.work("divide", *lhs.data, *rhs.data, *out.data);
        return out;
    };
    Array& operator/=(Array const& rhs) {
        this->work("divide_inplace", *data, *rhs.data);
        return *this;
    };

    friend Array operator+(Array const& lhs, T rhs) {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("add_single", *lhs.data, rhs, *out.data);
        return out;
    };
    Array& operator+=(T rhs) {
        this->work("add_single_inplace", *data, rhs);
        return *this;
    };
    friend Array operator-(Array const& lhs, T rhs) {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("sub_single", *lhs.data, rhs, *out.data);
        return out;
    };
    Array& operator-=(T rhs) {
        this->work("sub_single_inplace", *data, rhs);
        return *this;
    };
    friend Array operator*(Array const& lhs, T rhs) {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("mult_single", *lhs.data, rhs, *out.data);
        return out;
    };
    Array& operator*=(T rhs) {
        this->work("mult_single_inplace", *data, rhs);
        return *this;
    };
    friend Array operator/(Array const& lhs, T rhs) {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("divide_single", *lhs.data, rhs, *out.data);
        return out;
    };
    Array& operator/=(T rhs) {
        this->work("divide_single_inplace", *data, rhs);
        return *this;
    };

    friend Array operator+(T rhs, Array const& lhs) {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("add_single", *lhs.data, rhs, *out.data);
        return out;
    };
    friend Array operator-(T rhs, Array const& lhs) {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("mult_single", *lhs.data, -1, *out.data);
        out.work("add_single", *lhs.data, rhs, *out.data);
        return out;
    };
    friend Array operator*(T rhs, Array const& lhs) {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("mult_single", *lhs.data, rhs, *out.data);
        return out;
    };
    friend Array operator/(T rhs, Array const& lhs) {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("divide_single_inv", *lhs.data, rhs, *out.data);
        return out;
    };

    Array& operator=(T rhs) {
        this->work("copy_single", *data, rhs);
        return *this;
    };

    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // specialization of POW for integer powers
    Array pown(Array<int> const& rhs) const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks + rhs.tasks;
            out.working = true;
            out.local = false;
        }
        out.work("power_n", *data, *rhs.data, *out.data);
        return out;
    };
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // power of 
    Array pow(Array const& rhs) const {
        if constexpr (std::is_same_v<T, int>) return pown(rhs);

        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks + rhs.tasks;
            out.working = true;
            out.local = false;
        }
        out.work("power", *data, *rhs.data, *out.data);
        return out;
    };
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // specialization of POW for integer powers
    Array pown(int rhs) const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("power_n_single", *data, rhs, *out.data);
        return out;
    };
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // power of 
    Array pow(T rhs) const {
        if constexpr (std::is_same_v<T, int>) return pown(rhs);

        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("power_single", *data, rhs, *out.data);
        return out;
    };
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // sqrt
    Array sqrt() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("square_root", *data, *out.data);
        return out;
    };
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // round to nearest whole number
    Array round() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("round", *data, *out.data);
        return out;
    };
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // round to higher integer
    Array ceil() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("ceil", *data, *out.data);
        return out;
    };
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // round to lower integer
    Array floor() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("flr", *data, *out.data);
        return out;
    };
    // return (this * multiply) + add;
    Array fma(Array const& multiply, Array const& add) const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks + multiply.tasks + add.tasks;
            out.working = true;
            out.local = false;
        }
        out.work("fma", *data, *multiply.data, *add.data, *out.data);
        return out;
    };
    // absolute value
    Array abs() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("absolute", *data, *out.data);
        return out;
    };
    // +1 if positive, -1 if negative
    Array sign() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Sign", *data, *out.data);
        return out;
    };

    Array sin() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Sin", *data, *out.data);
        return out;
    };
    Array cos() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Cos", *data, *out.data);
        return out;
    };
    Array tan() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Tan", *data, *out.data);
        return out;
    };
    Array asin() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("aSin", *data, *out.data);
        return out;
    };
    Array acos() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("aCos", *data, *out.data);
        return out;
    };
    Array atan() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("aTan", *data, *out.data);
        return out;
    };
    Array sinh() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Sinh", *data, *out.data);
        return out;
    };
    Array cosh() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Cosh", *data, *out.data);
        return out;
    };
    Array tanh() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Tanh", *data, *out.data);
        return out;
    };
    Array asinh() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("aSinh", *data, *out.data);
        return out;
    };
    Array acosh() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("aCosh", *data, *out.data);
        return out;
    };
    Array atanh() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("aTanh", *data, *out.data);
        return out;
    };

    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // e^x
    Array exp() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Exp", *data, *out.data);
        return out;
    };
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // 2^x
    Array exp2() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Exp2", *data, *out.data);
        return out;
    };
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // 10^x
    Array exp10() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Exp10", *data, *out.data);
        return out;
    };
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // e^x-1
    Array expm1() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Expm1", *data, *out.data);
        return out;
    };
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // log gamma function
    Array lgamma() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Lgamma", *data, *out.data);
        return out;
    };
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // ln(x)
    Array log() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Log", *data, *out.data);
        return out;
    };
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // log_2(x)
    Array log2() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Log2", *data, *out.data);
        return out;
    };
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // log_10(x)
    Array log10() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Log10", *data, *out.data);
        return out;
    };
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // ln(1+x)
    Array log1p() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Log1p", *data, *out.data);
        return out;
    };

    // return this % rhs
    Array mod(Array const& rhs) const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks + rhs.tasks;
            out.working = true;
            out.local = false;
        }
        out.work("Mod", *data, *rhs.data, *out.data);
        return out;
    };
    // return this % rhs
    Array mod(T rhs) const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Mod_single", *data, rhs, *out.data);
        return out;
    };
    friend Array operator%(Array const& lhs, Array const& rhs) {
        return lhs.mod(rhs);
    };
    friend Array operator%(Array const& lhs, T rhs) {
        return lhs.mod(rhs);
    };

    Array max(Array const& rhs) const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks + rhs.tasks;
            out.working = true;
            out.local = false;
        }
        out.work("Max", *data, *rhs.data, *out.data);
        return out;
    };
    Array max(T rhs) const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Max_single", *data, rhs, *out.data);
        return out;
    };
    Array min(Array const& rhs) const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks + rhs.tasks;
            out.working = true;
            out.local = false;
        }
        out.work("Min", *data, *rhs.data, *out.data);
        return out;
    };
    Array min(T rhs) const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks.copy();
            out.working = true;
            out.local = false;
        }
        out.work("Min_single", *data, rhs, *out.data);
        return out;
    };

    Array<unsigned int> operator!() const {
        Array<unsigned int > out; {
            out.data = std::make_shared<Memory<unsigned int >>(Array<unsigned int >::GetDevice(), this->LenX * this->LenY * this->LenZ, this->Dim, false, true);
            out.LenX = this->LenX;
            out.LenY = this->LenY;
            out.LenZ = this->LenZ;
            out.Dim = this->Dim;
            out.tasks = this->tasks.copy();
            out.working = true;
            out.local = false;
        }
        Kernel kernel(this->GetDevice(), LenX * LenY * LenZ, "item_not", *out.data, *data);
        Event this_event;
        kernel.enqueue_run(1, &this->tasks.get(), &this_event);
        out.tasks.get().push_back(this_event);
        working = true;
        return out;
    };
    Array<unsigned int> operator==(T rhs) const {
        Array<unsigned int > out; {
            out.data = std::make_shared<Memory<unsigned int >>(Array<unsigned int >::GetDevice(), this->LenX * this->LenY * this->LenZ, this->Dim, false, true);
            out.LenX = this->LenX;
            out.LenY = this->LenY;
            out.LenZ = this->LenZ;
            out.Dim = this->Dim;
            out.tasks = this->tasks.copy();
            out.working = true;
            out.local = false;
        }
        
        Kernel kernel(this->GetDevice(), LenX * LenY * LenZ, "item_eq_single", *data, rhs, *out.data);
        Event this_event;
        kernel.enqueue_run(1, &this->tasks.get(), &this_event);
        out.tasks.get().push_back(this_event);
        working = true;
        return out;
    };
    Array<unsigned int> operator!=(T rhs) const {
        Array<unsigned int > out; {
            out.data = std::make_shared<Memory<unsigned int >>(Array<unsigned int>::GetDevice(), this->LenX * this->LenY * this->LenZ, this->Dim, false, true);
            out.LenX = this->LenX;
            out.LenY = this->LenY;
            out.LenZ = this->LenZ;
            out.Dim = this->Dim;
            out.tasks = this->tasks.copy();
            out.working = true;
            out.local = false;
        }

        Kernel kernel(this->GetDevice(), LenX * LenY * LenZ, "item_neq_single", *data, rhs, *out.data);
        Event this_event;
        kernel.enqueue_run(1, &this->tasks.get(), &this_event);
        out.tasks.get().push_back(this_event);
        working = true;
        return out;
    };
    friend Array<unsigned int> operator==(Array const& lhs, Array const& rhs) {
        Array<unsigned int> out; {
            out.data = std::make_shared<Memory<unsigned int>>(Array<unsigned int>::GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks + rhs.tasks;
            out.working = true;
            out.local = false;
        }

        Kernel kernel(Array<T>::GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, "item_eq", *lhs.data, *rhs.data, *out.data);
        Event this_event;
        kernel.enqueue_run(1, &out.tasks.get(), &this_event);
        out.tasks.get().push_back(this_event);
        out.working = true;
        return out;
    };
    friend Array<unsigned int> operator!=(Array const& lhs, Array const& rhs) {
        Array<unsigned int> out; {
            out.data = std::make_shared<Memory<unsigned int>>(Array<unsigned int>::GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks + rhs.tasks;
            out.working = true;
            out.local = false;
        }

        Kernel kernel(Array<T>::GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, "item_neq", *lhs.data, *rhs.data, *out.data);
        Event this_event;
        kernel.enqueue_run(1, &out.tasks.get(), &this_event);
        out.tasks.get().push_back(this_event);
        out.working = true;
        return out;
    };
    Array<unsigned int> operator<(T rhs) const {
        Array<unsigned int > out; {
            out.data = std::make_shared<Memory<unsigned int >>(Array<unsigned int >::GetDevice(), this->LenX * this->LenY * this->LenZ, this->Dim, false, true);
            out.LenX = this->LenX;
            out.LenY = this->LenY;
            out.LenZ = this->LenZ;
            out.Dim = this->Dim;
            out.tasks = this->tasks.copy();
            out.working = true;
            out.local = false;
        }

        Kernel kernel(this->GetDevice(), LenX * LenY * LenZ, "item_ls_single", *data, rhs, *out.data);
        Event this_event;
        kernel.enqueue_run(1, &this->tasks.get(), &this_event);
        out.tasks.get().push_back(this_event);
        working = true;
        return out;
    };
    Array<unsigned int> operator<=(T rhs) const {
        Array<unsigned int > out; {
            out.data = std::make_shared<Memory<unsigned int >>(Array<unsigned int>::GetDevice(), this->LenX * this->LenY * this->LenZ, this->Dim, false, true);
            out.LenX = this->LenX;
            out.LenY = this->LenY;
            out.LenZ = this->LenZ;
            out.Dim = this->Dim;
            out.tasks = this->tasks.copy();
            out.working = true;
            out.local = false;
        }

        Kernel kernel(this->GetDevice(), LenX * LenY * LenZ, "item_lse_single", *data, rhs, *out.data);
        Event this_event;
        kernel.enqueue_run(1, &this->tasks.get(), &this_event);
        out.tasks.get().push_back(this_event);
        working = true;
        return out;
    };
    friend Array<unsigned int> operator<(Array const& lhs, Array const& rhs) {
        Array<unsigned int> out; {
            out.data = std::make_shared<Memory<unsigned int>>(Array<unsigned int>::GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks + rhs.tasks;
            out.working = true;
            out.local = false;
        }

        Kernel kernel(Array<T>::GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, "item_ls", *lhs.data, *rhs.data, *out.data);
        Event this_event;
        kernel.enqueue_run(1, &out.tasks.get(), &this_event);
        out.tasks.get().push_back(this_event);
        out.working = true;
        return out;
    };
    friend Array<unsigned int> operator<=(Array const& lhs, Array const& rhs) {
        Array<unsigned int> out; {
            out.data = std::make_shared<Memory<unsigned int>>(Array<unsigned int>::GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks + rhs.tasks;
            out.working = true;
            out.local = false;
        }

        Kernel kernel(Array<T>::GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, "item_lse", *lhs.data, *rhs.data, *out.data);
        Event this_event;
        kernel.enqueue_run(1, &out.tasks.get(), &this_event);
        out.tasks.get().push_back(this_event);
        out.working = true;
        return out;
    };
    Array<unsigned int> operator>(T rhs) const {
        Array<unsigned int > out; {
            out.data = std::make_shared<Memory<unsigned int >>(Array<unsigned int >::GetDevice(), this->LenX * this->LenY * this->LenZ, this->Dim, false, true);
            out.LenX = this->LenX;
            out.LenY = this->LenY;
            out.LenZ = this->LenZ;
            out.Dim = this->Dim;
            out.tasks = this->tasks.copy();
            out.working = true;
            out.local = false;
        }

        Kernel kernel(this->GetDevice(), LenX * LenY * LenZ, "item_gr_single", *data, rhs, *out.data);
        Event this_event;
        kernel.enqueue_run(1, &this->tasks.get(), &this_event);
        out.tasks.get().push_back(this_event);
        working = true;
        return out;
    };
    Array<unsigned int> operator>=(T rhs) const {
        Array<unsigned int > out; {
            out.data = std::make_shared<Memory<unsigned int >>(Array<unsigned int>::GetDevice(), this->LenX * this->LenY * this->LenZ, this->Dim, false, true);
            out.LenX = this->LenX;
            out.LenY = this->LenY;
            out.LenZ = this->LenZ;
            out.Dim = this->Dim;
            out.tasks = this->tasks.copy();
            out.working = true;
            out.local = false;
        }

        Kernel kernel(this->GetDevice(), LenX * LenY * LenZ, "item_gre_single", *data, rhs, *out.data);
        Event this_event;
        kernel.enqueue_run(1, &this->tasks.get(), &this_event);
        out.tasks.get().push_back(this_event);
        working = true;
        return out;
    };
    friend Array<unsigned int> operator>(Array const& lhs, Array const& rhs) {
        Array<unsigned int> out; {
            out.data = std::make_shared<Memory<unsigned int>>(Array<unsigned int>::GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks + rhs.tasks;
            out.working = true;
            out.local = false;
        }

        Kernel kernel(Array<T>::GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, "item_gr", *lhs.data, *rhs.data, *out.data);
        Event this_event;
        kernel.enqueue_run(1, &out.tasks.get(), &this_event);
        out.tasks.get().push_back(this_event);
        out.working = true;
        return out;
    };
    friend Array<unsigned int> operator>=(Array const& lhs, Array const& rhs) {
        Array<unsigned int> out; {
            out.data = std::make_shared<Memory<unsigned int>>(Array<unsigned int>::GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks + rhs.tasks;
            out.working = true;
            out.local = false;
        }

        Kernel kernel(Array<T>::GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, "item_gre", *lhs.data, *rhs.data, *out.data);
        Event this_event;
        kernel.enqueue_run(1, &out.tasks.get(), &this_event);
        out.tasks.get().push_back(this_event);
        out.working = true;
        return out;
    };

    // grow a matrix by wrapping the new values around to the start. Only works for a 1-D vector. 
    Array grow_by_wrapping(size_t new_length) const {
        if (this->Dim == 1) {
            Array out; {
                out.data = std::make_shared<Memory<T>>(GetDevice(), new_length * 1 * 1, 1, false, true);
                out.LenX = new_length;
                out.LenY = 1;
                out.LenZ = 1;
                out.Dim = 1;
                out.tasks = tasks.copy();
                out.working = true;
                out.local = false;
            }
            out.work("wrap_around", *out.data, *data, this->size());
            return out;
        }
        else {
            // ??
            throw std::runtime_error("Cannot grow a matrix by wrapping -- yet. Depends on how we want to grow it? Y-axis growth is off, but X-axis growth makes sense with wrapping");
        }        
    };
    // create a new array by sampling this array at the provided indices. E.g. This = [5,4,3,2,1,0]
    // Indices = [5,5,5,5,5,5,5,4,4,4,4,4,4,3,3,3,3,3,2,2,2,2,1,1,1,0,0]
    // Result = [0,0,0,0,0,0,0,1,1,1,1,1,2,2,2,2,2,3,3,3,3,4,4,4,5,5]
    Array resample(Array<unsigned int> const& sample_indices) const {
        Array out; {
            out.tasks = this->tasks + sample_indices.tasks;
            out.working = true;
            out.local = false;
            out.LenX = sample_indices.LenX;
            out.LenY = sample_indices.LenY;
            out.LenZ = sample_indices.LenZ;
            out.Dim = std::max<size_t>(1ull, (size_t)(out.LenZ > 1ull) + (size_t)(out.LenY > 1ull) + (size_t)(out.LenX > 1ull));
            out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), out.LenX * out.LenY * out.LenZ, out.Dim, false, true);
        }
        out.work("resample", *out.data, *data, *sample_indices.data);
        return out;     
    };

protected:
    //std::tuple<size_t, size_t, size_t> position_to_coordinate(size_t pos) const {
    //    const auto Z = std::floor(pos / (LenY * LenX));
    //    const auto pos2 = pos - Z * (LenY * LenX);
    //    const auto Y = std::floor(pos2 / (LenX));
    //    return { pos2 - Y * LenX, Y, Z };
    //};
    //void position_to_coordinate(std::tuple<size_t, size_t, size_t>& out, size_t pos) const {
    //    const auto Z = std::floor(pos / (LenY * LenX));
    //    const auto pos2 = pos - Z * (LenY * LenX);
    //    const auto Y = std::floor(pos2 / (LenX));
    //    std::get<0>(out) = pos2 - Y * LenX;
    //    std::get<1>(out) = Y;
    //    std::get<2>(out) = Z;
    //};
    //size_t coordinate_to_position(std::tuple<size_t, size_t, size_t> const& pos) const {
    //    return (std::get<2>(pos) * LenY * LenX) + (std::get<1>(pos) * LenX) + std::get<0>(pos);
    //};
    //size_t coordinate_to_position(size_t x, size_t y, size_t z) const {
    //    return (z * LenY * LenX) + (y * LenX) + x;
    //};

public:
    // joins two matrices along one of the dimensions.
    Array join(unsigned int jdim, Array const& first) const {        
        // All dimensions except join dimension must be equal
        for (unsigned int I = 0; I < 3; ++I) {
            if (I == jdim) continue;
            if (this->size(I) != first.size(I)) {
                return Array();
            }
        }

        // Compute output dims
        size_t
            NewX = this->size(0) + first.size(0) * (jdim == 0),
            NewY = this->size(1) + first.size(1) * (jdim == 1),
            NewZ = this->size(2) + first.size(2) * (jdim == 2);

        Array out;
        out.tasks = this->tasks + first.tasks;
        out.working = true;
        out.local = false;
        if (NewZ != 1) {            
            out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), NewX * NewY * NewZ, 3, false, true);
            out.LenX = NewX;
            out.LenY = NewY;
            out.LenZ = NewZ;
            out.Dim = 3;            
        } 
        else if (NewY != 1) {             
            out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), NewX * NewY, 2, false, true);
            out.LenX = NewX;
            out.LenY = NewY;
            out.LenZ = 1;
            out.Dim = 2;
        }
        else {
            out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), NewX, 1, false, true);
            out.LenX = NewX;
            out.LenY = 1;
            out.LenZ = 1;
            out.Dim = 1;
        }        

        Array<unsigned int> lengths(9); {
            lengths[0] = this->LenX;
            lengths[1] = this->LenY;
            lengths[2] = this->LenZ;
            lengths[3] = first.LenX;
            lengths[4] = first.LenY;
            lengths[5] = first.LenZ;
            lengths[6] = out.LenX;
            lengths[7] = out.LenY;
            lengths[8] = out.LenZ;
            lengths.sync();
        }
        if (jdim == 0) {
            out.work("join_dim_0", *out.data, *this->data, (unsigned int)this->LenX, (unsigned int)this->LenY, (unsigned int)this->LenZ, *first.data, (unsigned int)first.LenX);
            return out;
        }
        else if (jdim == 1) {
            out.work("join_dim_1", *out.data, *this->data, (unsigned int)this->LenX, (unsigned int)this->LenY, (unsigned int)this->LenZ, *first.data, (unsigned int)first.LenY);
            return out;
        }
        else {
            out.work("join_dim_2", *out.data, *this->data, (unsigned int)this->LenX, (unsigned int)this->LenY, (unsigned int)this->LenZ, *first.data);
            return out;
        }

        return out;
    };

    // extract a row from this 2-D matrix as a 1-D array
    Array row(unsigned int rowN) const {
        Array out; {
            out.tasks = this->tasks.copy();
            out.working = true;
            out.local = false;
            out.LenX = LenY;
            out.LenY = LenZ;
            out.LenZ = 1;
            out.Dim = std::max<size_t>(1ull, (size_t)(out.LenZ > 1ull) + (size_t)(out.LenY > 1ull) + (size_t)(out.LenX > 1ull));
            out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), out.LenX * out.LenY * out.LenZ, out.Dim, false, true);
        }
        Kernel kernel(this->GetDevice(), out.size(), "row_of", *out.data, *this->data, (unsigned int)rowN, (unsigned int)this->LenX, (unsigned int)this->LenY, (unsigned int)this->LenZ);
        Event this_event;
        kernel.enqueue_run(1, &this->tasks.get(), &this_event);
        this->tasks.get().push_back(this_event);
        out.tasks.get().push_back(this_event);
        out.working = true;

        return out;
    };

    // transpose a 2-D matrix along its diagonal. Does not support transposition of 3-D matrices. 
    Array transpose() const {
        // matrix must be 2-D
        if (this->Dim == 0) return Array();
        else if (this->Dim > 2) return Array();

        Array out;
        out.tasks = this->tasks.copy();
        out.working = true;
        out.local = false;
        out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), LenX * LenY, 2, false, true);
        out.LenX = LenY;
        out.LenY = LenX;
        out.LenZ = 1;
        out.Dim = 2;

        Kernel kernel(this->GetDevice(), out.size(), "Transpose", *out.data, *this->data, (unsigned int)LenX, (unsigned int)LenY);
        Event this_event;
        kernel.enqueue_run(1, &this->tasks.get(), &this_event);
        this->tasks.get().push_back(this_event);
        out.tasks.get().push_back(this_event);
        out.working = true;

        // out.work("Transpose", *out.data, *this->data, (unsigned int)LenX, (unsigned int)LenY);

        return out;
    };

    // pad a matrix with zeros to make its X and Y components square. Used for calculating the inverse. 
    Array make_square() const {
        unsigned int len = std::max<unsigned int>(LenY, LenX);

        Array out;
        out.tasks = this->tasks.copy();
        out.working = true;
        out.local = false;
        out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), len * len * LenZ, std::max<unsigned int>(2, Dim), false, true);
        out.LenX = len;
        out.LenY = len;
        out.LenZ = LenZ;
        out.Dim = std::max<unsigned int>(2, Dim);

        out.work("make_square", *out.data, *this->data, (unsigned int)this->LenX, (unsigned int)this->LenY, (unsigned int)this->LenZ, (unsigned int)len);

        return out;
    }

    template<typename = std::enable_if_t<std::is_floating_point_v<T>>>    
    float determinant() const { // calculate the determinant for a square matrix
        if (this->LenX != this->LenY) {
            return 1; 
        }
        size_t dimension = this->LenX;

        if (dimension == 0) {
            return 1;
        }
        else if (dimension == 1) {
            return operator[](0);
        }
        else if (dimension == 2) {
            return (*this)(0, 0) * (*this)(1, 1) - (*this)(0, 1) * (*this)(1, 0);
        }
        else {
            float result = 0;
            int sign = 1;

            Array subVect; {
                subVect.working = false;
                subVect.local = true;
                subVect.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), (dimension - 1) * (dimension - 1), 2, true, false);
                subVect.LenX = dimension - 1;
                subVect.LenY = dimension - 1;
                subVect.LenZ = 1;
                subVect.Dim = 2;
            }

            for (size_t i = 0; i < dimension; ++i) {
                // build a sub-matrix
                for (int m = 1; m < dimension; m++) {
                    int z = 0;
                    for (int n = 0; n < dimension; n++) {
                        if (n != i) {
                            subVect(m - 1, z) = (*this)(m, n);
                            z++;
                        }
                    }
                }

                //recursive call
                result += sign * (*this)(0,i) * subVect.determinant();
                sign = -sign;
            }

            return result;
        }        
    }

    // cofactor of a square matrix, essential for calculating the inverse
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>>
    Array cofactor() const {
        if (this->LenX != this->LenY) {
            return make_square().cofactor();
        }
        size_t dimension = this->LenX;
        Array solution; {
            solution.tasks = this->tasks.copy();
            solution.working = this->working;
            solution.local = true;
            solution.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), (dimension) * (dimension), 2, true, true);
            solution.LenX = dimension;
            solution.LenY = dimension;
            solution.LenZ = 1;
            solution.Dim = 2;
        }
        Array subVect; {
            subVect.working = false;
            subVect.local = true;
            subVect.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), (dimension-1) * (dimension-1), 2, true, false);
            subVect.LenX = dimension-1;
            subVect.LenY = dimension-1;
            subVect.LenZ = 1;
            subVect.Dim = 2;
        }

        for (std::size_t i = 0; i < dimension; i++) {
            for (std::size_t j = 0; j < dimension; j++) {
                int p = 0;
                for (size_t x = 0; x < dimension; x++) {
                    if (x == i) {
                        continue;
                    }
                    int q = 0;

                    for (size_t y = 0; y < dimension; y++) {
                        if (y == j) {
                            continue;
                        }
                        subVect(p,q) = (*this)(x,y);
                        q++;
                    }
                    p++;
                }
                solution(i,j) = std::pow<float>(-1, i + j) * subVect.determinant();
            }
        }
        solution.sync();
        return solution;
    }

    // transpose of the cofactor of a square matrix
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>>
    Array adjoint() const {
        return cofactor().transpose();
    };

    // solve for the inverse of the matrix. Does not support solving for the inverse of a 3-D matrix. 
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>>
    Array inverse() const {
        return adjoint() / std::abs(determinant());
    };

    // performs a cross-multiplication of two square matrices. This is not accelerated by the GPU, and is CPU-bound.
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>>
    Array matrix_multiply(Array const& rhs) const {
        if (this->LenY == rhs.LenX) {
            // only useful for dim-2 matrices. 
            constexpr static bool always_cpu = true;
            if (always_cpu || ((this->LenY > this->LenX) && (this->LenY > 500))) {
                // this requires using the CPU, rather than the GPU.
                this->stop_work();
                rhs.stop_work();

                size_t final_num_rows = this->LenX;
                size_t final_num_cols = rhs.LenY;

                Array out;
                out.working = false;
                out.local = true;
                out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), final_num_rows * final_num_cols, 2, true, true);
                out.LenX = final_num_rows;
                out.LenY = final_num_cols;
                out.LenZ = 1;
                out.Dim = 2;

                parallel::Std_For<size_t>(0, out.size(), [&](size_t n) {
                    T v = (T)0;
                    const size_t destination_Y = (uint)std::floor((T)n / (T)final_num_rows);
                    const size_t destination_X = n - (final_num_rows * destination_Y);
                    const size_t LHS_X = destination_X; // row from LHS		
                    const size_t RHS_Y = destination_Y; // column from RHS                    
                    for (unsigned int index = 0; index < this->LenY; ++index) {
                        const size_t LHS_n = index * this->LenX + LHS_X;
                        const size_t RHS_n = RHS_Y * rhs.LenX + index;
                        v += (*this->data)[index * this->LenX + LHS_X] * (*rhs.data)[RHS_Y * rhs.LenX + index];
                    }
                    (*out.data)[n] = v;
                });
                out.sync();
                return out;
            }
            else {
                // this may benefit from using the GPU, rather than the CPU. 
                unsigned int final_num_rows = this->LenX;
                unsigned int final_num_cols = rhs.LenY;
                Array out;
                out.tasks = this->tasks + rhs.tasks;
                out.working = true;
                out.local = false;
                out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), final_num_rows * final_num_cols, 2, false, true);
                out.LenX = final_num_rows;
                out.LenY = final_num_cols;
                out.LenZ = 1;
                out.Dim = 2;
                out.work("matx_mult_2",
                    *out.data, (unsigned int)final_num_rows, (unsigned int)final_num_cols,
                    *this->data, (unsigned int)this->LenX, (unsigned int)this->LenY,
                    *rhs.data, (unsigned int)rhs.LenX, (unsigned int)rhs.LenY
                );
                return out;
            }  
        }
        else if (this->LenY > rhs.LenX) {
            return matrix_multiply(rhs.copy().join(0, Array(this->LenY - rhs.LenX, rhs.LenY, rhs.LenZ) = 1));
        }
        else /*if (this->LenY < rhs.LenX)*/ {
            // To-Do: need to set final column in joining array to 1?
            return this->copy().join(1, Array(this->LenX, rhs.LenX - this->LenY, this->LenZ) = 0).matrix_multiply(rhs);
        }
    }; 

    // test to see if there is any colinearity in the feature set. If so, it is impossible to solve for the linear regression. One or multiple features must be removed until it is no longer invalid.
    template<typename = std::enable_if_t<std::is_floating_point_v<T>>>
    bool is_colinear() const {
        return std::abs(this->transpose().matrix_multiply(*this).determinant()) == 0;
    };

private:
    Array sum_iteration() const {
        if (this->size() > 1000) {
            Array out; {
                out.tasks = this->tasks.copy();
                out.working = this->working;
                out.local = false;
                out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), std::ceilf((float)(LenX * LenY * LenZ) / (float)64), 1, false, true);
                out.LenX = std::ceilf((float)(LenX * LenY * LenZ) / (float)64);
                out.LenY = 1;
                out.LenZ = 1;
                out.Dim = 1;
            }
            Array scratch; {
                scratch.working = false;
                scratch.local = false;
                scratch.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), 65, Dim, false, true);
                scratch.LenX = 65;
                scratch.LenY = 1;
                scratch.LenZ = 1;
                scratch.Dim = Dim;
            }

            Kernel kernel(this->GetDevice(), this->size(), "reduce_sum", *data, *out.data, *scratch.data, this->size());
            Event this_event;
            kernel.enqueue_run(1, &out.tasks.get(), &this_event);
            out.tasks.get().push_back(this_event);
            out.working = true;
            // out.work("reduce_sum", *data, *out.data, *scratch.data, this->size());
            return out;
        }
        else {
            return *this;
        }
    };
    Array max_iteration() const {
        if (this->size() > 1000) {
            Array out; {
                out.tasks = this->tasks.copy();
                out.working = this->working;
                out.local = false;
                out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), std::ceilf((float)(LenX * LenY * LenZ) / (float)64), 1, false, true);
                out.LenX = std::ceilf((float)(LenX * LenY * LenZ) / (float)64);
                out.LenY = 1;
                out.LenZ = 1;
                out.Dim = 1;
            }
            Array scratch; {
                scratch.working = false;
                scratch.local = false;
                scratch.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), 65, Dim, false, true);
                scratch.LenX = 65;
                scratch.LenY = 1;
                scratch.LenZ = 1;
                scratch.Dim = Dim;
            }

            Kernel kernel(this->GetDevice(), this->size(), "reduce_max", *data, *out.data, *scratch.data, this->size(), std::numeric_limits<T>::lowest());
            Event this_event;
            kernel.enqueue_run(1, &out.tasks.get(), &this_event);
            out.tasks.get().push_back(this_event);
            out.working = true;
            // out.work("reduce_sum", *data, *out.data, *scratch.data, this->size());
            return out;
        }
        else {
            return *this;
        }
    };
    Array min_iteration() const {
        if (this->size() > 1000) {
            Array out; {
                out.tasks = this->tasks.copy();
                out.working = this->working;
                out.local = false;
                out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), std::ceilf((float)(LenX * LenY * LenZ) / (float)64), 1, false, true);
                out.LenX = std::ceilf((float)(LenX * LenY * LenZ) / (float)64);
                out.LenY = 1;
                out.LenZ = 1;
                out.Dim = 1;
            }
            Array scratch; {
                scratch.working = false;
                scratch.local = false;
                scratch.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), 65, Dim, false, true);
                scratch.LenX = 65;
                scratch.LenY = 1;
                scratch.LenZ = 1;
                scratch.Dim = Dim;
            }

            Kernel kernel(this->GetDevice(), this->size(), "reduce_min", *data, *out.data, *scratch.data, this->size(), std::numeric_limits<T>::max());
            Event this_event;
            kernel.enqueue_run(1, &out.tasks.get(), &this_event);
            out.tasks.get().push_back(this_event);
            out.working = true;
            // out.work("reduce_sum", *data, *out.data, *scratch.data, this->size());
            return out;
        }
        else {
            return *this;
        }
    };

public:
    T sum() const {
        Array t = sum_iteration();
        while (t.size() > 10000) {
            t = t.sum_iteration();
        }
        t.stop_work();
        auto N = t.size();
        T out = (T)0;
        for (size_t n = 0; n < N; ++n) {
            out += t.data->operator[](n);
        }
        return out;
    };
    T avg() const {
        return (T)((double)sum() / (double)this->size());
    };
    T max() const {
        Array t = max_iteration();
        while (t.size() > 10000) {
            t = t.max_iteration();
        }
        t.stop_work();
        auto N = t.size();
        T out = std::numeric_limits<T>::lowest();
        for (size_t n = 0; n < N; ++n) {
            out = std::max(out, t.data->operator[](n));
        }
        return out;
    };
    T min() const {
        Array t = min_iteration();
        while (t.size() > 10000) {
            t = t.min_iteration();
        }
        t.stop_work();
        auto N = t.size();
        T out = (T)std::numeric_limits<T>::max();
        for (size_t n = 0; n < N; ++n) {
            out = std::min(out, t.data->operator[](n));
        }
        return out;
    };

    // extracts the diagonal of a 2-D matrix as a 1-D array
    Array diagonal() const {
        if (this->Dim == 0) return *this;
        else if (this->Dim == 1) return *this;
        else if (this->Dim > 2) return Array();

        Array out; {
            out.tasks = this->tasks.copy();
            out.working = true;
            out.local = false;
            out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), std::min<size_t>(LenX, LenY), 1, false, true);
            out.LenX = std::min<size_t>(LenX, LenY);
            out.LenY = 1;
            out.LenZ = 1;
            out.Dim = 1;
        }
        
        Kernel kernel(this->GetDevice(), this->size(), "diagonal", *out.data, *data, this->LenX);
        Event this_event;
        kernel.enqueue_run(1, &out.tasks.get(), &this_event);
        out.tasks.get().push_back(this_event);
        return out;
    };
    // cast from the current type to the requested type. E.g. from int to float, or char to unsigned long, etc.
    template<typename G> Array<G> cast() const {
        if constexpr (std::is_same_v<G, T>) {
            return *this;
        }

        Array<G> out; {
            out.data = std::make_shared<Memory<G>>(Array<G>::GetDevice(), this->LenX * this->LenY * this->LenZ, this->Dim, false, true);
            out.LenX = this->LenX;
            out.LenY = this->LenY;
            out.LenZ = this->LenZ;
            out.Dim = this->Dim;
            out.tasks = this->tasks.copy();
            out.working = true;
            out.local = false;
        }

        out.work(std::string("from_") + type_name<T>(), *out.data, *this->data);

        return out;
    };
    // For floating-point values, returns 0-1. For all others, returns the range from 0 to the max value. 
    static Array random(size_t lenX, size_t lenY = 1, size_t lenZ = 1) {
        if constexpr (std::is_floating_point_v<T> || std::is_same_v<unsigned int, T>) {
            int num_dim = std::max<int>(1, ((int)(lenZ > 1) + (int)(lenY > 1) + (int)(lenX > 1)));
            Array<T> out; {
                out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), lenX * lenY * lenZ, num_dim, false, true);
                out.LenX = lenX;
                out.LenY = lenY;
                out.LenZ = lenZ;
                out.Dim = num_dim;
                out.working = true;
                out.local = false;
            }
            out.work("Rand", *out.data);
            return out;
        }
        else {
            int num_dim = std::max<int>(1, ((int)(lenZ > 1) + (int)(lenY > 1) + (int)(lenX > 1)));
            Array<float> out; {
                out.data = std::make_shared<Memory<float>>(Array<float>::GetDevice(), lenX * lenY * lenZ, num_dim, false, true);
                out.LenX = lenX;
                out.LenY = lenY;
                out.LenZ = lenZ;
                out.Dim = num_dim;
                out.working = true;
                out.local = false;
            }
            out.work("Rand", *out.data);
            out *= std::numeric_limits<T>::max();
            return out.cast<T>();
        }
    };
    // returns a random number in the range of (lower, upper]
    static Array random_between(T lower, T upper, size_t lenX, size_t lenY = 1, size_t lenZ = 1) {
        if constexpr (std::is_floating_point_v<T>) {
            int num_dim = std::max<int>(1, ((int)(lenZ > 1) + (int)(lenY > 1) + (int)(lenX > 1)));
            Array<T> out; {
                out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), lenX * lenY * lenZ, num_dim, false, true);
                out.LenX = lenX;
                out.LenY = lenY;
                out.LenZ = lenZ;
                out.Dim = num_dim;
                out.working = true;
                out.local = false;
            }
            out.work("Rand", *out.data);
            out *= (upper - lower);
            out += lower;
            return out;
        }
        else {
            int num_dim = std::max<int>(1, ((int)(lenZ > 1) + (int)(lenY > 1) + (int)(lenX > 1)));
            Array<float> out; {
                out.data = std::make_shared<Memory<float>>(Array<float>::GetDevice(), lenX * lenY * lenZ, num_dim, false, true);
                out.LenX = lenX;
                out.LenY = lenY;
                out.LenZ = lenZ;
                out.Dim = num_dim;
                out.working = true;
                out.local = false;
            }
            out.work("Rand", *out.data);
            out *= (upper - lower);
            out += lower;
            return out.cast<T>();
        }
    };
    // Returns a square 2-d matrix whose values are 1.0 along the diagonal, and 0.0 elsewhere.
    static Array identity(size_t width) {
        Array<T> out; {
            out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), width * width * 1, 2, false, true);
            out.LenX = width;
            out.LenY = width;
            out.LenZ = 1;
            out.Dim = 2;
            out.working = true;
            out.local = false;
        }
        out.work("identity", *out.data, width);
        return out;
    };
    // Returns a matrix with all values equal to the provided value
    static Array constant(T value, size_t lenX, size_t lenY = 1, size_t lenZ = 1) {
        int num_dim = std::max<int>(1, ((int)(lenZ > 1) + (int)(lenY > 1) + (int)(lenX > 1)));

        Array<T> out; {
            out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), lenX * lenY * lenZ, num_dim, false, true);
            out.LenX = lenX;
            out.LenY = lenY;
            out.LenZ = lenZ;
            out.Dim = num_dim;
            out.working = true;
            out.local = false;
        }
        out.work("copy_single", *out.data, value);
        return out;
    };
    // Returns a matrix with all values linearly increasing from the low value to the high value based on their index. 
    static Array linear(T low, T high, size_t lenX, size_t lenY = 1, size_t lenZ = 1) {
        int num_dim = std::max<int>(1, ((int)(lenZ > 1) + (int)(lenY > 1) + (int)(lenX > 1)));

        Array<T> out; {
            out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), lenX * lenY * lenZ, num_dim, false, true);
            out.LenX = lenX;
            out.LenY = lenY;
            out.LenZ = lenZ;
            out.Dim = num_dim;
            out.working = true;
            out.local = false;
        }
        out.work("linear_between", *out.data, low, high, (unsigned long)out.size());
        return out;
    };
    // For floating-point values, returns 0-1. For all others, returns the range from 0 to the max value. 
    template <typename P> static Array from_vector(const P& parameters) {
        size_t count=0;
        for (auto& x : parameters) {
            ++count;
        }
        Array out(count);
        count = 0;
        for (auto& x : parameters) {
            out[count++] = static_cast<T>(x);
        }
        out.sync();
        return out;
    };

private:
    std::string to_string_impl(size_t x) const {
        if constexpr (std::is_same_v<char, T> || std::is_same_v<unsigned char, T>) {
            auto c = this->operator[](x);
            if (std::isalpha(c) || std::isdigit(c)) {
                return "'" + std::string(1, c) + "'";
            }
            else {
                return std::to_string(c);
            }
        }
        else {
            return std::to_string(this->operator[](x));
        }
    };
    std::string to_string_impl(size_t x, size_t y) const {
        if constexpr (std::is_same_v<char, T> || std::is_same_v<unsigned char, T>) {
            auto c = this->operator()(x,y);
            if (std::isalpha(c) || std::isdigit(c)) {
                return "'" + std::string(1, c) + "'";
            }
            else {
                return std::to_string(c);
            }
        }
        else {
            return std::to_string(this->operator()(x,y));
        }
    };
    std::string to_string_impl(size_t x, size_t y, size_t z) const {
        if constexpr (std::is_same_v<char, T> || std::is_same_v<unsigned char, T>) {
            auto c = this->operator()(x, y, z);
            if (std::isalpha(c) || std::isdigit(c)) {
                return "'" + std::string(1, c) + "'";
            }
            else {
                return std::to_string(c);
            }
        }
        else {
            return std::to_string(this->operator()(x, y, z));
        }
    };
    std::vector<size_t> evaluate_column_sizes(std::vector<std::string> column_titles = {}) const {
        std::vector<size_t> out;
        out.resize(this->LenY);

        for (size_t i = 0; i < out.size(); ++i) {
            if (i < column_titles.size())
                out[i] = column_titles[i].size();
            else
                out[i] = 0;
        }

        // only tests the first and last 10 rows of each column
        for (size_t ColN = 0; ColN < this->LenY; ++ColN) {
            for (size_t RowN = 0; RowN < this->LenX && (RowN < 10); ++RowN) {
                out[ColN] = std::max<size_t>(out[ColN], to_string_impl(RowN, ColN).size());
            }
            if (this->LenX > 10) {
                for (size_t RowN = this->LenX - 10; RowN < this->LenX; ++RowN) {
                    out[ColN] = std::max<size_t>(out[ColN], to_string_impl(RowN, ColN).size());
                }
            }
        }

        return out;
    };
    static std::string resize(std::string&& rhs, size_t len, const char def = 0) {
        rhs.resize(len, def);
        return std::move(rhs);
    };
public:
    // y-axis are columns, x-axis are rows. Z-axis is ignored (for now). 
    std::string to_string(std::vector<std::string> column_titles = {}, bool doNotSkip = false) const {
        this->stop_work();

        std::string out;
        if (this->Dim == 0) return out;
        else if (this->Dim == 1) {
            auto col_sizes = evaluate_column_sizes(column_titles);
            
            unsigned int n = 0;
            for (; (n < this->size()) && (n < 1); ++n) {
                out += resize(to_string_impl(n), col_sizes[0], ' ');
            }
            if (!doNotSkip && (this->size() >= 21)) {
                for (; (n < this->size()) && (n < 10); ++n) {
                    out += "\n";
                    out += resize(to_string_impl(n), col_sizes[0], ' ');
                }
                out += "\n...";
                for (n = this->size() - 10; n < this->size(); ++n) {
                    out += "\n";
                    out += resize(to_string_impl(n), col_sizes[0], ' ');
                }                
            }
            else {
                for (; n < this->size(); ++n) {
                    out += "\n";
                    out += resize(to_string_impl(n), col_sizes[0], ' ');
                }
            }
            if (column_titles.size() > 0) {
                out = resize(std::string(column_titles[0]), col_sizes[0], ' ') + "\n" + out;
            }
        }
        else if (this->Dim == 2) {
            auto col_sizes = evaluate_column_sizes(column_titles);

            unsigned int n = 0;
            for (; (n < this->LenX) && (n < 1); ++n) {
                unsigned int y = 0;
                for (; (y < this->LenY) && (y < 1); ++y) {
                    out += resize(to_string_impl(n, y), col_sizes[y], ' ');
                }
                for (; y < this->LenY; ++y) {
                    out += "\t";
                    out += resize(to_string_impl(n, y), col_sizes[y], ' ');
                }                
            }
            if (!doNotSkip && (this->LenX >= 21)) {
                for (; (n < this->LenX) && (n < 10); ++n) {
                    out += "\n";
                    unsigned int y = 0;
                    for (; (y < this->LenY) && (y < 1); ++y) {
                        out += resize(to_string_impl(n, y), col_sizes[y], ' ');
                    }
                    for (; y < this->LenY; ++y) {
                        out += "\t";
                        out += resize(to_string_impl(n, y), col_sizes[y], ' ');
                    }
                }
                out += "\n...";
                for (n = this->LenX - 10; n < this->LenX; ++n) {
                    out += "\n";
                    unsigned int y = 0;
                    for (; (y < this->LenY) && (y < 1); ++y) {
                        out += resize(to_string_impl(n, y), col_sizes[y], ' ');
                    }
                    for (; y < this->LenY; ++y) {
                        out += "\t";
                        out += resize(to_string_impl(n, y), col_sizes[y], ' ');
                    }
                }
            }
            else {
                for (; n < this->LenX; ++n) {
                    out += "\n";
                    unsigned int y = 0;
                    for (; (y < this->LenY) && (y < 1); ++y) {
                        out += resize(to_string_impl(n, y), col_sizes[y], ' ');
                    }
                    for (; y < this->LenY; ++y) {
                        out += "\t";
                        out += resize(to_string_impl(n, y), col_sizes[y], ' ');
                    }
                }
            }

            if (column_titles.size() > 0) {
                std::string temp = column_titles[0];
                for (size_t i = 1; i < column_titles.size(); ++i){
                    temp += "\t";
                    temp += resize(std::string(column_titles[i]), col_sizes[i], ' ');
                }                    
                out = temp + "\n" + out;
            }
        }
        else if (this->Dim == 3) {
            out = "3 dims";
        }
        return out;
    };
    friend std::ostream& operator<<(std::ostream& os, Array const& obj) {
        os << obj.to_string();
        return os;
    };

};

// linear algebra functions to perform a linear regression.
// Meant to test and help build the linear algebra functions in the Array<T> class,
// to support comparing with standard regression tools like those found in Excel. 
namespace linear_regression {
    // solve for the weights to be used when performing linearized predictions, as determined by a basic linear regression.
    __forceinline static Array<float> solve_for_weights(Array<float> const& measurements, Array<float> const& features) {
        return (features.transpose().matrix_multiply(features)).inverse().matrix_multiply(features.transpose()).matrix_multiply(measurements);
    };
    // solve for the linearized prediction.
    __forceinline static Array<float> predict(Array<float> const& features, Array<float> const& weights) {
        return features.matrix_multiply(weights);
    };
    // returns the standard error of the linear regression.
    __forceinline static Array<float> standard_error(Array<float> const& measurements, Array<float> const& features, Array<float> const& weights) {
        auto prediction = predict(features, weights);
        return ((((measurements - prediction).pow(2.0).sum() / std::max<double>(1.0, static_cast<double>(features.size(0)) - 2.0)) * (features.transpose().matrix_multiply(features)).inverse()).pow(0.5)).diagonal();
    };
    // returns the population standard deviation.
    __forceinline static Array<float> standard_deviation(
        Array<float> const& measurements, 
        Array<float> const& features, 
        Array<float> const& weights
    ) {
        return standard_error(measurements, features, weights) * std::sqrt(measurements.size(0));
    };
    // evaluate for the students-t test
    __forceinline static Array<float> t_statistic(Array<float> const& weights, Array<float> const& std_err) {
        return weights / std_err;
    };
    // evaluate for the p-value
    __forceinline static Array<float> p_value(Array<float> const& features, Array<float> const& t_stat) {
        boost::math::students_t dist(features.size(0) - features.size(1)); // n - k - 1, but should include the intercept in the features list already
        Array<float> P(t_stat.size(0));
        size_t N = P.size();
        for (size_t i = 0; i < N; ++i) {
            P[i] = 
                (1.0f - (float)boost::math::cdf(dist, t_stat[i])) + boost::math::cdf(dist, -t_stat[i]);
            if ((P[i] > 1.0f) || (P[i] < 0.0f)) {
                P[i] = (1.0f - (float)boost::math::cdf(dist, -t_stat[i])) + boost::math::cdf(dist, t_stat[i]);
            }

        }
        P.sync();
        return P;
    };
    // build a collection of features for a linear regression while avoiding colinearity. 
    __forceinline static Array<float> build_features(Array<float> const& current_best) {
        return current_best;
    };
    // build a collection of features for a linear regression while avoiding colinearity. 
    template <typename T, typename... Ts> __forceinline static Array<float> build_features(Array<float> const& current_best, T const& candidate, const Ts&... further_candidates) {
        if (current_best.join(1, candidate).is_colinear()) {
            return build_features(current_best, further_candidates...);
        }
        else {
            return build_features(current_best.join(1, candidate), further_candidates...);
        }
    };
};

// TODO: This is an example of a library function
void fnGpuProgramming() {  
    Array<char>(1);
    Array<unsigned char>(1);
    Array<int>(1);
    Array<unsigned int>(1);
    Array<long>(1);
    Array<unsigned long>(1);
    Array<float>(1);

    if (1) {
        print(Array<float>::identity(5));
        print(Array<int>::constant(1, 1000000).sum());

        auto arr = Array<int>::random_between(0, 100, 1000000);
        print(arr.sum());
        print(arr.avg());
        print(arr.max());
        print(arr.min());
    }

    // Demonstrate the creation, use, and destruction of a floating-point matrix with 100M items as part of a CPU-bound matrix multiplication
    Array<float>::constant(1, 10000).matrix_multiply(Array<float>::constant(1, 10000).transpose()).stop_work();

    // Water Demand Modeling Example
    if (1) {
        Array<float> DemandPatterns = Array<float>::constant(1, 24); // series of demand patterns;
        for (int i = 0; i < 24; ++i) {
            DemandPatterns = DemandPatterns.join(1, ((Array<float>::linear(0.0f, 3.14f, 24).sin() * 0.5f) + 0.5f) * Array<float>::random_between(0.75, 1.25, 24));
        }

        const int num_timesteps = 24 * (60 / 5);
        Array<float> junctions_base_multipliers = Array<float>::random_between(0, 5, 40000);
        Array<unsigned int> junction_pattern_indices = Array<unsigned int>::random_between(1, 23, junctions_base_multipliers.size(0));
        Array<float> junctions_X_flow = Array<float>::constant(0, junctions_base_multipliers.size(0));

        print("");
        print(junction_pattern_indices);
        print("");

        Array<unsigned int> pipe_open = Array<unsigned int>::constant(1, 24000);
        Array<float> pipe_flow_resistance = Array<float>::random_between(80, 140, pipe_open.size(0));
        Array<float> pipe_headloss_gradient = Array<float>::constant(0, pipe_open.size(0));
        Array<float> pipe_headloss = Array<float>::constant(0, pipe_open.size(0));
        Array<float> pipe_flow = Array<float>::constant(0, pipe_open.size(0));
        Array<float> P_coeff;
        Array<float> Y_coeff;
        Array<unsigned int> pipe_upstream_node_index = Array<unsigned int>::random_between(0, junctions_base_multipliers.size(0) - 1, pipe_open.size(0));
        Array<unsigned int> pipe_downstream_node_index = Array<unsigned int>::random_between(0, junctions_base_multipliers.size(0) - 1, pipe_open.size(0));

        for (int TimeStep = 0; TimeStep < num_timesteps; ++TimeStep) {
            auto DemandPatterns_AtThisTime = DemandPatterns.row(TimeStep % DemandPatterns.size(0)); // sample a row from the demand patterns at this timestep
            auto junction_pattern_multipliers = DemandPatterns_AtThisTime.resample(junction_pattern_indices); // re-sample the row of demand pattern for each junction based on that junction's indices. 
            Array<float> junction_demands_this_iteration = junction_pattern_multipliers * junctions_base_multipliers; // junction.demand * pattern[now] = current flowrate at each junction in the model

            // pipecoeff
            {
                pipe_headloss_gradient.stop_work();

                pipe_headloss_gradient = Array<float>::constant(1.852, pipe_headloss_gradient.size(0));
                pipe_headloss_gradient *= pipe_flow_resistance;
                pipe_headloss_gradient *= pipe_flow.pow(1.852 - 1.0);

                auto switch_condition =
                    (pipe_headloss_gradient < 1E-7).cast<float>();
                pipe_headloss_gradient =
                    (switch_condition * 1E-7) + ((1.0f - switch_condition) * pipe_headloss_gradient);
                pipe_headloss =
                    (switch_condition * pipe_flow * 1E-7) // if (pipe_headloss_gradient < 1E-7)
                    + ((1.0f - switch_condition) * pipe_headloss_gradient * pipe_flow / 1.852); // ... otherwise use original formula            
                pipe_headloss *= pipe_flow.sign(); // Adjust head loss sign for flow direction

                // P and Y coeffs.
                P_coeff = 1.0 / pipe_headloss_gradient;
                Y_coeff = pipe_headloss / pipe_headloss_gradient;
            }

            // linkcoeff
            {
                auto do_nothing_check_1 = (P_coeff != 0).cast<float>();

                // Update nodal flow excess (Xflow). (Flow out of node is (-), flow into node is (+))
                junctions_X_flow -= pipe_flow.resample(pipe_upstream_node_index) * do_nothing_check_1;
                junctions_X_flow += pipe_flow.resample(pipe_downstream_node_index) * do_nothing_check_1;





            }
        }

        




        // junctions_base_multipliers = junctions_base_multipliers.grow_by_wrapping(junctions_base_multipliers.size(0) * num_timesteps);
        // junctions_base_multipliers





        // print(DemandPatterns["0"].grow_by_wrapping(128));




    }


    // Advertisement regression. Generally correct analysis.
    if (1) {
        /*          Coefficients    Standard Error	t Stat	        P-value	        Lower 95%	    Upper 95%	
        Intercept	4.625124079	    0.307501165	    15.04099695	    1.68268E-34	    4.018688356	    5.231559801
        TV	        0.05444578	    0.001375188	    39.59152448	    1.89294E-95	    0.051733716	    0.057157845
        Radio	    0.107001228	    0.008489563	    12.60385655	    4.6021E-27	    0.090258612	    0.123743844
        Newspaper	0.000335658	    0.005788056	    0.057991479	    0.953814495	    -0.011079206	0.011750522
        */


        auto TV_Ads = Array<float>::from_vector(std::vector<double>{
            230.1, 44.5, 17.2, 151.5, 180.8, 8.7, 57.5, 120.2, 8.6, 199.8, 66.1, 214.7, 23.8, 97.5, 204.1, 195.4, 67.8, 281.4, 69.2, 147.3, 218.4, 237.4, 13.2, 228.3, 62.3, 262.9, 142.9, 240.1, 248.8, 70.6, 292.9, 112.9, 97.2, 265.6, 95.7, 290.7, 266.9, 74.7, 43.1, 228.0, 202.5, 177.0, 293.6, 206.9, 25.1, 175.1, 89.7, 239.9, 227.2, 66.9, 199.8, 100.4, 216.4, 182.6, 262.7, 198.9, 7.3, 136.2, 210.8, 210.7, 53.5, 261.3, 239.3, 102.7, 131.1, 69.0, 31.5, 139.3, 237.4, 216.8, 199.1, 109.8, 26.8, 129.4, 213.4, 16.9, 27.5, 120.5, 5.4, 116.0, 76.4, 239.8, 75.3, 68.4, 213.5, 193.2, 76.3, 110.7, 88.3, 109.8, 134.3, 28.6, 217.7, 250.9, 107.4, 163.3, 197.6, 184.9, 289.7, 135.2, 222.4, 296.4, 280.2, 187.9, 238.2, 137.9, 25.0, 90.4, 13.1, 255.4, 225.8, 241.7, 175.7, 209.6, 78.2, 75.1, 139.2, 76.4, 125.7, 19.4, 141.3, 18.8, 224.0, 123.1, 229.5, 87.2, 7.8, 80.2, 220.3, 59.6, .7, 265.2, 8.4, 219.8, 36.9, 48.3, 25.6, 273.7, 43.0, 184.9, 73.4, 193.7, 220.5, 104.6, 96.2, 140.3, 240.1, 243.2, 38.0, 44.7, 280.7, 121.0, 197.6, 171.3, 187.8, 4.1, 93.9, 149.8, 11.7, 131.7, 172.5, 85.7, 188.4, 163.5, 117.2, 234.5, 17.9, 206.8, 215.4, 284.3, 50.0, 164.5, 19.6, 168.4, 222.4, 276.9, 248.4, 170.2, 276.7, 165.6, 156.6, 218.5, 56.2, 287.6, 253.8, 205.0, 139.5, 191.1, 286.0, 18.7, 39.5, 75.5, 17.2, 166.8, 149.7, 38.2, 94.2, 177.0, 283.6, 232.1
        });
        auto Radio_Ads = Array<float>::from_vector(std::vector<double>{
            37.8, 39.3, 45.9, 41.3, 10.8, 48.9, 32.8, 19.6, 2.1, 2.6, 5.8, 24.0, 35.1, 7.6, 32.9, 47.7, 36.6, 39.6, 20.5, 23.9, 27.7, 5.1, 15.9, 16.9, 12.6, 3.5, 29.3, 16.7, 27.1, 16.0, 28.3, 17.4, 1.5, 20.0, 1.4, 4.1, 43.8, 49.4, 26.7, 37.7, 22.3, 33.4, 27.7, 8.4, 25.7, 22.5, 9.9, 41.5, 15.8, 11.7, 3.1, 9.6, 41.7, 46.2, 28.8, 49.4, 28.1, 19.2, 49.6, 29.5, 2.0, 42.7, 15.5, 29.6, 42.8, 9.3, 24.6, 14.5, 27.5, 43.9, 30.6, 14.3, 33.0, 5.7, 24.6, 43.7, 1.6, 28.5, 29.9, 7.7, 26.7, 4.1, 20.3, 44.5, 43.0, 18.4, 27.5, 40.6, 25.5, 47.8, 4.9, 1.5, 33.5, 36.5, 14.0, 31.6, 3.5, 21.0, 42.3, 41.7, 4.3, 36.3, 10.1, 17.2, 34.3, 46.4, 11.0, .3, .4, 26.9, 8.2, 38.0, 15.4, 20.6, 46.8, 35.0, 14.3, .8, 36.9, 16.0, 26.8, 21.7, 2.4, 34.6, 32.3, 11.8, 38.9, .0, 49.0, 12.0, 39.6, 2.9, 27.2, 33.5, 38.6, 47.0, 39.0, 28.9, 25.9, 43.9, 17.0, 35.4, 33.2, 5.7, 14.8, 1.9, 7.3, 49.0, 40.3, 25.8, 13.9, 8.4, 23.3, 39.7, 21.1, 11.6, 43.5, 1.3, 36.9, 18.4, 18.1, 35.8, 18.1, 36.8, 14.7, 3.4, 37.6, 5.2, 23.6, 10.6, 11.6, 20.9, 20.1, 7.1, 3.4, 48.9, 30.2, 7.8, 2.3, 10.0, 2.6, 5.4, 5.7, 43.0, 21.3, 45.1, 2.1, 28.7, 13.9, 12.1, 41.1, 10.8, 4.1, 42.0, 35.6, 3.7, 4.9, 9.3, 42.0, 8.6
        });
        auto Newspaper_Ads = Array<float>::from_vector(std::vector<double>{
            69.2, 45.1, 69.3, 58.5, 58.4, 75.0, 23.5, 11.6, 1.0, 21.2, 24.2, 4.0, 65.9, 7.2, 46.0, 52.9, 114.0, 55.8, 18.3, 19.1, 53.4, 23.5, 49.6, 26.2, 18.3, 19.5, 12.6, 22.9, 22.9, 40.8, 43.2, 38.6, 30.0, .3, 7.4, 8.5, 5.0, 45.7, 35.1, 32.0, 31.6, 38.7, 1.8, 26.4, 43.3, 31.5, 35.7, 18.5, 49.9, 36.8, 34.6, 3.6, 39.6, 58.7, 15.9, 60.0, 41.4, 16.6, 37.7, 9.3, 21.4, 54.7, 27.3, 8.4, 28.9, .9, 2.2, 10.2, 11.0, 27.2, 38.7, 31.7, 19.3, 31.3, 13.1, 89.4, 20.7, 14.2, 9.4, 23.1, 22.3, 36.9, 32.5, 35.6, 33.8, 65.7, 16.0, 63.2, 73.4, 51.4, 9.3, 33.0, 59.0, 72.3, 10.9, 52.9, 5.9, 22.0, 51.2, 45.9, 49.8, 100.9, 21.4, 17.9, 5.3, 59.0, 29.7, 23.2, 25.6, 5.5, 56.5, 23.2, 2.4, 10.7, 34.5, 52.7, 25.6, 14.8, 79.2, 22.3, 46.2, 50.4, 15.6, 12.4, 74.2, 25.9, 50.6, 9.2, 3.2, 43.1, 8.7, 43.0, 2.1, 45.1, 65.6, 8.5, 9.3, 59.7, 20.5, 1.7, 12.9, 75.6, 37.9, 34.4, 38.9, 9.0, 8.7, 44.3, 11.9, 20.6, 37.0, 48.7, 14.2, 37.7, 9.5, 5.7, 50.5, 24.3, 45.2, 34.6, 30.7, 49.3, 25.6, 7.4, 5.4, 84.8, 21.6, 19.4, 57.6, 6.4, 18.4, 47.4, 17.0, 12.8, 13.1, 41.8, 20.3, 35.2, 23.7, 17.6, 8.3, 27.4, 29.7, 71.8, 30.0, 19.6, 26.6, 18.2, 3.7, 23.4, 5.8, 6.0, 31.6, 3.6, 6.0, 13.8, 8.1, 6.4, 66.2, 8.7
        });
        auto Sales_Revenue = Array<float>::from_vector(std::vector<double>{
            22.1, 10.4, 12.0, 16.5, 17.9, 7.2, 11.8, 13.2, 4.8, 15.6, 12.6, 17.4, 9.2, 13.7, 19.0, 22.4, 12.5, 24.4, 11.3, 14.6, 18.0, 17.5, 5.6, 20.5, 9.7, 17.0, 15.0, 20.9, 18.9, 10.5, 21.4, 11.9, 13.2, 17.4, 11.9, 17.8, 25.4, 14.7, 10.1, 21.5, 16.6, 17.1, 20.7, 17.9, 8.5, 16.1, 10.6, 23.2, 19.8, 9.7, 16.4, 10.7, 22.6, 21.2, 20.2, 23.7, 5.5, 13.2, 23.8, 18.4, 8.1, 24.2, 20.7, 14.0, 16.0, 11.3, 11.0, 13.4, 18.9, 22.3, 18.3, 12.4, 8.8, 11.0, 17.0, 8.7, 6.9, 14.2, 5.3, 11.0, 11.8, 17.3, 11.3, 13.6, 21.7, 20.2, 12.0, 16.0, 12.9, 16.7, 14.0, 7.3, 19.4, 22.2, 11.5, 16.9, 16.7, 20.5, 25.4, 17.2, 16.7, 23.8, 19.8, 19.7, 20.7, 15.0, 7.2, 12.0, 5.3, 19.8, 18.4, 21.8, 17.1, 20.9, 14.6, 12.6, 12.2, 9.4, 15.9, 6.6, 15.5, 7.0, 16.6, 15.2, 19.7, 10.6, 6.6, 11.9, 24.7, 9.7, 1.6, 17.7, 5.7, 19.6, 10.8, 11.6, 9.5, 20.8, 9.6, 20.7, 10.9, 19.2, 20.1, 10.4, 12.3, 10.3, 18.2, 25.4, 10.9, 10.1, 16.1, 11.6, 16.6, 16.0, 20.6, 3.2, 15.3, 10.1, 7.3, 12.9, 16.4, 13.3, 19.9, 18.0, 11.9, 16.9, 8.0, 17.2, 17.1, 20.0, 8.4, 17.5, 7.6, 16.7, 16.5, 27.0, 20.2, 16.7, 16.8, 17.6, 15.5, 17.2, 8.7, 26.2, 17.6, 22.6, 10.3, 17.3, 20.9, 6.7, 10.8, 11.9, 5.9, 19.6, 17.3, 7.6, 14.0, 14.8, 25.5, 18.4
        });
        auto Basic{ 
            Array<float>::constant(1, Sales_Revenue.size(0)) 
        };
        auto features = linear_regression::build_features( // double-checks and removes colinearity
            Basic, TV_Ads, Radio_Ads, Newspaper_Ads 
        );

        auto weights = linear_regression::solve_for_weights(Sales_Revenue, features); 
        auto std_err = linear_regression::standard_error(Sales_Revenue, features, weights);
        auto std_dev = linear_regression::standard_deviation(Sales_Revenue, features, weights);
        auto t_stat = linear_regression::t_statistic(weights, std_err);
        auto p_value = linear_regression::p_value(features, t_stat);
        auto lower_95 = weights - (1.96 * std_err);
        auto upper_95 = weights + (1.96 * std_err);
        auto prediction = linear_regression::predict(features, weights);
        
        print(""); 
        print(
            weights.join(1, 
                std_err).join(1, 
                    t_stat).join(1, 
                        p_value).join(1,
                            lower_95).join(1, 
                                upper_95).to_string(
                { "Coefficients", "Standard Error", "t Stat", "P-value", "Lower 95%", "Upper 95%" })
        );
        print("");       

        print(Sales_Revenue.join(1, prediction).to_string({"Measured", "Predicted"}));
        print("");
    }

    // Custom weather regression. Generally correct analysis.
    if (1) {
        /*          Coefficients	Standard Error	t Stat	        P-value	    Lower 95%	    Upper 95%
        Intercept	93.67835922	    0.121802957	    769.0975786	    0	        93.43959502	    93.91712343
        dawn	    -14.4227875	    0.153113204	    -94.19688942	0	        -14.72292761	-14.12264739
        dusk	    -10.26652003	0.162912373	    -63.01866368	0	        -10.58586896	-9.947171106
        winter	    -10.2061204	    0.131022469	    -77.89595536	0	        -10.46295715	-9.949283644
        */

        auto measured = Array<float>::random_between(2, 4, 24 * 365);
        auto random_noise = Array<float>::random_between(2, 4, 24 * 365);
        auto hours = Array<float>(24 * 365);
        auto months = Array<float>(24 * 365);

        std::array<float, 24> hourly{
            59, 58, 59, 60, 61, 62,
            64, 69, 72, 76, 79, 81,
            82, 80, 78, 76, 75, 74,
            70, 68, 66, 64, 62, 60
        };
        std::array<float, 12> monthly{
            0, -2, 6, 10, 14, 20,
            19, 18, 16, 12, 8, 4
        };
        for (size_t day = 0; day < 365; ++day) {
            for (size_t hr = 0; hr < 24; ++hr) {
                measured[(day * 24) + hr] += (monthly[day / 31] + hourly[hr]);
                hours[(day * 24) + hr] = hr;
                months[(day * 24) + hr] = day / 31;
            }
        }

        measured.sync();
        hours.sync();
        months.sync();

        auto dawn = (hours < 6).cast<float>();
        auto dusk = (hours > 18).cast<float>();
        auto midday = (!((hours > 18) + (hours < 6))).cast<float>();
        auto winter = ((months >= 10) + (months <= 4)).cast<float>();
        auto summer = (!winter).cast<float>();
        auto Basic{ Array<float>::constant(1, winter.size(0)) };
        auto features = linear_regression::build_features( // double-checks and removes colinearity
            Basic, dawn, dusk, midday, winter, summer
        );

        auto weights = linear_regression::solve_for_weights(measured, features);
        auto std_err = linear_regression::standard_error(measured, features, weights);
        auto std_dev = linear_regression::standard_deviation(measured, features, weights);
        auto t_stat = linear_regression::t_statistic(weights, std_err);
        auto p_value = linear_regression::p_value(features, t_stat);
        auto lower_95 = weights - (1.96 * std_err);
        auto upper_95 = weights + (1.96 * std_err);
        auto prediction = linear_regression::predict(features, weights);

        print("");
        print(
            weights.join(1,
                std_err).join(1,
                    t_stat).join(1,
                        p_value).join(1,
                            lower_95).join(1,
                                upper_95).to_string(
                                    { "Coefficients", "Standard Error", "t Stat", "P-value", "Lower 95%", "Upper 95%" })
        );
        print("");

        print(measured.join(1, prediction).to_string({ "Measured", "Predicted" }));
        print("");







    }

    // MPG regression. Not being performed correctly in any way, for reasons not yet understood.
    if (1) {
        /*              Coefficients	Standard Error	t Stat	        P-value	        Lower 95%	    Upper 95%
        Intercept	    -17.21843462	4.644294149	    -3.707438433	0.000240184	    -26.34986447	-8.087004775
        cylinders	    -0.493376319	0.323282315	    -1.526146951	0.127796468	    -1.129001385	0.142248747
        displacement	0.019895644	    0.007515079	    2.647429695	    0.008444649	    0.005119788	    0.034671499
        horsepower	    -0.016951144	0.013786891	    -1.229511695	0.219632823	    -0.044058392	0.010156103
        weight	        -0.006474043	0.000652048	    -9.928787106	7.87495E-21	    -0.007756074	-0.005192013
        acceleration	0.080575838	    0.098844957	    0.815173996	    0.415478018	    -0.113769257	0.274920933
        model year	    0.750772678	    0.050973122	    14.72879519	    3.05598E-39	    0.650551315	    0.850994041
        origin	        1.426140495	    0.278136092	    5.127491665	    4.66568E-07	    0.879280169	    1.973000822
        */

        auto mpg = Array<float>::from_vector(std::vector<double>{
            18, 15, 18, 16, 17, 15, 14, 14, 14, 15, 15, 14, 15, 14, 24, 22, 18, 21, 27, 26, 25, 24, 25, 26, 21, 10, 10, 11, 9, 27, 28, 25, 19, 16, 17, 19, 18, 14, 14, 14, 14, 12, 13, 13, 18, 22, 19, 18, 23, 28, 30, 30, 31, 35, 27, 26, 24, 25, 23, 20, 21, 13, 14, 15, 14, 17, 11, 13, 12, 13, 19, 15, 13, 13, 14, 18, 22, 21, 26, 22, 28, 23, 28, 27, 13, 14, 13, 14, 15, 12, 13, 13, 14, 13, 12, 13, 18, 16, 18, 18, 23, 26, 11, 12, 13, 12, 18, 20, 21, 22, 18, 19, 21, 26, 15, 16, 29, 24, 20, 19, 15, 24, 20, 11, 20, 19, 15, 31, 26, 32, 25, 16, 16, 18, 16, 13, 14, 14, 14, 29, 26, 26, 31, 32, 28, 24, 26, 24, 26, 31, 19, 18, 15, 15, 16, 15, 16, 14, 17, 16, 15, 18, 21, 20, 13, 29, 23, 20, 23, 24, 25, 24, 18, 29, 19, 23, 23, 22, 25, 33, 28, 25, 25, 26, 27, 17.5, 16, 15.5, 14.5, 22, 22, 24, 22.5, 29, 24.5, 29, 33, 20, 18, 18.5, 17.5, 29.5, 32, 28, 26.5, 20, 13, 19, 19, 16.5, 16.5, 13, 13, 13, 31.5, 30, 36, 25.5, 33.5, 17.5, 17, 15.5, 15, 17.5, 20.5, 19, 18.5, 16, 15.5, 15.5, 16, 29, 24.5, 26, 25.5, 30.5, 33.5, 30, 30.5, 22, 21.5, 21.5, 43.1, 36.1, 32.8, 39.4, 36.1, 19.9, 19.4, 20.2, 19.2, 20.5, 20.2, 25.1, 20.5, 19.4, 20.6, 20.8, 18.6, 18.1, 19.2, 17.7, 18.1, 17.5, 30, 27.5, 27.2, 30.9, 21.1, 23.2, 23.8, 23.9, 20.3, 17, 21.6, 16.2, 31.5, 29.5, 21.5, 19.8, 22.3, 20.2, 20.6, 17, 17.6, 16.5, 18.2, 16.9, 15.5, 19.2, 18.5, 31.9, 34.1, 35.7, 27.4, 25.4, 23, 27.2, 23.9, 34.2, 34.5, 31.8, 37.3, 28.4, 28.8, 26.8, 33.5, 41.5, 38.1, 32.1, 37.2, 28, 26.4, 24.3, 19.1, 34.3, 29.8, 31.3, 37, 32.2, 46.6, 27.9, 40.8, 44.3, 43.4, 36.4, 30, 44.6, 33.8, 29.8, 32.7, 23.7, 35, 32.4, 27.2, 26.6, 25.8, 23.5, 30, 39.1, 39, 35.1, 32.3, 37, 37.7, 34.1, 34.7, 34.4, 29.9, 33, 33.7, 32.4, 32.9, 31.6, 28.1, 30.7, 25.4, 24.2, 22.4, 26.6, 20.2, 17.6, 28, 27, 34, 31, 29, 27, 24, 36, 37, 31, 38, 36, 36, 36, 34, 38, 32, 38, 25, 38, 26, 22, 32, 36, 27, 27, 44, 32, 28, 31
        });
        auto cylinders = Array<float>::from_vector(std::vector<double>{
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4, 6, 6, 6, 4, 4, 4, 4, 4, 4, 6, 8, 8, 8, 8, 4, 4, 4, 6, 6, 6, 6, 6, 8, 8, 8, 8, 8, 8, 8, 6, 4, 6, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 3, 8, 8, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 8, 8, 8, 8, 6, 4, 4, 4, 3, 4, 6, 4, 8, 8, 4, 4, 4, 4, 8, 4, 6, 8, 6, 6, 6, 4, 4, 4, 4, 6, 6, 6, 8, 8, 8, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 6, 6, 8, 8, 8, 8, 6, 6, 6, 6, 6, 8, 8, 4, 4, 6, 4, 4, 4, 4, 6, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 8, 8, 6, 6, 6, 6, 4, 4, 4, 4, 6, 6, 6, 6, 4, 4, 4, 4, 4, 8, 4, 6, 6, 8, 8, 8, 8, 4, 4, 4, 4, 4, 8, 8, 8, 8, 6, 6, 6, 6, 8, 8, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 6, 4, 3, 4, 4, 4, 4, 4, 8, 8, 8, 6, 6, 6, 4, 6, 6, 6, 6, 6, 6, 8, 6, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 5, 6, 4, 6, 4, 4, 6, 6, 4, 6, 6, 8, 8, 8, 8, 8, 8, 8, 8, 4, 4, 4, 4, 5, 8, 4, 8, 4, 4, 4, 4, 4, 6, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 6, 3, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 6, 6, 8, 6, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4
        });
        auto displacement = Array<float>::from_vector(std::vector<double>{
            307, 350, 318, 304, 302, 429, 454, 440, 455, 390, 383, 340, 400, 455, 113, 198, 199, 200, 97, 97, 110, 107, 104, 121, 199, 360, 307, 318, 304, 97, 140, 113, 232, 225, 250, 250, 232, 350, 400, 351, 318, 383, 400, 400, 258, 140, 250, 250, 122, 116, 79, 88, 71, 72, 97, 91, 113, 97.5, 97, 140, 122, 350, 400, 318, 351, 304, 429, 350, 350, 400, 70, 304, 307, 302, 318, 121, 121, 120, 96, 122, 97, 120, 98, 97, 350, 304, 350, 302, 318, 429, 400, 351, 318, 440, 455, 360, 225, 250, 232, 250, 198, 97, 400, 400, 360, 350, 232, 97, 140, 108, 70, 122, 155, 98, 350, 400, 68, 116, 114, 121, 318, 121, 156, 350, 198, 232, 250, 79, 122, 71, 140, 250, 258, 225, 302, 350, 318, 302, 304, 98, 79, 97, 76, 83, 90, 90, 116, 120, 108, 79, 225, 250, 250, 250, 400, 350, 318, 351, 231, 250, 258, 225, 231, 262, 302, 97, 140, 232, 140, 134, 90, 119, 171, 90, 232, 115, 120, 121, 121, 91, 107, 116, 140, 98, 101, 305, 318, 304, 351, 225, 250, 200, 232, 85, 98, 90, 91, 225, 250, 250, 258, 97, 85, 97, 140, 130, 318, 120, 156, 168, 350, 350, 302, 318, 98, 111, 79, 122, 85, 305, 260, 318, 302, 250, 231, 225, 250, 400, 350, 400, 351, 97, 151, 97, 140, 98, 98, 97, 97, 146, 121, 80, 90, 98, 78, 85, 91, 260, 318, 302, 231, 200, 200, 140, 225, 232, 231, 200, 225, 258, 305, 231, 302, 318, 98, 134, 119, 105, 134, 156, 151, 119, 131, 163, 121, 163, 89, 98, 231, 200, 140, 232, 225, 305, 302, 351, 318, 350, 351, 267, 360, 89, 86, 98, 121, 183, 350, 141, 260, 105, 105, 85, 91, 151, 173, 173, 151, 98, 89, 98, 86, 151, 140, 151, 225, 97, 134, 120, 119, 108, 86, 156, 85, 90, 90, 121, 146, 91, 97, 89, 168, 70, 122, 107, 135, 151, 156, 173, 135, 79, 86, 81, 97, 85, 89, 91, 105, 98, 98, 105, 107, 108, 119, 120, 141, 145, 168, 146, 231, 350, 200, 225, 112, 112, 112, 112, 135, 151, 140, 105, 91, 91, 105, 98, 120, 107, 108, 91, 91, 91, 181, 262, 156, 232, 144, 135, 151, 140, 97, 135, 120, 119
        });
        auto horsepower = Array<float>::from_vector(std::vector<double>{
            130, 165, 150, 150, 140, 198, 220, 215, 225, 190, 170, 160, 150, 225, 95, 95, 97, 85, 88, 46, 87, 90, 95, 113, 90, 215, 200, 210, 193, 88, 90, 95, 100, 105, 100, 88, 100, 165, 175, 153, 150, 180, 170, 175, 110, 72, 100, 88, 86, 90, 70, 76, 65, 69, 60, 70, 95, 80, 54, 90, 86, 165, 175, 150, 153, 150, 208, 155, 160, 190, 97, 150, 130, 140, 150, 112, 76, 87, 69, 86, 92, 97, 80, 88, 175, 150, 145, 137, 150, 198, 150, 158, 150, 215, 225, 175, 105, 100, 100, 88, 95, 46, 150, 167, 170, 180, 100, 88, 72, 94, 90, 85, 107, 90, 145, 230, 49, 75, 91, 112, 150, 110, 122, 180, 95, 100, 100, 67, 80, 65, 75, 100, 110, 105, 140, 150, 150, 140, 150, 83, 67, 78, 52, 61, 75, 75, 75, 97, 93, 67, 95, 105, 72, 72, 170, 145, 150, 148, 110, 105, 110, 95, 110, 110, 129, 75, 83, 100, 78, 96, 71, 97, 97, 70, 90, 95, 88, 98, 115, 53, 86, 81, 92, 79, 83, 140, 150, 120, 152, 100, 105, 81, 90, 52, 60, 70, 53, 100, 78, 110, 95, 71, 70, 75, 72, 102, 150, 88, 108, 120, 180, 145, 130, 150, 68, 80, 58, 96, 70, 145, 110, 145, 130, 110, 105, 100, 98, 180, 170, 190, 149, 78, 88, 75, 89, 63, 83, 67, 78, 97, 110, 110, 48, 66, 52, 70, 60, 110, 140, 139, 105, 95, 85, 88, 100, 90, 105, 85, 110, 120, 145, 165, 139, 140, 68, 95, 97, 75, 95, 105, 85, 97, 103, 125, 115, 133, 71, 68, 115, 85, 88, 90, 110, 130, 129, 138, 135, 155, 142, 125, 150, 71, 65, 80, 80, 77, 125, 71, 90, 70, 70, 65, 69, 90, 115, 115, 90, 76, 60, 70, 65, 90, 88, 90, 90, 78, 90, 75, 92, 75, 65, 105, 65, 48, 48, 67, 67, 67, 67, 62, 132, 100, 88, 72, 84, 84, 92, 110, 84, 58, 64, 60, 67, 65, 62, 68, 63, 65, 65, 74, 75, 75, 100, 74, 80, 76, 116, 120, 110, 105, 88, 85, 88, 88, 88, 85, 84, 90, 92, 74, 68, 68, 63, 70, 88, 75, 70, 67, 67, 67, 110, 85, 92, 112, 96, 84, 90, 86, 52, 84, 79, 82
        });
        auto weight = Array<float>::from_vector(std::vector<double>{
            3504, 3693, 3436, 3433, 3449, 4341, 4354, 4312, 4425, 3850, 3563, 3609, 3761, 3086, 2372, 2833, 2774, 2587, 2130, 1835, 2672, 2430, 2375, 2234, 2648, 4615, 4376, 4382, 4732, 2130, 2264, 2228, 2634, 3439, 3329, 3302, 3288, 4209, 4464, 4154, 4096, 4955, 4746, 5140, 2962, 2408, 3282, 3139, 2220, 2123, 2074, 2065, 1773, 1613, 1834, 1955, 2278, 2126, 2254, 2408, 2226, 4274, 4385, 4135, 4129, 3672, 4633, 4502, 4456, 4422, 2330, 3892, 4098, 4294, 4077, 2933, 2511, 2979, 2189, 2395, 2288, 2506, 2164, 2100, 4100, 3672, 3988, 4042, 3777, 4952, 4464, 4363, 4237, 4735, 4951, 3821, 3121, 3278, 2945, 3021, 2904, 1950, 4997, 4906, 4654, 4499, 2789, 2279, 2401, 2379, 2124, 2310, 2472, 2265, 4082, 4278, 1867, 2158, 2582, 2868, 3399, 2660, 2807, 3664, 3102, 2901, 3336, 1950, 2451, 1836, 2542, 3781, 3632, 3613, 4141, 4699, 4457, 4638, 4257, 2219, 1963, 2300, 1649, 2003, 2125, 2108, 2246, 2489, 2391, 2000, 3264, 3459, 3432, 3158, 4668, 4440, 4498, 4657, 3907, 3897, 3730, 3785, 3039, 3221, 3169, 2171, 2639, 2914, 2592, 2702, 2223, 2545, 2984, 1937, 3211, 2694, 2957, 2945, 2671, 1795, 2464, 2220, 2572, 2255, 2202, 4215, 4190, 3962, 4215, 3233, 3353, 3012, 3085, 2035, 2164, 1937, 1795, 3651, 3574, 3645, 3193, 1825, 1990, 2155, 2565, 3150, 3940, 3270, 2930, 3820, 4380, 4055, 3870, 3755, 2045, 2155, 1825, 2300, 1945, 3880, 4060, 4140, 4295, 3520, 3425, 3630, 3525, 4220, 4165, 4325, 4335, 1940, 2740, 2265, 2755, 2051, 2075, 1985, 2190, 2815, 2600, 2720, 1985, 1800, 1985, 2070, 1800, 3365, 3735, 3570, 3535, 3155, 2965, 2720, 3430, 3210, 3380, 3070, 3620, 3410, 3425, 3445, 3205, 4080, 2155, 2560, 2300, 2230, 2515, 2745, 2855, 2405, 2830, 3140, 2795, 3410, 1990, 2135, 3245, 2990, 2890, 3265, 3360, 3840, 3725, 3955, 3830, 4360, 4054, 3605, 3940, 1925, 1975, 1915, 2670, 3530, 3900, 3190, 3420, 2200, 2150, 2020, 2130, 2670, 2595, 2700, 2556, 2144, 1968, 2120, 2019, 2678, 2870, 3003, 3381, 2188, 2711, 2542, 2434, 2265, 2110, 2800, 2110, 2085, 2335, 2950, 3250, 1850, 2145, 1845, 2910, 2420, 2500, 2290, 2490, 2635, 2620, 2725, 2385, 1755, 1875, 1760, 2065, 1975, 2050, 1985, 2215, 2045, 2380, 2190, 2210, 2350, 2615, 2635, 3230, 3160, 2900, 2930, 3415, 3725, 3060, 3465, 2605, 2640, 2395, 2575, 2525, 2735, 2865, 1980, 2025, 1970, 2125, 2125, 2160, 2205, 2245, 1965, 1965, 1995, 2945, 3015, 2585, 2835, 2665, 2370, 2950, 2790, 2130, 2295, 2625, 2720
        });
        auto acceleration = Array<float>::from_vector(std::vector<double>{
            12, 11.5, 11, 12, 10.5, 10, 9, 8.5, 10, 8.5, 10, 8, 9.5, 10, 15, 15.5, 15.5, 16, 14.5, 20.5, 17.5, 14.5, 17.5, 12.5, 15, 14, 15, 13.5, 18.5, 14.5, 15.5, 14, 13, 15.5, 15.5, 15.5, 15.5, 12, 11.5, 13.5, 13, 11.5, 12, 12, 13.5, 19, 15, 14.5, 14, 14, 19.5, 14.5, 19, 18, 19, 20.5, 15.5, 17, 23.5, 19.5, 16.5, 12, 12, 13.5, 13, 11.5, 11, 13.5, 13.5, 12.5, 13.5, 12.5, 14, 16, 14, 14.5, 18, 19.5, 18, 16, 17, 14.5, 15, 16.5, 13, 11.5, 13, 14.5, 12.5, 11.5, 12, 13, 14.5, 11, 11, 11, 16.5, 18, 16, 16.5, 16, 21, 14, 12.5, 13, 12.5, 15, 19, 19.5, 16.5, 13.5, 18.5, 14, 15.5, 13, 9.5, 19.5, 15.5, 14, 15.5, 11, 14, 13.5, 11, 16.5, 16, 17, 19, 16.5, 21, 17, 17, 18, 16.5, 14, 14.5, 13.5, 16, 15.5, 16.5, 15.5, 14.5, 16.5, 19, 14.5, 15.5, 14, 15, 15.5, 16, 16, 16, 21, 19.5, 11.5, 14, 14.5, 13.5, 21, 18.5, 19, 19, 15, 13.5, 12, 16, 17, 16, 18.5, 13.5, 16.5, 17, 14.5, 14, 17, 15, 17, 14.5, 13.5, 17.5, 15.5, 16.9, 14.9, 17.7, 15.3, 13, 13, 13.9, 12.8, 15.4, 14.5, 17.6, 17.6, 22.2, 22.1, 14.2, 17.4, 17.7, 21, 16.2, 17.8, 12.2, 17, 16.4, 13.6, 15.7, 13.2, 21.9, 15.5, 16.7, 12.1, 12, 15, 14, 18.5, 14.8, 18.6, 15.5, 16.8, 12.5, 19, 13.7, 14.9, 16.4, 16.9, 17.7, 19, 11.1, 11.4, 12.2, 14.5, 14.5, 16, 18.2, 15.8, 17, 15.9, 16.4, 14.1, 14.5, 12.8, 13.5, 21.5, 14.4, 19.4, 18.6, 16.4, 15.5, 13.2, 12.8, 19.2, 18.2, 15.8, 15.4, 17.2, 17.2, 15.8, 16.7, 18.7, 15.1, 13.2, 13.4, 11.2, 13.7, 16.5, 14.2, 14.7, 14.5, 14.8, 16.7, 17.6, 14.9, 15.9, 13.6, 15.7, 15.8, 14.9, 16.6, 15.4, 18.2, 17.3, 18.2, 16.6, 15.4, 13.4, 13.2, 15.2, 14.9, 14.3, 15, 13, 14, 15.2, 14.4, 15, 20.1, 17.4, 24.8, 22.2, 13.2, 14.9, 19.2, 14.7, 16, 11.3, 12.9, 13.2, 14.7, 18.8, 15.5, 16.4, 16.5, 18.1, 20.1, 18.7, 15.8, 15.5, 17.5, 15, 15.2, 17.9, 14.4, 19.2, 21.7, 23.7, 19.9, 21.8, 13.8, 18, 15.3, 11.4, 12.5, 15.1, 17, 15.7, 16.4, 14.4, 12.6, 12.9, 16.9, 16.4, 16.1, 17.8, 19.4, 17.3, 16, 14.9, 16.2, 20.7, 14.2, 14.4, 16.8, 14.8, 18.3, 20.4, 19.6, 12.6, 13.8, 15.8, 19, 17.1, 16.6, 19.6, 18.6, 18, 16.2, 16, 18, 16.4, 15.3, 18.2, 17.6, 14.7, 17.3, 14.5, 14.5, 16.9, 15, 15.7, 16.2, 16.4, 17, 14.5, 14.7, 13.9, 13, 17.3, 15.6, 24.6, 11.6, 18.6, 19.4
        });
        auto model_year = Array<float>::from_vector(std::vector<double>{
            70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82
        });
        auto origin = Array<float>::from_vector(std::vector<double>{
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 2, 1, 3, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 2, 2, 2, 2, 1, 3, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 3, 1, 3, 3, 1, 1, 2, 1, 1, 2, 2, 2, 2, 1, 2, 3, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 1, 2, 2, 3, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 3, 2, 3, 1, 2, 1, 2, 2, 2, 2, 3, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 1, 1, 1, 1, 2, 3, 3, 1, 2, 1, 2, 3, 2, 1, 1, 1, 1, 3, 1, 2, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 3, 2, 3, 2, 3, 2, 1, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 1, 3, 1, 1, 3, 2, 2, 2, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 1, 1, 2, 1, 2, 1, 1, 1, 3, 2, 1, 1, 1, 1, 2, 3, 1, 3, 1, 1, 1, 1, 2, 3, 3, 3, 3, 3, 1, 3, 2, 2, 2, 2, 3, 3, 2, 3, 3, 2, 3, 1, 1, 1, 1, 1, 3, 1, 3, 3, 3, 3, 3, 1, 1, 1, 2, 3, 3, 3, 3, 2, 2, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 3, 1, 1, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 3, 1, 1, 1, 2, 1, 1, 1
        });
        auto constant {
            Array<float>::constant(1, origin.size(0))
        };
        auto features = linear_regression::build_features( // double-checks and removes colinearity
            constant, cylinders, displacement, horsepower, weight, acceleration, model_year, origin
        );

        auto weights = linear_regression::solve_for_weights(mpg, features);
        auto std_err = linear_regression::standard_error(mpg, features, weights);
        auto std_dev = linear_regression::standard_deviation(mpg, features, weights);
        auto t_stat = linear_regression::t_statistic(weights, std_err);
        auto p_value = linear_regression::p_value(features, t_stat);
        auto lower_95 = weights - (1.96 * std_err);
        auto upper_95 = weights + (1.96 * std_err);
        auto prediction = linear_regression::predict(features, weights);

        print("");
        print(
            weights.join(1,
                std_err).join(1,
                    t_stat).join(1,
                        p_value).join(1,
                            lower_95).join(1,
                                upper_95).to_string(
                                    { "Coefficients", "Standard Error", "t Stat", "P-value", "Lower 95%", "Upper 95%" })
        );
        print("");
        print(mpg.join(1, prediction).to_string({ "Measured", "Predicted" }));
        print("");
    }

    // Fish market regression. Not being performed correctly in any way, for reasons not yet understood.
    if (1) {
        /*          Coefficients	Standard Error	t Stat	        P-value	        Lower 95%	    Upper 95%
        Intercept	-499.5869554	29.57223974	    -16.89378146	8.44136E-37	    -558.0095858	-441.1643249
        Length1	    62.35521443	    40.20873868	    1.550787627	    0.123018636	    -17.08078028	141.7912091
        Length2	    -6.526752492	41.75876083	    -0.156296604	0.876005273	    -89.02495596	75.97145098
        Length3	    -29.02621861	17.35295765	    -1.672695756	0.096430795	    -63.30855369	5.256116464
        Height	    28.29735132	    8.729226223	    3.241679228	    0.001458477	    11.05197654	    45.54272611
        Width	    22.47330665	    20.37173285	    1.103161268	    0.271689248	    -17.77289147	62.71950478
        */

        auto weight = Array<float>::from_vector(std::vector<double>{
            242, 290, 340, 363, 430, 450, 500, 390, 450, 500, 475, 500, 500, 340, 600, 600, 700, 700, 610, 650, 575, 685, 620, 680, 700, 725, 720, 714, 850, 1000, 920, 955, 925, 975, 950, 40, 69, 78, 87, 120, 0, 110, 120, 150, 145, 160, 140, 160, 169, 161, 200, 180, 290, 272, 390, 270, 270, 306, 540, 800, 1000, 55, 60, 90, 120, 150, 140, 170, 145, 200, 273, 300, 5.9, 32, 40, 51.5, 70, 100, 78, 80, 85, 85, 110, 115, 125, 130, 120, 120, 130, 135, 110, 130, 150, 145, 150, 170, 225, 145, 188, 180, 197, 218, 300, 260, 265, 250, 250, 300, 320, 514, 556, 840, 685, 700, 700, 690, 900, 650, 820, 850, 900, 1015, 820, 1100, 1000, 1100, 1000, 1000, 200, 300, 300, 300, 430, 345, 456, 510, 540, 500, 567, 770, 950, 1250, 1600, 1550, 1650, 6.7, 7.5, 7, 9.7, 9.8, 8.7, 10, 9.9, 9.8, 12.2, 13.4, 12.2, 19.7, 19.9
        });
        auto length1 = Array<float>::from_vector(std::vector<double>{
            23.2, 24, 23.9, 26.3, 26.5, 26.8, 26.8, 27.6, 27.6, 28.5, 28.4, 28.7, 29.1, 29.5, 29.4, 29.4, 30.4, 30.4, 30.9, 31, 31.3, 31.4, 31.5, 31.8, 31.9, 31.8, 32, 32.7, 32.8, 33.5, 35, 35, 36.2, 37.4, 38, 12.9, 16.5, 17.5, 18.2, 18.6, 19, 19.1, 19.4, 20.4, 20.5, 20.5, 21, 21.1, 22, 22, 22.1, 23.6, 24, 25, 29.5, 23.6, 24.1, 25.6, 28.5, 33.7, 37.3, 13.5, 14.3, 16.3, 17.5, 18.4, 19, 19, 19.8, 21.2, 23, 24, 7.5, 12.5, 13.8, 15, 15.7, 16.2, 16.8, 17.2, 17.8, 18.2, 19, 19, 19, 19.3, 20, 20, 20, 20, 20, 20.5, 20.5, 20.7, 21, 21.5, 22, 22, 22.6, 23, 23.5, 25, 25.2, 25.4, 25.4, 25.4, 25.9, 26.9, 27.8, 30.5, 32, 32.5, 34, 34, 34.5, 34.6, 36.5, 36.5, 36.6, 36.9, 37, 37, 37.1, 39, 39.8, 40.1, 40.2, 41.1, 30, 31.7, 32.7, 34.8, 35.5, 36, 40, 40, 40.1, 42, 43.2, 44.8, 48.3, 52, 56, 56, 59, 9.3, 10, 10.1, 10.4, 10.7, 10.8, 11.3, 11.3, 11.4, 11.5, 11.7, 12.1, 13.2, 13.8
        });
        auto length2 = Array<float>::from_vector(std::vector<double>{
            25.4, 26.3, 26.5, 29, 29, 29.7, 29.7, 30, 30, 30.7, 31, 31, 31.5, 32, 32, 32, 33, 33, 33.5, 33.5, 34, 34, 34.5, 35, 35, 35, 35, 36, 36, 37, 38.5, 38.5, 39.5, 41, 41, 14.1, 18.2, 18.8, 19.8, 20, 20.5, 20.8, 21, 22, 22, 22.5, 22.5, 22.5, 24, 23.4, 23.5, 25.2, 26, 27, 31.7, 26, 26.5, 28, 31, 36.4, 40, 14.7, 15.5, 17.7, 19, 20, 20.7, 20.7, 21.5, 23, 25, 26, 8.4, 13.7, 15, 16.2, 17.4, 18, 18.7, 19, 19.6, 20, 21, 21, 21, 21.3, 22, 22, 22, 22, 22, 22.5, 22.5, 22.7, 23, 23.5, 24, 24, 24.6, 25, 25.6, 26.5, 27.3, 27.5, 27.5, 27.5, 28, 28.7, 30, 32.8, 34.5, 35, 36.5, 36, 37, 37, 39, 39, 39, 40, 40, 40, 40, 42, 43, 43, 43.5, 44, 32.3, 34, 35, 37.3, 38, 38.5, 42.5, 42.5, 43, 45, 46, 48, 51.7, 56, 60, 60, 63.4, 9.8, 10.5, 10.6, 11, 11.2, 11.3, 11.8, 11.8, 12, 12.2, 12.4, 13, 14.3, 15
        });
        auto length3 = Array<float>::from_vector(std::vector<double>{
            30, 31.2, 31.1, 33.5, 34, 34.7, 34.5, 35, 35.1, 36.2, 36.2, 36.2, 36.4, 37.3, 37.2, 37.2, 38.3, 38.5, 38.6, 38.7, 39.5, 39.2, 39.7, 40.6, 40.5, 40.9, 40.6, 41.5, 41.6, 42.6, 44.1, 44, 45.3, 45.9, 46.5, 16.2, 20.3, 21.2, 22.2, 22.2, 22.8, 23.1, 23.7, 24.7, 24.3, 25.3, 25, 25, 27.2, 26.7, 26.8, 27.9, 29.2, 30.6, 35, 28.7, 29.3, 30.8, 34, 39.6, 43.5, 16.5, 17.4, 19.8, 21.3, 22.4, 23.2, 23.2, 24.1, 25.8, 28, 29, 8.8, 14.7, 16, 17.2, 18.5, 19.2, 19.4, 20.2, 20.8, 21, 22.5, 22.5, 22.5, 22.8, 23.5, 23.5, 23.5, 23.5, 23.5, 24, 24, 24.2, 24.5, 25, 25.5, 25.5, 26.2, 26.5, 27, 28, 28.7, 28.9, 28.9, 28.9, 29.4, 30.1, 31.6, 34, 36.5, 37.3, 39, 38.3, 39.4, 39.3, 41.4, 41.4, 41.3, 42.3, 42.5, 42.4, 42.5, 44.6, 45.2, 45.5, 46, 46.6, 34.8, 37.8, 38.8, 39.8, 40.5, 41, 45.5, 45.5, 45.8, 48, 48.7, 51.2, 55.1, 59.7, 64, 64, 68, 10.8, 11.6, 11.6, 12, 12.4, 12.6, 13.1, 13.1, 13.2, 13.4, 13.5, 13.8, 15.2, 16.2
        });
        auto height = Array<float>::from_vector(std::vector<double>{
            11.52, 12.48, 12.3778, 12.73, 12.444, 13.6024, 14.1795, 12.67, 14.0049, 14.2266, 14.2628, 14.3714, 13.7592, 13.9129, 14.9544, 15.438, 14.8604, 14.938, 15.633, 14.4738, 15.1285, 15.9936, 15.5227, 15.4686, 16.2405, 16.36, 16.3618, 16.517, 16.8896, 18.957, 18.0369, 18.084, 18.7542, 18.6354, 17.6235, 4.1472, 5.2983, 5.5756, 5.6166, 6.216, 6.4752, 6.1677, 6.1146, 5.8045, 6.6339, 7.0334, 6.55, 6.4, 7.5344, 6.9153, 7.3968, 7.0866, 8.8768, 8.568, 9.485, 8.3804, 8.1454, 8.778, 10.744, 11.7612, 12.354, 6.8475, 6.5772, 7.4052, 8.3922, 8.8928, 8.5376, 9.396, 9.7364, 10.3458, 11.088, 11.368, 2.112, 3.528, 3.824, 4.5924, 4.588, 5.2224, 5.1992, 5.6358, 5.1376, 5.082, 5.6925, 5.9175, 5.6925, 6.384, 6.11, 5.64, 6.11, 5.875, 5.5225, 5.856, 6.792, 5.9532, 5.2185, 6.275, 7.293, 6.375, 6.7334, 6.4395, 6.561, 7.168, 8.323, 7.1672, 7.0516, 7.2828, 7.8204, 7.5852, 7.6156, 10.03, 10.2565, 11.4884, 10.881, 10.6091, 10.835, 10.5717, 11.1366, 11.1366, 12.4313, 11.9286, 11.73, 12.3808, 11.135, 12.8002, 11.9328, 12.5125, 12.604, 12.4888, 5.568, 5.7078, 5.9364, 6.2884, 7.29, 6.396, 7.28, 6.825, 7.786, 6.96, 7.792, 7.68, 8.9262, 10.6863, 9.6, 9.6, 10.812, 1.7388, 1.972, 1.7284, 2.196, 2.0832, 1.9782, 2.2139, 2.2139, 2.2044, 2.0904, 2.43, 2.277, 2.8728, 2.9322
        });
        auto width = Array<float>::from_vector(std::vector<double>{
            4.02, 4.3056, 4.6961, 4.4555, 5.134, 4.9274, 5.2785, 4.69, 4.8438, 4.9594, 5.1042, 4.8146, 4.368, 5.0728, 5.1708, 5.58, 5.2854, 5.1975, 5.1338, 5.7276, 5.5695, 5.3704, 5.2801, 6.1306, 5.589, 6.0532, 6.09, 5.8515, 6.1984, 6.603, 6.3063, 6.292, 6.7497, 6.7473, 6.3705, 2.268, 2.8217, 2.9044, 3.1746, 3.5742, 3.3516, 3.3957, 3.2943, 3.7544, 3.5478, 3.8203, 3.325, 3.8, 3.8352, 3.6312, 4.1272, 3.906, 4.4968, 4.7736, 5.355, 4.2476, 4.2485, 4.6816, 6.562, 6.5736, 6.525, 2.3265, 2.3142, 2.673, 2.9181, 3.2928, 3.2944, 3.4104, 3.1571, 3.6636, 4.144, 4.234, 1.408, 1.9992, 2.432, 2.6316, 2.9415, 3.3216, 3.1234, 3.0502, 3.0368, 2.772, 3.555, 3.3075, 3.6675, 3.534, 3.4075, 3.525, 3.525, 3.525, 3.995, 3.624, 3.624, 3.63, 3.626, 3.725, 3.723, 3.825, 4.1658, 3.6835, 4.239, 4.144, 5.1373, 4.335, 4.335, 4.5662, 4.2042, 4.6354, 4.7716, 6.018, 6.3875, 7.7957, 6.864, 6.7408, 6.2646, 6.3666, 7.4934, 6.003, 7.3514, 7.1064, 7.225, 7.4624, 6.63, 6.8684, 7.2772, 7.4165, 8.142, 7.5958, 3.3756, 4.158, 4.3844, 4.0198, 4.5765, 3.977, 4.3225, 4.459, 5.1296, 4.896, 4.87, 5.376, 6.1712, 6.9849, 6.144, 6.144, 7.48, 1.0476, 1.16, 1.1484, 1.38, 1.2772, 1.2852, 1.2838, 1.1659, 1.1484, 1.3936, 1.269, 1.2558, 2.0672, 1.8792
        });
        auto constant{
            Array<float>::constant(1, width.size(0))
        };
        auto features = linear_regression::build_features( // double-checks and removes colinearity
            constant, length1, length2, length3, height, width
        );

        auto weights = linear_regression::solve_for_weights(weight, features);
        auto std_err = linear_regression::standard_error(weight, features, weights);
        auto std_dev = linear_regression::standard_deviation(weight, features, weights);
        auto t_stat = linear_regression::t_statistic(weights, std_err);
        auto p_value = linear_regression::p_value(features, t_stat);
        auto lower_95 = weights - (1.96 * std_err);
        auto upper_95 = weights + (1.96 * std_err);
        auto prediction = linear_regression::predict(features, weights);

        print("");
        print(
            weights.join(1,
                std_err).join(1,
                    t_stat).join(1,
                        p_value).join(1,
                            lower_95).join(1,
                                upper_95).to_string(
                                    { "Coefficients", "Standard Error", "t Stat", "P-value", "Lower 95%", "Upper 95%" })
        );
        print("");

        print(weight.join(1, prediction).to_string({ "Measured", "Predicted" }));
        print("");

    }









    const unsigned int N = 1000000u; // size of vectors
    if (1) {
        if (1) {
            print(Array<float>::random(N));
            print(Array<unsigned int>::random(N));
            print(Array<int>::random(N));
            print(Array<unsigned char>::random_between('0', 'z', N));
        }

        if (1) {
            Array<float> A(N);
            A = 0;
            A += 5;
            auto B = A.pow(2).round();
            EXPECT_EQ((B == 25.0f).sum(), 0);
            for (unsigned int n = 0u; n < N; n++) EXPECT_EQ(B[n], 25.0f);
            auto C = (B * -1.0f).abs().round();
            for (unsigned int n = 0u; n < N; n++) EXPECT_EQ(C[n], 25.0f);
            
            A = 5;
            auto D = (A % 2.0f).round();
            EXPECT_EQ(1.0f, D[0]);
        }
        if (1) {
            Array<float> A(10, 100000);
            for (unsigned int col = 0u; col < 10; col++) {
                float v = col * 100000;
                for (unsigned int row = 0u; row < 100000; row++, v++) A(col, row) = v;                
            }
        }
        if (1) {
            Array<float> A(N);
            Array<float> B(N);

            A = 0; 
            B = 1;

            auto C = ((A + (B * 2.0f)) / 2.0f);
            for (unsigned int n = 0u; n < N; n++) EXPECT_EQ(C[n], 1.0f);
            
            auto D = C.cast<int>();
            for (unsigned int n = 0u; n < N; n++) EXPECT_EQ(D[n], 1.0f);            

            auto CMP = (D == C.cast<int>());
            for (unsigned int n = 0u; n < N; n++) EXPECT_EQ(CMP[n], true);
        }
        if (1) {
            Array<float> A(N);
            Array<float> B(N);

            A = 0;
            B = 2;

            auto C = A + B; // 0 + 2 = 2
            auto D = C * B; // 2 * 2 = 4

            for (unsigned int n = 0u; n < N; n++) EXPECT_EQ(C[n], 2);
            for (unsigned int n = 0u; n < N; n++) EXPECT_EQ(D[n], 4);

            auto E = ((((((((A + B) * B) + B) * B) + B) * B) + B) * B);
            EXPECT_EQ(E[0], 60);
            for (unsigned int n = 0u; n < N; n++) EXPECT_EQ(E[n], 60);
            

            EXPECT_EQ(A[0], 0);
            for (unsigned int n = 0u; n < N; n++) EXPECT_EQ(A[n], 0);            
            A += B;
            A *= B;
            A += B;
            A *= B;
            A += B;
            A *= B;
            A += B;
            A *= B;
            EXPECT_EQ(A[0], 60);
            for (unsigned int n = 0u; n < N; n++) EXPECT_EQ(A[n], 60);
            
        }
        if (1) {
            Array<int> A(N);
            Array<int> B(N);

            // initialize memory
            for (unsigned int n = 0u; n < N; n++) {
                A[n] = 0;
                B[n] = 2;
            }

            A.data->write_to_device();
            B.data->write_to_device();

            auto C = A + B; // 0 + 2 = 2
            auto D = C * B; // 2 * 2 = 4

            for (unsigned int n = 0u; n < N; n++) EXPECT_EQ(C[n], 2);
            for (unsigned int n = 0u; n < N; n++) EXPECT_EQ(D[n], 4);
            auto E = ((((((((A + B) * B) + B) * B) + B) * B) + B) * B);
            for (unsigned int n = 0u; n < N; n++) EXPECT_EQ(E[n], 60);            
            for (unsigned int n = 0u; n < N; n++) EXPECT_EQ(A[n], 0);            
            A += B;
            A *= B;
            A += B;
            A *= B;
            A += B;
            A *= B;
            A += B;
            A *= B;
            for (unsigned int n = 0u; n < N; n++) EXPECT_EQ(A[n], 60);            
        }
    }



}
