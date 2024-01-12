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
#include "Precompiled.h"

#include "Strings.h"
#include "boost/date_time.hpp"
#include "cweeUnitedValue.h"
// #include "Toasts.h"
// #include <ctime>

class cweeTime {
private:
	/*
	inline static bool timeStrHasClock(const cweeStr& timeStr) {
		if (timeStr.Find(':') >= 0) {
			return true;
		}
		else {
			return false;
		}
	};
	inline static boost::posix_time::ptime formatTimeStr(cweeStrView origStr) {
		cweeStrView timeStr = origStr;
		// check if ISO format or space-seperated format
		bool isISO = timeStr.Find('T') >= 0;
		bool hassClock = isISO || timeStr.Find(':') >= 0;
		bool hasHyphen = timeStr.Find('-') >= 0;
		bool hasBackSlash = timeStr.Find('/') >= 0;
		bool hasForwardSlash = timeStr.Find('\\') >= 0;

		boost::posix_time::ptime out;

		// seperate the clock element
		if (isISO) {
			return boost::posix_time::from_iso_string(timeStr.c_str().c_str());
		} else {
			cweeStrView clock;
			if (hassClock) {
				clock = timeStr.Right(timeStr.Length() - timeStr.Find(' '));
				timeStr = timeStr.Left(timeStr.Find(' ') + 1);
			}
			else {
				static cweeStr defaultClock = "0:0:0";
				clock = defaultClock.View();
			}
			cweeParser a;
			if (hasBackSlash) {
				a = timeStr.c_str().Split("/");
			}
			else if (hasHyphen) {
				a = timeStr.c_str().Split("-");
			}
			else if (hasForwardSlash) {
				a = timeStr.c_str().Split("\\");
			}
			if (a.getNumVars() >= 3) {
				// try and find the largest value
				cweeStrView year;
				for (auto& x : a) {
					if (x.Length() > year.Length()) {
						year = x.View();
					}
				}
				cweeStr dateStamp = year.c_str();
				for (auto& x : a) {
					if (x.c_str() != year) {
						dateStamp.AddToDelimiter(x, "/");
					}
				}
				return boost::posix_time::time_from_string(dateStamp.AddToDelimiter(clock.c_str(), " ").c_str());
			}
			else {
				// we split wrong or the timestamp is wrong.
				// Either way just try and left the
				return boost::posix_time::time_from_string(origStr.c_str().c_str());
			}
		}
	};
	*/

public:
	inline static cweeTime timeFromString(const cweeStr& timeStr = "1970/1/1 0:0:0") {
		// determine if there is a time component to the timeStr		
		return cweeTime(boost::posix_time::time_from_string(timeStr.c_str()));
	};

private:
	static cweeUnitValues::second getUtcOffset_impl() {
		bool isNegative;

		time_t ts = 0;
		char buf[16];
		AUTO t = ::localtime(&ts);
		::strftime(buf, sizeof(buf), "%z", t);
		cweeStr offset = buf; // -0800
		isNegative = offset.Find('-') >= 0;
		// get the right 2 values
		AUTO minuteOffset = offset.Right(2).ReturnNumericD(); // 00
		AUTO hourOffset = offset.Right(4).Left(2).ReturnNumericD(); // 08

		cweeUnitValues::second offsetV = ((hourOffset * 3600.0) + (minuteOffset * 60.0)) * (isNegative ? -1.0 : 1.0);

		if (t->tm_isdst) {
			// offsetV -= cweeUnitValues::second(3600);
		}

		return offsetV;
	};
	static cweeUnitValues::second getUtcOffset() {
		static cweeUnitValues::second tr(getUtcOffset_impl());
		return tr;
	};
	static cweeUnitValues::second getUtcOffset_impl(boost::posix_time::ptime const& pt) {
		bool isNegative;

		// time_t ts = boost::posix_time::to_tm(pt);
		char buf[16];
		AUTO t = boost::posix_time::to_tm(pt);
		::mktime(&t);
		::strftime(buf, sizeof(buf), "%z", &t);
		cweeStr offset = buf; // -0800
		isNegative = offset.Find('-') >= 0;
		// get the right 2 values
		AUTO minuteOffset = offset.Right(2).ReturnNumericD(); // 00
		AUTO hourOffset = offset.Right(4).Left(2).ReturnNumericD(); // 08

		cweeUnitValues::second offsetV = ((hourOffset * 3600.0) + (minuteOffset * 60.0)) * (isNegative ? -1.0 : 1.0);

		if (t.tm_isdst) {
			// offsetV -= cweeUnitValues::second(3600);
		}

		return offsetV;
	};
	static cweeUnitValues::second getUtcOffset(boost::posix_time::ptime const& pt) {
		return cweeUnitValues::second(getUtcOffset_impl(pt));
	};

