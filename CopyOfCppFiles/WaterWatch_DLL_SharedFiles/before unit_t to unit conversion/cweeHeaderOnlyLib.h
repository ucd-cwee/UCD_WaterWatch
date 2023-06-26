#pragma once


#include "Heap.h"
#include "Clock.h"
#include "StringView.h" // basic string_view implimentation
#include "InterlockedValues.h" // system-level atomic items 
#include "SharedPtr.h" // custom implimentation of std::shared_ptr with more thread safety. 
#include "cweeAllocator.h" 
#include "DelayedInstantiation.h" // Thread-safe object that instantiates its underlying value only on request for its existance, otherwise remains NULL. 
#include "SharedPtrWithSentinel.h" // custom implimentation of std::shared_ptr with more thread safety and support for sentineled memory protection (Shared memory responsibility).
#include "Mutex.h" // Explicit thread-lock that puts threads to sleep rather than allowing them to spin and use CPU clocks. 
#include "cweeInterlocked.h"  // generic values with a lock attached to them.
#include "Strings.h" // basic string with in-class features and functions.
#include "cweeTime.h" // best available clock tool for tracking and sharing date-time explicitely.
#include "Iterator.h" // generic #DEFINE functions to allow lists, maps, etc. to support for-loops, std::begin(), etc. 
#include "List.h" // basic list or vector. Faster for sorted insert/delete, slower for random insert/delete.
#include "LinkedList.h" // list of lists, with the exact same interface as the basic list. Faster for random insert/delete, slower for sorted insert/delete.
#include "Toasts.h" // queue for "toasts" or messages from anywhere in the app. Acts as a message repo. 
#include "cwee_math.h" // basic math features
#include "Parser.h" // parse strings into components. 
#include "ZipLib.h" // support zipping and unzipping files.
#include "bitFlags.h" // small boolean flags in a collection
#include "Serialization.h" // try-serialize without guarranteeing presence of serialize functions
#include "vec.h" // vec2, vec3, vec4, vec5, vec6... etc.
#include "VecX.h" // generic-length vector of floats. 
#include "MatX.h" // generic-size grid of floats. 
#include "enum.h" // better_enums namespace for automatic to-from string conversions of enum objects. 
#include "cweeUnitedValue.h"
#include "BasicUnits.h" // basic units for engineering support
#include "File.h" // basic windows-file definitions. 
#include "Curve.h" // timeseries value container. Basic, lower-cost implimentation of Pattern
using Curve = cweeCurve_CatmullRomSpline<float>;
#include "Engineering.h" // engineering tasks and analysis tools 
#include "Geocoding.h" // automatic geocoding and elevation look-ups
#include "cweeAny.h" // thread-safe type erasure system with automatic casting and ptr sharing. /* cweeAny x = 100.0f; float& f_ref = x.cast(); cweeSharedPtr<float> f_ptr = x.cast(); */
#include "UnorderedList.h" // thread-safe list with delayed deletion of objects.
#include "cweeThreadedMap.h" // thread-safe dictionary or map. 
#include "cweeJob.h" // basic job definition and job queue support. Includes definition of parallel, async job processor. 
#include "DispatchTimer.h" // Queue a job every 'X' milliseconds.
#include "BalancedTree.h" // generic data structure that maintains a balanced B-tree.
#include "cweeSet.h"
#include "cweeScheduler.h" // automated job scheduling with iteration review to perform optimizations
#include "MachineLearningHeader.h" // basic regression tools
#include "MachineLearningTools.h" // tools to perform regressions on timeseries data with same-sized timeseries "features" as hints for future forecasts. 
#include "Pattern.h" // timeseries value container that uses a list to store data. Faster for sorted insert/delete, slower for random insert/delete.
using Pattern = cweePattern_CatmullRomSpline<float>;
#include "AlgLibWrapper.h"
#include "BalancedPattern.h" // timeseries value container that uses a tree to store data. Faster for random insert/delete, slower for sorted insert/delete.
#include "InterpolatedMatrix.h" // sparse matrix that interpolates between given values using a hilbert curve
#include "odbc.h" // nanodbc requests -> Does not support SQLite (at the moment)
#include "AppLayerRequests.h" // queue for job-requests for processing by another system
#include "chaiscript_wrapper.h" // real-time scripting, base systems 
#include "FileSystem.h"
#include "cweeSpatialAsset.h"
#include "EPAnetWrapper.h" // EPAnet-> and EPAnetProject-> tools
#include "InteropData.h"

#include "Asset.h"
#include "cweeEpanet_p1.h"
#include "cweeEpanet_p2.h"
#include "AssetControls.h"

//...

#include "Chaiscript_WaterWatch_Module.h" // intertie scripting language with WaterWatch
