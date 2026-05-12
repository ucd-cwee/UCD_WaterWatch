#pragma once

#include "units.h"
#include "atomic_numbers.h"

namespace GL {
	// Thread- and Fiber-safe wrapper for Units::day which supports fundamental datetime and Duration math. Utilizes Boost for the timezone math, AM/PM extractions, string conversions, etc.
	// Meant to be used in parallel with Units for proper unit management while manipulating datetime ranges.
	// E.g: using namespace literals; return datetime::make_time(1940, 1, 1) + 1_d - 30_s;
	class datetime {
	private:
		std::atomic< long long> time; // milliseconds since epoch
		explicit datetime(long long unixtime);

	public:
		datetime();
		datetime(int year, int month, int day, int hour = 0, int minute = 0, float second = 0, bool useLocalTime = true);
		datetime(const datetime& other) : time(other.time.load()) {};
		datetime(datetime&& other) noexcept : time(other.time.load()) {};
		datetime& operator=(const datetime& other) {
			time = other.time.load();
			return *this;
		};
		datetime& operator=(datetime&& other) noexcept {
			time = other.time.load();
			return *this;
		};
		datetime(GL::string const& t);
		~datetime() = default;

	private:
		// prints the current datetime in the unix format "year/month/day hour:minute:second"
		GL::string	ToString() const;
		// determine the datetime from a unix format string "year/month/day hour:minute:second"
		datetime& FromString(const GL::string& timeStr);

	public:
		static datetime timeFromString(const GL::string& timeStr = "1970/1/1 0:0:0");
		static datetime const& Epoch();
		static datetime Now();
		static datetime createTimeFromMinutes( long long minutes);
		static int		getNumDaysInSameMonth(datetime const& in);
		static datetime	make_time(int year = 1970, int month = 1, int day = 1, int hour = 0, int minute = 0, float second = 0/*, bool useLocalTime = true*/);
		static GL::minute GetUtcOffset(datetime const& in);

	public:
		// snaps to the start of the current month
		datetime ToStartOfMonth() const;
		// snaps to the start of the current day
		datetime ToStartOfDay() const;
		// snaps to the start of the current hour
		datetime ToStartOfHour() const;
		// snaps to the start of the current minute
		datetime ToStartOfMinute() const;
		// snaps to one millisecond before the next month
		datetime ToEndOfMonth() const;
		// snaps to one millisecond before the next day
		datetime ToEndOfDay() const;
		// snaps to one millisecond before the next hour
		datetime ToEndOfHour() const;
		// snaps to one millisecond before the next minute
		datetime ToEndOfMinute() const;
		// snaps to start of the next month
		datetime ToNextMonth() const;
		// snaps to start of the next day
		datetime ToNextDay() const;
		// snaps to start of the next hour
		datetime ToNextHour() const;
		// snaps to start of the next minute
		datetime ToNextMinute() const;

	public:
		/* milliseconds after the second - [0, 1000) including leap second */
		long double tm_fractionalsec() const;

		/* seconds after the minute - [0, 60) including leap second */
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
		// prints the current datetime in the unix format "year/month/day hour:minute:second"
		GL::string c_str() const;
		// prints the current datetime in the unix format "year/month/day hour:minute:second"
		operator GL::string() const;
		// milliseconds since epoch. May be negative, indicating time prior to the Epoch. 
		operator long long() const; 

		bool compare_exchange(datetime& expected, datetime const& newValue) {
			long long expect = expected.time.load();
			return this->time.compare_exchange_strong(expect, newValue.time.load());
		};

		friend bool	operator==(const datetime& a, datetime const& t);
		friend bool	operator!=(const datetime& a, datetime const& t);
		friend bool	operator>=(const datetime& a, datetime const& t);
		friend bool	operator<=(const datetime& a, datetime const& t);
		friend bool	operator>(const datetime& a, datetime const& t);
		friend bool	operator<(const datetime& a, datetime const& t);

		friend std::ostream& operator<<(std::ostream& os, datetime const& obj);

		// add the delta to the current datetime
		datetime& operator+=(GL::minute delta);
		// sub the delta to the current datetime
		datetime& operator-=(GL::minute delta);
		// multiply the current datetime by this value.
		datetime& operator*=(double mult);
		// divide the current datetime by this value.
		datetime& operator/=(double mult);

		// returns the time difference between the two dates. 
		friend GL::minute operator-(const datetime& a, const datetime& b);

		friend datetime operator+(const datetime& a, const GL::minute& b);
		friend datetime operator-(const datetime& a, const GL::minute& b);
		friend datetime operator*(const datetime& a, double b);
		friend datetime operator/(const datetime& a, double b);
		friend datetime operator+(const GL::minute& a, const datetime& b);
		friend datetime operator*(double a, const datetime& b);
	};
};