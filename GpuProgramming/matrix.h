#pragma once
#include <memory>
#include <string>

namespace GL {
    namespace GPU {
        // wrapper for the dimensions of a GPU matrix.
        struct 
            dimensions {
            unsigned int X;
            unsigned int Y;
            unsigned int Z;
            unsigned int num_dimensions() const {
                return std::max<unsigned int>(1u, (unsigned int)(X > 1u) + (unsigned int)(Y > 1u) + (unsigned int)(Z > 1u));
            };
            unsigned int count() const {
                return std::max<unsigned int>(1u, X) * std::max<unsigned int>(1u, Y) * std::max<unsigned int>(1u, Z);
            };
            // ensure a minimum of 1 in each slot
            dimensions ensure() const {
                return dimensions{
                    std::max<unsigned int>(1u, X),
                    std::max<unsigned int>(1u, Y),
                    std::max<unsigned int>(1u, Z)
                };
            };
        };

        // GPU-accelerated linear algebra wrapper. Should only be used by one thread at a time. 
        template <typename T> class 
            matrix;

        // a matrix_kernel is a matrix dedicated to the purpose of being a convolution kernel. 
        template <typename T> class 
            matrix_kernel {
        public:
            std::unique_ptr<matrix<T>>
                mat;
            T
                sum;

            matrix_kernel() 
                : mat{ nullptr }, sum{ T{} } 
            {};
            matrix_kernel(matrix_kernel const&) = delete;
            matrix_kernel(matrix<T>&& rhs) noexcept
                : mat{ std::make_unique<matrix<T>>(std::move(rhs)) }
                , sum{ T{} }
            {
                sum = mat->sum();
            };
            matrix_kernel& operator=(matrix_kernel const&) = delete;
            matrix_kernel& operator=(matrix_kernel&& rhs) noexcept {
                mat = std::move(rhs.mat);
                rhs.mat = nullptr;
                sum = rhs.sum;
                rhs.sum = T{};
                return *this;
            };
            ~matrix_kernel() = default;
        };

        // a static_matrix_kernel is a matrix_kernel which promises to never change its value and to out-live any jobs that use it as input. 
        // This will prevent accumulation of GPU jobs, but risks memory corruption if this is destroyed too early. Best used when static or thead_local. 
        // Leveraged when calling guassian_kernel. 
        template <typename T> struct 
            static_matrix_kernel { matrix_kernel<T>* ptr; };

