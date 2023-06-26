#ifndef __chaiScript_H__
#define __chaiScript_H__

#pragma region "INCLUDES"
#pragma hdrstop
#include "precompiled.h"

static chaiscript::ChaiScript chai;
INLINE chaiscript::ChaiScript& GetChai() {
    return chai;
}

#pragma endregion



#pragma region BOOST_DETECT_FUNCTION_DETAILS

#include <boost/function_types/function_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/preprocessor.hpp>

#include <algorithm>
#include <iostream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <tuple>
#include <utility>

template <typename FuncType>
using Arity = boost::function_types::function_arity<FuncType>;

template <typename FuncType>
using ResultType = typename boost::function_types::result_type<FuncType>::type;

template <typename FuncType, size_t ArgIndex>
using ArgType = typename boost::mpl::at_c<boost::function_types::parameter_types<FuncType>, ArgIndex>::type;

template <typename Func, typename IndexSeq>
struct ArgPrintHelper;

template <typename Func, size_t... Inds>
struct ArgPrintHelper<Func, std::integer_sequence<size_t, Inds...> >
{
    static cweeThreadedList<cweeStr> GetArguments() {
        cweeThreadedList<cweeStr> out;

        std::string typeNames[] = { typeid(ResultType<Func>).name(), typeid(ArgType<Func, Inds>).name()... };
        for (auto const& name : typeNames) {
            cweeStr t = name.c_str();
            t.ReplaceInline("class ", "");
            t.ReplaceInline("enum ", "");
            out.Append(t);
        }

        return out;
    }
};

template <typename Func>
cweeThreadedList<cweeStr> GetFunctionParameters(Func f)
{
    return ArgPrintHelper<Func, std::make_index_sequence<Arity<Func>::value> >::GetArguments();
};

#pragma endregion

struct RegisteredFunctionDef {
    cweeStr functionName;
    cweeThreadedList<cweeStr> inputs;
    cweeStr output;
    bool    isInstanceMethod = false;
    cweeStr instanceMemberClass;
};

struct RegisterObjectWithScriptingData {
    void* ptr = nullptr;
    cweeStr name;
};

template <typename T>
struct RegisterSharedObjectWithScriptingData {
    chaiscript::shared_ptr<T> ptr;
    cweeStr name;
};

class Scripting_FunctionRegistration {
public:
    template <typename T>
    Scripting_FunctionRegistration(T function, const char* name);
    template <typename T>
    Scripting_FunctionRegistration(T function, const char* name, bool isInstanceMethod);
};
#define REGISTER_METHOD_WITH_SCRIPT_ENGINE( function, registrationName )    static Scripting_FunctionRegistration scriptRegister_##registrationName( &function, STR_FUNC(registrationName) )
#define DEF_METHOD_WITH_SCRIPT_ENGINE( function, registrationName )         Scripting_FunctionRegistration scriptRegister_##registrationName( &function, STR_FUNC(registrationName) )
#define DEF_INSTANCE_METHOD_WITH_SCRIPT_ENGINE( function, registrationName )         Scripting_FunctionRegistration scriptRegister_##registrationName( &function, STR_FUNC(registrationName), true )

template <class T> class Scripting_ClassRegistration {
public:
    Scripting_ClassRegistration(const char* name);
};
#define REGISTER_CLASS_WITH_SCRIPT_ENGINE( Class )		                    static Scripting_ClassRegistration<Class> scriptRegister_##Class( STR_FUNC(Class) )
#define DEF_CLASS_WITH_SCRIPT_ENGINE( Class )		                        Scripting_ClassRegistration<Class> scriptRegister_##Class( STR_FUNC(Class) )

class Scripting_PtrRegistration {
public:
    template <typename T>
    Scripting_PtrRegistration(T* obj, const char* name);
};
#define REGISTER_PTR_WITH_SCRIPT_ENGINE( Ptr )		                        static Scripting_PtrRegistration scriptRegister_##Ptr( Ptr, STR_FUNC(Ptr) )
#define DEF_PTR_WITH_SCRIPT_ENGINE( Ptr )   		                        Scripting_PtrRegistration scriptRegister_##Ptr( Ptr, STR_FUNC(Ptr) )

