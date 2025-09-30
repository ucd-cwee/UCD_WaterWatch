// GpuProgramming.cpp : Defines the functions for the static library.
//

#include <iostream>
#include "GpuProgramming.h"
#include "../arrayfire/include/CL/opencl.hpp"
#include "opencl.hpp"

#define print(a) std::cout << a << std::endl
#define EXPECT_EQ(a, b) if (a != b){ std::cout << "FAILURE AT LINE " << __LINE__ << std::endl; }
#define EXPECT_NE(a, b) if (a == b){ std::cout << "FAILURE AT LINE " << __LINE__ << std::endl; }


class ArrayTasks {
public:
    ArrayTasks() = default;
    ArrayTasks(ArrayTasks const&) = default;
    ArrayTasks(ArrayTasks &&) = default;
    ArrayTasks& operator=(ArrayTasks const&) = default;
    ArrayTasks& operator=(ArrayTasks&&) = default;
    ~ArrayTasks() = default;

    std::vector<Event>& get() {
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
    unsigned int Dim;

    Array()
        : data{ nullptr }
        , LenX{ 0 }
        , LenY{ 0 }
        , LenZ{ 0 }
        , Dim{ 0 }
        , tasks{}
        , working{ false }
        , local{ true }
    {};
    Array(unsigned int lenX)
        : data{ std::make_shared<Memory<T>>(GetDevice(), lenX, 1) }
        , LenX{ lenX }
        , LenY{ 1 }
        , LenZ{ 1 }
        , Dim{ 1 }
        , tasks{}
        , working{ false }
        , local{ true }
    {};
    Array(unsigned int lenX, unsigned int lenY)
        : data{ std::make_shared<Memory<T>>(GetDevice(), lenX * lenY, 2) }
        , LenX{ lenX }
        , LenY{ lenY }
        , LenZ{ 1 }
        , Dim{ 2 }
        , tasks{}
        , working{ false }
        , local{ true }
    {};
    Array(unsigned int lenX, unsigned int lenY, unsigned int lenZ)
        : data{ std::make_shared<Memory<T>>(GetDevice(), lenX * lenY * lenZ, 3) }
        , LenX{ lenX }
        , LenY{ lenY }
        , LenZ{ lenZ }
        , Dim{ 3 }
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
            out.LenX = LenY;
            out.LenX = LenZ;
            out.Dim = Dim;
            out.working = false;
            out.local = true;
        }
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

