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

namespace chaiscript {
    namespace optimizer {
        template<typename... T>
        struct Optimizer : T... { Optimizer() = default; explicit Optimizer(T... t) : T(std::move(t))... { } template<typename Tracer> auto optimize(eval::AST_Node_Impl_Ptr<Tracer> p) {
                ((p = static_cast<T&>(*this).optimize(std::move(p))), ...);
                return p;
            } };

        template<typename T>
        eval::AST_Node_Impl<T>& child_at(eval::AST_Node_Impl<T>& node, const size_t offset) noexcept {
            if (node.children[offset]->identifier == AST_Node_Type::Compiled) {
                return *(dynamic_cast<eval::Compiled_AST_Node<T> &>(*node.children[offset]).m_original_node);
            }
            else {
                return *node.children[offset];
            }
        }

        template<typename T>
        const eval::AST_Node_Impl<T>& child_at(const eval::AST_Node_Impl<T>& node, const size_t offset) noexcept {
            if (node.children[offset]->identifier == AST_Node_Type::Compiled) {
                return *(dynamic_cast<const eval::Compiled_AST_Node<T> &>(*node.children[offset]).m_original_node);
            }
            else {
                return *node.children[offset];
            }

            /*
          if (node->identifier == AST_Node_Type::Compiled) {
            return dynamic_cast<const eval::Compiled_AST_Node<T>&>(*node).m_original_node->children[offset];
          } else {
            return node->children[offset];
          }
          */
        }

        template<typename T>
        auto child_count(const eval::AST_Node_Impl<T>& node) noexcept {
            if (node.identifier == AST_Node_Type::Compiled) {
                return dynamic_cast<const eval::Compiled_AST_Node<T> &>(node).m_original_node->children.size();
            }
            else {
                return node.children.size();
            }
        }

        template<typename T, typename Callable>
        auto make_compiled_node(eval::AST_Node_Impl_Ptr<T> original_node, chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> children, Callable callable) {
            return chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Compiled_AST_Node<T>>(std::move(original_node),
                std::move(children),
                std::move(callable));
        }

        struct Return {
            template<typename T>
            auto optimize(eval::AST_Node_Impl_Ptr<T> p) {
                if ((p->identifier == AST_Node_Type::Def || p->identifier == AST_Node_Type::Lambda) && !p->children.empty()) {
                    auto& last_child = p->children.back();
                    if (last_child->identifier == AST_Node_Type::Block) {
                        auto& block_last_child = last_child->children.back();
                        if (block_last_child->identifier == AST_Node_Type::Return) {
                            if (block_last_child->children.size() == 1) {
                                last_child->children.back() = std::move(block_last_child->children[0]);
                            }
                        }
                    }
                }

                return p;
            }
        };

        template<typename T>
        bool contains_var_decl_in_scope(const eval::AST_Node_Impl<T>& node) noexcept {
            if (node.identifier == AST_Node_Type::Var_Decl || node.identifier == AST_Node_Type::Assign_Decl
                || node.identifier == AST_Node_Type::Reference) {
                return true;
            }

            const auto num = child_count(node);

            for (size_t i = 0; i < num; ++i) {
                const auto& child = child_at(node, i);
                if (child.identifier != AST_Node_Type::Block && child.identifier != AST_Node_Type::For
                    && child.identifier != AST_Node_Type::Ranged_For && contains_var_decl_in_scope(child)) {
                    return true;
                }
            }

            return false;
        }

        struct Block {
            template<typename T>
            auto optimize(eval::AST_Node_Impl_Ptr<T> node) {
                if (node->identifier == AST_Node_Type::Block) {
                    if (!contains_var_decl_in_scope(*node)) {
                        if (node->children.size() == 1) {
                            return std::move(node->children[0]);
                        }
                        else {
                            return chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Scopeless_Block_AST_Node<T>>(node->text,
                                node->location,
                                std::move(node->children));
                        }
                    }
                }

                return node;
            }
        };

        struct Dead_Code {
            template<typename T>
            auto optimize(eval::AST_Node_Impl_Ptr<T> node) {
                if (node->identifier == AST_Node_Type::Block) {
                    chaiscript::small_vector<size_t> keepers;
                    const auto num_children = node->children.size();
                    keepers.reserve(num_children);
                    
                    for (size_t i = 0; i < (num_children - 1); ++i) {
                        const auto& child = *node->children[i];

                        switch (child.identifier) {
                        case AST_Node_Type::Constant: break;
                        case AST_Node_Type::Noop: break;
                        // case AST_Node_Type::Id: break;
                        default: keepers.push_back(i); break;
                        }
                    }
                    if (num_children > 0) { keepers.push_back(num_children - 1); };

                    if (keepers.size() == num_children) {
                        return node;
                    }
                    else {
                        const auto new_children = [&]() {
                            chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> retval;
                            for (const auto x : keepers) {
                                retval.push_back(std::move(node->children[x]));
                            }
                            return retval;
                        };

                        return chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Block_AST_Node<T>>(node->text, node->location, new_children());
                    }
                }
                else {
                    return node;
                }
            }
        };

