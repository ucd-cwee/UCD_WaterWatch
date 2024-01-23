/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include "WaterWatch_Module_P1.h"
#include "FileSystemH.h"
#include "DispatchTimer.h" 
#include "Parser.h"
#include "cweeTime.h"
#include "Clock.h" 
#include "enum.h"
#include "odbc.h"
#include "cweeJob.h"

namespace chaiscript {
    namespace WaterWatch_Lib {
        [[nodiscard]] ModulePtr library_1() {
            auto lib = chaiscript::make_shared<Module>();
            bootstrap::Bootstrap::bootstrap(*lib);

            ADD_BETTER_ENUM_TO_SCRIPT_ENGINE(chaiscript::AST_Node_Type, AST_Node_Type);

#ifndef CHAISCRIPT_NO_THREADS
            AddBasicClassTemplate_SpecializedName(FutureObj, future); // lib->add(chaiscript::user_type<FutureObj>(), "future");
            lib->add(chaiscript::fun([](FutureObj const& a) { return a.valid(); }), "valid");
            lib->add(chaiscript::fun([](FutureObj& a) { return a.get(); }), "get");
            lib->add(chaiscript::fun([](FutureObj& a) { return a.wait(); }), "wait");
            lib->add(chaiscript::fun([](FutureObj& a) { return a.wait(); }), "await");
            lib->add(chaiscript::fun([](FutureObj& a) { return a.valid(); }), "finished");
            lib->add(chaiscript::fun([](FutureObj& a, std::function<chaiscript::Boxed_Value()> const& f) { return a.continue_with(f); }), "continue_with");
            lib->add(chaiscript::fun([](FutureObj& a, std::function<void()> const& f) { return a.continue_with(f); }), "continue_with");
            lib->add(chaiscript::fun([](FutureObj& a, std::function<chaiscript::Boxed_Value(chaiscript::Boxed_Value)> const& f) { return a.continue_with(f); }), "continue_with");
            lib->add(chaiscript::fun([](FutureObj& a, std::function<void(chaiscript::Boxed_Value)> const& f) { return a.continue_with(f); }), "continue_with");

            lib->add(chaiscript::fun([](std::function<chaiscript::Boxed_Value()> const& t_func) {
                cweeAny p = make_cwee_shared<std::promise<chaiscript::Boxed_Value>>(new std::promise<chaiscript::Boxed_Value>());
                auto awaiter = cweeJob([](std::function<chaiscript::Boxed_Value()>& func, std::promise<chaiscript::Boxed_Value>& promise) {
                    chaiscript::Boxed_Value toReturn;
                    try {
                        toReturn = func();
                    }
                    catch (...) {}
                    promise.set_value(toReturn);
                }, t_func, p).AsyncInvoke();
                return FutureObj(
                    p, /* promise container */
                    p.cast< std::promise<chaiscript::Boxed_Value>& >().get_future(), /* future container */
                    awaiter /* job task */
                );
            }), "Async");
            lib->add(chaiscript::fun([](std::function<void()> const& t_func) {
                cweeAny p = make_cwee_shared<std::promise<chaiscript::Boxed_Value>>(new std::promise<chaiscript::Boxed_Value>());
                auto awaiter = cweeJob([](std::function<void()>& func, std::promise<chaiscript::Boxed_Value>& promise) {
                    chaiscript::Boxed_Value toReturn;
                    try {
                        func();
                    } catch (...) {}
                    promise.set_value(toReturn);
                }, t_func, p).AsyncInvoke();
                return FutureObj(
                    p, /* promise container */
                    p.cast< std::promise<chaiscript::Boxed_Value>& >().get_future(), /* future container */
                    awaiter /* job task */
                );

                //cweeJob([](std::function<chaiscript::Boxed_Value()>& func) {
                //    try {
                //        func();
                //    }
                //    catch (...) {}
                //    }, t_func).AsyncInvoke();
            }), "Async");
            lib->add(chaiscript::fun([](std::function<chaiscript::Boxed_Value()> const& t_func) {
                cweeAny p = make_cwee_shared<std::promise<chaiscript::Boxed_Value>>(new std::promise<chaiscript::Boxed_Value>());
                auto awaiter = cweeJob([](std::function<chaiscript::Boxed_Value()>& func, std::promise<chaiscript::Boxed_Value>& promise) {
                    chaiscript::Boxed_Value toReturn;
                    try {
                        toReturn = func();
                    }
                    catch (...) {}
                    promise.set_value(toReturn);
                    }, t_func, p).AsyncInvoke();
                    return FutureObj(
                        p, /* promise container */
                        p.cast< std::promise<chaiscript::Boxed_Value>& >().get_future(), /* future container */
                        awaiter /* job task */
                    );
                }), "async");
            lib->add(chaiscript::fun([](std::function<void()> const& t_func) {
                cweeAny p = make_cwee_shared<std::promise<chaiscript::Boxed_Value>>(new std::promise<chaiscript::Boxed_Value>());
                auto awaiter = cweeJob([](std::function<void()>& func, std::promise<chaiscript::Boxed_Value>& promise) {
                    chaiscript::Boxed_Value toReturn;
                    try {
                        func();
                    }
                    catch (...) {}
                    promise.set_value(toReturn);
                }, t_func, p).AsyncInvoke();
                return FutureObj(
                    p, /* promise container */
                    p.cast< std::promise<chaiscript::Boxed_Value>& >().get_future(), /* future container */
                    awaiter /* job task */
                );
            }), "async");

            // allow the user to submit evals in Async.
            lib->eval(R"(
                def Async(string r){ return Async(fun[r](){ return eval(r); }); };
                def async(string r){ return Async(fun[r](){ return eval(r); }); };
                def future::continue_with(string r){ return this.continue_with(Async(fun[r](){ return eval(r); })); };
            )");
            lib->add(chaiscript::fun([](units::time::second_t const& Seconds) {
                using namespace cwee_units;
                if (Seconds > 0_s) {
                    ::Sleep(Seconds() * 1000.0);
                }
            }), "sleep");
#endif

            // BASIC COUT 
            if (1) {
                lib->AddFunction(, cout, , std::cout << str << std::endl; , const std::string& str);
                lib->AddFunction(, cout, , std::cout << str << std::endl;, const cweeStr & str);
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
                lib->AddFunction(, Stop, -> units::time::second_t, using namespace cwee_units; units::time::nanosecond_t x; x = (float)stopwatch.Stop(); return x; , Stopwatch& stopwatch);
                lib->AddFunction(, Duration, -> units::time::second_t, using namespace cwee_units; units::time::second_t x; x = (float)stopwatch.Seconds_Passed(); return x;, const Stopwatch& stopwatch);
                lib->AddFunction(, Seconds_Passed, -> units::time::second_t, using namespace cwee_units; units::time::second_t x; x = (float)stopwatch.Seconds_Passed(); return x; , const Stopwatch& stopwatch);
                lib->AddFunction(, Nanoseconds_Passed, -> units::time::nanosecond_t, using namespace cwee_units; units::time::nanosecond_t x; x = (float)stopwatch.Nanoseconds_Passed(); return x; , const Stopwatch& stopwatch);
            }

            // DispatchTimer
            if (1) {
                AddBasicClassTemplate(DispatchTimer);
                lib->add(fun([](double millisecondsBetween, std::function<void()> const& t_func) {
                    DispatchTimer out = DispatchTimer(millisecondsBetween, cweeJob([](std::function<void()>& func) {
                        func();
                        }, t_func));
                    return out;
                    }, {"millisecondsBetween", "t_func"}), "DispatchTimer");         // do something on-queue until the timer goes out-of-scope 
            }

            // number to number operators
            if (1) {
#define AllowConversion(tA, tB) lib->add(chaiscript::fun([](tA& a, const tB& b) { a = b; return a; }), "=")
#define AllowConversions(tA) { AllowConversion(tA, std::int32_t);\
                AllowConversion(tA, std::uint8_t);\
                AllowConversion(tA, std::int8_t);\
                AllowConversion(tA, std::uint16_t);\
                AllowConversion(tA, std::int16_t);\
                AllowConversion(tA, std::uint32_t);\
                AllowConversion(tA, std::uint64_t);\
                AllowConversion(tA, std::int64_t);\
                AllowConversion(tA, double);\
                AllowConversion(tA, float);\
                AllowConversion(tA, long double); }

                AllowConversions(std::int32_t);
                AllowConversions(std::uint8_t);
                AllowConversions(std::int8_t);
                AllowConversions(std::uint16_t);
                AllowConversions(std::int16_t);
                AllowConversions(std::uint32_t);
                AllowConversions(std::uint64_t);
                AllowConversions(std::int64_t);
                AllowConversions(double);
                AllowConversions(float);
                AllowConversions(long double);
#undef AllowConversions
#undef AllowConversion
            }