template <class T, std::string(*toFunc)(const T&), T(*fromFunc)(const std::string&, const T&)> class Scripting_EnumRegistration {
public:
    //Scripting_EnumRegistration(const char* name, const T& EndEnum, std::string(*toString)(const T&), T(*fromString)(const std::string&, const T&));
    Scripting_EnumRegistration(const char* name, const T& EndEnum);
};
//#define DEF_ENUM_WITH_SCRIPT_ENGINE( Class, EndEnum, toStringFunc, fromStringFunc)		    Scripting_EnumRegistration<Class> scriptRegister_##Class( STR_FUNC(Class), EndEnum, toStringFunc, fromStringFunc )
//#define DEF_ENUM_WITH_SCRIPT_ENGINE( Class, EndEnum, toStringFunc, fromStringFunc)		    Scripting_EnumRegistration<Class, toStringFunc, fromStringFunc> scriptRegister_##Class( STR_FUNC(Class), EndEnum )
#define DEF_ENUM_WITH_SCRIPT_ENGINE( Class, EndEnum, toStringFunc, fromStringFunc,  SpecificNamespace)		    Scripting_EnumRegistration<##SpecificNamespace::Class, toStringFunc, fromStringFunc> scriptRegister_##Class( STR_FUNC(Class), EndEnum )

class Scripting_SharedPtrRegistration {
public:
    template <typename T>
    Scripting_SharedPtrRegistration(chaiscript::shared_ptr<T> obj, const char* name);
};
#define DEF_SHARED_PTR_WITH_SCRIPT_ENGINE( Ptr )   		                    Scripting_SharedPtrRegistration scriptSharedRegister_##Ptr( Ptr, STR_FUNC(Ptr) )

class Scripting {
public:
    /*! Initialize the runtime scripting engine. */
    void    Init() {
        

        // DEFINE CLASSES
        //DEF_CLASS_WITH_SCRIPT_ENGINE(EDMS_RUNTIME_DATABASE);
        // DEF_CLASS_WITH_SCRIPT_ENGINE(cweeStr);

        // DEFINE METHODS
        //DEF_INSTANCE_METHOD_WITH_SCRIPT_ENGINE(cweeStr::c_str, c_str);

        // CONVERSIONS        
        //chai.add(chaiscript::type_conversion<const std::string, cweeStr>([](const std::string& t_bt) { return cweeStr(t_bt.c_str()); })); // std::string to cweeStr
        //chai.add(chaiscript::type_conversion<const cweeStr, std::string>([](const cweeStr& t_bt) { return std::string(t_bt.c_str()); })); // cweeStr to std::string

        //chai.add(chaiscript::type_conversion<const std::string, const char* __ptr64>([](const std::string& t_bt) { return t_bt.c_str(); })); // std::string to const char*
        //chai.add(chaiscript::type_conversion<const char* __ptr64, std::string>([](const char* t_bt) { return std::string(t_bt); })); // const char* to std::string

        //chai.add(chaiscript::type_conversion<const cweeStr, const char* __ptr64>([](const cweeStr& t_bt) { return t_bt.c_str(); })); // cweeStr to const char*
        //chai.add(chaiscript::type_conversion<const char* __ptr64, cweeStr>([](const char* t_bt) { return cweeStr(t_bt); })); // const char* to cweeStr

    };

    /*! Shutdown the runtime scripting engine. */
    void    ShutDown() {

    };

    void    RequestEarlyExit() {
        chaiscript::earlyExit.store(true);
    };

    /*! Register a C++ REFERENCE object with the script engine. Ideally, the class type should be registered already. */
    template <typename T>
    void    AddObjectToScriptEnv(T& obj, const cweeStr& Name) {
        // parallel threads


        //for (int i = 0; i < parallelProcessor->GetNumJobLists(); i++) {
        //    auto jobList = parallelProcessor->GetJobList(i);
        //    if (jobList) {
        //        RegisterObjectWithScriptingData* io = new RegisterObjectWithScriptingData(); {
        //            io->name = Name;
        //            io->ptr = &obj;
        //        }

        //        jobList->AddParallelJob(Scripting::RegisterPtrWithScriptingJob<T>, io);
        //    }
        //}
        // main thread or current thread
        //chai.add(chaiscript::var(&obj), Name.c_str()); 
        try {
            chai.set_global(chaiscript::var(&obj), Name.c_str());
        }
        catch (std::exception ex) {
            return;
        }
    };

    /*! Register a C++ PTR object with the script engine.  Ideally, the class type should be registered already. */
    template <typename T>
    void    AddObjectToScriptEnv(T* obj, const cweeStr& Name) {       
        // parallel threads
        //for (int i = 0; i < parallelProcessor->GetNumJobLists(); i++) {
        //    auto jobList = parallelProcessor->GetJobList(i);
        //    if (jobList) {
        //        RegisterObjectWithScriptingData* io = new RegisterObjectWithScriptingData(); {
        //            io->name = Name;
        //            io->ptr = obj;
        //        }

        //        jobList->AddParallelJob(Scripting::RegisterPtrWithScriptingJob<T>, io);
        //    }
        //}
        // main thread or current thread
        //chai.add(chaiscript::var(&*obj), Name.c_str());  
        try {
            chai.set_global(chaiscript::var(obj), Name.c_str());
        }
        catch (std::exception ex) {
            return;
        }
        
    };

