#pragma region "Includes"
#pragma once

#include <arrayfire.h>
#include <math.h>
#include <stdio.h>
#include <af/util.h>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <iostream>
#include <string>
#include <string_view>
#include <regex>
#include <list>
#include <thread>

#include "util.h"
#include "atomic_allocator.h"
#include "atomic_vector.h"
#include "atomic_stack.h"
#include "atomic_queue.h"
#include "atomic_numbers.h"
#include "atomic_maps.h"
#include "stopwatch.h"
#include "strings.h"
#include "atomic_shared_ptr.h"

#include "types.h"



#include "Parallel.h"
#include "shared_ptr.h"


#include "../FiberTasks/Concurrent_Queue.h"


#include <concurrent_vector.h>
#include <concurrent_unordered_map.h>

#include "units.h"
#include "datetime.h"
#include "functions.h"

#include "../arrayfire/include/CL/opencl.hpp"

#include "scripting.h"


#pragma endregion

// functions or classes which leverage the GPU for improved performance. 
namespace GPU {
    // 1- to 3-dimensional matrix accelerated by the GPU, using ArrayFire as the base. Not thread-safe. 
    template <typename T>
    class matrix {
        template<typename F> friend class matrix;

    protected:
        static_assert(std::is_pod_v<T>, "matrix type must be POD value, such as int, float, double, etc.");
        
        static constexpr af::dtype get_type() {
            if constexpr (std::is_same_v<T, float>) {
                return af::dtype::f32;
            }
            else if constexpr (std::is_same_v<T, double>) {
                return af::dtype::f64;
            }
            else if constexpr (std::is_same_v<T, bool>) {
                return af::dtype::b8;
            }
            else if constexpr (std::is_same_v<long, T> || std::is_same_v<int, T>) {
                return af::dtype::s32;
            }
            else if constexpr (std::is_same_v<unsigned long, T> || std::is_same_v<unsigned int, T>) {
                return af::dtype::u32;
            }
            else if constexpr (std::is_same_v<long long, T>) {
                return af::dtype::s64;
            }
            else if constexpr (std::is_same_v<unsigned long long, T>) {
                return af::dtype::u64;
            }
            else if constexpr (std::is_same_v<T, unsigned char>) {
                return af::dtype::u8;
            }
            else if constexpr (std::is_same_v<T, char>) {
                return af::dtype::s8;
            }
            else {
                static_assert("FAILED TO DETERMINE THE TYPE OF THE MATRIX.");
            }

        };
    public:
        static constexpr af::dtype type = get_type();

    protected:
        af::array arr;
    public:
        explicit matrix(af::array&& rhs) 
            : arr(std::move(rhs)) 
        {};

    public:
        matrix() 
            : arr(0, type)
        {};
        matrix(size_t num_elements_dim0)
            : arr(num_elements_dim0, type)
        {};
        matrix(size_t num_elements_dim0, size_t num_elements_dim1)
            : arr(num_elements_dim0, num_elements_dim1, type)
        {};
        matrix(size_t num_elements_dim0, size_t num_elements_dim1, size_t num_elements_dim2)
            : arr(num_elements_dim0, num_elements_dim1, num_elements_dim2, type)
        {};
        matrix(matrix const&) = default;
        matrix(matrix &&) = default;
        matrix& operator=(matrix const&) = default;
        matrix& operator=(matrix&&) = default;
        ~matrix() = default;

        static matrix range(size_t x, size_t y = 1, size_t z = 1, int seq_dim = -1) {
            return matrix(af::range(x,y,z, 1, seq_dim, type));
        };
        static matrix sequence(T start, T end, T step = (T)1) {
            return matrix((af::array)(af::array(af::seq(start, end, step)).as(type)));
        };

        static matrix constant(T val, size_t num_elements_dim0) {
            return matrix(af::constant(val, num_elements_dim0, type));
        };
        static matrix constant(T val, size_t num_elements_dim0, size_t num_elements_dim1) {
            return matrix(af::constant(val, num_elements_dim0, num_elements_dim1, type));
        };
        static matrix constant(T val, size_t num_elements_dim0, size_t num_elements_dim1, size_t num_elements_dim2) {
            return matrix(af::constant(val, num_elements_dim0, num_elements_dim1, num_elements_dim2, type));
        };

        static matrix identity_matrix(size_t num_elements_dim0) {
            return matrix(af::identity(num_elements_dim0, type));
        };
        static matrix identity_matrix(size_t num_elements_dim0, size_t num_elements_dim1) {
            return matrix(af::identity(num_elements_dim0, num_elements_dim1, type));
        };
        static matrix identity_matrix(size_t num_elements_dim0, size_t num_elements_dim1, size_t num_elements_dim2) {
            return matrix(af::identity(num_elements_dim0, num_elements_dim1, num_elements_dim2, type));
        };

        static matrix random_uniform_matrix(size_t num_elements_dim0) {
            return matrix(af::randu(num_elements_dim0, type));
        };
        static matrix random_uniform_matrix(size_t num_elements_dim0, size_t num_elements_dim1) {
            return matrix(af::randu(num_elements_dim0, num_elements_dim1, type));
        };
        static matrix random_uniform_matrix(size_t num_elements_dim0, size_t num_elements_dim1, size_t num_elements_dim2) {
            return matrix(af::randu(num_elements_dim0, num_elements_dim1, num_elements_dim2, type));
        };

        static matrix random_normalized_matrix(size_t num_elements_dim0) {
            return matrix(af::randn(num_elements_dim0, type));
        };
        static matrix random_normalized_matrix(size_t num_elements_dim0, size_t num_elements_dim1) {
            return matrix(af::randn(num_elements_dim0, num_elements_dim1, type));
        };
        static matrix random_normalized_matrix(size_t num_elements_dim0, size_t num_elements_dim1, size_t num_elements_dim2) {
            return matrix(af::randn(num_elements_dim0, num_elements_dim1, num_elements_dim2, type));
        };

        static matrix from_vector(std::vector<T> const& RHS) {
            return matrix(af::array(RHS.size(), &RHS[0]));
        };
        static matrix from_vector(std::vector<T> const& RHS, size_t num_elements_dim0, size_t num_elements_dim1) {
            return matrix(af::array(num_elements_dim0, num_elements_dim1, &RHS[0]));
        };
        static matrix from_vector(std::vector<T> const& RHS, size_t num_elements_dim0, size_t num_elements_dim1, size_t num_elements_dim2) {
            return matrix(af::array(num_elements_dim0, num_elements_dim1, num_elements_dim2, &RHS[0]));
        };

        template <typename F>
        matrix<F> cast() const {
            return matrix<F>(af::array(arr.as(matrix<F>::type)));
        }

        size_t size_x() const {
            return arr.dims(0);
        }
        size_t size_y() const {
            return arr.dims(1);
        }
        size_t size_z() const {
            return arr.dims(2);
        }
        size_t size(unsigned int dimension) const {
            return arr.dims(dimension);
        };
        size_t size() const {
            return arr.elements();
        };

        /// get a copy of the current matrix data.
        std::shared_ptr<T> local() const {
            return std::shared_ptr<T>(arr.host<T>(), [&arr](T* p) {
                af::freeHost(p);
            });
        }
        af::array get_arr() const {
            return arr;
        }

        /// get a copy of the first value in the matrix. 
        T first() const {
            return arr.scalar<T>();
        };

        

        /// sum all the values in the matrix
        T sum() const {
            return af::sum<T>(arr);
        };
        /// avg all the values in the matrix
        T avg() const {
            return sum() / static_cast<T>(size());
        };
        T max() const {
            return af::max<T>(arr);
        };
        T min() const {
            return af::min<T>(arr);
        };

        matrix abs() const {
            return matrix(af::abs(arr));
        };
        matrix sin() const {
            return matrix(af::sin(arr));
        };
        matrix cos() const {
            return matrix(af::cos(arr));
        };
        matrix tan() const {
            return matrix(af::tan(arr));
        };
        matrix asin() const {
            return matrix(af::asin(arr));
        };
        matrix acos() const {
            return matrix(af::acos(arr));
        };
        matrix atan() const {
            return matrix(af::atan(arr));
        };
        matrix log() const {
            return matrix(af::log(arr));
        };
        matrix log2() const {
            return matrix(af::log2(arr));
        };
        matrix log10() const {
            return matrix(af::log10(arr));
        };
        matrix log1p() const {
            return matrix(af::log1p(arr));
        };
        matrix exp() const {
            return matrix(af::exp(arr));
        };
        matrix transpose() const {
            return matrix(arr.T());
        };
        matrix normalize() const {
            auto mn = min();
            auto mx = max();
            return matrix((arr - mn) / (mx - mn));
        };
        matrix flat() const {
            return matrix(af::flat(arr));
        };
        matrix histogram(unsigned int numBins) const {
            return matrix(af::histogram(arr, numBins));
        };
        matrix column(int _col) const {
            return matrix(af::array(arr.col(_col)));
        };
        matrix row(int _row) const {
            return matrix(af::array(arr.row(_row)));
        };        
        matrix inverse() const {
            return matrix(af::inverse(arr));
        };
        matrix matrix_multiplication(matrix const& rhs) const {
            return matrix(af::matmul(arr, rhs.arr));
        };
        matrix pow(double exponent) const {
            return matrix(af::pow(arr, exponent));
        };
        // extract the diagonal from the matrix
        matrix diagonal() const {
            return matrix(af::diag(arr));
        };

        template <typename G>
        matrix<T> convolve(matrix<G> const& filter, af::convMode mode = af::convMode::AF_CONV_DEFAULT, af::convDomain domain = af::convDomain::AF_CONV_AUTO) const {
            return matrix<T>(af::convolve(arr, filter.arr, mode, domain));
        };
        matrix join(int dimension, matrix const& second) const {
            return matrix(af::join(dimension, arr, second.arr));
        };
        matrix join(int dimension, matrix const& second, matrix const& third) const {
            return matrix(af::join(dimension, arr, second.arr, third.arr));
        };
        matrix join(int dimension, matrix const& second, matrix const& third, matrix const& fourth) const {
            return matrix(af::join(dimension, arr, second.arr, third.arr, fourth.arr));
        };
        matrix tile(unsigned int x, unsigned int y = 1, unsigned int z = 1, unsigned int w = 1) const {
            return matrix(af::tile(arr, x, y, z, w));
        }

        friend matrix<bool> operator&&(T lhs, matrix const& rhs) {
            return matrix<bool>(lhs && rhs.arr);
        };
        friend matrix<bool> operator&&(matrix const& lhs, T rhs) {
            return matrix<bool>(lhs.arr && rhs);
        };
        friend matrix<bool> operator&&(matrix const& lhs, matrix const& rhs) {
            return matrix<bool>(lhs.arr && rhs.arr);
        };

        friend matrix<bool> operator||(T lhs, matrix const& rhs) {
            return matrix<bool>(lhs || rhs.arr);
        };
        friend matrix<bool> operator||(matrix const& lhs, T rhs) {
            return matrix(lhs.arr || rhs);
        };
        friend matrix<bool> operator||(matrix const& lhs, matrix const& rhs) {
            return matrix<bool>(lhs.arr || rhs.arr);
        };

        friend matrix<bool> operator==(T lhs, matrix const& rhs) {
            return matrix<bool>(lhs == rhs.arr);
        };
        friend matrix<bool> operator==(matrix const& lhs, T rhs) {
            return matrix<bool>(lhs.arr == rhs);
        };
        friend matrix<bool> operator==(matrix const& lhs, matrix const& rhs) {
            return matrix<bool>(lhs.arr == rhs.arr);
        };
        friend matrix<bool> operator!=(T lhs, matrix const& rhs) {
            return matrix<bool>(lhs != rhs.arr);
        };
        friend matrix<bool> operator!=(matrix const& lhs, T rhs) {
            return matrix<bool>(lhs.arr != rhs);
        };
        friend matrix<bool> operator!=(matrix const& lhs, matrix const& rhs) {
            return matrix<bool>(lhs.arr != rhs.arr);
        };
        friend matrix<bool> operator>(T lhs, matrix const& rhs) {
            return matrix<bool>(lhs > rhs.arr);
        };
        friend matrix<bool> operator>(matrix const& lhs, T rhs) {
            return matrix<bool>(lhs.arr > rhs);
        };
        friend matrix<bool> operator>(matrix const& lhs, matrix const& rhs) {
            return matrix<bool>(lhs.arr > rhs.arr);
        };
        friend matrix<bool> operator>=(T lhs, matrix const& rhs) {
            return matrix<bool>(lhs >= rhs.arr);
        };
        friend matrix<bool> operator>=(matrix const& lhs, T rhs) {
            return matrix<bool>(lhs.arr >= rhs);
        };
        friend matrix<bool> operator>=(matrix const& lhs, matrix const& rhs) {
            return matrix<bool>(lhs.arr >= rhs.arr);
        };
        friend matrix<bool> operator<(T lhs, matrix const& rhs) {
            return matrix<bool>(lhs < rhs.arr);
        };
        friend matrix<bool> operator<(matrix const& lhs, T rhs) {
            return matrix<bool>(lhs.arr < rhs);
        };
        friend matrix<bool> operator<(matrix const& lhs, matrix const& rhs) {
            return matrix<bool>(lhs.arr < rhs.arr);
        };
        friend matrix<bool> operator<=(T lhs, matrix const& rhs) {
            return matrix<bool>(lhs <= rhs.arr);
        };
        friend matrix<bool> operator<=(matrix const& lhs, T rhs) {
            return matrix<bool>(lhs.arr <= rhs);
        };
        friend matrix<bool> operator<=(matrix const& lhs, matrix const& rhs) {
            return matrix<bool>(lhs.arr <= rhs.arr);
        };
        friend matrix operator+(T lhs, matrix const& rhs) {
            return matrix(lhs + rhs.arr);
        };
        friend matrix operator+(matrix const& lhs, T rhs) {
            return matrix(lhs.arr + rhs);
        };
        friend matrix operator+(matrix const& lhs, matrix const& rhs) {
            return matrix(lhs.arr + rhs.arr);
        };
        friend matrix operator-(T lhs, matrix const& rhs) {
            return matrix(lhs - rhs.arr);
        };
        friend matrix operator-(matrix const& lhs, T rhs) {
            return matrix(lhs.arr - rhs);
        };
        friend matrix operator-(matrix const& lhs, matrix const& rhs) {
            return matrix(lhs.arr - rhs.arr);
        };
        friend matrix operator*(T lhs, matrix const& rhs) {
            return matrix(lhs * rhs.arr);
        };
        friend matrix operator*(matrix const& lhs, T rhs) {
            return matrix(lhs.arr * rhs);
        };
        friend matrix operator*(matrix const& lhs, matrix const& rhs) {
            return matrix(lhs.arr * rhs.arr);
        };
        friend matrix operator/(T lhs, matrix const& rhs) {
            return matrix(lhs / rhs.arr);
        };
        friend matrix operator/(matrix const& lhs, T rhs) {
            return matrix(lhs.arr / rhs);
        };
        friend matrix operator/(matrix const& lhs, matrix const& rhs) {
            return matrix(lhs.arr / rhs.arr);
        };
        matrix operator-() const {
            return matrix(arr.operator-());
        };
        template < typename = typename std::enable_if_t< std::is_integral_v<T> > >
        matrix operator!() const {
            return matrix(arr.operator!());
        };
        template < typename = typename std::enable_if_t< std::is_integral_v<T> > >
        matrix operator~() const {
            return matrix(arr.operator~());
        };
        matrix& operator+=(T rhs) {
            arr += rhs;
            return *this;
        };
        matrix& operator-=(T rhs) {
            arr -= rhs;
            return *this;
        };
        matrix& operator*=(T rhs) {
            arr *= rhs;
            return *this;
        };
        matrix& operator/=(T rhs) {
            arr /= rhs;
            return *this;
        };
        matrix& operator+=(matrix const& rhs) {
            arr += rhs.arr;
            return *this;
        };
        matrix& operator-=(matrix const& rhs) {
            arr -= rhs.arr;
            return *this;
        };
        matrix& operator*=(matrix const& rhs) {
            arr *= rhs.arr;
            return *this;
        };
        matrix& operator/=(matrix const& rhs) {
            arr /= rhs.arr;
            return *this;
        };
        template < typename = typename std::enable_if_t< std::is_integral_v<T> > >
        friend matrix operator&(matrix const& lhs, matrix const& rhs) {
            return matrix(lhs.arr & rhs.arr);
        };
        template < typename = typename std::enable_if_t< std::is_integral_v<T> > >
        friend matrix operator|(matrix const& lhs, matrix const& rhs) {
            return matrix(lhs.arr | rhs.arr);
        };
        template < typename = typename std::enable_if_t< std::is_integral_v<T> > >
        friend matrix operator^(matrix const& lhs, matrix const& rhs) {
            return matrix(lhs.arr ^ rhs.arr);
        };
        template < typename = typename std::enable_if_t< std::is_integral_v<T> > >
        friend matrix operator%(matrix const& lhs, matrix const& rhs) {
            return matrix(lhs.arr % rhs.arr);
        };
        template < typename = typename std::enable_if_t< std::is_integral_v<T> > >
        friend matrix operator&(T lhs, matrix const& rhs) {
            return matrix(lhs & rhs.arr);
        };
        template < typename = typename std::enable_if_t< std::is_integral_v<T> > >
        friend matrix operator|(T lhs, matrix const& rhs) {
            return matrix(lhs | rhs.arr);
        };
        template < typename = typename std::enable_if_t< std::is_integral_v<T> > >
        friend matrix operator^(T lhs, matrix const& rhs) {
            return matrix(lhs ^ rhs.arr);
        };
        template < typename = typename std::enable_if_t< std::is_integral_v<T> > >
        friend matrix operator%(T lhs, matrix const& rhs) {
            return matrix(lhs % rhs.arr);
        };
        template < typename = typename std::enable_if_t< std::is_integral_v<T> > >
        friend matrix operator&(matrix const& lhs, T rhs) {
            return matrix(lhs.arr & rhs);
        };
        template < typename = typename std::enable_if_t< std::is_integral_v<T> > >
        friend matrix operator|(matrix const& lhs, T rhs) {
            return matrix(lhs.arr | rhs);
        };
        template < typename = typename std::enable_if_t< std::is_integral_v<T> > >
        friend matrix operator^(matrix const& lhs, T rhs) {
            return matrix(lhs.arr ^ rhs);
        };
        template < typename = typename std::enable_if_t< std::is_integral_v<T> > >
        friend matrix operator%(matrix const& lhs, T rhs) {
            return matrix(lhs.arr % rhs);
        };

