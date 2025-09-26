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
    uint Dim;

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
    Array(uint lenX)
        : data{ std::make_shared<Memory<T>>(GetDevice(), lenX, 1) }
        , LenX{ lenX }
        , LenY{ 1 }
        , LenZ{ 1 }
        , Dim{ 1 }
        , tasks{}
        , working{ false }
        , local{ true }
    {};
    Array(uint lenX, uint lenY)
        : data{ std::make_shared<Memory<T>>(GetDevice(), lenX * lenY, 2) }
        , LenX{ lenX }
        , LenY{ lenY }
        , LenZ{ 1 }
        , Dim{ 2 }
        , tasks{}
        , working{ false }
        , local{ true }
    {};
    Array(uint lenX, uint lenY, uint lenZ)
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

    template<class... T> static inline void enqueue_kernel(const ulong N, const string& name, const T&... parameters) { // accepts Memory<T> objects and fundamental data type constants        
        Kernel kernel(GetDevice(), N, name, parameters...); // kernel that runs on the device
        kernel.enqueue_run();
    }
    template<class... T> static inline void enqueue_kernel(const ulong N, const uint workgroup_size, const string& name, const T&... parameters) { // accepts Memory<T> objects and fundamental data type constants
        Kernel kernel(GetDevice(), N, workgroup_size, name, parameters...); // kernel that runs on the device
        kernel.enqueue_run();
    }
    static void complete_kernels() {
        GetDevice().get_cl_queue().finish();
    };

protected:
    template<class... T> inline void work(const string& name, const T&... parameters) { // accepts Memory<T> objects and fundamental data type constants
        Kernel kernel(GetDevice(), LenX * LenY * LenZ, name, parameters...);
        Event this_event;
        kernel.enqueue_run(1, &tasks.get(), &this_event);
        tasks.get().push_back(this_event);
        working = true;
    }
    template<class... T> inline void work(const uint workgroup_size, const string& name, const T&... parameters) { // accepts Memory<T> objects and fundamental data type constants
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

    //template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // specialization of POW for integer powers
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
    //template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // power of 
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
    //template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // specialization of POW for integer powers
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
    //template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // power of 
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
    //template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // sqrt
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
    //template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // round to nearest whole number
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
    //template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // round to higher integer
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
    //template<typename = std::enable_if_t<std::is_floating_point_v<T>>> // round to lower integer
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
    // e^x
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
    // 2^x
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
    // 10^x
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
    // e^x-1
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

    Array<uint > operator==(T rhs) const {
        Array<uint > out; {
            out.data = std::make_shared<Memory<uint >>(Array<uint >::GetDevice(), this->LenX * this->LenY * this->LenZ, this->Dim, false, true);
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
    Array<uint > operator!=(T rhs) const {
        Array<uint > out; {
            out.data = std::make_shared<Memory<uint >>(Array<uint>::GetDevice(), this->LenX * this->LenY * this->LenZ, this->Dim, false, true);
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
    friend Array<uint> operator==(Array const& lhs, Array const& rhs) {
        Array<uint> out; {
            out.data = std::make_shared<Memory<uint>>(Array<uint>::GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
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
    friend Array<uint> operator!=(Array const& lhs, Array const& rhs) {
        Array<uint> out; {
            out.data = std::make_shared<Memory<uint>>(Array<uint>::GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim, false, true);
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






    const uint N = 1000000u; // size of vectors
    if (1) {
        if (1) {
            Array<float> A(N);
            A = 0;
            A += 5;
            auto B = A.pow(2).round();
            for (uint n = 0u; n < N; n++) {
                EXPECT_EQ(B[n], 25.0f);
            }
            auto C = (B * -1.0f).abs().round();
            for (uint n = 0u; n < N; n++) {
                EXPECT_EQ(C[n], 25.0f);
            }


            A = 5;
            auto D = (A % 2.0f).round();
            EXPECT_EQ(1.0f, D[0]);







        }
        if (1) {
            Array<float> A(10, 100000);
            // initialize memory
            for (uint col = 0u; col < 10; col++) {
                float v = col * 100000;
                for (uint row = 0u; row < 100000; row++, v++) {
                    A(col, row) = v;
                }
            }
            //for (uint i = 0; i < 1000000; i++)
                //print(A[i]);


        }
        if (1) {





            Array<float> A(N);
            Array<float> B(N);

            A = 0; 
            B = 1;

            // A.data->write_to_device();
            // B.data->write_to_device();

            //auto C = B + 1.0f;
            //EXPECT_EQ(C[0], 2.0f);
            auto C = ((A + (B * 2.0f)) / 2.0f);
            print(C[0]);
            for (uint n = 0u; n < N; n++) {
                EXPECT_EQ(C[n], 1.0f);
            }

            auto D = C.cast<int>();
            print(D[0]);
            for (uint n = 0u; n < N; n++) {
                EXPECT_EQ(D[n], 1.0f);
            }

            auto CMP = (D == C.cast<int>());
            print(CMP[0]);
            for (uint n = 0u; n < N; n++) {
                EXPECT_EQ(CMP[n], 1);
            }








        }

        if (1) {
            Array<float> A(N);
            Array<float> B(N);

            A = 0;
            B = 2;

            auto C = A + B; // 0 + 2 = 2
            auto D = C * B; // 2 * 2 = 4

            for (uint n = 0u; n < N; n++) EXPECT_EQ(C[n], 2);
            for (uint n = 0u; n < N; n++) EXPECT_EQ(D[n], 4);


            auto E = ((((((((A + B) * B) + B) * B) + B) * B) + B) * B);
            EXPECT_EQ(E[0], 60);
            for (uint n = 0u; n < N; n++) {
                EXPECT_EQ(E[n], 60);
            }

            EXPECT_EQ(A[0], 0);
            for (uint n = 0u; n < N; n++) {
                EXPECT_EQ(A[n], 0);
            }
            A += B;
            A *= B;
            A += B;
            A *= B;
            A += B;
            A *= B;
            A += B;
            A *= B;
            EXPECT_EQ(A[0], 60);
            for (uint n = 0u; n < N; n++) {
                EXPECT_EQ(A[n], 60);
            }
        }
        if (1) {
            Array<int> A(N);
            Array<int> B(N);

            // initialize memory
            for (uint n = 0u; n < N; n++) {
                A[n] = 0;
                B[n] = 2;
            }

            A.data->write_to_device();
            B.data->write_to_device();

            auto C = A + B; // 0 + 2 = 2
            auto D = C * B; // 2 * 2 = 4

            for (uint n = 0u; n < N; n++) EXPECT_EQ(C[n], 2);
            for (uint n = 0u; n < N; n++) EXPECT_EQ(D[n], 4);


            auto E = ((((((((A + B) * B) + B) * B) + B) * B) + B) * B);
            for (uint n = 0u; n < N; n++) {
                EXPECT_EQ(E[n], 60);
            }

            for (uint n = 0u; n < N; n++) {
                EXPECT_EQ(A[n], 0);
            }
            A += B;
            A *= B;
            A += B;
            A *= B;
            A += B;
            A *= B;
            A += B;
            A *= B;
            for (uint n = 0u; n < N; n++) {
                EXPECT_EQ(A[n], 60);
            }
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

        //for (uint n = 0u; n < N; n++) {
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
    //for (uint n = 0u; n < N; n++) {
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

    //for (uint n = 0u; n < N; n++) {
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
    //for (uint n = 0u; n < N; n++) {
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
