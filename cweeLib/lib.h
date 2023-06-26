/*
=======================================================================
Energy Demand Management System for Water Distribution Systems
Developed by the Center for Water-Energy Efficiency, 2019 - 2020
=======================================================================
*/

#ifndef __LIB_H__
#define __LIB_H__

//#define		FORCE_MEM_TRACKING	// Note, do not activate. Slows down the program by 10000%, as the current mechanism saves to a file directly. 

class cweeLib {
public:
	static void* edms_epanet;
	static void* edms_rtdb;
	static class				cweeSys* sys;
	static void					Init(void);
	static void					ShutDown(void);

	static std::string															Mem_DefineTag(int tag);
	static std::vector< std::string >											Mem_GetTagsUsed();
	static unsigned long long													Mem_GetNumAllocations();
	static unsigned long long													Mem_GetNumAllocations(int tag);
	static unsigned long long													Mem_GetNumActiveAllocations();
	static unsigned long long													Mem_GetSumAllocations();
	static std::vector<std::tuple<int, int, uintptr_t, uintptr_t, bool>>		Mem_GetHistory();
};


/*
===============================================================================
	Types and defines used throughout the engine.
===============================================================================
*/

#define AUTO							decltype(auto)
#define INLINE							__forceinline
#define INLINE_EXTERN					extern inline

typedef unsigned char			byte;		// 8 bits // compare to Microsoft existing Byte. 
typedef unsigned short			word;		// 16 bits
typedef unsigned int			dword;		// 32 bits
typedef unsigned int			uint;
typedef unsigned long			ulong;
typedef int						handle;
typedef signed char				int8;
typedef unsigned char			uint8;
typedef short int				int16;
typedef unsigned short int		uint16;
typedef int						int32;
typedef unsigned int			uint32;
typedef long long				int64;
typedef unsigned long long		uint64;
typedef long double				u64;

#define INT8_SIGN_BIT		7
#define INT16_SIGN_BIT		15
#define INT32_SIGN_BIT		31
#define INT64_SIGN_BIT		63

#define INT8_SIGN_MASK		( 1 << INT8_SIGN_BIT )
#define INT16_SIGN_MASK		( 1 << INT16_SIGN_BIT )
#define INT32_SIGN_MASK		( 1UL << INT32_SIGN_BIT )
#define INT64_SIGN_MASK		( 1ULL << INT64_SIGN_BIT )


#define CONST_MAX( x, y )			( (x) > (y) ? (x) : (y) )

class vec3;

#ifndef NULL
#define NULL					((void *)0)
#endif

#ifndef BIT
#define BIT( num )				( 1 << ( num ) )
#endif

#define	MAX_STRING_CHARS		1024		// max length of a string
#define MAX_PRINT_MSG			16384		// buffer size for our various printf routines

// little/big endian conversion
short	BigShort(short l);
short	LittleShort(short l);
int		BigLong(int l);
int		LittleLong(int l);
float	BigFloat(float l);
float	LittleFloat(float l);
void	BigRevBytes(void* bp, int elsize, int elcount);
void	LittleRevBytes(void* bp, int elsize, int elcount);
void	LittleBitField(void* bp, int elsize);
void	Swap_Init(void);
bool	Swap_IsBigEndian(void);

// for base64
void	SixtetsForInt(::byte* out, int src);
int		IntForSixtets(::byte* in);

// move from Math.h to keep gcc happy
template<class T> INLINE T	Max(T x, T y) { return (x > y) ? x : y; }
template<class T> INLINE T	Min(T x, T y) { return (x < y) ? x : y; }

/*
================
Sys_GetSystemRam
	returns amount of physical memory in MB
================
*/
int Sys_GetSystemRam();
INLINE int Sys_GetSystemRam() {
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	int physRam = statex.ullTotalPhys / (1024 * 1024);
	physRam = (physRam + 8) & ~15; // HACK: For some reason, ullTotalPhys is sometimes off by a meg or two, so we round up to the nearest 16 megs
	return physRam;
}