        // GPU-accelerated linear algebra wrapper. Should only be used by one thread at a time.
        // Leverages an arena memory pool and deferred destruction for higher performance. 
        template <typename T> class 
            matrix {
        private:
            void* // std::unique_ptr<mem_matrix, mem_matrix::helper< mem_matrix>::array_delete>
                memory;
            GL::GPU::dimensions
                dim;
            template <typename G> friend class matrix;

        public:
            typedef T type;

            void*& 
                internal_memory() { return memory; };

            // normal constructor
            explicit matrix(GL::GPU::dimensions d);
            // normal constructor
            explicit matrix(unsigned int X, unsigned int Y = 1, unsigned int Z = 1, bool cpu_only = false);
            // empty constructor
            matrix();
            // copy another, existing matrix into this. Does not take or share ownership.
            matrix(matrix const& rhs);
            // move another, temporary matrix into this, taking ownership of data.
            matrix(matrix&& rhs) noexcept;
            matrix& operator=(matrix const& rhs);
            matrix& operator=(matrix&& rhs) noexcept;
            ~matrix();

            // x*y*z
            unsigned int size() const;
            // returns x, y, or z, depending on the requested dimension.
            unsigned int size(unsigned int d) const;

            // Copies data into CPU-GPU shared memory for quick access or reading. 
            class reader {
                std::shared_ptr<T[]> data;
                GL::GPU::dimensions dim;

            public:
                reader(matrix<T>& copy, GL::GPU::dimensions const& D);
                reader(reader const&) = delete;
                reader(reader&& rhs) noexcept;
                reader& operator=(reader const&) = delete;
                reader& operator=(reader&&) = delete;
                ~reader() = default;
                operator bool() const;
                T const& operator[](unsigned int X) const;
                T const& operator()(unsigned int X, unsigned int Y = 0, unsigned int Z = 0) const;

                std::shared_ptr<T[]> get() const;
            };
            // Copies data into CPU-GPU shared memory for quick access or reading. If requested, may also only work on the CPU side. 
            class writer {
                std::shared_ptr<T[]> gpu_cpu_data;
                T* cpu_data;
                matrix<T>* data;
                GL::GPU::dimensions dim;
                bool _cpu_only;

            public:
                writer(matrix<T>& copy, GL::GPU::dimensions const& D, bool cpu_only = false);
                writer(writer const&) = delete;
                writer(writer&& rhs) noexcept;
                writer& operator=(writer const&) = delete;
                writer& operator=(writer&&) noexcept = delete;
                ~writer();
                operator bool() const;
                T& operator[](unsigned int X) const;
                T& operator()(unsigned int X, unsigned int Y = 0, unsigned int Z = 0) const;
            };

            reader read() const;
            writer write(bool cpu_only = false);

            matrix& operator=(T rhs);
            matrix& operator+=(T rhs);
            matrix& operator-=(T rhs);
            matrix& operator*=(T rhs);
            matrix& operator/=(T rhs);
            matrix& operator+=(matrix const& rhs);
            matrix& operator-=(matrix const& rhs);
            matrix& operator*=(matrix const& rhs);
            matrix& operator/=(matrix const& rhs);

            template <typename U> friend matrix<U> operator+(matrix<U> const& lhs, matrix<U> const& rhs);
            template <typename U> friend matrix<U> operator-(matrix<U> const& lhs, matrix<U> const& rhs);
            template <typename U> friend matrix<U> operator*(matrix<U> const& lhs, matrix<U> const& rhs);
            template <typename U> friend matrix<U> operator/(matrix<U> const& lhs, matrix<U> const& rhs);
            template <typename U> friend matrix<U> operator+(matrix<U> const& lhs, U rhs);
            template <typename U> friend matrix<U> operator-(matrix<U> const& lhs, U rhs);
            template <typename U> friend matrix<U> operator*(matrix<U> const& lhs, U rhs);
            template <typename U> friend matrix<U> operator/(matrix<U> const& lhs, U rhs);
            template <typename U> friend matrix<U> operator+(U rhs, matrix<U> const& lhs);
            template <typename U> friend matrix<U> operator-(U rhs, matrix<U> const& lhs);
            template <typename U> friend matrix<U> operator*(U rhs, matrix<U> const& lhs);
            template <typename U> friend matrix<U> operator/(U rhs, matrix<U> const& lhs);

            // cast from the current type to the requested type. E.g. from int to float, or char to unsigned long, etc.
            template<typename G> matrix<G> cast() const;
            // For floating-point values, returns 0-1. For all others, returns the range from 0 to the max value. 
            static matrix random(unsigned int X, unsigned int Y = 1, unsigned int Z = 1);
            // returns a random number in the range of (lower, upper]
            static matrix random_between(T lower, T upper, unsigned int X, unsigned int Y = 1, unsigned int Z = 1);
            // Returns a square 2-d matrix whose values are 1.0 along the diagonal, and 0.0 elsewhere.
            static matrix identity(unsigned int width);
            // Returns a matrix with all values linearly increasing from the low value to the high value based on their index. 
            static matrix linear(T low, T high, unsigned int lenX, unsigned int lenY = 1, unsigned int lenZ = 1);
            // Returns a matrix with all values equal to the provided value
            static matrix constant(T value, unsigned int lenX, unsigned int lenY = 1, unsigned int lenZ = 1);
            // For floating-point values, returns 0-1. For all others, returns the range from 0 to the max value. 
            static matrix from_vector(const std::vector<T>& parameters);
            // For floating-point values, returns 0-1. For all others, returns the range from 0 to the max value. 
            static matrix from_vector(const std::vector<T>& parameters, unsigned int LenX);

            // specialization of POW for integer powers
            matrix pown(matrix<int> const& rhs) const;
            // power of 
            matrix pow(matrix const& rhs) const;
            // specialization of POW for integer powers
            matrix pown(int rhs) const;
            // power of 
            matrix pow(T rhs) const;
            // sqrt
            matrix<float> sqrt() const;
            // round to nearest whole number
            matrix round() const;
            // round to higher integer
            matrix ceil() const;
            // round to lower integer
            matrix floor() const;
            // return (this * multiply) + add;
            matrix fma(matrix const& multiply, matrix const& add) const;
            // absolute value
            matrix abs() const;

            matrix cos() const;
            matrix sin() const;
            matrix tan() const;
            matrix acos() const;
            matrix asin() const;
            matrix atan() const;
            matrix cosh() const;
            matrix sinh() const;
            matrix tanh() const;
            matrix acosh() const;
            matrix asinh() const;
            matrix atanh() const;
            // e^x
            matrix exp() const;
            // 2^x
            matrix exp2() const;
            // 10^x
            matrix exp10() const;
            // e^x-1
            matrix expm1() const;
            // log gamma function
            matrix lgamma() const;
            // ln(x)
            matrix log() const;
            // log_2(x)
            matrix log2() const;
            // log_10(x)
            matrix log10() const;
            // ln(1+x)
            matrix log1p() const;
            // return this % rhs
            matrix mod(T rhs) const;
            // return this % rhs
            matrix mod(matrix const& rhs) const;

            template <typename U> friend matrix<U> operator%(matrix<U> const& lhs, matrix<U> const& rhs);
            template <typename U> friend matrix<U> operator%(matrix<U> const& lhs, U rhs);
            // returns the max of the two arrays (item-by-item, as an array)
            matrix max(matrix const& rhs) const;
            // returns the max of the two arrays (item-by-item, as an array)
            matrix max(T rhs) const;
            // returns the min of the two arrays (item-by-item, as an array)
            matrix min(matrix const& rhs) const;
            // returns the min of the two arrays (item-by-item, as an array)
            matrix min(T rhs) const;
            matrix<unsigned int> operator!() const;
            matrix<unsigned int> operator==(T rhs) const;
            matrix<unsigned int> operator!=(T rhs) const;
            matrix<unsigned int> operator<(T rhs) const;
            matrix<unsigned int> operator<=(T rhs) const;
            matrix<unsigned int> operator>(T rhs) const;
            matrix<unsigned int> operator>=(T rhs) const;
            template <typename U> friend matrix<unsigned int> operator==(matrix<U> const& lhs, matrix<U> const& rhs);
            template <typename U> friend matrix<unsigned int> operator!=(matrix<U> const& lhs, matrix<U> const& rhs);
            template <typename U> friend matrix<unsigned int> operator<(matrix<U> const& lhs, matrix<U> const& rhs);
            template <typename U> friend matrix<unsigned int> operator<=(matrix<U> const& lhs, matrix<U> const& rhs);
            template <typename U> friend matrix<unsigned int> operator>(matrix<U> const& lhs, matrix<U> const& rhs);
            template <typename U> friend matrix<unsigned int> operator>=(matrix<U> const& lhs, matrix<U> const& rhs);
            matrix<unsigned int> operator&&(T rhs) const;
            matrix<unsigned int> operator&&(matrix const& rhs) const;
            matrix<unsigned int> operator||(T rhs) const;
            matrix<unsigned int> operator||(matrix const& rhs) const;
            // joins two matrices along one of the dimensions.
            matrix join(unsigned int jdim, matrix const& first) const;
            // transpose a 2-D matrix along its diagonal. Does not support transposition of 3-D matrices. 
            matrix transpose() const;
            // pad a matrix with zeros to make its X and Y components square. Used for calculating the inverse. 
            matrix make_square() const;
            // extracts the diagonal of a 2-D matrix as a 1-D array
            matrix diagonal() const;
            // extract a row from this 2-D matrix as a 1-D array
            matrix row(unsigned int rowN) const;
            // grow a matrix by wrapping the new values around to the start. Only works for a 1-D vector. 
            matrix grow_by_wrapping(unsigned int new_length) const;
            // create a new array by sampling this array at the provided indices. E.g. This = [5,4,3,2,1,0]
            // Indices = [5,5,5,5,5,5,5,4,4,4,4,4,4,3,3,3,3,3,2,2,2,2,1,1,1,0,0]
            // Result = [0,0,0,0,0,0,0,1,1,1,1,1,2,2,2,2,2,3,3,3,3,4,4,4,5,5]
            matrix resample(matrix<unsigned int> const& sample_indices) const;
            // calculate the determinant for a square matrix. Performed on the CPU, and minimizes exchanges with the GPU. 
            float determinant() const;

            // cofactor of a square matrix, essential for calculating the inverse
            matrix<float> cofactor() const;

            // transpose of the cofactor of a square matrix
            matrix<float> adjoint() const;

            // solve for the inverse of the matrix. Does not support solving for the inverse of a 3-D matrix. 
            matrix<float> inverse() const;

            // performs a cross-multiplication of two rectangular matrices. This is not accelerated by the GPU, and is CPU-bound. Uses CPU multithreading to (attempt) to speed-up this bottleneck. 
            // the number of columns in this matrix must equal the number of rows in the RHS matrix. 
            matrix<float> matrix_multiply(matrix const& rhs) const;

            // test to see if there is any colinearity in the feature set. If so, it is impossible to solve for the linear regression. One or multiple features must be removed until it is no longer invalid.
            bool is_colinear() const;

            T sum() const;
            T avg() const;
            T max() const;
            T min() const;

            matrix convolve(matrix_kernel<T> const& K) const;
            matrix convolve(static_matrix_kernel<T> const& K) const;
            static matrix_kernel<float> guassian_kernel(unsigned int X, unsigned int Y);
            template <unsigned int X, unsigned int Y>
            static static_matrix_kernel<float> guassian_kernel() {
                static matrix_kernel<float> out{ []() {
                    return guassian_kernel(X, Y);
                }() };
                return static_matrix_kernel<float>{ &out };
            };

            matrix<char> ASCII() const;
            matrix resize(unsigned int X, unsigned int Y, unsigned Z) const;
            matrix resize_stretch(unsigned int X, unsigned int Y, unsigned Z) const;
            matrix subsample_1D(matrix<float> const& FloatingPointIndexes) const;
            matrix<unsigned int> binomial_search_smallest_gre(matrix const& find) const;
            // assumes *this is the sample X-coordinates. X and Y are the components of a pattern that will be sub-sampled using a catmulrom spline.
            matrix subsample_pat(matrix const& X, matrix const& Y) const;
            matrix<float> halfsize() const;
            matrix<float> quartersize() const;
            matrix doublesize() const;
            matrix quadruplesize() const;

        private:
            static std::string&& resize(std::string&& rhs, unsigned int len, const char def = 0);
            std::string to_string_impl(reader const& R, unsigned int x) const;
            std::string to_string_impl(reader const& R, unsigned int x, unsigned int y) const;
            std::string to_string_impl(reader const& R, unsigned int x, unsigned int y, unsigned int z) const;
            std::vector<unsigned int> evaluate_column_sizes(reader const& R, std::vector<std::string> column_titles = {}) const;

        public:
            // y-axis are columns, x-axis are rows. Z-axis is ignored (for now). 
            std::string to_string(std::vector<std::string> column_titles = {}, bool doNotSkip = false) const;
            template <typename U> friend std::ostream& operator<<(std::ostream& os, matrix<U> const& obj);

        public:
            std::shared_ptr<T[]> slice(size_t offset = 0, size_t length = std::numeric_limits<size_t>::max()) const;
            T operator[](unsigned int n) const;
            T operator()(unsigned int x, unsigned int y = 0, unsigned int z = 0) const;

            class linear_regressions {
            public:
                // solve for the weights to be used when performing linearized predictions, as determined by a basic linear regression.
                __declspec(noinline) static matrix solve_for_weights(matrix const& measurements, matrix const& features);
                // solve for the linearized prediction.
                __declspec(noinline) static matrix predict(matrix const& features, matrix const& weights);
                // returns the standard error of the linear regression.
                __declspec(noinline) static matrix standard_error(matrix const& measurements, matrix const& features, matrix const& weights);
                // returns the population standard deviation.
                __declspec(noinline) static matrix standard_deviation(matrix const& measurements, matrix const& features, matrix const& weights);
                // evaluate for the students-t test
                __declspec(noinline) static matrix t_statistic(matrix const& weights, matrix const& std_err);
                // evaluate for the p-value
                __declspec(noinline) static matrix p_value(matrix const& features, matrix const& t_stat);
                // build a collection of features for a linear regression while avoiding colinearity. 
                __declspec(noinline) static matrix build_features(matrix const& current_best);
                // build a collection of features for a linear regression while avoiding colinearity. 
                template <typename T, typename... Ts> __declspec(noinline) static matrix build_features(matrix const& current_best, T const& candidate, const Ts&... further_candidates) {
                    auto joined = current_best.join(1, candidate);
                    if (joined.is_colinear()) {
                        return build_features(current_best, further_candidates...);
                    }
                    else {
                        return build_features(joined, further_candidates...);
                    }
                };

                // note: 
                // R2 = 1 - Residual SS / Total SS    (general formula for R2)
                // Adjusted R2 = R2 - (1-R2 )*(k-1)/(n-k) = .8025 - .1975*2/2 = 0.6050.
            };
        };

