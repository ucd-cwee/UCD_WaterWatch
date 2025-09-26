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
    static Device& GetDevice() {
        static Device device(select_device_with_most_flops(), get_opencl_c_code());
        return device;
    };

public:
    std::vector<Event> events;
    std::shared_ptr<Memory<T>> data;
    ArrayTasks tasks;
    mutable bool working = false;
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
    {};
    Array(uint lenX)
        : data{ std::make_shared<Memory<T>>(GetDevice(), lenX, 1) }
        , LenX{ lenX }
        , LenY{ 1 }
        , LenZ{ 1 }
        , Dim{ 1 }
        , tasks{}
        , working{ false }
    {};
    Array(uint lenX, uint lenY)
        : data{ std::make_shared<Memory<T>>(GetDevice(), lenX * lenY, 2) }
        , LenX{ lenX }
        , LenY{ lenY }
        , LenZ{ 1 }
        , Dim{ 2 }
        , tasks{}
        , working{ false }
    {};
    Array(uint lenX, uint lenY, uint lenZ)
        : data{ std::make_shared<Memory<T>>(GetDevice(), lenX * lenY * lenZ, 3) }
        , LenX{ lenX }
        , LenY{ lenY }
        , LenZ{ lenZ }
        , Dim{ 3 }
        , tasks{}
        , working{ false }
    {};
    Array(Array const& rhs)
        : data{ rhs.data }
        , LenX{ rhs.LenX }
        , LenY{ rhs.LenY }
        , LenZ{ rhs.LenZ }
        , Dim{ rhs.Dim }
        , tasks{ rhs.tasks }
        , working{ rhs.working }
    {};
    void stop_work() const {
        if (this->working) {
            this->tasks.wait();
            this->working = false;
            this->data->read_from_device();
        }
    }
    ~Array() {
        stop_work();
    };

    Array copy() const {
        Array out;

        this->tasks.wait();
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
        }
        return out;
    };

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

    friend Array operator+(Array const& lhs, Array const& rhs) {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks + rhs.tasks;
            out.working = true;
        }
        Kernel kernel(GetDevice(), out.LenX * out.LenY * out.LenZ, "add", *lhs.data, *rhs.data, *out.data); 
        Event this_event;
        kernel.enqueue_run(1, &out.tasks.get(), &this_event);
        out.tasks.get().push_back(this_event);
        return out;
    };
    Array& operator+=(Array const& rhs) {
        Kernel kernel(GetDevice(), LenX * LenY * LenZ, "add_inplace", *data, *rhs.data);
        Event this_event;
        kernel.enqueue_run(1, &tasks.get(), &this_event);
        tasks.get().push_back(this_event);
        working = true;
        return *this;
    };

    friend Array operator*(Array const& lhs, Array const& rhs) {
        Array out; {
            out.data = std::make_shared<Memory<T>>(GetDevice(), lhs.LenX * lhs.LenY * lhs.LenZ, lhs.Dim);
            out.LenX = lhs.LenX;
            out.LenY = lhs.LenY;
            out.LenZ = lhs.LenZ;
            out.Dim = lhs.Dim;
            out.tasks = lhs.tasks + rhs.tasks;
            out.working = true;
        }
        Kernel kernel(GetDevice(), out.LenX * out.LenY * out.LenZ, "mult", *lhs.data, *rhs.data, *out.data);
        Event this_event;
        kernel.enqueue_run(1, &out.tasks.get(), &this_event);
        out.tasks.get().push_back(this_event);
        return out;
    };
    Array& operator*=(Array const& rhs) {
        Kernel kernel(GetDevice(), LenX * LenY * LenZ, "mult_inplace", *data, *rhs.data);
        Event this_event;
        kernel.enqueue_run(1, &tasks.get(), &this_event);
        tasks.get().push_back(this_event);
        working = true;
        return *this;
    };

};