        struct Unused_Return {
            template<typename T>
            auto optimize(eval::AST_Node_Impl_Ptr<T> node) {
                if ((node->identifier == AST_Node_Type::Block || node->identifier == AST_Node_Type::Scopeless_Block) && !node->children.empty()) {
                    for (size_t i = 0; i < node->children.size() - 1; ++i) {
                        auto child = node->children[i].get();
                        if (child->identifier == AST_Node_Type::Fun_Call) {
                            node->children[i]
                                = chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Unused_Return_Fun_Call_AST_Node<T>>(child->text,
                                    child->location,
                                    std::move(child->children));
                        }
                    }
                }
                else if ((node->identifier == AST_Node_Type::For || node->identifier == AST_Node_Type::While) && child_count(*node) > 0) {
                    auto& child = child_at(*node, child_count(*node) - 1);
                    if (child.identifier == AST_Node_Type::Block || child.identifier == AST_Node_Type::Scopeless_Block) {
                        auto num_sub_children = child_count(child);
                        for (size_t i = 0; i < num_sub_children; ++i) {
                            auto& sub_child = child_at(child, i);
                            if (sub_child.identifier == AST_Node_Type::Fun_Call) {
                                child.children[i] = chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Unused_Return_Fun_Call_AST_Node<T>>(
                                    sub_child.text, sub_child.location, std::move(sub_child.children));
                            }
                        }
                    }
                }
                return node;
            }
        };

        struct Assign_Decl {
            template<typename T>
            auto optimize(eval::AST_Node_Impl_Ptr<T> node) {
                if ((node->identifier == AST_Node_Type::Equation) && node->children.size() == 2 
                    && node->children[0]->identifier == AST_Node_Type::Var_Decl) {
                    if (node->text == "=") {
                        AUTO prtPtr = node->children[1]->potentialReturnType;
                        chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> new_children;
                        new_children.push_back(std::move(node->children[0]->children[0]));
                        new_children.push_back(std::move(node->children[1]));
                        AUTO eq = chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Assign_Decl_AST_Node<T>>(node->text,
                            node->location,
                            std::move(new_children));
                        eq->potentialReturnType.ForwardRef(prtPtr);
                        return eq;
                    }
                    else if (node->text == "?=") {
                        AUTO prtPtr = node->children[1]->potentialReturnType;
                        chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> new_children;
                        new_children.push_back(std::move(node->children[0]->children[0]));
                        new_children.push_back(std::move(node->children[1]));
                        AUTO eq = chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Assign_Decl_IfNotDef_AST_Node<T>>(node->text,
                            node->location,
                            std::move(new_children));
                        eq->potentialReturnType.ForwardRef(prtPtr);
                        return eq;
                    }
                }else if ((node->identifier == AST_Node_Type::Equation) && node->children.size() == 2
                    && node->children[0]->identifier == AST_Node_Type::Reference) {
                    if (node->text == "=") {
                        AUTO prtPtr = node->children[1]->potentialReturnType;
                        chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> new_children;
                        new_children.push_back(std::move(node->children[0]->children[0]));
                        new_children.push_back(std::move(node->children[1]));
                        AUTO eq = chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Assign_Decl_AST_Node<T>>(node->text,
                            node->location,
                            std::move(new_children));
                        eq->potentialReturnType.ForwardRef(prtPtr);
                        return eq;
                    }
                    else if (node->text == "?=") {
                        AUTO prtPtr = node->children[1]->potentialReturnType;
                        chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> new_children;
                        new_children.push_back(std::move(node->children[0]->children[0]));
                        new_children.push_back(std::move(node->children[1]));
                        AUTO eq = chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Assign_Decl_IfNotDef_AST_Node<T>>(node->text,
                            node->location,
                            std::move(new_children));
                        eq->potentialReturnType.ForwardRef(prtPtr);
                        return eq;
                    }
                }
                return node;
            }
        };