            // string to number operators
            if (1) {
#define AllowConversions(tB) { \
                lib->add(chaiscript::fun([](std::string& a, const tB& b) { a = cweeStr((double)b).c_str(); return a; }), "="); \
                lib->add(chaiscript::fun([](tB& a, const std::string& b) { a = (u64)cweeStr(b); return a; }), "="); }

                AllowConversions(std::int32_t);
                AllowConversions(std::uint8_t);
                AllowConversions(std::int8_t);
                AllowConversions(std::uint16_t);
                AllowConversions(std::int16_t);
                AllowConversions(std::uint32_t);
                AllowConversions(std::uint64_t);
                AllowConversions(std::int64_t);
                AllowConversions(double);
                AllowConversions(float);
                AllowConversions(long double);
#undef AllowConversions
            }

            // size_t convenience functions
            if (1) {
#define AllowConversion(strName, tA, tB) \
                lib->add(chaiscript::constructor<tA(const tB&)>(), #strName); \
                lib->add(chaiscript::type_conversion<tB, tA>([](const tB& t_bt)->tA { return t_bt; }, nullptr)); \
                lib->add(chaiscript::fun([](tA& a, const tB& b) { a = b; return a; }), "=")

#define AllowConversions(strName, tA) { \
                AllowConversion(strName, tA, std::int32_t);\
                AllowConversion(strName, tA, std::uint8_t);\
                AllowConversion(strName, tA, std::int8_t);\
                AllowConversion(strName, tA, std::uint16_t);\
                AllowConversion(strName, tA, std::int16_t);\
                AllowConversion(strName, tA, std::uint32_t);\
                AllowConversion(strName, tA, std::uint64_t);\
                AllowConversion(strName, tA, std::int64_t);\
                AllowConversion(strName, tA, double);\
                AllowConversion(strName, tA, float);\
                AllowConversion(strName, tA, long double); }

                AllowConversions("size_t", size_t);

#undef AllowConversions
#undef AllowConversion
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
                lib->add(chaiscript::fun([](double& a, const cweeStr& b) { a = b.ReturnNumericD(); return a; }), "=");
                lib->add(chaiscript::fun([](float& a, const cweeStr& b) { a = b.ReturnNumericD(); return a; }), "=");
                lib->add(chaiscript::fun([](int& a, const cweeStr& b) { a = b.ReturnNumericD(); return a; }), "=");
                
                lib->add(chaiscript::fun([](std::string& a, const cweeStr& b) { a = b.c_str(); return a; }), "=");
                lib->add(chaiscript::fun([](double& a, const cweeStr& b) { a = b.ReturnNumericD(); return a; }), "=");
                lib->add(chaiscript::fun([](float& a, const cweeStr& b) { a = b.ReturnNumericD(); return a; }), "=");
                lib->add(chaiscript::fun([](int& a, const cweeStr& b) { a = b.ReturnNumericD(); return a; }), "=");

                lib->add(chaiscript::fun([](const cweeStr& a, const cweeStr& b) { return a + b; }), "+");
                lib->add(chaiscript::fun([](const cweeStr& a, const cweeStr& b) { return a < b; }), "<");
                lib->add(chaiscript::fun([](const cweeStr& a, const cweeStr& b) { return a > b; }), ">");
                lib->add(chaiscript::fun([](const cweeStr& a, const cweeStr& b) { return a <= b; }), "<=");
                lib->add(chaiscript::fun([](const cweeStr& a, const cweeStr& b) { return a >= b; }), ">=");
                lib->add(chaiscript::fun([](cweeStr& a, const cweeStr& b) { a += b; return a;  }), "+=");
                lib->add(chaiscript::fun([](const cweeStr& a, const cweeStr& b) { return a == b; }), "==");
                lib->add(chaiscript::fun([](const cweeStr& a, const cweeStr& b) { return a != b; }), "!=");

                lib->add(chaiscript::fun(&cweeStr::RemoveBetween), "RemoveBetween");
                lib->add(chaiscript::fun(&cweeStr::ReplaceBetween), "ReplaceBetween");
                lib->add(chaiscript::fun(&cweeStr::FindBetween), "FindBetween");
                lib->add(chaiscript::fun(&cweeStr::FindFirstBetween), "FindFirstBetween");
                lib->add(chaiscript::fun([](const cweeStr& a)->int { return a.Length(); }), "Length");
                lib->add(chaiscript::fun(&cweeStr::Empty), "Empty");
                lib->add(chaiscript::fun(&cweeStr::IsEmpty), "IsEmpty");
                lib->add(chaiscript::fun(&cweeStr::Clear), "Clear");
                
                lib->AddFunction(, ToLower, , a.ToLower(); return a;, cweeStr& a);
                lib->AddFunction(, ToUpper, , a.ToUpper(); return a; , cweeStr& a);
                lib->add(chaiscript::fun([](cweeStr& a) -> cweeStr {
                    bool newWord = true;
                    for (size_t i = 0; a[i]; i++) {
                        if (newWord) {
                            if (cweeStr::CharIsAlpha(a[i])) {
                                if (cweeStr::CharIsLower(a[i])) {
                                    a[i] -= ('a' - 'A');
                                }
                                newWord = false;
                            }
                        }
                        else {
                            if ((a[i] == ' ') || cweeStr::CharIsNewLine(a[i]) || cweeStr::CharIsTab(a[i])) {
                                newWord = true;
                            }
                        }
                    }
                    return a;
                }), "ToWordCase");
                lib->add(chaiscript::fun([](cweeStr& a) { return a.IsNumeric(); }), "IsNumeric");
                lib->add(chaiscript::fun([](cweeStr& a, cweeStr const& b) { return a.Find(b, true); }), "Find");
                lib->add(chaiscript::fun([](cweeStr& a, cweeStr const& b) { return a.Find(b, false); }), "iFind");
                lib->add(chaiscript::fun([](cweeStr& a, cweeStr const& b) { return a.rFind(b, true); }), "rFind");
                lib->add(chaiscript::fun([](cweeStr& a, cweeStr const& b) { return a.rFind(b, false); }), "riFind");

                lib->add(chaiscript::fun([](cweeStr& a, cweeStr const& b)->int { return a.Cmp(b); }), "Cmp");
                lib->add(chaiscript::fun([](cweeStr& a, cweeStr const& b)->int { return a.Icmp(b); }), "Icmp");

                lib->AddFunction(, Left, , return a.Left(num); return a;, cweeStr& a, int num);
                lib->AddFunction(, Right, , return a.Right(num); return a; , cweeStr& a, int num);
                lib->AddFunction(, Mid, , return a.Mid(start, num); return a;, cweeStr& a, int start, int num);
                lib->add(chaiscript::fun(&cweeStr::ReduceSpaces), "ReduceSpaces");

                lib->AddFunction(, Replace, , return a.ReplaceInline(findWhat, replaceWith);, cweeStr& a, cweeStr const& findWhat, cweeStr const& replaceWith);
                lib->add(chaiscript::fun(&cweeStr::ReturnNumeric), "ReturnNumeric");

                lib->AddFunction(, AddToDelimiter, , return a.AddToDelimiter(add, delim); , cweeStr& a, cweeStr const& add, cweeStr const& delim);
                lib->add(chaiscript::fun([](cweeStr& a) { return cweeStr::Hash(a); }), "Hash");

                lib->add(chaiscript::fun([](cweeStr const& a, cweeStr const& b)->int { return a.Levenshtein(b); }), "Levenshtein");
                lib->add(chaiscript::fun([](cweeStr const& a, cweeStr const& b, bool caseSensitive)->int { return a.Levenshtein(b, caseSensitive); }), "Levenshtein");
                lib->add(chaiscript::fun([](cweeStr const& a, cweeStr const& b) -> cweeStr { auto largestFind = a.FindLargestSharedSubstring(b); return largestFind; }), "FindLargestSharedSubstring");
                lib->add(chaiscript::fun([](cweeStr const& a, cweeStr const& b, bool caseSensitive) -> cweeStr { auto largestFind = a.FindLargestSharedSubstring(b, caseSensitive); return largestFind; }), "FindLargestSharedSubstring");

                lib->add(chaiscript::fun([](cweeStr const& lookFor, std::vector<std::string> const& options)->cweeStr {
                    return lookFor.BestMatch(options);
                }), "BestMatch");
                lib->add(chaiscript::fun([](cweeStr const& lookFor, std::vector<cweeStr> const& options)->cweeStr {
                    return lookFor.BestMatch(options);
                }), "BestMatch");                
                lib->add(chaiscript::fun([](cweeStr const& a) { return std::string(a.c_str()); }), "to_string");
            }

