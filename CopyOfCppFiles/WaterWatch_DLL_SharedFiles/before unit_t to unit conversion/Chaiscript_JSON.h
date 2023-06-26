#pragma once
#include "Precompiled.h"

namespace chaiscript::json {
    using std::enable_if;
    using std::initializer_list;
    using std::is_convertible;
    using std::is_floating_point;
    using std::is_integral;
    using std::is_same;

    class JSON {
    public:
        enum class Class {
            Null = 0,
            Object,
            Array,
            String,
            Floating,
            Integral,
            Boolean
        };

    private:
        using Data = std::variant<std::nullptr_t, chaiscript::utility::QuickFlatMap<std::string, JSON>, std::vector<JSON>, std::string, double, std::int64_t, bool>;

        struct Internal {
            Internal(std::nullptr_t)
                : d(nullptr) {
            }
            Internal()
                : d(nullptr) {
            }
            Internal(Class c)
                : d(make_type(c)) {
            }
            template<typename T>
            Internal(T t)
                : d(std::move(t)) {
            }

            static Data make_type(Class c) {
                switch (c) {
                case Class::Null:
                    return nullptr;
                case Class::Object:
                    return chaiscript::utility::QuickFlatMap<std::string, JSON>{};
                case Class::Array:
                    return std::vector<JSON>{};
                case Class::String:
                    return std::string{};
                case Class::Floating:
                    return double{};
                case Class::Integral:
                    return std::int64_t{};
                case Class::Boolean:
                    return bool{};
                }
                throw std::runtime_error("unknown type");
            }

            void set_type(Class c) {
                if (type() != c) {
                    d = make_type(c);
                }
            }

            Class type() const noexcept { return Class(d.index()); }

            template<auto ClassValue, typename Visitor, typename Or>
            decltype(auto) visit_or(Visitor&& visitor, Or&& other) const {
                if (type() == Class(ClassValue)) {
                    return visitor(std::get<static_cast<std::size_t>(ClassValue)>(d));
                }
                else {
                    return other();
                }
            }

            template<auto ClassValue>
            auto& get_set_type() {
                set_type(ClassValue);
                return (std::get<static_cast<std::size_t>(ClassValue)>(d));
            }

            auto& Map() { return get_set_type<Class::Object>(); }
            auto& Vector() { return get_set_type<Class::Array>(); }
            auto& String() { return get_set_type<Class::String>(); }
            auto& Int() { return get_set_type<Class::Integral>(); }
            auto& Float() { return get_set_type<Class::Floating>(); }
            auto& Bool() { return get_set_type<Class::Boolean>(); }

            auto Map() const noexcept { return std::get_if<static_cast<std::size_t>(Class::Object)>(&d); }
            auto Vector() const noexcept { return std::get_if<static_cast<std::size_t>(Class::Array)>(&d); }
            auto String() const noexcept { return std::get_if<static_cast<std::size_t>(Class::String)>(&d); }
            auto Int() const noexcept { return std::get_if<static_cast<std::size_t>(Class::Integral)>(&d); }
            auto Float() const noexcept { return std::get_if<static_cast<std::size_t>(Class::Floating)>(&d); }
            auto Bool() const noexcept { return std::get_if<static_cast<std::size_t>(Class::Boolean)>(&d); }

            Data d;
        };

        Internal internal;

    public:
        template<typename Container>
        class JSONWrapper {
            Container* object = nullptr;

        public:
            JSONWrapper(Container* val)
                : object(val) {
            }
            JSONWrapper(std::nullptr_t) {}

            typename Container::iterator begin() { return object ? object->begin() : typename Container::iterator(); }
            typename Container::iterator end() { return object ? object->end() : typename Container::iterator(); }
            typename Container::const_iterator begin() const { return object ? object->begin() : typename Container::iterator(); }
            typename Container::const_iterator end() const { return object ? object->end() : typename Container::iterator(); }
        };

        template<typename Container>
        class JSONConstWrapper {
            const Container* object = nullptr;

        public:
            JSONConstWrapper(const Container* val)
                : object(val) {
            }
            JSONConstWrapper(std::nullptr_t) {}

            typename Container::const_iterator begin() const noexcept {
                return object ? object->begin() : typename Container::const_iterator();
            }
            typename Container::const_iterator end() const noexcept { return object ? object->end() : typename Container::const_iterator(); }
        };

        JSON() = default;
        JSON(std::nullptr_t) {}

        explicit JSON(Class type)
            : internal(type) {
        }

