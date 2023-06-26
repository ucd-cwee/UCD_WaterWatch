#pragma once
#include "Precompiled.h"

namespace chaiscript {
    class FutureObj {
    public:
        FutureObj() : bv(false, chaiscript::Boxed_Value()), prom(), fut(), awaiter() {};
        FutureObj(cweeAny promise_p, std::future<chaiscript::Boxed_Value>&& fut_p, cweeJob const& awaiter_p) : bv(false, chaiscript::Boxed_Value()), prom(promise_p), fut(std::forward<std::future<chaiscript::Boxed_Value>>(fut_p)), awaiter(awaiter_p) {};
        
        bool valid() const { return fut.valid(); };
        AUTO wait() {
            if (!bv.get<0>()) {
                while (!valid()) {
                    awaiter.AwaitAll();
                }
                bv = cweeUnion<bool, chaiscript::Boxed_Value>(true, std::move(fut.get()));
            }
            return bv.get<1>();
        };
        AUTO get() { return wait(); };
             
    protected:
        cweeUnion<bool, chaiscript::Boxed_Value> bv; // after the user asks for it, it is stored here to prevent the double-tap throw from std::future
        std::future<chaiscript::Boxed_Value> fut; // grants the ability to 'check' for completion.
        cweeAny prom; // contains the underlying result until the user asks for it.
        cweeJob awaiter;
    };

    class Std_Lib {
    public:
        [[nodiscard]] static ModulePtr library() {
            auto lib = chaiscript::make_shared<Module>();
            bootstrap::Bootstrap::bootstrap(*lib);

            bootstrap::standard_library::vector_type<chaiscript::small_vector<Boxed_Value>>("Vector", *lib);
            bootstrap::standard_library::string_type<std::string>("string", *lib);
            bootstrap::standard_library::map_type<std::map<std::string, Boxed_Value>>("Map", *lib);
            bootstrap::standard_library::pair_type<std::pair<Boxed_Value, Boxed_Value>>("Pair", *lib);

#ifndef CHAISCRIPT_NO_THREADS
            lib->add(chaiscript::user_type<FutureObj>(), "future");

            lib->add(chaiscript::fun([](FutureObj const& a) { return a.valid(); }), "valid");
            lib->add(chaiscript::fun([](FutureObj& a) { return a.get(); }), "get");
            lib->add(chaiscript::fun([](FutureObj& a) { return a.wait(); }), "wait");
            lib->add(chaiscript::fun([](FutureObj& a) { return a.wait(); }), "await");
            lib->add(chaiscript::fun([](FutureObj& a) { return a.valid(); }), "finished");

            lib->add(chaiscript::fun([](std::function<chaiscript::Boxed_Value()> const& t_func) {
                cweeAny p = make_cwee_shared<std::promise<chaiscript::Boxed_Value>>(new std::promise<chaiscript::Boxed_Value>());
                auto awaiter = cweeJob([](std::function<chaiscript::Boxed_Value()>& func, std::promise<chaiscript::Boxed_Value>& promise) {
                    promise.set_value(func());
                }, t_func, p).AsyncInvoke();
                return FutureObj(
                    p, /* promise container */
                    p.cast< std::promise<chaiscript::Boxed_Value>& >().get_future(), /* future container */
                    awaiter /* job task */
                );
            }), "Async");
            lib->add(chaiscript::fun([](std::function<void()> const& t_func) {
                cweeJob([](std::function<chaiscript::Boxed_Value()>& func) {
                    func();
                }, t_func).AsyncInvoke(); 
            }), "Async");
            lib->add(chaiscript::fun([](std::function<chaiscript::Boxed_Value()> const& t_func) {
                cweeAny p = make_cwee_shared<std::promise<chaiscript::Boxed_Value>>(new std::promise<chaiscript::Boxed_Value>());
                auto awaiter = cweeJob([](std::function<chaiscript::Boxed_Value()>& func, std::promise<chaiscript::Boxed_Value>& promise) {
                    promise.set_value(func());
                }, t_func, p).AsyncInvoke();
                return FutureObj(
                    p, /* promise container */
                    p.cast< std::promise<chaiscript::Boxed_Value>& >().get_future(), /* future container */
                    awaiter /* job task */
                );
            }), "async");
            lib->add(chaiscript::fun([](std::function<void()> const& t_func) {
                cweeJob([](std::function<chaiscript::Boxed_Value()>& func) {
                    func();
                }, t_func).AsyncInvoke();
            }), "async");     

            // allow the user to submit evals in Async.
            lib->eval(R"(
                def Async(string r){ return Async(fun[r](){ return eval(r); }); };
                def async(string r){ return Async(fun[r](){ return eval(r); }); };
            )");
            lib->add(chaiscript::fun([](double milliseconds) { ::Sleep(milliseconds); }), "sleep");
#endif

            json_wrap::library(*lib);

            lib->eval(ChaiScript_Prelude::chaiscript_prelude() /*, "standard prelude"*/);

            return lib;
        }
    };
} // namespace chaiscript