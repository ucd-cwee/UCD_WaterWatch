#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <regex>
#include <list>
#include <cstdarg>
#include <ShlDisp.h>
#include <winnt.h>

// Good Language namespace
namespace GL {
    // Shared strings with fast, robust support functions. Modifying strings requires creating a new string.
    class string {
    public:
        using size_type = std::string_view::size_type;
        static constexpr const auto npos = std::string_view::npos;

    protected:
        std::shared_ptr<std::string>
            _data; // maintains ownership of the data if necessary
        std::string_view
            data;
        size_type
            _hash{ npos };

        string(std::shared_ptr<std::string> _d, std::string_view d) : _data(std::move(_d)), data(std::move(d)) {};

    public:
        string() {};
        string(string const&) = default;
        string(string&&) = default;
        string& operator=(string const&) = default;
        string& operator=(string&&) = default;
        virtual ~string() = default;

        template <size_t N> __forceinline string(const char(&r)[N]) : data(r) {};
        string(std::string&& _Copy) : _data(std::make_shared<std::string>(std::move(_Copy))) {
            data = *_data;
        };
        string(std::string_view&& _Copy) : data(_Copy) {};

        friend bool operator==(string const& A, string const& V) noexcept {
            if (A.data.length() != V.data.length()) return false;
            /*else if (A.data.length() > 1) */return A.hash() == V.hash();
            // else return A.data == V.data;
        };
        friend bool operator<(string const& A, string const& V) {
            if (A.data.length() < V.data.length()) return true;
            else if (A.data.length() > V.data.length()) return false;
            else /*if (A.length() > 1)*/ return A.hash() < V.hash();
            //else return A.data < V.data;
        };
        friend bool operator<=(string const& A, string const& V) {
            if (A.data.length() < V.data.length()) return true;
            else if (A.data.length() > V.data.length()) return false;
            else /*if (A.length() > 1)*/ return A.hash() <= V.hash();
            //else return A.data <= V.data;
        };
        friend bool operator>(string const& A, string const& V) { return !operator<=(A, V); };
        friend bool operator>=(string const& A, string const& V) { return !operator<(A, V); };
        friend bool operator!=(string const& A, string const& V) noexcept { return !operator==(A, V); };
        friend std::ostream& operator<<(std::ostream& os, string const& obj) {
            os << obj.data;
            return os;
        };
        friend string operator+(string const& A, string const& B) { return string(std::string(A.data) + std::string(B.data)); };

    private:
        static size_type	        FindString(std::string_view const& str, std::string_view const& text, bool casesensitive = true, long long start = 0, long long end = -1) {
            long long l, j, k;
            k = text.length();
            if (end == -1) {
                end = str.length();
            }
            l = end - k;

            if (k <= 0 || (l - start) < 0) return std::string::npos;

            if (casesensitive) {
                const char sample = text[0];
                if (!sample) return (size_t)start;
                for (; start <= l; ++start) // starting at the search position ... 
                    if (str[start] == sample)  // found a match for the first character ...
                        for (j = 1; ; ++j) { // for the remaining parts of the search text ... 
                            if (j >= k) return start;
                            if (str[start + j] != text[j]) break;
                        }
            }
            else {
                for (; start <= l; ++start)
                    for (j = 0;; j++) {
                        if (j >= k) return (size_t)start;
                        if (::toupper(str[start + j]) != ::toupper(text[j]))
                            break;
                    }
            }
            return std::string::npos;
        };
        static bool                 ReplaceString(string& String, const std::string_view& from, const std::string_view& to) {
            size_t startPos;
            bool ret;

            ret = false;
            if (from.empty() || (from == to)) return ret;

            startPos = FindString(String.data, from, true, 0);
            if (startPos != std::string::npos) String = string(std::string(String.data)); // make a new copy of the data
            while (startPos != std::string::npos) {
                ret = true;
                String._data->replace(startPos, from.length(), to);
                String.data = *String._data;
                startPos = FindString(String.data, from, true, to.length() + startPos);
            }
            return ret;
        };

    public:
        GL::string add_to_delim(GL::string const& to_add, GL::string const& delim) const {
            if (this->length() > 0) {
                return GL::string(*this) + delim + to_add;
            }
            else {
                return to_add;
            }
        };
        std::string to_string() const {
            return std::string(data);
        };
        double to_number() const {
            const char* p = data.data();
            double r = 0.0;
            if (this->length() > 0 && p) {
                bool neg = false;
                if (*p == '-') {
                    neg = true;
                    ++p;
                }
                while (*p >= '0' && *p <= '9') {
                    r = (r * 10.0) + (*p - '0');
                    ++p;
                }
                if (*p == '.') {
                    double f = 0.0;
                    int n = 0;
                    ++p;
                    while (*p >= '0' && *p <= '9') {
                        f = (f * 10.0) + (*p - '0');
                        ++p;
                        ++n;
                    }
                    r += f / std::pow(10.0, n);
                }
                if (neg) {
                    r = -r;
                }
            }
            return r;
        };