            // cweeTime
            if (1) {
                lib->add(chaiscript::user_type<cweeTime>(), "cweeTime");
                lib->add(chaiscript::constructor<cweeTime()>(), "cweeTime");
                lib->add(chaiscript::constructor<cweeTime(const cweeTime&)>(), "cweeTime");
                lib->add(chaiscript::constructor<cweeTime(const cweeUnitValues::unit_value&)>(), "cweeTime");
                //lib->add(chaiscript::constructor<cweeTime(const u64&)>(), "cweeTime");
                //lib->add(fun([](double a)->cweeTime { return cweeTime(a);  }), "cweeTime");
                //lib->add(fun([](int a)->cweeTime { return cweeTime(a); }), "cweeTime");
                //lib->add(fun([](float a)->cweeTime { return cweeTime(a); }), "cweeTime");
                //lib->add(fun([](long a)->cweeTime { return cweeTime(a); }), "cweeTime");
                lib->add(fun([](cweeUnitValues::unit_value const& a)->cweeTime { return cweeTime(a); }), "cweeTime");
                lib->add(chaiscript::constructor<cweeTime(const cweeStr&)>(), "cweeTime");
                lib->add(chaiscript::fun([](const cweeTime& a) -> cweeStr { return a.c_str(); }), "c_str");
                lib->add(chaiscript::constructor<u64(cweeTime)>(), "u64");
                lib->add(chaiscript::type_conversion<cweeTime, u64>([](const cweeTime& t_bt)->u64 { return (u64)t_bt; }, nullptr));

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
                lib->add(chaiscript::fun([](cweeTime const& T)-> cweeUnitValues::unit_value { return cweeUnitValues::second(cweeTime::GetUtcOffset(T)()); }), "GetUtcOffset");
                lib->add(chaiscript::fun([]()-> cweeUnitValues::unit_value { return cweeUnitValues::second(cweeTime::GetUtcOffset(cweeTime())()); }), "GetUtcOffset");

                lib->add(chaiscript::fun([](int year, int month, int day) { return cweeTime::make_time(year, month, day); }), "cweeTime");
                lib->add(chaiscript::fun([](int year, int month, int day, int hour) { return cweeTime::make_time(year, month, day, hour); }), "cweeTime");
                lib->add(chaiscript::fun([](int year, int month, int day, int hour, int minute) { return cweeTime::make_time(year, month, day, hour, minute); }), "cweeTime");
                lib->add(chaiscript::fun([](int year, int month, int day, int hour, int minute, int second) {return cweeTime::make_time(year, month, day, hour, minute, second); }), "cweeTime");
                lib->add(chaiscript::fun([](int year, int month, int day, int hour, int minute, int second, bool useLocalTime) {return cweeTime::make_time(year, month, day, hour, minute, second, useLocalTime); }), "cweeTime");

                lib->add(chaiscript::fun([](cweeTime const& a) { return std::string(a.c_str()); }), "to_string");
                lib->add(chaiscript::fun([]() { return cweeTime::Epoch(); }), "Epoch");
                lib->add(chaiscript::fun([]() { return cweeTime::Now(); }), "Now");
            }

