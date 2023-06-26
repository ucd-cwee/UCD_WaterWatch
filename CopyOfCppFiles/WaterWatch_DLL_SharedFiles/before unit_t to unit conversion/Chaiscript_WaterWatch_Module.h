#pragma once
#include "Precompiled.h"

namespace chaiscript {
    class WaterWatch_Lib {
    public:
        [[nodiscard]] static ModulePtr library() {
            auto lib = chaiscript::make_shared<Module>();
            bootstrap::Bootstrap::bootstrap(*lib);

#define AddBasicClassTemplate(Type) { \
                        lib->add(chaiscript::user_type<Type>(), #Type); \
                        lib->add(chaiscript::constructor<Type()>(), #Type); \
                        lib->add(chaiscript::constructor<Type(const Type&)>(), #Type); \
                        lib->add(chaiscript::fun([](Type& a, const Type& b) { a = b; return a; }), "="); \
                    }
#define AddBasicClassMember(Type, Member) { lib->add(chaiscript::fun(&##Type::##Member), #Member); }
#define AddNamespacedClassTemplate(Namespace, Type) { \
                        lib->add(chaiscript::user_type<##Namespace::##Type>(), #Type); \
                        lib->add(chaiscript::constructor<##Namespace::##Type()>(), #Type); \
                        lib->add(chaiscript::constructor<##Namespace::##Type(##Namespace::##Type const &)>(), #Type); \
                        lib->add(chaiscript::fun([](##Namespace::##Type& a, ##Namespace::##Type const& b) { a = b; return a; }), "="); \
                    }
#define AddNamespacedClassMember(Namespace, Type, Member) { lib->add(chaiscript::fun(&##Namespace::##Type::##Member), #Member); }

#define ChaiscriptConstructor(From, To)\
    lib->add(chaiscript::constructor<To(const From&)>(), #To);\
    lib->add(chaiscript::fun([](To& a, const From& b) { a = To(b); return a; }), "=");\
    lib->add(chaiscript::type_conversion<From, To>([](const From& t_bt)->To { return To(t_bt); }, nullptr));


            // BASIC COUT 
            if (1) {
                lib->add(chaiscript::fun([](const std::string& str) { std::cout << str << std::endl; }), "cout");
                lib->add(chaiscript::fun([](const cweeStr& str) { std::cout << str << std::endl; }), "cout");
            }

            // CLOCK
            if (1) {
                lib->add(chaiscript::fun(clock_s), "clock_s");
                lib->add(chaiscript::fun(clock_ms), "clock_ms");
                lib->add(chaiscript::fun(clock_us), "clock_us");
                lib->add(chaiscript::fun(clock_ns), "clock_ns");
                
                AddBasicClassTemplate(Stopwatch);
                lib->add(chaiscript::fun([](Stopwatch const& a) { return std::string(cweeStr(a.Seconds_Passed()) + "s"); }), "to_string");
                lib->add(chaiscript::fun(&Stopwatch::Start), "Start");
                lib->add(chaiscript::fun(&Stopwatch::Stop), "Stop");
                lib->add(chaiscript::fun(&Stopwatch::Seconds_Passed), "Seconds_Passed");
                lib->add(chaiscript::fun(&Stopwatch::Nanoseconds_Passed), "Nanoseconds_Passed");
            }

            // cweeStr
            if (1) {
                lib->add(chaiscript::user_type<cweeStr>(), "cweeStr");
                lib->add(chaiscript::constructor<cweeStr()>(), "cweeStr");
                ChaiscriptConstructor(bool, cweeStr);
                ChaiscriptConstructor(char, cweeStr);
                ChaiscriptConstructor(int, cweeStr);
                ChaiscriptConstructor(unsigned, cweeStr);
                ChaiscriptConstructor(float, cweeStr);
                ChaiscriptConstructor(double, cweeStr);
                ChaiscriptConstructor(time_t, cweeStr);
                ChaiscriptConstructor(u64, cweeStr);
                ChaiscriptConstructor(cweeStr, cweeStr);
                ChaiscriptConstructor(std::string, cweeStr);

                lib->add(chaiscript::type_conversion<cweeStr, std::string>([](const cweeStr& t_bt)->std::string { return t_bt.c_str(); }, nullptr));
                // lib->add(chaiscript::type_conversion(chaiscript::user_type<cweeStr>(), chaiscript::user_type<std::string>(), [](const chaiscript::Boxed_Value& t_bt) -> std::string { return chaiscript::boxed_cast<cweeStr>(t_bt).c_str(); }));
                lib->add(chaiscript::fun([](std::string& a, const cweeStr& b) { a = b.c_str(); return a; }), "=");

                lib->add(chaiscript::fun([](const cweeStr& a, const cweeStr& b) { return a + b; }), "+");
                lib->add(chaiscript::fun([](const cweeStr& a, const cweeStr& b) { return a < b; }), "<");
                lib->add(chaiscript::fun([](cweeStr& a, const cweeStr& b) { a += b; return a;  }), "+=");
                lib->add(chaiscript::fun([](const cweeStr& a, const cweeStr& b) { return a == b; }), "==");
                lib->add(chaiscript::fun([](const cweeStr& a, const cweeStr& b) { return a != b; }), "!=");

                lib->add(chaiscript::fun(&cweeStr::RemoveBetween), "RemoveBetween");
                lib->add(chaiscript::fun(&cweeStr::ReplaceBetween), "ReplaceBetween");
                lib->add(chaiscript::fun(&cweeStr::FindBetween), "FindBetween");
                lib->add(chaiscript::fun(&cweeStr::FindFirstBetween), "FindFirstBetween");
                lib->add(chaiscript::fun([](const cweeStr& a) { return a.Length(); }), "Length");
                lib->add(chaiscript::fun(&cweeStr::Empty), "Empty");
                lib->add(chaiscript::fun(&cweeStr::IsEmpty), "IsEmpty");
                lib->add(chaiscript::fun(&cweeStr::Clear), "Clear");
                lib->add(chaiscript::fun([](cweeStr& a) { a.ToLower(); return a; }), "ToLower");
                lib->add(chaiscript::fun([](cweeStr& a) { a.ToUpper(); return a; }), "ToUpper");
                lib->add(chaiscript::fun([](cweeStr& a) { return a.IsNumeric(); }), "IsNumeric");
                lib->add(chaiscript::fun([](cweeStr& a, cweeStr const& b) { return a.Find(b, true); }), "Find");
                lib->add(chaiscript::fun([](cweeStr& a, cweeStr const& b) { return a.Find(b, false); }), "iFind");
                lib->add(chaiscript::fun([](cweeStr& a, cweeStr const& b) { return a.rFind(b, true); }), "rFind");
                lib->add(chaiscript::fun([](cweeStr& a, cweeStr const& b) { return a.rFind(b, false); }), "riFind");
                lib->add(chaiscript::fun([](cweeStr& a, const int& N) { return a.Left(N); }), "Left");
                lib->add(chaiscript::fun([](cweeStr& a, const int& N) { return a.Right(N); }), "Right");
                lib->add(chaiscript::fun([](cweeStr& a, const int& start, const int& N) { return a.Mid(start, N); }), "Mid");
                lib->add(chaiscript::fun(&cweeStr::ReduceSpaces), "ReduceSpaces");
                lib->add(chaiscript::fun([](cweeStr& a, cweeStr const& b, cweeStr const& c) { a.Replace(b, c); return a; }), "Replace");
                lib->add(chaiscript::fun(&cweeStr::ReturnNumeric), "ReturnNumeric");

                lib->add(chaiscript::fun([](cweeStr& a, cweeStr const& b, cweeStr const& c) { return a.AddToDelimiter(b, c); }), "AddToDelimiter");
                lib->add(chaiscript::fun([](cweeStr& a) { return cweeStr::Hash(a); }), "Hash");

                lib->add(chaiscript::fun(&cweeStr::Levenshtein), "Levenshtein");
                lib->add(chaiscript::fun([](cweeStr const& a) { return std::string(a.c_str()); }), "to_string");
            }

            // cweeTime
            if (1) {
                lib->add(chaiscript::user_type<cweeTime>(), "cweeTime");
                lib->add(chaiscript::constructor<cweeTime()>(), "cweeTime");
                lib->add(chaiscript::constructor<cweeTime(const cweeTime&)>(), "cweeTime");
                lib->add(chaiscript::constructor<cweeTime(const u64&)>(), "cweeTime");
                lib->add(chaiscript::constructor<cweeTime(const cweeStr&)>(), "cweeTime");
                lib->add(chaiscript::fun([](const cweeTime& a) { return cweeStr(a.c_str()); }), "c_str");
                lib->add(chaiscript::constructor<u64(cweeTime)>(), "u64");
                //lib->add(chaiscript::constructor<double(cweeTime)>(), "double");
                //lib->add(chaiscript::constructor<float(cweeTime)>(), "float");
                //lib->add(chaiscript::constructor<int(cweeTime)>(), "int");

                lib->add(chaiscript::fun([](cweeTime& a, const cweeTime& b) { a = b; return a; }), "=");
                lib->add(chaiscript::fun([](cweeTime& a, const cweeStr& b) { a = cweeTime(b); return a; }), "=");
                lib->add(chaiscript::fun([](const cweeTime& a, const cweeTime& b) { return a == b; }), "==");
                lib->add(chaiscript::fun([](const cweeTime& a, const cweeTime& b) { return a != b; }), "!=");
                lib->add(chaiscript::fun([](const cweeTime& a, const cweeTime& b) { return a > b; }), ">");
                lib->add(chaiscript::fun([](const cweeTime& a, const cweeTime& b) { return a < b; }), "<");
                lib->add(chaiscript::fun([](const cweeTime& a, const cweeTime& b) { return a >= b; }), ">=");
                lib->add(chaiscript::fun([](const cweeTime& a, const cweeTime& b) { return a <= b; }), "<=");
                lib->add(chaiscript::fun([](const cweeTime& a, const cweeTime& b) { return a + b; }), "+");
                lib->add(chaiscript::fun([](const cweeTime& a, const cweeTime& b) { return a - b; }), "-");
                lib->add(chaiscript::fun([](cweeTime& a, const cweeTime& b) { return a += b; }), "+=");
                lib->add(chaiscript::fun([](cweeTime& a, const cweeTime& b) { return a -= b; }), "-=");

                lib->add(chaiscript::fun(&cweeTime::ToStartOfMonth), "ToStartOfMonth");
                lib->add(chaiscript::fun(&cweeTime::ToStartOfDay), "ToStartOfDay");
                lib->add(chaiscript::fun(&cweeTime::ToStartOfHour), "ToStartOfHour");
                lib->add(chaiscript::fun(&cweeTime::ToStartOfMinute), "ToStartOfMinute");

                lib->add(chaiscript::fun(&cweeTime::ToEndOfMonth), "ToEndOfMonth");
                lib->add(chaiscript::fun(&cweeTime::ToEndOfDay), "ToEndOfDay");
                lib->add(chaiscript::fun(&cweeTime::ToEndOfHour), "ToEndOfHour");
                lib->add(chaiscript::fun(&cweeTime::ToEndOfMinute), "ToEndOfMinute");

                lib->add(chaiscript::fun(&cweeTime::tm_fractionalsec), "tm_fractionalsec");
                lib->add(chaiscript::fun(&cweeTime::tm_sec), "tm_sec");
                lib->add(chaiscript::fun(&cweeTime::tm_min), "tm_min");
                lib->add(chaiscript::fun(&cweeTime::tm_hour), "tm_hour");
                lib->add(chaiscript::fun(&cweeTime::tm_mday), "tm_mday");
                lib->add(chaiscript::fun(&cweeTime::tm_mon), "tm_mon");
                lib->add(chaiscript::fun(&cweeTime::tm_year), "tm_year");
                lib->add(chaiscript::fun(&cweeTime::tm_wday), "tm_wday");
                lib->add(chaiscript::fun(&cweeTime::tm_yday), "tm_yday");
                lib->add(chaiscript::fun(&cweeTime::getNumDaysInSameMonth), "getNumDaysInSameMonth");
                lib->add(chaiscript::fun(&cweeTime::make_time), "make_time");
                
                lib->add(chaiscript::fun([](int year, int month, int day) { return cweeTime::make_time(year,month,day); }), "cweeTime");
                lib->add(chaiscript::fun([](int year, int month, int day, int hour) { return cweeTime::make_time(year, month, day, hour); }), "cweeTime");
                lib->add(chaiscript::fun([](int year, int month, int day, int hour, int minute) { return cweeTime::make_time(year, month, day, hour, minute); }), "cweeTime");
                lib->add(chaiscript::fun([](int year, int month, int day, int hour, int minute, int second) {return cweeTime::make_time(year, month, day, hour, minute, second); }), "cweeTime");

                lib->add(chaiscript::fun([](cweeTime const& a) { return std::string(a.c_str()); }), "to_string");
                lib->add(chaiscript::fun([]() { return cweeTime::Epoch(); }), "Epoch");
            }

            bootstrap::standard_library::vector_type<cweeList<Boxed_Value>>("cweeList", *lib);
            
            // cweeParser
            if (1) {
                lib->add(chaiscript::fun([](const cweeStr& a, const cweeStr& b) { auto c = a.Split(b); cweeList<Boxed_Value> out; for (auto& x : c) { out.Append(Boxed_Value(cweeStr(x))); } return out; }), "Split");
            }

            // Toasts
            if (1) {
                lib->add(chaiscript::fun(cweeToasts::submitToast), "submitToast");
            }

            // Math
            if (1) {
                // Random
                {
                    lib->add(chaiscript::fun([]() { return cweeRandomFloat(); }), "cweeRandomFloat");
                    lib->add(chaiscript::fun([](double a) { return cweeRandomFloat(a); }), "cweeRandomFloat");
                    lib->add(chaiscript::fun([](double a, double b) { return cweeRandomFloat(a,b); }), "cweeRandomFloat");
                }
                // Basic Math
                {
                    lib->add(chaiscript::fun(cweeMath::RSqrt), "RSqrt");
                    lib->add(chaiscript::fun(cweeMath::Sqrt), "Sqrt");
                    lib->add(chaiscript::fun(cweeMath::Pow), "Pow");
                    lib->add(chaiscript::fun(cweeMath::Rint), "Rint");
                    lib->add(chaiscript::fun(cweeMath::Ceil), "Ceil");
                    lib->add(chaiscript::fun(cweeMath::Floor), "Floor");
                    lib->add(chaiscript::fun(cweeMath::Abs), "Abs");
                    lib->add(chaiscript::fun(cweeMath::Fabs), "Fabs");
                    lib->add(chaiscript::fun(cweeMath::Frac), "Frac");
                    lib->add(chaiscript::fun(cweeMath::Fmin), "Min");
                    lib->add(chaiscript::fun(cweeMath::Fmax), "Max");
                    lib->add(chaiscript::fun([](double a) { return ::tan(a); }), "tan");
                    lib->add(chaiscript::fun([](double a) { return ::sin(a); }), "sin");
                    lib->add(chaiscript::fun([](double a) { return ::cos(a); }), "cos");
                    lib->add(chaiscript::fun([](double a) { return ::atan(a); }), "atan");
                    lib->add(chaiscript::fun([](double a) { return ::asin(a); }), "asin");
                    lib->add(chaiscript::fun([](double a) { return ::acos(a); }), "acos");
                    lib->add(chaiscript::fun([](double a) { return ::sqrt(a); }), "sqrt");
                    lib->add(chaiscript::fun([](double a) { return ::log(a); }), "log");
                    lib->add(chaiscript::fun([](float a) { return cweeMath::roundNearest(a, 1); }), "Round");
                    lib->add(chaiscript::fun([](float a, float b) { return cweeMath::roundNearest(a, b); }), "Round");
                    lib->add(chaiscript::fun([](float a) { return cweeMath::roundDownNearest(a, 1); }), "RoundDown");
                    lib->add(chaiscript::fun([](float a, float b) { return cweeMath::roundDownNearest(a, b); }), "RoundDown");
                    lib->add(chaiscript::fun([](float a, float b, float c) { return cweeMath::Lerp(a, b, c); }), "Lerp");
                    lib->add(chaiscript::fun([](double& currentAverage, const double& newSample, int& numSamples) { cweeMath::rollingAverageRef<double>(currentAverage, newSample, numSamples); return currentAverage; }), "RollingAverage");

                    lib->add(chaiscript::fun(cweeMath::roundNearest), "roundNearest");
                    lib->add(chaiscript::fun(cweeMath::roundDownNearest), "roundDownNearest");
                }

                lib->add_global_const(chaiscript::const_var(cweeMath::PI), "PI");
                lib->add_global_const(chaiscript::const_var(cweeMath::INF), "INF");
                lib->add_global_const(chaiscript::const_var(cweeMath::AngRad), "AngRad");
            }

            // unit_value?
            if (1) {
                // attempt 2
                using namespace cweeUnitValues;
                using namespace chaiscript;

                lib->add(user_type<unit_value>(),"value");
                lib->add(constructor<unit_value()>(),"value");
                lib->add(constructor<unit_value(const unit_value&)>(),"value");
                lib->add(fun([](unit_value& a, const unit_value& b) { a = b; return a; }), "=");

                // Numbers to unit_value
                lib->add(constructor<unit_value(int)>(),"value");
                lib->add(constructor<unit_value(double)>(),"value");
                lib->add(constructor<unit_value(float)>(),"value");
                lib->add(constructor<unit_value(u64)>(),"value");
                lib->add(constructor<unit_value(long)>(),"value");
                lib->add(type_conversion<int, unit_value>([](const int& a)->unit_value { return a; }, nullptr));
                lib->add(type_conversion<double, unit_value>([](const double& a)->unit_value { return a; }, nullptr));
                lib->add(type_conversion<float, unit_value>([](const float& a)->unit_value { return a; }, nullptr));
                lib->add(type_conversion<u64, unit_value>([](const u64& a)->unit_value { return a; }, nullptr));
                lib->add(type_conversion<long, unit_value>([](const long& a)->unit_value { return a; }, nullptr));
                lib->add(chaiscript::fun([](int const& a) -> unit_value { return a; }),"value");
                lib->add(chaiscript::fun([](double const& a) -> unit_value { return a; }),"value");
                lib->add(chaiscript::fun([](float const& a) -> unit_value { return a; }),"value");
                lib->add(chaiscript::fun([](u64 const& a) -> unit_value { return a; }),"value");
                lib->add(chaiscript::fun([](long const& a) -> unit_value { return a; }),"value");

                lib->add(fun([](unit_value& a, const int& b) { a = b; return a; }), "=");
                lib->add(fun([](unit_value& a, const double& b) { a = b; return a; }), "=");
                lib->add(fun([](unit_value& a, const float& b) { a = b; return a; }), "=");
                lib->add(fun([](unit_value& a, const u64& b) { a = b; return a; }), "=");
                lib->add(fun([](unit_value& a, const long& b) { a = b; return a; }), "=");

                // unit_value to Numbers
                lib->add(type_conversion<unit_value, int>([](const unit_value& a)->int { return a(); }, nullptr));
                lib->add(type_conversion<unit_value, double>([](const unit_value& a)->double { return a(); }, nullptr));
                lib->add(type_conversion<unit_value, float>([](const unit_value& a)->float { return a(); }, nullptr));
                lib->add(type_conversion<unit_value, u64>([](const unit_value& a)->u64 { return a(); }, nullptr));
                lib->add(type_conversion<unit_value, long>([](const unit_value& a)->long { return a(); }, nullptr));
                lib->add(chaiscript::fun([](unit_value const& a) -> int { return a(); }), "int");
                lib->add(chaiscript::fun([](unit_value const& a) -> double { return a(); }), "double");
                lib->add(chaiscript::fun([](unit_value const& a) -> float { return a(); }), "float");
                lib->add(chaiscript::fun([](unit_value const& a) -> u64 { return a(); }), "u64");
                lib->add(chaiscript::fun([](unit_value const& a) -> long { return a(); }), "long");

                lib->add(fun([](int& a, const unit_value& b) { a = b(); return a; }), "=");
                lib->add(fun([](double& a, const unit_value& b) { a = b(); return a; }), "=");
                lib->add(fun([](float& a, const unit_value& b) { a = b(); return a; }), "=");
                lib->add(fun([](u64& a, const unit_value& b) { a = b(); return a; }), "=");
                lib->add(fun([](long& a, const unit_value& b) { a = b(); return a; }), "=");

                // operators 
                lib->add(chaiscript::fun([](const unit_value& a, const unit_value& b) { return a.pow(b); }), "^");
                lib->add(chaiscript::fun([](unit_value& a, const unit_value& b) { return a.pow_value(b); }), "^=");
                lib->add(chaiscript::fun(&unit_value::Add), "+");
                lib->add(chaiscript::fun([](unit_value& a, const unit_value& b) { return a += b; }), "+=");
                lib->add(chaiscript::fun(&unit_value::Sub), "-");
                lib->add(chaiscript::fun([](unit_value& a, const unit_value& b) { return a -= b; }), "-=");
                lib->add(chaiscript::fun([](const unit_value& a, const unit_value& b) { return a * b; }), "*");
                lib->add(chaiscript::fun([](unit_value& a, const unit_value& b) { return a *= b; }), "*=");
                lib->add(chaiscript::fun([](const unit_value& a, const unit_value& b) { return a / b; }), "/");
                lib->add(chaiscript::fun([](unit_value& a, const unit_value& b) { return a /= b; }), "/=");
                lib->add(chaiscript::fun([](const unit_value& a) -> cweeStr { return a.ToString(); }), "to_string");

                // Comparators
                lib->add(fun([](unit_value& a, const unit_value& b)->bool {return a == b; }), "==");
                lib->add(fun([](unit_value& a, const unit_value& b)->bool {return a < b; }), "<");
                lib->add(fun([](unit_value& a, const unit_value& b)->bool {return a > b; }), ">");
                lib->add(fun([](unit_value& a, const unit_value& b)->bool {return a >= b; }), ">=");
                lib->add(fun([](unit_value& a, const unit_value& b)->bool {return a <= b; }), "<=");
                lib->add(fun([](unit_value& a, const unit_value& b)->bool {return a != b; }), "!=");

                // Other Operators
                lib->add(fun([](unit_value& a) { a--; return a; }), "--");
                lib->add(fun([](unit_value& a) { a++; return a; }), "++");
                lib->add(fun([](unit_value const& a, unit_value const& b) -> std::vector<Boxed_Value> {
                    std::vector<Boxed_Value> out;
                    out.push_back(Boxed_Value(a));
                    if (b > a) {
                        for (unit_value c = a + 1; c < b; c++) {
                            out.push_back(Boxed_Value(c));
                        }
                        out.push_back(Boxed_Value(b));
                    }
                    else if (b < a) {
                        for (unit_value c = a - 1; c > b; c--) {
                            out.push_back(Boxed_Value(c));
                        }
                        out.push_back(Boxed_Value(b));
                    }
                    return out;
                }), "..");

                // Functions
                lib->add(fun([](const unit_value& a)->std::string { return a.Abbreviation(); }), "Abbreviation");
                lib->add(fun([](const unit_value& a)->cweeStr { return a.ToString(); }), "ToString");

                // cout
                lib->add(chaiscript::fun([](const unit_value& str) { std::cout << str << std::endl; }), "cout");
                
                // Derived unit types
                {
//#define AddUnit(Type)\
//                    lib->add(user_type<Type>(),#Type); \
//                    lib->add(chaiscript::base_class<unit_value, Type>()); \
//                    lib->add(constructor<Type()>(), #Type); \
//                    lib->add(constructor<Type(const unit_value&)>(), #Type);

#define AddUnit(Type)\
                    lib->add(chaiscript::fun([]() -> unit_value { static Type F;  return F; }), #Type); \
                    lib->add(chaiscript::fun([](unit_value const& a) -> unit_value {  Type F; F = a; return F; }), #Type);

#define AddUnitWithMetricPrefixes(Type)\
	                AddUnit(Type); \
	                AddUnit(femto ## Type); \
	                AddUnit(pico ## Type); \
	                AddUnit(nano ## Type); \
	                AddUnit(micro ## Type); \
	                AddUnit(milli ## Type); \
	                AddUnit(centi ## Type); \
	                AddUnit(deci ## Type); \
	                AddUnit(deca ## Type); \
	                AddUnit(hecto ## Type); \
	                AddUnit(kilo ## Type); \
	                AddUnit(mega ## Type); \
	                AddUnit(giga ## Type); \
	                AddUnit(tera ## Type); \
	                AddUnit(peta ## Type)
                    
                    AddUnitWithMetricPrefixes(meter);
                    AddUnit(foot);
                    AddUnit(inch);
                    AddUnit(mile);
                    AddUnit(nauticalMile);
                    AddUnit(astronicalUnit);
                    AddUnit(yard);
                    AddUnitWithMetricPrefixes(gram);
                    AddUnit(metric_ton);
                    AddUnit(pound);
                    AddUnit(long_ton);
                    AddUnit(short_ton);
                    AddUnit(stone);
                    AddUnit(ounce);
                    AddUnit(carat);
                    AddUnit(slug);
                    AddUnitWithMetricPrefixes(second);
                    AddUnit(minute);
                    AddUnit(hour);
                    AddUnit(day);
                    AddUnit(week);
                    AddUnit(year);
                    AddUnit(julian_year);
                    AddUnit(gregorian_year);
                    AddUnitWithMetricPrefixes(ampere);
                    AddUnit(Dollar);
                    AddUnit(MillionDollar);
                    AddUnitWithMetricPrefixes(hertz);
                    AddUnit(meters_per_second);
                    AddUnit(feet_per_second);
                    AddUnit(feet_per_minute);
                    AddUnit(feet_per_hour);
                    AddUnit(miles_per_hour);
                    AddUnit(kilometers_per_hour);
                    AddUnit(knot);
                    AddUnit(meters_per_second_squared);
                    AddUnit(feet_per_second_squared);
                    AddUnit(standard_gravity);
                    AddUnitWithMetricPrefixes(newton);
                    AddUnitWithMetricPrefixes(pound_f);
                    AddUnit(dyne);
                    AddUnit(kilopond);
                    AddUnit(poundal);
                    AddUnitWithMetricPrefixes(pascals);
                    AddUnitWithMetricPrefixes(bar);
                    AddUnit(mbar);
                    AddUnit(atmosphere);
                    AddUnit(pounds_per_square_inch);
                    AddUnit(head);
                    AddUnit(torr);
                    AddUnitWithMetricPrefixes(coulomb);
                    AddUnitWithMetricPrefixes(ampere_hour);
                    AddUnitWithMetricPrefixes(watt);
                    AddUnit(horsepower);
                    AddUnitWithMetricPrefixes(joule);
                    AddUnitWithMetricPrefixes(calorie);
                    AddUnitWithMetricPrefixes(watt_minute);
                    AddUnitWithMetricPrefixes(watt_hour);
                    AddUnit(watt_day);
                    AddUnit(british_thermal_unit);
                    AddUnit(british_thermal_unit_iso);
                    AddUnit(british_thermal_unit_59);
                    AddUnit(therm);
                    AddUnit(foot_pound);
                    AddUnitWithMetricPrefixes(volt);
                    AddUnitWithMetricPrefixes(ohm);
                    AddUnitWithMetricPrefixes(siemens);
                    AddUnit(square_meter);
                    AddUnit(square_foot);
                    AddUnit(square_inch);
                    AddUnit(square_mile);
                    AddUnit(square_kilometer);
                    AddUnit(hectare);
                    AddUnit(acre);
                    AddUnit(cubic_meter);
                    AddUnit(cubic_millimeter);
                    AddUnit(cubic_kilometer);
                    AddUnitWithMetricPrefixes(liter);
                    AddUnit(cubic_inch);
                    AddUnit(cubic_foot);
                    AddUnit(cubic_yard);
                    AddUnit(cubic_mile);
                    AddUnitWithMetricPrefixes(gallon);
                    AddUnit(imperial_gallon);
                    AddUnit(million_gallon);
                    AddUnit(imperial_million_gallon);
                    AddUnit(acre_foot);
                    AddUnit(quart);
                    AddUnit(pint);
                    AddUnit(cup);
                    AddUnit(fluid_ounce);
                    AddUnit(barrel);
                    AddUnit(bushel);
                    AddUnit(cord);
                    AddUnit(tablespoon);
                    AddUnit(teaspoon);
                    AddUnit(pinch);
                    AddUnit(dash);
                    AddUnit(drop);
                    AddUnit(fifth);
                    AddUnit(dram);
                    AddUnit(gill);
                    AddUnit(peck);
                    AddUnit(sack);
                    AddUnit(shot);
                    AddUnit(strike);
                    AddUnitWithMetricPrefixes(gram_per_second);
                    AddUnit(metric_ton_per_second);
                    AddUnit(metric_ton_per_minute);
                    AddUnit(metric_ton_per_hour);
                    AddUnit(metric_ton_per_day);
                    AddUnit(metric_ton_per_year);
                    AddUnit(cubic_meter_per_second);
                    AddUnit(cubic_meter_per_hour);
                    AddUnit(cubic_meter_per_day);
                    AddUnit(cubic_millimeter_per_second);
                    AddUnitWithMetricPrefixes(liter_per_second);
                    AddUnit(liter_per_minute);
                    AddUnit(liter_per_day);
                    AddUnit(megaliter_per_day);
                    AddUnit(cubic_inch_per_second);
                    AddUnit(cubic_inch_per_hour);
                    AddUnit(cubic_foot_per_second);
                    AddUnit(cubic_foot_per_hour);
                    AddUnit(gallon_per_second);
                    AddUnit(gallon_per_minute);
                    AddUnit(gallon_per_hour);
                    AddUnit(gallon_per_day);
                    AddUnit(gallon_per_year);
                    AddUnit(million_gallon_per_second);
                    AddUnit(million_gallon_per_minute);
                    AddUnit(million_gallon_per_hour);
                    AddUnit(million_gallon_per_day);
                    AddUnit(million_gallon_per_year);
                    AddUnit(imperial_million_gallon_per_second);
                    AddUnit(imperial_million_gallon_per_minute);
                    AddUnit(imperial_million_gallon_per_hour);
                    AddUnit(imperial_million_gallon_per_day);
                    AddUnit(imperial_million_gallon_per_year);
                    AddUnit(acre_foot_per_second);
                    AddUnit(acre_foot_per_minute);
                    AddUnit(acre_foot_per_hour);
                    AddUnit(acre_foot_per_day);
                    AddUnit(acre_foot_per_year);
                    AddUnit(kilograms_per_cubic_meter);
                    AddUnit(grams_per_milliliter);
                    AddUnit(kilograms_per_liter);
                    AddUnit(ounces_per_cubic_foot);
                    AddUnit(ounces_per_cubic_inch);
                    AddUnit(ounces_per_gallon);
                    AddUnit(pounds_per_cubic_foot);
                    AddUnit(pounds_per_cubic_inch);
                    AddUnit(pounds_per_gallon);
                    AddUnit(slugs_per_cubic_foot);
                    AddUnit(Dollar_per_joule);
                    AddUnit(Dollar_per_kilowatt_hour);
                    AddUnit(Dollar_per_watt);
                    AddUnit(Dollar_per_kilowatt);
                    AddUnit(Dollar_per_cubic_meter);
                    AddUnit(Dollar_per_gallon);

#undef AddUnitWithMetricPrefixes
#undef AddUnit
                                    
                }
            }

            // Units
            if (1) {
                // lib->add(chaiscript::fun([](const Type& a) { return std::string((cweeStr((double)a) + units::abbreviation<Type>(a)).c_str()); }), "to_string"); 
#define AddUnit_t(Type)     { using namespace cwee_units;   \
                lib->add(chaiscript::user_type<Type>(), #Type);  \
                lib->add(chaiscript::constructor<Type()>(), #Type);  \
                lib->add(chaiscript::constructor<Type(const Type&)>(), #Type);   \
                lib->add(chaiscript::fun([](Type& a, const Type& b) { a = b; return a; }), "=");   \
                lib->add(chaiscript::constructor<Type(double)>(), #Type);   \
                lib->add(chaiscript::constructor<Type(float)>(), #Type);   \
                lib->add(chaiscript::constructor<Type(int)>(), #Type);   \
                lib->add(chaiscript::fun([](Type& a, chaiscript::Boxed_Number const& b) { a = b.get_as<double>(); return a; }), "=");   \
                lib->add(chaiscript::constructor<double(const Type&)>(), "double");   \
                lib->add(chaiscript::fun([](chaiscript::Boxed_Number& a, const Type& b) { a = chaiscript::Boxed_Number(b()); return a; }), "=");   \
                lib->add(chaiscript::fun([](const Type& a, const Type& b) { return a + b; }), "+");   \
                lib->add(chaiscript::fun([](const Type& a, const Type& b) { return a - b; }), "-");   \
                lib->add(chaiscript::fun([](Type& a, const Type& b) { return a += b; }), "+=");   \
                lib->add(chaiscript::fun([](Type& a, const Type& b) { return a -= b; }), "-="); \
                lib->add(chaiscript::fun([](const Type& a) { return std::string((cweeStr((double)a)).c_str()); }), "to_string"); \
                }
                
                AddUnit_t(scalar_t); 
                AddUnit_t(inch_t); 
                AddUnit_t(foot_t);
                AddUnit_t(meter_t);
                AddUnit_t(kilowatt_t);
                AddUnit_t(kilowatt_hour_t);
                AddUnit_t(square_foot_t);
                AddUnit_t(gallon_t);
                AddUnit_t(feet_per_second_t);
                AddUnit_t(feet_per_hour_t);
                AddUnit_t(pounds_per_square_inch_t);
                AddUnit_t(cubic_foot_t);
                AddUnit_t(cubic_foot_per_second_t);
                AddUnit_t(second_t);
                AddUnit_t(hour_t);
                AddUnit_t(Dollar_t);
#undef AddUnit_t
            }

            // Engineering
            if (1) {
                lib->add(chaiscript::fun(cweeEng::CentrifugalPumpEnergyDemand_kW), "CentrifugalPumpEnergyDemand_kW");
                lib->add(chaiscript::fun(cweeEng::SurfaceAreaCircle_ft2), "SurfaceAreaCircle_ft2");
                lib->add(chaiscript::fun(cweeEng::VolumeCylinder_gal), "VolumeCylinder_gal");
                lib->add(chaiscript::fun(cweeEng::Cylinder_FlowRate_to_LevelRate_fph), "Cylinder_FlowRate_to_LevelRate_fph");
                lib->add(chaiscript::fun(cweeEng::Cylinder_Volume_to_Level_f), "Cylinder_Volume_to_Level_f");
                lib->add(chaiscript::fun(cweeEng::Head_to_Pressure_psi), "Head_to_Pressure_psi");
            }

            // Geocoding
            if (1) {
                lib->add(chaiscript::fun([](double X, double Y) { return geocoding->GetElevation(vec2d(X,Y)); }), "GetElevation");
                lib->add(chaiscript::fun([](cweeStr const& address) { return geocoding->GetLongLat(address); }), "GetLongLat");
                lib->add(chaiscript::fun([](double X, double Y) { return geocoding->GetLongLat(X,Y); }), "GetLongLat");
                lib->add(chaiscript::fun([](double X, double Y) { return geocoding->GetAddress(vec2d(X, Y)); }), "GetAddress");
                lib->add(chaiscript::fun([](double X1, double Y1, double X2, double Y2) { return geocoding->Distance(vec2d(X1, Y1), vec2d(X2, Y2)); }), "Distance");
            }

            // cweeJob
            if (1) {
#if 0
                lib->add(chaiscript::fun([](const cweeStr& command) {
                    cweeJob([&](cweeStr& Command) {
                        
                        scripting->DoImmediately(Command, true);
                        }, cweeStr(command.c_str())).AsyncInvoke();
                    }), "Impl_Async");
                lib->add(chaiscript::fun([](const chaiscript::Boxed_Value& command) {
                    scripting->CreateJob(command).AsyncInvoke();
                    }), "Impl_Async");
                lib->eval(R"(
			        def Async(string command){ Impl_Async(command); }
			        def Async(Function func){ Impl_Async(func); }
		        )");
#endif
            }

            // Patterns
            if (1) {
                using namespace cwee_units;

                lib->add(chaiscript::user_type<cweeBalancedPattern<scalar_t>>(), "Pattern");
                lib->add(chaiscript::constructor<cweeBalancedPattern<scalar_t>()>(), "Pattern");
                lib->add(chaiscript::constructor<cweeBalancedPattern<scalar_t>(const cweeBalancedPattern<scalar_t>&)>(), "Pattern");
                // lib->add(chaiscript::constructor<cweeBalancedPattern<scalar_t>(const std::vector<std::pair<u64, float>>&)>(), "Pattern");

                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, cweeBalancedPattern<scalar_t>& b) { a = b; return a; }), "=");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const std::string& b) {
                    a.Clear();
                    cweeParser knots(b.c_str(), ", ", true);
                    cweeParser knotP;
                    for (auto& knot : knots) {
                        knotP.ParseFirstDelimiterOnly(knot, "|");
                        if (knotP.getNumVars() >= 2) {
                            a.AddValue((u64)knotP[0], (float)knotP[1]);
                        }
                    }
                    return a;
                    }), "=");
                lib->add(chaiscript::fun([](std::string& a, const cweeBalancedPattern<scalar_t>& b) {
                    cweeStr out;
                    for (auto& x : b.GetKnotSeries()) {
                        out.AddToDelimiter(cweeStr::printf("%s|%s", cweeStr(x.first).c_str(), cweeStr(x.second()).c_str()), ", ");
                    }
                    a = out.c_str();
                    return a;
                    }), "=");
                
                //chaiscript::type_conversion(chaiscript::user_type<std::string>(), chaiscript::user_type<cweeBalancedPattern<scalar_t>>(), [](const std::string& t_bt) {
                //    // assume we are casting to/from a vector of pairs
                //    cweeBalancedPattern<scalar_t> out;
                //    cweeParser knots(t_bt.c_str(), ", ", true);
                //    cweeParser knotP;
                //    for (auto& knot : knots) {
                //        knotP.ParseFirstDelimiterOnly(knot, "|");
                //        if (knotP.getNumVars() >= 2) {
                //            out.AddValue((u64)knotP[0], (float)knotP[1]);
                //        }
                //    }
                //    return out;
                //});
                //chaiscript::type_conversion(chaiscript::user_type<cweeBalancedPattern<scalar_t>>(), chaiscript::user_type<std::string>(), [](const cweeBalancedPattern<scalar_t>& t_bt) {
                //    cweeStr out;
                //    for (auto& x : t_bt.GetKnotSeries()) {
                //        out.AddToDelimiter(cweeStr::printf("%s|%s", cweeStr(x.first).c_str(), cweeStr(x.second()).c_str()), ", ");
                //    }
                //    return std::string(out.c_str());
                //});

                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const interpolation_t& b) { a.SetInterpolationType(b); }), "SetInterpolationType");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a) { return a.GetInterpolationType(); }), "GetInterpolationType");

                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const boundary_t& b) { a.SetBoundaryType(b); }), "SetBoundaryType");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a) { return a.GetBoundaryType(); }), "GetBoundaryType");

                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const u64& b, const float& c) { return a.AddValue(b, c); }), "AddValue");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const u64& b, const float& c) { return a.AddUniqueValue(b, c); }), "AddUniqueValue");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a) { a.Clear(); }), "Clear");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const u64& b) { return a.GetCurrentValue(b); }), "GetCurrentValue");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a) { return a.GetMinValue(); }), "GetMinValue");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a) { return a.GetAvgValue(); }), "GetAvgValue");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a) { return a.GetMaxValue(); }), "GetMaxValue");

                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const u64& b, const u64& c) { return a.GetMinValue(b, c); }), "GetMinValue");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const u64& b, const u64& c) { return a.GetAvgValue(b, c); }), "GetAvgValue");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const u64& b, const u64& c) { return a.GetMaxValue(b, c); }), "GetMaxValue");

                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a) { return a.GetMinimumTimeStep(); }), "GetMinimumTimeStep");

                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a) { a.RemoveUnnecessaryKnots(); }), "RemoveUnnecessaryKnots");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const float& percentToRemove) { a.ReduceMemory(percentToRemove); }), "ReduceMemory");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const float& min, const float& max) { a.ClampValues(min, max); }), "ClampValues");

                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a) { return a.GetMinTime(); }), "GetMinTime");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a) { return a.GetAvgTime(); }), "GetAvgTime");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a) { return a.GetMaxTime(); }), "GetMaxTime");

                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a) { return a.GetKnotSeries(); }), "GetKnotSeries"); // (std::vector<std::pair<u64, float>>)
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const u64& b) { return a.GetKnotSeries(b); }), "GetKnotSeries"); // (std::vector<std::pair<u64, float>>)
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const u64& b, const u64& c) { return a.GetKnotSeries(b, c); }), "GetKnotSeries"); // (std::vector<std::pair<u64, float>>)
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a) { return a.GetValueKnotSeries(); }), "GetValueKnotSeries"); // (std::vector<scalar_t>)
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const u64& b) { return a.GetValueKnotSeries(b); }), "GetValueKnotSeries"); // (std::vector<scalar_t>)
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const u64& b, const u64& c) { return a.GetValueKnotSeries(b, c); }), "GetValueKnotSeries"); // (std::vector<scalar_t>)
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const u64& b, const u64& c, const u64& d) { return a.GetTimeSeries(b, c, d); }), "GetTimeSeries"); // (std::vector<std::pair<u64, float>>)
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const u64& b, const u64& c, const u64& d) { return a.GetValueTimeSeries(b, c, d); }), "GetValueTimeSeries"); // (std::vector<scalar_t>)

                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a) { return a.GetNumValues(); }), "size");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a) { return a.GetNumValues(); }), "GetNumValues");

                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, int b) { a += (scalar_t)b; return a; }), "+=");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, float b) { a += (scalar_t)b; return a; }), "+=");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const double& b) { a += (scalar_t)b; return a; }), "+=");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, int b) { a -= (scalar_t)b; return a; }), "-=");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, float b) { a -= (scalar_t)b; return a; }), "-=");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const double& b) { a -= (scalar_t)b; return a; }), "-=");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, int b) { a *= (scalar_t)b; return a; }), "*=");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, float b) { a *= (scalar_t)b; return a; }), "*=");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const double& b) { a *= (scalar_t)b; return a; }), "*=");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, int b) { a /= (scalar_t)b; return a; }), "/=");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, float b) { a /= (scalar_t)b; return a; }), "/=");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const double& b) { a /= (scalar_t)b; return a; }), "/=");

                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const cweeBalancedPattern<scalar_t>& b) { a += b; return a; }), "+=");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const cweeBalancedPattern<scalar_t>& b) { a -= b; return a; }), "-=");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const cweeBalancedPattern<scalar_t>& b) { a *= b; return a; }), "*=");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const cweeBalancedPattern<scalar_t>& b) { a /= b; return a; }), "/=");

                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const double& divisor) { return (double)a.RombergIntegral(a.GetMinTime(), a.GetMaxTime()); }), "Integrate");
                lib->add(chaiscript::fun([](cweeBalancedPattern<scalar_t>& a, const u64& from, const u64& to, const double& divisor) { return (double)a.RombergIntegral(from, to); }), "Integrate");

                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, const cweeBalancedPattern<scalar_t>& b) { return a + b; }), "+");
                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, const cweeBalancedPattern<scalar_t>& b) { return a - b; }), "-");
                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, const cweeBalancedPattern<scalar_t>& b) { return a * b; }), "*");
                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, const cweeBalancedPattern<scalar_t>& b) { return a / b; }), "/");

                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, int b) { return a + b; }), "+");
                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, int b) { return a - b; }), "-");
                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, int b) { return a * b; }), "*");
                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, int b) { return a / b; }), "/");

                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, float b) { return a + b; }), "+");
                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, float b) { return a - b; }), "-");
                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, float b) { return a * b; }), "*");
                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, float b) { return a / b; }), "/");

                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, const double& b) { return a + b; }), "+");
                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, const double& b) { return a - b; }), "-");
                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, const double& b) { return a * b; }), "*");
                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, const double& b) { return a / b; }), "/");

                lib->add(chaiscript::fun([](const double& b, const cweeBalancedPattern<scalar_t>& a) { return b + a; }), "+");
                lib->add(chaiscript::fun([](const double& b, const cweeBalancedPattern<scalar_t>& a) { return b - a; }), "-");
                lib->add(chaiscript::fun([](const double& b, const cweeBalancedPattern<scalar_t>& a) { return b * a; }), "*");
                lib->add(chaiscript::fun([](const double& b, const cweeBalancedPattern<scalar_t>& a) { return b / a; }), "/");

                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, const double& b) {
                    auto x = cweeBalancedPattern<scalar_t>(a);
                    x.Lock();
                    for (auto& j : x.UnsafeGetValues()) {
                        *j.object = std::pow(*j.object, b);
                    }
                    x.Unlock();
                    return x;
                    }), "^");
                lib->add(chaiscript::fun([](const double& b, const cweeBalancedPattern<scalar_t>& a) {
                    auto x = cweeBalancedPattern<scalar_t>();
                    a.Lock();
                    for (auto& j : a.UnsafeGetKnotSeries()) {
                        x.AddValue(j->key, std::pow(b, *j->object));
                    }
                    a.Unlock();

                    return x;
                    }), "^");
            
                lib->add(chaiscript::fun([](const cweeBalancedPattern<scalar_t>& a, const cweeBalancedPattern<scalar_t>& b) { return a.R_Squared(b); }), "R_Squared");
            }

            // File System
            if (1) {
                // IP Address Information
                {

                    AddBasicClassTemplate(IpAddressInformation);
                    AddBasicClassMember(IpAddressInformation, city);

                    lib->add(chaiscript::fun(&IpAddressInformation::country_code), "country_code");
                    lib->add(chaiscript::fun(&IpAddressInformation::country_name), "country_name");
                    lib->add(chaiscript::fun(&IpAddressInformation::ip), "ip");
                    lib->add(chaiscript::fun(&IpAddressInformation::latitude), "latitude");
                    lib->add(chaiscript::fun(&IpAddressInformation::longitude), "longitude");
                    lib->add(chaiscript::fun(&IpAddressInformation::metro_code), "metro_code");
                    lib->add(chaiscript::fun(&IpAddressInformation::region_code), "region_code");
                    lib->add(chaiscript::fun(&IpAddressInformation::region_name), "region_name");
                    lib->add(chaiscript::fun(&IpAddressInformation::time_zone), "time_zone");
                    lib->add(chaiscript::fun(&IpAddressInformation::zip_code), "zip_code");
                    lib->add(chaiscript::fun([](IpAddressInformation& a) -> double& { return a.coordinates.z; }), "elevation");
                }
                lib->add(chaiscript::fun([](const cweeStr& Directory) { return fileSystem->ensureDirectoryExists(Directory); }), "ensureDirectoryExists");
                lib->add(chaiscript::fun([](const cweeStr& oldFilePath, const cweeStr& newFilePath) { return fileSystem->renameFile(oldFilePath, newFilePath); }), "renameFile");
                lib->add(chaiscript::fun([](const cweeStr& oldFilePath, const cweeStr& newFilePath) { return fileSystem->copyFile(oldFilePath, newFilePath); }), "copyFile");
                lib->add(chaiscript::fun([](const cweeStr& oldFilePath) { return fileSystem->checkFileExists(oldFilePath); }), "checkFileExists");
                lib->add(chaiscript::fun([]() { return fileSystem->getAppFolder(); }), "getAppFolder");
                lib->add(chaiscript::fun([]() { return fileSystem->getAppName(); }), "getAppName");
                lib->add(chaiscript::fun([]() { return fileSystem->getDataFolder(); }), "getDataFolder");
                lib->add(chaiscript::fun([](cweeStr const& in) { return fileSystem->setDataFolder(in); }), "setDataFolder");
                lib->add(chaiscript::fun([](cweeStr const& directory, cweeStr const& fileName, cweeStr const& fileType) { return fileSystem->createFilePath(directory, fileName, GetBetterEnum< fileType_t >(fileType)); }), "createFilePath");
                lib->add(chaiscript::fun([](cweeStr const& fileType) { return fileSystem->createRandomFilePath(GetBetterEnum< fileType_t >(fileType)); }), "createRandomFilePath");
                lib->add(chaiscript::fun([](cweeStr const& fileType) { return fileSystem->createRandomFile(GetBetterEnum< fileType_t >(fileType)); }), "createRandomFile");
                lib->add(chaiscript::fun([]() { return fileSystem->getCurrentTime(); }), "getCurrentTime");
                lib->add(chaiscript::fun([]() { return fileSystem->localtime(fileSystem->getCurrentTime()); }), "localtime");
                lib->add(chaiscript::fun([](u64 t) { return fileSystem->localtime(t); }), "localtime");
                lib->add(chaiscript::fun([]() { return fileSystem->GetIpAddress(); }), "GetIpAddress");
                {
                    lib->add(chaiscript::fun([](cweeStr const& filePath) { cweeList<chaiscript::Boxed_Value> bv; for (auto& s : fileSystem->readFileAsStrList(filePath)) { bv.Append(chaiscript::Boxed_Value(s)); } return bv; }), "readFileAsStrList");
                    lib->add(chaiscript::fun([](cweeStr const& filePath) { cweeStr out;  fileSystem->readFileAsCweeStr(out, filePath); return out; }), "readFileAsCweeStr");
                    lib->add(chaiscript::fun([](cweeStr const& filePath, cweeStr const& content) { fileSystem->writeFileFromCweeStr(filePath, content);  }), "writeFileFromCweeStr");
                    lib->add(chaiscript::fun([](cweeStr const& url) { fileSystem->DownloadCweeStrFromURL(url); }), "DownloadCweeStrFromURL");
                    lib->add(chaiscript::fun([](cweeStr const& directory, cweeStr const& extension) { cweeList<chaiscript::Boxed_Value> bv; for (auto& s : fileSystem->listFilesWithExtension(directory, extension)) { bv.Append(chaiscript::Boxed_Value(s)); } return bv; }), "listFilesWithExtension");
                }
            }

            // EPAnet 2.2 Wrapper
            if (1) {
                /* Spattern */ {
                    AddNamespacedClassTemplate(::epanet, Spattern);
                    AddNamespacedClassMember(::epanet, Spattern, Comment);
                    AddNamespacedClassMember(::epanet, Spattern, Pat);
                }
                /* Sdemand */ {
                    AddNamespacedClassTemplate(::epanet, Sdemand);
                    AddNamespacedClassMember(::epanet, Sdemand, Base);
                    AddNamespacedClassMember(::epanet, Sdemand, Pat);
                    AddNamespacedClassMember(::epanet, Sdemand, TimePat);
                }
                /* Senergy */ {
                    AddNamespacedClassTemplate(::epanet, Senergy);
                    AddNamespacedClassMember(::epanet, Senergy, TimeOnLine);
                    AddNamespacedClassMember(::epanet, Senergy, Efficiency);
                    AddNamespacedClassMember(::epanet, Senergy, KwHrsPerFlow);
                    AddNamespacedClassMember(::epanet, Senergy, KwHrs);
                    AddNamespacedClassMember(::epanet, Senergy, MaxKwatts);
                    AddNamespacedClassMember(::epanet, Senergy, TotalCost);
                    AddNamespacedClassMember(::epanet, Senergy, CurrentPower);
                    AddNamespacedClassMember(::epanet, Senergy, CurrentEffic);
                }
                /* Ssource */ {
                    AddNamespacedClassTemplate(::epanet, Ssource);
                    AddNamespacedClassMember(::epanet, Ssource, Concentration);
                    AddNamespacedClassMember(::epanet, Ssource, Pat);
                    AddNamespacedClassMember(::epanet, Ssource, TimePat);
                    AddNamespacedClassMember(::epanet, Ssource, Smass);
                }
                /* Svertices */ {
                    AddNamespacedClassTemplate(::epanet, Svertices);
                    AddNamespacedClassMember(::epanet, Svertices, Array);
                }
                /* Sasset */ {
                    AddNamespacedClassTemplate(::epanet, Sasset);
                    // lib->add(chaiscript::fun([](::epanet::Sasset const& a) {  }), "Type_p");
                    AddNamespacedClassMember(::epanet, Sasset, Name_p);
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_HEAD_>(t); }), "GetHeadValue");
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_DEMAND_>(t); }), "GetDemandValue");
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_FLOW_>(t); }), "GetFlowValue");
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_ENERGY_>(t); }), "GetEnergyValue");
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_HEADLOSS_>(t); }), "GetHeadlossValue");
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_VELOCITY_>(t); }), "GetVelocityValue");
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_SETTING_>(t); }), "GetSettingValue");
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_STATUS_>(t); }), "GetStatusValue");
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_QUALITY_>(t); }), "GetQualityValue");
                }
                /* Snode */ {
                    AddNamespacedClassTemplate(::epanet, Snode);
                    // lib->add(chaiscript::fun([](::epanet::Snode const& a) {  }), "Type_p");
                    AddNamespacedClassMember(::epanet, Snode, Name_p);
                    AddNamespacedClassMember(::epanet, Snode, X);
                    AddNamespacedClassMember(::epanet, Snode, Y);
                    AddNamespacedClassMember(::epanet, Snode, El);
                    AddNamespacedClassMember(::epanet, Snode, D);
                    AddNamespacedClassMember(::epanet, Snode, S);
                    AddNamespacedClassMember(::epanet, Snode, Ke);
                    AddNamespacedClassMember(::epanet, Snode, Zone);
                    // AddNamespacedClassMember(::epanet, Snode, Type);
                    lib->add(chaiscript::fun([](::epanet::Snode const& a, u64 t) { return a.GetCurrentValue<_HEAD_>(t); }), "GetHeadValue");
                    lib->add(chaiscript::fun([](::epanet::Snode const& a, u64 t) { return a.GetCurrentValue<_DEMAND_>(t); }), "GetDemandValue");
                    lib->add(chaiscript::fun([](::epanet::Snode const& a, u64 t) { return a.GetCurrentValue<_QUALITY_>(t); }), "GetQualityValue");
                }
                /* Slink */ {
                    AddNamespacedClassTemplate(::epanet, Slink);
                    // lib->add(chaiscript::fun([](::epanet::Sasset const& a) {  }), "Type_p");
                    AddNamespacedClassMember(::epanet, Slink, Name_p);

                    AddNamespacedClassMember(::epanet, Slink, StartingNode);
                    AddNamespacedClassMember(::epanet, Slink, EndingNode);
                    AddNamespacedClassMember(::epanet, Slink, Diam);
                    AddNamespacedClassMember(::epanet, Slink, Len);
                    AddNamespacedClassMember(::epanet, Slink, Kc);
                    AddNamespacedClassMember(::epanet, Slink, Km);
                    AddNamespacedClassMember(::epanet, Slink, Kb);
                    AddNamespacedClassMember(::epanet, Slink, Kw);
                    AddNamespacedClassMember(::epanet, Slink, R_FlowResistance);
                    AddNamespacedClassMember(::epanet, Slink, Rc);
                    // AddNamespacedClassMember(::epanet, Slink, Type);
                    AddNamespacedClassMember(::epanet, Slink, Vertices);

                    lib->add(chaiscript::fun(&::epanet::Slink::Area), "Area");
                    lib->add(chaiscript::fun(&::epanet::Slink::IsBiDirectionalPipe), "IsBiDirectionalPipe");

                    lib->add(chaiscript::fun([](::epanet::Slink const& a, u64 t) { return a.GetCurrentValue<_FLOW_>(t); }), "GetFlowValue");
                    lib->add(chaiscript::fun([](::epanet::Slink const& a, u64 t) { return a.GetCurrentValue<_ENERGY_>(t); }), "GetEnergyValue");
                    lib->add(chaiscript::fun([](::epanet::Slink const& a, u64 t) { return a.GetCurrentValue<_HEADLOSS_>(t); }), "GetHeadlossValue");
                    lib->add(chaiscript::fun([](::epanet::Slink const& a, u64 t) { return a.GetCurrentValue<_VELOCITY_>(t); }), "GetVelocityValue");
                    lib->add(chaiscript::fun([](::epanet::Slink const& a, u64 t) { return a.GetCurrentValue<_SETTING_>(t); }), "GetSettingValue");
                    lib->add(chaiscript::fun([](::epanet::Slink const& a, u64 t) { return a.GetCurrentValue<_STATUS_>(t); }), "GetStatusValue");
                }
                /* Stank */ {
                    AddNamespacedClassTemplate(::epanet, Stank);
                    // lib->add(chaiscript::fun([](::epanet::Snode const& a) {  }), "Type_p");
                    AddNamespacedClassMember(::epanet, Stank, Name_p);
                    AddNamespacedClassMember(::epanet, Stank, X);
                    AddNamespacedClassMember(::epanet, Stank, Y);
                    AddNamespacedClassMember(::epanet, Stank, El);
                    AddNamespacedClassMember(::epanet, Stank, D);
                    AddNamespacedClassMember(::epanet, Stank, S);
                    AddNamespacedClassMember(::epanet, Stank, Ke);
                    AddNamespacedClassMember(::epanet, Stank, Zone);
                    AddNamespacedClassMember(::epanet, Stank, Node);
                    AddNamespacedClassMember(::epanet, Stank, Diameter);
                    AddNamespacedClassMember(::epanet, Stank, Area);
                    AddNamespacedClassMember(::epanet, Stank, Hmin);
                    AddNamespacedClassMember(::epanet, Stank, Hmax);
                    AddNamespacedClassMember(::epanet, Stank, Kb);
                    AddNamespacedClassMember(::epanet, Stank, TimePat);
                    AddNamespacedClassMember(::epanet, Stank, Pat);
                    AddNamespacedClassMember(::epanet, Stank, Vcurve);
                    AddNamespacedClassMember(::epanet, Stank, Vcurve_Actual);
                    AddNamespacedClassMember(::epanet, Stank, MixModel);
                    AddNamespacedClassMember(::epanet, Stank, V1frac);
                    AddNamespacedClassMember(::epanet, Stank, CanOverflow);

                    // AddNamespacedClassMember(::epanet, Snode, Type);
                    lib->add(chaiscript::fun([](::epanet::Stank const& a, u64 t) { return a.GetCurrentValue<_HEAD_>(t); }), "GetHeadValue");
                    lib->add(chaiscript::fun([](::epanet::Stank const& a, u64 t) { return a.GetCurrentValue<_DEMAND_>(t); }), "GetDemandValue");
                    lib->add(chaiscript::fun([](::epanet::Stank const& a, u64 t) { return a.GetCurrentValue<_QUALITY_>(t); }), "GetQualityValue");
                }
                /* Spump */ {
                    AddNamespacedClassTemplate(::epanet, Spump);
                    // lib->add(chaiscript::fun([](::epanet::Sasset const& a) {  }), "Type_p");
                    AddNamespacedClassMember(::epanet, Spump, Name_p);

                    AddNamespacedClassMember(::epanet, Spump, StartingNode);
                    AddNamespacedClassMember(::epanet, Spump, EndingNode);
                    AddNamespacedClassMember(::epanet, Spump, Diam);
                    AddNamespacedClassMember(::epanet, Spump, Len);
                    AddNamespacedClassMember(::epanet, Spump, Kc);
                    AddNamespacedClassMember(::epanet, Spump, Km);
                    AddNamespacedClassMember(::epanet, Spump, Kb);
                    AddNamespacedClassMember(::epanet, Spump, Kw);
                    AddNamespacedClassMember(::epanet, Spump, R_FlowResistance);
                    AddNamespacedClassMember(::epanet, Spump, Rc);
                    // AddNamespacedClassMember(::epanet, Slink, Type);
                    AddNamespacedClassMember(::epanet, Spump, Vertices);

                    AddNamespacedClassMember(::epanet, Spump, Area);
                    AddNamespacedClassMember(::epanet, Spump, IsBiDirectionalPipe);
                    
                    AddNamespacedClassMember(::epanet, Spump, Link);
                    AddNamespacedClassMember(::epanet, Spump, Ptype);

                    AddNamespacedClassMember(::epanet, Spump, Qmax);
                    AddNamespacedClassMember(::epanet, Spump, Hmax);
                    AddNamespacedClassMember(::epanet, Spump, H0);
                    AddNamespacedClassMember(::epanet, Spump, R);
                    AddNamespacedClassMember(::epanet, Spump, N);
                    AddNamespacedClassMember(::epanet, Spump, Hcurve);
                    AddNamespacedClassMember(::epanet, Spump, Ecurve);
                    AddNamespacedClassMember(::epanet, Spump, TimeUpat);
                    AddNamespacedClassMember(::epanet, Spump, TimeEpat);
                    AddNamespacedClassMember(::epanet, Spump, Energy);

                    lib->add(chaiscript::fun([](::epanet::Spump const& a, u64 t) { return a.GetCurrentValue<_FLOW_>(t); }), "GetFlowValue");
                    lib->add(chaiscript::fun([](::epanet::Spump const& a, u64 t) { return a.GetCurrentValue<_ENERGY_>(t); }), "GetEnergyValue");
                    lib->add(chaiscript::fun([](::epanet::Spump const& a, u64 t) { return a.GetCurrentValue<_HEADLOSS_>(t); }), "GetHeadlossValue");
                    lib->add(chaiscript::fun([](::epanet::Spump const& a, u64 t) { return a.GetCurrentValue<_VELOCITY_>(t); }), "GetVelocityValue");
                    lib->add(chaiscript::fun([](::epanet::Spump const& a, u64 t) { return a.GetCurrentValue<_SETTING_>(t); }), "GetSettingValue");
                    lib->add(chaiscript::fun([](::epanet::Spump const& a, u64 t) { return a.GetCurrentValue<_STATUS_>(t); }), "GetStatusValue");
                }
                /* Svalve */ {
                    AddNamespacedClassTemplate(::epanet, Svalve);
                    // lib->add(chaiscript::fun([](::epanet::Sasset const& a) {  }), "Type_p");
                    AddNamespacedClassMember(::epanet, Svalve, Name_p);

                    AddNamespacedClassMember(::epanet, Svalve, StartingNode);
                    AddNamespacedClassMember(::epanet, Svalve, EndingNode);
                    AddNamespacedClassMember(::epanet, Svalve, Diam);
                    AddNamespacedClassMember(::epanet, Svalve, Len);
                    AddNamespacedClassMember(::epanet, Svalve, Kc);
                    AddNamespacedClassMember(::epanet, Svalve, Km);
                    AddNamespacedClassMember(::epanet, Svalve, Kb);
                    AddNamespacedClassMember(::epanet, Svalve, Kw);
                    AddNamespacedClassMember(::epanet, Svalve, R_FlowResistance);
                    AddNamespacedClassMember(::epanet, Svalve, Rc);
                    // AddNamespacedClassMember(::epanet, Slink, Type);
                    AddNamespacedClassMember(::epanet, Svalve, Vertices);

                    
                    lib->add(chaiscript::fun(&::epanet::Svalve::Area), "Area");
                    lib->add(chaiscript::fun(&::epanet::Svalve::IsBiDirectionalPipe), "IsBiDirectionalPipe");

                    AddNamespacedClassMember(::epanet, Svalve, Link);
                    AddNamespacedClassMember(::epanet, Svalve, ProducesElectricity);
                    AddNamespacedClassMember(::epanet, Svalve, Energy);


                    lib->add(chaiscript::fun([](::epanet::Svalve const& a, u64 t) { return a.GetCurrentValue<_FLOW_>(t); }), "GetFlowValue");
                    lib->add(chaiscript::fun([](::epanet::Svalve const& a, u64 t) { return a.GetCurrentValue<_ENERGY_>(t); }), "GetEnergyValue");
                    lib->add(chaiscript::fun([](::epanet::Svalve const& a, u64 t) { return a.GetCurrentValue<_HEADLOSS_>(t); }), "GetHeadlossValue");
                    lib->add(chaiscript::fun([](::epanet::Svalve const& a, u64 t) { return a.GetCurrentValue<_VELOCITY_>(t); }), "GetVelocityValue");
                    lib->add(chaiscript::fun([](::epanet::Svalve const& a, u64 t) { return a.GetCurrentValue<_SETTING_>(t); }), "GetSettingValue");
                    lib->add(chaiscript::fun([](::epanet::Svalve const& a, u64 t) { return a.GetCurrentValue<_STATUS_>(t); }), "GetStatusValue");
                }
                /* Szone */ {
                    AddNamespacedClassTemplate(::epanet, Szone);
                    // lib->add(chaiscript::fun([](::epanet::Sasset const& a) {  }), "Type_p");
                    AddNamespacedClassMember(::epanet, Szone, Name_p);

                    AddNamespacedClassMember(::epanet, Szone, Type);
                    AddNamespacedClassMember(::epanet, Szone, IllDefined);
                    AddNamespacedClassMember(::epanet, Szone, Node);
                    AddNamespacedClassMember(::epanet, Szone, Within_Link);
                    AddNamespacedClassMember(::epanet, Szone, Boundary_Link);
                    AddNamespacedClassMember(::epanet, Szone, AverageElevation);
                    AddNamespacedClassMember(::epanet, Szone, HasWaterDemand);


                    lib->add(chaiscript::fun([](::epanet::Szone const& a, u64 t) { return a.GetCurrentValue<_HEAD_>(t); }), "GetHeadValue");
                    lib->add(chaiscript::fun([](::epanet::Szone const& a, u64 t) { return a.GetCurrentValue<_DEMAND_>(t); }), "GetDemandValue");
                    lib->add(chaiscript::fun([](::epanet::Szone const& a, u64 t) { return a.GetCurrentValue<_FLOW_>(t); }), "GetFlowValue");
                }

                // ...

                /* Network */ {
                    AddNamespacedClassTemplate(::epanet, Network);
                    AddNamespacedClassMember(::epanet, Network, Nnodes);
                    AddNamespacedClassMember(::epanet, Network, Ntanks);
                    AddNamespacedClassMember(::epanet, Network, Njuncs);
                    AddNamespacedClassMember(::epanet, Network, Nlinks);
                    AddNamespacedClassMember(::epanet, Network, Npipes);
                    AddNamespacedClassMember(::epanet, Network, Npumps);
                    AddNamespacedClassMember(::epanet, Network, Nvalves);
                    AddNamespacedClassMember(::epanet, Network, Ncontrols);
                    AddNamespacedClassMember(::epanet, Network, Nrules);
                    AddNamespacedClassMember(::epanet, Network, Npats);
                    AddNamespacedClassMember(::epanet, Network, Ncurves);
                    AddNamespacedClassMember(::epanet, Network, System);

                    AddNamespacedClassMember(::epanet, Network, System);
                    AddNamespacedClassMember(::epanet, Network, Asset);
                    AddNamespacedClassMember(::epanet, Network, Node);
                    AddNamespacedClassMember(::epanet, Network, Link);
                    AddNamespacedClassMember(::epanet, Network, Tank);
                    AddNamespacedClassMember(::epanet, Network, Pump);
                    AddNamespacedClassMember(::epanet, Network, Valve);
                    AddNamespacedClassMember(::epanet, Network, Zone);
                    AddNamespacedClassMember(::epanet, Network, Pattern);
                    AddNamespacedClassMember(::epanet, Network, Curve);
                    AddNamespacedClassMember(::epanet, Network, Control);
                    AddNamespacedClassMember(::epanet, Network, Rule);
                    AddNamespacedClassMember(::epanet, Network, Adjlist);
                }
                /* Project */ {
                    AddNamespacedClassTemplate(::epanet, Project);
                    AddNamespacedClassMember(::epanet, Project, network);
                    AddNamespacedClassMember(::epanet, Project, parser);
                    AddNamespacedClassMember(::epanet, Project, times);
                    AddNamespacedClassMember(::epanet, Project, report);
                    AddNamespacedClassMember(::epanet, Project, outfile);
                    AddNamespacedClassMember(::epanet, Project, hydraul);
                    AddNamespacedClassMember(::epanet, Project, quality);
                }
            }

            // Specific Implimentations of cweeLists to assist with data member navigation
            {
                //... to-do
            }

            lib->eval(R"(

            )");

            return lib;
        }
    };
} // namespace chaiscript

namespace chaiscript {
    class WaterWatch_ChaiScript : public ChaiScript_Basic {
    public:
        WaterWatch_ChaiScript(std::vector<std::string> t_modulepaths = {}, std::vector<std::string> t_usepaths = {}, std::vector<Options> t_opts = chaiscript::default_options())
            : ChaiScript_Basic(
                { chaiscript::Std_Lib::library(), chaiscript::WaterWatch_Lib::library() }
                , chaiscript::make_parser<eval::Noop_Tracer, optimizer::Optimizer_Default>()
                , std::move(t_modulepaths)
                , std::move(t_usepaths)
                , std::move(t_opts)
            ) {
            // ModulePtr ptr;
            // this->add(ptr);
        }
    };
} // namespace chaiscript