        class linear_regression {
        public:
            // solve for the coefficients or weights in a linear regression, using the provided features and measurements. 
            static matrix solve_for_weights(matrix const& measurements, matrix const& features) {
                // return ((X^T * X)^-1) * X^T * Y;
                return (features.transpose().matrix_multiplication(features)).inverse().matrix_multiplication(features.transpose()).matrix_multiplication(measurements);
                // return matrix(af::solve(features.get_arr(), measurements.get_arr()));
            };
            // evaluate for the standard error of the linear regression
            static matrix standard_error(matrix const& measurements, matrix const& features, matrix const& weights) {
                auto prediction = features.matrix_multiplication(weights);
                auto std_err = (((measurements - prediction).pow(2.0).sum() / std::max<double>(1.0, static_cast<double>(features.size_x()) - 2.0)) * (features.transpose().matrix_multiplication(features)).inverse()).pow(0.5);
                return std_err.diagonal();
            };
            // evaluate for the standard error of the linear regression
            static matrix t_statistic(matrix const& weights, matrix const& std_err) {
                return weights / std_err;
            };
        };






        GL::string to_string() const {
            return std::string(af::toString("", arr));
        };
    };
}


// debug test from ArrayFire to demo the use of the GPU for computing and display. 
static int arrayfire_conway_game_of_life() {
    using namespace af;
    try {
        static const int game_w = 2048, game_h = 2048;

        af::info();

        std::cout << "This example demonstrates the Conway's Game of Life "
            "using ArrayFire"
            << std::endl
            << "There are 4 simple rules of Conways's Game of Life"
            << std::endl
            << "1. Any live cell with fewer than two live neighbours "
            "dies, as if caused by under-population."
            << std::endl
            << "2. Any live cell with two or three live neighbours lives "
            "on to the next generation."
            << std::endl
            << "3. Any live cell with more than three live neighbours "
            "dies, as if by overcrowding."
            << std::endl
            << "4. Any dead cell with exactly three live neighbours "
            "becomes a live cell, as if by reproduction."
            << std::endl
            << "Each white block in the visualization represents 1 alive "
            "cell, black space represents dead cells"
            << std::endl
            << std::endl;

        std::cout
            << "The conway_pretty example visualizes all the states in Conway"
            << std::endl
            << "Red   : Cells that have died due to under population"
            << std::endl
            << "Yellow: Cells that continue to live from previous state"
            << std::endl
            << "Green : Cells that are new as a result of reproduction"
            << std::endl
            << "Blue  : Cells that have died due to over population"
            << std::endl
            << std::endl;

        std::cout
            << "This examples is throttled so as to be a better visualization"
            << std::endl;

        af::Window prettyWindow(1024, 1024,
            R"( Conway's Game Of Life - Visualizing States )");
        prettyWindow.setPos(32, 32);

        int frame_count = 0;

        // Initialize the kernel array just once
        GPU::matrix<int> kernel = GPU::matrix<int>::from_vector({ 1, 1, 1, 1, 0, 1, 1, 1, 1 }, 3, 3);
        GPU::matrix<float> state = (GPU::matrix<float>::random_uniform_matrix(game_h, game_w) > 0.4f).cast<float>();
        auto display = state.tile(1, 1, 3, 1);

        while (!prettyWindow.close()) {
            af::timer delay = timer::start();

            prettyWindow.image(display.get_arr());
            frame_count++;

            // Convolve gets neighbors
            auto nHood = state.convolve(kernel);

            // Generate conditions for life
            // state == 1 && nHood < 2 ->> state = 0
            // state == 1 && nHood > 3 ->> state = 0
            // else if state == 1 ->> state = 1
            // state == 0 && nHood == 3 ->> state = 1
            auto C0 = (nHood == 2);
            auto C1 = (nHood == 3);

            auto a0 = (state == 1) && (nHood < 2);  // Die of under population
            auto a1 = (state != 0) && (C0 || C1);   // Continue to live
            auto a2 = (state == 0) && C1;           // Reproduction
            auto a3 = (state == 1) && (nHood > 3);  // Over-population
            
            display = (a0 + a1).join(2, a1 + a2, a3).cast<float>();

            // Update state
            state = state * C0.cast<float>() + C1.cast<float>();

            if (frame_count % 60 == 0)
                std::cout << 1.0 / timer::stop(delay) << " fps\n";
        }
    }
    catch (af::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    }
    return 0;
}
static int arrayfire_shallow_water() {
    using namespace af;
    Window win(1536, 768, "Shallow Water Equations");
    win.grid(2, 2);

    try {
        af::info();
        printf("Simulation of shallow water equations\n");

        // Grid length, number and spacing
        const unsigned Lx = 1600, nx = Lx + 1;
        const unsigned Ly = 1600, ny = Ly + 1;
        const float dx = Lx / (nx - 1);
        const float dy = Ly / (ny - 1);

        auto ZERO = GPU::matrix<float>::constant(0, nx, ny);
        auto
            um = ZERO,
            vm = ZERO;
        float
            io = (float)std::floor(Lx / 6.0f),
            jo = (float)std::floor(Ly / 6.0f),
            k = 15;

        auto x = GPU::matrix<float>::range(nx).tile(1, ny);
        auto y = GPU::matrix<float>::range(1, ny, 1, 1).tile(nx, 1);

        // initial condition
        auto etam =
            0.01f * ((-((x - io) * (x - io) + (y - jo) * (y - jo))) / (k * k)).exp();

        float m_eta = etam.max();
        auto eta = etam;
        float dt = 0.5;

        // conv kernels
        GPU::matrix<float> h_diff_kernel_arr = GPU::matrix<float>::from_vector({ 9.81f * (dt / dx), 0, -9.81f * (dt / dx) });
        GPU::matrix<float> h_lap_kernel_arr = GPU::matrix<float>::from_vector({ 0, 1, 0, 1, -4, 1, 0, 1, 0 }, 3, 3);

        unsigned iter = 0;
        unsigned random_interval = 30;

        while (!win.close()) {
            if (iter > 2000) {
                // Initial condition
                etam = 0.01f * ((-((x - io) * (x - io) + (y - jo) * (y - jo))) / (k * k)).exp();
                m_eta = etam.max();
                eta = etam;
                iter = 0;
            }

            // raindrops
            if (iter % 100 == 0 || iter % 130 == 0 || iter % random_interval == 0) {
                float
                    io = (float)std::floor(rand() % Lx),
                    jo = (float)std::floor(rand() % Ly);
                random_interval = rand() % 200 + 1;
                eta += 0.01f * ((-((x - io) * (x - io) + (y - jo) * (y - jo))) / (k * k)).exp();
            }

            // compute
            auto up = um + eta.convolve(h_diff_kernel_arr).cast<float>();
            auto vp = um + eta.convolve(h_diff_kernel_arr.transpose()).cast<float>();
            auto e = eta.convolve(h_lap_kernel_arr);
            auto etap = 2 * eta - etam + (2 * dt * dt) / (dx * dy) * e;

            etam = eta;
            eta = etap;
            m_eta = etam.max();

            win(0, 0).setColorMap(AF_COLORMAP_BLUE);

            auto histogram = eta.normalize().histogram(15);
            win(0, 1).setAxesLimits(0, static_cast<float>(histogram.size()), 0, histogram.max());
            win(0, 0).image(eta.normalize().get_arr());
            win(0, 1).hist(histogram.get_arr(), 0, 1, "Normalized Pressure Distribution");
            win(1, 0).plot(GPU::matrix<unsigned>::range(up.size(1)).cast<float>().get_arr(), vp.column(1).get_arr(), "Pressure at left boundary");
            win(1, 1).plot(GPU::matrix<float>(eta.cast<float>().column(0)).flat().get_arr(), GPU::matrix<float>(up.column(0)).flat().get_arr(), GPU::matrix<float>(vp.column(0)).flat().get_arr(), "Gradients versus Magnitude at left boundary");  // viz

            win.show();

            iter++;
        }

    }
    catch (af::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    }

    return 0;
}
static int arrayfire_fields() {
    using namespace af;

    const static float MINIMUM = -3.0f;
    const static float MAXIMUM = 3.0f;
    const static float STEP = 0.18f;

    try {
        af::info();
        af::Window myWindow(1024, 1024, "2D Vector Field example: ArrayFire");

        myWindow.grid(2, 2);

        auto dataRange = GPU::matrix<float>::sequence(MINIMUM, MAXIMUM, STEP);
        auto x = dataRange.tile(1, (unsigned int)dataRange.size_x());
        auto y = dataRange.transpose().tile((unsigned int)dataRange.size_x(), 1);

        // x.eval();
        // y.eval();

        float scale = 2.0f;
        while (!myWindow.close()) {
            auto points = x.flat().join(1, y.flat());
            auto saddle = x.flat().join(1, -1.0f * y.flat());
            auto bvals = (scale * ((x * x) + (y * y))).sin();
            auto hbowl = GPU::matrix<float>::constant(1.0f, x.size()).join(1, bvals.flat());
            // hbowl.eval();

            // 2D points
            myWindow(0, 0).vectorField(points.get_arr(), saddle.get_arr(), "Saddle point");
            myWindow(0, 1).vectorField(points.get_arr(), hbowl.get_arr(), "hilly bowl (in a loop with varying amplitude)");

            // 2D coordinates
            myWindow(1, 0).vectorField(
                (x.flat() * 2.0f).get_arr(), 
                y.flat().get_arr(),
                x.flat().get_arr(),
                (-y.flat()).get_arr(),
                "Saddle point");
            myWindow(1, 1).vectorField(
                (x.flat() * 2.0f).get_arr(),
                y.flat().get_arr(),
                GPU::matrix<float>::constant(1.0f, x.size()).get_arr(),
                bvals.flat().get_arr(),
                "hilly bowl (in a loop with varying amplitude)");

            myWindow.show();

            scale -= 0.0010f;
            if (scale < -0.01f) { scale = 2.0f; }
        };

    }
    catch (af::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    }
    return 0; 
};
static int arrayfire_linear_regression() {
    using namespace af;
    try {
        auto TV_Ads = GPU::matrix<double>::from_vector(std::vector<double>{ 
            230.1, 44.5, 17.2, 151.5, 180.8, 8.7, 57.5, 120.2, 8.6, 199.8, 66.1, 214.7, 23.8, 97.5, 204.1, 195.4, 67.8, 281.4, 69.2, 147.3, 218.4, 237.4, 13.2, 228.3, 62.3, 262.9, 142.9, 240.1, 248.8, 70.6, 292.9, 112.9, 97.2, 265.6, 95.7, 290.7, 266.9, 74.7, 43.1, 228.0, 202.5, 177.0, 293.6, 206.9, 25.1, 175.1, 89.7, 239.9, 227.2, 66.9, 199.8, 100.4, 216.4, 182.6, 262.7, 198.9, 7.3, 136.2, 210.8, 210.7, 53.5, 261.3, 239.3, 102.7, 131.1, 69.0, 31.5, 139.3, 237.4, 216.8, 199.1, 109.8, 26.8, 129.4, 213.4, 16.9, 27.5, 120.5, 5.4, 116.0, 76.4, 239.8, 75.3, 68.4, 213.5, 193.2, 76.3, 110.7, 88.3, 109.8, 134.3, 28.6, 217.7, 250.9, 107.4, 163.3, 197.6, 184.9, 289.7, 135.2, 222.4, 296.4, 280.2, 187.9, 238.2, 137.9, 25.0, 90.4, 13.1, 255.4, 225.8, 241.7, 175.7, 209.6, 78.2, 75.1, 139.2, 76.4, 125.7, 19.4, 141.3, 18.8, 224.0, 123.1, 229.5, 87.2, 7.8, 80.2, 220.3, 59.6, .7, 265.2, 8.4, 219.8, 36.9, 48.3, 25.6, 273.7, 43.0, 184.9, 73.4, 193.7, 220.5, 104.6, 96.2, 140.3, 240.1, 243.2, 38.0, 44.7, 280.7, 121.0, 197.6, 171.3, 187.8, 4.1, 93.9, 149.8, 11.7, 131.7, 172.5, 85.7, 188.4, 163.5, 117.2, 234.5, 17.9, 206.8, 215.4, 284.3, 50.0, 164.5, 19.6, 168.4, 222.4, 276.9, 248.4, 170.2, 276.7, 165.6, 156.6, 218.5, 56.2, 287.6, 253.8, 205.0, 139.5, 191.1, 286.0, 18.7, 39.5, 75.5, 17.2, 166.8, 149.7, 38.2, 94.2, 177.0, 283.6, 232.1
        });
        auto Radio_Ads = GPU::matrix<double>::from_vector(std::vector<double>{
            37.8, 39.3, 45.9, 41.3, 10.8, 48.9, 32.8, 19.6, 2.1, 2.6, 5.8, 24.0, 35.1, 7.6, 32.9, 47.7, 36.6, 39.6, 20.5, 23.9, 27.7, 5.1, 15.9, 16.9, 12.6, 3.5, 29.3, 16.7, 27.1, 16.0, 28.3, 17.4, 1.5, 20.0, 1.4, 4.1, 43.8, 49.4, 26.7, 37.7, 22.3, 33.4, 27.7, 8.4, 25.7, 22.5, 9.9, 41.5, 15.8, 11.7, 3.1, 9.6, 41.7, 46.2, 28.8, 49.4, 28.1, 19.2, 49.6, 29.5, 2.0, 42.7, 15.5, 29.6, 42.8, 9.3, 24.6, 14.5, 27.5, 43.9, 30.6, 14.3, 33.0, 5.7, 24.6, 43.7, 1.6, 28.5, 29.9, 7.7, 26.7, 4.1, 20.3, 44.5, 43.0, 18.4, 27.5, 40.6, 25.5, 47.8, 4.9, 1.5, 33.5, 36.5, 14.0, 31.6, 3.5, 21.0, 42.3, 41.7, 4.3, 36.3, 10.1, 17.2, 34.3, 46.4, 11.0, .3, .4, 26.9, 8.2, 38.0, 15.4, 20.6, 46.8, 35.0, 14.3, .8, 36.9, 16.0, 26.8, 21.7, 2.4, 34.6, 32.3, 11.8, 38.9, .0, 49.0, 12.0, 39.6, 2.9, 27.2, 33.5, 38.6, 47.0, 39.0, 28.9, 25.9, 43.9, 17.0, 35.4, 33.2, 5.7, 14.8, 1.9, 7.3, 49.0, 40.3, 25.8, 13.9, 8.4, 23.3, 39.7, 21.1, 11.6, 43.5, 1.3, 36.9, 18.4, 18.1, 35.8, 18.1, 36.8, 14.7, 3.4, 37.6, 5.2, 23.6, 10.6, 11.6, 20.9, 20.1, 7.1, 3.4, 48.9, 30.2, 7.8, 2.3, 10.0, 2.6, 5.4, 5.7, 43.0, 21.3, 45.1, 2.1, 28.7, 13.9, 12.1, 41.1, 10.8, 4.1, 42.0, 35.6, 3.7, 4.9, 9.3, 42.0, 8.6
        });
        auto Newspaper_Ads = GPU::matrix<double>::from_vector(std::vector<double>{
            69.2, 45.1, 69.3, 58.5, 58.4, 75.0, 23.5, 11.6, 1.0, 21.2, 24.2, 4.0, 65.9, 7.2, 46.0, 52.9, 114.0, 55.8, 18.3, 19.1, 53.4, 23.5, 49.6, 26.2, 18.3, 19.5, 12.6, 22.9, 22.9, 40.8, 43.2, 38.6, 30.0, .3, 7.4, 8.5, 5.0, 45.7, 35.1, 32.0, 31.6, 38.7, 1.8, 26.4, 43.3, 31.5, 35.7, 18.5, 49.9, 36.8, 34.6, 3.6, 39.6, 58.7, 15.9, 60.0, 41.4, 16.6, 37.7, 9.3, 21.4, 54.7, 27.3, 8.4, 28.9, .9, 2.2, 10.2, 11.0, 27.2, 38.7, 31.7, 19.3, 31.3, 13.1, 89.4, 20.7, 14.2, 9.4, 23.1, 22.3, 36.9, 32.5, 35.6, 33.8, 65.7, 16.0, 63.2, 73.4, 51.4, 9.3, 33.0, 59.0, 72.3, 10.9, 52.9, 5.9, 22.0, 51.2, 45.9, 49.8, 100.9, 21.4, 17.9, 5.3, 59.0, 29.7, 23.2, 25.6, 5.5, 56.5, 23.2, 2.4, 10.7, 34.5, 52.7, 25.6, 14.8, 79.2, 22.3, 46.2, 50.4, 15.6, 12.4, 74.2, 25.9, 50.6, 9.2, 3.2, 43.1, 8.7, 43.0, 2.1, 45.1, 65.6, 8.5, 9.3, 59.7, 20.5, 1.7, 12.9, 75.6, 37.9, 34.4, 38.9, 9.0, 8.7, 44.3, 11.9, 20.6, 37.0, 48.7, 14.2, 37.7, 9.5, 5.7, 50.5, 24.3, 45.2, 34.6, 30.7, 49.3, 25.6, 7.4, 5.4, 84.8, 21.6, 19.4, 57.6, 6.4, 18.4, 47.4, 17.0, 12.8, 13.1, 41.8, 20.3, 35.2, 23.7, 17.6, 8.3, 27.4, 29.7, 71.8, 30.0, 19.6, 26.6, 18.2, 3.7, 23.4, 5.8, 6.0, 31.6, 3.6, 6.0, 13.8, 8.1, 6.4, 66.2, 8.7
        });
        auto Sales_Revenue = GPU::matrix<double>::from_vector(std::vector<double>{
            22.1, 10.4, 12.0, 16.5, 17.9, 7.2, 11.8, 13.2, 4.8, 15.6, 12.6, 17.4, 9.2, 13.7, 19.0, 22.4, 12.5, 24.4, 11.3, 14.6, 18.0, 17.5, 5.6, 20.5, 9.7, 17.0, 15.0, 20.9, 18.9, 10.5, 21.4, 11.9, 13.2, 17.4, 11.9, 17.8, 25.4, 14.7, 10.1, 21.5, 16.6, 17.1, 20.7, 17.9, 8.5, 16.1, 10.6, 23.2, 19.8, 9.7, 16.4, 10.7, 22.6, 21.2, 20.2, 23.7, 5.5, 13.2, 23.8, 18.4, 8.1, 24.2, 20.7, 14.0, 16.0, 11.3, 11.0, 13.4, 18.9, 22.3, 18.3, 12.4, 8.8, 11.0, 17.0, 8.7, 6.9, 14.2, 5.3, 11.0, 11.8, 17.3, 11.3, 13.6, 21.7, 20.2, 12.0, 16.0, 12.9, 16.7, 14.0, 7.3, 19.4, 22.2, 11.5, 16.9, 16.7, 20.5, 25.4, 17.2, 16.7, 23.8, 19.8, 19.7, 20.7, 15.0, 7.2, 12.0, 5.3, 19.8, 18.4, 21.8, 17.1, 20.9, 14.6, 12.6, 12.2, 9.4, 15.9, 6.6, 15.5, 7.0, 16.6, 15.2, 19.7, 10.6, 6.6, 11.9, 24.7, 9.7, 1.6, 17.7, 5.7, 19.6, 10.8, 11.6, 9.5, 20.8, 9.6, 20.7, 10.9, 19.2, 20.1, 10.4, 12.3, 10.3, 18.2, 25.4, 10.9, 10.1, 16.1, 11.6, 16.6, 16.0, 20.6, 3.2, 15.3, 10.1, 7.3, 12.9, 16.4, 13.3, 19.9, 18.0, 11.9, 16.9, 8.0, 17.2, 17.1, 20.0, 8.4, 17.5, 7.6, 16.7, 16.5, 27.0, 20.2, 16.7, 16.8, 17.6, 15.5, 17.2, 8.7, 26.2, 17.6, 22.6, 10.3, 17.3, 20.9, 6.7, 10.8, 11.9, 5.9, 19.6, 17.3, 7.6, 14.0, 14.8, 25.5, 18.4
        });
        auto Basic = GPU::matrix<double>::constant(1.0, Sales_Revenue.size());

        auto features = Basic.join(1, TV_Ads).join(1, Radio_Ads).join(1, Newspaper_Ads); 

        /* From Excel, ANOVA Linear Regression:
                    Coefficients	Standard Error	t Stat	        P-value	        Lower 95%	    Upper 95%	
        Intercept	4.640017533	    0.309318899	    15.00075667	    2.50772E-34	    4.029977548	    5.250057519	
        230.1	    0.054400553	    0.001380292	    39.41233891	    8.02236E-95	    0.051678334	    0.057122771	
        37.8	    0.106889232	    0.008507645	    12.56390337	    6.55379E-27	    0.09011042	    0.123668044
        69.2	    3.57871E-06	    0.005831777	    0.000613656	    0.999511	    -0.011497876	0.011505033
        */

        auto weights = GPU::matrix<double>::linear_regression::solve_for_weights(
            Sales_Revenue, // measurements to be predicted
            features // appends each vector as the next column in the matrix
        );               
        auto std_err = GPU::matrix<double>::linear_regression::standard_error(
            Sales_Revenue,
            features, 
            weights
        );
        auto t_statistics = GPU::matrix<double>::linear_regression::t_statistic(weights, std_err);


        auto std_dev = std_err * std::sqrt((1.0 / static_cast<double>(Sales_Revenue.size())) + (std::pow(features.matrix_multiplication(weights).avg() - Sales_Revenue.avg(), 2.0) / (features.matrix_multiplication(weights) - Sales_Revenue.avg()).pow(2.0).avg()));


        


        // (Sales_Revenue.avg() - features.matrix_multiplication(weights).avg()) / 


        af_print(weights.get_arr());
        af_print(std_err.get_arr());
        af_print(t_statistics.get_arr());
        af_print(std_dev.get_arr());



        
    }
    catch (af::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;    
};

#pragma region "Definitions"
#define SINGLE_ARG(...) __VA_ARGS__
#define EXPECT_EQ_PRINTF(A,B) [a = (A), b = (B)]()->bool{ \
    if (a == b) { return true; } else { \
        std::string tempA{std::to_string(a)}, tempB{std::to_string(b)}; \
        std::cout << GoodLang::printf("FAILURE AT LINE %i: (%s != %s)\n", (int)__LINE__, tempA.c_str(), tempB.c_str()); \
    return false; } \
}()
#define print(a) std::cout << a << std::endl
#define EXPECT_EQ(a, b) if (a != b){ print(GL::printf("FAILURE AT LINE %i\n", (int)__LINE__)); }
#define EXPECT_NE(a, b) if (a == b){ print(GL::printf("FAILURE AT LINE %i\n", (int)__LINE__)); }
#pragma endregion