        struct Retro_Assign_Decl {
            template<typename T>
            auto optimize(eval::AST_Node_Impl_Ptr<T> node) {
                if ((node->identifier == AST_Node_Type::Equation) && node->children.size() == 2) {
                    if (node->children[0]->identifier == AST_Node_Type::Assign_Retroactively) {
                        if (node->children[0]->children[0]->identifier == AST_Node_Type::Fun_Call) {
                            std::string eqText = node->text;
                            if (eqText == "?=") {
                                eqText = "=";
                            }

                            if (node->children[0]->children[1]->identifier == AST_Node_Type::Reference) {
                                // { Equation<"=">(Ref, FunCall); Equation<"=">(Id, UNKNOWN); }

                                AUTO IdNode = chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Id_AST_Node<T>>(
                                    node->children[0]->children[1]->children[0]->text, 
                                    node->children[0]->children[1]->children[0]->location
                                );
                                IdNode->potentialReturnType.ForwardRef(node->children[0]->children[0]->potentialReturnType);

                                chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> block_children;
                                {
                                    chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> new_children;
                                    new_children.push_back(std::move(node->children[0]->children[1]));
                                    new_children.push_back(std::move(node->children[0]->children[0]));
                                    auto prtPtr = new_children[1]->potentialReturnType;
                                    AUTO eq = chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Equation_AST_Node<T>>("=",
                                        node->location,
                                        std::move(new_children));
                                    eq->potentialReturnType.ForwardRef(prtPtr);
                                    block_children.push_back(std::move(eq));
                                }
                                {
                                    chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> new_children;
                                    new_children.push_back(std::move(IdNode));
                                    new_children.push_back(std::move(node->children[1]));
                                    auto prtPtr = new_children[0]->potentialReturnType;
                                    AUTO eq = chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Equation_AST_Node<T>>(eqText,
                                        node->location,
                                        std::move(new_children));
                                    eq->potentialReturnType.ForwardRef(prtPtr);
                                    block_children.push_back(std::move(eq));
                                }
                                return chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Block_AST_Node<T>>("",
                                    node->location,
                                    std::move(block_children));
                            }
                            else if (node->children[0]->children[1]->identifier == AST_Node_Type::Var_Decl) {
                                // { Equation<"=">(Var_Decl, FunCall); Equation<"=">(Id, UNKNOWN); }

                                AUTO IdNode = chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Id_AST_Node<T>>(
                                    node->children[0]->children[1]->children[0]->text,
                                    node->children[0]->children[1]->children[0]->location
                                );
                                IdNode->potentialReturnType.ForwardRef(node->children[0]->children[0]->potentialReturnType);

                                chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> block_children;
                                {
                                    chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> new_children;
                                    new_children.push_back(std::move(node->children[0]->children[1]));
                                    new_children.push_back(std::move(node->children[0]->children[0]));
                                    auto prtPtr = new_children[1]->potentialReturnType;
                                    AUTO eq = chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Equation_AST_Node<T>>("=",
                                        node->location,
                                        std::move(new_children));
                                    eq->potentialReturnType.ForwardRef(prtPtr);
                                    block_children.push_back(std::move(eq));
                                }
                                {
                                    chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> new_children;
                                    new_children.push_back(std::move(IdNode));
                                    new_children.push_back(std::move(node->children[1]));
                                    auto prtPtr = new_children[0]->potentialReturnType;
                                    AUTO eq = chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Equation_AST_Node<T>>(eqText,
                                        node->location,
                                        std::move(new_children));
                                    eq->potentialReturnType.ForwardRef(prtPtr);
                                    block_children.push_back(std::move(eq));
                                }
                                return chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Block_AST_Node<T>>("",
                                    node->location,
                                    std::move(block_children));
                            }
                        }
                    }
                }
                if ((node->identifier == AST_Node_Type::Assign_Retroactively) && node->children.size() == 2) {
                    if (node->children[0]->identifier == AST_Node_Type::Fun_Call) {
                        if (node->children[1]->identifier == AST_Node_Type::Reference) {
                            chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> new_children;
                            new_children.push_back(std::move(node->children[1]));
                            new_children.push_back(std::move(node->children[0]));
                            auto prtPtr = new_children[0]->potentialReturnType;
                            AUTO eq = chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Equation_AST_Node<T>>("=",
                                node->location,
                                std::move(new_children));
                            eq->potentialReturnType.ForwardRef(prtPtr);
                            return std::move(eq);
                        }
                        else if (node->children[1]->identifier == AST_Node_Type::Var_Decl) {
                            chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> new_children;
                            new_children.push_back(std::move(node->children[1]));
                            new_children.push_back(std::move(node->children[0]));
                            auto prtPtr = new_children[0]->potentialReturnType;
                            AUTO eq = chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Equation_AST_Node<T>>("=",
                                node->location,
                                std::move(new_children));
                            eq->potentialReturnType.ForwardRef(prtPtr);
                            return std::move(eq);
                        }
                    }
                }
                return node;
            }
        };

