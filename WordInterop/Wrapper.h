#pragma once
#include "../WaterWatchCpp/Precompiled.h"
#include "../WaterWatchCpp/Units.h"
#include "../WaterWatchCpp/Strings.h"
#include "../WaterWatchCpp/SharedPtr.h"
#include "../WaterWatchCpp/cweeTime.h"
#include "../WaterWatchCpp/List.h"
#include "../WaterWatchCpp/enum.h"
#include "../WaterWatchCpp/Iterator.h"
#include "../WaterWatchCpp/chaiscript_wrapper.h"
#include "../WaterWatchCpp/WaterWatch_Module_Header.h"






class WordInteropWrapper {
public:
	static cweeStr test(cweeStr const& filePath);
};

namespace chaiscript {
    namespace WaterWatch_Lib {
        [[nodiscard]] ModulePtr MSWord_library();
    };
}; // namespace chaiscript