    /*! Register a C++ SHARED PTR object with the script engine.  Ideally, the class type should be registered already. */
    template <typename T>
    void    AddSharedObjectToScriptEnv(chaiscript::shared_ptr<T> obj, const cweeStr& Name) {
        // parallel threads
        //for (int i = 0; i < parallelProcessor->GetNumJobLists(); i++) {
        //    auto jobList = parallelProcessor->GetJobList(i);
        //    if (jobList) {
        //        RegisterSharedObjectWithScriptingData<T>* io = new RegisterSharedObjectWithScriptingData<T>(); {
        //            io->name = Name;
        //            io->ptr = obj;
        //        }
        //        jobList->AddParallelJob(Scripting::RegisterSharedPtrWithScriptingJob<T>, io);
        //    }
        //}
        // main thread or current thread
        //chai.add(chaiscript::var(&*obj), Name.c_str());  
        try {
            chai.set_global(chaiscript::var(obj), Name.c_str());
        }
        catch (std::exception ex) {
            return;
        }

    };

    /*! Register a class method with the script engine. */
    template <typename T>
    void    AddMethodToScriptEnv(T method, const cweeStr& methodName, bool isInstanceMethod = false) {
        auto outputThenInputs = GetFunctionParameters(method);

        functions.Lock();
        int id = functions.UnsafeAppend();
        auto ptr = functions.UnsafeRead(id);
        if (ptr) {
            ptr->functionName = methodName;
            if (outputThenInputs.Num() > 0) {
                ptr->isInstanceMethod = isInstanceMethod;
                if (ptr->isInstanceMethod) {
                    if (outputThenInputs.Num() > 1) {
                        // second parameter is the class in question.
                        ptr->output = outputThenInputs[0]; // output class, input classes
                        ptr->instanceMemberClass = outputThenInputs[1];
                        ptr->inputs = outputThenInputs;

                        ptr->inputs.RemoveIndex(0); // remove output
                        ptr->inputs.RemoveIndex(0); // remove member class                       
                    }
                }
                else {
                    // static method. 
                    ptr->output = outputThenInputs[0]; // output class, input classes
                    ptr->inputs = outputThenInputs;
                    ptr->inputs.RemoveIndex(0); // input classes only
                }                
            }                       
        }
        functions.Unlock();

        chai.add(chaiscript::fun(method), methodName.c_str());
    };

    /*! Register a class type with the script engine. */
    template <class T>
    void    AddClassToScriptEnv(const cweeStr& clssName) {
        chai.add(chaiscript::user_type<T>(), clssName.c_str());
    };
    
