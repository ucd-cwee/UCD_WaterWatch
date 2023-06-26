#pragma once
#include "Precompiled.h"
namespace chaiscript {
    class Postfix_Definition {
    public:
        Postfix_Definition() : postFix_m(), functionName_m() {};
        Postfix_Definition(std::string const& postFix_p, std::string const& functionName_p) : postFix_m(postFix_p), functionName_m(functionName_p) {};
        ~Postfix_Definition() {};
        std::string postFix_m;
        std::string functionName_m;
    };
};
namespace chaiscript {
    INLINE Postfix_Definition postfix(std::string const& postFix_p, std::string const& functionName_p) noexcept {
        return Postfix_Definition(postFix_p, functionName_p);
    };
};