#include <stdlib.h>

int main() {
    // arrayfire. This does not currently work without explicitely installing the arrayFire installer (yet).  
    // Some success was found by using OpenCL. 
    if (0) {
        try {
            // Get list of OpenCL platforms.
            std::vector<cl::Platform> platform;
            cl::Platform::get(&platform);

            if (platform.empty()) {
                std::cerr << "OpenCL platforms not found." << std::endl;
                return 1;
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
                            return 1;
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
                return 1;
            }

        }
        catch (...) {
            return 1;
        }



        //print(std::string(_ARRAYFIRELIB));

        //std::string env_new;
        //if (1) {
        //    char* env_result;
        //    EXPECT_EQ(0, ::_dupenv_s(&env_result, nullptr, "PATH"));
        //    env_new = env_result;
        //    if (env_result) free(env_result);
        //    print(env_new);
        //    std::cout << std::endl;
        //}
        //if (env_new.find(_ARRAYFIRELIB) == std::string::npos) {
        //    env_new += ";";
        //    env_new += _ARRAYFIRELIB;

        //    print(env_new);
        //    std::cout << std::endl;

        //    EXPECT_NE(0, SetEnvironmentVariable("PATH", env_new.c_str()));

        //    if (1) {
        //        char* env_result;
        //        EXPECT_EQ(0, ::_dupenv_s(&env_result, nullptr, "PATH"));
        //        env_new = env_result;
        //        if (env_result) free(env_result);
        //        print(env_new);
        //        std::cout << std::endl;
        //    }

        //}
        
        //EXPECT_NE(0, SetEnvironmentVariable("PATH", _ARRAYFIRELIB));
        //EXPECT_NE(0, SetEnvironmentVariable("AF_PATH", _ARRAYFIREDIR));
        //EXPECT_NE(0, SetEnvironmentVariable("AF_PATH_v3", _ARRAYFIREDIR));

        print(af::getDevice());
        print(af::getDeviceCount());
        af::info();

        using namespace GPU;
        print(matrix<float>::from_vector({ 0.0f, 1.0f, 2.0f, 3.0f }).to_string());
        print((matrix<float>::from_vector({ 0.0f, 1.0f, 2.0f, 3.0f }) + 2.0f).to_string());

        print((matrix<float>::from_vector({ 0.0f, 1.0f, 2.0f, 3.0f }) * matrix<float>::from_vector({ 0.0f, 1.0f, 2.0f, 3.0f })).to_string());

        matrix<int> two_by_four_matrix = matrix<int>::from_vector({ 0, 1, 2, 3, 4, 5, 6, 7 });
        print((two_by_four_matrix % 2).to_string());

        //(void)arrayfire_conway_game_of_life();
        //(void)arrayfire_shallow_water();
        //(void)arrayfire_fields();
        (void)arrayfire_linear_regression();
    }

    GL::parallel::For(0, 1000000, [&](size_t i) {});
    GL::parallel::Std_For(0, 1000000, [&](size_t i) {});

    GL::stopwatch sw;
    while (true) {


        // Testing Scopes::Scopes
#if 1
        if (1) {
            GL::scope::impl::RootScope root; // successfully starts a new script root

       // >> TEST SCOPES
#if 1
            GL::parallel::For(0, 1000000, [&](int i) {
                auto scope{ root.make_scope() };
            });

            GL::parallel::For(0, 1000000, [&](int i) {
                auto& scope{ root.make_namespace("std") };
            });

            GL::parallel::For(0, 1000000, [&](int i) {
                auto& scope{ root.make_namespace("std") };
                scope.invalidate_cache();
            });

            // Test recursive update calls. Should only recurse one time until the "call num" saturates. 
            if (1) {
                auto& scope1{ root.make_namespace("std") };
                auto& scope2{ root.make_namespace("UI") };

                scope2.add_using_here(scope1);
                scope1.add_using_here(scope2);

                scope1.invalidate_cache();
                scope2.invalidate_cache();
                root.invalidate_cache();
            }

#if 1
            GL::parallel::For(0, 1000000, [&](int i) {
                switch (i % 3) {
                case 0: {
#if 1
                    auto& scope1{ root.make_namespace("std") };
                    auto& scope2{ scope1.make_namespace("impl") };
                    auto scope3{ scope2.make_scope() };

                    scope3.add_using_here(scope2);
                    scope3.add_using_here(scope1);
                    scope3.add_using_here(root);

                    auto scope5{ scope3.make_scope() };
                    scope5.get_unique_index();
#endif 
                    break;
                }
                case 1: {
#if 1
                    auto& scope1{ root.make_namespace("std") };
                    auto& scope2{ scope1.make_namespace("string") };
                    auto& scope3{ scope2.make_namespace("impl") };
                    auto scope4{ scope3.make_scope() };

                    scope2.emplace_object_here("npos", 100); // slow due to conflict with GoodLang::shared_ptr... 

                    scope4.add_using_here(scope3);
                    scope4.add_using_here(scope2);
                    scope4.add_using_here(scope1);
                    scope4.add_using_here(root);

                    auto scope5{ scope3.make_scope() };
                    auto scope6{ scope4.make_scope() };
                    scope5.get_unique_index();
                    scope6.get_unique_index();

#endif 
                    break;
                }
                case 2: {
#if 1
                    auto& scope1{ root.make_namespace("string") };
                    auto& scope2{ scope1.make_namespace("impl") };
                    auto scope3{ scope2.make_scope() };

                    scope1.emplace_object_here("npos", 200);

                    scope3.add_using_here(scope2);
                    scope3.add_using_here(scope1);
                    scope3.add_using_here(root);

                    auto scope5{ scope3.make_scope() };
                    scope5.get_unique_index();
#endif
                    break;
                }
                }
            });
#endif // << NO LEAK

#if 1
            auto s = GL::string("::std::string::");
            print(s.left_of("d::").c_str());
            print(s.right_of("d::").c_str());
            print(s.left_of("::std").c_str());
            print(s.right_of("::std").c_str());
            print(s.left_of("string::").c_str());
            print(s.right_of("string::").c_str());

            print(s.left_of_last("d::").c_str());
            print(s.right_of_last("d::").c_str());
            print(s.left_of_last("::std").c_str());
            print(s.right_of_last("::std").c_str());
            print(s.left_of_last("string::").c_str());
            print(s.right_of_last("string::").c_str());

            EXPECT_EQ(true, s.ends_with("::"));
            EXPECT_EQ(true, s.begins_with("::"));

            EXPECT_NE(nullptr, root.find_namespace(GL::string("")));
            EXPECT_NE(nullptr, root.find_namespace(GL::string("::")));
            EXPECT_NE(nullptr, root.find_namespace(GL::string("::std::")));
            EXPECT_NE(nullptr, root.find_namespace(GL::string("::std::string::")));
            EXPECT_NE(nullptr, root.find_namespace(GL::string("::std::string::impl::")));
            EXPECT_NE(nullptr, root.find_namespace(GL::string("::string::")));
            EXPECT_NE(nullptr, root.find_namespace(GL::string("::string::impl::")));

            EXPECT_EQ(nullptr, root.find_namespace(GL::string("impl"))); // could not find "impl" from the root, which is (arguably) correct!             
            EXPECT_NE(nullptr, root.find_namespace(GL::string("std")));
            EXPECT_NE(nullptr, root.find_namespace(GL::string("std::string")));
            EXPECT_NE(nullptr, root.find_namespace(GL::string("std::string::impl")));
            EXPECT_NE(nullptr, root.find_namespace(GL::string("string"))->this_m.scope->find_namespace(GL::string("impl")));
            EXPECT_NE(nullptr, root.find_namespace(GL::string("string")));
            EXPECT_NE(nullptr, root.find_namespace(GL::string("string::impl")));

            EXPECT_NE(nullptr, root.find_namespace(GL::string("::std::string::"))->this_m.scope->find_object_here("npos"));
            EXPECT_NE(nullptr, root.find_object("::std::string::npos"));
            EXPECT_EQ(nullptr, root.find_object("npos")); // should not be successfully found.
            EXPECT_NE(nullptr, root.find_object("std::string::npos"));
            //EXPECT_EQ("100", GoodLang::ToString(**root.find_object("std::string::npos")));
            EXPECT_NE(nullptr, root.find_object("::string::npos"));
            //EXPECT_EQ("200", GoodLang::ToString(**root.find_object("::string::npos"))); 
            EXPECT_EQ(nullptr, root.find_object("::npos")); // should not be successfully found.

            EXPECT_EQ(nullptr, root.find_namespace("std")->this_m.scope->find_object("npos"));
            EXPECT_NE(nullptr, root.find_namespace("std")->this_m.scope->find_object("string::npos"));
            EXPECT_EQ(nullptr, root.find_namespace("std")->this_m.scope->find_object("string"));
            EXPECT_EQ(nullptr, root.find_namespace("std")->this_m.scope->find_object("string2::npos")); // this namespace does not exist and will not be found. 
            EXPECT_EQ(nullptr, root.find_object("std::npos")); // should not be successfully found.
            EXPECT_EQ(nullptr, root.find_object("std::string")); // should not be successfully found.
            EXPECT_EQ(nullptr, root.find_object("std::string2::npos")); // should not be successfully found.

            EXPECT_NE(nullptr, root.find_namespace("::string::impl::")->this_m.scope->find_object("npos"));
            EXPECT_NE(nullptr, root.find_namespace("std::string::impl::")->this_m.scope->find_object("npos"));

            //EXPECT_EQ("100", GoodLang::ToString(**root.find_namespace("std::string::impl::")->this_m.scope->find_object("npos")));
            //EXPECT_EQ("200", GoodLang::ToString(**root.find_namespace("::string::impl::")->this_m.scope->find_object("npos")));

            GL::parallel::For(0, 1000000, [&](int i) {
                EXPECT_NE(nullptr, root.find_namespace("std")->this_m.scope->find_object("string::npos"));
                EXPECT_NE(nullptr, root.find_object("std::string::npos"));
            });

            GL::parallel::For(0, 1000000, [&](int i) {
                EXPECT_EQ(nullptr, root.find_object("std::string2::npos")); // should not be successfully found.
            });

            if (1) {
                auto scope1{ root.make_scope() };
                scope1.add_using_here(*scope1.find_namespace("::std::string::")->this_m.scope->GetNamespace());
                EXPECT_NE(nullptr, scope1.find_object("npos")); // should be successfully found now, due to the using statement.
            }

            if (1) {
                root.add_using_here(*root.find_namespace("::std::string::")->this_m.scope->GetNamespace());
                EXPECT_NE(nullptr, root.find_object("npos")); // should be successfully found now, due to the using statement.
            }
            EXPECT_NE(nullptr, root.find_object("::std::string::npos"));
            EXPECT_NE(nullptr, root.find_object("::npos"));
            EXPECT_NE(nullptr, root.find_namespace("UI")->this_m.scope->find_object("npos"));


#endif // << NO LEAK
#endif // << NO LEAK

            // >> TEST FUNCTION CALLS
#if 0
            Functions funcs;
            funcs.emplace("a", utilities::FunctionWrapper(GoodLang::make_callable([](void) -> int { return 0; }), utilities::FunctionWrapper::FunctionState::Normal, {}));
            funcs.emplace("a", utilities::FunctionWrapper(GoodLang::make_callable([](int i) -> int { return i; }), utilities::FunctionWrapper::FunctionState::Normal, {}));
            funcs.emplace("b", utilities::FunctionWrapper(GoodLang::make_callable([](int i) -> int { return i; }), utilities::FunctionWrapper::FunctionState::Normal, {}));
            funcs.emplace("c", utilities::FunctionWrapper(GoodLang::make_callable([](int i, int j) -> int { return i + j; }), utilities::FunctionWrapper::FunctionState::Normal, {}));
            funcs.emplace("d", utilities::FunctionWrapper(GoodLang::make_callable([](int i, int j, int k) -> int { return i + j + k; }), utilities::FunctionWrapper::FunctionState::Normal, { 10, 10, 10 })); // has defaults!

            funcs.emplace("example", utilities::FunctionWrapper(GoodLang::make_callable(
                [](int const& i, int const& j, int const& k) -> std::string { return "3 params"; }
            ), utilities::FunctionWrapper::FunctionState::Normal,
                { 10, 10, 10 }));

            funcs.emplace("example", utilities::FunctionWrapper(GoodLang::make_callable(
                [](int const& i, int const& j) -> std::string { return "2 params"; }
            ), utilities::FunctionWrapper::FunctionState::Normal,
                { 10, 10 }));

            funcs.emplace("example", utilities::FunctionWrapper(GoodLang::make_callable(
                [](int const& i) -> std::string { return "1 param"; }
            ), utilities::FunctionWrapper::FunctionState::Normal,
                { 10 }));

            funcs.emplace("example", utilities::FunctionWrapper(GoodLang::make_callable(
                []() -> std::string { return "no params"; }
            ), utilities::FunctionWrapper::FunctionState::Normal,
                {}));

            funcs.emplace("example2", utilities::FunctionWrapper(GoodLang::make_callable(
                [](int const& i, int const& j, int const& k) -> std::string { return "3 params"; }
            ), utilities::FunctionWrapper::FunctionState::Normal,
                { 10.0, 10, 10.0 }));

            funcs.emplace("example2", utilities::FunctionWrapper(GoodLang::make_callable(
                [](int const& i, int const& j) -> std::string { return "2 params"; }
            ), utilities::FunctionWrapper::FunctionState::Normal,
                { 10.0, 10 }));

            funcs.emplace("example2", utilities::FunctionWrapper(GoodLang::make_callable(
                [](double const& i, double const& j) -> std::string { return "2 param doubles!"; }
            ), utilities::FunctionWrapper::FunctionState::Normal));

            GoodLang::TypeConverter converter;
            converter.AddConverter<bool, int>();
            converter.AddConverter<int, bool>();
            converter.AddConverter<double, int>();
            converter.AddConverter<int, double>();
            converter.AddConverter<float, int>();
            converter.AddConverter<int, float>();
            converter.AddConverter<bool, float>();
            converter.AddConverter<float, bool>();
            converter.AddConverter<double, float>();
            converter.AddConverter<float, double>();
            converter.AddConverter<bool, double>();
            converter.AddConverter<double, bool>();

            // including these conversion checks "fixes" it. IDK why. 
            print(converter.ConversionCost_Fast(GoodLang::user_type_shared_ptr<int>(), GoodLang::user_type_shared_ptr<double>()));
            print(converter.ConversionCost_Fast(GoodLang::user_type_shared_ptr<int>(), GoodLang::user_type_shared_ptr<double>()->MakeConstRef().lock()));
            print(converter.ConversionCost_Fast(GoodLang::user_type_shared_ptr<double>(), GoodLang::user_type_shared_ptr<int>()));
            print(converter.ConversionCost_Fast(GoodLang::user_type_shared_ptr<double>(), GoodLang::user_type_shared_ptr<int>()->MakeConstRef().lock()));





            EXPECT_NE(nullptr, funcs.BuildMatch("a", GoodLang::ParamTypes(), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("b", GoodLang::ParamTypes({ GoodLang::user_type_shared<int>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("c", GoodLang::ParamTypes({ GoodLang::user_type_shared<int>(), GoodLang::user_type_shared<int>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes({ GoodLang::user_type_shared<int>(), GoodLang::user_type_shared<int>(), GoodLang::user_type_shared<int>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("a", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("a", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>() }), converter).function); // test providing more params than needed
            EXPECT_NE(nullptr, funcs.BuildMatch("b", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("c", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>() }), converter).function);

            print(GoodLang::ToString(funcs.Call("a", {}, converter)));
            print(GoodLang::ToString(funcs.Call("b", { 100.0 }, converter)));
            print(GoodLang::ToString(funcs.Call("c", { 200.0, 200.0 }, converter)));
            print(GoodLang::ToString(funcs.Call("d", { 500.0, 50, true }, converter)));
            print(GoodLang::ToString(funcs.Call("a", { 100, 200.0 }, converter)));
            EXPECT_EQ(nullptr, funcs.BuildMatch("b", GoodLang::ParamTypes(), converter).function);
            EXPECT_EQ(nullptr, funcs.BuildMatch("c", GoodLang::ParamTypes(), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes(), converter).function);
            print(GoodLang::ToString(funcs.Call("d", {}, converter)));

            print(GoodLang::ToString(funcs.Call("example", {}, converter)));
            print(GoodLang::ToString(funcs.Call("example", { 10 }, converter)));
            print(GoodLang::ToString(funcs.Call("example", { 10, 10 }, converter)));
            print(GoodLang::ToString(funcs.Call("example", { 10, 10, 10 }, converter)));
            print(GoodLang::ToString(funcs.Call("example", { 10, 10, 10, 10 }, converter)));

            print(GoodLang::ToString(funcs.Call("example", { 10.0 }, converter)));
            print(GoodLang::ToString(funcs.Call("example", { 10.0, 10.0 }, converter)));
            print(GoodLang::ToString(funcs.Call("example", { 10.0, 10.0, 10.0 }, converter)));
            print(GoodLang::ToString(funcs.Call("example", { 10.0, 10.0, 10.0, 10.0 }, converter)));

            print(GoodLang::ToString(funcs.Call("example2", { 10, 10 }, converter)));
            print(GoodLang::ToString(funcs.Call("example2", { 10.0, 10.0 }, converter)));
            print(GoodLang::ToString(funcs.Call("example2", { 10, 10, 10 }, converter)));
            print(GoodLang::ToString(funcs.Call("example2", { 10.0, 10.0, 10.0 }, converter)));
            print(GoodLang::ToString(funcs.Call("example2", { 10.0, 10.0, 10.0, 10.0 }, converter)));
            print(GoodLang::ToString(funcs.Call("example2", {}, converter)));
            print(GoodLang::ToString(funcs.Call("example2", { 10.0, 10 }, converter))); // prefers the double-type since it keeps the first type
            print(GoodLang::ToString(funcs.Call("example2", { 10, 10.0 }, converter))); // prefers the int-type since it keeps the first type

#endif // << NO LEAK

       // TEST SEARCHING FOR SCOPES
#if 1
            GL::scope::impl::Breadcrumb* nearest;

            nearest = nullptr;
            EXPECT_EQ(nullptr, root.find_namespace(GL::string("impl"), nearest)); // does not find it, but returns the root as the nearest location
            EXPECT_NE(nullptr, nearest);
            if (nearest) print(nearest->GetCurrentNamespace().c_str());

            nearest = nullptr;
            EXPECT_NE(nullptr, root.find_namespace(GL::string("std::string::impl"), nearest)); // successfully finds it
            EXPECT_NE(nullptr, nearest);
            if (nearest) print(nearest->GetCurrentNamespace().c_str());

            nearest = nullptr;
            EXPECT_NE(nullptr, root.find_namespace(GL::string("std::impl"), nearest)); // successfully finds it
            EXPECT_NE(nullptr, nearest);
            if (nearest) print(nearest->GetCurrentNamespace().c_str());

            nearest = nullptr;
            EXPECT_NE(nullptr, root.find_namespace(GL::string("string::impl"), nearest)); // successfully finds it
            EXPECT_NE(nullptr, nearest);
            if (nearest) print(nearest->GetCurrentNamespace().c_str());

            nearest = nullptr;
            EXPECT_EQ(nullptr, root.find_namespace(GL::string("string::impl::impl"), nearest)); // does not find it, but does locate the nearest location
            EXPECT_NE(nullptr, nearest);
            if (nearest) print(nearest->GetCurrentNamespace().c_str());

            GL::parallel::For(0, 1000000, [&](int i) {
                auto scope{ root.make_scope() };
            });

            GL::parallel::For(0, 1000000, [&](int i) {
                auto scope{ root.make_scope() };
                scope.emplace_object_here(GL::printf("%i", i), GL::any(i)); // x = 100.0;
            });

            GL::parallel::For(0, 1000000, [&](int i) {
                auto scope{ root.make_scope() };
                scope.emplace_object_here(GL::printf("%i", i), GL::any(i)); // x = 100.0;
                if (auto* p = scope.find_object_here(GL::printf("%i", i))) {}
                else {
                    EXPECT_EQ(true, false);
                }
            });

#endif // << NO LEAK
        }
#endif // << NO LEAK

        if (1) {
            GL::scope::impl::RootScope root;
            auto& std_namespace = root.make_namespace("std");
            auto& std_string_namespace = std_namespace.make_namespace("string"); {
                std_string_namespace.emplace_object_here("npos", std::string::npos);
            }
            auto& std_vector_namespace = std_namespace.make_namespace("vector");
            auto& std_map_namespace = std_namespace.make_namespace("map");
            auto& std_set_namespace = std_namespace.make_namespace("set");
            auto& std_unordered_map_namespace = std_namespace.make_namespace("unordered_map");
            auto& std_unordered_set_namespace = std_namespace.make_namespace("unordered_set");
            
            auto function_scope = root.make_scope();
            function_scope.emplace_object_here("x", GL::any(0.0));
            GL::any* x = function_scope.find_object("x");
            GL::any* npos = function_scope.find_object("std::string::npos");
            if (x && npos) {
                EXPECT_EQ(std::string::npos, (*GL::make_callable("+", [](double a, size_t b) -> size_t { return b; }))({ x->fast(), npos->fast() }).cast<size_t>());
            }
            else {
                EXPECT_EQ(true, false);
            }


        }












        if (1) {
            GL::function_signature sig(
                "sum", 
                GL::type_of<int>(), 
                { { "a", GL::type_of<int const&>() }, { "b", GL::type_of<int const&>() } },
                {}
            );
            EXPECT_EQ(true, sig.can_call_with_cast({ GL::type_of<int>(), GL::type_of<int>() }));
            EXPECT_EQ(true, sig.can_call_with_free_cast({ GL::type_of<int>(), GL::type_of<int>() }));

            EXPECT_EQ(true, sig.can_call_with_cast({ GL::type_of<int const&>(), GL::type_of<int const&>() }));
            EXPECT_EQ(true, sig.can_call_with_free_cast({ GL::type_of<int const&>(), GL::type_of<int const&>() }));

            EXPECT_EQ(false, sig.can_call_with_cast({ GL::type_of<int const&>() }));
            EXPECT_EQ(false, sig.can_call_with_free_cast({ GL::type_of<int const&>() }));
        }
        if (1) {
            GL::function_signature sig(
                "sum", 
                GL::type_of<int>(), 
                { { "a", GL::type_of<int>() }, { "b", GL::type_of<int>() } },
                { GL::any{ 0 }, GL::any{ 0 } }
            );
            EXPECT_EQ(true, sig.can_call_with_cast({ GL::type_of<int>(), GL::type_of<int>() }));
            EXPECT_EQ(true, sig.can_call_with_free_cast({ GL::type_of<int>(), GL::type_of<int>() }));

            EXPECT_EQ(true, sig.can_call_with_cast({ GL::type_of<int const&>(), GL::type_of<int const&>() }));
            EXPECT_EQ(false, sig.can_call_with_free_cast({ GL::type_of<int const&>(), GL::type_of<int const&>() }));
            EXPECT_EQ(true, sig.can_call_with_cast({  }));
            EXPECT_EQ(true, sig.can_call_with_free_cast({  }));
        }
        if (1) {
            GL::details::Explicit_Function_Impl function(std::function([](int i) -> int {
                EXPECT_EQ(i, 100);
                return i + 1;
            }));
            EXPECT_EQ(101, function.operator()({ 100 }).cast<int>());
        }
        if (1) {
            GL::details::Explicit_Function_Impl function([](int i) -> int {
                return i + 1;
            }, { 0 });
            EXPECT_EQ(101, function.operator()({ 100 }).cast<int>());
            EXPECT_EQ(1, function.operator()({}).cast<int>());
        }
        if (1) {
            GL::details::Explicit_Function_Impl function([]() -> double {
                return 1.0;
            });
            EXPECT_EQ(1.0, function.operator()({}).cast<double>());
        }
        if (1) {
            class temp {
            public:                
                int x;
                double y;

                temp() : x{ 0 }, y{ 0 } {};
                temp(int X, double Y) : x{ X }, y{ Y } {};
                temp(temp const&) = default;
                temp(temp &&) = default;
                temp& operator=(temp const&) = default;
                temp& operator=(temp&&) = default;
                ~temp() = default;

                static double FUNC() {
                    return 100.0;
                };
                double SUM() {
                    return x+y;
                };
                double CONST_SUM() const {
                    return x + y;
                };
                int& Increment() {
                    return x;
                };
                const int& Get() const {
                    return x;
                };
            }; 

            if (1) {
                GL::details::Attribute_Access_Impl function(&temp::x);
                EXPECT_EQ(100, function.operator()({ temp(100, 200.0) }).cast<int>());

                auto T_ptr = GL::make_shared<temp>();
                if (1) {
                    T_ptr->x = 100;
                    T_ptr->y = 200.0;
                    EXPECT_EQ(100, function.operator()({ GL::any(GL::shared_ptr<temp>(T_ptr)) }).cast<int>());
                }
                if (1) {
                    T_ptr->x = 500;
                    EXPECT_EQ(500, function.operator()({ GL::any(GL::shared_ptr<temp>(T_ptr)) }).cast<int>());
                }
            }
            if (1) {
                GL::details::Static_Function_Impl function(&temp::FUNC);
                EXPECT_EQ(100.0, function().cast<double>());
            }
            if (1) {
                GL::details::Default_Member_Function_Impl function(&temp::SUM);
                EXPECT_EQ(300.0, function({ temp(100, 200.0) }).cast<double>());
            }
            if (1) {
                GL::details::Const_Member_Function_Impl function(&temp::CONST_SUM);
                EXPECT_EQ(300.0, function({ temp(100, 200.0) }).cast<double>());
            }

            EXPECT_EQ(300.0, GL::make_callable("CONST_SUM", &temp::CONST_SUM)->operator()({temp(100, 200.0)}).cast<double>());
            EXPECT_EQ(300.0, GL::make_callable("SUM", &temp::SUM)->operator()({ temp(100, 200.0) }).cast<double>());
            EXPECT_EQ(100, GL::make_callable("x", &temp::x)->operator()({ temp(100, 200.0) }).cast<int>());
            EXPECT_EQ(100.0, GL::make_callable("FUNC", &temp::FUNC)->operator()().cast<double>());

            EXPECT_EQ(decl_func(&temp::CONST_SUM)->m_signature.name_m, "CONST_SUM");


            decl_func(&temp::CONST_SUM, GL::function_signature::Constant, {}, { "parent" });
            EXPECT_EQ("int&", decl_func(&temp::x, GL::function_signature::Async, {}, {{"parent", GL::type_of<temp&>()}})->operator()({temp(100, 200.0)}).m_casted_type.name());
            EXPECT_EQ("const int&", decl_func(&temp::x, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", GL::type_of<temp const&>() } })->operator()({ GL::any(temp(100, 200.0)) + GL::type::Const }).m_casted_type.name());

            GL::decl_func(&temp::x);
            GL::decl_func(&temp::y, {}, {});


            EXPECT_EQ("int&", GL::decl_func(&temp::Increment)->operator()({ temp(100, 200.0) }).m_casted_type.name());
            EXPECT_EQ("const int&", GL::decl_func(&temp::Get)->operator()({ GL::any(temp(100, 200.0)) + GL::type::Const }).m_casted_type.name());

            if (1) {
                GL::any Temp;
                if (1) {
                    Temp = GL::decl_func(&temp::Get)->operator()({ GL::any(temp(100, 200.0)) + GL::type::Const });
                }
                EXPECT_EQ(100, Temp.cast<int>());
            }

            if (1) {
                auto converter_func = GL::make_callable("int", [](double x) -> int {
                    return static_cast<int>(x);
                }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static);
                auto to_convert = GL::any(100.0).fast();

                EXPECT_EQ(100, (*converter_func)(&to_convert, &to_convert + 1).cast<int>());
            }
            if (1) {
                auto to_convert = GL::any(100.0).fast();
                EXPECT_EQ(100, (*GL::make_converter<double, int>())(&to_convert, &to_convert + 1).cast<int>());
            }
            if (1) {
                auto to_convert = GL::any(GL::foot(100.0f)).fast();
                EXPECT_EQ(100, (*GL::make_converter<GL::foot, int>())(&to_convert, &to_convert + 1).cast<int>());
            }
            if (1) {
                auto to_convert = GL::any(100).fast();
                EXPECT_EQ(GL::foot(100.0f), (*GL::make_converter<int, GL::foot>())(&to_convert, &to_convert + 1).cast<GL::foot>());
            }
            if (1) {
                auto to_convert = GL::any(GL::meter(100.0f)).fast();
                EXPECT_EQ(GL::meter(100.0f), (*GL::make_converter<GL::meter, GL::foot>())(&to_convert, &to_convert + 1).cast<GL::foot>());
            }
            // because value and can constructed from a meter, and the user is requesting as-value, it will construct a new object.
            if (1) {
                auto to_convert = GL::any(GL::meter(100.0f)).fast();
                EXPECT_EQ(GL::meter(100.0f), (*GL::make_converter<GL::meter, GL::value>())(&to_convert, &to_convert + 1).cast<GL::value>());
            }
            // because temp and can constructed from a temp2, and the user is requesting as-value, it will construct a new object.
            if (1) {
                class temp2 final : public temp {};
                auto to_convert = GL::any(temp2()).fast();
                EXPECT_EQ(0, (*GL::make_converter<temp2, temp>())(&to_convert, &to_convert + 1).cast<temp>().x);
            }
            // because foot is not a base of meter, it will construct a new object.
            if (1) {
                auto to_convert = GL::any(GL::meter(100.0f)).fast();
                EXPECT_EQ(GL::meter(100.0f), (*GL::make_converter<GL::meter, GL::foot&>())(&to_convert, &to_convert + 1).cast<GL::foot>());
            }
            // because a value is base of meter AND we are requesting a reference or pointer, this will perform a polymorphic cast.
            if (1) {
                auto to_convert = GL::any(GL::meter(100.0f)).fast();
                EXPECT_EQ(GL::meter(100.0f), (*GL::make_converter<GL::meter, GL::value&>())(&to_convert, &to_convert + 1).cast<GL::value>());
            }
            // because a temp is base of temp2 AND we are requesting a reference or pointer, this will perform a polymorphic cast.
            if (1) {
                class temp2 final : public temp {};
                auto to_convert = GL::any(temp2()).fast();
                EXPECT_EQ(0, (*GL::make_converter<temp2, temp&>())(&to_convert, &to_convert + 1).cast<temp>().x);
            }

            if (1) {
                GL::script_type Type("CustomString");
                auto callable = GL::make_callable("custom_function", [&Type](GL::dynamic_object const& x) {
                    EXPECT_EQ(x.m_type, Type.load());
                    return x.m_type;
                }, 0, {}, { { "Parent", Type.load() } });
                auto temp_obj = GL::make_shared< GL::dynamic_object >(Type.load());
                EXPECT_EQ((*callable)({ temp_obj }).cast<GL::type>(), Type.load());
            }




        }







        // prove that GL::shared_ptr supports custom deleter functions. Note that these are always called on a different thread than the pointer was made on... 
        GL::shared_ptr<int> temp_ptr(new int(100), [](int* p) {
            EXPECT_EQ(*p, 100);
            delete p;
        });

        // check GL::value and GL::datetime
        if (1) {
            GL::value val{ 10.0f };
            val = 10.0f;
            val = 10;
            val = 10.0;
            val = 10ull;
            val = val;

            EXPECT_EQ(10, (int)(float)val);
            EXPECT_EQ(true, val.is_scalar());

            GL::value meter(GL::value::get_si_unit(1, 0, 0, 0, 0, 0).get_impl_unit(1.0, "meter", "m"));
            EXPECT_EQ(false, meter.is_scalar());
            EXPECT_EQ(0, (int)(float)meter);
            EXPECT_EQ(meter.name(), "meter");
            EXPECT_EQ(meter.abbreviation(), "m");

            meter += GL::value(0);
            EXPECT_EQ(false, meter.is_scalar());
            EXPECT_EQ(0, (int)(float)meter);
            meter -= GL::value(0);
            EXPECT_EQ(false, meter.is_scalar());
            EXPECT_EQ(0, (int)(float)meter);
            meter *= GL::value(0);
            EXPECT_EQ(false, meter.is_scalar());
            EXPECT_EQ(0, (int)(float)meter);
            meter /= GL::value(1);
            EXPECT_EQ(false, meter.is_scalar());
            EXPECT_EQ(0, (int)(float)meter);

            GL::value foot(GL::value::get_si_unit(1, 0, 0, 0, 0, 0).get_impl_unit(381.0 / 1250.0, "foot", "ft"));
            EXPECT_EQ(false, foot.is_scalar());

            GL::value inch(GL::value::get_si_unit(1, 0, 0, 0, 0, 0).get_impl_unit((1.0 / 12.0) * (381.0 / 1250.0), "inch", "in"));
            EXPECT_EQ(false, inch.is_scalar());

            GL::value square_meter(GL::value::get_si_unit(2, 0, 0, 0, 0, 0).get_impl_unit(1.0, "square_meter", "sq_m"));
            EXPECT_EQ(false, square_meter.is_scalar());

            GL::value square_foot(GL::value::get_si_unit(2, 0, 0, 0, 0, 0).get_impl_unit(((381.0 / 1250.0) * (381.0 / 1250.0)), "square_foot", "sq_ft"));
            EXPECT_EQ(false, square_foot.is_scalar());

            GL::value cubic_meter(GL::value::get_si_unit(3, 0, 0, 0, 0, 0).get_impl_unit(1.0, "cubic_meter", "cu_m"));
            EXPECT_EQ(false, cubic_meter.is_scalar());

            GL::value scalar;
            EXPECT_EQ(0, (int)(float)scalar);
            EXPECT_EQ(true, scalar.is_scalar());
            EXPECT_EQ(scalar.name(), "scalar");

            GL::value scalar2(GL::value::get_si_unit(0, 0, 0, 0, 0, 0).get_impl_unit(1, "scalar", ""));
            EXPECT_EQ(0, (int)(float)scalar2);
            EXPECT_EQ(true, scalar2.is_scalar());
            EXPECT_EQ(scalar2.name(), "scalar");

            meter = 0.0f;
            meter += 10.0f;
            EXPECT_EQ(10, (int)(float)meter);
            foot += 1.0f;
            EXPECT_EQ(1, (int)(float)foot);
            inch += 12.0f;
            EXPECT_EQ(12, (int)(float)inch);
            foot += inch;
            EXPECT_EQ(2, (int)(float)foot);
            scalar += 100.0f;
            EXPECT_EQ(100, (int)(float)scalar);

            cubic_meter += 1;
            EXPECT_EQ(1, (int)(float)cubic_meter);

            cubic_meter += scalar;
            EXPECT_EQ(101, (int)(float)cubic_meter);

            try { // expected to throw an error, because adding an inch to a cubic meter is nonsense. 
                cubic_meter += inch;
                EXPECT_EQ(true, false);
            }
            catch (...) {}

            auto manual_sq_m = meter * meter;
            EXPECT_EQ(manual_sq_m.abbreviation(), "sq_m");
            EXPECT_EQ(manual_sq_m.name(), "square_meter");
            auto manual_cu_m = manual_sq_m * meter;
            EXPECT_EQ(manual_cu_m.abbreviation(), "cu_m");
            EXPECT_EQ(manual_cu_m.name(), "cubic_meter");
            auto manual_sq_ft = foot * foot;
            EXPECT_EQ(manual_sq_ft.abbreviation(), "sq_ft");
            EXPECT_EQ(manual_sq_ft.name(), "square_foot");
            auto manual_cu_ft = manual_sq_ft * foot;
            EXPECT_EQ(manual_cu_ft.abbreviation(), "cu_ft");
            EXPECT_EQ(manual_cu_ft.name(), "cubic_foot");
            auto manual_sq_in = inch * inch;
            EXPECT_EQ(manual_sq_in.abbreviation(), "sq_in");
            EXPECT_EQ(manual_sq_in.name(), "square_inch");
            auto manual_scalar = manual_cu_ft / manual_cu_m;
            EXPECT_EQ(manual_scalar.abbreviation(), "");
            EXPECT_EQ(manual_scalar.name(), "scalar");
            EXPECT_EQ(GL::foot(100), GL::foot(100));
            EXPECT_EQ(GL::meter(GL::foot(100)), GL::foot(100));
            EXPECT_EQ(GL::millimeter(1000), GL::meter(1));
            EXPECT_EQ(GL::megameter(1), GL::meter(1000000));
            EXPECT_EQ(GL::second(60), GL::minute(1));
            EXPECT_EQ(GL::miles_per_hour(1), (GL::mile(1) / GL::hour(1)));

            if (1) {
                using namespace GL::literals;
                EXPECT_EQ(100_ft, 100_ft);
                EXPECT_EQ(GL::meter(100_ft), 100_ft);
                EXPECT_EQ(1000_mm, 1_m);
                EXPECT_EQ(1_Mm, 1000000_m);
                EXPECT_EQ(60_s, 1_min);
                EXPECT_EQ(1_mph, 1_mi / 1_hr);
                GL::datetime DT1 = GL::datetime(2025, 1, 1, 0, 0, 0);
                GL::datetime DT2 = GL::datetime(2025, 1, 1, 0, 0, 1.05f);
                EXPECT_EQ(DT2 - DT1, 1.05_s);
                EXPECT_EQ(365, (int)(float)(GL::day((DT1 + 365_d) - DT1)));
                EXPECT_EQ(DT1.ToNextDay() - DT1.ToStartOfDay(), GL::day(1));
                EXPECT_EQ(DT1.ToNextHour() - DT1.ToStartOfHour(), GL::hour(1));
                EXPECT_EQ(DT1.ToNextMinute() - DT1.ToStartOfMinute(), GL::minute(1));
            }

            if (1) {
                GL::foot v{ 100 };
                GL::parallel::For(0, 1000000, [&](size_t const& index) {
                    ++v;
                    });
                EXPECT_EQ((int)(float)v, 1000100);
            }
            if (1) {
                GL::foot v{ 0 };
                GL::scalar s{ 0 };
                GL::parallel::For(0, 1000000, [&](size_t const& index) {
                    v *= s;
                    });
                EXPECT_EQ((int)(float)v, 0);
            }
            if (1) {
                GL::foot v = 0;
                GL::parallel::For(0, 1000000, [&](size_t const& index) {
                    for (;;) {
                        GL::foot expected = v;
                        if (v.compare_exchange(expected, expected + 1)) {
                            break;
                        }
                    }
                    });
                EXPECT_EQ((int)(float)v, 1000000);
            }
            if (1) {
                GL::datetime v = GL::datetime(2025, 1, 1, 0, 0, 0);
                GL::parallel::For(0, 1000000, [&](size_t const& index) {
                    v += GL::minute(1);
                    });
                EXPECT_EQ((int)(float)GL::minute(v - GL::datetime(2025, 1, 1, 0, 0, 0)), 1000000);
            }
            if (1) {
                GL::datetime v = GL::datetime(2025, 1, 1, 0, 0, 0);
                GL::parallel::For(0, 1000000, [&](size_t const& index) {
                    for (;;) {
                        GL::datetime expected = v;
                        if (v.compare_exchange(expected, expected.ToNextMinute())) {
                            break;
                        }
                    }
                    });
                EXPECT_EQ((int)(float)GL::minute(v - GL::datetime(2025, 1, 1, 0, 0, 0)), 1000000);
            }


        }

        // check GL::type
        if (1) {
            GL::type ti = GL::type_of<std::string>();
            EXPECT_EQ(false, ti.is_void());
            EXPECT_EQ(true, ti.is_cpp_type());
            EXPECT_EQ(true, ti.is_base());
            EXPECT_EQ(false, ti.is_const());
            EXPECT_EQ(false, ti.is_ref());
            EXPECT_EQ(false, ti.is_temp());
            ti |= GL::type::Const;
            EXPECT_EQ(false, ti.is_void());
            EXPECT_EQ(true, ti.is_cpp_type());
            EXPECT_EQ(true, ti.is_const());
            EXPECT_EQ(false, ti.is_base());
            EXPECT_EQ(false, ti.is_ref());
            EXPECT_EQ(false, ti.is_temp());
        }
        if (1) {
            GL::type ti = GL::type_of<const std::string&>();
            EXPECT_EQ(false, ti.is_void());
            EXPECT_EQ(false, ti.is_base());
            EXPECT_EQ(true, ti.is_cpp_type());
            EXPECT_EQ(true, ti.is_const());
            EXPECT_EQ(true, ti.is_ref());
            EXPECT_EQ(false, ti.is_temp());
        }
        if (1) {
            GL::type ti;
            EXPECT_EQ(true, ti.is_void());
            EXPECT_EQ(true, ti.is_base());
            EXPECT_EQ(true, ti.is_cpp_type());
            EXPECT_EQ(ti.name(), "void");
        }
        if (1) {
            GL::type ti = GL::type_of<void>();
            EXPECT_EQ(true, ti.is_void());
            EXPECT_EQ(true, ti.is_base());
            EXPECT_EQ(true, ti.is_cpp_type());
            EXPECT_EQ(ti.name(), "void");
        }
        if (1) {
            GL::script_type custom_type("string");
            GL::type ti = custom_type;
            EXPECT_EQ(false, ti.is_void());
            EXPECT_EQ(true, ti.is_base());
            EXPECT_EQ(false, ti.is_cpp_type());
            EXPECT_EQ(false, ti.is_const());
            EXPECT_EQ(false, ti.is_ref());
            EXPECT_EQ(false, ti.is_temp());
            EXPECT_EQ(ti.name(), "string");
            ti |= GL::type::Const;
            ti |= GL::type::Reference;
            EXPECT_EQ(false, ti.is_void());
            EXPECT_EQ(false, ti.is_base());
            EXPECT_EQ(false, ti.is_cpp_type());
            EXPECT_EQ(true, ti.is_const());
            EXPECT_EQ(true, ti.is_ref());
            EXPECT_EQ(false, ti.is_temp());
            EXPECT_EQ(ti.name(), "const string&");

            // a second type (with the same name!) being named by a different class should have a different hash, and be recognized as a different type. 
            GL::script_type custom_type2("string");
            GL::type ti2 = custom_type2;
            EXPECT_EQ(false, ti2.is_void());
            EXPECT_EQ(true, ti2.is_base());
            EXPECT_EQ(false, ti2.is_cpp_type());
            EXPECT_EQ(false, ti2.is_const());
            EXPECT_EQ(false, ti2.is_ref());
            EXPECT_EQ(false, ti2.is_temp());
            EXPECT_EQ(ti2.name(), "string");
            ti2 |= GL::type::Const;
            ti2 |= GL::type::Reference;
            EXPECT_EQ(false, ti2.is_void());
            EXPECT_EQ(false, ti2.is_base());
            EXPECT_EQ(false, ti2.is_cpp_type());
            EXPECT_EQ(true, ti2.is_const());
            EXPECT_EQ(true, ti2.is_ref());
            EXPECT_EQ(false, ti2.is_temp());
            EXPECT_EQ(ti2.name(), "const string&");

            EXPECT_EQ(false, (bool)(ti == ti2));
            EXPECT_EQ(true, (bool)(ti != ti2));
        }

        // check GL::any, including casting and multi-threaded overwrites and access. 
        if (1) {
            using namespace GL;
            using namespace GL::type_erasure;
            if (1) {
                shared_data<std::string> instanced(GL::make_shared<std::string>("TEST"));
                // EXPECT_EQ(instanced.m_ptr, "TEST");
                std::string* p = static_cast<std::string*>(instanced.m_data);
                EXPECT_EQ(*p, "TEST");
            }
            if (auto instanced = new shared_data<std::string>(GL::make_shared<std::string>("TEST"))) {
                // EXPECT_EQ(instanced->m_ptr, "TEST");
                std::string* p = static_cast<std::string*>(instanced->m_data);
                EXPECT_EQ(*p, "TEST");
                delete instanced;
            }
            if (auto instanced = GL::shared_ptr<shared_data<std::string>>(new shared_data<std::string>(GL::make_shared<std::string>("TEST")))) {
                // EXPECT_EQ(instanced.get()->m_ptr, "TEST");
                std::string* p = static_cast<std::string*>(instanced.get()->m_data);
                EXPECT_EQ(*p, "TEST");
            }
            if (auto instanced = GL::static_pointer_cast<any_data>(GL::shared_ptr<shared_data<std::string>>(new shared_data<std::string>(GL::make_shared<std::string>("TEST"))))) {
                std::string* p = static_cast<std::string*>(instanced.get()->m_data);
                EXPECT_EQ(*p, "TEST");
            }
            if (1) {
                GL::atomic_shared_ptr< any_data > atomic{ GL::static_pointer_cast<any_data>(GL::shared_ptr<shared_data<std::string>>(new shared_data<std::string>(GL::make_shared<std::string>("TEST")))) };
                if (auto instanced = atomic.load()) {
                    std::string* p = static_cast<std::string*>(instanced.get()->m_data);
                    EXPECT_EQ(*p, "TEST");
                }
                if (auto instanced = atomic.load_fast()) {
                    std::string* p = static_cast<std::string*>(instanced.get()->m_data);
                    EXPECT_EQ(*p, "TEST");
                }
            }

            if (1) {
                GL::any wrap; {
                    wrap = std::string("TEST");

                    EXPECT_EQ(true, wrap.can_cast(GL::type_of<std::string>()));
                    EXPECT_EQ(true, wrap.can_cast(GL::type_of<std::string const&>()));
                    EXPECT_EQ(true, wrap.can_free_cast(GL::type_of<std::string>()));
                    EXPECT_EQ(true, wrap.can_free_cast(GL::type_of<std::string const&>()));

                    wrap += GL::type::Const | GL::type::Reference;

                    EXPECT_EQ(true, wrap.can_cast(GL::type_of<std::string const&>()));
                    EXPECT_EQ(true, wrap.can_cast(GL::type_of<std::string>()));
                    EXPECT_EQ(false, wrap.can_free_cast(GL::type_of<std::string>())); // cannot free-cast from const& to && because it requires a constructor. 
                    EXPECT_EQ(true, wrap.can_free_cast(GL::type_of<std::string const&>()));

                    EXPECT_EQ(wrap.cast<std::string>(), "TEST");
                    if (auto p = wrap.cast<GL::shared_ptr<std::string>>()) {
                        EXPECT_EQ(*p, "TEST");
                    }
                    EXPECT_EQ(wrap.cast<std::string const&>(), "TEST");
                }
            }

            if (1) {
                if (1) {
                    any wrap;
                    GL::parallel::For(0, 1000000, [&](size_t const& index) {
                        wrap = std::to_string(index);
                        });
                }
                if (1) {
                    any wrap = GL::string("TEST");
                    GL::parallel::For(0, 1000000, [&](size_t const& index) {
                        auto& ptr = wrap.cast<GL::string>();
                        EXPECT_EQ(ptr, GL::string("TEST"));
                        });
                }
                if (1) {
                    any wrap = GL::string("TEST");
                    GL::parallel::For(0, 1000000, [&](size_t const& index) {
                        auto ptr = wrap.cast<GL::shared_ptr<GL::string>>();
                        EXPECT_EQ(*ptr, GL::string("TEST"));
                        });
                }
                if (1) {
                    any wrap{ GL::string("TEST") };
                    GL::parallel::For(0, 1000000, [&](size_t const& index) {
                        wrap = GL::string("TEST");
                        auto cmp = GL::string("TEST");
                        if (auto ptr = wrap.cast<GL::shared_ptr<GL::string>>()) {
                            EXPECT_EQ(*ptr, GL::string("TEST"));
                        }
                        else {
                            EXPECT_EQ(false, true);
                        }
                        });
                }
                if (1) {
                    GL::var wrap(GL::make_shared<any>(GL::string("TEST")));
                    GL::parallel::For(0, 1000000, [&](size_t const& index) {
                        wrap = GL::var(GL::make_shared<any>(GL::string("TEST")));
                        if (auto ptr = wrap.p_data.load_fast()) {
                            if (auto ptr2 = ptr->cast<GL::shared_ptr<GL::string>>()) {
                                EXPECT_EQ(*ptr2, GL::string("TEST"));
                            }
                        }
                        });
                }
                if (1) {
                    any temp = 100;
                    any temp2 = temp.m_casted_type.instance_by_copy(temp);
                    temp2.cast<int>() += 100;
                    EXPECT_EQ(100, temp.cast<int&>());
                    EXPECT_EQ(200, temp2.cast<int&>());
                    EXPECT_EQ(100, GL::type_of<int>().instance_by_value(100.0f).cast<int>());
                }
            }
        }

#if 0
        if (1) {
            std::atomic<long long> L{ 0 };
            GL::parallel::task(0, 1000000, [&](size_t i) {
                ++L;
                })->and_then([&]() {
                    EXPECT_EQ(1000000, L.load());
                    });
                EXPECT_EQ(1000000, L.load());
        }
        if (1) {
            std::atomic<long long> L{ 0 };
            GL::parallel::task([&]() {
                L += 1;
                return L.load();
            })->and_then([&](GL::job_base& parent) {
                auto V = parent.result.cast<long long>();
                EXPECT_EQ(1, L.load());
                EXPECT_EQ(1, V);
            });
            EXPECT_EQ(1, L.load());
        }

        if (1) {
            auto ptr = GL::parallel::task([]() {
                print("I was Async 2");
                });
        }
        if (1) {
            GL::parallel::task([]() {
                print("I was Async 3");
                });
        }
        if (1) {
            GL::parallel::task([]() {
                ::Sleep(1000);
                print("I was Async 1");
                })->and_then([]() {
                    print("I was Async 2");
                    });
        }
        if (1) {
            auto job1 = GL::parallel::task([]() {
                ::Sleep(1000);
                print("I was Async 1");
                });
            auto job2 = job1->and_then([]() {
                print("I was Async 2");
                });
        }

        if (1) {
            auto job1 = GL::parallel::task([]() {
                ::Sleep(1000);
                print("I was Async 1");
                });
            auto job2 = job1->and_then([]() {
                print("I was Async 2");
                });
            job2 = nullptr;
            job1 = nullptr;
        }

        if (1) {
            GL::parallel::task([]() {
                ::Sleep(1000);
                print("I was Async 1");
                })->and_then([]() {
                    print("I was Async 2");
                    });
        }
        if (1) {
            auto job = GL::parallel::task([]() {
                print("1");
                ::Sleep(1000);
                print("2");
                return 10;
                });
            job->and_then([]() {
                print("3");
                })->and_then([]() {
                    ::Sleep(1000);
                    print("4");
                    })->and_then([]() {
                        print("5");
                        })->and_then([]() {
                            print("6");
                            });
                        job->wait(); // waits for only this job, and does not wait for its children. 
                        print(job->result.cast<int>());
        }
        if (1) {
            auto job = GL::parallel::task([]() {
                print("1");
                ::Sleep(1000);
                print("2");
                return 10;
                });
            job->and_then([]() {
                print("3");
                })->and_then([]() {
                    ::Sleep(1000);
                    print("4");
                    })->and_then([]() {
                        print("5");
                        })->and_then([]() {
                            print("6");
                            });
                        job->wait(); // waits for only this job, and does not wait for its children. 
                        print(job->result.cast<int>());
        }
        if (auto timer = sw.debug_timer("Inline Test")) {
            size_t out = 0;
            std::vector<size_t> jobs;
            {
                jobs.resize(1000000, 0);
                for (size_t i = 0; i < 1000000; ++i) {
                    EXPECT_EQ(1000000, jobs.size());
                    auto start = GL::clock::ns();
                    while ((GL::clock::ns() - start) < 1000) {}
                    ++jobs[i];
                }
                out = std::accumulate(jobs.begin(), jobs.end(), 0);
            }
            EXPECT_EQ(1000000, out);
        }
        if (auto timer = sw.debug_timer("Parallel Jobs Test 1")) {
            size_t out = 0; {
                auto job1 = GL::parallel::task([&]() {
                    std::vector<size_t> jobs;
                    jobs.resize(1000000, 0);
                    return jobs;
                    });
                job1->and_then(0, 1000000, [&](size_t i, GL::job_base& parent) {
                    auto& jobs = parent.result.cast< std::vector<size_t> >();
                    EXPECT_EQ(1000000, jobs.size());
                    auto start = GL::clock::ns();
                    while ((GL::clock::ns() - start) < 1000) {}
                    ++jobs[i];
                })->and_then([job1, &out]() {
                    std::vector<size_t>& jobs = job1->result.cast();
                    EXPECT_EQ(1000000, jobs.size());
                    out = std::accumulate(jobs.begin(), jobs.end(), 0);
                    EXPECT_EQ(1000000, out);
                    print("success?");
                    });
                job1->wait();
            }
            EXPECT_EQ(1000000, out);
        }
        if (auto timer = sw.debug_timer("Parallel Jobs Test 2")) {
            size_t out = 0;
            {
                std::vector<size_t> jobs;
                jobs.resize(1000000, 0);
                GL::parallel::task(0, 1000000, [&](size_t i) {
                    EXPECT_EQ(1000000, jobs.size());
                    auto start = GL::clock::ns();
                    while ((GL::clock::ns() - start) < 1000) {}
                    ++jobs[i];
                    });
                out = std::accumulate(jobs.begin(), jobs.end(), 0);

            }
            EXPECT_EQ(1000000, out);
        }
        if (auto timer = sw.debug_timer("Parallel Jobs Test 3")) {
            size_t out = 0;
            {
                std::vector<size_t> jobs;
                jobs.resize(1000000, 0);
                GL::parallel::For(0, 1000000, [&](size_t i) {
                    EXPECT_EQ(1000000, jobs.size());
                    auto start = GL::clock::ns();
                    while ((GL::clock::ns() - start) < 1000) {}
                    ++jobs[i];
                    });
                out = std::accumulate(jobs.begin(), jobs.end(), 0);
            }
            EXPECT_EQ(1000000, out);
        }


#else
        (void)GL::parallel::async([]() {
            return std::string("TEST 0");
        }).wait();

        (void)GL::parallel::async([](int i) {
            EXPECT_EQ(i, 10);
            return std::string("TEST 1");
        }, 10).wait();

        (void)GL::parallel::async([](int& i, int j) {
            EXPECT_EQ(i, 10);
            EXPECT_EQ(j, 10);
            return std::string("TEST 2");
        }, 10, 10).wait();

        (void)GL::parallel::async([](int& i, int* j, double k) {
            EXPECT_EQ(i, 10);
            EXPECT_EQ(*j, 10);
            EXPECT_EQ((int)k, 10);
            return std::string("TEST 3");
        }, 10, 10, 10.0).wait();

        // will complete immediately since it has to wait on destruction
        GL::parallel::async([](GL::string& i, float& j, double& k, int& L) {
            (void)(i + " World -> " + GL::printf("%f %f %i", j, k, L));
        }, GL::string("Hello"), 1.0f, 2.0, 3);

        EXPECT_EQ(true, GL::type_of<GL::foot>().is_derived_from(GL::type_of<GL::value>()));
        EXPECT_EQ(true, GL::type_of<GL::millinewton>().is_derived_from(GL::type_of<GL::value>()));
        EXPECT_EQ(true, GL::type_of<GL::value>().is_base_of(GL::type_of<GL::decigallon>()));

        if (1) {
            using namespace GL::literals;
            
            GL::parallel::For(0, 1000000, [](size_t i) {});
            if (auto timer = sw.debug_timer("No Sin()")) {
                GL::parallel::For(0, 1000000, [](size_t i) {
                    (void)GL::degree(static_cast<float>(i));
                });
            }
            if (auto timer = sw.debug_timer("Sin()")) {
                GL::parallel::For(0, 1000000, [](size_t i) {
                    (void)GL::degree(static_cast<float>(i)).sin();
                });
            }
            if (auto timer = sw.debug_timer("SinFast()")) {
                GL::parallel::For(0, 1000000, [](size_t i) {
                    (void)GL::degree(static_cast<float>(i)).sin_fast();
                });
            }






            EXPECT_EQ(3_kg * 10_mps / 5_s, 6_N);

            auto t_rest = (
                (-2.40_mps_sq + GL::value((2.40_mps_sq).pow(2) - 4.0 * 0.5 * (0.3_mps_sq / 1_s) * -12.0_mps).sqrt()) / (2.0 * 0.5 * (0.3_mps_sq / 1_s))
            ).max(
                (-2.40_mps_sq - GL::value((2.40_mps_sq).pow(2) - 4.0 * 0.5 * (0.3_mps_sq / 1_s) * -12.0_mps).sqrt()) / (2.0 * 0.5 * (0.3_mps_sq / 1_s))
            );
            EXPECT_EQ(t_rest, 4_s);
            EXPECT_EQ(GL::constants::pi(), 180_deg);
            EXPECT_EQ((0_deg).sin(), 0.0f);
            print((0_deg).sin_fast());
            EXPECT_EQ((0_deg).cos(), 1.0f);
            EXPECT_EQ((90_deg).sin(), 1.0f);  
            print((90_deg).sin_fast());
            EXPECT_EQ((90_deg).cos(), 0.0f);
            EXPECT_EQ((180_deg).cos(), -1.0f);
            EXPECT_EQ((180_deg).sin(), 0.0f);
            print((180_deg).sin_fast());
            EXPECT_EQ((270_deg).sin(), -1.0f);
            print((270_deg).sin_fast());
            EXPECT_EQ((270_deg).cos(), 0.0f);
            EXPECT_EQ((GL::constants::pi() - 37_deg).sin(), (37_deg).sin().abs()); // trig identity
            print((GL::constants::pi() - 37_deg).sin());
            print((GL::constants::pi() - 37_deg).sin_fast());
            print((37_deg).sin_fast());

            print((137_deg).sin());
            print((137_deg).sin_fast());

            print((237_deg).sin());
            print((237_deg).sin_fast());

            print((337_deg).sin());
            print((337_deg).sin_fast());

            print((-337_deg).sin());
            print((-337_deg).sin_fast());

            EXPECT_EQ(GL::celsius(0.0f), 0_degC);
            EXPECT_EQ(0_degC, 32_degF);
            EXPECT_EQ(32_degF, 0_degC);
            EXPECT_EQ(41_degF, 5_degC);
            EXPECT_EQ(5_degC, 41_degF);
            EXPECT_EQ(GL::foot(56_ft).wrap(0_mm, GL::meter(5_ft)), 1_ft);

            auto T_0 = GL::fahrenheit(10);
            auto T_melting = GL::celsius(0);
            auto mass_ice = 4_kg;
            auto specific_heat_ice = (2100_J / 1_kg) / 1_degC;
            auto specific_heat_water = (4186_J / 1_kg) / 1_degC;
            auto latent_heat_of_fusion_of_water = 333000.0_J / 1_kg;

            GL::joule heat_to_raise_temp_of_ice = mass_ice * specific_heat_ice * (T_melting - T_0);
            print(heat_to_raise_temp_of_ice);

            GL::joule heat_to_melt_ice = mass_ice * latent_heat_of_fusion_of_water;
            print(heat_to_melt_ice);
            
            GL::joule total_heat = heat_to_raise_temp_of_ice + heat_to_melt_ice;
            print(total_heat);

            auto time = total_heat / 500_W;
            print(time);

            auto heat_added = 500_W * 1200_s;
            GL::celsius final_water_temp = 0_degC + (heat_added / (mass_ice * specific_heat_water));
            print(final_water_temp);
            print(GL::fahrenheit(final_water_temp));

            

            print(GL::value(-1.0f).asin());
            print(GL::value(1.0f).asin());
            print(GL::value(-2.0f).asin());
            print(GL::value(2.0f).asin());
            print(GL::value(-0.999f).asin());
            print(GL::value(0.999f).asin());

            print(GL::value(-1.0f).acos());
            print(GL::value(1.0f).acos());
            print(GL::value(-2.0f).acos());
            print(GL::value(2.0f).acos());
            print(GL::value(-0.999f).acos());
            print(GL::value(0.999f).acos());





        }

        // can automatically cast from a foot to float...
        EXPECT_EQ(100, (int)GL::type_of<float>().instance_by_value(GL::foot(100.0f)).cast<float>());
        // can automatically cast from a foot to double...
        EXPECT_EQ(100, (int)GL::type_of<double>().instance_by_value(GL::foot(100.0f)).cast<double>());
        // can automatically cast from a foot to a meter...
        EXPECT_EQ(100, (int)(float)GL::type_of<GL::meter>().instance_by_value(GL::foot(GL::meter(100.0f))).cast<GL::meter>());










        try {
            GL::parallel::task([&]() {
                return GL::foot(100) + GL::gallon(1); // will throw
            })->wait(); // calling wait gives the opportunity to catch the exception.
            EXPECT_EQ(true, false);
        }
        catch (std::exception&) {} // exception from the async job will ultimately be caught here

        GL::parallel::task([&]() {
            return GL::foot(100) + GL::gallon(1); // will throw
        }); // the task may dispatch on construction, but will wait till complete on destruction. Rethrowing exceptions during destruction is a recipe for death of a program, and so the exception is free'd and nothing is done with it. Basically silent failure.

        if (1) {
            std::atomic<long long> L{ 0 };
            GL::parallel::task([&]() {
                L += 1;
                return L.load();
            })->and_then([&](GL::job_base& parent) {
                auto V = parent.result.cast<long long>();
                EXPECT_EQ(1, L.load());
                EXPECT_EQ(1, V);
            });
            EXPECT_EQ(1, L.load());
        }
        if (1) {
            std::atomic<long long> L{ 0 };
            GL::parallel::task([&]() {
                L += 1;
                return L.load();
            })->and_then([&](long long V) {
                EXPECT_EQ(1, L.load());
                EXPECT_EQ(1, V);
            });
            EXPECT_EQ(1, L.load());
        }
        if (1) {
            std::atomic<long long> L{ 0 };
            GL::parallel::task([&]() {
                L += 1;
                return L.load();
            })->and_then([&](GL::job_base& parent, long long V) {
                EXPECT_EQ(1, parent.result.cast<long long>());
                EXPECT_EQ(1, L.load());
                EXPECT_EQ(1, V);
            });
            EXPECT_EQ(1, L.load());
        }
        if (1) {
            std::atomic<long long> L{ 0 };
            GL::parallel::task([&]() {
                L += 100;
            })->and_then(0, 1000000, [&](size_t i) {
                L += 1;
            })->and_then([&]() {
                L -= 100;
            });
            EXPECT_EQ(L.load(), 1000000);
        }

        if (1) {
            std::atomic<long long> L{ 0 };
            GL::parallel::task([&]() {
                L += 100;
            })->and_then(0, 1000000, [&](size_t i, GL::job_base& parent) {
                L += 1;
            })->and_then([&]() {
                L -= 100;
            });
            EXPECT_EQ(L.load(), 1000000);
        }

        if (1) {
            std::atomic<long long> L{ 0 };
            GL::parallel::task([&]() {
                L += 100;
                return 1ull;
            })->and_then(0, 1000000, [&](size_t i, GL::job_base& parent, unsigned long long V) {
                L += V;
            })->and_then([&]() {
                L -= 100;
            });
            EXPECT_EQ(L.load(), 1000000);
        }

        if (1) {
            std::atomic<long long> L{ 0 };
            std::shared_ptr<GL::job_base> job; {
                auto job1 = GL::parallel::task([&]() {
                    L += 100;
                    return 1ull;
                }); // this job is dispatched
                auto job2 = job1->and_then(0, 1000000, [&](size_t i, unsigned long long V, GL::job_base& parent) {
                    L += V;
                });
                job = std::dynamic_pointer_cast<GL::job_base>(job2->and_then([&]() {
                    L -= 100;
                }));
            }
            job->wait();
            EXPECT_EQ(L.load(), 1000000);
        }

        if (1) {
            std::atomic<long long> L{ 0 };
            GL::parallel::task([&]() {
                L += 100;
                return 1ull;
            })->and_then(0, 1000000, [&](size_t i, unsigned long long V, GL::job_base& parent) {
                L += V;
            })->and_then([&]() {
                L -= 100;
            });
            EXPECT_EQ(L.load(), 1000000);
        }

        if (1) {
            auto job = GL::parallel::task([]() {
                print("1");
                ::Sleep(10);
                print("2");
                return 10;
            });
            job->and_then([]() {
                print("3");
            })->and_then([]() {
                ::Sleep(10);
                print("4");
            })->and_then([]() {
                print("5");
            })->and_then([]() {
                print("6");
            });
            job->wait();
            print(job->result.cast<int>());
        }
        if (1) {
            auto job1 = GL::parallel::task([]() {
                print("1");
                ::Sleep(10);
                print("2");
                return 10;
            });
            auto job2 = job1->and_then([]() {
                print("3");
            })->and_then([]() {
                ::Sleep(10);
                print("4");
            })->and_then([]() {
                print("5");
            })->and_then([]() {
                print("6");
            });            
            job1->wait();
            print(job1->result.cast<int>());
        }
        if (auto timer = sw.debug_timer("Inline Test")) {
            size_t out = 0;
            std::vector<size_t> jobs;
            {
                jobs.resize(10000000, 0);
                for (size_t i = 0; i < 10000000; ++i) {
                    EXPECT_EQ(10000000, jobs.size());
                    ++jobs[i];
                }
                out = std::accumulate(jobs.begin(), jobs.end(), 0ull);
            }
            EXPECT_EQ(10000000, out);
        }
        if (auto timer = sw.debug_timer("Parallel Jobs Test 1")) {
            size_t out = 0; {
                GL::parallel::task([&]() {
                    std::vector<size_t> jobs;
                    jobs.resize(10000000, 0);
                    return jobs;
                })->and_then(0, 10000000, [](size_t i, GL::job_base& parent) {
                    auto& jobs = parent.result.cast<std::vector<size_t>>();
                    EXPECT_EQ(10000000, jobs.size());
                    ++jobs[i];
                })->and_then([&out](GL::job_base& parent) {
                    if (auto* p = parent.parent_ptr()) {
                        auto& jobs = p->result.cast<std::vector<size_t>>();
                        EXPECT_EQ(10000000, jobs.size());
                        out = std::accumulate(jobs.begin(), jobs.end(), 0ull);
                        EXPECT_EQ(10000000, out);
                    }
                });
            }
            EXPECT_EQ(10000000, out);
        }
        if (auto timer = sw.debug_timer("Parallel Jobs Test 2")) {
            size_t out = 0;
            {
                std::vector<size_t> jobs;
                jobs.resize(10000000, 0);
                GL::parallel::task(0, 10000000, [&](size_t i) {
                    EXPECT_EQ(10000000, jobs.size());
                    ++jobs[i];
                });
                out = std::accumulate(jobs.begin(), jobs.end(), 0ull);
            }
            EXPECT_EQ(10000000, out);
        }
        if (auto timer = sw.debug_timer("Parallel Jobs Test 3")) {
            size_t out = 0;
            {
                std::vector<size_t> jobs;
                jobs.resize(10000000, 0);
                GL::parallel::For(0, 10000000, [&](size_t i) {
                    EXPECT_EQ(10000000, jobs.size());
                    ++jobs[i];
                });
                out = std::accumulate(jobs.begin(), jobs.end(), 0ull);
            }
            EXPECT_EQ(10000000, out);
        }
#endif





#if 1
        for (size_t repeats = 10; repeats <= 1000000; repeats *= 10) {
            print(repeats);

            GL::atomic_double result = GL::parallel::Dispatch(repeats, GL::atomic_double{ 0 }, [](size_t pos, GL::atomic_double& D) {
                ++D;
            });
            EXPECT_EQ((size_t)result.load(), repeats);

            if (1) {
                std::vector<std::string> calcs(repeats, "");
                if (auto timer = sw.debug_timer("parallel::std single-threaded calculations")) {                    
                    GL::parallel::Std_For(0ull, repeats, [&](size_t const& index) {
                        calcs[index] = std::to_string(index);
                    });
                };
                if (auto timer = sw.debug_timer("parallel::manual single-threaded calculations")) {
                    GL::parallel::For(0ull, repeats, [&](size_t const& index) {
                        calcs[index] = std::to_string(index);
                    });
                };
                if (auto timer = sw.debug_timer("single-threaded single-threaded calculations")) {
                    size_t index = 0ull;
                    for (; index < repeats; ) {
                        calcs[index] = std::to_string(index);
                        ++index;
                    };
                };
            }

            //if (auto timer = sw.debug_timer("parallel::std alloc")) {
            //    GL::atomic_shared_ptr<size_t> ptr; 
            //    GL::parallel::Std_For<size_t>(0, repeats, [&](size_t i) {
            //        ptr.store(GL::shared_ptr<size_t>(new size_t(i)));
            //        ptr = nullptr;
            //    });
            //}
            if (auto timer = sw.debug_timer("parallel::manual alloc")) {
                GL::atomic_shared_ptr<size_t> ptr;
                GL::parallel::For<size_t>(0, repeats, [&](size_t i) {
                    ptr.store(GL::shared_ptr<size_t>(new size_t(i)));
                    ptr = nullptr;
                });
            }
            //if (auto timer = sw.debug_timer("parallel::std increment")) {
            //    std::atomic<size_t> D{ 0 };
            //    GL::parallel::Std_For<size_t>(0, repeats, [&](size_t i) {
            //        ++D;
            //    });
            //}
            if (auto timer = sw.debug_timer("parallel::manual increment")) {
                std::atomic<size_t> D{ 0 };
                GL::parallel::For<size_t>(0, repeats, [&](size_t i) {
                    ++D;
                });
            }
            //if (auto timer = sw.debug_timer("parallel::std map")) {
            //    concurrency::concurrent_unordered_map<size_t, size_t> map;
            //    GL::parallel::Std_For<size_t>(0, repeats, [&](size_t i) {
            //        map[i] = i;
            //    });
            //}
            if (auto timer = sw.debug_timer("parallel::manual map")) {
                concurrency::concurrent_unordered_map<size_t, size_t> map;
                GL::parallel::For<size_t>(0, repeats, [&](size_t i) {
                    map[i] = i;
                });
            }

            //if (auto timer = sw.debug_timer("parallel::std ForEach")) {
            //    std::vector<size_t*> vec(1000000, nullptr);
            //    GL::parallel::Std_ForEach(vec, [](size_t*& p) {
            //        p = reinterpret_cast<size_t*>(100);
            //    });
            //}
            if (1) {
                std::vector<size_t*> vec(repeats, nullptr);
                if (auto timer = sw.debug_timer("parallel::manual ForEach")) {
                    GL::parallel::For_Each(vec, [](size_t*& p) {
                        p = reinterpret_cast<size_t*>(100);
                    });
                }
            }

        }
#endif










#if 0
        if (auto timer = sw.debug_timer(GL::string("queue"))) {
            GL::atomic_queue<size_t> queue;
            size_t L;
            queue.push(0);
            queue.push(1);
            queue.push(2);
            EXPECT_EQ(true, queue.try_pop(L));
            EXPECT_EQ(L, 0);
            EXPECT_EQ(true, queue.try_pop(L));
            EXPECT_EQ(L, 1);
            EXPECT_EQ(true, queue.try_pop(L));
            EXPECT_EQ(L, 2);
        }
        if (auto timer = sw.debug_timer("atomic_stack")) {
            GL::atomic_stack<size_t> queue;
            size_t L;
            queue.push(0);
            queue.push(1);
            queue.push(2);
            EXPECT_EQ(true, queue.try_pop(L));
            EXPECT_EQ(L, 2);
            EXPECT_EQ(true, queue.try_pop(L));
            EXPECT_EQ(L, 1);
            EXPECT_EQ(true, queue.try_pop(L));
            EXPECT_EQ(L, 0);
        }
        if (auto timer = sw.debug_timer("thread_object")) {
            if (1) {
                GL::thread_object<int> thread_local_object(100);
                GL::parallel::For(0, 1000000, [&](int i) {
                    EXPECT_EQ(100, *thread_local_object);
                });
                GL::parallel::For(0, 1000000, [&](int i) {
                    *thread_local_object = static_cast<int>(GL::util::get_thread_id()); // thread-local and therefore thread-safe to change its value
                });
            }
            if (1) {
                GL::thread_object<std::string> thread_local_object("TEST");
                GL::parallel::For(0, 1000000, [&thread_local_object](int) {
                    EXPECT_EQ("TEST", *thread_local_object);
                });
                GL::parallel::For(0, 1000000, [&](int i) {
                    *thread_local_object = "\"" + std::to_string(GL::util::get_thread_id()) + "\""; // thread-local and therefore thread-safe to change its value
                });
            }
            if (1) {
                GL::thread_object<GL::string> thread_local_object("TEST");
                GL::parallel::For(0, 1000000, [&thread_local_object](int) {
                    EXPECT_EQ("TEST", *thread_local_object);
                });
                GL::parallel::For(0, 1000000, [&](int i) {
                    *thread_local_object = "\"" + std::to_string(GL::util::get_thread_id()) + "\""; // thread-local and therefore thread-safe to change its value
                });
            }
        };
        if (auto timer = sw.debug_timer("atomic_allocator")) {
            GL::atomic_allocator<std::string, 1024> alloc;      
            if (1) {
                GL::parallel::For(0, 1000000, [&](int) {
                    alloc.Free(alloc.Alloc());
                });
            }
            if (1) {
                std::vector< std::string* > ptrs(1000000, nullptr);
                GL::parallel::For(0, 1000000, [&](int i) {
                    ptrs[i] = alloc.Alloc();
                });
                GL::parallel::For(0, 1000000, [&](int i) {
                    alloc.Free(ptrs[i]);
                });
            }
            if (1) {
                GL::atomic_parallel_stack<std::string*> to_delete;
                GL::parallel::For(0, 1000000, [&](int) {
                    to_delete.push(alloc.Alloc());
                    std::string* p{ nullptr };
                    if (to_delete.try_pop(p)) {
                        alloc.Free(p);
                    }
                });
            }
        }
        if (auto timer = sw.debug_timer("atomic_parallel_allocator ST")) {
            GL::atomic_parallel_allocator<std::string, 1024> alloc;
            if (1) {
                for (int i = 0; i < 1000000; ++i){
                    alloc.Free(alloc.Alloc());
                };
            }
            if (1) {
                std::vector< std::string* > ptrs(1000000, nullptr);
                for (int i = 0; i < 1000000; ++i) {
                    ptrs[i] = alloc.Alloc();
                };
                for (int i = 0; i < 1000000; ++i) {
                    alloc.Free(ptrs[i]);
                };
            }
            if (1) {
                GL::atomic_parallel_stack<std::string*> to_delete;
                for (int i = 0; i < 1000000; ++i) {
                    to_delete.push(alloc.Alloc());
                    std::string* p{ nullptr };
                    if (to_delete.try_pop(p)) {
                        alloc.Free(p);
                    }
                };
            }
        }
        if (auto timer = sw.debug_timer("atomic_parallel_allocator MT")) {
            GL::atomic_parallel_allocator<std::string, 1024> alloc;
            if (1) {
                GL::parallel::For(0, 1000000, [&](int) {
                    alloc.Free(alloc.Alloc());
                });
            }
            if (1) {
                std::vector< std::string* > ptrs(1000000, nullptr);
                GL::parallel::For(0, 1000000, [&](int i) {
                    ptrs[i] = alloc.Alloc();
                });
                GL::parallel::For(0, 1000000, [&](int i) {
                    alloc.Free(ptrs[i]);
                });
            }
            if (1) {
                GL::atomic_parallel_stack<std::string*> to_delete;
                GL::parallel::For(0, 1000000, [&](int) {
                    to_delete.push(alloc.Alloc());
                    std::string* p{ nullptr };
                    if (to_delete.try_pop(p)) {
                        alloc.Free(p);
                    }
                });
            }
        }       
        if (auto timer = sw.debug_timer("atomic_epoch_allocator")) {
            GL::atomic_epoch_allocator<std::string> alloc;
            if (1) {
                GL::parallel::For(0, 1000000, [&](int) {
                    auto Protected = alloc.ProtectCurrentEpoch();
                    alloc.Free(alloc.Alloc());
                });
            }
            if (1) {
                std::vector< std::string* > ptrs(1000000, nullptr);
                GL::parallel::For(0, 1000000, [&](int i) {
                    ptrs[i] = alloc.Alloc();
                });
                GL::parallel::For(0, 1000000, [&](int i) {
                    auto Protected = alloc.ProtectCurrentEpoch();
                    alloc.Free(ptrs[i]);
                });
            }
            if (1) {
                GL::atomic_parallel_stack<std::string*> to_delete;
                GL::parallel::For(0, 1000000, [&](int) {
                    auto Protected = alloc.ProtectCurrentEpoch();
                    to_delete.push(alloc.Alloc());
                    std::string* p{ nullptr };
                    if (to_delete.try_pop(p)) {
                        alloc.Free(p);

                        p->push_back('t');
                        p->push_back('e');
                        p->push_back('s');
                        p->push_back('t');
                        (void)p->c_str();
                    }
                });
            }
        }
        if (auto timer = sw.debug_timer("GL::atomic_map 1")) {
            GL::atomic_map<size_t, GL::atomic_double> map;
            
            GL::parallel::For(0, 1000000, [&](int i) {
                if (i % 2 == 0) {
                    map[(size_t)GL::util::rand(0, 10)] = i;
                }
                else {
                    (void)map.erase((size_t)GL::util::rand(0, 10));
                }
            });
        }
        if (auto timer = sw.debug_timer("GL::atomic_map 2")) {
            GL::atomic_map<size_t, GL::atomic_double> map;
            GL::parallel::For(0, 1000000, [&](int i) {
                (void)map.erase(i % 10);
                map[i % 10] = i;
            });
        }
        if (auto timer = sw.debug_timer("atomic_stack<size_t>")) {
            GL::atomic_stack<size_t> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push(i);
                EXPECT_EQ(true, queue.try_pop(i));
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push(i);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                EXPECT_EQ(true, queue.try_pop(i));
            });
        }
        if (auto timer = sw.debug_timer("atomic_parallel_stack<size_t>")) {
            GL::atomic_parallel_stack<size_t> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push(i);
                EXPECT_EQ(true, queue.try_pop(i));
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push(i);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                EXPECT_EQ(true, queue.try_pop(i));
            });
        }
        if (auto timer = sw.debug_timer("atomic_queue<size_t>")) {
            GL::atomic_queue<size_t> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push(i);
                EXPECT_EQ(true, queue.try_pop(i));
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push(i);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                EXPECT_EQ(true, queue.try_pop(i));
            });
        }
        if (auto timer = sw.debug_timer("atomic_parallel_queue<size_t>")) {
            GL::atomic_parallel_queue<size_t> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push(i);
                EXPECT_EQ(true, queue.try_pop(i));
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push(i);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                EXPECT_EQ(true, queue.try_pop(i));
            });
        }
        if (auto timer = sw.debug_timer("atomic_parallel_queue<short>")) {
            GL::atomic_parallel_queue<short> queue;

            GL::parallel::For(0, 1000000, [&](short i) {
                queue.push(i);
                EXPECT_EQ(true, queue.try_pop(i));
            });
            GL::parallel::For(0, 1000000, [&](short i) {
                queue.push(i);
            });
            GL::parallel::For(0, 1000000, [&](short i) {
                EXPECT_EQ(true, queue.try_pop(i));
            });
        }
        if (auto timer = sw.debug_timer("atomic_parallel_stack<GL::string>")) {
            GL::atomic_parallel_stack<GL::string> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                GL::string str = std::to_string(i);
                queue.push(str);
                EXPECT_EQ(true, queue.try_pop(str));
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                GL::string str = std::to_string(i);
                queue.push(str);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                GL::string str;
                EXPECT_EQ(true, queue.try_pop(str));
            });
        }
        if (auto timer = sw.debug_timer("atomic_parallel_queue<GL::string>")) {
            GL::atomic_parallel_queue<GL::string> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                GL::string str = std::to_string(i);
                queue.push(str);
                EXPECT_EQ(true, queue.try_pop(str));
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                GL::string str = std::to_string(i);
                queue.push(str);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                GL::string str;
                EXPECT_EQ(true, queue.try_pop(str));
            });
        }
        if (auto timer = sw.debug_timer("atomic_priority_queue<std::string>")) {         
            GL::atomic_priority_queue < std::string > queue; 
            std::string str;

            queue.push("1");
            queue.push("2");
            queue.push("3");

            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "1"); 

            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "2"); 

            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "3");

            queue.push("banana");
            queue.push("cucumber");
            queue.push("apple");
            queue.push("Banana");
            queue.push("Cucumber");
            queue.push("Apple");

            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "Apple"); 
            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "Banana"); 
            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "Cucumber"); 
            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "apple"); 
            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "banana"); 
            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "cucumber");

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                std::string str = std::to_string(i);
                queue.push(str);
                EXPECT_EQ(true, queue.try_pop(str));
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                std::string str = std::to_string(i);
                queue.push(str);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                std::string str;
                EXPECT_EQ(true, queue.try_pop(str));
            });
        }
        if (auto timer = sw.debug_timer("atomic_parallel_priority_queue<std::string>")) {
            GL::atomic_parallel_priority_queue < std::string > queue;
            std::string str;

            queue.push("1");
            queue.push("2");
            queue.push("3");

            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "1");

            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "2");

            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "3");

            queue.push("banana");
            queue.push("cucumber");
            queue.push("apple");
            queue.push("Banana");
            queue.push("Cucumber");
            queue.push("Apple");

            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "Apple");
            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "Banana");
            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "Cucumber");
            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "apple");
            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "banana");
            EXPECT_EQ(true, queue.try_pop(str));
            EXPECT_EQ(str, "cucumber");

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                std::string str = std::to_string(i);
                queue.push(str);
                EXPECT_EQ(true, queue.try_pop(str));
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                std::string str = std::to_string(i);
                queue.push(str);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                std::string str;
                EXPECT_EQ(true, queue.try_pop(str));
            });
        }
        if (auto timer = sw.debug_timer("concurrency::concurrent_vector<size_t>")) {
            concurrency::concurrent_vector<size_t> queue;
            queue.grow_to_at_least(1000000);
        }
        if (auto timer = sw.debug_timer("GL::atomic_vector<size_t>")) {
            GL::atomic_vector<size_t> queue;
            queue.grow_to_at_least(1000000);
        }
        if (auto timer = sw.debug_timer("concurrency::concurrent_vector<size_t>")) {
            concurrency::concurrent_vector<size_t> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push_back(i);
            });
        }
        if (auto timer = sw.debug_timer("GL::atomic_vector<size_t>")) {
            GL::atomic_vector<size_t> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push_back(i);
            });
        }
        if (auto timer = sw.debug_timer("concurrency::concurrent_vector<size_t>")) {
            concurrency::concurrent_vector<size_t> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push_back(i);
                });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                ++queue[i];
                });
        }
        if (auto timer = sw.debug_timer("GL::atomic_vector<size_t>")) {
            GL::atomic_vector<size_t> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push_back(i);
                });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                ++queue[i];
                });
        }
        if (auto timer = sw.debug_timer("GL::atomic_map<size_t, size_t>")) {
            GL::atomic_map<size_t, size_t> map;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                map[i] += i;
            });
        }
        if (auto timer = sw.debug_timer("GL::atomic_hash_map<size_t, size_t>")) {
            GL::atomic_hash_map<size_t, size_t> map;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                map[i] += i;
            });
        }
        if (auto timer = sw.debug_timer("concurrency::concurrent_unordered_map<size_t, size_t>")) {
            concurrency::concurrent_unordered_map<size_t, size_t> map;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                map[i] += i;
            });
        }
        if (auto timer = sw.debug_timer("GL::atomic_map<size_t, size_t> w/ erasure")) {
            GL::atomic_map<size_t, size_t> map;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                map[i] += i;
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                auto& loc = map[i];
                map.erase(i);
                ++loc;
            });
        }
        if (auto timer = sw.debug_timer("GL::atomic_hash_map<size_t, size_t> w/ erasure")) {
            GL::atomic_hash_map<size_t, size_t> map;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                map[i] += i;
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                auto& loc = map[i];
                map.erase(i);
                ++loc;
            });
        }