    // template <class T>
    //void    AddEnumToScriptEnv(const cweeStr& clssName, const T& EndEnum, std::string (*toString)(const T&), T(*fromString)(const std::string&, const T&)) {
    template <class T, std::string(*toFunc)(const T&), T(*fromFunc)(const std::string&, const T&)>
    void    AddEnumToScriptEnv(const cweeStr & clssName, const T & EndEnum) {
        chai.add(chaiscript::user_type<T>(), clssName.c_str());

        chai.add(chaiscript::constructor<T()>(), clssName.c_str()); // initialize
        chai.add(chaiscript::constructor<T(const T&)>(), clssName.c_str()); // initialize by copy
        chai.add(chaiscript::type_conversion<const T, int>([](const T& t_bt) { return static_cast<int>(t_bt); })); // cast to int
        chai.add(chaiscript::type_conversion<const int, T>([](int t_bt) { return static_cast<T>(t_bt); })); // initialize by int



        //chai.add(chaiscript::type_conversion<const std::string, T>([fromString](const std::string& t_bt) { T a; return fromString(t_bt.c_str(), a); }));
        //chai.add(chaiscript::type_conversion<const T, std::string>([toString](std::string& a, const T& b) { a = toString(b).c_str(); return a; }));
        //chai.add(chaiscript::fun([fromString](T& a, const std::string& b) { a = fromString(b.c_str(), a); return a; }), "=");
        //chai.add(chaiscript::fun([toString](std::string& a, const T& b) { a = toString(b).c_str(); return a; }), "=");


        chai.add(chaiscript::type_conversion<const std::string, T>([](const std::string& t_bt) { T a; return fromFunc(t_bt.c_str(), a); }));
        chai.add(chaiscript::type_conversion<const T, std::string>([](const T& t_bt) { std::string a = toFunc(t_bt).c_str(); return a; }));
        chai.add(chaiscript::fun([](T& a, const std::string& b) { a = fromFunc(b.c_str(), a); return a; }), "=");
        chai.add(chaiscript::fun([](std::string& a, const T& b) { a = toFunc(b).c_str(); return a; }), "=");





        //chai.add(chaiscript::type_conversion<const std::string, T>([myfunc = fromString](const std::string& t_bt)mutable { T a; return myfunc(t_bt.c_str(), a); }));
        //chai.add(chaiscript::type_conversion<const T, std::string>([myfunc = toString](std::string& a, const T& b)mutable { a = myfunc(b).c_str(); return a; }));
        //chai.add(chaiscript::fun([myfunc = fromString](T& a, const std::string& b)mutable { a = myfunc(b.c_str(), a); return a; }), "=");
        //chai.add(chaiscript::fun([myfunc = toString](std::string& a, const T& b)mutable { a = myfunc(b).c_str(); return a; }), "=");





        //auto func1 = std::function<T(const std::string&)>([fromString](const std::string& t_bt) { T a; return fromString(t_bt.c_str(), a); });
        //auto func2 = std::function<std::string(const T&)>([toString](const T& t_bt) { return toString(t_bt).c_str(); });
        //auto func3 = std::function<T(T&, const std::string&)>([fromString](T& a, const std::string& b) { a = fromString(b.c_str(), a); return a; });
        //auto func4 = std::function<std::string& (std::string&, const T&)>([toString](const T& t_bt) { return toString(t_bt).c_str(); });
        //chai.add(chaiscript::type_conversion<const std::string, T>(func1));
        //chai.add(chaiscript::type_conversion<const T, std::string>(func2));
        //chai.add(chaiscript::fun(func3), "=");
        //chai.add(chaiscript::fun(func4), "=");


        {
            for (int i = 0; i < static_cast<int>(EndEnum); i++) {
                T at = static_cast<T>(i);
                cweeStr nm = toFunc(at).c_str();

                nm.ReduceSpaces();
                nm.ReplaceInline(" ", "");  // nm == "Junction"
                nm.ReplaceInline("$", "");
                nm.ReplaceInline("&", "");
                nm.ReplaceInline("*", "");
                nm.ReplaceInline("%", "");
                nm.ReplaceInline("#", "");
                nm.ReplaceInline("@", "");
                nm.ReplaceInline("!", "");
                nm.ReplaceInline("+", "");
                nm.ReplaceInline("=", "");
                nm.ReplaceInline("-", "");
                nm.ReplaceInline("`", "");
                nm.ReplaceInline("\"", "");
                nm.ReplaceInline("'", "");
                nm.ReplaceInline("[", "");
                nm.ReplaceInline("]", "");
                nm.ReplaceInline("(", "");
                nm.ReplaceInline(")", "");
                nm.ReplaceInline("/", "");
                nm.ReplaceInline("\\", "");
                nm.ReplaceInline("^", "");
                nm.ReplaceInline("?", "");
                nm.ReplaceInline("²", "");

                cweeStr addEnumFunc = cweeStr::printf("def %s::%s() { return %s(%i); }", clssName.c_str(), nm.c_str(), clssName.c_str(), i);
                try {
                    chai.eval(addEnumFunc.c_str());
                }
                catch (std::exception e) {
                    submitToast(cweeStr("Error adding Enum ") + cweeStr(nm.c_str()) + " to Enum class " + cweeStr(clssName.c_str()), e.what());
                }
            }
        }
        
    };
    
    template <typename T>
    static void RegisterPtrWithScriptingJob(RegisterObjectWithScriptingData* data) {
        if (data) {
            if (data->ptr) {
                T* obj = (T*)data->ptr;
                try {
                    chai.set_global(chaiscript::var(&*obj), data->name.c_str());
                }
                catch (std::exception ex) {
                    delete data; 
                    return;
                }               
            }
            delete data;
        }
    };

    template <typename T>
    static void RegisterSharedPtrWithScriptingJob(RegisterSharedObjectWithScriptingData<T>* data) {
        if (data) {
            try {
                chai.set_global(chaiscript::var(data->ptr), data->name.c_str());
            }
            catch (std::exception ex) {
                delete data;
                return;
            }            
            delete data;
        }
    };

public:
    /*! Run user-defined command. Returns result, error codes, or nothing as appropriate. Formatting is similar to a reduced version of C++ or C#. */
    cweeJob Do(const cweeStr& command, bool noConversion = false) { // cweeStr   
        return CreateJob(command, noConversion).AsyncInvoke();
    };