        struct If {
            template<typename T>
            auto optimize(eval::AST_Node_Impl_Ptr<T> node) {
                if ((node->identifier == AST_Node_Type::If) && node->children.size() >= 2 && node->children[0]->identifier == AST_Node_Type::Constant) {
                    const auto condition = dynamic_cast<eval::Constant_AST_Node<T> *>(node->children[0].get())->m_value;
                    if (condition.get_type_info().bare_equal_type_info(typeid(bool))) {
                        if (boxed_cast<bool>(condition)) {
                            return std::move(node->children[1]);
                        }
                        else if (node->children.size() == 3) {
                            return std::move(node->children[2]);
                        }
                    }
                }

                return node;
            }
        };

        struct Partial_Fold {
            template<typename T>
            auto optimize(eval::AST_Node_Impl_Ptr<T> node) {
                // Fold right side
                if (node->identifier == AST_Node_Type::Binary && node->children.size() == 2
                    && node->children[0]->identifier != AST_Node_Type::Constant && node->children[1]->identifier == AST_Node_Type::Constant) {
                    try {
                        const auto& oper = node->text;
                        const auto parsed = Operators::to_operator(oper);
                        if (parsed != Operators::Opers::invalid) {
                            const auto rhs = dynamic_cast<eval::Constant_AST_Node<T> *>(node->children[1].get())->m_value;
                            if (rhs.get_type_info().is_arithmetic()) {
                                return chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Fold_Right_Binary_Operator_AST_Node<T>>(
                                    node->text, node->location, std::move(node->children), rhs);
                            }
                        }
                    }
                    catch (const std::exception&) {
                        // failure to fold, that's OK
                    }
                }

                return node;
            }
        };

