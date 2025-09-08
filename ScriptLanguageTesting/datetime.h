#pragma once

#include "units.h"
#include "atomic_numbers.h"

namespace GL {
	// Thread- and Fiber-safe wrapper for Units::day which supports fundamental datetime and Duration math. Utilizes Boost for the timezone math, AM/PM extractions, string conversions, etc.
	// Meant to be used in parallel with Units for proper unit management while manipulating datetime ranges.
	// E.g: using namespace literals; return datetime::make_time(1940, 1, 1) + 1_d - 30_s;
	class datetime {
	public:
		std::atomic<double> time; // stored as a thread-safe double. 

	public:
		datetime();
		datetime(int year, int month, int day, int hour = 0, int minute = 0, float second = 0, bool useLocalTime = true);
		datetime(double unixtime);
		datetime(const datetime& other) : time(other.time.load()) {};
		datetime(datetime&& other) : time(other.time.load()) {};
		datetime& operator=(const datetime& other) {
			time = other.time.load();
			return *this;
		};
		datetime& operator=(datetime&& other) {
			time = other.time.load();
			return *this;
		};
		datetime(GL::string const& t);
		~datetime() = default;

	private:
		// prints the current datetime in the unix format "year/month/day hour:minute:second"
		GL::string	ToString() const;
		datetime& FromString(const GL::string& timeStr);

	public:
		static datetime timeFromString(const GL::string& timeStr = "1970/1/1 0:0:0");
		static datetime Epoch();
		static datetime Now();
		static datetime createTimeFromMinutes(double minutes);
		static int		getNumDaysInSameMonth(datetime const& in);
		static datetime	make_time(int year = 1970, int month = 1, int day = 1, int hour = 0, int minute = 0, float second = 0, bool useLocalTime = true);
		static GL::second GetUtcOffset(datetime const& in);

	public:
		datetime& ToStartOfMonth();
		datetime& ToStartOfDay();
		datetime& ToStartOfHour();
		datetime& ToStartOfMinute();
		datetime& ToEndOfMonth();
		datetime& ToEndOfDay();
		datetime& ToEndOfHour();
		datetime& ToEndOfMinute();

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
		GL::string c_str() const;
		operator GL::string() const;
		operator double() const;

		friend bool	operator==(const datetime& a, datetime const& t);
		friend bool	operator!=(const datetime& a, datetime const& t);
		friend bool	operator>=(const datetime& a, datetime const& t);
		friend bool	operator<=(const datetime& a, datetime const& t);
		friend bool	operator>(const datetime& a, datetime const& t);
		friend bool	operator<(const datetime& a, datetime const& t);

		friend std::ostream& operator<<(std::ostream& os, datetime const& obj);

		datetime& operator+=(GL::second seconds);
		datetime& operator-=(GL::second seconds);
		datetime& operator*=(double seconds);
		datetime& operator/=(double seconds);

		friend datetime operator+(const datetime& a, const datetime& b);
		friend GL::second operator-(const datetime& a, const datetime& b);
		friend datetime operator*(const datetime& a, const datetime& b);
		friend datetime operator/(const datetime& a, const datetime& b);

		friend datetime operator+(const datetime& a, const GL::second& b);
		friend datetime operator-(const datetime& a, const GL::second& b);
		friend datetime operator*(const datetime& a, double b);
		friend datetime operator/(const datetime& a, double b);

		friend datetime operator+(const GL::second& a, const datetime& b);
		friend datetime operator-(const GL::second& a, const datetime& b);
		friend datetime operator*(double a, const datetime& b);
		friend datetime operator/(double a, const datetime& b);
	};
};