    cweeStr DoImmediately(const cweeStr& command, bool noConversion = false) {
        auto job = CreateJob(command, noConversion);
        auto reply = job.Invoke();
        cweeStr out = reply.cast<cweeStr>();
        return out;
    };

    cweeJob GetBoxedValue(cweeStr command) { // chaiscript::Boxed_Value
        return cweeJob([&](cweeStr& Command, bool NoConversion) {
            chaiscript::Boxed_Value chaiReturned;

            auto guard =
#ifndef CHAISCRIPT_NO_THREADS
                true;
#else
                scriptingMutex->Guard();
#endif
            if (guard){
                while(true){
                    try {
                        chaiReturned = chai.eval(Command.c_str());
                    }
                    catch (std::exception ex) {
                        break;
                    }
                    break;
                }
            }

            return chaiReturned;
        }, command, false).AsyncInvoke();
    };

    chaiscript::Boxed_Value Parse(const cweeStr& command) {
        chaiscript::Boxed_Value chaiReturned;
        chaiscript::Type_Info typeInfo;
        constexpr auto evalErrType = chaiscript::user_type<chaiscript::exception::eval_error>();

        auto guard =
#ifndef CHAISCRIPT_NO_THREADS
            true;
#else
            scriptingMutex->Guard();
#endif
        if (guard) {
            try {
                std::string input = command.c_str();
                auto p = chai.eval<std::function<chaiscript::small_vector<chaiscript::Boxed_Value> (std::string)>>("Parse");
                chaiscript::small_vector<chaiscript::Boxed_Value> output = p(input);
                chaiReturned = chaiscript::Boxed_Value(output);
                typeInfo = chaiReturned.get_type_info();
            }
            catch (chaiscript::Boxed_Value bv) {
                chaiReturned.swap(bv);
                typeInfo = chaiReturned.get_type_info();
                if (typeInfo == evalErrType) {
                    chaiscript::exception::eval_error err = chaiscript::boxed_cast<chaiscript::exception::eval_error>(chaiReturned);

                    auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + err.pretty_print().c_str());
                    chaiReturned.swap(x);
                }
                else {
                    try {
                        auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + chai.to_string(chaiReturned).c_str());
                        chaiReturned.swap(x);
                    }
                    catch (...) {
                        auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + cweeStackTrace::GetTrace(false));
                        chaiReturned.swap(x);
                    }
                }
            }
            catch (chaiscript::exception::eval_error ex) {
                auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + ex.pretty_print().c_str());
                chaiReturned.swap(x);
            }
            catch (std::exception ex) {
                auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + ex.what());
                chaiReturned.swap(x);
            }
            catch (...) {
                auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + cweeStackTrace::GetTrace(false));
                chaiReturned.swap(x);
            }
        }

        return chaiReturned;
    };
    std::string ParseToJSON(const cweeStr& command) {
        std::string output;
        chaiscript::Type_Info typeInfo;
        constexpr auto evalErrType = chaiscript::user_type<chaiscript::exception::eval_error>();

        auto guard =
#ifndef CHAISCRIPT_NO_THREADS
            true;
#else
            scriptingMutex->Guard();
#endif
        if (guard) {
            try {
                std::string input = command.c_str();
                auto p = chai.eval<std::function<chaiscript::Boxed_Value(std::string)>>("Parse");
                auto p2 = chai.eval<std::function<std::string(chaiscript::Boxed_Value)>>("ListNodesToJson");
                output = p2(p(input));
            }
            catch (chaiscript::Boxed_Value bv) {                
                typeInfo = bv.get_type_info();
                if (typeInfo == evalErrType) {
                    chaiscript::exception::eval_error err = chaiscript::boxed_cast<chaiscript::exception::eval_error>(bv);
                    auto x = (cweeStr("Error: ") + err.pretty_print().c_str());
                    output = x.c_str();
                }
                else {
                    try {
                        auto x = (cweeStr("Error: ") + chai.to_string(bv).c_str());
                        output = x.c_str();
                    }
                    catch (...) {
                        auto x = (cweeStr("Error: ") + cweeStackTrace::GetTrace(false));
                        output = x.c_str();
                    }
                }
            }
            catch (chaiscript::exception::eval_error ex) {
                auto x = (cweeStr("Error: ") + ex.pretty_print().c_str());
                output = x.c_str();
            }
            catch (std::exception ex) {
                auto x = (cweeStr("Error: ") + ex.what());
                output = x.c_str();
            }
            catch (...) {
                auto x = (cweeStr("Error: ") + cweeStackTrace::GetTrace(false));
                output = x.c_str();
            }
        }

        return output;
    };



    chaiscript::Boxed_Value GetBoxedValueImmediately(const cweeStr& command) {
        chaiscript::Boxed_Value chaiReturned;
        chaiscript::Type_Info typeInfo;
        constexpr auto evalErrType = chaiscript::user_type<chaiscript::exception::eval_error>();

        auto guard =
#ifndef CHAISCRIPT_NO_THREADS
            true;
#else
            scriptingMutex->Guard();
#endif
        if (guard) {
            try {
                chaiReturned = chai.eval(command.c_str());
                typeInfo = chaiReturned.get_type_info();
            }                    
            catch (chaiscript::Boxed_Value bv) {
                chaiReturned.swap(bv);
                typeInfo = chaiReturned.get_type_info();
                if (typeInfo == evalErrType) {
                    chaiscript::exception::eval_error err = chaiscript::boxed_cast<chaiscript::exception::eval_error>(chaiReturned);

                    auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + err.pretty_print().c_str());
                    chaiReturned.swap(x);
                }
                else {
                    try {
                        auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + chai.to_string(chaiReturned).c_str());
                        chaiReturned.swap(x);
                    }
                    catch (...) {
                        auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + cweeStackTrace::GetTrace(false));
                        chaiReturned.swap(x);
                    }
                }
            }
            catch (chaiscript::exception::eval_error ex) {
                auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + ex.pretty_print().c_str());
                chaiReturned.swap(x);
            }
            catch (std::exception ex) {
                auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + ex.what());
                chaiReturned.swap(x);
            }
            catch (...) {
                auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + cweeStackTrace::GetTrace(false));
                chaiReturned.swap(x);
            }            
        }

        return chaiReturned;
    };

    cweeThreadedList<cweeStr> GetRegisteredFunctions() {
        cweeThreadedList<cweeStr> out;

        for (auto& x : functions.GetList()) {
            cweeStr toAdd; cweeStr params;

            functions.Lock(); {
                auto ptr = functions.UnsafeRead(x);
                if (ptr) {
                    if (!ptr->output.IsEmpty()) {
                        toAdd.AddToDelimiter(ptr->output, " ");                 // __int64
                    }
                    if (!ptr->instanceMemberClass.IsEmpty()) {
                        toAdd.AddToDelimiter(ptr->instanceMemberClass, " ");    // __int64 EDMS_RUNTIME_DATABASE
                        toAdd += ".";                                           // __int64 EDMS_RUNTIME_DATABASE.
                        toAdd += ptr->functionName;                             // __int64 EDMS_RUNTIME_DATABASE.GetCurrentTime
                        toAdd += +"(";                                          // __int64 EDMS_RUNTIME_DATABASE.GetCurrentTime(
                    }
                    else {
                        toAdd.AddToDelimiter(ptr->functionName + "(", " ");     // __int64 GetCurrentTime(
                    }
                    
                    for (auto& x : ptr->inputs) {
                        params.AddToDelimiter(x, ", ");
                    }

                    if (!params.IsEmpty()) {
                        toAdd += params;                                        // __int64 EDMS_RUNTIME_DATABASE.GetCurrentTime(char const*, int, double, [etc.]
                    }                                                           

                    toAdd += ")";                                               // __int64 EDMS_RUNTIME_DATABASE.GetCurrentTime()
                }
            } functions.Unlock();

            if (!toAdd.IsEmpty()) {
                toAdd.ReplaceInline(",", ", ");
                toAdd.ReplaceInline("> >",">>");
                toAdd.ReduceSpaces();
                out.Append(toAdd);
            }
        }

        return out;
    };

    cweeThreadedList<cweeStr> GetRegisteredObjects() {
        cweeThreadedList<cweeStr> out;

        return out;
    };

    void LoadEnvironment() {
        auto guard =
#ifndef CHAISCRIPT_NO_THREADS
            true;
#else
            scriptingMutex->Guard();
#endif
        if (guard) {
            chai.set_state(initializedState);
        }
    };

    void SaveEnvironment() {
        auto guard =
#ifndef CHAISCRIPT_NO_THREADS
            true;
#else
            scriptingMutex->Guard();
#endif
        if (guard) {
            initializedState = chai.get_state();
        }
    };

    cweeJob CreateJob(const chaiscript::Boxed_Value& func) {
        return cweeJob([&](chaiscript::Boxed_Value& Func) {
            chaiscript::Boxed_Value chaiReturned;
            chaiscript::Type_Info typeInfo;
            constexpr auto evalErrType = chaiscript::user_type<chaiscript::exception::eval_error>();

            auto guard =
#ifndef CHAISCRIPT_NO_THREADS
                true;
#else
                scriptingMutex->Guard();
#endif
            if (guard) {

                try {
                    chaiReturned = chai.call_lambda(Func);
                    typeInfo = chaiReturned.get_type_info();
                }
                catch (chaiscript::Boxed_Value bv) {
                    chaiReturned.swap(bv);
                    typeInfo = chaiReturned.get_type_info();
                    if (typeInfo == evalErrType) {
                        chaiscript::exception::eval_error err = chaiscript::boxed_cast<chaiscript::exception::eval_error>(chaiReturned);

                        auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + err.pretty_print().c_str());
                        chaiReturned.swap(x);
                    }
                    else {
                        try {
                            auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + chai.to_string(chaiReturned).c_str());
                            chaiReturned.swap(x);
                        }
                        catch (...) {
                            auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + cweeStackTrace::GetTrace(false));
                            chaiReturned.swap(x);
                        }
                    }
                }
                catch (chaiscript::exception::eval_error ex) {
                    auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + ex.pretty_print().c_str());
                    chaiReturned.swap(x);
                }
                catch (std::exception ex) {
                    auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + ex.what());
                    chaiReturned.swap(x);
                }
                catch (...) {
                    auto x = chaiscript::Boxed_Value(cweeStr("Error: ") + cweeStackTrace::GetTrace(false));
                    chaiReturned.swap(x);
                }
            }

            return chaiReturned;
            }, chaiscript::Boxed_Value(func));
    };