        struct Constant_Fold {
            template<typename T>
            auto optimize(eval::AST_Node_Impl_Ptr<T> node) {
                if (node->identifier == AST_Node_Type::Prefix && node->children.size() == 1 && node->children[0]->identifier == AST_Node_Type::Constant) {
                    try {
                        const auto& oper = node->text;
                        const auto parsed = Operators::to_operator(oper, true);
                        const auto lhs = dynamic_cast<const eval::Constant_AST_Node<T> *>(node->children[0].get())->m_value;
                        const auto match = oper + node->children[0]->text;

                        if (parsed != Operators::Opers::invalid && parsed != Operators::Opers::bitwise_and && lhs.get_type_info().is_arithmetic()) {
                            const auto val = Boxed_Number::do_oper(parsed, lhs);
                            return chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Constant_AST_Node<T>>(std::move(match),
                                node->location,
                                std::move(val));
                        }
                        else if (lhs.get_type_info().bare_equal_type_info(typeid(bool)) && oper == "!") {
                            return chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Constant_AST_Node<T>>(std::move(match),
                                node->location,
                                Boxed_Value(!boxed_cast<bool>(lhs)));
                        }
                    }
                    catch (const std::exception&) {
                        // failure to fold, that's OK
                    }
                }
                else if (node->identifier == AST_Node_Type::Postfix && node->children.size() == 1 && node->children[0]->identifier == AST_Node_Type::Constant) {
                    try {
                        const auto& oper = node->text;
                        const auto parsed = Operators::to_operator(oper, true);
                        const auto lhs = dynamic_cast<const eval::Constant_AST_Node<T> *>(node->children[0].get())->m_value;
                        const auto match = node->children[0]->text + oper;

                        if (parsed != Operators::Opers::invalid) { // invalid indicates a custom postfix
                            // postfix increment or decrement will always return the lhs value; 
                            return chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Constant_AST_Node<T>>(std::move(match), node->location, lhs);
                        }
                    }
                    catch (const std::exception&) {
                        // failure to fold, that's OK
                    }
                }
                else if ((node->identifier == AST_Node_Type::Logical_And || node->identifier == AST_Node_Type::Logical_Or)
                    && node->children.size() == 2 && node->children[0]->identifier == AST_Node_Type::Constant
                    && node->children[1]->identifier == AST_Node_Type::Constant) {
                    try {
                        const auto lhs = dynamic_cast<const eval::Constant_AST_Node<T> &>(*node->children[0]).m_value;
                        const auto rhs = dynamic_cast<const eval::Constant_AST_Node<T> &>(*node->children[1]).m_value;
                        if (lhs.get_type_info().bare_equal_type_info(typeid(bool)) && rhs.get_type_info().bare_equal_type_info(typeid(bool))) {
                            const auto match = node->children[0]->text + " " + node->text + " " + node->children[1]->text;
                            const auto val = [lhs_val = boxed_cast<bool>(lhs), rhs_val = boxed_cast<bool>(rhs), id = node->identifier]{
                              if (id == AST_Node_Type::Logical_And) {
                                return Boxed_Value(lhs_val && rhs_val);
                              }
                              else {
                                return Boxed_Value(lhs_val || rhs_val);
                              }
                            }();

                            return chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Constant_AST_Node<T>>(std::move(match),
                                node->location,
                                std::move(val));
                        }
                    }
                    catch (const std::exception&) {
                        // failure to fold, that's OK
                    }
                }
                else if (node->identifier == AST_Node_Type::Binary && node->children.size() == 2
                    && node->children[0]->identifier == AST_Node_Type::Constant && node->children[1]->identifier == AST_Node_Type::Constant) {
                    try {
                        const auto& oper = node->text;
                        const auto parsed = Operators::to_operator(oper);
                        if (parsed != Operators::Opers::invalid) {
                            const auto lhs = dynamic_cast<const eval::Constant_AST_Node<T> &>(*node->children[0]).m_value;
                            const auto rhs = dynamic_cast<const eval::Constant_AST_Node<T> &>(*node->children[1]).m_value;
                            if (lhs.get_type_info().is_arithmetic() && rhs.get_type_info().is_arithmetic()) {
                                const auto val = Boxed_Number::do_oper(parsed, lhs, rhs);
                                const auto match = node->children[0]->text + " " + oper + " " + node->children[1]->text;
                                return chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Constant_AST_Node<T>>(std::move(match),
                                    node->location,
                                    std::move(val));
                            }
                        }
                    }
                    catch (const std::exception&) {
                        // failure to fold, that's OK
                    }
                }
                else if (node->identifier == AST_Node_Type::Fun_Call && node->children.size() == 2
                    && node->children[0]->identifier == AST_Node_Type::Id && node->children[1]->identifier == AST_Node_Type::Arg_List
                    && node->children[1]->children.size() == 1 && node->children[1]->children[0]->identifier == AST_Node_Type::Constant) {
                    const auto arg = dynamic_cast<const eval::Constant_AST_Node<T> &>(*node->children[1]->children[0]).m_value;
                    if (arg.get_type_info().is_arithmetic()) {
                        const auto& fun_name = node->children[0]->text;

                        const auto make_constant = [&node, &fun_name](auto val) {
                            const auto match = fun_name + "(" + node->children[1]->children[0]->text + ")";
                            return chaiscript::make_unique<eval::AST_Node_Impl<T>, eval::Constant_AST_Node<T>>(std::move(match),
                                node->location,
                                const_var(val));
                        };

                        if (fun_name == "double") {
                            return make_constant(Boxed_Number(arg).get_as<double>());
                        }
                        else if (fun_name == "int") {
                            return make_constant(Boxed_Number(arg).get_as<int>());
                        }
                        else if (fun_name == "float") {
                            return make_constant(Boxed_Number(arg).get_as<float>());
                        }
                        else if (fun_name == "long") {
                            return make_constant(Boxed_Number(arg).get_as<long>());
                        }
                        else if (fun_name == "size_t") {
                            return make_constant(Boxed_Number(arg).get_as<size_t>());
                        }
                    }
                }

                return node;
            }
        };

        struct While_Loop {
            template<typename T>
            auto optimize(eval::AST_Node_Impl_Ptr<T> node) {
                if ((node->identifier == AST_Node_Type::While) && node->children.size() >= 2 && node->children[0]->identifier == AST_Node_Type::Constant) {
                    const auto condition = dynamic_cast<eval::Constant_AST_Node<T> *>(node->children[0].get())->m_value;
                    if (condition.get_type_info().bare_equal_type_info(typeid(bool))) {
                        if (boxed_cast<bool>(condition)) {
                            chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> body_vector;
                            auto body_child = std::move(node->children[1]);
                            node->children.pop_back();
                            body_vector.emplace_back(std::move(body_child));

                            return make_compiled_node(std::move(node),
                                std::move(body_vector),
                                [](const chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>>& children,
                                    const chaiscript::detail::Dispatch_State& t_ss) {
                                        assert(children.size() == 1);
                                        chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

                                        try {
                                            while (true) {
                                                try {
                                                    // Body of Loop
                                                    children[0]->eval(t_ss);
                                                }
                                                catch (eval::detail::Continue_Loop&) {
                                                    // we got a continue exception, which means all of the remaining
                                                    // loop implementation is skipped and we just need to continue to
                                                    // the next iteration step
                                                }
                                            }
                                        }
                                        catch (eval::detail::Break_Loop&) {
                                            // loop broken
                                        }
                                        return void_var();
                                });
                        }
                        else {
                            chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> body_vector;
                            return make_compiled_node(std::move(node), std::move(body_vector),
                                [](const chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>>& children, const chaiscript::detail::Dispatch_State& t_ss) {
                                    return void_var();
                                });
                        }
                    }
                }
                return node;
            }
        };

