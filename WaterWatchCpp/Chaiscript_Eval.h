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
#include "cweeJob.h"
#include "../FiberTasks/Fibers.h"

namespace chaiscript::exception {
    class bad_boxed_cast;
} // namespace chaiscript::exception

namespace chaiscript {
    //static std::atomic<bool> earlyExit(false);
    //void EarlyExitCheck();
    //__forceinline void EarlyExitCheck() {
    //    if (earlyExit.load()) {
    //        earlyExit.store(false);
    //        throw(exception::eval_error("Exited Mid-Script Due to Early Exit Request."));
    //    }
    //}
    void EarlyExitCheck(const chaiscript::detail::Dispatch_State& t_e);
    __forceinline void EarlyExitCheck(const chaiscript::detail::Dispatch_State& t_e) {
        if (t_e.earlyExitScript && t_e.earlyExitScript->load()) {
            t_e.earlyExitScript->store(false);
            throw(exception::eval_error("Exited Mid-Script Due to Early Exit Request."));
        }
    };

    /// \brief Classes and functions that are part of the runtime eval system
    namespace eval {
        template<typename T>
        struct AST_Node_Impl;

        template<typename T>
        using AST_Node_Impl_Ptr = typename chaiscript::unique_ptr<AST_Node_Impl<T>>;

        namespace detail {
            /// Helper function that will set up the scope around a function call, including handling the named function parameters
            template<typename T>
            Boxed_Value eval_function(chaiscript::detail::Dispatch_Engine& t_ss,
                const AST_Node_Impl<T>& t_node,
                const chaiscript::small_vector<std::string>& t_param_names,
                const Function_Params& t_vals,
                const std::map<std::string, Boxed_Value>* t_locals = nullptr,
                bool has_this_capture = false) {
                chaiscript::detail::Dispatch_State state(t_ss);

                const Boxed_Value* thisobj = [&]() -> const Boxed_Value* {
                    if (auto& stack = t_ss.get_stack_data(state.stack_holder()).back(); !stack.empty() && stack.back().first == "__this") {
#ifdef UseCweeThreadedMapAsQuickFlatMap
                        return &*stack.back().second;
#else
                        return &stack.back().second;
#endif
                    }
                    else if (!t_vals.empty()) {
                        return &t_vals[0];
                    }
                    else {
                        return nullptr;
                    }
                }();

                chaiscript::eval::detail::Stack_Push_Pop tpp(state);
                if (thisobj && !has_this_capture) {
                    state.add_object("this", *thisobj);
                }

                if (t_locals) {
                    for (const auto& [name, value] : *t_locals) {
                        state.add_object(name, value);
                    }
                }

                for (size_t i = 0; i < t_param_names.size(); ++i) {
                    if (t_param_names[i] != "this") {
                        state.add_object(t_param_names[i], t_vals[i]);
                    }
                }

                try {
                    return t_node.eval(state);
                }
                catch (detail::Return_Value& rv) {
                    return std::move(rv.retval);
                }
            }

            inline Boxed_Value clone_if_necessary(Boxed_Value incoming, std::atomic_uint_fast32_t& t_loc, const chaiscript::detail::Dispatch_State& t_ss) {
                if (incoming.is_null()) {
                    incoming.reset_return_value();
                    return incoming;
                } else if (!incoming.is_return_value()) {
                    if (incoming.get_type_info().is_arithmetic()) {
                        return Boxed_Number::clone(incoming);
                    }
                    else if (incoming.get_type_info().bare_equal_type_info(typeid(bool))) {
                        return Boxed_Value(*static_cast<const bool*>(incoming.get_const_ptr()));
                    }
                    else if (incoming.get_type_info().bare_equal_type_info(typeid(std::string))) {
                        return Boxed_Value(*static_cast<const std::string*>(incoming.get_const_ptr()));
                    }
                    else {
                        std::array<Boxed_Value, 1> params{ std::move(incoming) };
                        return t_ss->call_function("clone", t_loc, Function_Params{ params }, t_ss.conversions());
                    }
                }
                else {
                    incoming.reset_return_value();
                    return incoming;
                }
            }
        } // namespace detail

        template<typename T>
        struct AST_Node_Impl : AST_Node {
            AST_Node_Impl(std::string t_ast_node_text,
                AST_Node_Type t_id,
                Parse_Location t_loc,
                chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children = chaiscript::small_vector<AST_Node_Impl_Ptr<T>>())
                : AST_Node(std::move(t_ast_node_text), t_id, std::move(t_loc))
                , children(std::move(t_children)) {
            }

            static bool get_scoped_bool_condition(const AST_Node_Impl<T>& node, const chaiscript::detail::Dispatch_State& t_ss) {
                chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);
                return get_bool_condition(node.eval(t_ss), t_ss);
            }

            chaiscript::small_vector<std::reference_wrapper<AST_Node>> get_children() const override final {
                chaiscript::small_vector<std::reference_wrapper<AST_Node>> retval;
                retval.reserve(children.size());
                for (const AST_Node_Impl_Ptr<T>& child : children) {
                    retval.emplace_back(*child);
                }

                return retval;
            }

            Boxed_Value eval(const chaiscript::detail::Dispatch_State& t_e) const override final {
                try {
                    T::trace(t_e, this);
                    EarlyExitCheck(t_e);
                    return eval_internal(t_e);
                } 
                catch (exception::eval_error &ee) {
                  ee.call_stack.push_back(*this);
                  // RG -->
                  //return Boxed_Value(ee);       
                  throw ee; // previously threw   
                  // <-- RG 
                }
                catch (std::runtime_error &ee) {
                    auto e = exception::eval_error(ee.what(), this->location.start, "Compiled C++ Function" /*, {}, {}, false, t_e*/ );
                    e.call_stack.push_back(*this);
                    throw e;
                }
                catch (std::exception &ee) {
                    auto e = exception::eval_error(ee.what(), this->location.start, "Compiled C++ Function" /*, {}, {}, false, t_e*/);
                    e.call_stack.push_back(*this);
                    throw e;
                }
            }

            chaiscript::small_vector<AST_Node_Impl_Ptr<T>> children;