// TODO: This is an example of a library function
void fnGpuProgramming() {  

    const uint N = 1000000u; // size of vectors
    if (1) {
        Array<float> A(N);
        Array<float> B(N);

        // initialize memory
        for (uint n = 0u; n < N; n++) {
            A[n] = 0.0f;
            B[n] = 2.0f;
        }
        A.data->write_to_device();
        B.data->write_to_device();

        auto C = A + B; // 0 + 2 = 2
        auto D = C * B; // 2 * 2 = 4

        for (uint n = 0u; n < N; n++) {
            EXPECT_EQ(C[n], 2);
        }
        for (uint n = 0u; n < N; n++) {
            EXPECT_EQ(D[n], 4);
        }

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




        Array<float>::enqueue_kernel(N, "add_inplace", *A.data, *B.data);  // 0 + 2 = 2
        Array<float>::enqueue_kernel(N, "mult_inplace", *A.data, *B.data); // 2 * 2 = 4
        Array<float>::enqueue_kernel(N, "add_inplace", *A.data, *B.data);  // 4 + 2 = 6
        Array<float>::enqueue_kernel(N, "mult_inplace", *A.data, *B.data); // 6 * 2 = 12
        Array<float>::enqueue_kernel(N, "add_inplace", *A.data, *B.data);  // 12 + 2 = 14
        Array<float>::enqueue_kernel(N, "mult_inplace", *A.data, *B.data); // 14 * 2 = 28
        Array<float>::enqueue_kernel(N, "add_inplace", *A.data, *B.data);  // 28 + 2 = 30
        Array<float>::enqueue_kernel(N, "mult_inplace", *A.data, *B.data); // 30 * 2 = 60
        Array<float>::complete_kernels();

        A.data->read_from_device();

        for (uint n = 0u; n < N; n++) {
            EXPECT_EQ(A[n], 60);
        }
    }
    














    Device device(select_device_with_most_flops()); // compile OpenCL C code for the fastest available device
    Memory<float> A(device, N, 3); // allocate memory on both host and device
    Memory<float> B(device, N, 3);
    Memory<float> C(device, N, 3);
    
    

    // initialize memory
    for (uint n = 0u; n < N; n++) {
        A[n] = 0.0f; 
        B[n] = 2.0f;
        C[n] = 0.0f;
    }

    A.write_to_device(); // copy data from host memory to device memory
    B.write_to_device();

    if (1) {
        Kernel add_kernel(device, N, "add_inplace", A, B); // kernel that runs on the device
        Kernel mult_kernel(device, N, "mult_inplace", A, B); // kernel that runs on the device

        std::vector<Event> events;

        add_kernel.enqueue_run(1, &events); // 0 + 2 = 2
        mult_kernel.enqueue_run(1, &events); // 2 * 2 = 4

        add_kernel.enqueue_run(1, &events); // 4 + 2 = 6
        mult_kernel.enqueue_run(1, &events); // 6 * 2 = 12

        add_kernel.enqueue_run(1, &events); // 12 + 2 = 14
        mult_kernel.enqueue_run(1, &events); // 14 * 2 = 28

        add_kernel.enqueue_run(1, &events); // 28 + 2 = 30
        mult_kernel.enqueue_run(1, &events); // 30 * 2 = 60

        Event::waitForEvents(events);

        //add_kernel.finish_queue();
        //mult_kernel.finish_queue();
    }

    A.read_from_device(); // copy data from device memory to host memory

    for (uint n = 0u; n < N; n++) {
        EXPECT_EQ(A[n], 60);
    }


    if (1) {
        Kernel add_kernel(device, N, "add_inplace", A, B); // kernel that runs on the device
        Kernel mult_kernel(device, N, "mult_inplace", A, B); // kernel that runs on the device
        
        add_kernel.finish_queue();
        mult_kernel.finish_queue();
    }

    A.read_from_device(); // copy data from device memory to host memory
    
    for (uint n = 0u; n < N; n++) {
        EXPECT_EQ(A[n], 60);
    }









    try {
        // Get list of OpenCL platforms.
        std::vector<cl::Platform> platform;
        cl::Platform::get(&platform);

        if (platform.empty()) {
            std::cerr << "OpenCL platforms not found." << std::endl;
            return;
        }

        // Get first available GPU device which supports double precision.
        cl::Context context;
        std::vector<cl::Device> device;
        for (auto p = platform.begin(); device.empty() && p != platform.end(); p++) {
            std::vector<cl::Device> pldev;

            try {
                p->getDevices(CL_DEVICE_TYPE_GPU, &pldev);

                for (auto d = pldev.begin(); device.empty() && d != pldev.end(); d++) {
                    if (!d->getInfo<CL_DEVICE_AVAILABLE>()) continue;

                    std::string ext = d->getInfo<CL_DEVICE_EXTENSIONS>();
                    print(ext);

                    std::cout << d->getInfo<CL_DEVICE_NAME>() << std::endl;

                    device.push_back(*d);
                    context = cl::Context(device);

                    // Create command queue.
                    cl::CommandQueue queue(context, device[0]);

                    // Compute c = a + b.
                    std::string script =
                        "kernel void add(\n"
                        "       ulong n,\n"
                        "       global const float *a,\n"
                        "       global const float *b,\n"
                        "       global float *c\n"
                        "       )\n"
                        "{\n"
                        "    size_t i = get_global_id(0);\n"
                        "    if (i < n) {\n"
                        "       c[i] = a[i] + b[i];\n"
                        "    }\n"
                        "}\n";

                    // Compile OpenCL program for found device.
                    cl::Program program(context, cl::Program::Sources{ script }, nullptr);

                    try {
                        program.build(device);
                    }
                    catch (...) {
                        std::cerr
                            << "OpenCL compilation error" << std::endl
                            << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device[0])
                            << std::endl;
                        return;
                    }


                    cl::Kernel add(program, "add");

                    // Prepare input data.
                    const size_t N = 1 << 20;
                    std::vector<float> a(N, 1);
                    std::vector<float> b(N, 2);
                    std::vector<float> c(N);

                    // Allocate device buffers and transfer input data to device.
                    cl::Buffer A(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                        a.size() * sizeof(float), a.data());

                    cl::Buffer B(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                        b.size() * sizeof(float), b.data());

                    cl::Buffer C(context, CL_MEM_READ_WRITE,
                        c.size() * sizeof(float));

                    // Set kernel parameters.
                    add.setArg(0, static_cast<cl_ulong>(N));
                    add.setArg(1, A);
                    add.setArg(2, B);
                    add.setArg(3, C);

                    // Launch kernel on the compute device.
                    queue.enqueueNDRangeKernel(add, cl::NullRange, N, cl::NullRange);

                    // Get result back to host.
                    queue.enqueueReadBuffer(C, CL_TRUE, 0, c.size() * sizeof(float), c.data());

                    // Should get '3' here.
                    EXPECT_EQ(c[42], 3);
                }
            }
            catch (...) {
                device.clear();
            }
        }

        if (device.empty()) {
            std::cerr << "GPUs with double precision not found." << std::endl;
            return;
        }

    }
    catch (...) {
        return;
    }



}
