/*
=======================================================================
Energy Demand Management System for Water Distribution Systems
Developed by the Center for Water-Energy Efficiency, 2019 - 2020
=======================================================================
*/

#ifndef __PRECOMPILED_H__
#define __PRECOMPILED_H__

// #define debugUwpInstallation


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

#include <malloc.h>							
#include <windows.h>		

#undef FindText								

// #define _DEBUG 0
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
#include <ctime>
#include <ctype.h>
#include <typeinfo>
#include <errno.h>
#include <math.h>
#include <stdexcept>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <ctime>
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
#include <cmath>
#include <memory>
#include <string>
#include <cstdint>
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
#include <cassert>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <array>
#include <cstddef>
#include <unordered_set>
#include <cctype>
#include <cmath>
#include <initializer_list>
#include <iostream>
#include <ostream>
#include <variant>
#include <cctype>
#include <cstring>
#include <exception>
#include <sstream>
#include <atomic>
#include <limits>

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
// Win32 desktop app
#include <atlbase.h>
#include <atlstr.h>
#endif

#include <windows.h>

#include <urlmon.h>
#include <wincred.h>
#include <tchar.h>
#include <io.h>
#pragma comment(lib, "urlmon.lib")

// non-portable system services
#include "../cweeLib/sys_assert.h"
#include "../cweeLib/sys_threading.h"
#include "../cweeLib/sys_public.h"

// cwee lib
#include "../cweeLib/Lib.h"

#endif