        // User should never call this function. It exists to ensure the matrix<T> is correctly compiled at least once from the cpp file. 
        void 
            pre_compile();
    };
    
    // shared memory manager, that allows faster allocation/deallocation in multithreaded environments. 
    // Threads allocate in their own TLS without competition, but can (in parallel) deallocate back into other threads. 
    // At-risk of temporary memory leaks if a thread ID is made, used, and then never used again. The memory would be recovered once another thread comes to use it's old slot.
    // Overall, not high a risk as long as the workload is consistant or patterned. 
    class arena_memory_pool {
    public:
        constexpr arena_memory_pool() noexcept = default;
        constexpr arena_memory_pool(const arena_memory_pool&) noexcept = default;
        constexpr arena_memory_pool(arena_memory_pool&&) noexcept = default;
        arena_memory_pool& operator=(const arena_memory_pool&) noexcept = default;
        arena_memory_pool& operator=(arena_memory_pool&&) noexcept = default;
        ~arena_memory_pool() noexcept = default;
        void operator()(void* p) const noexcept { free(p); };
    private:
        static _NODISCARD void*
            malloc_bytes(unsigned int bytes);
        template<typename T, typename... Args> _NODISCARD static T*
            instance(Args&&... args) {
            T* out = (T*)(malloc_bytes(sizeof(T) * 1));
            new (&out[0]) T(std::move(args)...);
            return out;
        };
    public:    
        static void
            free(void* p);
        template<typename T> _NODISCARD static T*
            malloc(unsigned int count) {
            return (T*)malloc_bytes(sizeof(T) * count);
        };
        template <class _Ty, class... _Types, std::enable_if_t<!std::is_array_v<_Ty>, int> = 0> _NODISCARD static auto 
            make_unique(_Types&&... _Args) { // make a unique_ptr
            return std::unique_ptr<_Ty, arena_memory_pool>(instance<_Ty>(std::move(_Args)...));
        };
        template <class _Ty, class... _Types, std::enable_if_t<std::is_array_v<_Ty>, int> = 0> _NODISCARD static auto
            make_unique(unsigned int count) { // make a unique_ptr
            return std::unique_ptr<_Ty, arena_memory_pool>(malloc<std::remove_pointer_t<std::decay_t<_Ty>>>(count));
        };
        template <class _Ty, class... _Types, std::enable_if_t<!std::is_array_v<_Ty>, int> = 0> _NODISCARD static auto
            make_shared(_Types&&... _Args) { // make a unique_ptr
            return std::shared_ptr<_Ty>(instance<_Ty>(std::move(_Args)...), [](_Ty* p) {
                free(p);
            });
        };
        template <class _Ty, class... _Types, std::enable_if_t<std::is_array_v<_Ty>, int> = 0> _NODISCARD static auto
            make_shared(unsigned int count) { // make a unique_ptr
            return std::shared_ptr<_Ty>(malloc<std::remove_pointer_t<std::decay_t<_Ty>>>(count), [](std::decay_t<_Ty> p) {
                free(p);
            });
        };
        static std::string
            debug();
    };
};