            // cweeParser
            if (1) {
                lib->add(chaiscript::fun([](const cweeStr& a, const cweeStr& b) { auto c = a.Split(b); cweeList<Boxed_Value> out; for (auto& x : c) { out.Append(Boxed_Value(cweeStr(x))); } return out; }), "Split");
                lib->add(chaiscript::fun([](const cweeStr& a, const cweeStr& b) { auto c = a.SplitQuotes(b); cweeList<Boxed_Value> out; for (auto& x : c) { out.Append(Boxed_Value(cweeStr(x))); } return out; }), "SplitQuotes");
                lib->add(chaiscript::fun([](const cweeStr& a, const cweeStr& rowSplit, const cweeStr& columnSplit, bool handleQuotes) {
                    cweeList<Boxed_Value> out; 
                    for (auto& x : a.Split(rowSplit)) {
                        cweeList<Boxed_Value> row;
                        if (handleQuotes) {
                            for (auto& y : x.SplitQuotes(columnSplit)) {
                                row.Append(var(cweeStr(y)));
                            }
                        }
                        else {
                            for (auto& y : x.Split(columnSplit)) {
                                row.Append(var(cweeStr(y)));
                            }
                        }                        
                        out.Append(var(std::move(row)));
                    } 
                    return out; 
                }), "ParseCSV");
                lib->add(chaiscript::fun([](const cweeStr& a, const cweeStr& rowSplit, bool handleQuotes) {
                    cweeList<Boxed_Value> out; 
                    for (auto& x : a.Split(rowSplit)) {
                        cweeList<Boxed_Value> row;
                        if (handleQuotes) {
                            for (auto& y : x.SplitQuotes(",")) {
                                row.Append(var(cweeStr(y)));
                            }
                        }
                        else {
                            for (auto& y : x.Split(",")) {
                                row.Append(var(cweeStr(y)));
                            }
                        }
                        out.Append(var(std::move(row)));
                    } 
                    return out; 
                }), "ParseCSV");
                lib->add(chaiscript::fun([](const cweeStr& a, bool handleQuotes) {
                    cweeList<Boxed_Value> out; 
                    for (auto& x : a.Split("\n")) {
                        cweeList<Boxed_Value> row;
                        if (handleQuotes) {
                            for (auto& y : x.SplitQuotes(",")) {
                                row.Append(var(cweeStr(y)));
                            }
                        }
                        else {
                            for (auto& y : x.Split(",")) {
                                row.Append(var(cweeStr(y)));
                            }
                        }
                        out.Append(var(std::move(row)));
                    } 
                    return out; 
                }), "ParseCSV");
            }