public:
    cweeJob CreateJob(const cweeStr& command, bool noConversion = false) { // cweeStr   
        return cweeJob([&](cweeStr& Command, bool NoConversion) {
            // cweeStackTrace::GetTrace();

            cweeStr out;

            auto guard = 
#ifndef CHAISCRIPT_NO_THREADS
                true; 
#else
                scriptingMutex->Guard();
#endif
            if (guard) {
                constexpr auto evalErrType = chaiscript::user_type<chaiscript::exception::eval_error>();
                chaiscript::Boxed_Value chaiReturned;
                chaiscript::Type_Info typeInfo;
                { // perform evaluation
                    try {
                        chaiReturned = chai.eval(Command.c_str());
                        typeInfo = chaiReturned.get_type_info();
                    }
                    catch (chaiscript::Boxed_Value bv) {
                        chaiReturned.swap(bv);
                        typeInfo = chaiReturned.get_type_info();
                        if (typeInfo == evalErrType) {
                            chaiscript::exception::eval_error err = chaiscript::boxed_cast<chaiscript::exception::eval_error>(chaiReturned);
                            out = cweeStr("Error: ") + err.pretty_print().c_str();
                            return out;
                        }
                        else {
                            try {
                                out = cweeStr("Error: ") + chai.to_string(chaiReturned).c_str();
                                return out;
                            }
                            catch (...) {
                                out = cweeStr("Error: ") + cweeStackTrace::GetTrace(false);
                                return out;
                            }
                        }
                    }
                    catch (chaiscript::exception::eval_error ex) {
                        out = cweeStr("Error: ") + ex.pretty_print().c_str();
                        return out;
                    }
                    catch (std::exception ex) {
                        out = cweeStr("Error: ") + ex.what();
                        return out;
                    }
                    catch (...) {
                        out = cweeStr("Error: ") + cweeStackTrace::GetTrace(false);
                        return out;
                    }
                }

                if (typeInfo == evalErrType) {
                    chaiscript::exception::eval_error err = chaiscript::boxed_cast<chaiscript::exception::eval_error>(chaiReturned);
                    out = cweeStr("Error: ") + err.pretty_print().c_str();
                    return out;
                }

                { // get result
                    bool isUndef = typeInfo.is_undef();
                    bool isVoid = typeInfo.is_void();
                    if (!isUndef && !isVoid && !NoConversion) {
                        cweeStr typeName = typeInfo.name();

                        try {
                            //auto toStrFunc = chai.eval<std::function<std::string(chaiscript::Boxed_Value)>>("to_string");
                            //out = toStrFunc(chaiReturned).c_str();

                            if (typeName.Find("class std::vector") == 0) {
                                out = chai.to_string(chaiReturned).c_str();
                            }
                            else if (typeName.Find("class std::map") == 0) {
                                out = chai.to_string(chaiReturned).c_str();
                            }
                            else if (typeName.Find("class std::basic_string") == 0) {
                                std::string z = chaiscript::boxed_cast<std::string>(chaiReturned);
                                out = cweeStr(z.c_str());
                            }
                            else if (typeName.Find("class cweeStr") == 0) {
                                out = chaiscript::boxed_cast<cweeStr>(chaiReturned);
                            }
                            else if (typeName.Icmp("int") == 0) {
                                int z = chaiscript::boxed_cast<int>(chaiReturned);
                                out = cweeStr(z);
                            }
                            else if (typeName.Icmp("double") == 0) {
                                double z = chaiscript::boxed_cast<double>(chaiReturned);
                                out = cweeStr(z);
                            }
                            else if (typeName.Icmp("__int64") == 0) {
                                __int64 z = chaiscript::boxed_cast<__int64>(chaiReturned);
                                out = cweeStr((u64)z);
                            }
                            else if (typeName.Icmp("bool") == 0) {
                                bool z = chaiscript::boxed_cast<bool>(chaiReturned);
                                out = cweeStr(z);
                            }
                            else 
                            {
                                //auto toStrFunc = chai.eval<std::function<std::string(chaiscript::Boxed_Value)>>("to_string");
                                //out = toStrFunc(chaiReturned).c_str();

                                out = chai.to_string(chaiReturned).c_str();
                            }
                        }
                        catch (chaiscript::exception::eval_error ex) {
                            out = "Successfuly recieved a 'dynamic object' of type: " + typeName;
                            // out = cweeStr("Error: ") + ex.pretty_print().c_str();
                            return out;
                        }
                        catch (std::exception ex) {
                            out = "Successfuly recieved a 'dynamic object' of type: " + typeName;
                            return out;
                        }
                        catch (...) {
                            out = "Could not parse the returned scripting object.";
                            return out;
                        }
                    }
                }
                return out;
            }

            return out;
        }, command, noConversion);
    };

