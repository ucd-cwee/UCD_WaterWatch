#pragma once

#include "foo.h"
#include "../WindowsPlatformTools/WindowsPlatformTools.h"
#include "../cNominitum/cNominitum.h"

int Foo::Static() {
    return WindowsPlatform::GetCPUInfo().logicalProcessorCount;

    // return WindowsPlatform::cweeCpuInfo_t().logicalProcessorCount;
    //return 1000000.0f;
};

std::string Foo::Geocode() {
    return "TEST";
};