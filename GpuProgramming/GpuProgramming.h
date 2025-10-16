#pragma once
#define WIN32_LEAN_AND_MEAN
#include <map>

namespace GL {
    // type-erased version on a floating-point OR integer type.
    class Number {
    public:
        union {
            double V1;
            long long V2;
        } data;
        bool integer;

        Number() = default;
        Number(Number const&) = default;
        Number(Number&&) = default;
        Number& operator=(Number const&) = default;
        Number& operator=(Number&&) = default;
        ~Number() = default;

        template <typename T, typename = std::enable_if_t<!std::is_same_v<T, Number>>>
        Number(T rhs) {
            if constexpr (std::is_floating_point_v<T>) {
                data.V1 = (long double)rhs;
                integer = false;
            }
            else {
                data.V2 = (long long)rhs;
                integer = true;
            }
        };

        operator char() const {
            if (integer) return static_cast<char>(data.V2);
            else return static_cast<char>((long long)data.V1);
        };
        operator unsigned char() const {
            if (integer) return static_cast<unsigned char>(data.V2);
            else return static_cast<unsigned char>((long long)data.V1);
        };
        operator int() const {
            if (integer) return static_cast<int>(data.V2);
            else return static_cast<int>((long long)data.V1);
        };
        operator unsigned int() const {
            if (integer) return static_cast<unsigned int>(data.V2);
            else return static_cast<unsigned int>((long long)data.V1);
        };
        operator long() const {
            if (integer) return static_cast<long>(data.V2);
            else return static_cast<long>((long long)data.V1);
        };
        operator unsigned long() const {
            if (integer) return static_cast<unsigned long>(data.V2);
            else return static_cast<unsigned long>((long long)data.V1);
        };
        operator float() const {
            if (integer) return (float)(long double)data.V2;
            else return (float)data.V1;
        };
        operator double() const {
            if (integer) return (double)(long double)data.V2;
            else return (double)data.V1;
        };
    };
    enum class ArrayTypes {
        EMPTY
        , CHAR
        , UCHAR
        , INT
        , UINT
        , LONG
        , ULONG
        , FLOAT
        , DOUBLE
    };
    // type-erased version of gpu_array, to be used as the library API
    class Array {
    private:
        ArrayTypes _type;
        std::shared_ptr<void> _data; // gpu_array<T>, type-erased

    public:
        Array(ArrayTypes _t, std::shared_ptr<void>&& _d) : _type(_t), _data{ std::move(_d) } {};
        Array() : _type(ArrayTypes::EMPTY), _data{ nullptr } {};
        Array(ArrayTypes type);
        Array(ArrayTypes type, unsigned int X, unsigned int Y = 1, unsigned int Z = 1);
        Array(Array const&) = default;
        Array(Array&&) = default;
        Array& operator=(Array const&) = default;
        Array& operator=(Array&&) = default;
        ~Array() = default;

        class reader {
        private:
            ArrayTypes _type;
            std::shared_ptr<void> _reader_impl;

        public:
            reader(ArrayTypes T, std::shared_ptr<void>&& _reader);
            reader(reader const&) = delete;
            reader(reader&&) = delete;
            reader& operator=(reader const&) = delete;
            reader& operator=(reader&&) = delete;
            ~reader() = default;
            operator bool() const;
            Number operator[](unsigned int X) const;
            Number operator()(unsigned int X, unsigned int Y = 0, unsigned int Z = 0) const;
        };
        class writer {
        private:
            ArrayTypes _type;
            std::shared_ptr<void> _reader_impl;

        public:
            writer(ArrayTypes T, std::shared_ptr<void>&& _reader);
            writer(writer const&) = delete;
            writer(writer&&) = delete;
            writer& operator=(writer const&) = delete;
            writer& operator=(writer&&) = delete;
            ~writer() = default;
            operator bool() const;
            Number load(unsigned int X, unsigned int Y = 0, unsigned int Z = 0) const;
            void store(Number V, unsigned int X, unsigned int Y = 0, unsigned int Z = 0) const;
            Number exchange(Number V, unsigned int X, unsigned int Y = 0, unsigned int Z = 0) const;

        };

        // wrapper that allows reads the current values from the GPU buffer. Reading is done once on construction. 
        reader read() const;
        // wrapper that allows overwritting the current values on the GPU buffer. Updates are queued until the wrapper is destroyed then submitted all-at-once. 
        writer write() const;
        unsigned int size() const;
        unsigned int size(unsigned int D) const;

        Array copy() const;
        Array& operator=(Number rhs);
        Array& operator+=(Number rhs);
        Array& operator-=(Number rhs);
        Array& operator*=(Number rhs);
        Array& operator/=(Number rhs);
        Array& operator+=(Array const& rhs);
        Array& operator-=(Array const& rhs);
        Array& operator*=(Array const& rhs);
        Array& operator/=(Array const& rhs);