        JSON(initializer_list<JSON> list)
            : internal(Class::Object) {
            for (auto i = list.begin(), e = list.end(); i != e; ++i, ++i) {
                operator[](i->to_string()) = *std::next(i);
            }
        }

        template<typename T>
        explicit JSON(T b, typename enable_if<is_same<T, bool>::value>::type* = nullptr) noexcept
            : internal(static_cast<bool>(b)) {
        }

        template<typename T>
        explicit JSON(T i, typename enable_if<is_integral<T>::value && !is_same<T, bool>::value>::type* = nullptr) noexcept
            : internal(static_cast<std::int64_t>(i)) {
        }

        template<typename T>
        explicit JSON(T f, typename enable_if<is_floating_point<T>::value>::type* = nullptr) noexcept
            : internal(static_cast<double>(f)) {
        }

        template<typename T>
        explicit JSON(T s, typename enable_if<is_convertible<T, std::string>::value>::type* = nullptr)
            : internal(static_cast<std::string>(s)) {
        }

        static JSON Load(const std::string&);

        JSON& operator[](const std::string& key) {
#ifdef UseCweeThreadedMapAsQuickFlatMap
            return *internal.Map().operator[](key);
#else
            return internal.Map().operator[](key);
#endif        
        }

        JSON& operator[](const size_t index) {
            auto& vec = internal.Vector();
            if (index >= vec.size()) {
                vec.resize(index + 1);
            }

            return vec.operator[](index);
        }

        JSON& at(const std::string& key) { return operator[](key); }

        const JSON& at(const std::string& key) const {
            return internal.visit_or<Class::Object>([&](const auto& m) -> const JSON& {
#ifdef UseCweeThreadedMapAsQuickFlatMap
                return *m.at(key);
#else
                return m.at(key);
#endif

                },
                []() -> const JSON& { throw std::range_error("Not an object, no keys"); });
        }

        JSON& at(size_t index) { return operator[](index); }

        const JSON& at(size_t index) const {
            return internal.visit_or<Class::Array>([&](const auto& m) -> const JSON& { return m.at(index); },
                []() -> const JSON& { throw std::range_error("Not an array, no indexes"); });
        }

        auto length() const noexcept {
            return internal.visit_or<Class::Array>([&](const auto& m) { return static_cast<int>(m.size()); }, []() { return -1; });
        }

        bool has_key(const std::string& key) const noexcept {
            return internal.visit_or<Class::Object>([&](const auto& m) { return m.count(key) != 0; }, []() { return false; });
        }

        int size() const noexcept {
            if (auto m = internal.Map(); m != nullptr) {
                return static_cast<int>(m->size());
            }
            if (auto v = internal.Vector(); v != nullptr) {
                return static_cast<int>(v->size());
            }
            else {
                return -1;
            }
        }

        Class JSONType() const noexcept { return internal.type(); }

        /// Functions for getting primitives from the JSON object.
        bool is_null() const noexcept { return internal.type() == Class::Null; }

        std::string to_string() const noexcept {
            return internal.visit_or<Class::String>([](const auto& o) { return o; }, []() { return std::string{}; });
        }
        double to_float() const noexcept {
            return internal.visit_or<Class::Floating>([](const auto& o) { return o; }, []() { return double{}; });
        }
        std::int64_t to_int() const noexcept {
            return internal.visit_or<Class::Integral>([](const auto& o) { return o; }, []() { return std::int64_t{}; });
        }
        bool to_bool() const noexcept {
            return internal.visit_or<Class::Boolean>([](const auto& o) { return o; }, []() { return false; });
        }

        JSONWrapper<chaiscript::utility::QuickFlatMap<std::string, JSON>> object_range() {
            return std::get_if<static_cast<std::size_t>(Class::Object)>(&internal.d);
        }

        JSONWrapper<std::vector<JSON>> array_range() { return std::get_if<static_cast<std::size_t>(Class::Array)>(&internal.d); }

        JSONConstWrapper<chaiscript::utility::QuickFlatMap<std::string, JSON>> object_range() const {
            return std::get_if<static_cast<std::size_t>(Class::Object)>(&internal.d);
        }

        JSONConstWrapper<std::vector<JSON>> array_range() const { return std::get_if<static_cast<std::size_t>(Class::Array)>(&internal.d); }