// CPU usage
#define BYTES_IN_A_MB 1048576
static float CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks);
static unsigned long long FileTimeToInt64(const FILETIME& ft);
float GetCPULoad(void);
static unsigned long long _previousTotalTicks = 0;
static unsigned long long _previousIdleTicks = 0;
INLINE static float CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks)
{
	//static unsigned long long _previousTotalTicks = 0;
	//static unsigned long long _previousIdleTicks = 0;

	unsigned long long totalTicksSinceLastTime = totalTicks - _previousTotalTicks;
	unsigned long long idleTicksSinceLastTime = idleTicks - _previousIdleTicks;

	float ret = 1.0f - ((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);

	_previousTotalTicks = totalTicks;
	_previousIdleTicks = idleTicks;
	return ret;
}
INLINE static unsigned long long FileTimeToInt64(const FILETIME& ft)
{
	return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime);
}
// Returns 1.0f for "CPU fully pinned", 0.0f for "CPU idle", or somewhere in between
// You'll need to call this at regular intervals, since it measures the load between
// the previous call and the current one.  Returns -1.0 on error.
INLINE float GetCPULoad(void) {
	FILETIME idleTime, kernelTime, userTime;
	return GetSystemTimes(&idleTime, &kernelTime, &userTime) ? CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime)) : -1.0f;
}

// Support for enum class casting to int. (C++ can't have a template based on the return type - so there isn't a version to cast from int to enum class)
template <typename E>
constexpr typename std::underlying_type<E>::type enumClassCast(E e) noexcept {
	return static_cast<typename std::underlying_type<E>::type>(e);
}
template <typename E>
constexpr int enumClassToInt(E e) noexcept {
	return (int)enumClassCast(e);
}

// Shared system-wide mutex 
#include <mutex>
static std::mutex* cweeStackTraceLock = nullptr;

#if 1
namespace typenames {
#include <string_view>
	// If you can't use C++17's standard library, you'll need to use the GSL 
	// string_view or implement your own struct (which would not be very difficult,
	// since we only need a few methods here)

	template <typename T> constexpr const char* type_name();
	template <typename T> constexpr std::string_view sv_type_name();

	template <> constexpr const char* type_name<void>()
	{
		return "void";
	}

	template <> constexpr std::string_view sv_type_name<void>()
	{
		return "void";
	}

	namespace detail {

		using type_name_prober = void;

		template <typename T>
		constexpr std::string_view wrapped_type_name()
		{
#ifdef __clang__
			return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
			return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
			return __FUNCSIG__;
#else
#error "Unsupported compiler"
#endif
		}

		constexpr std::size_t wrapped_type_name_prefix_length() {
			return wrapped_type_name<type_name_prober>().find(sv_type_name<type_name_prober>());
		}

		constexpr std::size_t wrapped_type_name_suffix_length() {
			return wrapped_type_name<type_name_prober>().length()
				- wrapped_type_name_prefix_length()
				- sv_type_name<type_name_prober>().length();
		}

	} // namespace detail

	template <typename T>
	constexpr std::string_view sv_type_name() {
		constexpr std::string_view wrapped_name = typenames::detail::wrapped_type_name<T>();
		constexpr auto prefix_length = typenames::detail::wrapped_type_name_prefix_length();
		constexpr auto suffix_length = typenames::detail::wrapped_type_name_suffix_length();
		constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
		return wrapped_name.substr(prefix_length, type_name_length);
	}

	template <typename T>
	constexpr const char* type_name() {
		constexpr std::string_view wrapped_name = typenames::detail::wrapped_type_name<T>();
		constexpr auto prefix_length = typenames::detail::wrapped_type_name_prefix_length();
		constexpr auto suffix_length = typenames::detail::wrapped_type_name_suffix_length();
		constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
		return wrapped_name.substr(prefix_length, type_name_length).data();
	}
}
#endif

/*
===============================================================================
	cweeLib headers.
===============================================================================
*/
// memory management and strings
#include "Clock.h"
#include "Heap.h"
#include "Sort.h"
#include "StringView.h"
#include "Multithreading.h"
#include "strings.h" // swapped location with after List.h to before List.h
#include "List.h"
// #include "strings.h" 

