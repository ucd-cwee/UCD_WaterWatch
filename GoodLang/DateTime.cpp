#pragma once
#include "DateTime.h"
#include <boost/date_time.hpp>

namespace GoodLang {
	boost::posix_time::ptime const& Shared_Epoch_posixTime() {
		static boost::posix_time::ptime rc{ boost::posix_time::time_from_string("1970/1/1 0:0:0") };
		return rc;
	};
	boost::posix_time::ptime ToPTime(DateTime const& F) {
		Units::millisecond D{ F.time };
		if (D < 0) {
			return Shared_Epoch_posixTime() + boost::posix_time::milliseconds((long long)(D()));
		}
		else {
			return Shared_Epoch_posixTime() - boost::posix_time::milliseconds((long long)(-(D())));
		}
	};
	Units::day FromPTime(boost::posix_time::ptime const& time) {
		boost::posix_time::ptime const& epoch = Shared_Epoch_posixTime();

		if (time >= epoch) {
			return Units::millisecond((time - epoch).total_milliseconds());
		}
		else {
			return -Units::millisecond((epoch - time).total_milliseconds());
		}
	};

	// DateTime::DateTime(const boost::posix_time::ptime& t) : time{ FromPTime(t) } {};
	DateTime::DateTime() : time{ FromPTime(Shared_Epoch_posixTime()) } {};
	DateTime::DateTime(std::string const& t) : time{ FromPTime(Shared_Epoch_posixTime()) } { this->FromString(t); };

	Units::day& DateTime::Add(const Units::day& v) { time += v; return time; }; // atomically adds a value and returns the new value
	Units::day& DateTime::Sub(const Units::day & v) { time -= v; return time; }; // atomically subtracts a value and returns the new value
	Units::day DateTime::Add(const Units::day & v) const { return time + v; }; // adds a value and returns the new value
	Units::day DateTime::Sub(const Units::day & v) const { return time - v; }; // subtracts a value and returns the new value
	const Units::day& DateTime::GetValue() const { return time; };
	Units::day& DateTime::SetValue(const Units::day & v) { return time = v; }; // returns the previous value while setting with the new value
	Units::day& DateTime::SetValue(Units::day && v) { return time = std::forward<Units::day>(v); }; // returns the previous value while setting with the new value
	const Units::day& DateTime::load() const { return GetValue(); };

	Units::second getUtcOffset_impl() {
		bool isNegative;

		time_t ts = 0;
		char buf[16];
		decltype(auto) t = ::localtime(&ts);
		::strftime(buf, sizeof(buf), "%z", t);
		std::string offset = buf; // -0800
		isNegative = offset.find('-') >= 0;
		// get the right 2 values
		decltype(auto) minuteOffset = std::atof(offset.substr(offset.length() - 2, 2).c_str()); // 00
		decltype(auto) hourOffset = std::atof(offset.substr(offset.length() - 4, 4).substr(0, 2).c_str()); // 08

		Units::second offsetV = ((hourOffset * 3600.0) + (minuteOffset * 60.0)) * (isNegative ? -1.0 : 1.0);

		if (t->tm_isdst) {
			// offsetV -= Units::second(3600);
		}

		return offsetV;
	};
	Units::second getUtcOffset() {
		static Units::second tr(getUtcOffset_impl());
		return tr;
	};
	Units::second getUtcOffset_impl(boost::posix_time::ptime const& pt) {
		bool isNegative;

		// time_t ts = boost::posix_time::to_tm(pt);
		char buf[16];
		decltype(auto) t = boost::posix_time::to_tm(boost::posix_time::ptime(pt.date()));
		::mktime(&t);
		::strftime(buf, sizeof(buf), "%z", &t);
		std::string offset = buf; // -0800

		isNegative = offset.find('-') >= 0;
		// get the right 2 values

		decltype(auto) minuteOffset = std::atof(offset.substr(offset.length() - 2, 2).c_str()); // 00
		decltype(auto) hourOffset = std::atof(offset.substr(offset.length() - 4, 4).substr(0, 2).c_str()); // 08

		Units::second offsetV = ((hourOffset * 3600.0) + (minuteOffset * 60.0)) * (isNegative ? -1.0 : 1.0);

		if (t.tm_isdst) {
			// offsetV -= Units::second(3600);
		}

		return offsetV;
	};
	Units::second getUtcOffset(boost::posix_time::ptime const& pt) {
		return getUtcOffset_impl(pt);
	};
	DateTime const& Shared_Epoch() { static DateTime rc{ FromPTime(Shared_Epoch_posixTime()) }; return rc; }
	std::string	DateTime::ToString() const {
		DateTime temp{ this->time + getUtcOffset(ToPTime(*this)) };

		return GoodLang::printf("%i/%i/%i %i:%i:%f",
			temp.tm_year() + 1900,
			temp.tm_mon() + 1,
			temp.tm_mday(),
			temp.tm_hour(),
			temp.tm_min(),
			temp.tm_sec()
		);
	};
	DateTime& DateTime::FromString(const std::string & timeStr) {
		DateTime t{ FromPTime(boost::posix_time::time_from_string(timeStr.c_str())) };
		return operator=(DateTime{ t.time - getUtcOffset(ToPTime(t)) });
	};

