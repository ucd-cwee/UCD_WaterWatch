#pragma once
#include "Precompiled.h"
// #define CHAISCRIPT_NO_THREADS
#define CHAISCRIPT_ALLOW_NAME_CONFLICTS
#include <future>
#include "Chaiscript_QuickFlatMap.h"
#include "Chaiscript_Defines.h"
#include "Chaiscript_ConstexprString.h"
#include "Chaiscript_Hash.h"
#include "Chaiscript_Algebra.h"
#include "Chaiscript_Any.h"
#include "Chaiscript_Type_Info.h"
#include "Chaiscript_BadBoxedCast.h"
#include "Chaiscript_BoxedValue.h"
#include "Chaiscript_BoxedCastHelper.h"
#include "Chaiscript_Threading.h"
#include "Chaiscript_TypeConversions.h"
#include "Chaiscript_BoxedCast.h"
#include "Chaiscript_BoxedNumber.h"
#include "Chaiscript_DynamicObject.h"
#include "Chaiscript_FunctionParams.h"
#include "Chaiscript_ProxyFunctions.h"
#include "Chaiscript_ProxyConstructors.h"
#include "Chaiscript_ShortAlloc.h" // why is this here / why is this included? It's a char-byte allocator i.e. cweeUnion<...>
#include "Chaiscript_Postfix.h"
#include "Chaiscript_DispatchKit.h"
#include "Chaiscript_LanguageCommon.h"
#include "Chaiscript_FunctionCall.h"
#include "Chaiscript_DynamicObjectImpl.h" // Was after the chaiscript engine...
#include "Chaiscript_Eval.h" 
#include "Chaiscript_Optimizer.h" 
#include "Chaiscript_Tracer.h"

// Bootstrap code
#include "Chaiscript_Operators.h"
#include "Chaiscript_Utility.h" // Programmer helper to add Classes to chaiscript module
#include "Chaiscript_Bootstrap.h" // Programmer helper to add Classes to chaiscript module
#include "Chaiscript_STL_Bootstrap.h"
#include "Chaiscript_Prelude.h" // pre-defined functions for the chaiscript engine to get it going
#include "Chaiscript_JSON.h"
#include "Chaiscript_STL_Module.h" // loads the json and the prelude and the STL bootstrap into a module, loaded all at once into chaiscript_basic on instantiation

// engine? 
#include "Chaiscript_Windows.h"
#include "Chaiscript_ExceptionSpecification.h"
#include "Chaiscript_Engine.h"
#include "Chaiscript_Parser.h"
#include "Chaiscript.h"
// static cweeSharedPtr<chaiscript::ChaiScript> shared_chaiscript = make_cwee_shared<chaiscript::ChaiScript>();