// math 
#include "Simd.h"

// stack tracer
#pragma region StackTracer
#include "StackWalker.h"
class cweeStackTrace : public StackWalker
{
public:
	cweeStackTrace() : StackWalker() {}
	cweeStr content;
	static cweeStr GetTrace(bool outputToVisualStudio = true) {
		if (!cweeStackTraceLock) cweeStackTraceLock = new std::mutex();
		std::lock_guard<std::mutex> guard(*cweeStackTraceLock);

		cweeStackTrace sw;
		sw.ShowCallstack();
		if (outputToVisualStudio) {
			try {
				OutputDebugStringA(sw.content.c_str());
			}
			catch (...) {
				return sw.content;
			}
		}

		return sw.content;
	};
protected:
	virtual void OnOutput(LPCSTR szText) {	
		cweeStr line = szText;
		line.ReduceSpaces();	
		content.AddToDelimiter(line.c_str(), "\n");
		// StackWalker::OnOutput(szText);
	}
};
#pragma endregion


#include "boost/date_time.hpp"
class cweeTime
{
public:
	static cweeTime timeFromString(const cweeStr& timeStr = "1970/1/1 0:0:0") {
		return cweeTime(boost::posix_time::time_from_string(timeStr.c_str()));
	};

	cweeTime() {};
	cweeTime(const cweeTime& t) : time(t.time) {};
	cweeTime(const boost::posix_time::ptime& t) : time(t) {};
	cweeTime(const u64& t) {
		time = boost::posix_time::time_from_string("1970/1/1 0:0:0");
		if (t > 0) {
			time += boost::posix_time::seconds((long long)t);
		}
		else {
			time -= boost::posix_time::seconds((long long)(-t));
		}
	};
	cweeTime(const char* t) {
		this->FromString(t);
	};

	operator const char*() const {
		return ToString();
	};
	explicit operator u64() const {
		u64 out = 0;
		boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970/1/1 0:0:0");

		if (time >= epoch) {
			out = (time - epoch).total_milliseconds() / 1000.0;
		}
		else {
			out = -1.0 * ((epoch - time).total_milliseconds() / 1000.0);
		}

		return out;
	};
	cweeTime& operator=(const cweeTime& t) {
		time = t.time;
		return *this;
	};
	bool	operator==(const cweeTime& t) const {
		return (time == t.time);
	};
	bool	operator!=(const cweeTime& t) const {
		return (time != t.time);
	};
	bool	operator>(const cweeTime& t) const {
		return (time > t.time);
	};
	bool	operator<(const cweeTime& t) const {
		return (time < t.time);
	};
	bool	operator>=(const cweeTime& t) const {
		return (time >= t.time);
	};
	bool	operator<=(const cweeTime& t) const {
		return (time <= t.time);
	};

	bool	operator==(const double t) const {
		return ((u64)*this == t);
	};
	bool	operator!=(const double t) const {
		return ((u64)*this != t);
	};
	bool	operator>(const double t) const {
		return ((u64)*this > t);
	};
	bool	operator<(const double t) const {
		return ((u64)*this < t);
	};
	bool	operator>=(const double t) const {
		return ((u64)*this >= t);
	};
	bool	operator<=(const double t) const {
		return ((u64)*this <= t);
	};

	cweeTime& operator+=(const cweeTime& seconds) {
		return this->operator+=((u64)seconds);
	};
	cweeTime& operator-=(const cweeTime& seconds) {
		return this->operator-=((u64)seconds);
	};
	cweeTime& operator*=(const cweeTime& seconds) {
		return this->operator*=((u64)seconds);
	};
	cweeTime& operator/=(const cweeTime& seconds) {
		return this->operator/=((u64)seconds);
	};