        std::string dump(long depth = 1, std::string tab = "  ") const {
            switch (internal.type()) {
            case Class::Null:
                return "null";
            case Class::Object: {
                std::string pad = "";
                for (long i = 0; i < depth; ++i, pad += tab) {
                }

                std::string s = "{\n";
                bool skip = true;
                for (auto& p : *internal.Map()) {
                    if (!skip) {
                        s += ",\n";
                    }
                    s += (pad + "\"" + json_escape(p.first) + "\" : " +
#ifdef UseCweeThreadedMapAsQuickFlatMap
                        p.second->dump(depth + 1, tab)
#else
                        p.second.dump(depth + 1, tab)
#endif
                        );
                    skip = false;
                }
                s += ("\n" + pad.erase(0, 2) + "}");
                return s;
            }
            case Class::Array: {
                std::string s = "[";
                bool skip = true;
                for (auto& p : *internal.Vector()) {
                    if (!skip) {
                        s += ", ";
                    }
                    s += p.dump(depth + 1, tab);
                    skip = false;
                }
                s += "]";
                return s;
            }
            case Class::String:
                return "\"" + json_escape(*internal.String()) + "\"";
            case Class::Floating:
                return std::to_string(*internal.Float());
            case Class::Integral:
                return std::to_string(*internal.Int());
            case Class::Boolean:
                return *internal.Bool() ? "true" : "false";
            }

            throw std::runtime_error("Unhandled JSON type");
        }

    private:
        static std::string json_escape(const std::string& str) {
            std::string output;
            for (char i : str) {
                switch (i) {
                case '\"':
                    output += "\\\"";
                    break;
                case '\\':
                    output += "\\\\";
                    break;
                case '\b':
                    output += "\\b";
                    break;
                case '\f':
                    output += "\\f";
                    break;
                case '\n':
                    output += "\\n";
                    break;
                case '\r':
                    output += "\\r";
                    break;
                case '\t':
                    output += "\\t";
                    break;
                default:
                    output += i;
                    break;
                }
            }
            return output;
        }

    private:
    };
    struct JSONParser {
        static bool isspace(const char c) noexcept {
#ifdef CHAISCRIPT_MSVC
            // MSVC warns on these line in some circumstances
#pragma warning(push)
#pragma warning(disable : 6330)
#endif
            return ::isspace(c) != 0;
#ifdef CHAISCRIPT_MSVC
#pragma warning(pop)
#endif
        }

        static void consume_ws(const std::string& str, size_t& offset) {
            while (isspace(str.at(offset)) && offset <= str.size()) {
                ++offset;
            }
        }

        static JSON parse_object(const std::string& str, size_t& offset) {
            JSON Object(JSON::Class::Object);

            ++offset;
            consume_ws(str, offset);
            if (str.at(offset) == '}') {
                ++offset;
                return Object;
            }

            for (; offset < str.size();) {
                JSON Key = parse_next(str, offset);
                consume_ws(str, offset);
                if (str.at(offset) != ':') {
                    throw std::runtime_error(std::string("JSON ERROR: Object: Expected colon, found '") + str.at(offset) + "'\n");
                }
                consume_ws(str, ++offset);
                JSON Value = parse_next(str, offset);
                Object[Key.to_string()] = Value;

                consume_ws(str, offset);
                if (str.at(offset) == ',') {
                    ++offset;
                    continue;
                }
                else if (str.at(offset) == '}') {
                    ++offset;
                    break;
                }
                else {
                    throw std::runtime_error(std::string("JSON ERROR: Object: Expected comma, found '") + str.at(offset) + "'\n");
                }
            }

            return Object;
        }

        static JSON parse_array(const std::string& str, size_t& offset) {
            JSON Array(JSON::Class::Array);
            size_t index = 0;

            ++offset;
            consume_ws(str, offset);
            if (str.at(offset) == ']') {
                ++offset;
                return Array;
            }

            for (; offset < str.size();) {
                Array[index++] = parse_next(str, offset);
                consume_ws(str, offset);

                if (str.at(offset) == ',') {
                    ++offset;
                    continue;
                }
                else if (str.at(offset) == ']') {
                    ++offset;
                    break;
                }
                else {
                    throw std::runtime_error(std::string("JSON ERROR: Array: Expected ',' or ']', found '") + str.at(offset) + "'\n");
                }
            }

            return Array;
        }