        protected:
            virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State&) const {
                throw std::runtime_error("Undispatched ast_node (internal error)");
            }
        };

        template<typename T>
        struct Compiled_AST_Node : AST_Node_Impl<T> {
            Compiled_AST_Node(AST_Node_Impl_Ptr<T> t_original_node,
                chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children,
                std::function<Boxed_Value(const chaiscript::small_vector<AST_Node_Impl_Ptr<T>>&, const chaiscript::detail::Dispatch_State& t_ss)> t_func)
                : AST_Node_Impl<T>(t_original_node->text, AST_Node_Type::Compiled, t_original_node->location, std::move(t_children))
                , m_func(std::move(t_func))
                , m_original_node(std::move(t_original_node)) {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Compiled, true); // compiled functions return nothing
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override { EarlyExitCheck(t_ss); return m_func(this->children, t_ss); }

            std::function<Boxed_Value(const chaiscript::small_vector<AST_Node_Impl_Ptr<T>>&, const chaiscript::detail::Dispatch_State& t_ss)> m_func;
            AST_Node_Impl_Ptr<T> m_original_node;
        };

        template<typename T>
        struct Fold_Right_Binary_Operator_AST_Node : AST_Node_Impl<T> {
            Fold_Right_Binary_Operator_AST_Node(const std::string& t_oper, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children, Boxed_Value t_rhs)
                : AST_Node_Impl<T>(t_oper, AST_Node_Type::Binary, std::move(t_loc), std::move(t_children))
                , m_oper(Operators::to_operator(t_oper))
                , m_rhs(std::move(t_rhs)) {



                if (m_oper != Operators::Opers::invalid &&
                    this->children[0]->potentialReturnType.Type().is_arithmetic() &&
                    this->children[1]->potentialReturnType.Type().is_arithmetic()
                    ) {
                    // normal numeric mathematics
                    switch (m_oper) {
                    case Operators::Opers::shift_left:
                    case Operators::Opers::shift_right:
                    case Operators::Opers::remainder:
                    case Operators::Opers::bitwise_and:
                    case Operators::Opers::bitwise_or:
                    case Operators::Opers::bitwise_xor:
                    case Operators::Opers::bitwise_complement:
                    case Operators::Opers::sum:
                    case Operators::Opers::quotient:
                    case Operators::Opers::product:
                    case Operators::Opers::difference:
                    case Operators::Opers::unary_plus:
                    case Operators::Opers::unary_minus:
                    case Operators::Opers::pre_increment:
                    case Operators::Opers::pre_decrement:
                    case Operators::Opers::assign_product:
                    case Operators::Opers::assign_sum:
                    case Operators::Opers::assign_quotient:
                    case Operators::Opers::assign_difference:
                    case Operators::Opers::assign_bitwise_and:
                    case Operators::Opers::assign_bitwise_or:
                    case Operators::Opers::assign_shift_left:
                    case Operators::Opers::assign_shift_right:
                    case Operators::Opers::assign_remainder:
                    case Operators::Opers::assign_bitwise_xor:
                    case Operators::Opers::assign_if_null:
                    case Operators::Opers::assign:
                        this->potentialReturnType.ForwardRef(this->children[this->children.size() - 1]->potentialReturnType);
                        break;
                    case Operators::Opers::logical_list:
                        this->potentialReturnType = ReturnType(user_type<std::vector<Boxed_Value>>(), AST_Node_Type::Binary, true); break;
                    case Operators::Opers::invalid:
                    case Operators::Opers::equals:
                    case Operators::Opers::less_than:
                    case Operators::Opers::greater_than:
                    case Operators::Opers::less_than_equal:
                    case Operators::Opers::greater_than_equal:
                    case Operators::Opers::not_equal:
                    default:
                        this->potentialReturnType = ReturnType(user_type<bool>(), AST_Node_Type::Binary, true); break;
                    }
                }
                else if ((m_oper == Operators::Opers::assign) || (this->text == ":=") || (m_oper == Operators::Opers::assign_if_null || this->text == "?=")) {
                    // assignment of left term to the type of the right term. 
                    this->potentialReturnType.ForwardRef(this->children[this->children.size() - 1]->potentialReturnType);
                    this->children[0]->potentialReturnType.ForwardRef(this->potentialReturnType);
                }
                else {
                    // could be a number of "functions" such as ==, !=, -, +, etc.
                    switch (m_oper) {
                    case Operators::Opers::shift_left:
                    case Operators::Opers::shift_right:
                    case Operators::Opers::remainder:
                    case Operators::Opers::bitwise_and:
                    case Operators::Opers::bitwise_or:
                    case Operators::Opers::bitwise_xor:
                    case Operators::Opers::bitwise_complement:
                    case Operators::Opers::sum:
                    case Operators::Opers::quotient:
                    case Operators::Opers::product:
                    case Operators::Opers::difference:
                    case Operators::Opers::unary_plus:
                    case Operators::Opers::unary_minus:
                    case Operators::Opers::pre_increment:
                    case Operators::Opers::pre_decrement:
                    case Operators::Opers::assign_product:
                    case Operators::Opers::assign_sum:
                    case Operators::Opers::assign_quotient:
                    case Operators::Opers::assign_difference:
                    case Operators::Opers::assign_bitwise_and:
                    case Operators::Opers::assign_bitwise_or:
                    case Operators::Opers::assign_shift_left:
                    case Operators::Opers::assign_shift_right:
                    case Operators::Opers::assign_remainder:
                    case Operators::Opers::assign_bitwise_xor:
                    case Operators::Opers::assign_if_null:
                    case Operators::Opers::assign:
                        this->potentialReturnType.ForwardRef(this->children[this->children.size() - 1]->potentialReturnType); break;
                    case Operators::Opers::logical_list:
                        this->potentialReturnType = ReturnType(user_type<std::vector<Boxed_Value>>(), AST_Node_Type::Binary, true); break;
                    case Operators::Opers::invalid:
                    case Operators::Opers::equals:
                    case Operators::Opers::less_than:
                    case Operators::Opers::greater_than:
                    case Operators::Opers::less_than_equal:
                    case Operators::Opers::greater_than_equal:
                    case Operators::Opers::not_equal:
                    default:
                        this->potentialReturnType = ReturnType(user_type<bool>(), AST_Node_Type::Binary, true); break;
                    }
                }
                // this->potentialReturnType = ReturnType(m_rhs.get_type_info(), AST_Node_Type::Binary, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                return do_oper(t_ss, this->text, this->children[0]->eval(t_ss));
            }

        protected:
            Boxed_Value do_oper(const chaiscript::detail::Dispatch_State& t_ss, const std::string& t_oper_string, const Boxed_Value& t_lhs) const {
                try {
                    if (t_lhs.get_type_info().is_arithmetic()) {
                        // If it's an arithmetic operation we want to short circuit dispatch
                        try {
                            return Boxed_Number::do_oper(m_oper, t_lhs, m_rhs);
                        }
                        catch (const chaiscript::exception::arithmetic_error&) {
                            throw;
                        }
                        catch (...) {
                            throw exception::eval_error("Error with numeric operator calling: " + t_oper_string);
                        }
                    }
                    else {
                        chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
                        std::array<Boxed_Value, 2> params{ t_lhs, m_rhs };
                        fpp.save_params(Function_Params{ params });
                        return t_ss->call_function(t_oper_string, m_loc, Function_Params{ params }, t_ss.conversions());
                    }
                }
                catch (const exception::dispatch_error& e) {
                    throw exception::eval_error("Can not find appropriate '" + t_oper_string + "' operator.", e.parameters, e.functions, false, *t_ss);
                }
            }

        private:
            Operators::Opers m_oper;
            Boxed_Value m_rhs;
            mutable std::atomic_uint_fast32_t m_loc = { 0 };
        };

        template<typename T>
        struct Binary_Operator_AST_Node : AST_Node_Impl<T> {
            Binary_Operator_AST_Node(const std::string& t_oper, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(t_oper, AST_Node_Type::Binary, std::move(t_loc), std::move(t_children))
                , m_oper(Operators::to_operator(t_oper)) {
                
                if (m_oper != Operators::Opers::invalid &&
                    this->children[0]->potentialReturnType.Type().is_arithmetic() &&
                    this->children[1]->potentialReturnType.Type().is_arithmetic()
                    ) {
                    // normal numeric mathematics
                    switch (m_oper) {
                    case Operators::Opers::shift_left:
                    case Operators::Opers::shift_right:
                    case Operators::Opers::remainder:
                    case Operators::Opers::bitwise_and:
                    case Operators::Opers::bitwise_or:
                    case Operators::Opers::bitwise_xor:
                    case Operators::Opers::bitwise_complement:
                    case Operators::Opers::sum:
                    case Operators::Opers::quotient:
                    case Operators::Opers::product:
                    case Operators::Opers::difference:
                    case Operators::Opers::unary_plus:
                    case Operators::Opers::unary_minus:
                    case Operators::Opers::pre_increment:
                    case Operators::Opers::pre_decrement:
                    case Operators::Opers::assign_product:
                    case Operators::Opers::assign_sum:
                    case Operators::Opers::assign_quotient:
                    case Operators::Opers::assign_difference:
                    case Operators::Opers::assign_bitwise_and:
                    case Operators::Opers::assign_bitwise_or:
                    case Operators::Opers::assign_shift_left:
                    case Operators::Opers::assign_shift_right:
                    case Operators::Opers::assign_remainder:
                    case Operators::Opers::assign_bitwise_xor:
                    case Operators::Opers::assign_if_null:
                    case Operators::Opers::assign:
                        this->potentialReturnType.ForwardRef(this->children[this->children.size() - 1]->potentialReturnType);
                        break;
                    case Operators::Opers::logical_list:
                        this->potentialReturnType = ReturnType(user_type<std::vector<Boxed_Value>>(), AST_Node_Type::Binary, true); break;
                    case Operators::Opers::invalid:
                    case Operators::Opers::equals:
                    case Operators::Opers::less_than:
                    case Operators::Opers::greater_than:
                    case Operators::Opers::less_than_equal:
                    case Operators::Opers::greater_than_equal:
                    case Operators::Opers::not_equal:
                    default:
                        this->potentialReturnType = ReturnType(user_type<bool>(), AST_Node_Type::Binary, true); break;
                    }
                }
                else if ((m_oper == Operators::Opers::assign) || (this->text == ":=") || (m_oper == Operators::Opers::assign_if_null || this->text == "?=")) {
                    // assignment of left term to the type of the right term. 
                    this->potentialReturnType.ForwardRef(this->children[this->children.size() - 1]->potentialReturnType);
                    this->children[0]->potentialReturnType.ForwardRef(this->potentialReturnType);
                }
                else {
                    // could be a number of "functions" such as ==, !=, -, +, etc.
                    switch (m_oper) {
                    case Operators::Opers::shift_left:
                    case Operators::Opers::shift_right:
                    case Operators::Opers::remainder:
                    case Operators::Opers::bitwise_and:
                    case Operators::Opers::bitwise_or:
                    case Operators::Opers::bitwise_xor:
                    case Operators::Opers::bitwise_complement:
                    case Operators::Opers::sum:
                    case Operators::Opers::quotient:
                    case Operators::Opers::product:
                    case Operators::Opers::difference:
                    case Operators::Opers::unary_plus:
                    case Operators::Opers::unary_minus:
                    case Operators::Opers::pre_increment:
                    case Operators::Opers::pre_decrement:
                    case Operators::Opers::assign_product:
                    case Operators::Opers::assign_sum:
                    case Operators::Opers::assign_quotient:
                    case Operators::Opers::assign_difference:
                    case Operators::Opers::assign_bitwise_and:
                    case Operators::Opers::assign_bitwise_or:
                    case Operators::Opers::assign_shift_left:
                    case Operators::Opers::assign_shift_right:
                    case Operators::Opers::assign_remainder:
                    case Operators::Opers::assign_bitwise_xor:
                        this->potentialReturnType.ForwardRef(this->children[this->children.size() - 1]->potentialReturnType); break;
                    case Operators::Opers::logical_list:
                        this->potentialReturnType = ReturnType(user_type<std::vector<Boxed_Value>>(), AST_Node_Type::Binary, true); break;
                    case Operators::Opers::invalid:
                    case Operators::Opers::equals:
                    case Operators::Opers::less_than:
                    case Operators::Opers::greater_than:
                    case Operators::Opers::less_than_equal:
                    case Operators::Opers::greater_than_equal:
                    case Operators::Opers::not_equal:
                    default:
                        this->potentialReturnType = ReturnType(user_type<bool>(), AST_Node_Type::Binary, true); break;
                    case Operators::Opers::assign_if_null:
                    case Operators::Opers::assign:
                        throw std::runtime_error("Something went wrong"); break;
                    }
                }
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                auto lhs = this->children[0]->eval(t_ss);
                auto rhs = this->children[1]->eval(t_ss);
                return do_oper(t_ss, m_oper, this->text, lhs, rhs);
            }

        protected:
            Boxed_Value do_oper(const chaiscript::detail::Dispatch_State& t_ss,
                Operators::Opers t_oper,
                const std::string& t_oper_string,
                const Boxed_Value& t_lhs,
                const Boxed_Value& t_rhs) const {
                try {
                    if (t_oper != Operators::Opers::invalid && t_lhs.get_type_info().is_arithmetic() && t_rhs.get_type_info().is_arithmetic()) {
                        // If it's an arithmetic operation we want to short circuit dispatch
                        try {
                            return Boxed_Number::do_oper(t_oper, t_lhs, t_rhs);
                        }
                        catch (const chaiscript::exception::arithmetic_error&) {
                            throw;
                        }
                        catch (...) {
                            throw exception::eval_error("Error with numeric operator calling: " + t_oper_string);
                        }
                    }
                    else {
                        chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
                        std::array<Boxed_Value, 2> params{ t_lhs, t_rhs };
                        fpp.save_params(Function_Params(params));
                        return t_ss->call_function(t_oper_string, m_loc, Function_Params(params), t_ss.conversions());
                    }
                }
                catch (const exception::dispatch_error& e) {
                    throw exception::eval_error("Can not find appropriate '" + t_oper_string + "' operator.", e.parameters, e.functions, false, *t_ss);
                }
            }

        private:
            Operators::Opers m_oper;
            mutable std::atomic_uint_fast32_t m_loc = { 0 };
        };

        template<typename T>
        struct Constant_AST_Node final : AST_Node_Impl<T> {
            Constant_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, Boxed_Value t_value)
                : AST_Node_Impl<T>(t_ast_node_text, AST_Node_Type::Constant, std::move(t_loc))
                , m_value(std::move(t_value)) {
                this->potentialReturnType = ReturnType(m_value.get_type_info(), AST_Node_Type::Constant, true);
            }

            explicit Constant_AST_Node(Boxed_Value t_value)
                : AST_Node_Impl<T>("", AST_Node_Type::Constant, Parse_Location())
                , m_value(std::move(t_value)) {
                this->potentialReturnType = ReturnType(m_value.get_type_info(), AST_Node_Type::Constant, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State&) const override { return m_value; }

            Boxed_Value m_value;
        };

        template<typename T>
        struct Id_AST_Node final : AST_Node_Impl<T> {
            Id_AST_Node(const std::string& t_ast_node_text, Parse_Location t_loc)
                : AST_Node_Impl<T>(t_ast_node_text, AST_Node_Type::Id, std::move(t_loc)) {
                this->potentialReturnType = ReturnType(Type_Info(), AST_Node_Type::Id, false);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                try {
                    return t_ss.get_object(this->text, m_loc);
                }
                catch (std::exception&) {
                    throw exception::eval_error("Can not find object: " + this->text);
                }
            }

        private:
            mutable std::atomic_uint_fast32_t m_loc = { 0 };
        };

        template<typename T>
        INLINE static bool TryGetIdNode(AST_Node const& m, Id_AST_Node<T>*& out) {
            if (m.identifier == AST_Node_Type::Id) {
                chaiscript::eval::Id_AST_Node<T>* postfixNodePointer = dynamic_cast<chaiscript::eval::Id_AST_Node<T>*>(const_cast<AST_Node*>(&m));
                if (postfixNodePointer) {
                    out = postfixNodePointer;
                }
                return true;
            }

            AUTO childs = m.get_children(); // search backwards / deepest first
            bool toReturn = false;
            for (int i = childs.size() - 1; i >= 0; i--) {
                if (TryGetIdNode(childs[i], out)) {
                    toReturn = true;
                }
            }
            return toReturn;
        };

        template<typename T>
        struct Fun_Call_AST_Node : AST_Node_Impl<T> {
            Fun_Call_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Fun_Call, std::move(t_loc), std::move(t_children)) {
                assert(!this->children.empty());

                this->potentialReturnType = ReturnType(Type_Info(), AST_Node_Type::Fun_Call, false);
                this->children[0]->potentialReturnType.ForwardRef(this->potentialReturnType); // this child is permanently set, unless it was already set!
            }

            template<bool Save_Params>
            Boxed_Value do_eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const {
                EarlyExitCheck(t_ss);
                chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);

                chaiscript::small_vector<Boxed_Value> params;

                params.reserve(this->children[1]->children.size());
                for (const auto& child : this->children[1]->children) {
                    params.push_back(child->eval(t_ss));
                }

                if (Save_Params) {
                    fpp.save_params(Function_Params{ params });
                }
                Boxed_Value fn;
                try {
                    fn = this->children[0]->eval(t_ss);
                }
                catch (const exception::dispatch_error e) {
                    std::pair<std::string, Proxy_Function> tryFind = t_ss->get_function_with_similar_name(this->children[0]->text, e.parameters, t_ss.conversions());
                    if (tryFind.first != "" && tryFind.first != this->children[0]->text) {
                        throw exception::eval_error("'" + this->children[0]->text + "' is not a function. Did you mean '" + tryFind.first + "' instead?");
                    }
                    
                    throw e;
                } catch (chaiscript::exception::eval_error e) {
                    // object was not found? Potentially. Suggest a new name in this case... 
                    std::pair<std::string, Proxy_Function> tryFind = t_ss->get_function_with_similar_name(this->children[0]->text, params, t_ss.conversions());
                    if (tryFind.first != "" && tryFind.first != this->children[0]->text) {
                        throw exception::eval_error("'" + this->children[0]->text + "' is not a function. Did you mean '" + tryFind.first + "' instead?");
                    }
                    
                    throw e;
                }

                try {
                    return (*t_ss->boxed_cast<const dispatch::Proxy_Function_Base*>(fn))(Function_Params{ params }, t_ss.conversions());
                }
                catch (const exception::dispatch_error& e) {
                    throw exception::eval_error(std::string(e.what()) + " with function '" + this->children[0]->text + "'",
                        e.parameters,
                        e.functions,
                        false,
                        *t_ss);
                }
                catch (const exception::bad_boxed_cast& e) {
                    try {
                        using ConstFunctionTypeRef = const Const_Proxy_Function&;
                        Const_Proxy_Function f = t_ss->boxed_cast<ConstFunctionTypeRef>(fn);

                        // handle the case where there is only 1 function to try to call and dispatch fails on it
                        throw exception::eval_error(std::string(e.what()) + " with function '" + this->children[0]->text + "'", params, make_vector(f), false, *t_ss);                      
                    }
                    catch (const exception::bad_boxed_cast&) {
                        throw exception::eval_error("'" + this->children[0]->pretty_print() + "' does not evaluate to a function.");
                    }
                }
                catch (const exception::arity_error& e) {
                    throw exception::eval_error(std::string(e.what()) + " with function '" + this->children[0]->text + "'");
                }
                catch (const exception::guard_error& e) {
                    throw exception::eval_error(std::string(e.what()) + " with function '" + this->children[0]->text + "'");
                }
                catch (detail::Return_Value& rv) {
                    return rv.retval;
                }
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override { EarlyExitCheck(t_ss); return do_eval_internal<true>(t_ss); }
        };

        template<typename T>
        struct Unused_Return_Fun_Call_AST_Node final : Fun_Call_AST_Node<T> {
            Unused_Return_Fun_Call_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : Fun_Call_AST_Node<T>(std::move(t_ast_node_text), std::move(t_loc), std::move(t_children)) {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Unused_Return_Fun_Call, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                return this->template do_eval_internal<false>(t_ss);
            }
        };

        template<typename T>
        struct Arg_AST_Node final : AST_Node_Impl<T> {
            Arg_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Arg_List, std::move(t_loc), std::move(t_children)) {
                if (this->children.size() > 0) {
                    this->potentialReturnType.ForwardRef(this->children[this->children.size() - 1]->potentialReturnType);
                }
                else {
                    this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Arg_List, true);
                }
            }
        };

        template<typename T>
        struct Arg_List_AST_Node final : AST_Node_Impl<T> {
            Arg_List_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Arg_List, std::move(t_loc), std::move(t_children)) {                
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Arg_List, true);
            }

            static std::string get_arg_name(const AST_Node_Impl<T>& t_node) {
                if (t_node.children.empty()) {
                    return t_node.text;
                }
                else if (t_node.children.size() == 1) {
                    return t_node.children[0]->text;
                }
                else {
                    return t_node.children[1]->text;
                }
            }

            static chaiscript::small_vector<std::string> get_arg_names(const AST_Node_Impl<T>& t_node) {
                chaiscript::small_vector<std::string> retval;

                for (const auto& node : t_node.children) {
                    retval.push_back(get_arg_name(*node));
                }

                return retval;
            }

            static std::pair<std::string, Type_Info> get_arg_type(const AST_Node_Impl<T>& t_node, const chaiscript::detail::Dispatch_State& t_ss) {
                if (t_node.children.size() < 2) {
                    return {};
                }
                else {
                    return { t_node.children[0]->text, t_ss->get_type(t_node.children[0]->text, false) };
                }
            }

            static dispatch::Param_Types get_arg_types(const AST_Node_Impl<T>& t_node, const chaiscript::detail::Dispatch_State& t_ss) {
                chaiscript::small_vector<std::pair<std::string, Type_Info>> retval;

                for (const auto& child : t_node.children) {
                    retval.push_back(get_arg_type(*child, t_ss));
                }

                return dispatch::Param_Types(std::move(retval));
            }
        };

        template<typename T>
        struct Equation_AST_Node final : AST_Node_Impl<T> {
            Equation_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Equation, std::move(t_loc), std::move(t_children))
                , m_oper(Operators::to_operator(this->text)) {
                assert(this->children.size() == 2);

                if (m_oper != Operators::Opers::invalid && 
                    this->children[0]->potentialReturnType.Type().is_arithmetic() && 
                    this->children[1]->potentialReturnType.Type().is_arithmetic()
                    ) {
                    // normal numeric mathematics
                    switch (m_oper) {
                    case Operators::Opers::shift_left:
                    case Operators::Opers::shift_right:
                    case Operators::Opers::remainder:
                    case Operators::Opers::bitwise_and:
                    case Operators::Opers::bitwise_or:
                    case Operators::Opers::bitwise_xor:
                    case Operators::Opers::bitwise_complement:
                    case Operators::Opers::sum:
                    case Operators::Opers::quotient:
                    case Operators::Opers::product:
                    case Operators::Opers::difference:
                    case Operators::Opers::unary_plus:
                    case Operators::Opers::unary_minus:
                    case Operators::Opers::pre_increment:
                    case Operators::Opers::pre_decrement:
                    case Operators::Opers::assign_product:
                    case Operators::Opers::assign_sum:
                    case Operators::Opers::assign_quotient:
                    case Operators::Opers::assign_difference:
                    case Operators::Opers::assign_bitwise_and:
                    case Operators::Opers::assign_bitwise_or:
                    case Operators::Opers::assign_shift_left:
                    case Operators::Opers::assign_shift_right:
                    case Operators::Opers::assign_remainder:
                    case Operators::Opers::assign_bitwise_xor:
                    case Operators::Opers::assign_if_null:
                    case Operators::Opers::assign:
                        this->potentialReturnType.ForwardRef(this->children[this->children.size()-1]->potentialReturnType); 
                        break;
                    case Operators::Opers::logical_list:
                        this->potentialReturnType = ReturnType(user_type<std::vector<Boxed_Value>>(), AST_Node_Type::Equation, true); break;
                    case Operators::Opers::invalid:
                    case Operators::Opers::equals:
                    case Operators::Opers::less_than:
                    case Operators::Opers::greater_than:
                    case Operators::Opers::less_than_equal:
                    case Operators::Opers::greater_than_equal:
                    case Operators::Opers::not_equal:
                    default:
                        this->potentialReturnType = ReturnType(user_type<bool>(), AST_Node_Type::Equation, true); break;
                    }
                }
                else if ((m_oper == Operators::Opers::assign) || (this->text == ":=") || (m_oper == Operators::Opers::assign_if_null || this->text == "?=")) {
                    // assignment of left term to the type of the right term. 
                    this->potentialReturnType.ForwardRef(this->children[this->children.size() - 1]->potentialReturnType); 
                    this->children[0]->potentialReturnType.ForwardRef(this->potentialReturnType);
                    //this->children[0]->potentialReturnType = this->potentialReturnType;
                }
                else {
                    // could be a number of "functions" such as ==, !=, -, +, etc.
                    switch (m_oper) {
                    case Operators::Opers::shift_left:
                    case Operators::Opers::shift_right:
                    case Operators::Opers::remainder:
                    case Operators::Opers::bitwise_and:
                    case Operators::Opers::bitwise_or:
                    case Operators::Opers::bitwise_xor:
                    case Operators::Opers::bitwise_complement:
                    case Operators::Opers::sum:
                    case Operators::Opers::quotient:
                    case Operators::Opers::product:
                    case Operators::Opers::difference:
                    case Operators::Opers::unary_plus:
                    case Operators::Opers::unary_minus:
                    case Operators::Opers::pre_increment:
                    case Operators::Opers::pre_decrement:
                    case Operators::Opers::assign_product:
                    case Operators::Opers::assign_sum:
                    case Operators::Opers::assign_quotient:
                    case Operators::Opers::assign_difference:
                    case Operators::Opers::assign_bitwise_and:
                    case Operators::Opers::assign_bitwise_or:
                    case Operators::Opers::assign_shift_left:
                    case Operators::Opers::assign_shift_right:
                    case Operators::Opers::assign_remainder:
                    case Operators::Opers::assign_bitwise_xor:
                        this->potentialReturnType.ForwardRef(this->children[this->children.size() - 1]->potentialReturnType); break;
                    case Operators::Opers::logical_list:
                        this->potentialReturnType = ReturnType(user_type<std::vector<Boxed_Value>>(), AST_Node_Type::Equation, true); break;
                    case Operators::Opers::invalid:
                    case Operators::Opers::equals:
                    case Operators::Opers::less_than:
                    case Operators::Opers::greater_than:
                    case Operators::Opers::less_than_equal:
                    case Operators::Opers::greater_than_equal:
                    case Operators::Opers::not_equal:
                    default:
                        this->potentialReturnType = ReturnType(user_type<bool>(), AST_Node_Type::Equation, true); break;
                    case Operators::Opers::assign_if_null:
                    case Operators::Opers::assign:
                        throw std::runtime_error("Something went wrong"); break;
                    }
                }
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);

                chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);

                cweeSharedPtr< std::array<Boxed_Value, 2> > Pptr;
                if (m_oper == Operators::Opers::assign_if_null || this->text == "?=") {
                    auto lhs = this->children[0]->eval(t_ss);
                    if (!lhs.is_undef()) {
                        return lhs;
                    }
                    auto rhs = this->children[1]->eval(t_ss);
                    std::array<Boxed_Value, 2> p{ std::move(lhs), std::move(rhs) };
                    Pptr = make_cwee_shared< std::array<Boxed_Value, 2> >(std::move(p));
                }
                if (!Pptr) {
                    auto rhs = this->children[1]->eval(t_ss);
                    auto lhs = this->children[0]->eval(t_ss);
                    std::array<Boxed_Value, 2> p{ std::move(lhs), std::move(rhs) };
                    Pptr = make_cwee_shared< std::array<Boxed_Value, 2> >(std::move(p));
                }

                auto& params = *Pptr;

                if (params[0].is_return_value()) {
                    throw exception::eval_error("Error, return value cannot be assigned to.");
                }
                else if (params[0].is_const() && m_oper != Operators::Opers::assign_if_null) {
                    throw exception::eval_error("Error, constant value cannot be assigned to.");
                }

                if (m_oper != Operators::Opers::invalid && params[0].get_type_info().is_arithmetic() && params[1].get_type_info().is_arithmetic()) {
                    try {
                        return Boxed_Number::do_oper(m_oper, params[0], params[1]);
                    }
                    catch (const std::exception&) {
                        throw exception::eval_error("Error with unsupported arithmetic assignment operation.");
                    }
                }
                else if (m_oper == Operators::Opers::assign) {
                    try {
                        if (params[0].is_undef()) {
                            if (!this->children.empty()
                                && ((this->children[0]->identifier == AST_Node_Type::Reference)
                                    || (!this->children[0]->children.empty() && this->children[0]->children[0]->identifier == AST_Node_Type::Reference)))

                            {
                                /// \todo This does not handle the case of an unassigned reference variable
                                ///       being assigned outside of its declaration
                                params[0].assign(params[1]);
                                params[0].reset_return_value();
                                return params[1];
                            }
                            else {
                                params[1] = detail::clone_if_necessary(std::move(params[1]), m_clone_loc, t_ss);
                            }
                        }

                        try {
                            return t_ss->call_function(this->text, m_loc, Function_Params{params}, t_ss.conversions());
                        }
                        catch (const exception::dispatch_error& e) {
                            throw exception::eval_error("Unable to find appropriate'" + this->text + "' operator.", e.parameters, e.functions, false, *t_ss);
                        }
                    }
                    catch (const exception::dispatch_error& e) {
                        throw exception::eval_error("Missing clone or copy constructor for right hand side of equation",
                            e.parameters,
                            e.functions,
                            false,
                            *t_ss);
                    }
                }
                else if (this->text == ":=") {
                    //if (params[0].is_undef() || Boxed_Value::type_match(params[0], params[1])) {
                        params[0].assign(params[1]);
                        params[0].reset_return_value();
                    //}
                    //else {
                    //    throw exception::eval_error("Mismatched types in equation");
                    //}
                }
                else if (m_oper == Operators::Opers::assign_if_null || this->text == "?=") {
                    // null-coalescing assignment operator. If the left-hand operand is null, it evaluates the right-hand operand and assigns it. 
                    if (params[0].is_undef()) {
                        try {
                            if (params[0].is_undef()) {
                                if (!this->children.empty()
                                    && ((this->children[0]->identifier == AST_Node_Type::Reference)
                                        || (!this->children[0]->children.empty() && this->children[0]->children[0]->identifier == AST_Node_Type::Reference)))

                                {
                                    /// \todo This does not handle the case of an unassigned reference variable
                                    ///       being assigned outside of its declaration
                                    params[0].assign(params[1]);
                                    params[0].reset_return_value();
                                    return params[1];
                                }
                                else {
                                    params[1] = detail::clone_if_necessary(std::move(params[1]), m_clone_loc, t_ss);
                                }
                            }

                            try {
                                return t_ss->call_function("=", m_loc, Function_Params{ params }, t_ss.conversions()); // likely returns params[1] or its value
                            }
                            catch (const exception::dispatch_error& e) {
                                throw exception::eval_error("Unable to find appropriate'" + this->text + "' operator.", e.parameters, e.functions, false, *t_ss);
                            }
                        }
                        catch (const exception::dispatch_error& e) {
                            throw exception::eval_error("Missing clone or copy constructor for right hand side of equation",
                                e.parameters,
                                e.functions,
                                false,
                                *t_ss);
                        }
                    }
                    else {
                        // example: 
                        // var x;
                        // x ?= 100; // x = 100
                        // return (x ?= 20) // x remains 100, returns x
                        return params[0]; // this shouldn't happen
                    }
                }
                else {
                    try {
                        return t_ss->call_function(this->text, m_loc, Function_Params{params}, t_ss.conversions());
                    }
                    catch (const exception::dispatch_error& e) {
                        throw exception::eval_error("Unable to find appropriate'" + this->text + "' operator.", e.parameters, e.functions, false, *t_ss);
                    }
                }

                return params[1];


            }

        private:
            Operators::Opers m_oper;
            mutable std::atomic_uint_fast32_t m_loc = { 0 };
            mutable std::atomic_uint_fast32_t m_clone_loc = { 0 };
        };

        /*
        global x;
        global& x;
        */
        template<typename T>
        struct Global_Decl_AST_Node final : AST_Node_Impl<T> {
            Global_Decl_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Global_Decl, std::move(t_loc), std::move(t_children)) {
                if (this->children.size() > 0) {
                    this->potentialReturnType = ReturnType(Type_Info(), AST_Node_Type::Global_Decl, false);
                    this->children[0]->potentialReturnType.ForwardRef(this->potentialReturnType);
                }
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);

                const std::string& idname = [&]() -> const std::string& {
                    if (this->children[0]->identifier == AST_Node_Type::Reference) {
                        return this->children[0]->children[0]->text;
                    }
                    else {
                        return this->children[0]->text;
                    }
                }();

                return t_ss->add_global_no_throw(Boxed_Value(), idname);
            }
        };

        /*
        var x;
        var& x;
        */
        template<typename T>
        struct Var_Decl_AST_Node final : AST_Node_Impl<T> {
            Var_Decl_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Var_Decl, std::move(t_loc), std::move(t_children)) {
                if (this->children.size() > 0) {
                    this->potentialReturnType = ReturnType(Type_Info(), AST_Node_Type::Var_Decl, false);
                    this->children[0]->potentialReturnType.ForwardRef(this->potentialReturnType);
                }
            }

            /*! Empty variable assignment:
              var j;
            */
            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);

                const std::string& idname = this->children[0]->text;

                try {
                    Boxed_Value bv;
                    t_ss.add_nodef_object(idname, bv);
                    return bv;

                    // return t_ss.add_get_nodef_object(idname, bv);
                }
                catch (const exception::name_conflict_error& e) {
                    throw exception::eval_error("Variable redefined '" + e.name() + "'");
                }
            }
        };

        /* 
        double& x;
        var x = double();
        double x;
        */
        template<typename T>
        struct Assign_Decl_AST_Node final : AST_Node_Impl<T> {
            Assign_Decl_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Assign_Decl, std::move(t_loc), std::move(t_children)) {

                this->potentialReturnType.ForwardRef(this->children[this->children.size() - 1]->potentialReturnType);
                this->children[0]->potentialReturnType.ForwardRef(this->children[this->children.size() - 1]->potentialReturnType);
            }

            /*! Non-Empty variable assignment:
              var j = 100;
            */
            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);

                const std::string& idname = this->children[0]->text;

                try {
                    Boxed_Value bv(detail::clone_if_necessary(this->children[1]->eval(t_ss), m_loc, t_ss));
                    bv.reset_return_value();
                    t_ss.add_object(idname, bv);
                    return bv;
                }
                catch (const exception::name_conflict_error& e) {
                    throw exception::eval_error("Variable redefined '" + e.name() + "'");
                }
            }

        private:
            mutable std::atomic_uint_fast32_t m_loc = { 0 };
        };

        /*
        var x ?= double();
        */
        template<typename T>
        struct Assign_Decl_IfNotDef_AST_Node final : AST_Node_Impl<T> {
            Assign_Decl_IfNotDef_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Assign_Decl, std::move(t_loc), std::move(t_children)) {
                this->potentialReturnType.ForwardRef(this->children[this->children.size() - 1]->potentialReturnType);
                this->children[0]->potentialReturnType.ForwardRef(this->children[this->children.size() - 1]->potentialReturnType);
            }

            /*! Non-Empty variable if_not_already_defined assignment:
              var j ?= 100;
            */
            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);

                const std::string& idname = this->children[0]->text;

                try {
                    if (t_ss.object_defined(idname)) {
                        // find and return this existing object - the "return type" does not need to match the right-hand value
                        return t_ss.get_object(idname);
                    }
                    else {
                        Boxed_Value bv(detail::clone_if_necessary(this->children[1]->eval(t_ss), m_loc, t_ss)); // make a new one
                        bv.reset_return_value();
                        t_ss.add_object(idname, bv);
                        return bv;
                    }
                }
                catch (const exception::name_conflict_error& e) {
                    throw exception::eval_error("Variable redefined '" + e.name() + "'");
                }
            }

        private:
            mutable std::atomic_uint_fast32_t m_loc = { 0 };
        };

        /*
        double x;
        */
        template<typename T>
        struct Assign_Retroactively_AST_Node final : AST_Node_Impl<T> {
            Assign_Retroactively_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Assign_Retroactively, std::move(t_loc), std::move(t_children)) {
                // the type will be determined at the end of the parsing
                this->children[0]->potentialReturnType.Deterministic() = true; // prevent others from messing it up
                this->potentialReturnType.ForwardRef(this->children[0]->potentialReturnType);
                this->children[this->children.size()-1]->potentialReturnType.ForwardRef(this->children[0]->potentialReturnType);
            }

            /*! Variable assignment with a function, with the order flipped: 
            children[0] = FunCall(), children[1] = var_decl or ref

            int j; 
            int& j;
            */
            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);

                AUTO def_type = detail::clone_if_necessary(this->children[0]->eval(t_ss), m_loc, t_ss); // the typename default (i.e. int(0) or string(""))
                Boxed_Value retval = this->children[1]->eval(t_ss); // container to return (i.e. var j)
                retval.reset_return_value();

                chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);

                auto params = make_vector(retval, def_type);                
                fpp.save_params(Function_Params{ params });

                try {
                    retval = t_ss->call_function("=", m_loc, Function_Params{ params }, t_ss.conversions()); 
                }
                catch (const exception::dispatch_error& e) {
                    throw exception::eval_error(std::string(e.what()) + " during execution of retroactive assignment with explicit typename", e.parameters, e.functions, true, *t_ss);                    
                }
                catch (detail::Return_Value& rv) {
                    retval = std::move(rv.retval);
                }

                retval.reset_return_value();
                return retval;
            }

        private:
            mutable std::atomic_uint_fast32_t m_loc = { 0 };
        };

        template<typename T>
        struct Array_Call_AST_Node final : AST_Node_Impl<T> {
            Array_Call_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Array_Call, std::move(t_loc), std::move(t_children)) {
                this->potentialReturnType = ReturnType(user_type<Boxed_Value>(), AST_Node_Type::Array_Call, false);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);

                chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);

                std::array<Boxed_Value, 2> params{ this->children[0]->eval(t_ss), this->children[1]->eval(t_ss) };

                if (params[0].is_type(user_type<std::vector<Boxed_Value>>())) {
                    auto* vecP = t_ss->boxed_cast<std::vector<Boxed_Value>*>(params[0]);
                    if (vecP) {
                        if (params[1].is_type(user_type<int>())) {
                            auto index = t_ss->boxed_cast<int>(params[1]);
                            return vecP->operator[](index);
                        }
                        else if (params[1].is_type(user_type<size_t>())) {
                            auto index = t_ss->boxed_cast<size_t>(params[1]);
                            return vecP->operator[](index);
                        }
                        else if (params[1].is_type(user_type<float>())) {
                            auto index = t_ss->boxed_cast<float>(params[1]);
                            return vecP->operator[](index);
                        }
                        else if (params[1].is_type(user_type<double>())) {
                            auto index = t_ss->boxed_cast<double>(params[1]);
                            return vecP->operator[](index);
                        }
                    }
                }
                else if (params[0].is_type(user_type<cweeList<Boxed_Value>>())) {
                    auto* vecP = t_ss->boxed_cast<cweeList<Boxed_Value>*>(params[0]);
                    if (vecP) {
                        if (params[1].is_type(user_type<int>())) {
                            auto index = t_ss->boxed_cast<int>(params[1]);
                            return vecP->operator[](index);
                        }
                        else if (params[1].is_type(user_type<size_t>())) {
                            auto index = t_ss->boxed_cast<size_t>(params[1]);
                            return vecP->operator[](index);
                        }
                        else if (params[1].is_type(user_type<float>())) {
                            auto index = t_ss->boxed_cast<float>(params[1]);
                            return vecP->operator[](index);
                        }
                        else if (params[1].is_type(user_type<double>())) {
                            auto index = t_ss->boxed_cast<double>(params[1]);
                            return vecP->operator[](index);
                        }
                    }
                } else if (params[0].is_type(user_type<std::map<std::string, Boxed_Value>>())) {
                    auto* vecP = t_ss->boxed_cast<std::map<std::string, Boxed_Value>*>(params[0]);
                    if (vecP) {
                        if (params[1].is_type(user_type<std::string>())) {
                            auto index = t_ss->boxed_cast<std::string>(params[1]);
                            return vecP->operator[](index);
                        }
                        else if (params[1].is_type(user_type<cweeStr>())) {
                            auto index = t_ss->boxed_cast<cweeStr>(params[1]);
                            return vecP->operator[](index.c_str());
                        }
                    }
                }

                try {
                    fpp.save_params(Function_Params{ params });
                    return t_ss->call_function("[]", m_loc, Function_Params{ params }, t_ss.conversions());
                }
                catch (const exception::dispatch_error& e) {
                    throw exception::eval_error("Can not find appropriate array lookup operator '[]'.", e.parameters, e.functions, false, *t_ss);
                }
            }

        private:
            mutable std::atomic_uint_fast32_t m_loc = { 0 };
        };

        template<typename T>
        struct Dot_Access_AST_Node final : AST_Node_Impl<T> {
            Dot_Access_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Dot_Access, std::move(t_loc), std::move(t_children))
                , m_fun_name(((this->children[1]->identifier == AST_Node_Type::Fun_Call) || (this->children[1]->identifier == AST_Node_Type::Array_Call))
                    ? this->children[1]->children[0]->text
                    : this->children[1]->text) {
                
                this->potentialReturnType = ReturnType(Type_Info(), AST_Node_Type::Dot_Access, false);
                this->children[1]->potentialReturnType.ForwardRef(this->potentialReturnType);
                if ((this->children[1]->identifier == AST_Node_Type::Fun_Call) || (this->children[1]->identifier == AST_Node_Type::Array_Call)) {
                    this->children[1]->children[0]->potentialReturnType.ForwardRef(this->potentialReturnType);
                }
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);

                chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);

                Boxed_Value retval = this->children[0]->eval(t_ss);
                auto params = make_vector(retval);

                bool has_function_params = false;
                if (this->children[1]->children.size() > 1) {
                    has_function_params = true;
                    for (const auto& child : this->children[1]->children[1]->children) {
                        params.push_back(child->eval(t_ss));
                    }
                }

                fpp.save_params(Function_Params{ params });

                try {
                    retval = t_ss->call_member(m_fun_name, m_loc, Function_Params{ params }, has_function_params, t_ss.conversions());
                }
                catch (const exception::dispatch_error& e) {
                    if (e.functions.empty()) {
#if 1
                        std::pair<std::string, Proxy_Function> tryFind = t_ss->get_function_with_similar_name(m_fun_name, e.parameters, t_ss.conversions());
                        if (tryFind.first != "") {
                            throw exception::eval_error("'" + m_fun_name + "' is not a function. Did you mean '" + tryFind.first + "' instead?");
                        }
#endif
                        throw exception::eval_error("'" + m_fun_name + "' is not a function.");
                        
                    }
                    else {
                        throw exception::eval_error(std::string(e.what()) + " for function '" + m_fun_name + "'", e.parameters, e.functions, true, *t_ss);
                    }
                }
                catch (detail::Return_Value& rv) {
                    retval = std::move(rv.retval);
                }

                if (this->children[1]->identifier == AST_Node_Type::Array_Call) {
                    try {
                        std::array<Boxed_Value, 2> p{ retval, this->children[1]->children[1]->eval(t_ss) };
                        retval = t_ss->call_function("[]", m_array_loc, Function_Params{ p }, t_ss.conversions());
                    }
                    catch (const exception::dispatch_error& e) {
                        throw exception::eval_error("Can not find appropriate array lookup operator '[]'.", e.parameters, e.functions, true, *t_ss);
                    }
                }

                return retval;
            }
            const std::string m_fun_name;

        private:
            mutable std::atomic_uint_fast32_t m_loc = { 0 };
            mutable std::atomic_uint_fast32_t m_array_loc = { 0 };
            
        };

        template<typename T>
        struct Lambda_AST_Node final : AST_Node_Impl<T> {
            Lambda_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(t_ast_node_text,
                    AST_Node_Type::Lambda,
                    std::move(t_loc),
                    chaiscript::small_vector<AST_Node_Impl_Ptr<T>>(std::make_move_iterator(t_children.begin()),
                        std::make_move_iterator(std::prev(t_children.end()))))
                , m_param_names(Arg_List_AST_Node<T>::get_arg_names(*this->children[1]))
                , m_this_capture(has_this_capture(this->children[0]->children))
                , m_lambda_node(std::move(t_children.back())) 
            {
                this->potentialReturnType = ReturnType(user_type<Const_Proxy_Function>(), AST_Node_Type::Lambda, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                const auto captures = [&]() -> std::map<std::string, Boxed_Value> {
                    std::map<std::string, Boxed_Value> named_captures;
                    for (const auto& capture : this->children[0]->children) {
                        named_captures.insert(std::make_pair(capture->children[0]->text, capture->children[0]->eval(t_ss)));
                    }
                    return named_captures;
                }();
                const auto numparams = this->children[1]->children.size();
                const auto param_types = Arg_List_AST_Node<T>::get_arg_types(*this->children[1], t_ss);

                std::reference_wrapper<chaiscript::detail::Dispatch_Engine> engine(*t_ss);

                return Boxed_Value(dispatch::make_dynamic_proxy_function(
                    [engine, lambda_node = this->m_lambda_node, param_names = this->m_param_names, captures, this_capture = this->m_this_capture](
                        const Function_Params& t_params) {
                    return detail::eval_function(engine, *lambda_node, param_names, t_params, &captures, this_capture);
                },
                    static_cast<int>(numparams),
                    m_lambda_node,
                    param_types));
            }

            static bool has_this_capture(const chaiscript::small_vector<AST_Node_Impl_Ptr<T>>& t_children) noexcept {
                return std::any_of(std::begin(t_children), std::end(t_children), [](const auto& child) { return child->children[0]->text == "this"; });
            }

        private:
            const chaiscript::small_vector<std::string> m_param_names;
            const bool m_this_capture = false;
            const chaiscript::shared_ptr<AST_Node_Impl<T>> m_lambda_node;
        };

        template<typename T>
        struct Scopeless_Block_AST_Node final : AST_Node_Impl<T> {
            Scopeless_Block_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Scopeless_Block, std::move(t_loc), std::move(t_children)) {
                if (this->children.size() > 0) {
                    this->potentialReturnType.ForwardRef(this->children[this->children.size() - 1]->potentialReturnType);
                }
                else {
                    this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Scopeless_Block, true);
                }
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                const auto num_children = this->children.size();
                for (size_t i = 0; i < num_children - 1; ++i) {
                    this->children[i]->eval(t_ss);
                }
                return this->children.back()->eval(t_ss);
            }
        };

        template<typename T>
        struct Block_AST_Node final : AST_Node_Impl<T> {
            Block_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Block, std::move(t_loc), std::move(t_children)) {
                // the final child determines the return type
                if (this->children.size() > 0) {
                    this->potentialReturnType.ForwardRef(this->children[this->children.size() - 1]->potentialReturnType);
                }
                else {
                    this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Block, true);
                }
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

                const auto num_children = this->children.size();
                for (size_t i = 0; i < num_children - 1; ++i) {
                    this->children[i]->eval(t_ss);
                }
                return this->children.back()->eval(t_ss);
            }
        };

        template<typename T>
        struct Def_AST_Node final : AST_Node_Impl<T> {
            chaiscript::shared_ptr<AST_Node_Impl<T>> m_body_node;
            chaiscript::shared_ptr<AST_Node_Impl<T>> m_guard_node;

            Def_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text),
                    AST_Node_Type::Def,
                    std::move(t_loc),
                    chaiscript::small_vector<AST_Node_Impl_Ptr<T>>(std::make_move_iterator(t_children.begin()),
                        std::make_move_iterator(
                            std::prev(t_children.end(), has_guard(t_children, 1) ? 2 : 1))))
                ,
                // This apparent use after move is safe because we are only moving out the specific elements we need
                // on each operation.
                m_body_node(get_body_node(std::move(t_children)))
                , m_guard_node(get_guard_node(std::move(t_children), t_children.size() - this->children.size() == 2))

            {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Def, true);
            }

            static chaiscript::shared_ptr<AST_Node_Impl<T>> get_guard_node(chaiscript::small_vector<AST_Node_Impl_Ptr<T>>&& vec, bool has_guard) {
                if (has_guard) {
                    return std::move(*std::prev(vec.end(), 2));
                }
                else {
                    return {};
                }
            }

            static chaiscript::shared_ptr<AST_Node_Impl<T>> get_body_node(chaiscript::small_vector<AST_Node_Impl_Ptr<T>>&& vec) { return std::move(vec.back()); }

            static bool has_guard(const chaiscript::small_vector<AST_Node_Impl_Ptr<T>>& t_children, const std::size_t offset) noexcept {
                if ((t_children.size() > 2 + offset) && (t_children[1 + offset]->identifier == AST_Node_Type::Arg_List)) {
                    if (t_children.size() > 3 + offset) {
                        return true;
                    }
                }
                else {
                    if (t_children.size() > 2 + offset) {
                        return true;
                    }
                }
                return false;
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                chaiscript::small_vector<std::string> t_param_names;
                size_t numparams = 0;

                dispatch::Param_Types param_types;

                if ((this->children.size() > 1) && (this->children[1]->identifier == AST_Node_Type::Arg_List)) {
                    numparams = this->children[1]->children.size();
                    t_param_names = Arg_List_AST_Node<T>::get_arg_names(*this->children[1]);
                    param_types = Arg_List_AST_Node<T>::get_arg_types(*this->children[1], t_ss);
                }

                std::reference_wrapper<chaiscript::detail::Dispatch_Engine> engine(*t_ss);
                chaiscript::shared_ptr<dispatch::Proxy_Function_Base> guard;
                if (m_guard_node) {
                    guard = dispatch::make_dynamic_proxy_function(
                        [engine, guardnode = m_guard_node, t_param_names](const Function_Params& t_params) {
                            return detail::eval_function(engine, *guardnode, t_param_names, t_params);
                        },
                        static_cast<int>(numparams),
                            m_guard_node);
                    guard->set_parameterNames(t_param_names);
                }

                try {
                    if (m_body_node) {
                        // chaiscript::shared_ptr<AST_Node> tryGetNode;
                        // tryGetNode = chaiscript::dynamic_shared_ptr_cast(m_body_node, tryGetNode);
                        // tryGetNode->location.filename = nullptr;
                        
                        // m_body_node->location.filename = nullptr;
                    }

                    const std::string& l_function_name = this->children[0]->text;
                    AUTO funcPtr = dispatch::make_dynamic_proxy_function(
                        [engine, func_node = m_body_node, t_param_names](const Function_Params& t_params) {
                            return detail::eval_function(engine, *func_node, t_param_names, t_params);
                        },
                        static_cast<int>(numparams),
                            m_body_node,
                            param_types,
                            guard);
                    funcPtr->set_parameterNames(t_param_names);
                    t_ss->add(funcPtr, l_function_name);
                }
                catch (const exception::name_conflict_error& e) {
                    throw exception::eval_error("Function redefined '" + e.name() + "'");
                }
                return void_var();
            }
        };

        template<typename T>
        struct While_AST_Node final : AST_Node_Impl<T> {
            While_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::While, std::move(t_loc), std::move(t_children)) {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::While, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

                try {
                    while (this->get_scoped_bool_condition(*this->children[0], t_ss)) {
                        try {
                            this->children[1]->eval(t_ss);
                        }
                        catch (detail::Continue_Loop&) {
                            // we got a continue exception, which means all of the remaining
                            // loop implementation is skipped and we just need to continue to
                            // the next condition test
                        }
                    }
                }
                catch (detail::Break_Loop&) {
                    // loop was broken intentionally
                }

                return void_var();
            }
        };

        template<typename T>
        struct Class_AST_Node final : AST_Node_Impl<T> {
            Class_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Class, std::move(t_loc), std::move(t_children)) {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Class, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);

                chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

                /// \todo do this better
                // put class name in current scope so it can be looked up by the attrs and methods
                t_ss.add_object("_current_class_name", const_var(this->children[0]->text));

                this->children[1]->eval(t_ss);

                return void_var();
            }
        };

        template<typename T>
        struct If_AST_Node final : AST_Node_Impl<T> {
            If_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::If, std::move(t_loc), std::move(t_children)) {
                assert(this->children.size() == 3);
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::If, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                if (this->get_bool_condition(this->children[0]->eval(t_ss), t_ss)) {
                    return this->children[1]->eval(t_ss);
                }
                else {
                    return this->children[2]->eval(t_ss);
                }
            }
        };

        template<typename T>
        struct ControlBlock_AST_Node final : AST_Node_Impl<T> {
            ControlBlock_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::ControlBlock, std::move(t_loc), std::move(t_children)) {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::ControlBlock, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

                chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::recursive_mutex> shared_guard(t_ss->get_mut());

                const auto num_children = this->children.size();
                for (size_t i = 0; i < num_children - 1; ++i) {
                    this->children[i]->eval(t_ss);
                }
                return this->children.back()->eval(t_ss);
            }
        };

        INLINE static bool TryGetVariableName(AST_Node const& m, std::string& out) {
            if (m.text != "") { 
                out = m.text;
                return true;
            }

            AUTO childs = m.get_children(); // search backwards / deepest first
            for (int i = childs.size() - 1; i >= 0; i--) {
                if (TryGetVariableName(childs[i], out)) {
                    return true;
                }
            }

            return false;
        };

        template<typename T>
        struct Ranged_For_AST_Node final : AST_Node_Impl<T> {
            Ranged_For_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Ranged_For, std::move(t_loc), std::move(t_children)) {
                assert(this->children.size() == 3);
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Ranged_For, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                const auto get_function = [&t_ss](const std::string& t_name, auto& t_hint) {
                    uint_fast32_t hint = t_hint;
                    auto [funs_loc, funs] = t_ss->get_function(t_name, hint);
                    if (funs_loc != hint) {
                        t_hint = uint_fast32_t(funs_loc);
                    }
                    return std::move(funs);
                };

                const auto call_function = [&t_ss](const auto& t_funcs, const Boxed_Value& t_param) {
                    return dispatch::dispatch(*t_funcs, Function_Params{ t_param }, t_ss.conversions());
                };
                
                Boxed_Value range_expression_result = this->children[1]->eval(t_ss); // 0..100 or [0,1,2,3] or vectorObjName, etc.
                
                std::string loop_var_name = this->children[0]->text; /* "x", or "var x", or "auto& x" */ 
                if (!TryGetVariableName(*this->children[0], loop_var_name)) throw exception::eval_error("Ranged For-Loop Variable Name Could Not Be Resolved.");
                
                enum class RangedForMode {
                    ReferenceBasedVar, 
                    AssignmentVar
                };
                RangedForMode mode;
                chaiscript::Boxed_Value rangedObjContainer;

                switch (this->children[0]->identifier) {
                default:
                case AST_Node_Type::Global_Decl: // for (global i : 0..10){} or for (global& i : 0..10){}  
                    mode = RangedForMode::ReferenceBasedVar;
                    break;
                case AST_Node_Type::Var_Decl:    // for (var i : 0..10){}
                case AST_Node_Type::Reference:   // for (var& i : 0..10){} 
                    mode = RangedForMode::ReferenceBasedVar;
                    break;
                case AST_Node_Type::Assign_Decl: // for (int i : 0..10){} or for (int& i : 0..10){}
                    mode = RangedForMode::AssignmentVar;
                    rangedObjContainer = this->children[0]->eval(t_ss);
                    rangedObjContainer = t_ss.add_get_object(loop_var_name, rangedObjContainer);
                    break;
                case AST_Node_Type::Id:          // for (i : 0..10){}
                    if (t_ss.object_exists(loop_var_name)) {
                        mode = RangedForMode::AssignmentVar;
                        rangedObjContainer = t_ss.get_object(loop_var_name);
                    }
                    else {
                        mode = RangedForMode::ReferenceBasedVar;
                    }                  
                    break;
                }

                const auto do_loop = [&loop_var_name, &t_ss, &mode, &rangedObjContainer, this](const auto& ranged_thing)->chaiscript::Boxed_Value {
                    try {
                        switch (mode) {
                        case RangedForMode::AssignmentVar: {
                            chaiscript::eval::detail::Scope_Push_Pop spp1(t_ss);
                            for (auto&& loop_var : ranged_thing) {
                                if constexpr (!std::is_same<std::decay_t<decltype(loop_var)>, Boxed_Value>::value) {
                                    std::array<Boxed_Value, 2> p{ rangedObjContainer, Boxed_Value(std::ref(loop_var)) };
                                    try {
                                        t_ss->call_function("=", m_array_loc, chaiscript::Function_Params{ p }, t_ss.conversions());
                                    }
                                    catch (...) {
                                        throw exception::eval_error(cweeStr::printf("Could not perform `=` cast from `%s` to `%s` during ranged loop",
                                            t_ss->get_type_name(Boxed_Value(std::ref(loop_var)).get_type_info()).c_str(),
                                            t_ss->get_type_name(rangedObjContainer.get_type_info()).c_str(),
                                            this->location.start, *(this->location.filename)).c_str()
                                        );
                                    }
                                }
                                else {
                                    std::array<Boxed_Value, 2> p{ rangedObjContainer, Boxed_Value(loop_var) };
                                    try {
                                        t_ss->call_function("=", m_array_loc, chaiscript::Function_Params{ p }, t_ss.conversions());
                                    }
                                    catch (...) {
                                        throw exception::eval_error(cweeStr::printf("Could not perform `=` cast from `%s` to `%s` during ranged loop",
                                            t_ss->get_type_name(Boxed_Value(std::ref(loop_var)).get_type_info()).c_str(),
                                            t_ss->get_type_name(rangedObjContainer.get_type_info()).c_str(),
                                            this->location.start, *(this->location.filename)).c_str()
                                        );
                                    }
                                }

                                try {
                                    this->children[2]->eval(t_ss);
                                }
                                catch (detail::Continue_Loop&) {
                                }
                            }
                        } break;
                        case RangedForMode::ReferenceBasedVar: {
                            for (auto&& loop_var : ranged_thing) {
                                chaiscript::eval::detail::Scope_Push_Pop spp1(t_ss);
                                chaiscript::Boxed_Value sharedObj;
                                if constexpr (!std::is_same<std::decay_t<decltype(loop_var)>, Boxed_Value>::value) {
                                    sharedObj = Boxed_Value(std::ref(loop_var));
                                }
                                else {
                                    sharedObj = Boxed_Value(loop_var);
                                }
                                t_ss.add_get_object(loop_var_name, sharedObj);

                                try {
                                    this->children[2]->eval(t_ss);
                                }
                                catch (detail::Continue_Loop&) {}
                                catch (detail::Return_Value& rv) {
                                    if (rv.retval.get_ptr() == sharedObj.get_ptr()) {
                                        if (rv.retval.get_type_info() == sharedObj.get_type_info()) {
                                            // user is trying to return the loop var. 
                                            std::decay_t<decltype(loop_var)> newObj = loop_var;
                                            throw detail::Return_Value{ Boxed_Value(newObj) };
                                        }
                                        else {
                                            // user is doing something I don't understand, such as element-accessing the iterator
                                            std::array<Boxed_Value, 1> p{ rv.retval };                                            
                                            throw detail::Return_Value{ t_ss->call_function("__clone", m_array_loc, chaiscript::Function_Params{ p }, t_ss.conversions()) };
                                        }
                                    }
                                    throw detail::Return_Value{ rv.retval };
                                }
                            }
                        } break;
                        }
                    }
                    catch (detail::Break_Loop&) {
                        // loop broken
                    }
                    return void_var();
                };

                if (range_expression_result.get_type_info().bare_equal_type_info(typeid(chaiscript::small_vector<Boxed_Value>))) 
                    return do_loop(boxed_cast<const chaiscript::small_vector<Boxed_Value>&>(range_expression_result));
                else if (range_expression_result.get_type_info().bare_equal_type_info(typeid(std::map<std::string, Boxed_Value>))) 
                    return do_loop(boxed_cast<const std::map<std::string, Boxed_Value>&>(range_expression_result));
                else {
                    const auto range_funcs = get_function("range", m_range_loc);
                    const auto empty_funcs = get_function("empty", m_empty_loc);
                    const auto front_funcs = get_function("front", m_front_loc);
                    const auto pop_front_funcs = get_function("pop_front", m_pop_front_loc);

                    try {
                        const auto range_obj = call_function(range_funcs, range_expression_result);

                        switch (mode) {
                        case RangedForMode::AssignmentVar: {
                            chaiscript::eval::detail::Scope_Push_Pop spp1(t_ss);
                            while (!boxed_cast<bool>(call_function(empty_funcs, range_obj))) {
                                auto&& loop_var = call_function(front_funcs, range_obj);

                                if constexpr (!std::is_same<std::decay_t<decltype(loop_var)>, Boxed_Value>::value) {
                                    std::array<Boxed_Value, 2> p{ rangedObjContainer, Boxed_Value(std::ref(loop_var)) };
                                    try {
                                        t_ss->call_function("=", m_array_loc, chaiscript::Function_Params{ p }, t_ss.conversions());
                                    }
                                    catch (...) {
                                        throw exception::eval_error(cweeStr::printf("Could not perform `=` cast from `%s` to `%s` during ranged loop",
                                            t_ss->get_type_name(Boxed_Value(std::ref(loop_var)).get_type_info()).c_str(),
                                            t_ss->get_type_name(rangedObjContainer.get_type_info()).c_str(),
                                            this->location.start, *(this->location.filename)).c_str()
                                        );
                                    }
                                }
                                else {
                                    std::array<Boxed_Value, 2> p{ rangedObjContainer, Boxed_Value(loop_var) };
                                    try {
                                        t_ss->call_function("=", m_array_loc, chaiscript::Function_Params{ p }, t_ss.conversions());
                                    }
                                    catch (...) {
                                        throw exception::eval_error(cweeStr::printf("Could not perform `=` cast from `%s` to `%s` during ranged loop",
                                            t_ss->get_type_name(Boxed_Value(std::ref(loop_var)).get_type_info()).c_str(),
                                            t_ss->get_type_name(rangedObjContainer.get_type_info()).c_str(),
                                            this->location.start, *(this->location.filename)).c_str()
                                        );
                                    }
                                }

                                try {
                                    this->children[2]->eval(t_ss);
                                }
                                catch (detail::Continue_Loop&) {
                                }

                                call_function(pop_front_funcs, range_obj);
                            }
                        } break;
                        case RangedForMode::ReferenceBasedVar: {
                            while (!boxed_cast<bool>(call_function(empty_funcs, range_obj))) {
                                chaiscript::eval::detail::Scope_Push_Pop spp1(t_ss);
                                auto&& loop_var = call_function(front_funcs, range_obj);
                                chaiscript::Boxed_Value sharedObj;
                                if constexpr (!std::is_same<std::decay_t<decltype(loop_var)>, Boxed_Value>::value) {
                                    sharedObj = Boxed_Value(std::ref(loop_var));
                                }
                                else {
                                    sharedObj = Boxed_Value(loop_var);
                                }
                                t_ss.add_get_object(loop_var_name, sharedObj);

                                try {
                                    this->children[2]->eval(t_ss);
                                }
                                catch (detail::Continue_Loop&) {}
                                catch (detail::Return_Value& rv) {
                                    if (rv.retval.get_ptr() == sharedObj.get_ptr()) {
                                        if (rv.retval.get_type_info() == sharedObj.get_type_info()) {
                                            // user is trying to return the loop var. 
                                            std::decay_t<decltype(loop_var)> newObj = loop_var;
                                            throw detail::Return_Value{ Boxed_Value(newObj) };
                                        }
                                        else {
                                            // user is doing something I don't understand, such as element-accessing the iterator
                                            std::array<Boxed_Value, 1> p{ rv.retval };
                                            throw detail::Return_Value{ t_ss->call_function("__clone", m_array_loc, chaiscript::Function_Params{ p }, t_ss.conversions()) };
                                        }
                                    }
                                    throw detail::Return_Value{ rv.retval };
                                }

                                call_function(pop_front_funcs, range_obj);
                            }
                        } break;
                        }
                    }
                    catch (detail::Break_Loop&) {
                        // loop broken
                    }
                    return void_var();
                }
            }

        private:
            mutable std::atomic_uint_fast32_t m_array_loc = { 0 };
            mutable std::atomic_uint_fast32_t m_eqFunc_loc = { 0 };
            mutable std::atomic_uint_fast32_t m_range_loc = { 0 };
            mutable std::atomic_uint_fast32_t m_empty_loc = { 0 };
            mutable std::atomic_uint_fast32_t m_front_loc = { 0 };
            mutable std::atomic_uint_fast32_t m_pop_front_loc = { 0 };
        };

        template<typename T>
        struct For_AST_Node final : AST_Node_Impl<T> {
            For_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::For, std::move(t_loc), std::move(t_children)) {
                assert(this->children.size() == 4);
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::For, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

                try {
                    for (this->children[0]->eval(t_ss); this->get_scoped_bool_condition(*this->children[1], t_ss); this->children[2]->eval(t_ss)) {
                        try {
                            // Body of Loop
                            this->children[3]->eval(t_ss);
                        }
                        catch (detail::Continue_Loop&) {
                            // we got a continue exception, which means all of the remaining
                            // loop implementation is skipped and we just need to continue to
                            // the next iteration step
                        }
                    }
                }
                catch (detail::Break_Loop&) {
                    // loop broken
                }

                return void_var();
            }
        };

        template<typename T>
        struct Switch_AST_Node final : AST_Node_Impl<T> {
            Switch_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Switch, std::move(t_loc), std::move(t_children)) {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Switch, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                bool breaking = false;
                size_t currentCase = 1;
                bool hasMatched = false;

                chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

                Boxed_Value match_value(this->children[0]->eval(t_ss));

                while (!breaking && (currentCase < this->children.size())) {
                    try {
                        if (this->children[currentCase]->identifier == AST_Node_Type::Case) {
                            // This is a little odd, but because want to see both the switch and the case simultaneously, I do a downcast here.
                            try {
                                std::array<Boxed_Value, 2> p{ match_value, this->children[currentCase]->children[0]->eval(t_ss) };
                                if (hasMatched || boxed_cast<bool>(t_ss->call_function("==", m_loc, Function_Params{ p }, t_ss.conversions()))) {
                                    this->children[currentCase]->eval(t_ss);
                                    hasMatched = true;
                                }
                            }
                            catch (const exception::bad_boxed_cast&) {
                                throw exception::eval_error("Internal error: case guard evaluation not boolean");
                            }
                        }
                        else if (this->children[currentCase]->identifier == AST_Node_Type::Default) {
                            this->children[currentCase]->eval(t_ss);
                            hasMatched = true;
                        }
                    }
                    catch (detail::Break_Loop&) {
                        breaking = true;
                    }
                    ++currentCase;
                }
                return void_var();
            }

            mutable std::atomic_uint_fast32_t m_loc = { 0 };
        };

        template<typename T>
        struct Case_AST_Node final : AST_Node_Impl<T> {
            Case_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Case, std::move(t_loc), std::move(t_children)) {
                assert(this->children.size() == 2); /* how many children does it have? */
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Case, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

                this->children[1]->eval(t_ss);

                return void_var();
            }
        };

        template<typename T>
        struct Default_AST_Node final : AST_Node_Impl<T> {
            Default_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Default, std::move(t_loc), std::move(t_children)) {
                assert(this->children.size() == 1);
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Default, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

                this->children[0]->eval(t_ss);

                return void_var();
            }
        };

        template<typename T>
        struct Inline_Array_AST_Node final : AST_Node_Impl<T> {
            Inline_Array_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Inline_Array, std::move(t_loc), std::move(t_children)) {
                this->potentialReturnType = ReturnType(user_type<std::vector<Boxed_Value>>(), AST_Node_Type::Inline_Array, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                try {
                    chaiscript::small_vector<Boxed_Value> vec;
                    if (!this->children.empty()) {
                        vec.reserve(this->children[0]->children.size());
                        for (const auto& child : this->children[0]->children) {
                            vec.push_back(detail::clone_if_necessary(child->eval(t_ss), m_loc, t_ss));
                        }
                    }
                    return const_var(std::move(vec));
                }
                catch (const exception::dispatch_error&) {
                    throw exception::eval_error("Can not find appropriate 'clone' or copy constructor for vector elements");
                }
            }

        private:
            mutable std::atomic_uint_fast32_t m_loc = { 0 };
        };

        template<typename T>
        struct Inline_Map_AST_Node final : AST_Node_Impl<T> {
            Inline_Map_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Inline_Map, std::move(t_loc), std::move(t_children)) {
                this->potentialReturnType = ReturnType(user_type<std::map<std::string, Boxed_Value>>(), AST_Node_Type::Inline_Array, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                try {
                    std::map<std::string, Boxed_Value> retval;

                    for (const auto& child : this->children[0]->children) {
                        retval.insert(std::make_pair(t_ss->boxed_cast<std::string>(child->children[0]->eval(t_ss)),
                            detail::clone_if_necessary(child->children[1]->eval(t_ss), m_loc, t_ss)));
                    }

                    return const_var(std::move(retval));
                }
                catch (const exception::dispatch_error& e) {
                    throw exception::eval_error("Can not find appropriate copy constructor or 'clone' while inserting into Map.",
                        e.parameters,
                        e.functions,
                        false,
                        *t_ss);
                }
            }

        private:
            mutable std::atomic_uint_fast32_t m_loc = { 0 };
        };

        template<typename T>
        struct Return_AST_Node final : AST_Node_Impl<T> {
            Return_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Return, std::move(t_loc), std::move(t_children)) {
                if (!this->children.empty()) {
                    this->potentialReturnType.ForwardRef(this->children[0]->potentialReturnType);
                }
                else {
                    this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Return, true);
                }
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                if (!this->children.empty()) {
                    throw detail::Return_Value{ this->children[0]->eval(t_ss) };
                }
                else {
                    throw detail::Return_Value{ void_var() };
                }
            }
        };

        template<typename T>
        struct File_AST_Node final : AST_Node_Impl<T> {
            File_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::File, std::move(t_loc), std::move(t_children)) {
                if (this->children.size() > 0) {
                    this->potentialReturnType.ForwardRef(this->children[0]->potentialReturnType);
                }
                else {
                    this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::File, true);
                }
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                try {
                    const auto num_children = this->children.size();

                    if (num_children > 0) {
                        for (size_t i = 0; i < num_children - 1; ++i) {
                            this->children[i]->eval(t_ss);
                        }
                        return this->children.back()->eval(t_ss);
                    }
                    else {
                        return void_var();
                    }
                }
                catch (const detail::Continue_Loop&) {
                    throw exception::eval_error("Unexpected `continue` statement outside of a loop");
                }
                catch (const detail::Break_Loop&) {
                    throw exception::eval_error("Unexpected `break` statement outside of a loop");
                }
            }
        };

        template<typename T>
        struct Reference_AST_Node final : AST_Node_Impl<T> {
            Reference_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Reference, std::move(t_loc), std::move(t_children)) {
                assert(this->children.size() == 1);
                // this ID is void to start with... 
                this->potentialReturnType = ReturnType(Type_Info(), AST_Node_Type::Reference, false);
                this->children[0]->potentialReturnType.ForwardRef(this->potentialReturnType);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                Boxed_Value bv;
                t_ss.add_object(this->children[0]->text, bv); // assumes the child is almost certainly an Id type or other "defined name" type
                return bv;
            }
        };

        template<typename T>
        struct Prefix_AST_Node final : AST_Node_Impl<T> {
            Prefix_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Prefix, std::move(t_loc), std::move(t_children))
                , m_oper(Operators::to_operator(this->text, true)) {
                this->potentialReturnType.ForwardRef(this->children[0]->potentialReturnType);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                Boxed_Value bv(this->children[0]->eval(t_ss));

                try {
                    // short circuit arithmetic operations
                    if (m_oper != Operators::Opers::invalid && m_oper != Operators::Opers::bitwise_and && bv.get_type_info().is_arithmetic()) {
                        if ((m_oper == Operators::Opers::pre_increment || m_oper == Operators::Opers::pre_decrement) && bv.is_const()) {
                            throw exception::eval_error("Error with prefix operator evaluation: cannot modify constant value.");
                        }
                        return Boxed_Number::do_oper(m_oper, bv);
                    }
                    else {
                        chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
                        fpp.save_params(Function_Params{ bv });
                        return t_ss->call_function(this->text, m_loc, Function_Params{bv}, t_ss.conversions());
                    }
                }
                catch (const exception::dispatch_error& e) {
                    throw exception::eval_error("Error with prefix operator evaluation: '" + this->text + "'", e.parameters, e.functions, false, *t_ss);
                }
            }

        private:
            Operators::Opers m_oper = Operators::Opers::invalid;
            mutable std::atomic_uint_fast32_t m_loc = { 0 };
        };

        template<typename T>
        struct Postfix_AST_Node final : AST_Node_Impl<T> {
            Postfix_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Postfix, std::move(t_loc), std::move(t_children))
                , m_oper(Operators::to_operator(this->text, true)) {
                this->potentialReturnType = ReturnType(this->children[0]->potentialReturnType.Type(), AST_Node_Type::Postfix, false);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                Boxed_Value bv(this->children[0]->eval(t_ss)); // an int, float, etc.                 

                try {
                    // short circuit arithmetic operations
                    if (m_oper == Operators::Opers::invalid) {
                        // the user is likely typing something like "10_ft" or "100_gpm" to generate a new, custom type. 
                        if (this->text != "") {
                            chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
                            fpp.save_params(Function_Params{ bv });
                            AUTO ptr = t_ss->get_postfixes().GetPtr(this->text); // convert from "_ft" to "foot"
                            if (ptr) {
                                return t_ss->call_function(*ptr, m_loc, Function_Params{ bv }, t_ss.conversions());
                            }
                            return t_ss->call_function(this->text, m_loc, Function_Params{bv}, t_ss.conversions());
                        }
                    }
                    else {
                        if (bv.is_const()) { 
                            throw exception::eval_error("Error with postfix operator evaluation: cannot modify constant value.");
                        }
                        
                        // the user typed a known postfix such as ++ or --
                        if (bv.get_type_info().is_arithmetic()) {
                            // the type is known. Short-circuit and get out fast. 
                            if (m_oper == Operators::Opers::pre_increment) {
                                Boxed_Value result = Boxed_Number::do_oper(chaiscript::Operators::Opers::sum, var(0), bv);
                                Boxed_Number::do_oper(m_oper, bv);
                                return result;

                            }
                            else if (m_oper == Operators::Opers::pre_decrement) {
                                Boxed_Value result = Boxed_Number::do_oper(chaiscript::Operators::Opers::sum, var(0), bv);
                                Boxed_Number::do_oper(m_oper, bv);
                                return result;
                            }
                            else {
                                throw exception::eval_error("Only increment (i++) or decrement (i--) operators are supported in a postfix context.");
                            }
                        }
                        else {
                            if (bv.is_return_value()) {
                                return bv;
                            }
                            // the type is something non-number (such as a custom type or a value with units)
                            // get a copy of the value, since this is a post-fix.
                            auto result = chaiscript::eval::detail::clone_if_necessary(bv, m_loc, t_ss);
                            if (m_oper == Operators::Opers::pre_increment) {
                                chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
                                fpp.save_params(Function_Params{ bv });
                                t_ss->call_function("++", m_loc, Function_Params{ bv }, t_ss.conversions()); // bv should be incremented
                            }
                            else if (m_oper == Operators::Opers::pre_decrement) {
                                chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
                                fpp.save_params(Function_Params{ bv });
                                t_ss->call_function("++", m_loc, Function_Params{ bv }, t_ss.conversions()); // bv should be incremented
                            }
                            else {
                                throw exception::eval_error("Only increment (i++) or decrement (i--) operators are supported in a postfix context.");
                            }
                            return result;                            
                        }
                    }
                }
                catch (const exception::dispatch_error& e) {
                    throw exception::eval_error("Error with postfix operator evaluation: '" + this->text + "'", e.parameters, e.functions, false, *t_ss);
                }
            }

            Operators::Opers m_oper = Operators::Opers::invalid;
        private:
            // Operators::Opers m_oper = Operators::Opers::invalid;
            mutable std::atomic_uint_fast32_t m_loc = { 0 };
        };

        template<typename T>
        struct Break_AST_Node final : AST_Node_Impl<T> {
            Break_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Break, std::move(t_loc), std::move(t_children)) {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Break, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State&) const override { throw detail::Break_Loop(); }
        };

        template<typename T>
        struct Continue_AST_Node final : AST_Node_Impl<T> {
            Continue_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Continue, std::move(t_loc), std::move(t_children)) {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Continue, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State&) const override { throw detail::Continue_Loop(); }
        };

        template<typename T>
        struct Noop_AST_Node final : AST_Node_Impl<T> {
            Noop_AST_Node(std::string t_ast_node_text, Parse_Location t_loc)
                : AST_Node_Impl<T>(t_ast_node_text, AST_Node_Type::Noop, t_loc) {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Noop, true);
            }
            Noop_AST_Node()
                : AST_Node_Impl<T>("", AST_Node_Type::Noop, Parse_Location()) {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Noop, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State&) const override {
                // It's a no-op, that evaluates to "void"
                return val;
            }

            Boxed_Value val = void_var();
        };

        template<typename T>
        struct Map_Pair_AST_Node final : AST_Node_Impl<T> {
            Map_Pair_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Map_Pair, std::move(t_loc), std::move(t_children)) {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Map_Pair, true);
            }
        };

        template<typename T>
        struct Value_Range_AST_Node final : AST_Node_Impl<T> {
            Value_Range_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Value_Range, std::move(t_loc), std::move(t_children)) {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Value_Range, true);
            }
        };

        template<typename T>
        struct Inline_Range_AST_Node final : AST_Node_Impl<T> {
            Inline_Range_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Inline_Range, std::move(t_loc), std::move(t_children)) {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Inline_Range, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                try {
                    std::array<Boxed_Value, 2> params{ this->children[0]->children[0]->children[0]->eval(t_ss),
                                                      this->children[0]->children[0]->children[1]->eval(t_ss) };

                    return t_ss->call_function("generate_range", m_loc, Function_Params{ params }, t_ss.conversions());
                }
                catch (const exception::dispatch_error& e) {
                    throw exception::eval_error("Unable to generate range vector, while calling 'generate_range'", e.parameters, e.functions, false, *t_ss);
                }
            }

        private:
            mutable std::atomic_uint_fast32_t m_loc = { 0 };
        };

        template<typename T>
        struct Do_AST_Node final : AST_Node_Impl<T> {
            Do_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Do, std::move(t_loc), std::move(t_children)) {
                // assume no throw will occur for the type propogation -> first child result
                if (this->children.size() > 0) {
                    this->potentialReturnType.ForwardRef(this->children[0]->potentialReturnType);
                }
                else {
                    this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Do, true);
                }
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                Boxed_Value retval;

                chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

                try {
                    retval = this->children[0]->eval(t_ss);
                }
                catch (const exception::eval_error& e) {
                    if (this->children.back()->identifier == AST_Node_Type::Finally) {
                        this->children.back()->children[0]->eval(t_ss);
                    }
                    throw e;                    
                }
                catch (const std::runtime_error& e) {
                    if (this->children.back()->identifier == AST_Node_Type::Finally) {
                        this->children.back()->children[0]->eval(t_ss);
                    }
                    throw e;
                }
                catch (const std::out_of_range& e) {
                    if (this->children.back()->identifier == AST_Node_Type::Finally) {
                        this->children.back()->children[0]->eval(t_ss);
                    }
                    throw e;
                }
                catch (const std::exception& e) {
                    if (this->children.back()->identifier == AST_Node_Type::Finally) {
                        this->children.back()->children[0]->eval(t_ss);
                    }
                    throw e;
                }
                catch (Boxed_Value& e) {
                    if (this->children.back()->identifier == AST_Node_Type::Finally) {
                        this->children.back()->children[0]->eval(t_ss);
                    }
                    throw e;
                }
                catch (...) {
                    if (this->children.back()->identifier == AST_Node_Type::Finally) {
                        this->children.back()->children[0]->eval(t_ss);
                    }
                    throw;
                }
                if (this->children.back()->identifier == AST_Node_Type::Finally) {
                    this->children.back()->children[0]->eval(t_ss);
                }
                return retval;
            }
        };

        template<typename T>
        struct Try_AST_Node final : AST_Node_Impl<T> {
            Try_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Try, std::move(t_loc), std::move(t_children)) {
                // assume no throw will occur for the type propogation -> first child result
                if (this->children.size() > 0) {
                    this->potentialReturnType.ForwardRef(this->children[0]->potentialReturnType);
                }
                else {
                    this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Try, true);
                }
            }

            Boxed_Value handle_exception(const chaiscript::detail::Dispatch_State& t_ss, const Boxed_Value& t_except) const {
                Boxed_Value retval;

                size_t end_point = this->children.size();
                if (this->children.back()->identifier == AST_Node_Type::Finally) {
                    assert(end_point > 0);
                    end_point = this->children.size() - 1;
                }
                for (size_t i = 1; i < end_point; ++i) {
                    chaiscript::eval::detail::Scope_Push_Pop catch_scope(t_ss);
                    auto& catch_block = *this->children[i];

                    if (catch_block.children.size() == 1) {
                        // No variable capture
                        retval = catch_block.children[0]->eval(t_ss);
                        break;
                    }
                    else if (catch_block.children.size() == 2 || catch_block.children.size() == 3) {
                        const auto name = Arg_List_AST_Node<T>::get_arg_name(*catch_block.children[0]);

                        if (dispatch::Param_Types(
                            chaiscript::small_vector<std::pair<std::string, Type_Info>>{Arg_List_AST_Node<T>::get_arg_type(*catch_block.children[0], t_ss)})
                            .match(Function_Params{ t_except }, t_ss.conversions())
                            .first) {
                            t_ss.add_object(name, t_except);

                            if (catch_block.children.size() == 2) {
                                // Variable capture
                                retval = catch_block.children[1]->eval(t_ss);
                                break;
                            }
                        }
                    }
                    else {
                        if (this->children.back()->identifier == AST_Node_Type::Finally) {
                            this->children.back()->children[0]->eval(t_ss);
                        }
                        throw exception::eval_error("Internal error: catch block size unrecognized");
                    }
                }

                return retval;
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                Boxed_Value retval;

                chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

                try {
                    retval = this->children[0]->eval(t_ss);
                }
                catch (const exception::eval_error& e) {
                    retval = handle_exception(t_ss, Boxed_Value(std::ref(e)));
                }
                catch (const std::runtime_error& e) {
                    retval = handle_exception(t_ss, Boxed_Value(std::ref(e)));
                }
                catch (const std::out_of_range& e) {
                    retval = handle_exception(t_ss, Boxed_Value(std::ref(e)));
                }
                catch (const std::exception& e) {
                    retval = handle_exception(t_ss, Boxed_Value(std::ref(e)));
                }
                catch (Boxed_Value& e) {
                    retval = handle_exception(t_ss, e);
                }
                catch (...) {
                    if (this->children.back()->identifier == AST_Node_Type::Finally) {
                        this->children.back()->children[0]->eval(t_ss);
                    }
                    throw;
                }

                if (this->children.back()->identifier == AST_Node_Type::Finally) {
                    retval = this->children.back()->children[0]->eval(t_ss);
                }

                return retval;
            }
        };

        template<typename T>
        struct Catch_AST_Node final : AST_Node_Impl<T> {
            Catch_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Catch, std::move(t_loc), std::move(t_children)) {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Catch, true);
            }
        };

        template<typename T>
        struct Finally_AST_Node final : AST_Node_Impl<T> {
            Finally_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Finally, std::move(t_loc), std::move(t_children)) {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Finally, true);
            }
        };

        template<typename T>
        struct Method_AST_Node final : AST_Node_Impl<T> {
            chaiscript::shared_ptr<AST_Node_Impl<T>> m_body_node;
            chaiscript::shared_ptr<AST_Node_Impl<T>> m_guard_node;

            Method_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text),
                    AST_Node_Type::Method,
                    std::move(t_loc),
                    chaiscript::small_vector<AST_Node_Impl_Ptr<T>>(std::make_move_iterator(t_children.begin()),
                        std::make_move_iterator(
                            std::prev(t_children.end(), Def_AST_Node<T>::has_guard(t_children, 1) ? 2 : 1))))
                , m_body_node(Def_AST_Node<T>::get_body_node(std::move(t_children)))
                , m_guard_node(Def_AST_Node<T>::get_guard_node(std::move(t_children), t_children.size() - this->children.size() == 2)) {
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Method, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                AST_Node_Impl_Ptr<T> guardnode;

                const std::string& class_name = this->children[0]->text;

                // The first param of a method is always the implied this ptr.
                chaiscript::small_vector<std::string> t_param_names{ "this" };
                dispatch::Param_Types param_types;

                if ((this->children.size() > 2) && (this->children[2]->identifier == AST_Node_Type::Arg_List)) {
                    auto args = Arg_List_AST_Node<T>::get_arg_names(*this->children[2]);
                    t_param_names.insert(t_param_names.end(), args.begin(), args.end());
                    param_types = Arg_List_AST_Node<T>::get_arg_types(*this->children[2], t_ss);
                }

                const size_t numparams = t_param_names.size();

                chaiscript::shared_ptr<dispatch::Proxy_Function_Base> guard;
                std::reference_wrapper<chaiscript::detail::Dispatch_Engine> engine(*t_ss);
                if (m_guard_node) {
                    guard = dispatch::make_dynamic_proxy_function(
                        [engine, t_param_names, guardnode = m_guard_node](const Function_Params& t_params) {
                            return chaiscript::eval::detail::eval_function(engine, *guardnode, t_param_names, t_params);
                        },
                        static_cast<int>(numparams),
                            m_guard_node);
                }

                try {
                    const std::string& function_name = this->children[1]->text;

                    if (function_name == class_name) {
                        param_types.push_front(class_name, Type_Info());

                        AUTO funcPtr = dispatch::make_dynamic_proxy_function(
                            [engine, t_param_names, node = m_body_node](const Function_Params& t_params) {
                                return chaiscript::eval::detail::eval_function(engine, *node, t_param_names, t_params);
                            },
                            static_cast<int>(numparams),
                                m_body_node,
                                param_types,
                                guard);
                        funcPtr->set_parameterNames(t_param_names);
                        t_ss->add(chaiscript::make_shared<dispatch::detail::Dynamic_Object_Constructor>( class_name, funcPtr ), function_name);

                    }
                    else {
                        // if the type is unknown, then this generates a function that looks up the type
                        // at runtime. Defining the type first before this is called is better
                        auto type = t_ss->get_type(class_name, false);
                        param_types.push_front(class_name, type);
                        AUTO funcPtr = dispatch::make_dynamic_proxy_function(
                            [engine, t_param_names, node = m_body_node](const Function_Params& t_params) {
                                return chaiscript::eval::detail::eval_function(engine, *node, t_param_names, t_params);
                            },
                            static_cast<int>(numparams),
                                m_body_node,
                                param_types,
                                guard);
                        funcPtr->set_parameterNames(t_param_names);
                        t_ss->add(chaiscript::make_shared<dispatch::detail::Dynamic_Object_Function>(
                            class_name,
                            funcPtr,
                            type),
                            function_name);
                    }
                }
                catch (const exception::name_conflict_error& e) {
                    throw exception::eval_error("Method redefined '" + e.name() + "'");
                }
                return void_var();
            }
        };

        template<typename T>
        struct Attr_Decl_AST_Node final : AST_Node_Impl<T> {
            Attr_Decl_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Attr_Decl, std::move(t_loc), std::move(t_children)) {
                // we are in a class statement, but the initial thing will be a void. 
                this->potentialReturnType = ReturnType(user_type<void>(), AST_Node_Type::Attr_Decl, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                std::string class_name = this->children[0]->text;

                try {
                    std::string attr_name = this->children[1]->text;

                    t_ss->add(chaiscript::make_shared<dispatch::detail::Dynamic_Object_Function>(std::move(class_name),
                        fun([attr_name](dispatch::Dynamic_Object& t_obj) {
                            return t_obj.get_attr(attr_name);
                            }),
                        true

                                ),
                        this->children[1]->text);
                }
                catch (const exception::name_conflict_error& e) {
                    throw exception::eval_error("Attribute redefined '" + e.name() + "'");
                }
                return void_var();
            }
        };

        template<typename T>
        struct Logical_And_AST_Node final : AST_Node_Impl<T> {
            Logical_And_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Logical_And, std::move(t_loc), std::move(t_children)) {
                assert(this->children.size() == 2);
                this->potentialReturnType = ReturnType(user_type<bool>(), AST_Node_Type::Logical_And, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                return const_var(this->get_bool_condition(this->children[0]->eval(t_ss), t_ss)
                    && this->get_bool_condition(this->children[1]->eval(t_ss), t_ss));
            }
        };

        template<typename T>
        struct Logical_Or_AST_Node final : AST_Node_Impl<T> {
            Logical_Or_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, chaiscript::small_vector<AST_Node_Impl_Ptr<T>> t_children)
                : AST_Node_Impl<T>(std::move(t_ast_node_text), AST_Node_Type::Logical_Or, std::move(t_loc), std::move(t_children)) {
                assert(this->children.size() == 2);
                this->potentialReturnType = ReturnType(user_type<bool>(), AST_Node_Type::Logical_Or, true);
            }

            Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State& t_ss) const override {
                EarlyExitCheck(t_ss);
                return const_var(this->get_bool_condition(this->children[0]->eval(t_ss), t_ss)
                    || this->get_bool_condition(this->children[1]->eval(t_ss), t_ss));
            }
        };
    } // namespace eval

} // namespace chaiscript