	cweeTime& operator+=(const double seconds) {
		u64 t = seconds + (u64)*this;

		time = boost::posix_time::time_from_string("1970/1/1 0:0:0");
		if (t > 0) {
			time += boost::posix_time::seconds((long long)t);
		}
		else {
			time -= boost::posix_time::seconds((long long)(-t));
		}

		return *this;
	};
	cweeTime& operator-=(const double seconds) {
		u64 t = (u64)*this - seconds;

		time = boost::posix_time::time_from_string("1970/1/1 0:0:0");
		if (t > 0) {
			time += boost::posix_time::seconds((long long)t);
		}
		else {
			time -= boost::posix_time::seconds((long long)(-t));
		}

		return *this;
	};
	cweeTime& operator*=(const double seconds) {
		u64 t = seconds * (u64)*this;

		time = boost::posix_time::time_from_string("1970/1/1 0:0:0");
		if (t > 0) {
			time += boost::posix_time::seconds((long long)t);
		}
		else {
			time -= boost::posix_time::seconds((long long)(-t));
		}

		return *this;
	};
	cweeTime& operator/=(const double seconds) {
		u64 t = ((u64)*this) / seconds;

		time = boost::posix_time::time_from_string("1970/1/1 0:0:0");
		if (t > 0) {
			time += boost::posix_time::seconds((long long)t);
		}
		else {
			time -= boost::posix_time::seconds((long long)(-t));
		}

		return *this;
	};

	friend cweeTime operator+(const cweeTime& a, const cweeTime& b) {			
		cweeTime out(a);
		out += b;
		return out;
	};
	friend cweeTime operator+(const cweeTime& a, const double b) {
		cweeTime out(a);
		out += b;
		return out;
	};
	friend cweeTime operator+(const double a, const cweeTime& b) {
		cweeTime out(a);
		out += b;
		return out;
	};

	friend cweeTime operator-(const cweeTime& a, const cweeTime& b) {
		cweeTime out(a);
		out += b;
		return out;
	};
	friend cweeTime operator-(const cweeTime& a, const double b) {
		cweeTime out(a);
		out += b;
		return out;
	};
	friend cweeTime operator-(const double a, const cweeTime& b) {
		cweeTime out(a);
		out += b;
		return out;
	};

	friend cweeTime operator*(const cweeTime& a, const cweeTime& b) {
		cweeTime out(a);
		out *= b;
		return out;
	};
	friend cweeTime operator*(const cweeTime& a, const double b) {
		cweeTime out(a);
		out *= b;
		return out;
	};
	friend cweeTime operator*(const double a, const cweeTime& b) {
		cweeTime out(a);
		out *= b;
		return out;
	};

	friend cweeTime operator/(const cweeTime& a, const cweeTime& b) {
		cweeTime out(a);
		out /= b;
		return out;
	};
	friend cweeTime operator/(const cweeTime& a, const double b) {
		cweeTime out(a);
		out /= b;
		return out;
	};
	friend cweeTime operator/(const double a, const cweeTime& b) {
		cweeTime out(a);
		out /= b;
		return out;
	};

	cweeTime& ToStartOfMonth() {
		this->operator-=(tm_sec());
		this->operator-=(tm_min() * 60);
		this->operator-=(tm_hour() * 3600);
		this->operator-=((tm_mday() - 1) * 3600 * 24);
		return *this;
	};
	cweeTime& ToStartOfDay() {
		this->operator-=(tm_sec());
		this->operator-=(tm_min() * 60);
		this->operator-=(tm_hour() * 3600);
		return *this;
	};
	cweeTime& ToStartOfHour() {
		this->operator-=(tm_sec());
		this->operator-=(tm_min() * 60);
		return *this;
	};
	cweeTime& ToStartOfMinute() {
		this->operator-=(tm_sec());
		return *this;
	};

	boost::posix_time::ptime	time;

	// seconds after the minute - [0, 60] including leap second
	int tm_sec() {
		return time.time_of_day().seconds();
	};

	// minutes after the hour - [0, 59]
	int tm_min() {
		return time.time_of_day().minutes();
	};

	// hours since midnight - [0, 23]
	int tm_hour() {
		return time.time_of_day().hours();
	};

