/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

This file is distributed under the BSD License.
Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
http://www.chaiscript.com

 History: RTG	/	2023		1. Modified original source code to use WaterWatch tools, and for better real-time support, including object-typing from parsed code, pre-parsing code without running, real multithreaded code analysis, and more.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include "chaiscript_wrapper.h"
#include "Chaiscript_Defines.h"

#include <cassert>
#include <cstring>
#include <exception>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <vector>

namespace chaiscript {
    /// Namespace alias to provide cleaner and more explicit syntax to users.
    using Namespace = dispatch::Dynamic_Object;

    namespace detail {
        using Loadable_Module_Ptr = chaiscript::shared_ptr<Loadable_Module>;
    }

    /// \brief The main object that the ChaiScript user will use.
    class ChaiScript_Basic {
        mutable chaiscript::detail::threading::shared_mutex m_mutex;
        mutable chaiscript::detail::threading::recursive_mutex m_use_mutex;
        chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> GUARD() const {
            return chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex>(m_mutex);
        };
        chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> SHARED_GUARD() const {
            return chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex>(m_mutex);
        };
        
        std::set<std::string> m_used_files;
        std::map<std::string, detail::Loadable_Module_Ptr> m_loaded_modules;
        std::set<std::string> m_active_loaded_modules;

        chaiscript::small_vector<std::string> m_module_paths;
        chaiscript::small_vector<std::string> m_use_paths;

        chaiscript::unique_ptr<parser::ChaiScript_Parser_Base> m_parser;

        chaiscript::detail::Dispatch_Engine m_engine;

        std::map<std::string, std::function<Namespace& ()>> m_namespace_generators;

        /// Evaluates the given string in by parsing it and running the results through the evaluator
        Boxed_Value do_eval(const std::string& t_input, const std::string& t_filename = "__EVAL__", bool /* t_internal*/ = false) {
            try {
                const auto p = m_parser->parse(t_input, t_filename, get_eval_engine());
                return p->eval(chaiscript::detail::Dispatch_State(m_engine, get_eval_engine().earlyExitScript));
            }
            catch (chaiscript::eval::detail::Return_Value& rv) {
                return rv.retval;
            }
        }

        /// Evaluates the given file and looks in the 'use' paths
        Boxed_Value internal_eval_file(const std::string& t_filename) {
            for (const auto& path : m_use_paths) {
                try {
                    const auto appendedpath = path + t_filename;
                    return do_eval(load_file(appendedpath), appendedpath, true);
                }
                catch (const exception::file_not_found_error&) {
                    // failed to load, try the next path
                }
                catch (const exception::eval_error& t_ee) {
                    throw Boxed_Value(t_ee);
                }
            }

            // failed to load by any name
            throw exception::file_not_found_error(t_filename);
        }

        /// Evaluates the given string, used during eval() inside of a script
        Boxed_Value internal_eval(const std::string& t_e) {
            try {
                return do_eval(t_e, "__EVAL__", true);
            }
            catch (const exception::eval_error& t_ee) {
                throw Boxed_Value(t_ee);
            }
        }
        /// Evaluates the given string, used during eval() inside of a script
        Boxed_Value internal_eval(const std::string& t_e, const std::string& t_fN) {
            try {
                return do_eval(t_e, t_fN, true);
            }
            catch (const exception::eval_error& t_ee) {
                throw Boxed_Value(t_ee);
            }
        }