	DateTime DateTime::timeFromString(const std::string & timeStr) { return DateTime().FromString(timeStr); };
	DateTime DateTime::Epoch() { return Shared_Epoch(); };
	DateTime DateTime::Now() { return DateTime{ static_cast<long double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) / 1000.0 }; };
	DateTime DateTime::createTimeFromMinutes(float minutes) {
		decltype(auto) t = Now();
		t.ToStartOfDay();
		t += (minutes * 60.0f);
		return t;
	}
	int			DateTime::getNumDaysInSameMonth(DateTime const& in) {
		Units::day duration = (Units::day)(DateTime(in).ToEndOfMonth() - DateTime(in).ToStartOfMonth());
		duration += 0.5;
		return (int)duration.floor()();
		// return (int)std::floor((((long double)(DateTime(in).ToEndOfMonth() - DateTime(in).ToStartOfMonth())) / (24.0 * 60.0 * 60.0)) + 0.5);
	};
	DateTime	DateTime::make_time(int year, int month, int day, int hour, int minute, float second, bool useLocalTime) {
		DateTime t = DateTime::timeFromString(GoodLang::printf("%i/%i/%i %i:%i:%f", year, month, day, hour, minute, second));
		//if (useLocalTime) { t -= getUtcOffset(t.ToPTime())(); }
		return t;
	};
	Units::second DateTime::GetUtcOffset(DateTime const& in) {
		return getUtcOffset(ToPTime(in));
	};

	DateTime& DateTime::ToStartOfMonth() {
		this->operator-=(Units::second(tm_sec()));
		this->operator-=(Units::minute(tm_min()));
		this->operator-=(Units::hour(tm_hour()));
		this->operator-=(Units::day(tm_mday() - 1));
		return *this;
	};
	DateTime& DateTime::ToStartOfDay() {
		this->operator-=(Units::second(tm_sec()));
		this->operator-=(Units::minute(tm_min()));
		this->operator-=(Units::hour(tm_hour()));
		return *this;
	};
	DateTime& DateTime::ToStartOfHour() {
		this->operator-=(Units::second(tm_sec()));
		this->operator-=(Units::minute(tm_min()));
		return *this;
	};
	DateTime& DateTime::ToStartOfMinute() {
		this->operator-=(Units::second(tm_sec()));
		return *this;
	};
	DateTime& DateTime::ToEndOfMonth() {
		if (tm_mon() >= 11) {
			DateTime out(make_time(tm_year() + 1901, 1, 1, 0, 0, 0));
			out -= 1;
			*this = out; //  (make_time(tm_year() + 1901, 1, 1, 0, 0, 0) - 1);
		}
		else {
			DateTime out(make_time(tm_year() + 1900, tm_mon() + 2, 1, 0, 0, 0));
			out -= 1;
			*this = out; // (make_time(tm_year() + 1900, tm_mon() + 2, 1, 0, 0, 0) - 1);
		}
		return *this;
	};
	DateTime& DateTime::ToEndOfDay() {
		this->operator+=(Units::second(60 - tm_sec()));
		this->operator+=(Units::second((59 - tm_min()) * 60));
		this->operator+=(Units::second((23 - tm_hour()) * 3600));
		return *this;
	};
	DateTime& DateTime::ToEndOfHour() {
		this->operator+=(Units::second(60 - tm_sec()));
		this->operator+=(Units::second((59 - tm_min()) * 60));
		return *this;
	};
	DateTime& DateTime::ToEndOfMinute() {
		this->operator+=(Units::second(60 - tm_sec()));
		return *this;
	};

	/* milliseconds after the second - [0, 1000] including leap second */
	long double DateTime::tm_fractionalsec() const {
		boost::posix_time::time_duration td(0, 0, 0, ToPTime(*this).time_of_day().fractional_seconds());
		decltype(auto) t = (long double)(td.total_nanoseconds()) / 1000000000.0L;
		return t;
	};

	/* seconds after the minute - [0, 60] including leap second */
	long double DateTime::tm_sec() const {
		return ((long double)ToPTime(*this).time_of_day().seconds()) + tm_fractionalsec();
	};

	/* minutes after the hour - [0, 59] */
	int DateTime::tm_min() const { return ToPTime(*this).time_of_day().minutes(); };

	/* hours since midnight - [0, 23] */
	int DateTime::tm_hour() const { return ToPTime(*this).time_of_day().hours(); };

	/* day of the month - [1, 31] */
	int DateTime::tm_mday() const { return ToPTime(*this).date().year_month_day().day; };

	/* months since January - [0, 11] */
	int DateTime::tm_mon() const { return ToPTime(*this).date().year_month_day().month - 1; };

	/* years since 1900 */
	int DateTime::tm_year() const { return ToPTime(*this).date().year_month_day().year - 1900; };

	/* days since Sunday - [0, 6] */
	int DateTime::tm_wday() const { return ToPTime(*this).date().day_of_week().as_number(); };

	/* days since January 1 - [0, 365] */
	int DateTime::tm_yday() const { return ToPTime(*this).date().day_of_year() - 1; };

	std::string DateTime::c_str() const {
		return ToString();
	};
	DateTime::operator std::string() const { return ToString(); };
	DateTime::operator Units::second() const { return time; };
	DateTime::operator Units::millisecond() const { return time; };
	DateTime::operator Units::minute() const { return time; };
	DateTime::operator Units::hour() const { return time; };
	DateTime::operator Units::day() const { return time; };
	DateTime::operator Units::year() const { return time; };

	bool	operator==(const DateTime & a, DateTime const& t) { return a.time == t.time; };
	bool	operator!=(const DateTime & a, DateTime const& t) { return !(a == t); };
	bool	operator>=(const DateTime & a, DateTime const& t) { return a.time >= t.time; };
	bool	operator<=(const DateTime & a, DateTime const& t) { return a.time <= t.time; };
	bool	operator>(const DateTime & a, DateTime const& t) { return !(a <= t); };
	bool	operator<(const DateTime & a, DateTime const& t) { return !(a >= t); };

	bool	operator==(const DateTime & a, Units::second const& t) { return a.time == t; };
	bool	operator!=(const DateTime & a, Units::second const& t) { return !(a == t); };
	bool	operator>=(const DateTime & a, Units::second const& t) { return a.time >= t; };
	bool	operator<=(const DateTime & a, Units::second const& t) { return a.time <= t; };
	bool	operator>(const DateTime & a, Units::second const& t) { return !(a <= t); };
	bool	operator<(const DateTime & a, Units::second const& t) { return !(a >= t); };

	bool	operator==(const Units::second & a, DateTime const& t) { return a == t.time; };
	bool	operator!=(const Units::second & a, DateTime const& t) { return !(a == t); };
	bool	operator>=(const Units::second & a, DateTime const& t) { return a >= t.time; };
	bool	operator<=(const Units::second & a, DateTime const& t) { return a <= t.time; };
	bool	operator>(const Units::second & a, DateTime const& t) { return !(a <= t); };
	bool	operator<(const Units::second & a, DateTime const& t) { return !(a >= t); };

	std::ostream& operator<<(std::ostream & os, DateTime const& obj) { os << obj.ToString(); return os; };
	std::stringstream& operator>>(std::stringstream & os, DateTime & obj) { Units::second v = 0; os >> v; obj = v; return os; };

	DateTime& DateTime::operator+=(Units::second seconds) {
		time += Units::second(seconds);
		return *this;
	};
	DateTime& DateTime::operator-=(Units::second seconds) {
		time -= Units::second(seconds);
		return *this;
	};
	DateTime& DateTime::operator*=(Units::second seconds) {
		time *= seconds;
		return *this;
	};
	DateTime& DateTime::operator/=(Units::second seconds) {
		time /= seconds;
		return *this;
	};

	DateTime operator+(const DateTime & a, const DateTime & b) {
		DateTime out(a);
		out += b.time;
		return out;
	};
	DateTime operator-(const DateTime & a, const DateTime & b) {
		DateTime out(a);
		out -= b.time;
		return out;
	};
	DateTime operator*(const DateTime & a, const DateTime & b) {
		DateTime out(a);
		out *= b.time;
		return out;
	};
	DateTime operator/(const DateTime & a, const DateTime & b) {
		DateTime out(a);
		out /= b.time;
		return out;
	};

	DateTime operator+(const DateTime & a, const Units::second & b) {
		DateTime out(a);
		out.time += b;
		return out;
	};
	DateTime operator-(const DateTime & a, const Units::second & b) {
		DateTime out(a);
		out.time -= b;
		return out;
	};
	DateTime operator*(const DateTime & a, const Units::second & b) {
		DateTime out(a);
		out.time *= b;
		return out;
	};
	DateTime operator/(const DateTime & a, const Units::second & b) {
		DateTime out(a);
		out.time /= b;
		return out;
	};

	DateTime operator+(const Units::second & a, const DateTime & b) {
		DateTime out{ Units::second(a) };
		out += b.time;
		return out;
	};
	DateTime operator-(const Units::second & a, const DateTime & b) {
		DateTime out{ Units::second(a) };
		out -= b.time;
		return out;
	};
	DateTime operator*(const Units::second & a, const DateTime & b) {
		DateTime out{ Units::second(a) };
		out *= b.time;
		return out;
	};
	DateTime operator/(const Units::second & a, const DateTime & b) {
		DateTime out{ Units::second(a) };
		out /= b.time;
		return out;
	};






};