	// day of the month - [1, 31]
	int tm_mday() {
		return time.date().year_month_day().day;
	};

	// months since January - [0, 11]
	int tm_mon() {
		return time.date().year_month_day().month - 1;
	};

	// years since 1900
	int tm_year() {
		return time.date().year_month_day().year - 1900;
	};

	// days since Sunday - [0, 6]
	int tm_wday() {
		return time.date().day_of_week().as_number();
	};

	// days since January 1 - [0, 365]
	int tm_yday() {
		return time.date().day_of_year() - 1;
	};

private:
	cweeStr	ToString() const {
		return cweeStr((u64)*this);
	};
	void	FromString(const cweeStr& a) {
		*this = (u64)a;
	};

};

// arrays, serialization, custom containers
#include "Iterator.h"
#include "ThreadedList.h"

// Machine Learning Libraries
#include "../dlib-19.17/source/dlib/bayes_utils.h"
#include "../dlib-19.17/source/dlib/graph_utils.h"
#include "../dlib-19.17/source/dlib/graph.h"
#include "../dlib-19.17/source/dlib/directed_graph.h"
#include "../dlib-19.17/source/dlib/svm.h"
#include "../dlib-19.17/source/dlib/mlp.h"

template< typename _type_, memTag_t _tag_ >
cweeStr ListToString(cweeThreadedList<_type_, _tag_>& list, const cweeStr& delim = ",") {
	cweeStr out;	
	for (auto& x : list) {
		out.AddToDelimiter(cweeStr(x), delim);
	}
	return out;
}

#include "math.h"
#include "StaticList.h"
#include "Parser.h"
#include "bitFlags.h"
#include "Serialization.h"
#include "vector.h"
#include "VecX.h"
#include "MatX.h"
#include "enum.h"
#include "BasicUnits.h"

#include "ZipLib.h"			// zip or unzip files easily. NOTE: Cannot stack zip files using this methodology. (I.e. zipping zip files)
#include "SQLite.h"			// create or query or modify local .db files. 
#include "File.h"			// containers for memory files. 
#include "Geocoding.h"		// geocoding addresses

// Ordered 2-D containers.
#include "LinkedList.h"
#include "LinkedList.cpp"
#include "Curve.h"		// X float - Y <template type> plots
typedef cweeCurve_CatmullRomSpline<float>		Curve;

int	 Sys_Milliseconds(); INLINE int Sys_Milliseconds() { return clock_ms(); }
uint64	Sys_Microseconds(void); INLINE uint64 Sys_Microseconds(void) { return clock_us(); }

// containers
#include "BTree.h"

// Multithreading
void submitToast(cweeStr title, cweeStr content);
#include "objManager.h"
#include "interlockedGeneric.h"
#include "cweeAny.h"
#include "UnorderedList.h"	
#include "cweeThreadedMap.h"

static cweeUnorderedList< std::pair<cweeStr, cweeStr> > Toasts;
INLINE void submitToast(cweeStr title, cweeStr content) {
	std::pair<cweeStr, cweeStr> toast;	toast.first = title;	toast.second = content;
	Toasts.Lock();
	auto ptr = Toasts.UnsafeRead(Toasts.UnsafeAppend());
	if (ptr) *ptr = toast;
	Toasts.Unlock();
};

// Machine Learning Tools
#include "cweeMachineLearn.h"
#include "Machine_Learning.h"
#include "Pattern.h"		// X time_t - Y <template type> plots
#include "BalancedTree.h"
#include "BalancedTree.cpp"
#include "BalancedPattern.h"
#include "InterpolatedMatrix.h"

typedef cweePattern_CatmullRomSpline<float>		Pattern;

#include "nanodbc.h"		// support for ODBC connections over the internet or locally. 

// water customer. Placed here to allow for global dataset. 
class customer {
public: // data containers
	int												accountNumber = -1;
	cweeStr											serviceType;
	bool											isActive = false;

	cweeStr											customerName;
	cweeStr											customerType;
	cweeStr											serviceAddress;
	vec2											serviceLongLat;