        AUTO get_compatible_functions(const dispatch::Proxy_Function_Base* func) {
            std::map<std::string, Boxed_Value> out;

            if (func->get_arity() >= 0) {

                const chaiscript::small_vector<Type_Info>& inputFuncTypes = func->get_param_types();
                if (inputFuncTypes.size() >= 1) {
                    const Type_Info& TheType = inputFuncTypes[0];
                    const Type_Conversions_State convs(m_engine.conversions(), m_engine.conversions().conversion_saves());

                    std::map<std::string, Boxed_Value> functions = m_engine.get_function_objects();
                    const auto boxed_value_ti = user_type<Boxed_Value>();
                    for (auto& x : functions) {
                        auto& funcName = x.first;
                        AUTO f = chaiscript::boxed_cast<chaiscript::shared_ptr<const dispatch::Proxy_Function_Base>>(x.second);
                        if (f) {
                            AUTO funcs = f->get_contained_functions();
                            funcs.push_back(f);
                            for (chaiscript::shared_ptr<const dispatch::Proxy_Function_Base> ff : funcs) {
                                if (ff) {
                                    const chaiscript::small_vector<Type_Info>& resultFuncTypes = ff->get_param_types();
                                    if (resultFuncTypes.size() >= 2) {
                                        const Type_Info& inputParam1 = resultFuncTypes[1];
                                        if (inputParam1.is_undef() || inputParam1.bare_equal(boxed_value_ti)) { //  || inputParam1.name() == ""
                                            // anything could match with this... it kinda isn't fair and should be skipped
                                            continue;
                                        }
                                        else {
                                            if (dispatch::Proxy_Function_Base::compare_type_to_type(inputParam1, TheType, convs)) {
                                                out[funcName] = chaiscript::var(ff);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

            }
            else {
                for (auto& internalFunc : func->get_contained_functions()) {

                    const chaiscript::small_vector<Type_Info>& inputFuncTypes = internalFunc->get_param_types();
                    if (inputFuncTypes.size() >= 1) {
                        const Type_Info& TheType = inputFuncTypes[0];
                        const Type_Conversions_State convs(m_engine.conversions(), m_engine.conversions().conversion_saves());

                        std::map<std::string, Boxed_Value> functions = m_engine.get_function_objects();
                        const auto boxed_value_ti = user_type<Boxed_Value>();
                        for (auto& x : functions) {
                            auto& funcName = x.first;
                            AUTO f = chaiscript::boxed_cast<chaiscript::shared_ptr<const dispatch::Proxy_Function_Base>>(x.second);
                            if (f) {
                                AUTO funcs = f->get_contained_functions();
                                funcs.push_back(f);
                                for (chaiscript::shared_ptr<const dispatch::Proxy_Function_Base> ff : funcs) {
                                    if (ff) {
                                        const chaiscript::small_vector<Type_Info>& resultFuncTypes = ff->get_param_types();
                                        if (resultFuncTypes.size() >= 2) {
                                            const Type_Info& inputParam1 = resultFuncTypes[1];
                                            if (inputParam1.is_undef() || inputParam1.bare_equal(boxed_value_ti)) { //  || inputParam1.name() == ""
                                                // anything could match with this... it kinda isn't fair and should be skipped
                                                continue;
                                            }
                                            else {
                                                if (dispatch::Proxy_Function_Base::compare_type_to_type(inputParam1, TheType, convs)) {
                                                    out[funcName] = chaiscript::var(ff);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }





                }
            }

            return out;
        };

        /// Builds all the requirements for ChaiScript, including its evaluator and a run of its prelude.
        void build_eval_system(const std::vector<ModulePtr>& t_libs, const chaiscript::small_vector<Options>& t_opts) {
            for (auto& t_lib : t_libs) add(t_lib);

            // "Functions"
            // "AllFunctions"
            AddFunctionTo(m_engine, this, TypeNames, , { return m_engine.get_type_names_exposed(); });
            AddFunctionTo(m_engine, this, FunctionNames, , { return m_engine.FunctionNames_exposed(); });
            AddFunctionTo(m_engine, this, ObjectNames, , { return m_engine.ObjectNames_exposed(); }); 
            AddFunctionTo(m_engine, this, dump_system, , { m_engine.dump_system(); }); 
            AddFunctionTo(m_engine, this, dump_object, , { m_engine.dump_object(t_bv); }, const Boxed_Value& t_bv); 
            AddFunctionTo(m_engine, this, is_type, , { return m_engine.is_type(t_bv, t_type); }, const Boxed_Value& t_bv, const std::string& t_type); 
            AddFunctionTo(m_engine, this, is_type, , return m_engine.is_type(t_bv, m_engine.get_type_name(t_ti)), const Boxed_Value& t_bv, const Type_Info& t_ti);             
            AddFunctionTo(m_engine, this, dynamic_cast, , return m_engine.polymorphic_cast(t_bv, t_type), const Boxed_Value& t_bv, const std::string& t_type);             
            AddFunctionTo(m_engine, this, type_name, , return m_engine.type_name(t_bv), const Boxed_Value& t_bv);
            AddFunctionTo(m_engine, this, function_exists, -> bool, return m_engine.function_exists(t_f), const std::string& t_f);
            

            m_engine.add(fun([this](const dispatch::Proxy_Function_Base* func) {
                std::string returnType = m_engine.get_type_name(func->get_param_types()[0]);
                std::vector<Boxed_Value> out;
                for (auto& x : this->get_compatible_functions(func)) {
                    AUTO f = boxed_cast<const dispatch::Proxy_Function_Base*>(x.second);
                    if (f) {
                        AUTO arity = f->get_arity();

                        AUTO paramTypes = f->get_param_types();

                        cweeStr caller;
                        std::string thisReturnType = m_engine.get_type_name(paramTypes[0]);
                        for (auto i = 1; i <= arity; ++i) {
                            std::string paramTypeName = m_engine.get_type_name(paramTypes[i]);
                            std::string paramName = f->get_ParameterName(i);

                            caller.AddToDelimiter(cweeStr::printf("%s %s", paramTypeName.c_str(), paramName.c_str()), ", ");
                        }
                        out.push_back(var(cweeStr::printf("%s %s.`%s`(%s)", thisReturnType.c_str(), returnType.c_str(), x.first.c_str(), caller.c_str())));
                    }   
                }
                return out; 
            }, { "func" }), "summarize_compatible_functions"); // returns the functions that could be reasonably accessed by a dot accessor

            m_engine.add(fun([this](const dispatch::Proxy_Function_Base* func) {                
                std::vector<Boxed_Value> out;
                for (auto& f : func->get_contained_functions()) {
                    if (f) {
                        std::string functionName = m_engine.get_function_name(f.get());

                        AUTO arity = f->get_arity();

                        AUTO paramTypes = f->get_param_types();

                        cweeStr caller;
                        std::string thisReturnType = m_engine.get_type_name(paramTypes[0]);
                        for (auto i = 1; i <= arity; ++i) {
                            std::string paramTypeName = m_engine.get_type_name(paramTypes[i]);
                            std::string paramName = f->get_ParameterName(i-1);

                            caller.AddToDelimiter(cweeStr::printf("%s %s", paramTypeName.c_str(), paramName.c_str()), ", ");
                        }
                        out.push_back(var(cweeStr::printf("%s `%s`(%s)", thisReturnType.c_str(), functionName.c_str(), caller.c_str())));
                    }
                }
                return out;
                }, { "func" }), "summarize_contained_functions"); // returns the functions that could be reasonably accessed by a dot accessor

            m_engine.add(fun([this](const dispatch::Proxy_Function_Base* func) { 
                return get_compatible_functions(func);
            }, { "func" }), "get_compatible_functions"); // returns the functions that could be reasonably accessed by a dot accessor
            m_engine.add(fun([this](std::string const& className) {
                std::map<std::string, Boxed_Value> out;

                AUTO TheType = m_engine.get_type(className, false);
                const Type_Conversions_State convs(m_engine.conversions(), m_engine.conversions().conversion_saves());

                std::map<std::string, Boxed_Value> functions = m_engine.get_function_objects();
                const auto boxed_value_ti = user_type<Boxed_Value>();
                for (auto& x : functions) {
                    auto& funcName = x.first;
                    AUTO f = chaiscript::boxed_cast<chaiscript::shared_ptr<const dispatch::Proxy_Function_Base>>(x.second);
                    if (f) {
                        AUTO funcs = f->get_contained_functions();
                        funcs.push_back(f);
                        for (chaiscript::shared_ptr<const dispatch::Proxy_Function_Base> ff : funcs) {
                            if (ff) {
                                const chaiscript::small_vector<Type_Info>& resultFuncTypes = ff->get_param_types();
                                if (resultFuncTypes.size() >= 2) {
                                    const Type_Info& inputParam1 = resultFuncTypes[1];
                                    if (inputParam1.is_undef() || inputParam1.bare_equal(boxed_value_ti)) { //  || inputParam1.name() == ""
                                        // anything could match with this... it kinda isn't fair and should be skipped
                                        continue;
                                    }
                                    else {
                                        if (dispatch::Proxy_Function_Base::compare_type_to_type(inputParam1, TheType, convs)) {
                                            out[funcName] = chaiscript::var(ff);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                return out;
            }, { "className" }), "get_compatible_functions"); // returns the functions that could be reasonably accessed by a dot accessor
            m_engine.add(fun([this](cweeStr const& startsWith) {
                chaiscript::small_vector< Boxed_Value > out;
                auto FunctionNames = m_engine.FunctionNames_CweeStr();
                AUTO selected = FunctionNames.Select([=](const cweeStr& a)->bool {
                    // if (a.StartsWith(startsWith)) {
                    if (a.iStartsWith(startsWith)) {
                        return true;
                    }
                    return false;
                });  
                out.reserve(selected.size() + 1);
                for (auto& x : selected) {
                    if (x) {
                        out.emplace_back(chaiscript::var(std::string(x->c_str())));
                    }
                }
                return out;
            }, { "startsWith" }), "get_functions_that_start_with"); // returns the functions that could be reasonably accessed by typing
/*
            m_engine.add(fun([this](cweeStr const& startsWith, bool isInvariant) {
                chaiscript::small_vector< Boxed_Value > out;
                auto FunctionNames = m_engine.FunctionNames_CweeStr();
                AUTO selected = FunctionNames.Select([=](const cweeStr& a)->bool {
                    if (isInvariant) {
                        if (a.iStartsWith(startsWith)) {
                            return true;
                        }
                    }
                    else if (a.StartsWith(startsWith)) {
                        return true;
                    }      
                    return false;                                      
                });
                out.reserve(selected.size() + 1);
                for (auto& x : selected) {
                    if (x) {
                        out.emplace_back(chaiscript::var(std::string(x->c_str())));
                    }
                }
                return out;
                }, { "startsWith", "isInvariant"}), "get_functions_that_start_with"); // returns the functions that could be reasonably accessed by typing
*/
            m_engine.add(fun([this](const dispatch::Proxy_Function_Base* func) {
                std::vector<Boxed_Value> names;
                auto DEF = func->get_parameterNames();
                if (DEF.size() <= 0) {
                    for (auto& fun : func->get_contained_functions()) {
                        AUTO pT = fun->get_param_types();
                        for (int i = 1; i < pT.size(); i++) {
                            names.emplace_back(var(std::string(fun->get_ParameterName(i-1))));
                        }
                        return names;
                    }
                }

                {
                    AUTO pT = func->get_param_types();
                    for (int i = 1; i < pT.size(); i++) {
                        names.emplace_back(var(std::string(func->get_ParameterName(i - 1))));
                    }
                    return names;
                }

                return names;
            }, { "func" }), "get_function_param_names");
            m_engine.add(fun([this](const dispatch::Proxy_Function_Base* func, std::string const& className) {
                std::vector<Boxed_Value> names;
                auto DEF = func->get_parameterNames();
                if (DEF.size() <= 0) {
                    AUTO TheType = m_engine.get_type(className, false);
                    const Type_Conversions_State convs(m_engine.conversions(), m_engine.conversions().conversion_saves());

                    for (auto& fun : func->get_contained_functions()) {
                        if (fun->compare_first_type_to_type(TheType, convs)) {
                            AUTO pT = fun->get_param_types();
                            for (int i = 2; i < pT.size(); i++) {
                                names.emplace_back(var(std::string(fun->get_ParameterName(i - 1))));
                            }
                            return names;
                        }
                    }
                }

                {
                    AUTO pT = func->get_param_types();
                    for (int i = 2; i < pT.size(); i++) {
                        names.emplace_back(var(std::string(func->get_ParameterName(i - 1))));
                    }
                    return names;
                }

                return names;
            }, { "func, className" }), "get_function_param_names");

            AddFunctionTo(m_engine, this, get_functions, , return m_engine.get_function_objects());
            AddFunctionTo(m_engine, this, get_all_functions, , return m_engine.get_all_function_objects());
            AddFunctionTo(m_engine, this, get_function_name, , return m_engine.get_function_name(func), const dispatch::Proxy_Function_Base* func);
            AddFunctionTo(m_engine, this, get_function_description, , return func->get_description(), const dispatch::Proxy_Function_Base* func);
            AddFunctionTo(m_engine, this, get_objects, , return m_engine.get_scripting_objects());
            AddFunctionTo(m_engine, this, get_all_objects, , return m_engine.get_all_scripting_objects());
            AddFunctionTo(m_engine, this, get_global_objects, , return m_engine.get_global_objects());
            AddFunctionTo(m_engine, this, delete, , m_engine.Delete(t_bv); t_bv.assign(Boxed_Value()); , Boxed_Value& t_bv);
            AddFunctionTo(m_engine, this, get_conversions, , return m_engine.conversions().get_conversions());

            m_engine.add(dispatch::make_dynamic_proxy_function([this](const Function_Params& t_params) { return m_engine.call_exists(t_params); }),
                "call_exists");

            m_engine.add(fun([this](const dispatch::Proxy_Function_Base& t_fun, const chaiscript::small_vector<Boxed_Value>& t_params) -> Boxed_Value {
                Type_Conversions_State s(this->m_engine.conversions(), this->m_engine.conversions().conversion_saves());
                return t_fun(Function_Params{ t_params }, s);
            }), "call");

            m_engine.add(fun([this](const chaiscript::small_vector<Boxed_Value>& t_params) -> Boxed_Value {
                if (t_params.size() < 1) {
                    throw chaiscript::exception::arithmetic_error("Can not call a vector with no parameters.");
                }
                auto* t_fun = this->boxed_cast<const dispatch::Proxy_Function_Base*>(t_params[0]);
                return t_fun->operator()(Function_Params{ &t_params.front() + 1, &t_params.front() + t_params.size() }, Type_Conversions_State(this->m_engine.conversions(), this->m_engine.conversions().conversion_saves()));
             }), "call_vector");

            AddFunctionTo(m_engine, this, name, , return m_engine.get_type_name(t_ti); , const Type_Info& t_ti);
            AddFunctionTo(m_engine, this, to_string, , return m_engine.get_type_name(t_ti);, const Type_Info& t_ti);
            AddFunctionTo(m_engine, this, get_name, , return this->get_eval_engine().get_function_name(func); , const chaiscript::dispatch::Proxy_Function_Base* func);

            AddFunctionTo(m_engine, this, type, , return m_engine.get_type(t_type_name, t_throw); , const std::string& t_type_name, bool t_throw);
            AddFunctionTo(m_engine, this, type, , return m_engine.get_type(t_type_name, true);, const std::string& t_type_name);

            m_engine.add(fun([this](const Type_Info& t_from, const Type_Info& t_to, const std::function<Boxed_Value(const Boxed_Value&)>& t_func) {
                m_engine.add(chaiscript::type_conversion(t_from, t_to, t_func));
                }),
                "add_type_conversion");

            if (std::find(t_opts.begin(), t_opts.end(), Options::No_Load_Modules) == t_opts.end()
                && std::find(t_opts.begin(), t_opts.end(), Options::Load_Modules) != t_opts.end()) {
                m_engine.add(fun([this](const std::string& t_module, const std::string& t_file) { load_module(t_module, t_file); }), "load_module");
                m_engine.add(fun([this](const std::string& t_module) { return load_module(t_module); }), "load_module");
            }

            if (std::find(t_opts.begin(), t_opts.end(), Options::No_External_Scripts) == t_opts.end()
                && std::find(t_opts.begin(), t_opts.end(), Options::External_Scripts) != t_opts.end()) {
                m_engine.add(fun([this](const std::string& t_file) { return use(t_file); }), "use");
                m_engine.add(fun([this](const std::string& t_file) { return internal_eval_file(t_file); }), "eval_file");
            }

            m_engine.add(fun([this](const std::string& t_str, const std::string& t_fileName) { return internal_eval(t_str, t_fileName); }), "eval");
            m_engine.add(fun([this](const std::string& t_str) { return internal_eval(t_str); }), "eval");
            m_engine.add(fun([this](const AST_Node& t_ast) { return eval(t_ast); }), "eval");

            m_engine.add(fun([this](const std::string& t_str, const std::string& fileName) { return this->parse(t_str, fileName); }), "parse");
            m_engine.add(fun([this](const std::string& t_str, const bool t_dump) { return this->parse(t_str, t_dump); }), "parse");
            m_engine.add(fun([this](const std::string& t_str) { return this->parse(t_str); }), "parse");

            AddFunctionTo(m_engine, this, ListNodes, , return ListNodes(t_ast), const chaiscript::Boxed_Value& t_ast);

            m_engine.add(fun([this](const Boxed_Value& t_bv, const std::string& t_name) { add_global_const(t_bv, t_name); }), "add_global_const");
            m_engine.add(fun([this](const Boxed_Value& t_bv, const std::string& t_name) { add_global(t_bv, t_name); }), "add_global");
            m_engine.add(fun([this](const Boxed_Value& t_bv, const std::string& t_name) { set_global(t_bv, t_name); }), "set_global");

            // why this unused parameter to Namespace?
            m_engine.add(fun([this](const std::string& t_namespace_name) {
                register_namespace([](Namespace& /*space*/) noexcept {}, t_namespace_name);
                import(t_namespace_name);
                }),
                "namespace");
            m_engine.add(fun([this](const std::string& t_namespace_name) { import(t_namespace_name); }), "import");
        }

        /// Skip BOM at the beginning of file
        static bool skip_bom(std::ifstream& infile) {
            size_t bytes_needed = 3;
            char buffer[3];

            memset(buffer, '\0', bytes_needed);

            infile.read(buffer, static_cast<std::streamsize>(bytes_needed));

            if ((buffer[0] == '\xef') && (buffer[1] == '\xbb') && (buffer[2] == '\xbf')) {
                infile.seekg(3);
                return true;
            }

            infile.seekg(0);

            return false;
        }

        /// Helper function for loading a file
        static std::string load_file(const std::string& t_filename) {
            std::ifstream infile(t_filename.c_str(), std::ios::in | std::ios::ate | std::ios::binary);

            if (!infile.is_open()) {
                throw chaiscript::exception::file_not_found_error(t_filename);
            }

            auto size = infile.tellg();
            infile.seekg(0, std::ios::beg);

            assert(size >= 0);

            if (skip_bom(infile)) {
                size -= 3; // decrement the BOM size from file size, otherwise we'll get parsing errors
                assert(size >= 0); // and check if there's more text
            }

            if (size == std::streampos(0)) {
                return std::string();
            }
            else {
                chaiscript::small_vector<char> v(static_cast<size_t>(size));
                infile.read(&v[0], static_cast<std::streamsize>(size));
                return std::string(v.begin(), v.end());
            }
        }

        chaiscript::small_vector<std::string> ensure_minimum_path_vec(chaiscript::small_vector<std::string> paths) {
            if (paths.empty()) {
                return { "" };
            }
            else {
                return paths;
            }
        }

    public:
        /// Returns the current evaluation m_engine
        chaiscript::detail::Dispatch_Engine& get_eval_engine() noexcept { return m_engine; }

        /// \brief Constructor for ChaiScript
        /// \param[in] t_lib Standard library to apply to this ChaiScript instance
        /// \param[in] t_modulepaths Vector of paths to search when attempting to load a binary module
        /// \param[in] t_usepaths Vector of paths to search when attempting to "use" an included ChaiScript file
        ChaiScript_Basic(const std::vector<ModulePtr>& t_libs,
            chaiscript::unique_ptr<parser::ChaiScript_Parser_Base>&& parser,
            chaiscript::small_vector<std::string> t_module_paths = {},
            chaiscript::small_vector<std::string> t_use_paths = {},
            const chaiscript::small_vector<chaiscript::Options>& t_opts = chaiscript::default_options())
            : m_module_paths(ensure_minimum_path_vec(std::move(t_module_paths)))
            , m_use_paths(ensure_minimum_path_vec(std::move(t_use_paths)))
            , m_parser(std::move(parser))
            , m_engine(*m_parser) {
#if !defined(CHAISCRIPT_NO_DYNLOAD) && defined(_POSIX_VERSION) && !defined(__CYGWIN__)
            // If on Unix, add the path of the current executable to the module search path
            // as windows would do

            union cast_union {
                Boxed_Value(ChaiScript_Basic::* in_ptr)(const std::string&);
                void* out_ptr;
            };

            Dl_info rInfo;
            memset(&rInfo, 0, sizeof(rInfo));
            cast_union u;
            u.in_ptr = &ChaiScript_Basic::use;
            if ((dladdr(static_cast<void*>(u.out_ptr), &rInfo) != 0) && (rInfo.dli_fname != nullptr)) {
                std::string dllpath(rInfo.dli_fname);
                const size_t lastslash = dllpath.rfind('/');
                if (lastslash != std::string::npos) {
                    dllpath.erase(lastslash);
                }

                // Let's see if this is a link that we should expand
                chaiscript::small_vector<char> buf(2048);
                const auto pathlen = readlink(dllpath.c_str(), &buf.front(), buf.size());
                if (pathlen > 0 && static_cast<size_t>(pathlen) < buf.size()) {
                    dllpath = std::string(&buf.front(), static_cast<size_t>(pathlen));
                }

                m_module_paths.insert(m_module_paths.begin(), dllpath + "/");
            }
#endif
            build_eval_system(t_libs, t_opts);
        };

        virtual ~ChaiScript_Basic() {};

#ifndef CHAISCRIPT_NO_DYNLOAD
        /// \brief Constructor for ChaiScript.
        ///
        /// This version of the ChaiScript constructor attempts to find the stdlib module to load
        /// at runtime generates an error if it cannot be found.
        ///
        /// \param[in] t_modulepaths Vector of paths to search when attempting to load a binary module
        /// \param[in] t_usepaths Vector of paths to search when attempting to "use" an included ChaiScript file
        explicit ChaiScript_Basic(chaiscript::unique_ptr<parser::ChaiScript_Parser_Base>&& parser,
            chaiscript::small_vector<std::string> t_module_paths = {},
            chaiscript::small_vector<std::string> t_use_paths = {},
            const chaiscript::small_vector<chaiscript::Options>& t_opts = chaiscript::default_options())
            : ChaiScript_Basic({}, std::move(parser), t_module_paths, t_use_paths, t_opts) {
            try {
                // attempt to load the stdlib
                load_module("chaiscript_stdlib-" + Build_Info::version());
            }
            catch (const exception::load_module_error& t_err) {
                std::cout << "An error occurred while trying to load the chaiscript standard library.\n"
                    "\n"
                    "You must either provide a standard library, or compile it in.\n"
                    "For an example of compiling the standard library in,\n"
                    "see: https://gist.github.com/lefticus/9456197\n"
                    "Compiling the stdlib in is the recommended and MOST SUPPORTED method.\n"
                    "\n\n"
                    << t_err.what();
                throw;
            }
        }
#else // CHAISCRIPT_NO_DYNLOAD
        explicit ChaiScript_Basic(chaiscript::unique_ptr<parser::ChaiScript_Parser_Base>&& parser,
            chaiscript::small_vector<std::string> t_module_paths = {},
            chaiscript::small_vector<std::string> t_use_paths = {},
            const chaiscript::small_vector<chaiscript::Options>& t_opts = chaiscript::default_options())
            = delete;
#endif

        parser::ChaiScript_Parser_Base& get_parser() noexcept {
            return *m_parser;
        }

        const Boxed_Value eval(const AST_Node& t_ast) {
            try {
                return t_ast.eval(chaiscript::detail::Dispatch_State(m_engine, get_eval_engine().earlyExitScript));
            }
            catch (exception::eval_error& t_ee) { // catch (const exception::eval_error& t_ee) {
                throw Boxed_Value(t_ee);
            }
        }

        //chaiscript::utility::add_class<AST_Node>(m,
        //    "AST_Node",
        //    {},
        //    { {fun(&AST_Node::text), "text"},
        //     {fun(&AST_Node::identifier), "identifier"},
        //     {fun(&AST_Node::filename), "filename"},
        //     {fun(&AST_Node::start), "start"},
        //     {fun(&AST_Node::end), "end"},
        //     {fun(&AST_Node::to_string), "to_string"},
        //     {fun(&AST_Node::to_string), "print"},
        //     {fun([](const chaiscript::AST_Node& t_node) -> chaiscript::small_vector<Boxed_Value> {
        //        chaiscript::small_vector<Boxed_Value> retval;
        //        const auto children = t_node.get_children();
        //        std::transform(children.begin(),
        //                       children.end(),
        //                       std::back_inserter(retval),
        //                       &chaiscript::var<const std::reference_wrapper<chaiscript::AST_Node>&>);
        //        return retval;
        //      }),
        //      "children"}
        //    }
        //);


#if 0
        std::map<std::string, chaiscript::Boxed_Value> MapNodesImpl(chaiscript::Boxed_Value const& t_node, int depth) {
            AUTO node = chaiscript::boxed_cast<chaiscript::AST_Node*>(t_node, nullptr);
            AUTO out = std::map<std::string, chaiscript::Boxed_Value>();
            if (node) {               
                out["node"] = t_node;
                out["depth"] = chaiscript::var((int)depth);
                AUTO children = node->get_children();
                if (children.size() > 0) {
                    std::vector<chaiscript::Boxed_Value> t;
                    t.reserve(children.size());
                    for (auto& j : children) {
                        t.push_back(chaiscript::var(MapNodesImpl(chaiscript::var<const std::reference_wrapper<chaiscript::AST_Node>&>(j), depth + 1)));
                    }
                    out["children"] = chaiscript::var(t);
                }
                if (node->identifier == AST_Node_Type::Constant) {
                    //try {
                    //    auto t = m_engine.type_name(eval(cweeStr::printf("`%s`", node->text.c_str()).c_str()));
                    //    out["type"] = chaiscript::var(t);
                    //    return out;
                    //}
                    //catch (...) {
                        try {
                            auto t = m_engine.type_name(eval(cweeStr::printf("%s", node->text.c_str()).c_str()));
                            out["type"] = chaiscript::var(t);
                            //return out;
                        }
                        catch (...) {
                            out["type"] = chaiscript::var(std::string("string"));
                            //return out;
                        }
                    //}
                }
                else if (node->identifier == AST_Node_Type::Id) {
                    if (node->text.size() > 0) {
                        if (m_engine.function_exists(node->text)) {
                            // is a function
                            out["type"] = chaiscript::var(std::string("Function"));
                            //return out;
                        }
                        else {
                            // unknown -- could be an int, a double, a string without its quotes, etc. 
                            try {
                                auto t = eval(cweeStr::printf("%s.type_name()", node->text.c_str()).c_str());
                                out["type"] = t;
                                //return out;
                            }
                            catch (...) {
                                out["type"] = chaiscript::var(std::string("Object"));
                                //return out;
                            }
                        }
                    }
                    else {
                        out["type"] = chaiscript::var(std::string("Object"));
                        //return out;
                    }


                    //try {
                    //    auto t = eval(cweeStr::printf("`%s`.type_name()", node->text.c_str()).c_str());
                    //    out["type"] = t;
                    //    return out;
                    //}
                    //catch (...) {
                    //    try {
                    //        auto t = eval(cweeStr::printf("%s.type_name()", node->text.c_str()).c_str());
                    //        out["type"] = t;
                    //        return out;
                    //    }
                    //    catch (...) {
                    //        out["type"] = chaiscript::var("Object");
                    //        return out;
                    //    }
                    //}
                }
            }
            return out;
        }
#endif
        std::vector< std::map<std::string, chaiscript::Boxed_Value> > MapNodesImpl_List(chaiscript::Boxed_Value const& t_node, int depth) {
            AUTO node = chaiscript::boxed_cast<chaiscript::AST_Node*>(t_node, nullptr);
            std::vector< std::map<std::string, chaiscript::Boxed_Value> > final_out;
            std::map<std::string, chaiscript::Boxed_Value> out;
            if (node) {
                AUTO children = node->get_children();
                final_out.reserve(2 + children.size());
                
                out["node"] = t_node;
                out["depth"] = chaiscript::var((int)depth);


                if (!node->potentialReturnType.Type().is_undef()) {
                    out["type"] = chaiscript::var(m_engine.get_type_name(node->potentialReturnType.Type()));
                }

                final_out.push_back(out);
                
                if (children.size() > 0) {
                    for (auto& j : children) {
                        AUTO node_list = MapNodesImpl_List(chaiscript::var<const std::reference_wrapper<chaiscript::AST_Node>&>(j), depth + 1);
                        for (auto& sub_node : node_list) {
                            final_out.push_back(sub_node);
                        }
                    }
                }                
            }            
            return final_out;
        }

        std::vector<Boxed_Value> ListNodes(chaiscript::Boxed_Value const& t_node) {
            AUTO node = chaiscript::boxed_cast<chaiscript::AST_Node*>(t_node, nullptr);
            std::vector<Boxed_Value> out;
            if (node) {
                std::vector< std::map<std::string, chaiscript::Boxed_Value> > node_list = MapNodesImpl_List(t_node, 0);
                if (node_list.size() > 0) {
                    std::sort(node_list.begin(), node_list.end(), [](std::map<std::string, chaiscript::Boxed_Value> const& a, std::map<std::string, chaiscript::Boxed_Value> const& b) {
                        return chaiscript::boxed_cast<int>(a.at("depth")) < chaiscript::boxed_cast<int>(b.at("depth"));
                    });
                    out.reserve(node_list.size() + 1);
                    out.insert(out.end(), node_list.begin(), node_list.end());
                }            
            }
            return out;
        }

        AST_NodePtr parse(const std::string& t_input, const bool t_debug_print = false) {
            auto ast = m_parser->parse(t_input, "PARSE", this->m_engine);
            if (t_debug_print) {
                m_parser->debug_print(*ast);
            }
            return ast;
        }

        AST_NodePtr parse(const std::string& t_input, const std::string& t_filename) {
            auto ast = m_parser->parse(t_input, t_filename, this->m_engine);
            return ast;
        }


        std::string get_type_name(const Type_Info& ti) const { return m_engine.get_type_name(ti); }

        template<typename T>
        std::string get_type_name() const {
            return get_type_name(user_type<T>());
        }

        /// \brief Loads and parses a file. If the file is already open, it is not
        /// reloaded. The use paths specified at ChaiScript construction time are
        /// searched for the requested file.
        ///
        /// \param[in] t_filename Filename to load and evaluate
        Boxed_Value use(const std::string& t_filename) {
            for (const auto& path : m_use_paths) {
                const auto appendedpath = path + t_filename;
                try {
                    chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);
                    AUTO shared_guard = GUARD();

                    Boxed_Value retval;

                    if (m_used_files.count(appendedpath) == 0) {
                        //l2.unlock();
                        retval = eval_file(appendedpath);
                        //l2.lock();
                        m_used_files.insert(appendedpath);
                    }

                    return retval; // return, we loaded it, or it was already loaded
                }
                catch (const exception::file_not_found_error& e) {
                    if (e.filename != appendedpath) {
                        // a nested file include failed
                        throw;
                    }
                    // failed to load, try the next path
                }
            }

            // failed to load by any name
            throw exception::file_not_found_error(t_filename);
        }

        /// \brief Adds a constant object that is available in all contexts and to all threads
        /// \param[in] t_bv Boxed_Value to add as a global
        /// \param[in] t_name Name of the value to add
        /// \throw chaiscript::exception::global_non_const If t_bv is not a constant object
        /// \sa Boxed_Value::is_const
        ChaiScript_Basic& add_global_const(const Boxed_Value& t_bv, const std::string& t_name) {
            Name_Validator::validate_object_name(t_name);
            m_engine.add_global_const(t_bv, t_name);
            return *this;
        }

        /// \brief Adds a mutable object that is available in all contexts and to all threads
        /// \param[in] t_bv Boxed_Value to add as a global
        /// \param[in] t_name Name of the value to add
        /// \warning The user is responsible for making sure the object is thread-safe if necessary
        ///          ChaiScript is thread-safe but provides no threading locking mechanism to the script
        ChaiScript_Basic& add_global(const Boxed_Value& t_bv, const std::string& t_name) {
            Name_Validator::validate_object_name(t_name);
            m_engine.add_global(t_bv, t_name);
            return *this;
        }

        ChaiScript_Basic& set_global(const Boxed_Value& t_bv, const std::string& t_name) {
            Name_Validator::validate_object_name(t_name);
            m_engine.set_global(t_bv, t_name);
            return *this;
        }

        /// \brief Represents the current state of the ChaiScript system. State and be saved and restored
        /// \warning State object does not contain the user defined type conversions of the engine. They
        ///          are left out due to performance considerations involved in tracking the state
        /// \sa ChaiScript::get_state
        /// \sa ChaiScript::set_state
        struct State {
            std::set<std::string> used_files;
            chaiscript::detail::Dispatch_Engine::State engine_state;
            std::set<std::string> active_loaded_modules;
        };

        /// \brief Returns a state object that represents the current state of the global system
        ///
        /// The global system includes the reserved words, global const objects, functions and types.
        /// local variables are thread specific and not included.
        ///
        /// \return Current state of the global system
        ///
        /// \b Example:
        ///
        /// \code
        /// chaiscript::ChaiScript chai;
        /// chaiscript::ChaiScript::State s = chai.get_state(); // represents bootstrapped initial state
        /// \endcode
        State get_state() const {
            chaiscript::detail::threading::lock_guard<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);
            AUTO shared_guard = SHARED_GUARD();

            State s;
            s.used_files = m_used_files;
            s.engine_state = m_engine.get_state();
            s.active_loaded_modules = m_active_loaded_modules;
            return s;
        }

        /// \brief Sets the state of the system
        ///
        /// The global system includes the reserved words, global objects, functions and types.
        /// local variables are thread specific and not included.
        ///
        /// \param[in] t_state New state to set
        ///
        /// \b Example:
        /// \code
        /// chaiscript::ChaiScript chai;
        /// chaiscript::ChaiScript::State s = chai.get_state(); // get initial state
        /// chai.add(chaiscript::fun(&somefunction), "somefunction");
        /// chai.set_state(s); // restore initial state, which does not have the recently added "somefunction"
        /// \endcode
        void set_state(const State& t_state) {
            chaiscript::detail::threading::lock_guard<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);
            AUTO shared_guard = SHARED_GUARD();

            m_used_files = t_state.used_files;
            m_active_loaded_modules = t_state.active_loaded_modules;
            m_engine.set_state(t_state.engine_state);
        }

        /// \returns All values in the local thread state, added through the add() function
        std::map<std::string, Boxed_Value> get_locals() const { return m_engine.get_locals(); }

        /// \brief Sets all of the locals for the current thread state.
        ///
        /// \param[in] t_locals The map<name, value> set of variables to replace the current state with
        ///
        /// Any existing locals are removed and the given set of variables is added
        void set_locals(const std::map<std::string, Boxed_Value>& t_locals) { m_engine.set_locals(t_locals); }

        /// \brief Adds a type, function or object to ChaiScript. Objects are added to the local thread state.
        /// \param[in] t_t Item to add
        /// \param[in] t_name Name of item to add
        /// \returns Reference to current ChaiScript object
        ///
        /// \b Examples:
        /// \code
        /// chaiscript::ChaiScript chai;
        /// chai.add(chaiscript::user_type<MyClass>(), "MyClass"); // Add explicit type info (not strictly necessary)
        /// chai.add(chaiscript::fun(&MyClass::function), "function"); // Add a class method
        /// MyClass obj;
        /// chai.add(chaiscript::var(&obj), "obj"); // Add a pointer to a locally defined object
        /// \endcode
        ///
        /// \sa \ref adding_items
        template<typename T>
        ChaiScript_Basic& add(const T& t_t, const std::string& t_name) {
            Name_Validator::validate_object_name(t_name);
            m_engine.add(t_t, t_name);
            return *this;
        }

        ChaiScript_Basic& add(const Proxy_Function& t_t, const std::string& t_name, const std::string& t_description) {
            Name_Validator::validate_object_name(t_name);
            m_engine.add(t_t, t_name, t_description);
            return *this;
        };
        ChaiScript_Basic& add(const Postfix_Definition& t_t) {
            m_engine.add(t_t);
            return *this;
        };


        /// \brief Add a new conversion for upcasting to a base class
        /// \sa chaiscript::base_class
        /// \param[in] d Base class / parent class
        ///
        /// \b Example:
        /// \code
        /// chaiscript::ChaiScript chai;
        /// chai.add(chaiscript::base_class<std::runtime_error, chaiscript::dispatch_error>());
        /// \endcode
        ChaiScript_Basic& add(const Type_Conversion& d) {
            m_engine.add(d);
            return *this;
        }

        /// \brief Adds all elements of a module to ChaiScript runtime
        /// \param[in] t_p The module to add.
        /// \sa chaiscript::Module
        ChaiScript_Basic& add(const ModulePtr& t_p) {
            t_p->apply(*this, this->get_eval_engine());
            return *this;
        }

        /// \brief Load a binary module from a dynamic library. Works on platforms that support
        ///        dynamic libraries.
        /// \param[in] t_module_name Name of the module to load
        ///
        /// The module is searched for in the registered module path folders (chaiscript::ChaiScript::ChaiScript)
        /// and with standard prefixes and postfixes: ("lib"|"")\<t_module_name\>(".dll"|".so"|".bundle"|"").
        ///
        /// Once the file is located, the system looks for the symbol "create_chaiscript_module_\<t_module_name\>".
        /// If no file can be found matching the search criteria and containing the appropriate entry point
        /// (the symbol mentioned above), an exception is thrown.
        ///
        /// \throw chaiscript::exception::load_module_error In the event that no matching module can be found.
        std::string load_module(const std::string& t_module_name) {
#ifdef CHAISCRIPT_NO_DYNLOAD
            throw chaiscript::exception::load_module_error("Loadable module support was disabled (CHAISCRIPT_NO_DYNLOAD)");
#else
            chaiscript::small_vector<exception::load_module_error> errors;
            std::string version_stripped_name = t_module_name;
            size_t version_pos = version_stripped_name.find("-" + Build_Info::version());
            if (version_pos != std::string::npos) {
                version_stripped_name.erase(version_pos);
            }

            chaiscript::small_vector<std::string> prefixes{ "lib", "cyg", "" };

            chaiscript::small_vector<std::string> postfixes{ ".dll", ".so", ".bundle", "" };

            for (auto& elem : m_module_paths) {
                for (auto& prefix : prefixes) {
                    for (auto& postfix : postfixes) {
                        try {
                            const auto name = elem + prefix + t_module_name + postfix;
                            // std::cerr << "trying location: " << name << '\n';
                            load_module(version_stripped_name, name);
                            return name;
                        }
                        catch (const chaiscript::exception::load_module_error& e) {
                            // std::cerr << "error: " << e.what() << '\n';
                            errors.push_back(e);
                            // Try next set
                        }
                    }
                }
            }

            throw chaiscript::exception::load_module_error(t_module_name, errors);
#endif
        }

        /// \brief Load a binary module from a dynamic library. Works on platforms that support
        ///        dynamic libraries.
        ///
        /// \param[in] t_module_name Module name to load
        /// \param[in] t_filename Ignore normal filename search process and use specific filename
        ///
        /// \sa ChaiScript::load_module(const std::string &t_module_name)
        void load_module(const std::string& t_module_name, const std::string& t_filename) {
            chaiscript::detail::threading::lock_guard<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);

            if (m_loaded_modules.count(t_module_name) == 0) {
                detail::Loadable_Module_Ptr lm(new detail::Loadable_Module(t_module_name, t_filename));
                m_loaded_modules[t_module_name] = lm;
                m_active_loaded_modules.insert(t_module_name);
                add(lm->m_moduleptr);
            }
            else if (m_active_loaded_modules.count(t_module_name) == 0) {
                m_active_loaded_modules.insert(t_module_name);
                add(m_loaded_modules[t_module_name]->m_moduleptr);
            }
        }

        /// \brief Evaluates a string. Equivalent to ChaiScript::eval.
        ///
        /// \param[in] t_script Script to execute
        /// \param[in] t_handler Optional Exception_Handler used for automatic unboxing of script thrown exceptions
        ///
        /// \return result of the script execution
        ///
        /// \throw chaiscript::exception::eval_error In the case that evaluation fails.
        Boxed_Value operator()(const std::string& t_script, const Exception_Handler& t_handler = Exception_Handler()) {
            return eval(t_script, t_handler);
        }

        /// \brief Evaluates a string and returns a typesafe result.
        ///
        /// \tparam T Type to extract from the result value of the script execution
        /// \param[in] t_input Script to execute
        /// \param[in] t_handler Optional Exception_Handler used for automatic unboxing of script thrown exceptions
        /// \param[in] t_filename Optional filename to report to the user for where the error occured. Useful
        ///                       in special cases where you are loading a file internally instead of using eval_file
        ///
        /// \return result of the script execution
        ///
        /// \throw chaiscript::exception::eval_error In the case that evaluation fails.
        /// \throw chaiscript::exception::bad_boxed_cast In the case that evaluation succeeds but the result value cannot be converted
        ///        to the requested type.
        template<typename T>
        T eval(const std::string& t_input, const Exception_Handler& t_handler = Exception_Handler(), const std::string& t_filename = "__EVAL__") {
            return m_engine.boxed_cast<T>(eval(t_input, t_handler, t_filename));
        }

        /// \brief casts an object while applying any Dynamic_Conversion available
        template<typename Type>
        decltype(auto) boxed_cast(const Boxed_Value& bv) const {
            return (m_engine.boxed_cast<Type>(bv));
        }

        /// \brief casts an object while applying any Dynamic_Conversion available
        std::string to_string(const Boxed_Value& bv) {
            auto t_ss = chaiscript::detail::Dispatch_State(m_engine, get_eval_engine().earlyExitScript);

            chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);

            std::array<Boxed_Value, 1> params{ bv };

            fpp.save_params(Function_Params{ params });

            std::atomic_uint_fast32_t m_loc = { 0 };

            Boxed_Value returned = t_ss->call_function("to_string", m_loc, Function_Params{ params }, t_ss.conversions());

            return m_engine.boxed_cast<std::string>(returned);
        }

        std::string type_name(const Boxed_Value& bv) {
            auto t_ss = chaiscript::detail::Dispatch_State(m_engine, get_eval_engine().earlyExitScript);

            chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);

            std::array<Boxed_Value, 1> params{ bv };

            fpp.save_params(Function_Params{ params });

            std::atomic_uint_fast32_t m_loc = { 0 };

            Boxed_Value returned = t_ss->call_function("type_name", m_loc, Function_Params{ params }, t_ss.conversions());

            return m_engine.boxed_cast<std::string>(returned);
        }


        Boxed_Value call_lambda(const Boxed_Value& bv) {
            auto t_ss = chaiscript::detail::Dispatch_State(m_engine, get_eval_engine().earlyExitScript);
            chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);

            chaiscript::small_vector<Boxed_Value> params;
            fpp.save_params(Function_Params{ params });
            return (*t_ss->boxed_cast<const dispatch::Proxy_Function_Base*>(bv))(Function_Params{ params }, t_ss.conversions());
        }
    private:
        void unroll_args(chaiscript::small_vector<Boxed_Value>& params) {};
        template<typename... Fargs> void unroll_args(chaiscript::small_vector<Boxed_Value>& params, const Boxed_Value& bv, Fargs... args) { params.push_back(bv); unroll_args(params, args...); };
    public:
        template<typename... Fargs> Boxed_Value call_function(const Boxed_Value& bv, Fargs... args) {
            auto t_ss = chaiscript::detail::Dispatch_State(m_engine, get_eval_engine().earlyExitScript);
            chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);

            chaiscript::small_vector<Boxed_Value> params;
            unroll_args(params, args...);

            fpp.save_params(Function_Params{ params });
            return (*t_ss->boxed_cast<const dispatch::Proxy_Function_Base*>(bv))(Function_Params{ params }, t_ss.conversions());
        }



        /// \brief Evaluates a string.
        ///
        /// \param[in] t_input Script to execute
        /// \param[in] t_handler Optional Exception_Handler used for automatic unboxing of script thrown exceptions
        /// \param[in] t_filename Optional filename to report to the user for where the error occurred. Useful
        ///                       in special cases where you are loading a file internally instead of using eval_file
        ///
        /// \return result of the script execution
        ///
        /// \throw exception::eval_error In the case that evaluation fails.
        Boxed_Value
            eval(
                const std::string& t_input, 
                const Exception_Handler& t_handler = Exception_Handler(), 
                const std::string& t_filename = "__EVAL__"
            ) {
            try {
                return do_eval(t_input, t_filename);
            }
            catch (Boxed_Value& bv) {
                if (t_handler) {
                    t_handler->handle(bv, m_engine);
                }
                throw;
            }
        }

        /// \brief Loads the file specified by filename, evaluates it, and returns the result.
        /// \param[in] t_filename File to load and parse.
        /// \param[in] t_handler Optional Exception_Handler used for automatic unboxing of script thrown exceptions
        /// \return result of the script execution
        /// \throw chaiscript::exception::eval_error In the case that evaluation fails.
        Boxed_Value eval_file(const std::string& t_filename, const Exception_Handler& t_handler = Exception_Handler()) {
            return eval(load_file(t_filename), t_handler, t_filename);
        }

        /// \brief Loads the file specified by filename, evaluates it, and returns the type safe result.
        /// \tparam T Type to extract from the result value of the script execution
        /// \param[in] t_filename File to load and parse.
        /// \param[in] t_handler Optional Exception_Handler used for automatic unboxing of script thrown exceptions
        /// \return result of the script execution
        /// \throw chaiscript::exception::eval_error In the case that evaluation fails.
        /// \throw chaiscript::exception::bad_boxed_cast In the case that evaluation succeeds but the result value cannot be converted
        ///        to the requested type.
        template<typename T>
        T eval_file(const std::string& t_filename, const Exception_Handler& t_handler = Exception_Handler()) {
            return m_engine.boxed_cast<T>(eval_file(t_filename, t_handler));
        }

        /// \brief Imports a namespace object into the global scope of this ChaiScript instance.
        /// \param[in] t_namespace_name Name of the namespace to import.
        /// \throw std::runtime_error In the case that the namespace name was never registered.
        void import(const std::string& t_namespace_name) {
            chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);

            if (m_engine.get_scripting_objects().count(t_namespace_name)) {
                throw std::runtime_error("Namespace: " + t_namespace_name + " was already defined");
            }
            else if (m_namespace_generators.count(t_namespace_name)) {
                m_engine.add_global(var(std::ref(m_namespace_generators[t_namespace_name]())), t_namespace_name);
            }
            else {
                throw std::runtime_error("No registered namespace: " + t_namespace_name);
            }
        }

        /// \brief Registers a namespace generator, which delays generation of the namespace until it is imported, saving memory if it is never
        /// used. \param[in] t_namespace_generator Namespace generator function. \param[in] t_namespace_name Name of the Namespace function
        /// being registered. \throw std::runtime_error In the case that the namespace name was already registered.
        void register_namespace(const std::function<void(Namespace&)>& t_namespace_generator, const std::string& t_namespace_name) {
            chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);

            if (!m_namespace_generators.count(t_namespace_name)) {
                // contain the namespace object memory within the m_namespace_generators map
                m_namespace_generators.emplace(std::make_pair(t_namespace_name, [=, space = Namespace()]() mutable->Namespace& {
                    t_namespace_generator(space);
                    return space;
                }));
            }
            else {
                throw std::runtime_error("Namespace: " + t_namespace_name + " was already registered.");
            }
        }
    };
} // namespace chaiscript