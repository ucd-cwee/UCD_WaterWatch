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
#include "enum.h"

namespace chaiscript {
    /// Types of AST nodes available to the parser and eval
    BETTER_ENUM(AST_Node_Type, uint8_t,
        Id,
        Fun_Call,
        Unused_Return_Fun_Call,
        Arg_List,
        Equation,
        Var_Decl,
        Assign_Decl,
        Array_Call,
        Dot_Access,
        Lambda,
        Block,
        Scopeless_Block,
        Def,
        While,
        If,
        For,        
        Ranged_For,
        Inline_Array,
        Inline_Map,
        Return,
        File,
        Prefix,
        Break,
        Continue,
        Map_Pair,
        Value_Range,
        Inline_Range,
        Do, 
        Try,
        Catch,
        Finally,
        Method,
        Attr_Decl,
        Logical_And,
        Logical_Or,
        Reference,
        Switch,
        Case,
        Default,
        Noop,
        Class,
        Binary,
        Arg,
        Global_Decl,
        Constant,
        Compiled,
        ControlBlock,
        Postfix,
        Assign_Retroactively,
        Parallel,
        AST_Node_Type_end
    );

    struct AST_Node_Trace;

};