	cweeStr											rateCode;
	cweeStr											meterSize;

	Pattern											waterUsage;
public: // data saving and retrieval
	cweeStr											Serialize(int option = -1) {
		cweeStr delim = ":Customers_DELIM:";
		cweeStr out;

		out.AddToDelimiter(accountNumber, delim);
		out.AddToDelimiter(serviceType, delim);
		out.AddToDelimiter((int)isActive, delim);
		out.AddToDelimiter(customerName, delim);
		out.AddToDelimiter(customerType, delim);
		out.AddToDelimiter(serviceAddress, delim);
		out.AddToDelimiter(serviceLongLat.x, delim);
		out.AddToDelimiter(serviceLongLat.y, delim);
		out.AddToDelimiter(rateCode, delim);
		out.AddToDelimiter(waterUsage.Serialize(), delim);
		out.AddToDelimiter(rateCode, meterSize);
		return out;
	}
	void											Deserialize(cweeStr& in) {
		cweeStr delim = ":Customers_DELIM:";
		cweeParser obj(in, delim, true);

		accountNumber = (int)obj[0];
		serviceType = obj[1];
		isActive = (bool)(int)obj[2];
		customerName = obj[3];
		customerType = obj[4];
		serviceAddress = obj[5];
		serviceLongLat.x = (float)obj[6];
		serviceLongLat.y = (float)obj[7];
		rateCode = obj[8];
		waterUsage.Deserialize(obj[9]);
		meterSize = obj[10];
	}
}; 

#include "ConcurrentJobs.h"
#include "ParallelThreads.h"

// #define CHAISCRIPT_NO_THREADS
#define CHAISCRIPT_ALLOW_NAME_CONFLICTS
namespace chaiscript {
	class ChaiScript;
}
/*! Scripting layer */
static chaiscript::ChaiScript& GetChai();

#pragma region CHAISCRIPT_INCLUDES
#include "utility/quick_flat_map.hpp"
#include "chaiscript_defines.hpp"
#include "utility/static_string.hpp"
#include "utility/hash.hpp"
#include "language/chaiscript_algebraic.hpp"
#include "dispatchkit/any.hpp"
#include "dispatchkit/bad_boxed_cast.hpp"	
#include "dispatchkit/type_info.hpp"
#include "dispatchkit/boxed_value.hpp"
#include "dispatchkit/boxed_cast_helper.hpp"
#include "chaiscript_threading.hpp"
#include "dispatchkit/type_conversions.hpp"
#include "dispatchkit/boxed_cast.hpp"
#include "dispatchkit/boxed_number.hpp"	
#include "dispatchkit/dynamic_object.hpp"
#include "dispatchkit/function_params.hpp"
#include "dispatchkit/handle_return.hpp"
#include "dispatchkit/proxy_functions_detail.hpp"
#include "dispatchkit/proxy_functions.hpp"
#include "dispatchkit/proxy_constructors.hpp"
#include "dispatchkit/short_alloc.hpp"
#include "dispatchkit/dispatchkit.hpp"
#include "language/chaiscript_common.hpp"
#include "dispatchkit/function_call_detail.hpp"
#include "dispatchkit/function_call.hpp"
#include "dispatchkit/bind_first.hpp"
#include "dispatchkit/function_signature.hpp"
#include "dispatchkit/register_function.hpp"
#include "dispatchkit/operators.hpp"
#include "utility/utility.hpp"
#include "dispatchkit/bootstrap.hpp"
#include "dispatchkit/bootstrap_stl.hpp"
#include "language/chaiscript_prelude.hpp"
#include "utility/json.hpp"	
#include "utility/json_wrap.hpp"
#ifndef CHAISCRIPT_NO_THREADS
#include <future>
#endif
#include "language/chaiscript_engine.hpp"
#include "chaiscript_stdlib.hpp"
#include "../cweeLib/dispatchkit/dynamic_object_detail.hpp"
#include "../cweeLib/language/chaiscript_eval.hpp"
#include "../cweeLib/language/chaiscript_optimizer.hpp"
#include "../cweeLib/language/chaiscript_tracer.hpp"
#if defined(CHAISCRIPT_UTF16_UTF32)
#include <codecvt>
#include <locale>
#endif
#include "../cweeLib/language/chaiscript_parser.hpp"
#pragma endregion
#include "chaiscript.hpp"
#include "ChaiScriptManager.h"	// Real-Time Scripting Tools
#include "ChaiScriptHelperClasses.h" // helper classes

