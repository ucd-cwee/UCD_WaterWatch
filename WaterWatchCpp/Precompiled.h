#pragma once

#pragma region Compile-Time Unit Management
#include "Units.h"
namespace cwee_units {
	using namespace units;
	using namespace units::length;
	using namespace units::time;
	using namespace units::area;
	using namespace units::velocity;
	using namespace units::volume;
	using namespace units::flowrate;
	using namespace units::temperature;
	using namespace units::pressure;
	using namespace units::power;
	using namespace units::energy;
	using namespace units::fillrate;
	using namespace units::dimensionless;
	using namespace units::dollar;
	using namespace units::energy_cost_rate;
	using namespace units::power_cost_rate;
	using namespace units::volume_cost_rate_unit;	
	using namespace units::energy_intensity_unit;
	using namespace units::length_cost_rate_unit;
	using namespace units::mass_cost_rate_unit;
	using namespace units::emission_rate_unit;
	using namespace units::time_rate_unit;

	using namespace units::literals;

	//constexpr extern auto example_val = 1_ft;
}
#pragma endregion
#pragma region Precompiled STL Headers
#include <winsock2.h>
#include <mmsystem.h>
#include <mmreg.h>
#define DIRECTINPUT_VERSION  0x0800	
#define DIRECTSOUND_VERSION  0x0800
#define GRANULARITY_SCALER 2.0f
#include <dsound.h>
#include <dinput.h>
#pragma warning(disable : 4005)				// macro redefinition
#pragma warning(disable : 4010)				// single-line comment contains line-continuation character
#pragma warning(disable : 4018)				// singed / unsigned mismatch
#pragma warning(disable : 4100)				// unreferenced formal parameter
#pragma warning(disable : 4101)				// unreferenced local variable
#pragma warning(disable : 4127)				// conditional expression is constant
#pragma warning(disable : 4172)				// returning address of local variable or temporary
#pragma warning(disable : 4189)				// local variable is initialized but not referenced
#pragma warning(disable : 4238)				// nonstandard extension used: class rvalue used as lvalue
#pragma warning(disable : 4239)				// conversion from 'T' to 'T&'
#pragma warning(disable : 4244)				// conversion to smaller type, possible loss of data
#pragma warning(disable : 4251)				// needs to have dll-interface
#pragma warning(disable : 4267)				// conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable : 4273)				// inconsistent DLL linkage
#pragma warning(disable : 4297)				// function assumed not to throw but does
#pragma warning(disable : 4302)				// truncation from 'void *' to 'int'
#pragma warning(disable : 4305)				// truncating a literal from double to float
#pragma warning(disable : 4311)				// pointer truncation from 'void *' to 'int'
#pragma warning(disable : 4312)				// conversion from 'int' to 'void*' of greater size
#pragma warning(disable : 4390)				// ';' empty controlled statement
#pragma warning(disable : 4456)				// declaration hides previous local declaration
#pragma warning(disable : 4458)				// hides class member
#pragma warning(disable : 4459)				// hides global declaration
#pragma warning(disable : 4499)				// 'static': an explicit specialization cannot have a storage class
#pragma warning(disable : 4505)				// unreferenced local function has been removed
#pragma warning(disable : 4595)				// non-member operator new or delete functions may not be declared inline
#pragma warning(disable : 4701)				// potentially uninitialized local variable
#pragma warning(disable : 4714)				// function marked as __forceinline not inlined
#pragma warning(disable : 4715)				// not all control paths return a value
#pragma warning(disable : 4996)				// unsafe string operations
#pragma warning(disable : 6011)				// Dereferencing NULL ptr
#pragma warning(disable : 6385)				// Reading invalid data from buf
#pragma warning(disable : 26110)			// Caller failing to hold lock
#pragma warning(disable : 26439)			// This kind of function may not throw
#pragma warning(disable : 26450)			// Arithmetic overflow: using '<<'
#pragma warning(disable : 26451)			// Arithmetic overflow: using '*' on a 4 byte variable and casting to 8 bytes
#pragma warning(disable : 26495)			// uninitialized member variable type 6
#pragma warning(disable : 26498)			// Mark function constexpr if compile-time evaluation is desired
#pragma warning(disable : 26812)			// prefer enum class to enum
#pragma warning(disable : 28182)			// Dereferencing NULL pointer
#pragma warning(disable : 28251)			// Inconsistent annotation for 'new'
#define _CRT_FUNCTIONS_REQUIRED 1
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#include <malloc.h>							
#include <windows.h>		

#undef FindText								

#ifndef NDEBUG
#define NDEBUG
#endif
#define INLINE	__forceinline
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <ctype.h>
#include <typeinfo>
#include <errno.h>
#include <math.h>
#include <stdexcept>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>
//#include <ctime>
#include <chrono>
#include <random>
#include <map>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <ShlDisp.h>
#include <Urlmon.h>
#include <sys/stat.h>
#include <limits.h>
#include <initializer_list>
#include <string_view>
#include <vector>
//#include <cmath>
#include <memory>
#include <string>
// #include <cstdint>
#include <sstream>
#include <typeinfo>	
#include <type_traits>
#include <map>
#include <algorithm>
#include <atomic>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <algorithm>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeinfo>
#include <utility>
#include <vector>
//#include <cassert>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <array>
//#include <cstddef>
#include <unordered_set>
//#include <cctype>
//#include <cmath>
#include <initializer_list>
#include <iostream>
#include <ostream>
#include <variant>
//#include <cstring>
#include <exception>
#include <sstream>
#include <atomic>
#include <limits>

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
//#include <atlbase.h>
//#include <atlstr.h>
#endif

#include <windows.h>
#include <urlmon.h>
#include <wincred.h>
#include <tchar.h>
#include <io.h>
#pragma comment(lib, "urlmon.lib")

#pragma endregion
#pragma region Non-Portable Services
// TO-DO, switch statement based on the target platforms
#pragma region WINDOWS
#include "Windows_SysAssert.h"
#include "Windows_SysThreading.h"
#pragma endregion
#pragma endregion

#include "BasicDefines.h" // memory services and fundamental templated types
#include "Heap.h" // memory services and fundamental templated types