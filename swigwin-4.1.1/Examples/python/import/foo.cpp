#pragma once

#include "foo.h"
#include "../StaticLib/StaticLib.h"

int Foo::Static() {
    return StaticFoo();
};