#endif
#if 0
        if (auto timer = sw.debug_timer("GL::atomic_double")) {
            GL::atomic_double d;
            EXPECT_EQ(0, d);
            EXPECT_EQ(sizeof(GL::atomic_double), sizeof(double));

            d = 0;
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                ++d;
            });
            EXPECT_EQ(1000000, d);

            d = 0;
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                d += 2;
            });
            EXPECT_EQ(1000000, d / 2);
        }
        if (auto timer = sw.debug_timer("GL::atomic_float")) {
            GL::atomic_float d;
            EXPECT_EQ(0, d);
            EXPECT_EQ(sizeof(GL::atomic_float), sizeof(float));

            d = 0;
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                ++d;
            });
            EXPECT_EQ(1000000, d);

            d = 0;
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                d += 2;
            });
            EXPECT_EQ(1000000, d / 2);
        }
#endif
#if 1
#if 0
        if (auto timer = sw.debug_timer("GL::impl::atomic_ptr")) {
            GL::impl::atomic_ptr<long long> ptr{ reinterpret_cast<long long*>(100), 0 };

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                EXPECT_EQ(reinterpret_cast<long long*>(100), ptr.load().first);
                EXPECT_EQ(0, ptr.load().second);
            });

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                auto prev_val = ptr.exchange(reinterpret_cast<long long*>(i), 0);
            });

            std::atomic<char> flag{ 0 };
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                // will only succeed in the exchange if the 'flag' matches our flag value. 
                auto prev_val = ptr.exchange_if(reinterpret_cast<long long*>(i), flag++);
                if (std::get<0>(prev_val)) {
                    // print(GL::printf("%i: %i\n", reinterpret_cast<int>(prev_val.first), static_cast<int>(prev_val.second)));
                }
            });
        }
        if (auto timer = sw.debug_timer("GL::impl::atomic_ptr")) {

            GL::impl::atomic_ptr<long long> ptr{ reinterpret_cast<long long*>(100), 1 };

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                if (i == 10101) {
                    // kills the pointer, sets flag to zero, returns previous value
                    ptr.exchange(reinterpret_cast<long long*>(i), 0); // sets the pointer to 0
                }
                else {
                    // keeps the pointer at 1 (if it was still at 1), otherwise returns empty. 
                    auto prev_val = ptr.exchange_if(reinterpret_cast<long long*>(i), 1);
                    if (std::get<2>(prev_val)) {
                        print(GL::printf("%i: %i\n", reinterpret_cast<int>(std::get<0>(prev_val)), static_cast<int>(std::get<1>(prev_val))));
                    }
                }
            });

        }
