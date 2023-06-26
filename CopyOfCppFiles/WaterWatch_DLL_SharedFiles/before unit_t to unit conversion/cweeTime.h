#ifndef __CWEETIME_H__
#define __CWEETIME_H__

#pragma hdrstop
#include "Precompiled.h"
#include "boost/date_time.hpp"

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
	static boost::posix_time::ptime const& Shared_Epoch_posixTime() { static boost::posix_time::ptime rc(boost::posix_time::time_from_string("1970/1/1 0:0:0")); return rc; }
	static cweeTime const& Shared_Epoch() { static cweeTime rc(Shared_Epoch_posixTime()); return rc; }

public:	
	static cweeTime Epoch() { return Shared_Epoch(); };
	static cweeTime Now() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0; };

	cweeTime() {};
	cweeTime(const cweeTime& t) : time(t.time) {};
	cweeTime(const boost::posix_time::ptime& t) : time(t) {};
	cweeTime(const u64& t) {
		*this = Shared_Epoch();
		if (t > 0) {
			time += boost::posix_time::seconds((long long)t);
		}
		else {
			time -= boost::posix_time::seconds((long long)(-t));
		}
	};
	cweeTime(const char* t) { this->FromString(t); };

	const char* c_str() const { 
		return cweeStr::printf("%i/%i/%i %i:%i:%f",
			tm_year() + 1900, 
			tm_mon() + 1,
			tm_mday(),
			tm_hour(),
			tm_min(),
			(float)tm_sec()
		);	
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

	cweeTime& ToEndOfMonth() {		
		if (tm_mon() >= 11) {
			*this = make_time(tm_year() + 1901, 1, 1, 0, 0, 0) - 1;			
		}
		else {
			*this = make_time(tm_year() + 1900, tm_mon() + 2, 1, 0, 0, 0) - 1;
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
	static cweeTime		make_time(int year = 1970, int month = 1, int day = 1, int hour = 0, int minute = 0, float second = 0) {
		return cweeTime::timeFromString(cweeStr::printf("%i/%i/%i %i:%i:%f", year, month, day, hour, minute, second));
	};

private:
	cweeStr	ToString() const {
		return cweeStr::printf("%i/%i/%i %i:%i:%f", tm_year()+1900, tm_mon()+1, tm_mday(), tm_hour(), tm_min(), tm_sec());
	};
	void	FromString(const cweeStr& a) {

		*this = timeFromString(a);
	};

};

#endif