        static JSON parse_string(const std::string& str, size_t& offset) {
            std::string val;
            for (char c = str.at(++offset); c != '\"'; c = str.at(++offset)) {
                if (c == '\\') {
                    switch (str.at(++offset)) {
                    case '\"':
                        val += '\"';
                        break;
                    case '\\':
                        val += '\\';
                        break;
                    case '/':
                        val += '/';
                        break;
                    case 'b':
                        val += '\b';
                        break;
                    case 'f':
                        val += '\f';
                        break;
                    case 'n':
                        val += '\n';
                        break;
                    case 'r':
                        val += '\r';
                        break;
                    case 't':
                        val += '\t';
                        break;
                    case 'u': {
                        val += "\\u";
                        for (size_t i = 1; i <= 4; ++i) {
                            c = str.at(offset + i);
                            if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                                val += c;
                            }
                            else {
                                throw std::runtime_error(
                                    std::string("JSON ERROR: String: Expected hex character in unicode escape, found '") + c + "'");
                            }
                        }
                        offset += 4;
                    } break;
                    default:
                        val += '\\';
                        break;
                    }
                }
                else {
                    val += c;
                }
            }
            ++offset;
            return JSON(val);
        }

        static JSON parse_number(const std::string& str, size_t& offset) {
            std::string val, exp_str;
            char c = '\0';
            bool isDouble = false;
            bool isNegative = false;
            std::int64_t exp = 0;
            bool isExpNegative = false;
            if (offset < str.size() && str.at(offset) == '-') {
                isNegative = true;
                ++offset;
            }
            for (; offset < str.size();) {
                c = str.at(offset++);
                if (c >= '0' && c <= '9') {
                    val += c;
                }
                else if (c == '.' && !isDouble) {
                    val += c;
                    isDouble = true;
                }
                else {
                    break;
                }
            }
            if (offset < str.size() && (c == 'E' || c == 'e')) {
                c = str.at(offset++);
                if (c == '-') {
                    isExpNegative = true;
                }
                else if (c == '+') {
                    // do nothing
                }
                else {
                    --offset;
                }

                for (; offset < str.size();) {
                    c = str.at(offset++);
                    if (c >= '0' && c <= '9') {
                        exp_str += c;
                    }
                    else if (!isspace(c) && c != ',' && c != ']' && c != '}') {
                        throw std::runtime_error(std::string("JSON ERROR: Number: Expected a number for exponent, found '") + c + "'");
                    }
                    else {
                        break;
                    }
                }
                exp = chaiscript::parse_num<std::int64_t>(exp_str) * (isExpNegative ? -1 : 1);
            }
            else if (offset < str.size() && (!isspace(c) && c != ',' && c != ']' && c != '}')) {
                throw std::runtime_error(std::string("JSON ERROR: Number: unexpected character '") + c + "'");
            }
            --offset;

            if (isDouble) {
                return JSON((isNegative ? -1 : 1) * chaiscript::parse_num<double>(val) * std::pow(10, exp));
            }
            else {
                if (!exp_str.empty()) {
                    return JSON((isNegative ? -1 : 1) * static_cast<double>(chaiscript::parse_num<std::int64_t>(val)) * std::pow(10, exp));
                }
                else {
                    return JSON((isNegative ? -1 : 1) * chaiscript::parse_num<std::int64_t>(val));
                }
            }
        }

        static JSON parse_bool(const std::string& str, size_t& offset) {
            if (str.substr(offset, 4) == "true") {
                offset += 4;
                return JSON(true);
            }
            else if (str.substr(offset, 5) == "false") {
                offset += 5;
                return JSON(false);
            }
            else {
                throw std::runtime_error(std::string("JSON ERROR: Bool: Expected 'true' or 'false', found '") + str.substr(offset, 5) + "'");
            }
        }

        static JSON parse_null(const std::string& str, size_t& offset) {
            if (str.substr(offset, 4) != "null") {
                throw std::runtime_error(std::string("JSON ERROR: Null: Expected 'null', found '") + str.substr(offset, 4) + "'");
            }
            offset += 4;
            return JSON();
        }

        static JSON parse_next(const std::string& str, size_t& offset) {
            char value;
            consume_ws(str, offset);
            value = str.at(offset);
            switch (value) {
            case '[':
                return parse_array(str, offset);
            case '{':
                return parse_object(str, offset);
            case '\"':
                return parse_string(str, offset);
            case 't':
            case 'f':
                return parse_bool(str, offset);
            case 'n':
                return parse_null(str, offset);
            default:
                if ((value <= '9' && value >= '0') || value == '-') {
                    return parse_number(str, offset);
                }
            }
            throw std::runtime_error(std::string("JSON ERROR: Parse: Unexpected starting character '") + value + "'");
        }
    };
    inline JSON JSON::Load(const std::string& str) {
        size_t offset = 0;
        return JSONParser::parse_next(str, offset);
    }
} // namespace chaiscript::json
namespace chaiscript {
    class json_wrap {
    public:
        static Module& library(Module& m) {
            m.add(chaiscript::fun([](const std::string& t_str) { return from_json(t_str); }), "from_json");
            m.add(chaiscript::fun(&json_wrap::to_json), "to_json");

            return m;
        }