#endif

        if (auto timer = sw.debug_timer("increment as individuals")) {
            GL::thread_object<size_t> counter{ 0 };
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                ++*counter;
            });
        }
        if (auto timer = sw.debug_timer("increment as atomic")) {
            std::atomic<size_t> counter{ 0 };
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                ++counter;
            });
        }

        // under low contention, the GL::atomic_shared_ptr using fast_shared_ptr is ~40% faster than a locked shared_ptr, even keeping pace with accessing a shared pointer without copying it. 
        // under moderate contention, this is still true, up to about 50 reads per value change
        // under extremely heavy contention (around 10 reads for every value change), the GL::atomic_shared_ptr is significantly bloated and results in significant slow-downs.
        for (double ratio = 1000000.0; ratio >= 1.0; ratio /= 10) {
            print(ratio);

            if (auto timer = sw.debug_timer("std::shared_ptr<std::string> with std shared lock")) {
                std::shared_mutex mut;
                std::shared_ptr<std::string> ptr{ new std::string(std::to_string(1)) };
                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                    std::shared_ptr<std::string> ptr2;
                    if (i % (int)ratio == 0) {
                        mut.lock();
                        ptr = std::shared_ptr<std::string>(new std::string(std::to_string(i + 1)));
                        mut.unlock();
                    }

                    mut.lock_shared();
                    ptr2 = ptr;
                    mut.unlock_shared();

                    if (ptr2) {
                        EXPECT_EQ((ptr2->length() > 0), true);
                    }
                });
            }
            if (auto timer = sw.debug_timer("std::shared_ptr<std::string> access with std shared lock")) {
                std::shared_mutex mut;
                std::shared_ptr<std::string> ptr{ new std::string(std::to_string(1)) };
                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                    if (i % (int)ratio == 0) {
                        mut.lock();
                        ptr = std::shared_ptr<std::string>(new std::string(std::to_string(i + 1)));
                        mut.unlock();
                    }

                    mut.lock_shared();
                    EXPECT_EQ((ptr->length() > 0), true);
                    mut.unlock_shared();
                });
            }
            if (auto timer = sw.debug_timer("GL::atomic_shared_ptr<std::string> slow test")) {
                GL::atomic_shared_ptr<std::string> ptr{ new std::string(std::to_string(1)) };                
                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                    if (i % (int)ratio == 0) {
                        ptr.store(GL::shared_ptr<std::string>(new std::string(std::to_string(i + 1))));
                    }
                    if (auto ptr2 = ptr.load()) {
                        EXPECT_EQ((ptr2->length() > 0), true);
                    }
                });
            }
            if (auto timer = sw.debug_timer("GL::atomic_shared_ptr<std::string> fast test")) {
                GL::atomic_shared_ptr<std::string> ptr{ new std::string(std::to_string(1)) };         
                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                    if (i % (int)ratio == 0) {
                        ptr.store(GL::shared_ptr<std::string>(new std::string(std::to_string(i + 1))));
                    }
                    if (auto ptr2 = ptr.load_fast()) {
                        EXPECT_EQ((ptr2->length() > 0), true);
                    }
                });
            }
        }

        if (auto timer = sw.debug_timer("GL::atomic_shared_ptr<void>")) {
            auto ptr = GL::static_pointer_cast<void>(GL::atomic_shared_ptr<std::string>(new std::string("test")));
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                if (i % 50 == 0) {
                    ptr.store(GL::shared_ptr<std::string>(new std::string(std::to_string(i + 1))));
                }
                if (auto ptr2 = GL::static_pointer_cast<std::string>(ptr.load())) {
                    EXPECT_EQ((ptr2->length() > 0), true);
                }
            });
        }
        if (auto timer = sw.debug_timer("std::shared_ptr<void>")) {
            std::shared_mutex mut;
            std::shared_ptr<void> ptr{ std::shared_ptr<std::string>(new std::string("test")) };
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                if (i % 50 == 0) {
                    mut.lock();
                    ptr = std::shared_ptr<std::string>(new std::string(std::to_string(i + 1)));
                    mut.unlock();
                }
                mut.lock_shared();                
                auto ptr2 = std::static_pointer_cast<std::string>(ptr);
                mut.unlock_shared();
                if (ptr2) {
                    EXPECT_EQ((ptr2->length() > 0), true);
                }
            });
        }

        auto void_type = GL::type_of<void>();
        auto int_type = GL::type_of<int>();
        EXPECT_EQ(int_type.name(), "int");
        auto double_type = GL::type_of<double>();
        EXPECT_EQ(double_type.name(), "double");
        auto float_type = GL::type_of<float>();
        EXPECT_EQ(float_type.name(), "float");
        auto str_type = GL::type_of<GL::string>();
        EXPECT_EQ(str_type.name(), "class GL::string");

#endif
#if 0
        if (auto timer = sw.debug_timer("atomic_wait")) {
            std::atomic<long> lock{ 0 };
            std::thread temp_thread([&]() {
                ::Sleep(1100);
                lock.store(1);
                GL::atomic_notify_one(&lock);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                GL::stopwatch sw2;
                sw2.reset();
                GL::atomic_wait(&lock, 0l);
                EXPECT_EQ(true, (sw.stop() > 1));
            });
            temp_thread.join();
            

            //std::atomic<size_t> prog{ 0 };
            //GL::_Locked_pointer<long> ptr{ reinterpret_cast<long*>(1ull) };
            //GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
            //    size_t p = reinterpret_cast<size_t>(ptr._Lock_and_load());
            //    EXPECT_EQ(++prog, p++);
            //    ptr._Store_and_unlock(reinterpret_cast<long*>(p));
            //});
            //EXPECT_EQ(1000000, reinterpret_cast<size_t>(ptr._Unsafe_load_relaxed()));
        }






        if (1) {
            GL::atomic_shared_ptr<int> ptr;
            auto* p = ptr.load();



        }

#endif
    }
    return 0;
};