private:
    cweeUnorderedList< RegisteredFunctionDef > functions;
    chaiscript::ChaiScript_Basic::State initializedState = chai.get_state();
     
    cweeSharedPtr< cweeSysMutex > scriptingMutex = cweeSharedPtr< cweeSysMutex >(new cweeSysMutex());
};

/*! Runtime Scripting Engine */
static Scripting scripting_local;
/*! Runtime Scripting Engine */
static Scripting* scripting = &scripting_local;

template <typename T>   INLINE    Scripting_FunctionRegistration::Scripting_FunctionRegistration(T function, const char* name) {
    cweeStr t = name;
    if (t.Find("::") >= 0) {
        cweeParser a(t, "::");
        if (a.getNumVars() >= 2) {
            scripting->AddMethodToScriptEnv(function, a[1].c_str());
            return;
        }
    }

    scripting->AddMethodToScriptEnv(function, name);
    return;
};
template <typename T>   INLINE    Scripting_FunctionRegistration::Scripting_FunctionRegistration(T function, const char* name, bool isInstanceMethod) {
    cweeStr t = name;
    if (t.Find("::") >= 0) {
        cweeParser a(t, "::");
        if (a.getNumVars() >= 2) {
            scripting->AddMethodToScriptEnv(function, a[1].c_str(), true);
            return;
        }
    }
   
    scripting->AddMethodToScriptEnv(function, name, true); // should not happen?
    return;
};
template <class T>      INLINE    Scripting_ClassRegistration<T>::Scripting_ClassRegistration(const char* name) { 
    scripting->AddClassToScriptEnv<T>(name); 
};
//template <class T>      INLINE    Scripting_EnumRegistration<T>::Scripting_EnumRegistration(const char* name, const T& EndEnum, std::string(*toString)(const T&), T(*fromString)(const std::string&, const T&)) {
//    scripting->AddEnumToScriptEnv<T>(name, EndEnum, toString, fromString);
//};
template <class T, std::string(*toFunc)(const T&), T(*fromFunc)(const std::string&, const T&)>      INLINE    Scripting_EnumRegistration<T, toFunc, fromFunc>::Scripting_EnumRegistration(const char* name, const T& EndEnum) {
    scripting->AddEnumToScriptEnv<T, toFunc, fromFunc>(name, EndEnum);
};
template <typename T>   INLINE    Scripting_PtrRegistration::Scripting_PtrRegistration(T* obj, const char* name) { 
#if 1
    chai.add(chaiscript::fun([obj]()-> T& { return *obj; }), name);
#else
    scripting->AddObjectToScriptEnv(*obj, name); 
#endif
};
template <typename T>   INLINE    Scripting_SharedPtrRegistration::Scripting_SharedPtrRegistration(chaiscript::shared_ptr<T> obj, const char* name) {
#if 1
    chai.add(chaiscript::fun([obj]()-> T& { return *obj; }), name);
#else
    scripting->AddSharedObjectToScriptEnv(obj, name);
#endif
};
#endif