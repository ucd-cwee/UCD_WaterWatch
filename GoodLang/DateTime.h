#pragma once

#include "Units_Base.h"
#include "Units_Instances.h"

namespace GoodLang {
	// Thread- and Fiber-safe wrapper for Units::day which supports fundamental DateTime and Duration math. Utilizes Boost for the timezone math, AM/PM extractions, string conversions, etc.
	// Meant to be used in parallel with Units for proper unit management while manipulating DateTime ranges.
	// E.g: using namespace literals; return DateTime::make_time(1940, 1, 1) + 1_d - 30_s;
	class DateTime {
	public:
		Units::day time; // stored as a thread-safe double

	public:
		DateTime();
		DateTime(int year, int month, int day, int hour = 0, int minute = 0, float second = 0, bool useLocalTime = true);
		DateTime(Units::second const& a) : time{ a } {};
		DateTime(Units::second&& a) : time{ a } {};
		DateTime(const DateTime& other) = default;
		DateTime(DateTime&& other) = default;
		DateTime& operator=(const DateTime& other) = default;
		DateTime& operator=(DateTime&& other) = default;
		DateTime& operator=(Units::second const& other) { time = other; return *this; };
		DateTime(std::string const& t);
		~DateTime() = default;

	public:
		Units::day& Add(const Units::day& v); // atomically adds a value and returns the new value
		Units::day& Sub(const Units::day& v); // atomically subtracts a value and returns the new value
		Units::day Add(const Units::day& v) const; // adds a value and returns the new value
		Units::day Sub(const Units::day& v) const; // subtracts a value and returns the new value
		const Units::day& GetValue() const;
		Units::day& SetValue(const Units::day& v); // returns the previous value while setting with the new value
		Units::day& SetValue(Units::day&& v); // returns the previous value while setting with the new value
		const Units::day& load() const;

	private:
		std::string	ToString() const;
		DateTime& FromString(const std::string& timeStr);

	public:
		static DateTime timeFromString(const std::string& timeStr = "1970/1/1 0:0:0");
		static DateTime Epoch();
		static DateTime Now();
		static DateTime createTimeFromMinutes(float minutes);
		static int			getNumDaysInSameMonth(DateTime const& in);
		static DateTime	make_time(int year = 1970, int month = 1, int day = 1, int hour = 0, int minute = 0, float second = 0, bool useLocalTime = true);
		static Units::second GetUtcOffset(DateTime const& in);

	public:
		DateTime& ToStartOfMonth();
		DateTime& ToStartOfDay();
		DateTime& ToStartOfHour();
		DateTime& ToStartOfMinute();
		DateTime& ToEndOfMonth();
		DateTime& ToEndOfDay();
		DateTime& ToEndOfHour();
		DateTime& ToEndOfMinute();

	public:
		/* milliseconds after the second - [0, 1000] including leap second */
		long double tm_fractionalsec() const;

		/* seconds after the minute - [0, 60] including leap second */
		long double tm_sec() const;

		/* minutes after the hour - [0, 59] */
		int tm_min() const;

		/* hours since midnight - [0, 23] */
		int tm_hour() const;

		/* day of the month - [1, 31] */
		int tm_mday() const;

		/* months since January - [0, 11] */
		int tm_mon() const;

		/* years since 1900 */
		int tm_year() const;

		/* days since Sunday - [0, 6] */
		int tm_wday() const;

		/* days since January 1 - [0, 365] */
		int tm_yday() const;

	public:
		std::string c_str() const;
		operator std::string() const;
		operator Units::second() const;
		operator Units::millisecond() const;
		operator Units::minute() const;
		operator Units::hour() const;
		operator Units::day() const;
		operator Units::year() const;

		friend bool	operator==(const DateTime& a, DateTime const& t);
		friend bool	operator!=(const DateTime& a, DateTime const& t);
		friend bool	operator>=(const DateTime& a, DateTime const& t);
		friend bool	operator<=(const DateTime& a, DateTime const& t);
		friend bool	operator>(const DateTime& a, DateTime const& t);
		friend bool	operator<(const DateTime& a, DateTime const& t);

		friend bool	operator==(const DateTime& a, Units::second const& t);
		friend bool	operator!=(const DateTime& a, Units::second const& t);
		friend bool	operator>=(const DateTime& a, Units::second const& t);
		friend bool	operator<=(const DateTime& a, Units::second const& t);
		friend bool	operator>(const DateTime& a, Units::second const& t);
		friend bool	operator<(const DateTime& a, Units::second const& t);

		friend bool	operator==(const Units::second& a, DateTime const& t);
		friend bool	operator!=(const Units::second& a, DateTime const& t);
		friend bool	operator>=(const Units::second& a, DateTime const& t);
		friend bool	operator<=(const Units::second& a, DateTime const& t);
		friend bool	operator>(const Units::second& a, DateTime const& t);
		friend bool	operator<(const Units::second& a, DateTime const& t);

		friend std::ostream& operator<<(std::ostream& os, DateTime const& obj);
		friend std::stringstream& operator>>(std::stringstream& os, DateTime& obj);

		DateTime& operator+=(Units::second seconds);
		DateTime& operator-=(Units::second seconds);
		DateTime& operator*=(Units::second seconds);
		DateTime& operator/=(Units::second seconds);

		friend DateTime operator+(const DateTime& a, const DateTime& b);
		friend DateTime operator-(const DateTime& a, const DateTime& b);
		friend DateTime operator*(const DateTime& a, const DateTime& b);
		friend DateTime operator/(const DateTime& a, const DateTime& b);

		friend DateTime operator+(const DateTime& a, const Units::second& b);
		friend DateTime operator-(const DateTime& a, const Units::second& b);
		friend DateTime operator*(const DateTime& a, const Units::second& b);
		friend DateTime operator/(const DateTime& a, const Units::second& b);

		friend DateTime operator+(const Units::second& a, const DateTime& b);
		friend DateTime operator-(const Units::second& a, const DateTime& b);
		friend DateTime operator*(const Units::second& a, const DateTime& b);
		friend DateTime operator/(const Units::second& a, const DateTime& b);


	};
	template <> __forceinline std::string ToString(DateTime const& r) { return (std::string)r; };
};