class appLayerRequests {
public:
	using arguments_type = typename cweeThreadedList<cweeStr>;
	using method_type = typename cweeStr;
	using container_type = typename cweeUnion<cweeStr, cweeThreadedList<cweeStr>, cweeSharedPtr<cweeStr>>;

private:
	cweeThreadedMap< int, container_type >									requests;
	cweeThreadedMap< int, cweeUnion<cweeJob, cweeSharedPtr<cweeStr>> >		jobs;
	cweeSysInterlockedInteger												jobNums;

public:
	/* for the backend layer to insert to, with a job to await */
	std::pair<int, cweeJob> AddRequest(method_type const& methodName, arguments_type const& arguments = arguments_type()) {
		int n = jobNums.Increment();
		AUTO result = container_type(methodName, arguments, cweeSharedPtr<cweeStr>(cweeStr("")));
		requests.Emplace(n, result);

		cweeJob out([this](int num, cweeStr& get) -> cweeStr { return get; }, n, result.get<2>());

		jobs.Emplace(n, cweeUnion<cweeJob, cweeSharedPtr<cweeStr>>(out, result.get<2>()));

		return std::pair<int, cweeJob>(n, out);
	};
	/* for the app layer to dequeue from */
	std::pair<int, container_type> DequeueRequest() {
		std::pair<int, container_type> out = std::pair<int, container_type>(-1, container_type());		
		if (1) {
			AUTO g = requests.Guard();
			AUTO pair = requests.unsafe_pair_at_index(0);
			if (pair.first >= 0) {
				out.first = pair.first;
				out.second = *pair.second;
				requests.UnsafeErase(pair.first);
			}
		}
		return out;
	};
	/* for the app layer to check that there's work to be done */
	int	Num() {
		return requests.Num();
	};
	// for the app layer to respond to
	void FinishRequest(int index, cweeStr const& reply) {
		//AUTO g = jobs.Guard();
		//if (g)
		{
			auto ptr = jobs.at(index);
			if (ptr) {
				{
					ptr->get<1>().Lock();
					ptr->get<1>().UnsafeGet()->operator=(reply);
					ptr->get<1>().Unlock();
				}
				{
					ptr->get<0>().AsyncInvoke();
				}
				{
					jobs.Erase(index);
				}
			}
		}
	};
	// for the backend layer to hard - wait for the app layer to perform the requested job
	cweeStr Query(method_type const& methodName, arguments_type const& arguments = arguments_type()) {
		cweeStr out;

		AUTO req = AddRequest(methodName, arguments);
		cweeAny reply = req.second.Await();
		if (reply) {
			out = reply.cast<cweeStr>();
		}

		return out;
	};
};
static appLayerRequests AppLayerRequests;

#include "FileSystem.h"		// general fileSystem interface.

// engineering
#include "engineering.h"
#include "cweeSpatialAsset.h"

static cweeSharedPtr < cweeUnorderedList < cweeThreadedSpatialAsset > >	Scada = nullptr;			// global scope Scada patternsList object. Purpose is to assist the cweeControls. 
static cweeSharedPtr < cweeUnorderedList < Pattern > >					Globals = nullptr;			// global scope Globals patternsList object. Purpose is to assist the cweeControls.
static cweeSharedPtr < cweeUnorderedList < cweeThreadedSpatialAsset > >	Customers = nullptr;		// global scope Customers object. Purpose is to assist the cweeControls.

#include "Pattern.cpp"				// X time_t - Y <template type> plots
#include "BalancedPattern.cpp"		// X time_t - Y <template type> plots

#endif