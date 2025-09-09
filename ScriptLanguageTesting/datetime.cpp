#include "datetime.h"
#include <boost/date_time.hpp>

namespace GL {
	boost::posix_time::ptime const& Shared_Epoch_posixTime() {
		static boost::posix_time::ptime rc{ boost::posix_time::time_from_string("1970/1/1 0:0:0") };
		return rc;
	};
	boost::posix_time::ptime ToPTime(datetime const& F) {
		long long D = F;
		if (D >= 0) {
			long long num_millisec = static_cast<long long>(D);
			return Shared_Epoch_posixTime() + boost::posix_time::milliseconds(num_millisec);
		}
		else {
			long long num_millisec = static_cast<long long>(-D);
			return Shared_Epoch_posixTime() - boost::posix_time::milliseconds(num_millisec);
		}
	};
	long long FromPTime(boost::posix_time::ptime const& time) {
		boost::posix_time::ptime const& epoch = Shared_Epoch_posixTime();
		if (time >= epoch) {
			return (time - epoch).total_milliseconds();
		}
		else {
			return -(epoch - time).total_milliseconds();
		}
	};
	datetime::datetime()
		: time{ FromPTime(Shared_Epoch_posixTime()) } {};
	datetime::datetime(int year, int month, int day, int hour, int minute, float second, bool useLocalTime) 
		: time{ FromPTime(Shared_Epoch_posixTime()) } { this->FromString(GL::printf("%i/%i/%i %i:%i:%f", year, month, day, hour, minute, second)); };
	datetime::datetime(GL::string const& t)
		: time{ FromPTime(Shared_Epoch_posixTime()) } { this->FromString(t); };
	datetime::datetime( long long unixtime)
		: time{ unixtime } {};
	long long getUtcOffset_impl(boost::posix_time::ptime const& pt) {
		bool isNegative;

		// time_t ts = boost::posix_time::to_tm(pt);
		char buf[16];
		decltype(auto) t = boost::posix_time::to_tm(boost::posix_time::ptime(pt.date()));
		::mktime(&t);
		::strftime(buf, sizeof(buf), "%z", &t);
		std::string offset = buf; // -0800

		isNegative = offset.find('-') >= 0;
		// get the right 2 values

		decltype(auto) minuteOffset = std::atoll(offset.substr(offset.length() - 2, 2).c_str()); // 00
		decltype(auto) hourOffset = std::atoll(offset.substr(offset.length() - 4, 4).substr(0, 2).c_str()); // 08

		long long offsetV = ((hourOffset * 3600000) + (minuteOffset * 60000)) * (isNegative ? -1 : 1);

		if (t.tm_isdst) {
			// offsetV -= Units::second(3600);
		}

		return offsetV;
	};
	long long getUtcOffset(boost::posix_time::ptime const& pt) {
		return getUtcOffset_impl(pt);
	};
	GL::string datetime::ToString() const {
		datetime temp{ *this };
		temp.time += getUtcOffset(ToPTime(*this));

		auto date = ToPTime(temp).date();
		auto Time = ToPTime(temp).time_of_day();
		return GL::printf("%i/%i/%i %i:%i:%i",
			(int)date.year_month_day().year,
			(int)date.year_month_day().month,
			(int)date.year_month_day().day.as_number(),
			(int)Time.hours(),
			(int)Time.minutes(),
			(int)Time.seconds()
		);
	};
	datetime& datetime::FromString(const GL::string& timeStr) {
		datetime t{ FromPTime(boost::posix_time::time_from_string(timeStr.to_string())) };
		return operator=(datetime{ t.time.load() - getUtcOffset(ToPTime(t)) });
	};
	datetime datetime::timeFromString(const GL::string& timeStr/* = "1970/1/1 0:0:0"*/) { 
		datetime out;
		out.FromString(timeStr);
		return out;
	};
	datetime const& datetime::Epoch() { 
		static datetime rc{ FromPTime(Shared_Epoch_posixTime()) };
		return rc;
	};
	datetime datetime::Now() { 
		return datetime{ static_cast< long long>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) };
	};
	datetime datetime::createTimeFromMinutes( long long minutes) {
		decltype(auto) t = Now();
		t.ToStartOfDay();
		t.time = t.time.load() + (minutes * 60000);
		return t;
	}
	int	datetime::getNumDaysInSameMonth(datetime const& in) {
		datetime delta = in;
		return (int)(float)(GL::day(datetime(delta).ToEndOfMonth() - datetime(delta).ToStartOfMonth()) + 0.5).floor();
	};
	datetime datetime::make_time(int year, int month , int day, int hour , int minute , float second/*, bool useLocalTime*/) {
		datetime t = datetime::timeFromString(GL::printf("%i/%i/%i %i:%i:%f", year, month, day, hour, minute, second));
		// if (useLocalTime) { t -= getUtcOffset(t.ToPTime())(); }
		return t;
	};
	GL::minute datetime::GetUtcOffset(datetime const& in) {
		return GL::millisecond(static_cast<float>(getUtcOffset(ToPTime(in))));
	};
	datetime datetime::ToStartOfMonth() const {
		datetime out{ *this };
		out -= GL::second(static_cast<float>(out.tm_sec()));
		out -= GL::minute(static_cast<float>(out.tm_min()));
		out -= GL::hour(static_cast<float>(out.tm_hour()));
		out -= GL::day(static_cast<float>(out.tm_mday() - 1));
		out -= GetUtcOffset(out);
		return out;
	};
	datetime datetime::ToStartOfDay() const {
		datetime out{ *this };
		out -= GL::second(static_cast<float>(out.tm_sec()));
		out -= GL::minute(static_cast<float>(out.tm_min()));
		out -= GL::hour(static_cast<float>(out.tm_hour()));
		out -= GetUtcOffset(out);
		return out;
	};
	datetime datetime::ToStartOfHour() const {
		datetime out{ *this };
		out -= GL::second(static_cast<float>(out.tm_sec()));
		out -= GL::minute(static_cast<float>(out.tm_min()));
		return out;
	};
	datetime datetime::ToStartOfMinute() const {
		datetime out{ *this };
		out -= GL::second(static_cast<float>(out.tm_sec()));
		return out;
	};
	datetime datetime::ToEndOfMonth() const {
		datetime out{ *this };
		if (out.tm_mon() >= 11) {
			return make_time(out.tm_year() + 1901, 1, 1, 0, 0, 0) - GL::millisecond(1);
		}
		else {
			return make_time(out.tm_year() + 1900, out.tm_mon() + 2, 1, 0, 0, 0) - GL::millisecond(1);
		}
	};
	datetime datetime::ToEndOfDay() const {
		datetime out{ *this };
		out += GL::second(static_cast<float>(60 - out.tm_sec()));
		out += GL::second(static_cast<float>((60 - out.tm_min()) * 60));
		out += GL::second(static_cast<float>((24 - out.tm_hour()) * 3600));
		out -= GetUtcOffset(out);
		out -= GL::millisecond(1);		
		return out;
	};
	datetime datetime::ToEndOfHour() const {
		datetime out{ *this };
		out += GL::second(static_cast<float>(60 - out.tm_sec()));
		out += GL::second(static_cast<float>((60 - out.tm_min()) * 60));
		out -= GL::millisecond(1);
		return out;
	};
	datetime datetime::ToEndOfMinute() const {
		datetime out{ *this };
		out += GL::second(static_cast<float>(60 - out.tm_sec()));
		out -= GL::millisecond(1);
		return out;
	};
	datetime datetime::ToNextMonth() const {
		datetime out{ *this };
		if (out.tm_mon() >= 11) {
			return make_time(out.tm_year() + 1901, 1, 1, 0, 0, 0);
		}
		else {
			return make_time(out.tm_year() + 1900, out.tm_mon() + 2, 1, 0, 0, 0);
		}
	};
	datetime datetime::ToNextDay() const {
		datetime out{ *this };
		out += GL::second(static_cast<float>(60 - out.tm_sec()));
		out += GL::second(static_cast<float>((60 - out.tm_min()) * 60));
		out += GL::second(static_cast<float>((24 - out.tm_hour()) * 3600));
		out -= GetUtcOffset(out);
		return out;
	};
	datetime datetime::ToNextHour() const {
		datetime out{ *this };
		out += GL::second(static_cast<float>(60 - out.tm_sec()));
		out += GL::second(static_cast<float>((60 - out.tm_min()) * 60));
		return out;
	};
	datetime datetime::ToNextMinute() const {
		datetime out{ *this };
		out += GL::second(static_cast<float>(60 - out.tm_sec()));
		return out;
	};
	long double datetime::tm_fractionalsec() const {
		boost::posix_time::time_duration td(0, 0, 0, ToPTime(*this).time_of_day().fractional_seconds());
		decltype(auto) t = (long double)(td.total_nanoseconds()) / 1000000000.0L;
		return t;
	};
	long double datetime::tm_sec() const {
		return ((long double)ToPTime(*this).time_of_day().seconds()) + tm_fractionalsec();
	};
	int datetime::tm_min() const { 
		return static_cast<int>(ToPTime(*this).time_of_day().minutes()); 
	};
	int datetime::tm_hour() const { 
		return static_cast<int>(ToPTime(*this).time_of_day().hours()); 
	};
	int datetime::tm_mday() const { 
		return static_cast<int>(ToPTime(*this).date().year_month_day().day);
	};
	int datetime::tm_mon() const { 
		return ToPTime(*this).date().year_month_day().month - 1;
	};
	int datetime::tm_year() const { 
		return ToPTime(*this).date().year_month_day().year - 1900; 
	};
	int datetime::tm_wday() const {
		return ToPTime(*this).date().day_of_week().as_number(); 
	};
	int datetime::tm_yday() const { 
		return ToPTime(*this).date().day_of_year() - 1; 
	};
	GL::string datetime::c_str() const {
		return ToString();
	};
	datetime::operator GL::string() const {
		return c_str();
	};
	datetime::operator  long long() const {
		return time.load();
	};
	bool operator==(const datetime& a, datetime const& t) { return a.time == t.time; };
	bool operator!=(const datetime& a, datetime const& t) { return !(a == t); };
	bool operator>=(const datetime& a, datetime const& t) { return a.time >= t.time; };
	bool operator<=(const datetime& a, datetime const& t) { return a.time <= t.time; };
	bool operator>(const datetime& a, datetime const& t) { return !(a <= t); };
	bool operator<(const datetime& a, datetime const& t) { return !(a >= t); };
	std::ostream& operator<<(std::ostream& os, datetime const& obj) { os << obj.ToString(); return os; };
	datetime& datetime::operator+=(GL::minute delta) {
		time += (long long)((double)(float)delta * 60000.0);
		return *this; 
	};
	datetime& datetime::operator-=(GL::minute delta) {
		time -= (long long)((double)(float)delta * 60000.0);
		return *this;
	};
	datetime& datetime::operator*=(double mult) {
		long long old;
		for (;;) {
			old = time.load();
			if (time.compare_exchange_strong(old, static_cast<long long>(static_cast<double>(old) * mult))) break;
		}
		return *this;
	};
	datetime& datetime::operator/=(double mult) { 
		long long old;
		for (;;) {
			old = time.load();
			if (time.compare_exchange_strong(old, static_cast<long long>(static_cast<double>(old) / mult))) break;
		}
		return *this;
	};
	GL::minute operator-(const datetime& a, const datetime& b) { return (float)((long long)a - (long long)b) / 60000.0f; };
	datetime operator+(const datetime& a, const GL::minute& b) {
		datetime out{ a };
		out += b;
		return out;
	};	
	datetime operator-(const datetime& a, const GL::minute& b) { 
		return operator+(a, -b);
	};
	datetime operator*(const datetime& a, double b) { 
		datetime out{ a };
		out *= b;
		return out;
	};
	datetime operator/(const datetime& a, double b) { 
		datetime out{ a };
		out /= b;
		return out;
	};
	datetime operator+(const GL::minute& a, const datetime& b) {
		return b + a;
	};
	datetime operator*(double a, const datetime& b) {
		datetime out{ b };
		out *= a;
		return out;
	};

};