        static Boxed_Value from_json(const std::string& t_json) {
            try {
                return from_json(json::JSON::Load(t_json));
            }
            catch (const std::out_of_range&) {
                throw std::runtime_error("Unparsed JSON input");
            }
        }

    private:
        static Boxed_Value from_json(const json::JSON& t_json) {
            switch (t_json.JSONType()) {
            case json::JSON::Class::Null:
                return Boxed_Value();
            case json::JSON::Class::Object: {
                std::map<std::string, Boxed_Value> m;

                for (auto& p : t_json.object_range()) {
#ifdef UseCweeThreadedMapAsQuickFlatMap
                    m.insert(std::make_pair(p.first, from_json(*p.second)));
#else
                    m.insert(std::make_pair(p.first, from_json(p.second)));
#endif
                }

                return Boxed_Value(m);
            }
            case json::JSON::Class::Array: {
                std::vector<Boxed_Value> vec;

                for (const auto& p : t_json.array_range()) {
                    vec.emplace_back(from_json(p));
                }

                return Boxed_Value(vec);
            }
            case json::JSON::Class::String:
                return Boxed_Value(t_json.to_string());
            case json::JSON::Class::Floating:
                return Boxed_Value(t_json.to_float());
            case json::JSON::Class::Integral:
                return Boxed_Value(t_json.to_int());
            case json::JSON::Class::Boolean:
                return Boxed_Value(t_json.to_bool());
            }

            throw std::runtime_error("Unknown JSON type");
        }

        static std::string to_json(const Boxed_Value& t_bv) { return to_json_object(t_bv).dump(); }

        static json::JSON to_json_object(const Boxed_Value& t_bv) {
            try {
                const std::map<std::string, Boxed_Value>& m = chaiscript::boxed_cast<const std::map<std::string, Boxed_Value>&>(t_bv);

                json::JSON obj(json::JSON::Class::Object);
                for (const auto& o : m) {
                    obj[o.first] = to_json_object(o.second);
                }
                return obj;
            }
            catch (const chaiscript::exception::bad_boxed_cast&) {
                // not a map
            }

            try {
                const std::vector<Boxed_Value>& v = chaiscript::boxed_cast<const std::vector<Boxed_Value>&>(t_bv);

                json::JSON obj(json::JSON::Class::Array);
                for (size_t i = 0; i < v.size(); ++i) {
                    obj[i] = to_json_object(v[i]);
                }
                return obj;
            }
            catch (const chaiscript::exception::bad_boxed_cast&) {
                // not a vector
            }

            try {
                const cweeThreadedList<Boxed_Value>& v = chaiscript::boxed_cast<const cweeThreadedList<Boxed_Value>&>(t_bv);

                json::JSON obj(json::JSON::Class::Array);
                for (size_t i = 0; i < v.size(); ++i) {
                    obj[i] = to_json_object(v[i]);
                }
                return obj;
            }
            catch (const chaiscript::exception::bad_boxed_cast&) {
                // not a cweeVector
            }

            try {
                Boxed_Number bn(t_bv);
                if (Boxed_Number::is_floating_point(t_bv)) {
                    return json::JSON(bn.get_as<double>());
                }
                else {
                    return json::JSON(bn.get_as<std::int64_t>());
                }
            }
            catch (const chaiscript::detail::exception::bad_any_cast&) {
                // not a number
            }

            try {
                return json::JSON(boxed_cast<bool>(t_bv));
            }
            catch (const chaiscript::exception::bad_boxed_cast&) {
                // not a bool
            }

            try {
                return json::JSON(boxed_cast<std::string>(t_bv));
            }
            catch (const chaiscript::exception::bad_boxed_cast&) {
                // not a string
            }

            try {
                return json::JSON(std::string(boxed_cast<cweeStr>(t_bv).c_str()));
            }
            catch (const chaiscript::exception::bad_boxed_cast&) {
                // not a cweeStr
            }

            try {
                const chaiscript::dispatch::Dynamic_Object& o = boxed_cast<const dispatch::Dynamic_Object&>(t_bv);

                json::JSON obj(json::JSON::Class::Object);
                for (const auto& attr : o.get_attrs()) {
                    obj[attr.first] = to_json_object(attr.second);
                }
                return obj;
            }
            catch (const chaiscript::exception::bad_boxed_cast&) {
                // not a dynamic object
            }

            if (t_bv.is_null())
                return json::JSON(); // a null value

            throw std::runtime_error("Unknown object type to convert to JSON");
        }
    };
} // namespace chaiscript