        friend Array operator+(Array const& lhs, Array const& rhs);
        friend Array operator-(Array const& lhs, Array const& rhs);
        friend Array operator*(Array const& lhs, Array const& rhs);
        friend Array operator/(Array const& lhs, Array const& rhs);
        friend Array operator+(Array const& lhs, Number rhs);
        friend Array operator-(Array const& lhs, Number rhs);
        friend Array operator*(Array const& lhs, Number rhs);
        friend Array operator/(Array const& lhs, Number rhs);
        friend Array operator+(Number rhs, Array const& lhs);
        friend Array operator-(Number rhs, Array const& lhs);
        friend Array operator*(Number rhs, Array const& lhs);
        friend Array operator/(Number rhs, Array const& lhs);

        Array operator!() const;
        Array operator==(Number rhs) const;
        Array operator!=(Number rhs) const;
        Array operator<(Number rhs) const;
        Array operator<=(Number rhs) const;
        Array operator>(Number rhs) const;
        Array operator>=(Number rhs) const;

        friend Array operator==(Array const& lhs, Array const& rhs);
        friend Array operator!=(Array const& lhs, Array const& rhs);
        friend Array operator<(Array const& lhs, Array const& rhs);
        friend Array operator<=(Array const& lhs, Array const& rhs);
        friend Array operator>(Array const& lhs, Array const& rhs);
        friend Array operator>=(Array const& lhs, Array const& rhs);
        friend Array operator%(Array const& lhs, Array const& rhs);
        friend Array operator%(Array const& lhs, Number rhs);

        Array operator&&(Number rhs) const;
        Array operator&&(Array const& rhs) const;
        Array operator||(Number rhs) const;
        Array operator||(Array const& rhs) const;

        // power of 
        Array pow(Array const& rhs) const;
        // specialization of POW for integer powers
        Array pown(int rhs) const;
        // power of 
        Array pow(Number rhs) const;
        // sqrt
        Array sqrt() const;
        // round to nearest whole number
        Array round() const;
        // round to higher integer
        Array ceil() const;
        // round to lower integer
        Array floor() const;
        // absolute value
        Array abs() const;
        Array cos() const;
        Array sin() const;
        Array tan() const;
        Array acos() const;
        Array asin() const;
        Array atan() const;
        Array cosh() const;
        Array sinh() const;
        Array tanh() const;
        Array acosh() const;
        Array asinh() const;
        Array atanh() const;
        // e^x
        Array exp() const;
        // 2^x
        Array exp2() const;
        // 10^x
        Array exp10() const;
        // e^x-1
        Array expm1() const;
        // log gamma function
        Array lgamma() const;
        // ln(x)
        Array log() const;
        // log_2(x)
        Array log2() const;
        // log_10(x)
        Array log10() const;
        // ln(1+x)
        Array log1p() const;
        // return this % rhs
        Array mod(Number rhs) const;
        // return this % rhs
        Array mod(Array const& rhs) const;
        // return (this * multiply) + add;
        Array fma(Array const& multiply, Array const& add) const;
        // returns the max of the two arrays (item-by-item, as an array)
        Array max(Array const& rhs) const;
        // returns the max of the two arrays (item-by-item, as an array)
        Array max(Number rhs) const;
        // returns the min of the two arrays (item-by-item, as an array)
        Array min(Array const& rhs) const;
        // returns the min of the two arrays (item-by-item, as an array)
        Array min(Number rhs) const;
        // resize the array, without stretching. Shrinking will sample less than the original, growing will fill with zero's. 
        Array resize(unsigned int X, unsigned int Y, unsigned Z) const;
        // resize the array with stretching. Shrinking will sample fewer cells, growing may sample cells repeatedly. 
        Array resize_stretch(unsigned int X, unsigned int Y, unsigned Z) const;
        // blurs the array and then reduces the X and Y dimensions by 2. 
        Array halfsize(bool skip_blur = false) const;
        // blurs the array and then reduces the X and Y dimensions by 4. 
        Array quartersize(bool skip_blur = false) const;
        // increases the X and Y dimensions by 2. 
        Array doublesize() const;
        // increases the X and Y dimensions by 4. 
        Array quadruplesize() const;

        std::string to_string(std::vector<std::string> column_titles = {}, bool doNotSkip = false) const;
        friend std::ostream& operator<<(std::ostream& os, Array const& obj);

        // cast from the current type to the requested type. E.g. from int to float, or char to unsigned long, etc.
        Array cast(ArrayTypes T) const;