        struct For_Loop {
            template<typename T>
            auto optimize(eval::AST_Node_Impl_Ptr<T> for_node) {
                if (for_node->identifier != AST_Node_Type::For) {
                    return for_node;
                }

                const auto& eq_node = child_at(*for_node, 0);
                const auto& binary_node = child_at(*for_node, 1);
                const auto& prefix_node = child_at(*for_node, 2);

                // in-line declaration of a variable, AND a binary operation is used in the binary slot
                if (child_count(*for_node) == 4 && eq_node.identifier == AST_Node_Type::Assign_Decl && binary_node.identifier == AST_Node_Type::Binary) {                    
                    // in-line declaration of a variable using an ID and a constant (i.e. var& i = 0)
                    if (child_count(eq_node) == 2
                        && child_at(eq_node, 0).identifier == AST_Node_Type::Id 
                        && child_at(eq_node, 1).identifier == AST_Node_Type::Constant) {

                        // binary test with the correct variable name and a constant
                        if (child_count(binary_node) == 2
                            && child_at(binary_node, 0).identifier == AST_Node_Type::Id 
                            && child_at(binary_node, 0).text == child_at(eq_node, 0).text
                            && child_at(binary_node, 1).identifier == AST_Node_Type::Constant
                            ) {
                            
                            const std::string& id = child_at(eq_node, 0).text;
                            const Boxed_Value& begin = dynamic_cast<const eval::Constant_AST_Node<T> &>(child_at(eq_node, 1)).m_value;
                            double start_int = 0; // = boxed_cast<int>(begin);
                            int mode = 0;
                            if (begin.get_type_info().bare_equal(user_type<int>())) {
                                start_int = boxed_cast<int>(begin);
                                mode = 0;
                            }
                            else if (begin.get_type_info().bare_equal(user_type<float>())) {
                                start_int = boxed_cast<float>(begin);
                                mode = 1;
                            }
                            else if (begin.get_type_info().bare_equal(user_type<double>())) {
                                start_int = boxed_cast<double>(begin);
                                mode = 2;
                            }
                            else {
                                return for_node;
                            }

                            const Boxed_Value& end = dynamic_cast<const eval::Constant_AST_Node<T> &>(child_at(binary_node, 1)).m_value;
                            double end_int = 0;
                            if (end.get_type_info().bare_equal(user_type<int>())) {
                                end_int = boxed_cast<int>(end);
                            }
                            else if (end.get_type_info().bare_equal(user_type<float>())) {
                                end_int = boxed_cast<float>(end);
                            }
                            else if (end.get_type_info().bare_equal(user_type<double>())) {
                                end_int = boxed_cast<double>(end);
                            }
                            else {
                                return for_node;
                            }

                            cweeSharedPtr<std::function<bool(double&)>> finishedFunc;  {
                                const auto parsed = Operators::to_operator(binary_node.text);
                                switch (parsed) {
                                case Operators::Opers::equals:
                                    finishedFunc = make_cwee_shared<std::function<bool(double&)>>([=](double& val) {
                                        return val == end_int;
                                    });
                                    break;
                                case Operators::Opers::less_than:
                                    finishedFunc = make_cwee_shared<std::function<bool(double&)>>([=](double& val) {
                                        return val < end_int;
                                    });
                                    break;
                                case Operators::Opers::less_than_equal:
                                    finishedFunc = make_cwee_shared<std::function<bool(double&)>>([=](double& val) {
                                        return val <= end_int;
                                    });
                                    break;
                                case Operators::Opers::greater_than:
                                    finishedFunc = make_cwee_shared<std::function<bool(double&)>>([=](double& val) {
                                        return val > end_int;
                                    });
                                    break;
                                case Operators::Opers::greater_than_equal:
                                    finishedFunc = make_cwee_shared<std::function<bool(double&)>>([=](double& val) {
                                        return val >= end_int;
                                    });
                                    break;
                                case Operators::Opers::not_equal:
                                    finishedFunc = make_cwee_shared<std::function<bool(double&)>>([=](double& val) {
                                        return val != end_int;
                                    });
                                    break;
                                default:
                                    return for_node;
                                }                            
                            }
                            cweeSharedPtr<std::function<void(double&)>> incrementFunc; {

                                // prefix? i.e. ++i
                                if (prefix_node.identifier == AST_Node_Type::Prefix) {
                                    if (child_count(prefix_node) == 1
                                        && child_at(prefix_node, 0).identifier == AST_Node_Type::Id
                                        && child_at(prefix_node, 0).text == child_at(eq_node, 0).text) {

                                        const auto parsed = Operators::to_operator(prefix_node.text);
                                        switch (parsed) {
                                        case Operators::Opers::pre_increment:
                                            incrementFunc = make_cwee_shared<std::function<void(double&)>>([=](double& val) {
                                                ++val;
                                                });
                                            break;
                                        case Operators::Opers::pre_decrement:
                                            incrementFunc = make_cwee_shared<std::function<void(double&)>>([=](double& val) {
                                                --val;
                                                });
                                            break;
                                        default:
                                            return for_node;
                                        }

                                    }
                                }
                                // postfix? i.e. i++
                                else if (prefix_node.identifier == AST_Node_Type::Postfix) {
                                    if (child_count(prefix_node) == 1
                                        && child_at(prefix_node, 0).identifier == AST_Node_Type::Id
                                        && child_at(prefix_node, 0).text == child_at(eq_node, 0).text) {
                                        const auto parsed = Operators::to_operator(prefix_node.text);
                                        switch (parsed) {
                                        case Operators::Opers::pre_increment:
                                            incrementFunc = make_cwee_shared<std::function<void(double&)>>([=](double& val) {
                                                ++val;
                                                });
                                            break;
                                        case Operators::Opers::pre_decrement:
                                            incrementFunc = make_cwee_shared<std::function<void(double&)>>([=](double& val) {
                                                --val;
                                                });
                                            break;
                                        default:
                                            return for_node;
                                        }
                                    }
                                }
                                // equation? I.e. i += 2;
                                else if (prefix_node.identifier == AST_Node_Type::Equation) {
                                    if (child_count(prefix_node) == 2
                                        && child_at(prefix_node, 0).identifier == AST_Node_Type::Id
                                        && child_at(prefix_node, 0).text == child_at(eq_node, 0).text
                                        && child_at(prefix_node, 1).identifier == AST_Node_Type::Constant
                                        ) {
                                        const Boxed_Value& incremental = dynamic_cast<const eval::Constant_AST_Node<T> &>(child_at(prefix_node, 1)).m_value;
                                        double incremental_int = 0;
                                        if (incremental.get_type_info().bare_equal(user_type<int>())) {
                                            incremental_int = boxed_cast<int>(incremental);
                                        }
                                        else if (incremental.get_type_info().bare_equal(user_type<float>())) {
                                            incremental_int = boxed_cast<float>(incremental);
                                        }
                                        else if (incremental.get_type_info().bare_equal(user_type<double>())) {
                                            incremental_int = boxed_cast<double>(incremental);
                                        }
                                        else {
                                            return for_node;
                                        }

                                        const auto parsed = Operators::to_operator(prefix_node.text);
                                        switch (parsed) {
                                        case Operators::Opers::assign_sum:
                                            incrementFunc = make_cwee_shared<std::function<void(double&)>>([=](double& val) {
                                                val += incremental_int;
                                                });
                                            break;
                                        case Operators::Opers::assign_difference:
                                            incrementFunc = make_cwee_shared<std::function<void(double&)>>([=](double& val) {
                                                val -= incremental_int;
                                                });
                                            break;
                                        default:
                                            return for_node;
                                        }
                                    }
                                }
                                else {
                                    return for_node;
                                }

                            } 

                            if (finishedFunc && incrementFunc) {
                                // note that we are moving the last element out, then popping the empty shared_ptr
                                // from the vector
                                chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> body_vector;
                                auto body_child = std::move(for_node->children[3]);
                                for_node->children.pop_back();
                                body_vector.emplace_back(std::move(body_child));

                                return make_compiled_node(std::move(for_node), std::move(body_vector),
                                    [mode, finishedFunc, incrementFunc, id, start_int, end_int](const chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>>& children,
                                        const chaiscript::detail::Dispatch_State& t_ss) {
                                            assert(children.size() == 1);
                                            chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);
                                            AUTO i = chaiscript::make_shared<double>(start_int);
                                            auto& func1 = *finishedFunc;
                                            auto& func2 = *incrementFunc;

                                            switch (mode) {
                                            default:
                                            case 0: {
                                                AUTO j = chaiscript::make_shared<int>(start_int);
                                                t_ss.add_object(id, var(j));
                                                try {
                                                    for (; func1(*i); func2(*i)) {
                                                        *j = *i;
                                                        try {
                                                            children[0]->eval(t_ss);
                                                        }
                                                        catch (eval::detail::Continue_Loop&) {}
                                                    }
                                                }
                                                catch (eval::detail::Break_Loop&) {}
                                                break;
                                            }
                                            case 1: {
                                                AUTO j = chaiscript::make_shared<float>(start_int);
                                                t_ss.add_object(id, var(j));
                                                try {
                                                    for (; func1(*i); func2(*i)) {
                                                        *j = *i;
                                                        try {
                                                            children[0]->eval(t_ss);
                                                        }
                                                        catch (eval::detail::Continue_Loop&) {}
                                                    }
                                                }
                                                catch (eval::detail::Break_Loop&) {}
                                                break;
                                            }
                                            case 2: {
                                                t_ss.add_object(id, var(i));
                                                try {
                                                    for (; func1(*i); func2(*i)) {
                                                        try {
                                                            children[0]->eval(t_ss);
                                                        }
                                                        catch (eval::detail::Continue_Loop&) {}
                                                    }
                                                }
                                                catch (eval::detail::Break_Loop&) {}
                                                break;
                                            }
                                            }

                                            return void_var();
                                    });
                            }
                        }
                    }
                }

#if 0
                if (child_count(*for_node) == 4 && eq_node.identifier == AST_Node_Type::Assign_Decl && child_count(eq_node) == 2
                    && child_at(eq_node, 0).identifier == AST_Node_Type::Id && child_at(eq_node, 1).identifier == AST_Node_Type::Constant
                    && binary_node.identifier == AST_Node_Type::Binary && binary_node.text == "<" && child_count(binary_node) == 2
                    && child_at(binary_node, 0).identifier == AST_Node_Type::Id && child_at(binary_node, 0).text == child_at(eq_node, 0).text
                    && child_at(binary_node, 1).identifier == AST_Node_Type::Constant && prefix_node.identifier == AST_Node_Type::Prefix
                    && prefix_node.text == "++" && child_count(prefix_node) == 1 && child_at(prefix_node, 0).identifier == AST_Node_Type::Id
                    && child_at(prefix_node, 0).text == child_at(eq_node, 0).text) {
                    const Boxed_Value& begin = dynamic_cast<const eval::Constant_AST_Node<T> &>(child_at(eq_node, 1)).m_value;
                    const Boxed_Value& end = dynamic_cast<const eval::Constant_AST_Node<T> &>(child_at(binary_node, 1)).m_value;
                    const std::string& id = child_at(prefix_node, 0).text;

                    if (begin.get_type_info().bare_equal(user_type<int>()) && end.get_type_info().bare_equal(user_type<int>())) {
                        const auto start_int = boxed_cast<int>(begin);
                        const auto end_int = boxed_cast<int>(end);

                        // note that we are moving the last element out, then popping the empty shared_ptr
                        // from the vector
                        chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>> body_vector;
                        auto body_child = std::move(for_node->children[3]);
                        for_node->children.pop_back();
                        body_vector.emplace_back(std::move(body_child));

                        return make_compiled_node(std::move(for_node),
                            std::move(body_vector),
                            [id, start_int, end_int](const chaiscript::small_vector<eval::AST_Node_Impl_Ptr<T>>& children,
                                const chaiscript::detail::Dispatch_State& t_ss) {
                                    assert(children.size() == 1);
                                    chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

                                    chaiscript::shared_ptr<int> i = chaiscript::make_shared<int>(start_int);
                                    t_ss.add_object(id, var(i));

                                    try {
                                        for (; *i < end_int; *i += 1) {
                                            try {
                                                // Body of Loop
                                                children[0]->eval(t_ss);
                                            }
                                            catch (eval::detail::Continue_Loop&) {
                                                // we got a continue exception, which means all of the remaining
                                                // loop implementation is skipped and we just need to continue to
                                                // the next iteration step
                                            }
                                        }
                                    }
                                    catch (eval::detail::Break_Loop&) {
                                        // loop broken
                                    }

                                    return void_var();
                            });
                    }
                }
#endif

                return for_node;
            }
        };

        using Optimizer_Default = Optimizer<optimizer::Partial_Fold,
            optimizer::Unused_Return,
            optimizer::Constant_Fold,
            optimizer::If,
            optimizer::Return,
            optimizer::Dead_Code,
            optimizer::Block,
            optimizer::While_Loop,
            optimizer::For_Loop,
            optimizer::Retro_Assign_Decl,
            optimizer::Assign_Decl>;

    } // namespace optimizer
} // namespace chaiscript