    // call this if this vector has been manually updated on the host, or if you need the results to be finished. Slow on repeat calls.
    void sync() {
        stop_work();
        if (data) data->write_to_device();
    }

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
            out.tasks = lhs.tasks;
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
            out.tasks = lhs.tasks;
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
            out.tasks = lhs.tasks;
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
            out.tasks = lhs.tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
            out.working = true;
            out.local = false;
        }
        out.work("absolute", *data, *out.data);
        return out;
    };

    Array sin() const {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), LenX * LenY * LenZ, Dim, false, true);
            out.LenX = LenX;
            out.LenY = LenY;
            out.LenZ = LenZ;
            out.Dim = Dim;
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
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
            out.tasks = tasks;
            out.working = true;
            out.local = false;
        }
        out.work("Min_single", *data, rhs, *out.data);
        return out;
    };

    Array<unsigned int> operator==(T rhs) const {
        Array<unsigned int > out; {
            out.data = std::make_shared<Memory<unsigned int >>(Array<unsigned int >::GetDevice(), this->LenX * this->LenY * this->LenZ, this->Dim, false, true);
            out.LenX = this->LenX;
            out.LenY = this->LenY;
            out.LenZ = this->LenZ;
            out.Dim = this->Dim;
            out.tasks = this->tasks;
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
            out.tasks = this->tasks;
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
    Array join(unsigned int jdim, Array const& first) {        
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

    // transpose a 2-D matrix along its diagonal. Does not support transposition of 3-D matrices. 
    Array transpose() const {
        // matrix must be 2-D
        if (this->Dim == 0) return Array();
        else if (this->Dim > 2) return Array();
        
        Array out;
        out.tasks = this->tasks;
        out.working = true;
        out.local = false;
        out.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), LenX * LenY, 2, false, true);
        out.LenX = LenY;
        out.LenY = LenX;
        out.LenZ = 1;
        out.Dim = 2;

        Array<unsigned int> lengths(4); {
            lengths[0] = this->LenX;
            lengths[1] = this->LenY;
            lengths[2] = out.LenX;
            lengths[3] = out.LenY;
            lengths.sync();
        }
        
        out.work("Transpose", *out.data, *this->data, (unsigned int)LenX, (unsigned int)LenY);

        return out;
    }

    // pad a matrix with zeros to make its X and Y components square. Used for calculating the inverse. 
    Array make_square() const {
        unsigned int len = std::max<unsigned int>(LenY, LenX);

        Array out;
        out.tasks = this->tasks;
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
                result = result + sign * (*this)(0,i) * subVect.determinant();
                sign = -sign;
            }

            return result;
        }        
    }

    // cofactor of a square matrix, essential for calculating the inverse
    Array cofactor() const {
        if (this->LenX != this->LenY) {
            return {};
        }
        size_t dimension = this->LenX;
        Array solution; {
            solution.tasks = this->tasks;
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
    Array adjoint() const {
        return cofactor().transpose();
    }


    // solve for the inverse of the matrix. Does not support solving for the inverse of a 3-D matrix. 
    Array inverse() const {
        if (this->LenX != this->LenY) {
            return {};
        }
        size_t dimension = this->LenX;

        float det = this->determinant();
        if (det == 0) {
            return {};
        }

        float d = 1.0 / det;

        Array solution; {
            solution.working = false;
            solution.local = true;
            solution.data = std::make_shared<Memory<T>>(Array<T>::GetDevice(), (dimension) * (dimension), 2, true, true);
            solution.LenX = dimension;
            solution.LenY = dimension;
            solution.LenZ = 1;
            solution.Dim = 2;
        }

        for (size_t i = 0; i < dimension; i++) {
            for (size_t j = 0; j < dimension; j++) {
                solution(i,j) = (*this)(i, j);
            }
        }

        auto adj = solution.cofactor().transpose();

        for (size_t i = 0; i < dimension; i++) {
            for (size_t j = 0; j < dimension; j++) {
                adj(i,j) *= d;
            }
        }

        return adj;
    };



    template<typename G>
    Array<G> cast() const {
        if constexpr (std::is_same_v<G, T>) {
            return *this;
        }

        Array<G> out; {
            out.data = std::make_shared<Memory<G>>(Array<G>::GetDevice(), this->LenX * this->LenY * this->LenZ, this->Dim, false, true);
            out.LenX = this->LenX;
            out.LenY = this->LenY;
            out.LenZ = this->LenZ;
            out.Dim = this->Dim;
            out.tasks = this->tasks;
            out.working = true;
            out.local = false;
        }

        out.work(std::string("from_") + type_name<T>(), *out.data, *this->data);

        return out;
    };

    // For floating-point values, returns 0-1. For all others, returns the range from 0 to the max value. 
    template <typename... P> static Array random(const P&... parameters) {
        if constexpr (std::is_floating_point_v<T> || std::is_same_v<unsigned int, T>) {
            Array out(parameters...);
            out.work("Rand", *out.data);
            return out;
        }
        else {
            Array<float> out(parameters...);
            out.work("Rand", *out.data);
            out *= std::numeric_limits<T>::max();
            return out.cast<T>();
        }
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

public:
    // y-axis are columns, x-axis are rows. Z-axis is ignored (for now). 
    std::string to_string() const {
        std::string out;
        if (this->Dim == 0) return out;
        else if (this->Dim == 1) {
            unsigned int n = 0;
            for (; (n < this->size()) && (n < 1); ++n) {
                out += to_string_impl(n);
            }
            for (; n < this->size(); ++n) {
                out += "\n";
                out += to_string_impl(n);
            }
        }
        else if (this->Dim == 2) {
            unsigned int n = 0;
            for (; (n < this->LenX) && (n < 1); ++n) {
                unsigned int y = 0;
                for (; (y < this->LenY) && (y < 1); ++y) {
                    out += to_string_impl(n, y);
                }
                for (; y < this->LenY; ++y) {
                    out += "\t";
                    out += to_string_impl(n, y);
                }                
            }
            for (; n < this->LenX; ++n) {
                out += "\n";
                unsigned int y = 0;
                for (; (y < this->LenY) && (y < 1); ++y) {
                    out += to_string_impl(n, y);
                }
                for (; y < this->LenY; ++y) {
                    out += "\t";
                    out += to_string_impl(n, y);
                }
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
        // https://stackoverflow.com/questions/60300482/c-calculating-the-inverse-of-a-matrix
        Array<float> mat(3, 3);
        for (int i = 0; i < 9; ++i) mat[i] = i + 1;
        mat[8] = 8;
        mat.sync();
        mat = mat.transpose();

        print(mat);
        print("");
        print(mat.transpose());
        print("");
        print(mat.determinant());
        print("");
        print(mat.cofactor());
        print("");
        print(mat.adjoint());
        print("");
        print((mat.adjoint() / std::abs(mat.determinant())).to_string());
        print("");
        print(mat.inverse());



    }



    if (1) {
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
        Array<float> Basic(Sales_Revenue.size()); Basic = 1.0;

        auto features = Basic.join(1, TV_Ads).join(1, Radio_Ads).join(1, Newspaper_Ads);


        print((Array<char>(4, 2) = 'T').make_square().to_string());


        print(features.make_square().to_string());
        print(TV_Ads.to_string());
        print(TV_Ads.transpose().to_string());


        // return (features.transpose().matrix_multiplication(features)).inverse().matrix_multiplication(features.transpose()).matrix_multiplication(measurements);





    }








    //Typically, you have one thread which intializes the shared(local) atomic followed by some barrier.I.e.your kernel starts like this:

    //__local int sharedNum;
    //if (get_local_id(0) == 0) {
    //    sharedNum = 0;
    //}
    //barrier(CLK_LOCAL_MEM_FENCE);

    //// Now, you can use sharedNum
    //while (is_work_left()) {
    //    atomic_inc(&sharedNum);
    //}
    //There's not much magic to it -- all items in a work-group can see the same local variables, so you can just access it as usual.



    const unsigned int N = 1000000u; // size of vectors
    if (1) {
        if (1) {
            auto A{ Array<float>::random(N) };
            for (unsigned int n = 0u; n < 10; n++) {
                print(A[n]);
            }
        }
        if (1) {
            auto A{ Array<unsigned int>::random(N) };
            for (unsigned int n = 0u; n < 10; n++) {
                print(A[n]);
            }
        }
        if (1) {
            auto A{ Array<int>::random(N) };
            for (unsigned int n = 0u; n < 10; n++) {
                print(A[n]);
            }
        }
        if (1) {
            auto A{ Array<unsigned char>::random(N) };
            for (unsigned int n = 0u; n < 10; n++) {
                print(A[n]);
            }
        }

        if (1) {
            Array<float> A(N);
            A = 0;
            A += 5;
            auto B = A.pow(2).round();
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


        //Array<float>::enqueue_kernel(N, "add_inplace", *A.data, *B.data);  // 0 + 2 = 2
        //Array<float>::enqueue_kernel(N, "mult_inplace", *A.data, *B.data); // 2 * 2 = 4
        //Array<float>::enqueue_kernel(N, "add_inplace", *A.data, *B.data);  // 4 + 2 = 6
        //Array<float>::enqueue_kernel(N, "mult_inplace", *A.data, *B.data); // 6 * 2 = 12
        //Array<float>::enqueue_kernel(N, "add_inplace", *A.data, *B.data);  // 12 + 2 = 14
        //Array<float>::enqueue_kernel(N, "mult_inplace", *A.data, *B.data); // 14 * 2 = 28
        //Array<float>::enqueue_kernel(N, "add_inplace", *A.data, *B.data);  // 28 + 2 = 30
        //Array<float>::enqueue_kernel(N, "mult_inplace", *A.data, *B.data); // 30 * 2 = 60
        //Array<float>::complete_kernels();

        //A.data->read_from_device();

        //for (unsigned int n = 0u; n < N; n++) {
        //    EXPECT_EQ(A[n], 60);
        //}
    }
    














    //Device device(select_device_with_most_flops()); // compile OpenCL C code for the fastest available device
    //Memory<float> A(device, N, 3); // allocate memory on both host and device
    //Memory<float> B(device, N, 3);
    //Memory<float> C(device, N, 3);
    //
    //

    //// initialize memory
    //for (unsigned int n = 0u; n < N; n++) {
    //    A[n] = 0.0f; 
    //    B[n] = 2.0f;
    //    C[n] = 0.0f;
    //}

    //A.write_to_device(); // copy data from host memory to device memory
    //B.write_to_device();

    //if (1) {
    //    Kernel add_kernel(device, N, "add_inplace", A, B); // kernel that runs on the device
    //    Kernel mult_kernel(device, N, "mult_inplace", A, B); // kernel that runs on the device

    //    std::vector<Event> events;

    //    add_kernel.enqueue_run(1, &events); // 0 + 2 = 2
    //    mult_kernel.enqueue_run(1, &events); // 2 * 2 = 4

    //    add_kernel.enqueue_run(1, &events); // 4 + 2 = 6
    //    mult_kernel.enqueue_run(1, &events); // 6 * 2 = 12

    //    add_kernel.enqueue_run(1, &events); // 12 + 2 = 14
    //    mult_kernel.enqueue_run(1, &events); // 14 * 2 = 28

    //    add_kernel.enqueue_run(1, &events); // 28 + 2 = 30
    //    mult_kernel.enqueue_run(1, &events); // 30 * 2 = 60

    //    Event::waitForEvents(events);

    //    //add_kernel.finish_queue();
    //    //mult_kernel.finish_queue();
    //}

    //A.read_from_device(); // copy data from device memory to host memory

    //for (unsigned int n = 0u; n < N; n++) {
    //    EXPECT_EQ(A[n], 60);
    //}


    //if (1) {
    //    Kernel add_kernel(device, N, "add_inplace", A, B); // kernel that runs on the device
    //    Kernel mult_kernel(device, N, "mult_inplace", A, B); // kernel that runs on the device
    //    
    //    add_kernel.finish_queue();
    //    mult_kernel.finish_queue();
    //}

    //A.read_from_device(); // copy data from device memory to host memory
    //
    //for (unsigned int n = 0u; n < N; n++) {
    //    EXPECT_EQ(A[n], 60);
    //}









    //try {
    //    // Get list of OpenCL platforms.
    //    std::vector<cl::Platform> platform;
    //    cl::Platform::get(&platform);

    //    if (platform.empty()) {
    //        std::cerr << "OpenCL platforms not found." << std::endl;
    //        return;
    //    }

    //    // Get first available GPU device which supports double precision.
    //    cl::Context context;
    //    std::vector<cl::Device> device;
    //    for (auto p = platform.begin(); device.empty() && p != platform.end(); p++) {
    //        std::vector<cl::Device> pldev;

    //        try {
    //            p->getDevices(CL_DEVICE_TYPE_GPU, &pldev);

    //            for (auto d = pldev.begin(); device.empty() && d != pldev.end(); d++) {
    //                if (!d->getInfo<CL_DEVICE_AVAILABLE>()) continue;

    //                std::string ext = d->getInfo<CL_DEVICE_EXTENSIONS>();
    //                print(ext);

    //                std::cout << d->getInfo<CL_DEVICE_NAME>() << std::endl;

    //                device.push_back(*d);
    //                context = cl::Context(device);

    //                // Create command queue.
    //                cl::CommandQueue queue(context, device[0]);

    //                // Compute c = a + b.
    //                std::string script =
    //                    "kernel void add(\n"
    //                    "       ulong n,\n"
    //                    "       global const float *a,\n"
    //                    "       global const float *b,\n"
    //                    "       global float *c\n"
    //                    "       )\n"
    //                    "{\n"
    //                    "    size_t i = get_global_id(0);\n"
    //                    "    if (i < n) {\n"
    //                    "       c[i] = a[i] + b[i];\n"
    //                    "    }\n"
    //                    "}\n";

    //                // Compile OpenCL program for found device.
    //                cl::Program program(context, cl::Program::Sources{ script }, nullptr);

    //                try {
    //                    program.build(device);
    //                }
    //                catch (...) {
    //                    std::cerr
    //                        << "OpenCL compilation error" << std::endl
    //                        << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device[0])
    //                        << std::endl;
    //                    return;
    //                }


    //                cl::Kernel add(program, "add");

    //                // Prepare input data.
    //                const size_t N = 1 << 20;
    //                std::vector<float> a(N, 1);
    //                std::vector<float> b(N, 2);
    //                std::vector<float> c(N);

    //                // Allocate device buffers and transfer input data to device.
    //                cl::Buffer A(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
    //                    a.size() * sizeof(float), a.data());

    //                cl::Buffer B(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
    //                    b.size() * sizeof(float), b.data());

    //                cl::Buffer C(context, CL_MEM_READ_WRITE,
    //                    c.size() * sizeof(float));

    //                // Set kernel parameters.
    //                add.setArg(0, static_cast<cl_ulong>(N));
    //                add.setArg(1, A);
    //                add.setArg(2, B);
    //                add.setArg(3, C);

    //                // Launch kernel on the compute device.
    //                queue.enqueueNDRangeKernel(add, cl::NullRange, N, cl::NullRange);

    //                // Get result back to host.
    //                queue.enqueueReadBuffer(C, CL_TRUE, 0, c.size() * sizeof(float), c.data());

    //                // Should get '3' here.
    //                EXPECT_EQ(c[42], 3);
    //            }
    //        }
    //        catch (...) {
    //            device.clear();
    //        }
    //    }

    //    if (device.empty()) {
    //        std::cerr << "GPUs with double precision not found." << std::endl;
    //        return;
    //    }

    //}
    //catch (...) {
    //    return;
    //}



}