        // For floating-point values, returns 0-1. For all others, returns the range from 0 to the max value. 
        static Array random(ArrayTypes T, unsigned int X, unsigned int Y = 1, unsigned int Z = 1);
        // returns a random number in the range of (lower, upper]
        static Array random_between(ArrayTypes T, Number lower, Number upper, unsigned int X, unsigned int Y = 1, unsigned int Z = 1);
        // Returns a square 2-d matrix whose values are 1.0 along the diagonal, and 0.0 elsewhere.
        static Array identity(ArrayTypes T, unsigned int width);
        // Returns a matrix with all values linearly increasing from the low value to the high value based on their index. 
        static Array linear(ArrayTypes T, Number low, Number high, unsigned int X, unsigned int Y = 1, unsigned int Z = 1);
        // Returns a matrix with all values equal to the provided value
        static Array constant(ArrayTypes T, Number value, unsigned int X, unsigned int Y = 1, unsigned int Z = 1);
        static Array from_vector(ArrayTypes T, const std::vector<Number>& parameters, unsigned int Y = 1, unsigned int Z = 1);
        static Array guassian_kernel(unsigned int X, unsigned int Y);

        // joins two matrices along one of the dimensions.
        Array join(unsigned int jdim, Array const& first) const;
        // transpose a 2-D matrix along its diagonal. Does not support transposition of 3-D matrices. 
        Array transpose() const;
        // pad a matrix with zeros to make its X and Y components square. Used for calculating the inverse. 
        Array make_square() const;
        // extracts the diagonal of a 2-D matrix as a 1-D array
        Array diagonal() const;
        // extract a row from this 2-D matrix as a 1-D array
        Array row(unsigned int rowN) const;
        // grow a matrix by wrapping the new values around to the start. Only works for a 1-D vector. 
        Array grow_by_wrapping(unsigned int new_length) const;
        // create a new array by sampling this array at the provided indices. E.g. This = [5,4,3,2,1,0]
        // Indices = [5,5,5,5,5,5,5,4,4,4,4,4,4,3,3,3,3,3,2,2,2,2,1,1,1,0,0]
        // Result = [0,0,0,0,0,0,0,1,1,1,1,1,2,2,2,2,2,3,3,3,3,4,4,4,5,5]
        Array resample(Array const& sample_indices) const;
        // calculate the determinant for a square matrix. Performed on the CPU, and minimizes exchanges with the GPU. 
        float determinant() const;
        // cofactor of a square matrix, essential for calculating the inverse
        Array cofactor() const;
        // transpose of the cofactor of a square matrix
        Array adjoint() const;
        // solve for the inverse of the matrix. Does not support solving for the inverse of a 3-D matrix. 
        Array inverse() const;
        // performs a cross-multiplication of two rectangular matrices. This is not accelerated by the GPU, and is CPU-bound. Uses CPU multithreading to (attempt) to speed-up this bottleneck. 
        // the number of columns in this matrix must equal the number of rows in the RHS matrix. 
        Array matrix_multiply(Array const& rhs) const;
        // test to see if there is any colinearity in the feature set. If so, it is impossible to solve for the linear regression. One or multiple features must be removed until it is no longer invalid.
        bool is_colinear() const;
        Number sum() const;
        Number avg() const;
        Number max() const;
        Number min() const;
        Array convolve(Array const& kernel) const;
        Array ASCII() const;
    };
    class linear_regression {
    public:
        // solve for the weights to be used when performing linearized predictions, as determined by a basic linear regression.
        static Array solve_for_weights(Array const& measurements, Array const& features);
        // solve for the linearized prediction.
        static Array predict(Array const& features, Array const& weights);
        // returns the standard error of the linear regression.
        static Array standard_error(Array const& measurements, Array const& features, Array const& weights);
        // returns the population standard deviation.
        static Array standard_deviation(
            Array const& measurements,
            Array const& features,
            Array const& weights
        );
        // evaluate for the students-t test
        static Array t_statistic(Array const& weights, Array const& std_err);
        // evaluate for the p-value
        static Array p_value(Array const& features, Array const& t_stat);

        // build a collection of features for a linear regression while avoiding colinearity. 
        static Array build_features(Array const& current_best);
        // build a collection of features for a linear regression while avoiding colinearity. 
        static Array build_features(Array&& current_best);
        // build a collection of features for a linear regression while avoiding colinearity. 
        template <typename T, typename... Ts> __forceinline static Array build_features(Array const& current_best, T const& candidate, const Ts&... further_candidates) {
            auto conjoined = current_best.join(1, candidate);
            if (conjoined.is_colinear()) {
                return build_features(current_best, further_candidates...);
            }
            else {
                return build_features(conjoined, further_candidates...);
            }
        };

    };

};

void fnGpuProgramming();