            // Math
            if (1) {
                // Random
                {
                    lib->add(chaiscript::fun([]() { return cweeRandomFloat(); }), "cweeRandomFloat");
                    lib->add(chaiscript::fun([](double a) { return cweeRandomFloat(a); }), "cweeRandomFloat");
                    lib->add(chaiscript::fun([](double a, double b) { return cweeRandomFloat(a, b); }), "cweeRandomFloat");
                    lib->add(chaiscript::fun([]() { return cweeRandomInt(); }), "cweeRandomInt");
                    lib->add(chaiscript::fun([](double a) { return cweeRandomInt(a); }), "cweeRandomInt");
                    lib->add(chaiscript::fun([](double a, double b) { return cweeRandomInt(a, b); }), "cweeRandomInt");
                    lib->add(chaiscript::fun([]() { return cweeRandomFloat(); }), "rand");
                    lib->add(chaiscript::fun([](double a) { return cweeRandomFloat(a); }), "rand");
                    lib->add(chaiscript::fun([](double a, double b) { return cweeRandomFloat(a, b); }), "rand");
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

                    lib->AddFunction(, Lerp, , return cweeMath::Lerp<float>(from, to, by); , float from, float to, float by);
                    lib->AddFunction(, RollingAverage, , cweeMath::rollingAverageRef<double>(currentAverage, newSample, numSamples); return currentAverage; , double& currentAverage, const double& newSample, int& numSamples);
                    lib->eval("def RollingAverage(currentAvg, newSample, numSamples){ ++numSamples; currentAvg = currentAvg - (currentAvg / numSamples); currentAvg = currentAvg + (newSample / numSamples); };");

                    lib->add(chaiscript::fun(cweeMath::roundNearest), "roundNearest");
                    lib->add(chaiscript::fun(cweeMath::roundDownNearest), "roundDownNearest");
                }

                lib->add_global_const(chaiscript::const_var(cweeMath::PI), "PI");
                lib->add_global_const(chaiscript::const_var(cweeMath::INF), "INF");
                lib->add_global_const(chaiscript::const_var(cweeMath::AngRad), "AngRad");
            }

            return lib;
        };
    };
}; // namespace chaiscript