	static boost::posix_time::ptime const& Shared_Epoch_posixTime() {
		static boost::posix_time::ptime rc(boost::posix_time::time_from_string("1970/1/1 0:0:0"));
		return rc;
	};
	static cweeTime const& Shared_Epoch() { static cweeTime rc(Shared_Epoch_posixTime()); return rc; }

public:	
	static cweeTime Epoch() { return Shared_Epoch(); };
	static cweeTime Now() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0; };

	cweeTime() : time(Shared_Epoch_posixTime()) {};
	cweeTime(const cweeTime& t) : time(t.time) {};
	cweeTime(const boost::posix_time::ptime& t) : time(t) {};
	cweeTime(const u64& t) : time(Shared_Epoch_posixTime()) {
		*this = Shared_Epoch();
		if (t > 0) {
			time += boost::posix_time::seconds((long long)t);
		}
		else {
			time -= boost::posix_time::seconds((long long)(-t));
		}
	};
	cweeTime(const char* t) : time(Shared_Epoch_posixTime()) { this->FromString(t); };
	cweeTime(const cweeUnitValues::unit_value& t) : time(Shared_Epoch_posixTime()) {
		*this = Shared_Epoch();
		if (t > 0) {
			time += boost::posix_time::seconds((long long)cweeUnitValues::second(t)());
		}
		else {
			time -= boost::posix_time::seconds((long long)(-cweeUnitValues::second(t)()));
		}
	};
	cweeStr c_str() const { 
		return ToString();
	};
	operator const char* () const { return ToString(); };
	explicit operator u64() const {
		u64 out = 0;
		boost::posix_time::ptime const& epoch = Shared_Epoch_posixTime();

		if (time >= epoch) {
			out = (u64)((time - epoch).total_nanoseconds()) / 1000000000.0L;
		}
		else {
			out = -1.0L * ((u64)((epoch - time).total_nanoseconds()) / 1000000000.0L);
		}

		return out;
	};
	operator cweeUnitValues::unit_value() const { return cweeUnitValues::second((u64)*this); };

	cweeTime& operator=(const cweeTime& t) { time = t.time; return *this; };
	bool	operator==(const cweeTime& t) const { return (time == t.time); };
	bool	operator!=(const cweeTime& t) const { return (time != t.time); };
	bool	operator>(const cweeTime& t) const { return (time > t.time); };
	bool	operator<(const cweeTime& t) const { return (time < t.time); };
	bool	operator>=(const cweeTime& t) const { return (time >= t.time); };
	bool	operator<=(const cweeTime& t) const { return (time <= t.time); };

	bool	operator==(const double t) const { return ((u64)*this == t); };
	bool	operator!=(const double t) const { return ((u64)*this != t); };
	bool	operator>(const double t) const { return ((u64)*this > t); };
	bool	operator<(const double t) const { return ((u64)*this < t); };
	bool	operator>=(const double t) const { return ((u64)*this >= t); };
	bool	operator<=(const double t) const { return ((u64)*this <= t); };

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

		*this = Shared_Epoch();
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

		*this = Shared_Epoch();
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

		*this = Shared_Epoch();
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

		*this = Shared_Epoch();
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
		out -= b;
		return out;
	};
	friend cweeTime operator-(const cweeTime& a, const double b) {
		cweeTime out(a);
		out -= b;
		return out;
	};
	friend cweeTime operator-(const double a, const cweeTime& b) {
		cweeTime out(a);
		out -= b;
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

	cweeTime& ToEndOfMonth() {		
		if (tm_mon() >= 11) {
			cweeTime out(make_time(tm_year() + 1901, 1, 1, 0, 0, 0));
			out -= 1;
			*this = out; //  (make_time(tm_year() + 1901, 1, 1, 0, 0, 0) - 1);
		}
		else {
			cweeTime out(make_time(tm_year() + 1900, tm_mon() + 2, 1, 0, 0, 0));
			out -= 1;
			*this = out; // (make_time(tm_year() + 1900, tm_mon() + 2, 1, 0, 0, 0) - 1);
		}
		return *this;
	};
	cweeTime& ToEndOfDay() {
		this->operator+=(60 - tm_sec());
		this->operator+=((59 - tm_min()) * 60);
		this->operator+=((23 - tm_hour()) * 3600);
		return *this;
	};
	cweeTime& ToEndOfHour() {
		this->operator+=(60 - tm_sec());
		this->operator+=((59 - tm_min()) * 60);
		return *this;
	};
	cweeTime& ToEndOfMinute() {
		this->operator+=(60 - tm_sec());
		return *this;
	};

	boost::posix_time::ptime	time;

	/* milliseconds after the second - [0, 1000] including leap second */
	u64 tm_fractionalsec() const {
		boost::posix_time::time_duration td(0, 0, 0, time.time_of_day().fractional_seconds());
		AUTO t = (u64)(td.total_nanoseconds()) / 1000000000.0L;
		return t;
	};

	/* seconds after the minute - [0, 60] including leap second */
	u64 tm_sec() const {
		return ((u64)time.time_of_day().seconds()) + tm_fractionalsec();
	};

	/* minutes after the hour - [0, 59] */
	int tm_min() const { return time.time_of_day().minutes(); };

	/* hours since midnight - [0, 23] */
	int tm_hour() const { return time.time_of_day().hours(); };

	/* day of the month - [1, 31] */
	int tm_mday() const { return time.date().year_month_day().day; };

	/* months since January - [0, 11] */
	int tm_mon() const { return time.date().year_month_day().month - 1; };

	/* years since 1900 */
	int tm_year() const { return time.date().year_month_day().year - 1900; };

	/* days since Sunday - [0, 6] */
	int tm_wday() const { return time.date().day_of_week().as_number(); };

	/* days since January 1 - [0, 365] */
	int tm_yday() const { return time.date().day_of_year() - 1; };

	static cweeTime		createTimeFromMinutes(float minutes) {
		AUTO t = Now();
		t.ToStartOfDay();
		t += (minutes * 60.0f);
		return t;
	}
	static int			getNumDaysInSameMonth(cweeTime const& in) {
		return (int)std::floor((((u64)(cweeTime(in).ToEndOfMonth() - cweeTime(in).ToStartOfMonth())) / (24.0 * 60.0 * 60.0)) + 0.5);
	};
	static cweeTime		make_time(int year = 1970, int month = 1, int day = 1, int hour = 0, int minute = 0, float second = 0, bool useLocalTime = true) {
		cweeTime t = cweeTime::timeFromString(cweeStr::printf("%i/%i/%i %i:%i:%f", year, month, day, hour, minute, second));
		if (useLocalTime) { t -= getUtcOffset(t.time)(); }
		return t;
	};
	static cweeUnitValues::second GetUtcOffset(cweeTime const& in) {
		return getUtcOffset(in.time);
	};
private:
	cweeStr	ToString() const {
		cweeTime temp = *this + getUtcOffset(this->time)();

		return cweeStr::printf("%i/%i/%i %i:%i:%f", 
			temp.tm_year()+1900,
			temp.tm_mon()+1,
			temp.tm_mday(),
			temp.tm_hour(),
			temp.tm_min(),
			temp.tm_sec()
		);
	};
	void	FromString(const cweeStr& a) {
		cweeTime t = timeFromString(a);
		*this = t - getUtcOffset(t.time)();
	};

};