/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once 

#include <iostream>
#include "../WaterWatchCpp/Precompiled.h"
#include "../WindowsPlatformTools/WindowsPlatformTools.h"
#include "../WaterWatchCpp/Clock.h"
#include "../WaterWatchCpp/Strings.h"
#include "../WaterWatchCpp/List.h"
#include "../WaterWatchCpp/SharedPtr.h"
#include "../WaterWatchCpp/AppLayerRequests.h"
#include "../WaterWatchCpp/Toasts.h"
#include "../WaterWatchCpp/FileSystemH.h"
#include "../WaterWatchCpp/Geocoding.h"
#include "../WaterWatchCpp/MachineLearning.h"
#include "../WaterWatchCpp/cweeJob.h"
#include "../WaterWatchCpp/Chaiscript_WaterWatch_Module.h"
#include "../WaterWatchCpp/ExternData.h"
#include "../WaterWatchCpp/DispatchTimer.h"
#include "../WaterWatchCpp/EPAnetWrapper.h" 
#include <conio.h> // Enable real-time keystroke collection without pausing console. NOTE: THIS IS NOT UWP APP SAFE, DO NOT INCLUDE ELSEWHERE IN DLL OR LIB.