        auto begin() const {
            return data.begin();
        };
        auto cbegin() const {
            return data.cbegin();
        };
        auto end() const {
            return data.end();
        };
        auto cend() const {
            return data.cend();
        };
        auto rbegin() const {
            return data.rbegin();
        };
        auto rend() const {
            return data.rbegin();
        };
        auto crbegin() const {
            return data.crbegin();
        };
        auto crend() const {
            return data.crbegin();
        };

    private:
        static bool			        CharIsLower(size_t c) {
            return (c >= 'a' && c <= 'z') || (c >= 0xE0 && c <= 0xFF);
        };
        static bool			        CharIsUpper(size_t c) {
            return (c <= 'Z' && c >= 'A') || (c >= 0xC0 && c <= 0xDF);
        };
        static void                 ToLower(char* s) {
            for (size_t i = 0; s[i]; i++) {
                if (CharIsUpper(s[i])) {
                    s[i] += ('a' - 'A');
                }
            }
        };
        static void                 ToUpper(char* s) {
            for (size_t i = 0; s[i]; i++) {
                if (CharIsLower(s[i])) {
                    s[i] -= ('a' - 'A');
                }
            }
        };
        static bool			        CharIsAlpha(size_t c) {
            return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= 0xC0 && c <= 0xFF));
        };
        static bool			        CharIsNumeric(size_t c) {
            return (c <= '9' && c >= '0');
        };
        static bool			        CharIsNewLine(char c) {
            return (c == '\n' || c == '\r' || c == '\v');
        };
        static bool			        CharIsTab(char c) {
            return (c == '\t');
        };
    public:
        bool has_lower() const {
            for (auto& x : *this) {
                if (CharIsLower(x)) {
                    return true;
                }
            }
            return false;
        };
        bool has_upper() const {
            for (auto& x : *this) {
                if (CharIsUpper(x)) {
                    return true;
                }
            }
            return false;
        };
        string to_lower() const {
            if (has_lower()) {
                auto out = this->to_string();
                ToLower(const_cast<char*>(out.c_str()));
                return out;
            }
            return *this;
        };
        string to_upper() const {
            if (has_upper()) {
                auto out = this->to_string();
                ToUpper(const_cast<char*>(out.c_str()));
                return out;
            }
            return *this;
        };

    private:
        static int                  LevenshteinDistance(const string& a, const string& b, bool caseSensitive = true) {
            string s1;
            string s2;

            if (caseSensitive == true) {
                s1 = a;
                s2 = b;
            }
            else {
                s1 = a.to_lower();
                s2 = b.to_lower();
            }

            const long long m(s1.size());
            const long long n(s2.size());

            if (m == 0) return (int)n;
            if (n == 0) return (int)m;

            int* costs = new int[n + 1];

            for (int k = 0; k <= n; k++) costs[k] = k;

            int i = 0;
            for (auto it1 = s1.cbegin(); it1 != s1.end(); ++it1, ++i) {
                costs[0] = i + 1;
                int corner = i;

                int j = 0;
                for (auto it2 = s2.cbegin(); it2 != s2.end(); ++it2, ++j) {
                    int upper = costs[j + 1];
                    if (*it1 == *it2)
                    {
                        costs[j + 1] = corner;
                    }
                    else
                    {
                        int t(upper < corner ? upper : corner);
                        costs[j + 1] = (costs[j] < t ? costs[j] : t) + 1;
                    }

                    corner = upper;
                }
            }

            int result = costs[n];
            delete[] costs;

            return result;
        }

    public:
        size_t	distance(const string& other, bool caseSensitive = true) const {
            return (size_t)LevenshteinDistance(*this, other, caseSensitive);
        };

    public:
        std::string_view const& c_str() const {
            return data;
        };
        string substr(const size_type _Off = 0, size_type _Count = npos) const {
            return string(this->_data, data.substr(_Off, _Count));
        };
        bool empty() const {
            return data.empty();
        };
        size_type size() const {
            return data.length();
        };
        size_type length() const {
            return data.length();
        };
        const char& at(const size_type _Off) const {
            return data.at(_Off);
        };
        const char& front() const {
            return data.front();
        };
        const char& back() const {
            return data.back();
        };
        const char& operator[](const size_type index) const {
            return data.operator[](index);
        };
        size_type hash(size_type out = 0) const {
            if ((out == 0) && (length() > 16)) {
                if (_hash == npos) {
                    for (auto& x : data) out ^= (size_t)x + 0x9e3779b9 + (out << 6) + (out >> 2);
                    InterlockedExchange(reinterpret_cast<volatile size_type*>(const_cast<size_type*>(&_hash)), out);
                }
                else {
                    out = _hash;
                }
            }
            else {
                for (auto& x : data) out ^= (size_t)x + 0x9e3779b9 + (out << 6) + (out >> 2);
            }
            return out;
        };
        string& remove_prefix(const size_type _Count) noexcept {
            data.remove_prefix(_Count);
            return *this;
        };
        string& remove_suffix(const size_type _Count) noexcept {
            data.remove_suffix(_Count);
            return *this;
        };
        string& remove_prefix(const string& prefix) noexcept {
            if (left(prefix.size()) == prefix) data.remove_prefix(prefix.size());
            return *this;
        };
        string& remove_suffix(const string& suffix) noexcept {
            if (right(suffix.size()) == suffix) data.remove_suffix(suffix.size());
            return *this;
        };
        size_type rfind(const string& _Right) const {
            return data.rfind(_Right.data);
        };
        size_type find(const string& FIND, bool casesensitive = true, long long start = 0, long long end = -1) const {
            if (end == -1) {
                end = this->length();
            }
            if (this->length() == 0 || FIND.length() == 0) return std::string::npos;
            if (FIND.length() > this->length()) return std::string::npos;
            return FindString(this->data, FIND.data, casesensitive, start, end);
        };
        string replace(const string& what, const string& with) const {
            string out(*this);
            (void)ReplaceString(out, what.data, with.data);
            return out;
        };
        string remove_trailing(char _Right) const {
            string out{ *this };
            while (
                (out.length() > 0)
                && ((out.operator[](out.length() - 1) == _Right))
                ) {
                out.remove_suffix(1);
            }
            return out;
        };
        string remove_leading(char _Right) const {
            string out{ *this };
            while (
                (out.length() > 0)
                && (out.operator[](0) == _Right)
                ) {
                out.remove_prefix(1);
            }
            return out;
        };
        string remove_leading_and_trailing(char _Right) const {
            string out{ *this };
            return out.remove_trailing(_Right).remove_leading(_Right);
        };
        static const string& empty_string() {
            static string out{ "" };
            return out;
        };
        static const string& namespace_colons() {
            static string out{ "::" };
            return out;
        };
        string right(size_type _Count) const {
            if (_Count >= length()) return *this;
            else return string(this->_data, this->data.substr(this->data.length() - _Count, _Count));
        };
        string left(size_type _Count) const {
            if (_Count >= length()) return *this;
            else return string(this->_data, this->data.substr(0, _Count));
        };
        // returns the part of this string that is left of the searched content, if found. Otherwise returns everything.
        string left_of(const string& what) const {
            if (auto p = this->find(what); p != npos) {
                return substr(0, p);
            }
            else {
                return *this;
            }
        };
        // returns the part of this string that is right of the searched content, if found. Otherwise returns everything.
        string right_of(const string& what) const {
            if (auto p = this->find(what); p != npos) {
                return substr(p + what.length());
            }
            else {
                return *this;
            }
        };
        // returns the part of this string that is left of the searched content, if found. Otherwise returns everything.
        string left_of_last(const string& what) const {
            if (auto p = this->rfind(what); p != npos) {
                return substr(0, p);
            }
            else {
                return *this;
            }
        };
        // returns the part of this string that is right of the searched content, if found. Otherwise returns everything.
        string right_of_last(const string& what) const {
            if (auto p = this->rfind(what); p != npos) {
                return substr(p + what.length());
            }
            else {
                return *this;
            }
        };
        std::pair<string, string> left_and_right_of(const string& what) const {
            if (auto p = this->find(what); p != npos) {
                return std::pair<string, string>{ substr(0, p), substr(p + what.length()) };
            }
            else {
                return std::pair<string, string>{ *this, "" };
            }
        };
        std::pair<string, string> left_and_right_of_last(const string& what) const {
            if (auto p = this->rfind(what); p != npos) {
                return std::pair<string, string>{ substr(0, p), substr(p + what.length()) };
            }
            else {
                return std::pair<string, string>{ *this, "" };
            }
        };
        bool ends_with(const string& what) const {
            if (auto p = this->rfind(what); p != npos) {
                return (p + what.length()) == this->length();
            }
            else {
                return false;
            }
        };
        bool begins_with(const string& what) const {
            if (auto p = this->find(what); p != npos) {
                return p == 0;
            }
            else {
                return false;
            }
        };

    };

    // formatted string to text with variadric inputs
    __forceinline string printf(const char* format, ...) {
        va_list args;
        va_start(args, format);

        // Determine the required buffer size
        int size = ::vsnprintf(nullptr, 0, format, args);
        va_end(args);

        if ((size < 0) || (size >= 128000)) {
            // Handle error, e.g., return an empty string or throw an exception
            return "";
        }
        else {
            // Allocate buffer and print to it
            std::string buffer(size, '\0'); // Initialize string with null characters
            va_start(args, format);
            ::vsnprintf(&buffer[0], size + 1, format, args); // +1 for null terminator
            va_end(args);
            return buffer;
        }
    };
};

namespace std {
    _NODISCARD inline std::string to_string(GL::string const& _Val) { // convert string to string
        return _Val.to_string();
    };
    template <> struct hash<GL::string> {
        std::size_t operator()(const GL::string& k) const {
            return k.hash();
        };
    };
};