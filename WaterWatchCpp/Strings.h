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
#include "enum.h"
#include "StringView.h"

#define ASSERT_ENUM_STRING( string, index )		( 1 / (size_t)!( string - index ) ) ? #string : ""
#define FLOATSIGNBITSET(f)		((*(const unsigned long *)&(f)) >> 31)
#define FLOATSIGNBITNOTSET(f)	((~(*(const unsigned long *)&(f))) >> 31)
#define FLOATNOTZERO(f)			((*(const unsigned long *)&(f)) & ~(1<<31) )
#define INTSIGNBITSET(i)		(((const unsigned long)(i)) >> 31)
#define INTSIGNBITNOTSET(i)		((~((const unsigned long)(i))) >> 31)
#define OLD_INT32_SIGNBITSET(i)		(static_cast<const unsigned int>(i) >> INT32_SIGN_BIT)
#define OLD_INT32_SIGNBITNOTSET(i)	((~static_cast<const unsigned int>(i)) >> INT32_SIGN_BIT)
#define PSEUDO_MAX 1000000000
#define PSEUDO_PENALTY 1000000000
#define DEFAULT_PARTICLE_SIZE 5


class cweeParser;
class cweeStrView;

const size_t STR_ALLOC_BASE = 20;
constexpr size_t STR_ALLOC_GRAN = 32; // was 32. Increasing to see if this helps with performance of saving to file. 

class cweeStr {
public:
	static int INT32_SIGNBITSET(int i) {
		int	r = OLD_INT32_SIGNBITSET(i);
		assert(r == 0 || r == 1);
		return r;
	}
	static int INT32_SIGNBITNOTSET(int i) {
		int	r = OLD_INT32_SIGNBITNOTSET(i);
		assert(r == 0 || r == 1);
		return r;
	}

public:
	cweeStr() {
		Construct();
	};
	cweeStr(const cweeStr& text) {
		Construct();
		size_t l;
		l = text.Length();
		EnsureAlloced(l + 1);
		strncpy(data, text.data, l+1);
		len = l;
	};
	cweeStr(cweeStr&& text) {
		Construct();
		size_t l;
		l = text.Length();
		EnsureAlloced(l + 1);
		strncpy(data, text.data, l + 1);
		len = l;

		text.Clear();
	};
	cweeStr(const cweeStr& text, size_t start, size_t end) {
		Construct();
		size_t i;
		size_t l;

		if (end > text.Length()) {
			end = text.Length();
		}
		if (start > text.Length()) {
			start = text.Length();
		}
		else if (start < 0) {
			start = 0;
		}

		l = end - start;
		if (l < 0) {
			l = 0;
		}

		EnsureAlloced(l + 1);

		for (i = 0; i < l; i++) {
			data[i] = text[start + i];
		}

		data[l] = '\0';
		len = l;
	};
	cweeStr(const char* text) {
		Construct();
		size_t l;
		if (text != NULL) {
			l = strlen(text);
			EnsureAlloced(l + 1);
			strncpy(data, text, l + 1);
			len = l;
		}
	};
	cweeStr(const char* text, size_t start, size_t end) {
		Construct();
		size_t i;
		size_t l = strlen(text);

		if (end > l) {
			end = l;
		}
		if (start > l) {
			start = l;
		}
		else if (start < 0) {
			start = 0;
		}

		l = end - start;
		if (l < 0) {
			l = 0;
		}

		EnsureAlloced(l + 1);

		for (i = 0; i < l; i++) {
			data[i] = text[start + i];
		}

		data[l] = '\0';
		len = l;
	};
	explicit cweeStr(const bool b) {
		Construct();
		EnsureAlloced(2);
		data[0] = b ? '1' : '0';
		data[1] = '\0';
		len = 1;
	};
	explicit cweeStr(const char c) {
		Construct();
		EnsureAlloced(2);
		data[0] = c;
		data[1] = '\0';
		len = 1;
	};
	explicit cweeStr(const int i) {
		Construct();
		this->operator=(std::to_string(i).c_str());
	};
	explicit cweeStr(const unsigned u) {
		Construct();
		this->operator=(std::to_string(u).c_str());
	};
	explicit cweeStr(const float f) {
		Construct();
		this->operator=(std::to_string(f).c_str());
		if (Find(".") >= 0) {
			StripTrailing('0');
			StripTrailing('.'); // if the previous call removed all 0's, this will clean-up the final .
		}
	};
	explicit cweeStr(const double f) {
		Construct();
		this->operator=(std::to_string(f).c_str());
		if (Find(".") >= 0) {
			StripTrailing('0');
			StripTrailing('.'); // if the previous call removed all 0's, this will clean-up the final .
		}
	};
	explicit cweeStr(const time_t time) {
		Construct();
		const char* text = ctime(&time);
		this->operator=(text);
		ReduceSpaces();
	};
	explicit cweeStr(const u64 time) {
		Construct();
		this->operator=(std::to_string(time).c_str());
		if (Find(".") >= 0) {
			StripTrailing('0');
			StripTrailing('.'); // if the previous call removed all 0's, this will clean-up the final .
		}
	};
	explicit cweeStr(const std::string& in) {
		Construct();
		this->operator=(in.c_str());
	};
	explicit cweeStr(const std::pair<u64, float> in) {
		auto first = cweeStr((time_t)in.first);
		auto second = cweeStr((float)in.second);

		Construct();
		size_t l;
		const char* text = cweeStr::printf("(%s, %s)", first.c_str(), second.c_str());
		if (text) {
			l = strlen(text);
			EnsureAlloced(l + 1);
			strncpy(data, text, l + 1);
			len = l;
		}
		ReduceSpaces();
	};
	~cweeStr() {
		FreeData();
	};

	size_t				Size() const {
		return sizeof(*this) + Allocated();
	};
	const char* c_str() const {
		return data;
	};
	operator			const char* () const {
		return c_str();
	};
	explicit operator float() const {
		return this->ReturnNumeric();
	};
	explicit operator double() const {
		return this->ReturnNumericD();
	};
	explicit operator u64() const {
		return (u64)atof(c_str());
	};
	explicit operator int() const {
		return this->ReturnNumericD();
	};
	explicit operator bool() const {
		bool f = this->ReturnNumeric();
		return (f || (this->Icmp(data, "true") == 0));
	};

	char				operator[](size_t index) const {
		return data[index];
	};
	char& operator[](size_t index) {
		return data[index];
	};

	void				operator=(const cweeStr& text) {
		size_t l;
		l = text.Length();
		EnsureAlloced(l + 1, false);
		// std::memcpy(data, text.data, l);
		strncpy(data, text.data, l + 1);
		data[l] = '\0';
		len = l;
	};
	void				operator=(cweeStr&& text) {
		size_t l;
		l = text.Length();
		EnsureAlloced(l + 1, false);
		// std::memcpy(data, text.data, l);
		strncpy(data, text.data, l + 1);
		data[l] = '\0';
		len = l;

		text.Clear();
	};
	void				operator=(const char* text) { this->operator=(cweeStr(text)); };

	friend cweeStr		operator+(const cweeStr& a, const cweeStr& b) {
		cweeStr out = cweeStr(a); out.Append(cweeStr(b)); return out;
	};
	friend cweeStr		operator+(const cweeStr& a, const char* b) {
		cweeStr out = cweeStr(a); out.Append(cweeStr(b)); return out;
	};
	friend cweeStr		operator+(const char* a, const cweeStr& b) {
		cweeStr out = cweeStr(a); out.Append(cweeStr(b)); return out;
	};

	friend cweeStr		operator+(const cweeStr& a, const float b) {
		cweeStr out = cweeStr(a); out.Append(cweeStr(b)); return out;
	};
	friend cweeStr		operator+(const cweeStr& a, const double b) {
		cweeStr out = cweeStr(a); out.Append(cweeStr(b)); return out;
	};
	friend cweeStr		operator+(const cweeStr& a, const int b) {
		cweeStr out = cweeStr(a); out.Append(cweeStr(b)); return out;
	};
	friend cweeStr		operator+(const cweeStr& a, const unsigned b) {
		cweeStr out = cweeStr(a); out.Append(cweeStr(b)); return out;
	};
	friend cweeStr		operator+(const cweeStr& a, const bool b) {
		cweeStr out = cweeStr(a); out.Append(cweeStr(b)); return out;
	};
	friend cweeStr		operator+(const cweeStr& a, const char b) {
		cweeStr out = cweeStr(a); out.Append(cweeStr(b)); return out;
	};

	friend bool			operator<(const cweeStr& a, const cweeStr& b) {
		return a.c_str() < b.c_str();
	};

	cweeStr& operator+=(const cweeStr& a) {
		Append(cweeStr(a));
		return *this;
	};
	cweeStr& operator+=(const char* a) {
		Append(cweeStr(a));
		return *this;
	};
	cweeStr& operator+=(const float a) {
		Append(cweeStr(a));
		return *this;
	};
	cweeStr& operator+=(const double a) {
		Append(cweeStr(a));
		return *this;
	};
	cweeStr& operator+=(const char a) {
		Append(cweeStr(a));
		return *this;
	};
	cweeStr& operator+=(const int a) {
		Append(cweeStr(a));
		return *this;
	};
	cweeStr& operator+=(const unsigned a) {
		Append(cweeStr(a));
		return *this;
	};
	cweeStr& operator+=(const bool a) {
		Append(a ? "true" : "false");
		return *this;
	};
	cweeStr& ReplaceInline(const char* _old, const char* _new) {
		Replace(_old, _new);
		return *this;
	};

	// case sensitive compare
	friend bool			operator==(const cweeStr& a, const cweeStr& b) {
		return (!cweeStr::Cmp(a.data, b.data));
	};
	friend bool			operator==(const cweeStr& a, const char* b) {
		return (!cweeStr::Cmp(a.data, b));
	};
	friend bool			operator==(const char* a, const cweeStr& b) {
		return (!cweeStr::Cmp(a, b.data));
	};

	// case sensitive compare
	friend bool			operator!=(const cweeStr& a, const cweeStr& b) {
		return !(a == b);
	};
	friend bool			operator!=(const cweeStr& a, const char* b) {
		return !(a == b);
	};
	friend bool			operator!=(const char* a, const cweeStr& b) {
		return !(a == b);
	};

	// split based on a delimiter
	cweeParser			Split(const cweeStr& delim) const;
	/* 
	split based on a delimiter, specialized for:
	"a,b,c", "123", 123,, "a,b,c"
	*/ 
	cweeParser			SplitQuotes(const cweeStr& delim) const;
	INLINE cweeStrView	View() const;

	// case sensitive compare
	size_t					Cmp(const char* text) const {
		return cweeStr::Cmp(data, text);
	};
	size_t					Cmpn(const char* text, size_t n) const {
		return cweeStr::Cmpn(data, text, n);
	};
	size_t					CmpPrefix(const char* text) const {
		return cweeStr::Cmpn(data, text, strlen(text));
	};

	// case insensitive compare
	size_t					Icmp(const char* text) const {
		return cweeStr::Icmp(data, text);
	};
	size_t					Icmpn(const char* text, size_t n) const {
		return cweeStr::Icmpn(data, text, n);
	};
	size_t					IcmpPrefix(const char* text) const {
		return cweeStr::Icmpn(data, text, strlen(text));
	};

	// compares paths and makes sure folders come first
	size_t					IcmpPath(const char* text) const {
		return cweeStr::IcmpPath(data, text);
	};
	size_t					IcmpnPath(const char* text, size_t n) const {
		return cweeStr::IcmpnPath(data, text, n);
	};
	size_t					IcmpPrefixPath(const char* text) const {
		return cweeStr::IcmpnPath(data, text, strlen(text));
	};

	template <typename T>
	static cweeStr ToString(T obj) {
		return ToString(obj, typenames::identity<T>());
	};

private:
	template <typename T>
	static cweeStr ToString(T any, typenames::identity<T>)
	{
		throw(
			cweeStr(
				cweeStr("Attempted to cast ") + typeid(any).name() + cweeStr(" to cweeStr.")
			).c_str()
		);
	};
	static cweeStr ToString(bool any, typenames::identity<bool>) { return cweeStr(any); };
	static cweeStr ToString(float any, typenames::identity<float>) { return cweeStr(any); };
	static cweeStr ToString(double any, typenames::identity<double>) { return cweeStr(any); };
	static cweeStr ToString(int any, typenames::identity<int>) { return cweeStr(any); };
	static cweeStr ToString(u64 any, typenames::identity<u64>) { return cweeStr(any); };
	static cweeStr ToString(time_t any, typenames::identity<time_t>) { return cweeStr(any); };
	static cweeStr ToString(const char* any, typenames::identity<const char*>) { return any; };
	static cweeStr ToString(char any, typenames::identity<char>) { return cweeStr(any); };
	static cweeStr ToString(std::string any, typenames::identity<std::string>) { return any.c_str(); };
	static cweeStr ToString(cweeStr any, typenames::identity<cweeStr>) { return any; };

public:

	cweeStr				ParseHtml() const
	{
		const cweeStr& copy = *this;

		// Store the length of the
		// input string
		int n = copy.Length();
		int start = 0, end = 0;

		// Traverse the string
		for (int i = 0; i < n; i++) {
			// If S[i] is '>', update
			// start to i+1 and break
			if (copy.operator [](i) == '>') {
				start = i + 1;
				break;
			}
		}

		// Remove the blank space
		while (copy.operator [](start) == ' ') {
			start++;
		}

		// Traverse the string
		for (int i = start; i < n; i++) {
			// If S[i] is '<', update
			// end to i-1 and break
			if (copy.operator [](i) == '<') {
				end = i - 1;
				break;
			}
		}

		return copy.Mid(start, end);
	};
	static cweeStr		ParseHtml(const cweeStr& copy)
	{
		return copy.ParseHtml();
	};

	cweeStr				RemoveBetween(const cweeStr& left, const cweeStr& right) const {
		// i.e. 
		// left  =   <!-- 
		// right =   -->

		cweeStr out;
		int pos = 0;
		int n = this->Length();
		while (1) {
			if (pos >= n) break;

			int find = this->Find(left, true, pos);
			if (find >= 0) {
				if ((find - pos) > 0) out += this->Mid(pos, find - pos); // left of <!--

				int find2 = this->Find(right, true, find); // start of -->
				if (find2 >= 0) {
					// skip the material in between
					pos = find2 + right.Length();
				}
				else {
					pos = n;
				}
			}
			else {
				break;
			}
		}
		if ((n - pos) > 0)
			out += this->Mid(pos, n - pos);

		return out;
	};
	cweeStr				ReplaceBetween(const cweeStr& left, const cweeStr& right, const cweeStr& replaceWith) const {
		// i.e. 
		// left  =   <!-- 
		// right =   -->

		cweeStr out;
		int pos = 0;
		int n = this->Length();
		while (1) {
			if (pos >= n) break;

			int find = this->Find(left, true, pos);
			if (find >= 0) {
				if ((find - pos) > 0) out += this->Mid(pos, find - pos); // left of <!--

				int find2 = this->Find(right, true, find); // start of -->
				if (find2 >= 0) {
					// skip the material in between
					pos = find2 + right.Length();
					out += replaceWith;
				}
				else {
					pos = n;
				}
			}
			else {
				break;
			}
		}
		if ((n - pos) > 0)
			out += this->Mid(pos, n - pos);

		return out;
	};
	cweeStr				FindBetween(const cweeStr& left, const cweeStr& right) const {
		auto first = this->Find(left);
		auto last = this->rFind(right);
		if (first == -1 || last == -1 || first == last || last < first) {
			return *this;
		}
		else {
			return Mid(first + left.Length(), last - (first + left.Length()));
		}
	};
	cweeStr				FindFirstBetween(const cweeStr& left, const cweeStr& right) const {
		auto first = this->Find(left, true);
		auto last = this->Find(right, true, first + left.Length() + 1);
		if (first == -1 || last == -1 || first == last || last < first) {
			return *this;
		}
		else {
			return Mid(first + left.Length(), last - (first + left.Length()));
		}
	};

	size_t				Length() const {
		return len;
	};
	size_t				Allocated() const {
		if (data != baseBuffer) {
			return GetAlloced();
		}
		else {
			return 0;
		}
	};
	void				Empty() {
		EnsureAlloced(1);
		data[0] = '\0';
		len = 0;
	};
	bool				IsEmpty() const {
		return (cweeStr::Cmp(data, "") == 0);
	};
	void				Clear() {
		if (IsStatic()) {
			len = 0;
			data[0] = '\0';
			return;
		}
		FreeData();
		Construct();
	};
	cweeStr&			Append(const char a) {
		EnsureAlloced(len + 2);
		data[len] = a;
		len++;
		data[len] = '\0';
		return *this;
	};
	cweeStr&			Append(const cweeStr& text) {
		size_t newLen;
		size_t i;

		newLen = len + text.Length();
		EnsureAlloced(newLen + 1);
		for (i = 0; i < text.len; i++) {
			data[len + i] = text[i];
		}
		len = newLen;
		data[len] = '\0';
		return *this;
	};
	cweeStr&			Append(const char* text) {
		size_t newLen;
		size_t i;

		if (text) {
			newLen = len + strlen(text);
			EnsureAlloced(newLen + 1);
			for (i = 0; text[i]; i++) {
				data[len + i] = text[i];
			}
			len = newLen;
			data[len] = '\0';
		}
		return *this;
	};
	cweeStr&			Append(const char* text, long long l) {
		long long newLen; long long i;
		if (text && l) {
			newLen = len + l;
			EnsureAlloced(newLen + 1);
			//for (; i >= 0; i--) {
			//	data[len + i] = text[i];
			//}

			for (i = 0; text[i] && i < l; i++) {
				data[len + i] = text[i];
			}

			len = newLen;
			data[len] = '\0';
		}
		return *this;
	};
	void				Insert(const char a, long long index) {
		long long i, l;

		if (index < 0) {
			index = 0;
		}
		else if (index > (long long)len) {
			index = len;
		}

		l = 1;
		EnsureAlloced(len + l + 1);
		for (i = len; i >= index; i--) {
			data[i + l] = data[i];
		}
		data[index] = a;
		len++;
	};
	void				Insert(const char* text, long long index) {
		long long i, l;

		if (index < 0) {
			index = 0;
		}
		else if (index > (long long)len) {
			index = len;
		}

		l = strlen(text);
		EnsureAlloced(len + l + 1);
		for (i = len; i >= index; i--) {
			data[i + l] = data[i];
		}
		for (i = 0; i < l; i++) {
			data[index + i] = text[i];
		}
		len += l;
	};
	void				ToLower() {
		for (size_t i = 0; data[i]; i++) {
			if (CharIsUpper(data[i])) {
				data[i] += ('a' - 'A');
			}
		}
	};
	void				ToUpper() {
		for (size_t i = 0; data[i]; i++) {
			if (CharIsLower(data[i])) {
				data[i] -= ('a' - 'A');
			}
		}
	};
	bool				IsNumeric() const {
		return cweeStr::IsNumeric(data);
	};
	bool				HasLower() const {
		return cweeStr::HasLower(data);
	};
	bool				HasUpper() const {
		return cweeStr::HasUpper(data);
	};
	void				CapLength(size_t newlen) {
		if (len <= newlen) {
			return;
		}
		data[newlen] = 0;
		len = newlen;
	};
	void				Fill(const char ch, size_t newlen) {
		EnsureAlloced(newlen + 1);
		len = newlen;
		memset(data, ch, len);
		data[len] = 0;
	};

	long long			Find(const char c, long long start = 0, long long end = -1) const {
		if (end == -1) {
			end = len;
		}
		return cweeStr::FindChar(data, c, start, end);
	};
	long long			Find(const char* text, bool casesensitive = true, long long start = 0, long long end = -1) const {
		if (end == -1) {
			end = len;
		}
		if (std::strlen(text) == 0 || this->Length() == 0) return -1;

		return cweeStr::FindText(data, text, casesensitive, start, end);
	};
	long long           Find(bpstd::string_view const& viewRef, bool casesensitive = true, long long start = 0, long long end = -1) const {
		if (end == -1) {
			end = len;
		}
		if (viewRef.length() == 0 || this->Length() == 0) return -1;

		return cweeStr::FindText(data, viewRef, casesensitive, start, end);
	};
	long long			rFind(const char* text, bool casesensitive = true, long long start = 0, long long end = -1) const {
		if (std::strlen(text) == 0 || this->Length() == 0) return -1;

		long long found = Find(text, casesensitive, start, end);
		long long prevFound = found;
		long long l = ::Max((size_t)1, std::strlen(text));
		while (found != -1) {
			prevFound = found;
			found = Find(text, casesensitive, prevFound + l, end); // if fails to find (-1) then returns the last good find
		}
		return prevFound;
	};
	bool				Filter(const char* filter, bool casesensitive) const {
		return cweeStr::Filter(filter, data, casesensitive);
	};
	long long			Last(const char c) const {
		long long i;
		for (i = Length(); i > 0; i--) {
			if (data[i - 1] == c) {
				return i - 1;
			}
		}
		return -1;
	};						// return the index to the last occurance of 'c', returns -1 if not found
	const char*			Left(size_t len, cweeStr& result) const {
		return Mid(0, len, result);
	};			// store the leftmost 'len' characters in the result
	const char*			Right(size_t len, cweeStr& result) const {
		if (len >= Length()) {
			result = *this;
			return result;
		}
		return Mid(Length() - len, len, result);
	};			// store the rightmost 'len' characters in the result
	const char*			Mid(long long start, long long l, cweeStr& result) const {
		long long i = Length();

		if (i == 0 || l <= 0 || start >= i) {
			result.Empty();
			return NULL;
		}

		if (start + l >= i) {
			l = i - start;
		}

		cweeStr temp;
		result = temp.Append(&data[start], l);
		return result;
	};	// store 'len' characters starting at 'start' in result
	cweeStr				Left(size_t len) const {
		return Mid(0, len);
	};							// return the leftmost 'len' characters
	cweeStr				Right(size_t len) const {
		if (len >= Length()) {
			return *this;
		}
		return Mid(Length() - len, len);
	};							// return the rightmost 'len' characters
	cweeStr				Mid(long long start, long long l) const {
		long long i = Length();
		cweeStr result;

		if (i == 0 || l <= 0 || start >= i) {
			return result;
		}

		if (start + l >= i) {
			l = i - start;
		}

		return result.Append(&data[start], l);
	};				// return 'len' characters starting at 'start'
	bpstd::string_view	Mid_Ref(long long start, long long l) const {
		long long i = Length();

		// bpstd::string_view out;
		if (i == 0 || l <= 0 || start >= i) {
			return bpstd::string_view();
		}

		if (start + l >= i) {
			l = i - start;
		}

		return bpstd::string_view(&data[start], l);
		// out.mid(0, l);

		// return out;
	};				// return 'len' characters starting at 'start'
	bool				StartsWith(const cweeStr& startsWith) const {
		if (startsWith.Length() <= 0) return true;
		if (this->Length() >= startsWith.Length()) {
			if (this->Left(startsWith.Length()) == startsWith) {
				return true;
			}
		}
		return false;
	};
	bool				iStartsWith(const cweeStr& startsWith) const {
		if (startsWith.Length() <= 0) return true;
		if (this->Length() >= startsWith.Length()) {
			if (this->Left(startsWith.Length()).Icmp(startsWith)==0) {
				return true;
			}
		}
		return false;
	};
	void				ReduceSpaces(bool keepNewLine = false) {
		//ReplaceChar('\t', ' '); // may be worth to keep tabs...
		if (!keepNewLine) ReplaceChar('\n', ' ');
		StripLeading(' ');
		StripTrailing(' ');
		while (Find("  ") >= 0) {
			// Replace(cweeStr("  ").c_str(), cweeStr(" ").c_str());
			Replace("  ", " ");
		}
	};								// strip \t, \n, and ' ' from string until only one ' ' is between each word, and there is no trailing/leading spaces
	void				StripLeading(const char c) {
		while (data[0] == c) {
			memmove(&data[0], &data[1], len);
			len--;
		}
	};					// strip char from front as many times as the char occurs
	void				StripLeading(const char* string) {
		long long l;

		l = strlen(string);
		if (l > 0) {
			while (!Cmpn(string, l)) {
				memmove(data, data + l, len - l + 1);
				len -= l;
			}
		}
	};			// strip string from front as many times as the string occurs
	bool				StripLeadingOnce(const char* string) {
		long long l;

		l = strlen(string);
		if ((l > 0) && !Cmpn(string, l)) {
			memmove(data, data + l, len - l + 1);
			len -= l;
			return true;
		}
		return false;
	};		// strip string from front just once if it occurs
	void				StripTrailing(const char c) {
		long long i;

		for (i = Length(); i > 0 && data[i - 1] == c; i--) {
			data[i - 1] = '\0';
			len--;
		}
	};				// strip char from end as many times as the char occurs
	void				StripTrailing(const char* string) {
		long long l;

		l = strlen(string);
		if (l > 0) {
			while (((long long)len >= l) && !Cmpn(string, data + len - l, l)) {
				len -= l;
				data[len] = '\0';
			}
		}
	};			// strip string from end as many times as the string occurs
	bool				StripTrailingOnce(const char* string) {
		long long l;

		l = strlen(string);
		if ((l > 0) && ((long long)len >= l) && !Cmpn(string, data + len - l, l)) {
			len -= l;
			data[len] = '\0';
			return true;
		}
		return false;
	};		// strip string from end just once if it occurs
	cweeStr&				Strip(const char c) {
		StripLeading(c);
		StripTrailing(c);
		return *this;
	};						// strip char from front and end as many times as the char occurs
	cweeStr&				Strip(const char* string) {
		StripLeading(string);
		StripTrailing(string);
		return *this;
	};					// strip string from front and end as many times as the string occurs
	void				StripOnce(const char* string) {
		StripLeadingOnce(string);
		StripTrailingOnce(string);
	};					// strip string from front and end as many times as the string occurs
	void				StripTrailingWhitespace() {
		long long i;

		// cast to unsigned char to prevent stripping off high-ASCII characters
		for (i = Length(); i > 0 && (unsigned char)(data[i - 1]) <= ' '; i--) {
			data[i - 1] = '\0';
			len--;
		}
	};					// strip trailing white space characters
	cweeStr&			StripQuotes()
	{
		if (data[0] != '\"')
		{
			return *this;
		}

		// Remove the trailing quote first
		if (data[len - 1] == '\"')
		{
			data[len - 1] = '\0';
			len--;
		}

		// Strip the leading quote now
		len--;
		memmove(&data[0], &data[1], len);
		data[len] = '\0';

		return *this;
	};								// strip quotes around string
	bool				Replace(const char* old, const char* nw) {
		long long oldLen = strlen(old);
		long long newLen = strlen(nw);
		long long prevlength = Length();

		if (oldLen <= 0) return false;

		if (newLen <= 2 || oldLen <= 2 || Length() < 1000) {

			long long count = 0;
			for (long long i = 0; i < prevlength; i++) {
				if (cweeStr::Cmpn(&data[(size_t)i], old, oldLen) == 0) {
					count++;
					i += oldLen - 1;
				}
				else if (((i + oldLen) < prevlength) && (data[(size_t)(i + oldLen)] != old[(size_t)(oldLen - 1)])) {
					i++;
				}
			}
			if (count) {
				cweeStr oldString(data);

				EnsureAlloced(len + ((newLen - oldLen) * count) + 2, false);

				// Replace the old data with the new data
				size_t j = 0;
				for (long long i = 0; i < (long long)oldString.Length(); i++) {
					if (cweeStr::Cmpn(&oldString[(size_t)i], old, oldLen) == 0) {
						memcpy(data + j, nw, newLen);
						i += oldLen - 1;
						j += newLen;
					}
					else {
						data[j] = oldString[(size_t)i];
						j++;
					}
				}
				data[j] = 0;
				len = strlen(data);
				return true;
			}

		}
		else {

			long long capacity = 16;
			std::vector<long long> found;
			found.reserve(capacity);
			long long count = 0;
			for (long long i = 0; i < prevlength; i++) {
				if (cweeStr::Cmpn(&data[i], old, oldLen) == 0) {
					if (capacity <= ++count) {
						capacity *= 7;
						found.reserve(capacity);
					}
					found.push_back(i);
					//count++;
					i += oldLen - 1;
				}
				else if (((i + oldLen) < prevlength) && (data[i + oldLen] != old[oldLen - 1])) {
					i++;
				}
			}
			if (count) {
				// inline replace without copying the data. 
				long long finalLen = len + ((newLen - oldLen) * count);
				if (finalLen > (long long)len)
				{
					EnsureAlloced(finalLen + 2, true); // data[] is now the size of "finalLen + 2"
					long long diff = finalLen - len;
					long long i, j;
					for (i = finalLen; i >= diff; i--) data[i] = data[i - diff]; // move all of our data to the right-most edge.

					j = 0; i = 0;
					for (auto& find : found) {
						if (find > i) {
							// move previous content
							memcpy(data + j, data + (i + diff), find - i);
							j += (find - i);
							i = find + oldLen;
						}
						// move new content
						{
							while (j + newLen >= finalLen) {
								EnsureAlloced(j + newLen + 2, true);
								finalLen = j + newLen + 2;
							}
							memcpy(data + j, nw, newLen);
							j += newLen;
						}
					}
					// move the remaining text 
					if (prevlength >= i) {
						// move previous
						memcpy(data + j, data + (i + diff), (prevlength - i) + 1);
						j += ((prevlength - i) + 1);
					}
					data[j] = 0;
					len = strlen(data);
					return true;
				}
				else {
					long long buffer = ::Max(oldLen, newLen) - ::Min((long long)0.0f, finalLen - (long long)len);
					EnsureAlloced(finalLen + 2 + buffer, true); // data[] is now the size of "finalLen + 2"					
					long long diff = (finalLen + buffer) - len;
					long long i, j;
					for (i = finalLen + buffer; i >= diff; i--) data[i] = data[i - diff]; // move all of our data to the right-most edge. 

					j = 0; i = 0;
					for (auto& find : found) {
						if (find > i) {
							// move previous content
							memcpy(data + j, data + (i + diff), find - i);
							j += (find - i);
							i = find + oldLen;
						}
						// move new content
						{
							while (j + newLen >= (finalLen + buffer)) {
								EnsureAlloced(j + newLen + 2, true);
								finalLen = j + newLen + 2;
							}
							memcpy(data + j, nw, newLen);
							j += newLen;
						}
					}
					// move the remaining text 
					if (prevlength >= i) {
						// move previous
						memcpy(data + j, data + (i + diff), (prevlength - i) + 1);
						j += ((prevlength - i) + 1);
					}
					data[j] = 0;
					len = strlen(data);
					return true;
				}
			}
		}

		return false;
	};
	bool				ReplaceChar(const char old, const char nw) {
		bool replaced = false;
		for (long long i = 0; i < (long long)Length(); i++) {
			if (data[i] == old) {
				data[i] = nw;
				replaced = true;
			}
		}
		return replaced;
	};
	void				Replace(const std::vector<cweeStr>& olds, const std::vector<cweeStr>& news) {
		long long numOlds = olds.size();
		std::vector<long long> counts; for (size_t c = 0; c < olds.size(); c++){ counts.push_back(0); } 
		size_t overall = 0;
		long long oldLen = 0, newLen = 0;
		long long i = 0, j = 0, k = 0;

		// Work out how big the new string will be
		for (i = 0; i < (long long)Length(); i++) {
			for (k = 0; k < numOlds; k++) {
				if (cweeStr::Cmpn(&data[i], olds[k], olds[k].Length()) == 0) {
					counts[k]++;
					overall++;
					i += olds[k].Length() - 1;
					break; // early exit to the first for-loop
				}
			}
		}


		if (overall > 0) {
			oldLen = len;

			// inline replace without copying the data. 
			long long finalLen = len;
			for (long long k = 0; k < numOlds; k++) {
				finalLen += ((((long long)news[k].Length()) - ((long long)olds[k].Length())) * counts[k]);
			}

			if (finalLen > oldLen) {
				EnsureAlloced(finalLen + 2, true);
				long long diff = finalLen - oldLen;

				// data[] is now the size of "finalLen + 2"
				// move all of our data to the right-most edge. 
				for (i = finalLen; i >= diff; i--) data[i] = data[i - diff];
				// for (; i >= 0; i--) data[i] = ' ';
				// "old\0" 
				// "   old\0"

				j = 0;
				// Replace the old data with the new data
				for (i = diff; i < finalLen; i++) {
					bool found = false;
					for (k = 0; k < numOlds; k++) { // for each possible replacement
						oldLen = olds[k].Length();
						if (cweeStr::Cmpn(&data[i], olds[k], oldLen) == 0) {
							newLen = news[k].Length();
							found = true;
							while (j + newLen >= finalLen) {
								EnsureAlloced(j + newLen + 2, true);
								finalLen = j + newLen + 2;
							}

							memcpy(data + j, news[k], newLen);

							i += oldLen - 1;
							j += newLen;
							break; // exit to i-based for-loop
						}
					}

					if (!found) {
						data[j] = data[i];
						j++;
					}
				}
				data[j] = 0;
				len = strlen(data);
			}
			else {
				// determine buffer size
				long long buffer = 0;
				for (k = 0; k < numOlds; k++) {
					buffer = ::Max(::Max(buffer, (long long)olds[k].Length()), (long long)news[k].Length());
				}
				buffer -= ::Min((long long)0.0f, finalLen - oldLen);

				EnsureAlloced(finalLen + 2 + buffer, true);
				long long diff = (finalLen + buffer) - oldLen;

				// data[] is now the size of "finalLen + 2"
				// move all of our data to the right-most edge. 
				for (i = (finalLen + buffer); i >= diff; i--) data[i] = data[i - diff];
				// for (; i >= 0; i--) data[i] = ' ';
				// "old\0" 
				// "   old\0"

				j = 0;
				// Replace the old data with the new data
				for (i = diff; i < (finalLen + buffer); i++) {
					bool found = false;
					for (k = 0; k < numOlds; k++) { // for each possible replacement
						oldLen = olds[k].Length();
						if (cweeStr::Cmpn(&data[i], olds[k], oldLen) == 0) {
							newLen = news[k].Length();
							found = true;
							while (j + newLen >= (finalLen + buffer)) {
								EnsureAlloced(j + newLen + 2 + buffer, true);
								finalLen = j + newLen + 2;
							}

							memcpy(data + j, news[k], newLen);

							i += oldLen - 1;
							j += newLen;
							break; // exit to i-based for-loop
						}
					}

					if (!found) {
						data[j] = data[i];
						j++;
					}
				}
				data[j] = 0;
				len = strlen(data);
			}
		}
	};
	void				CopyRange(const char* text, long long start, long long end) {

		long long endStr = end;
		if (endStr <= 0)
			endStr = cweeStr(text).Length();

		long long l = endStr - start;
		if (l < 0) {
			l = 0;
		}

		EnsureAlloced(l + 1);

		for (long long i = 0; i < l; i++) {
			data[i] = text[start + i];
		}

		data[l] = '\0';
		len = l;
	};

	// file name methods
	cweeStr&			BackSlashesToSlashes() {
		size_t i;

		for (i = 0; i < len; i++) {
			if (data[i] == '\\') {
				data[i] = '/';
			}
		}
		return *this;
	};					// convert slashes
	cweeStr&			SlashesToBackSlashes() {
		size_t i;

		for (i = 0; i < len; i++) {
			if (data[i] == '/') {
				data[i] = '\\';
			}
		}
		return *this;
	};					// convert slashes
	cweeStr&			SetFileExtension(const char* extension) {
		StripFileExtension();
		if (*extension != '.') {
			Append('.');
		}
		Append(extension);
		return *this;
	};		// set the given file extension
	cweeStr&			StripFileExtension() {
		long long i;

		for (i = ((long long)len) - 1; i >= 0; i--) {
			if (data[i] == '.') {
				data[i] = '\0';
				len = i;
				break;
			}
		}
		return *this;
	};						// remove any file extension
	cweeStr&			StripAbsoluteFileExtension() {
		long long i;

		for (i = 0; i < (long long)len; i++) {
			if (data[i] == '.') {
				data[i] = '\0';
				len = i;
				break;
			}
		}

		return *this;
	};				// remove any file extension looking from front (useful if there are multiple .'s)
	cweeStr&			DefaultFileExtension(const char* extension) {
		long long i;

		// do nothing if the string already has an extension
		for (i = ((long long)len) - 1; i >= 0; i--) {
			if (data[i] == '.') {
				return *this;
			}
		}
		if (*extension != '.') {
			Append('.');
		}
		Append(extension);
		return *this;
	};	// if there's no file extension use the default
	cweeStr&			DefaultPath(const char* basepath) {
		if ((this->operator [](0) == '/') || (this->operator [](0) == '\\')) {
			// absolute path location
			return *this;
		}

		*this = basepath + *this;
		return *this;
	};			// if there's no path use the default
	void				AppendPath(const char* text) {
		size_t pos;
		size_t i = 0;

		if (text && text[i]) {
			pos = len;
			EnsureAlloced(len + strlen(text) + 2);

			if (pos) {
				if (data[pos - 1] != '/') {
					data[pos++] = '/';
				}
			}
			if (text[i] == '/') {
				i++;
			}

			for (; text[i]; i++) {
				if (text[i] == '\\') {
					data[pos++] = '/';
				}
				else {
					data[pos++] = text[i];
				}
			}
			len = pos;
			data[pos] = '\0';
		}
	};					// append a partial path
	cweeStr&			StripFilename() {
		int pos;

		pos = Length() - 1;
		while ((pos > 0) && (this->operator [](pos) != '/') && (this->operator [](pos) != '\\')) {
			pos--;
		}

		if (pos < 0) {
			pos = 0;
		}

		CapLength(pos);
		return *this;
	};							// remove the filename from a path
	cweeStr&			StripPath() {
		int pos;

		pos = Length();
		while ((pos > 0) && (this->operator [](pos-1) != '/') && (this->operator [](pos-1) != '\\')) {
			pos--;
		}

		*this = Right(Length() - pos);
		return *this;
	};								// remove the path from the filename
	void				ExtractFilePath(cweeStr& dest) const {
		int pos;

		//
		// back up until a \ or the start
		//
		pos = Length();
		while ((pos > 0) && (this->operator [](pos - 1) != '/') && (this->operator [](pos - 1) != '\\')) {
			pos--;
		}

		Left(pos, dest);
	};			// copy the file path to another string
	void				ExtractFileName(cweeStr& dest) const {
		int pos;

		//
		// back up until a \ or the start
		//
		pos = Length() - 1;
		while ((pos > 0) && (this->operator [](pos - 1) != '/') && (this->operator [](pos - 1) != '\\')) {
			pos--;
		}

		Right(Length() - pos, dest);
	};			// copy the filename to another string
	void				ExtractFileBase(cweeStr& dest) const {
		int pos;
		long long start;

		//
		// back up until a \ or the start
		//
		pos = Length() - 1;
		while ((pos > 0) && (this->operator [](pos - 1) != '/') && (this->operator [](pos - 1) != '\\')) {
			pos--;
		}

		start = pos;
		while ((pos < (long long)Length()) && (this->operator [](pos) != '.')) {
			pos++;
		}

		Mid(start, pos - start, dest);
	};			// copy the filename minus the extension to another string
	void				ExtractFileExtension(cweeStr& dest) const {
		int pos;

		//
		// back up until a . or the start
		//
		pos = Length() - 1;
		while ((pos > 0) && (this->operator [](pos - 1) != '.')) {
			pos--;
		}

		if (!pos) {
			// no extension
			dest.Empty();
		}
		else {
			Right(Length() - pos, dest);
		}
	};		// copy the file extension to another string
	bool				CheckExtension(const char* ext) {
		return cweeStr::CheckExtension(data, ext);
	};

	float				ReturnNumeric() const {
		return static_cast<float>(ReturnNumericD());
	};
	double				ReturnNumericD() const {
		const char* p = c_str();
		double r = 0.0;
		if (this->len > 0 && p) {
			bool neg = false;
			if (*p == '-') {
				neg = true;
				++p;
			}
			while (*p >= '0' && *p <= '9') {
				r = (r * 10.0) + (*p - '0');
				++p;
			}
			if (*p == '.') {
				double f = 0.0;
				int n = 0;
				++p;
				while (*p >= '0' && *p <= '9') {
					f = (f * 10.0) + (*p - '0');
					++p;
					++n;
				}
				r += f / std::pow(10.0, n);
			}
			if (neg) {
				r = -r;
			}
		}
		return r;

		// return atof(c_str());
	};
	static std::wstring	toWideString(cweeStr in);
	static cweeStr		fromTime(time_t in);
	cweeStr& AddToDelimiter(cweeStr& in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		if ((out.len + in.len) > out.GetAlloced()) {
			out.EnsureAlloced((size_t)((len + in.len) * 1.5f), true);
		}
		out.Append(in);
		return out;
	};
	cweeStr& AddToDelimiter(const char* in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(in);
		return out;
	};
	cweeStr& AddToDelimiter(char in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr(in));
		return out;
	};
	cweeStr& AddToDelimiter(int in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((int)in));
		return out;
	};
	cweeStr& AddToDelimiter(float in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr(in));
		return out;
	};
	cweeStr& AddToDelimiter(bool in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr(in));
		return out;
	};
	cweeStr& AddToDelimiter(double in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((float)in));
		return out;
	};
	cweeStr& AddToDelimiter(time_t in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((float)in));
		return out;
	};
	cweeStr& AddToDelimiter(u64 in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((float)in));
		return out;
	};

	cweeStr& AddToDelimiter(cweeStr& in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		if ((out.len + in.len) > out.GetAlloced()) {
			out.EnsureAlloced((size_t)((len + in.len) * 1.5f), true);
		}
		out.Append(in);
		return out;
	};
	cweeStr& AddToDelimiter(const char* in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr(in));
		return out;
	};
	cweeStr& AddToDelimiter(char in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr(in));
		return out;
	};
	cweeStr& AddToDelimiter(int in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((int)in));
		return out;
	};
	cweeStr& AddToDelimiter(float in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr(in));
		return out;
	};
	cweeStr& AddToDelimiter(bool in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((int)in));
		return out;
	};
	cweeStr& AddToDelimiter(double in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((float)in));
		return out;
	};
	cweeStr& AddToDelimiter(time_t in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((float)in));
		return out;
	};
	cweeStr& AddToDelimiter(u64 in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((float)in));
		return out;
	};


	// char * methods to replace library functions
	static size_t		Length(const char* s) {
		size_t i;
		for (i = 0; s[i]; i++) {}
		return i;
	};
	static char*		ToLower(char* s) {
		for (size_t i = 0; s[i]; i++) {
			if (CharIsUpper(s[i])) {
				s[i] += ('a' - 'A');
			}
		}
		return s;
	};
	static char*		ToUpper(char* s) {
		for (size_t i = 0; s[i]; i++) {
			if (CharIsLower(s[i])) {
				s[i] -= ('a' - 'A');
			}
		}
		return s;
	};
	static bool			IsNumeric(const char* s) {
		size_t		i;
		bool	dot, finalSpace;

		if (*s == '-') {
			s++;
		}

		dot = false;
		finalSpace = false;
		for (i = 0; s[i]; i++) {
			if (!isdigit((const unsigned char)s[i])) {
				if (s[i] == ' ') {
					finalSpace = true;
					continue;
				}
				else {
					if (finalSpace)
						return false;
				}
				if ((s[i] == '.') && !dot) {
					dot = true;
					continue;
				}
				return false;
			}
		}
		return true;
	};
	static bool			HasLower(const char* s) {
		if (!s) {
			return false;
		}

		while (*s) {
			if (CharIsLower(*s)) {
				return true;
			}
			s++;
		}

		return false;
	};
	static bool			HasUpper(const char* s) {
		if (!s) {
			return false;
		}

		while (*s) {
			if (CharIsUpper(*s)) {
				return true;
			}
			s++;
		}

		return false;
	};
	static long long	Cmp(const char* s1, const char* s2) {
		long long c1, c2, d;
		if (!s1 || !s2) return 0;
		do {
			c1 = *s1++;
			c2 = *s2++;

			d = c1 - c2;
			if (d) {
				return (INT32_SIGNBITNOTSET(d) << 1) - 1;
			}
		} while (c1);

		return 0;		// strings are equal
	};
	static long long	Cmpn(const char* s1, const char* s2, long long n) {
		long long c1, c2, d;

		assert(n >= 0);
		if (!s1 || !s2) return 0;
		do {
			c1 = *s1++;
			c2 = *s2++;

			if (!n--) {
				return 0;		// strings are equal until end posize_t
			}

			d = c1 - c2;
			if (d) {
				return (INT32_SIGNBITNOTSET(d) << 1) - 1;
			}
		} while (c1);

		return 0;		// strings are equal
	};
	static long long	Icmp(const char* s1, const char* s2) {
		long long c1, c2, d;
		if (!s1 || !s2) return 0;
		do {
			c1 = *s1++;
			c2 = *s2++;

			d = c1 - c2;
			while (d) {
				if (c1 <= 'Z' && c1 >= 'A') {
					d += ('a' - 'A');
					if (!d) {
						break;
					}
				}
				if (c2 <= 'Z' && c2 >= 'A') {
					d -= ('a' - 'A');
					if (!d) {
						break;
					}
				}
				return (INT32_SIGNBITNOTSET(d) << 1) - 1;
			}
		} while (c1);

		return 0;		// strings are equal
	};
	static long long	Icmpn(const char* s1, const char* s2, long long n) {
		long long c1, c2, d;

		assert(n >= 0);
		if (!s1 || !s2) return 0;
		do {
			c1 = *s1++;
			c2 = *s2++;

			if (!n--) {
				return 0;		// strings are equal until end posize_t
			}

			d = c1 - c2;
			while (d) {
				if (c1 <= 'Z' && c1 >= 'A') {
					d += ('a' - 'A');
					if (!d) {
						break;
					}
				}
				if (c2 <= 'Z' && c2 >= 'A') {
					d -= ('a' - 'A');
					if (!d) {
						break;
					}
				}
				return (INT32_SIGNBITNOTSET(d) << 1) - 1;
			}
		} while (c1);

		return 0;		// strings are equal
	};
	static long long	IcmpPath(const char* s1, const char* s2) {
		long long c1, c2, d;

		if (!s1 || !s2) return 0;
		do {
			c1 = *s1++;
			c2 = *s2++;

			d = c1 - c2;
			while (d) {
				if (c1 <= 'Z' && c1 >= 'A') {
					d += ('a' - 'A');
					if (!d) {
						break;
					}
				}
				if (c1 == '\\') {
					d += ('/' - '\\');
					if (!d) {
						break;
					}
				}
				if (c2 <= 'Z' && c2 >= 'A') {
					d -= ('a' - 'A');
					if (!d) {
						break;
					}
				}
				if (c2 == '\\') {
					d -= ('/' - '\\');
					if (!d) {
						break;
					}
				}
				// make sure folders come first
				while (c1) {
					if (c1 == '/' || c1 == '\\') {
						break;
					}
					c1 = *s1++;
				}
				while (c2) {
					if (c2 == '/' || c2 == '\\') {
						break;
					}
					c2 = *s2++;
				}
				if (c1 && !c2) {
					return -1;
				}
				else if (!c1 && c2) {
					return 1;
				}
				// same folder depth so use the regular compare
				return (INT32_SIGNBITNOTSET(d) << 1) - 1;
			}
		} while (c1);

		return 0;
	};			// compares paths and makes sure folders come first
	static long long	IcmpnPath(const char* s1, const char* s2, long long n) {
		long long c1, c2, d;

		assert(n >= 0);
		if (!s1 || !s2) return 0;
		do {
			c1 = *s1++;
			c2 = *s2++;

			if (!n--) {
				return 0;		// strings are equal until end posize_t
			}

			d = c1 - c2;
			while (d) {
				if (c1 <= 'Z' && c1 >= 'A') {
					d += ('a' - 'A');
					if (!d) {
						break;
					}
				}
				if (c1 == '\\') {
					d += ('/' - '\\');
					if (!d) {
						break;
					}
				}
				if (c2 <= 'Z' && c2 >= 'A') {
					d -= ('a' - 'A');
					if (!d) {
						break;
					}
				}
				if (c2 == '\\') {
					d -= ('/' - '\\');
					if (!d) {
						break;
					}
				}
				// make sure folders come first
				while (c1) {
					if (c1 == '/' || c1 == '\\') {
						break;
					}
					c1 = *s1++;
				}
				while (c2) {
					if (c2 == '/' || c2 == '\\') {
						break;
					}
					c2 = *s2++;
				}
				if (c1 && !c2) {
					return -1;
				}
				else if (!c1 && c2) {
					return 1;
				}
				// same folder depth so use the regular compare
				return (INT32_SIGNBITNOTSET(d) << 1) - 1;
			}
		} while (c1);

		return 0;
	};	// compares paths and makes sure folders come first
	static void			Copynz(char* dest, const char* src, size_t destsize) {
		if (!src) {
			return;
		}
		if (destsize < 1) {
			return;
		}

		strncpy(dest, src, destsize - 1);
		dest[destsize - 1] = 0;
	};
	static size_t		vsnPrintf(char* dest, size_t size, const char* fmt, va_list argptr) {
		size_t ret;
#undef _vsnprintf
		ret = _vsnprintf(dest, size - 1, fmt, argptr);
#define _vsnprintf	use_cweeStr_vsnPrintf
		dest[size - 1] = '\0';
		if (ret < 0 || ret >= size) {
			return -1;
		}
		return ret;
	};
	static long long	FindChar(const char* str, const char c, long long start = 0, long long end = -1) {
		long long i;

		if (end == -1) {
			end = strlen(str) - 1;
		}
		for (i = start; i <= end; i++) {
			if (str[i] == c) {
				return i;
			}
		}
		return -1;
	};
	static long long	FindText(const char* str, const char* text, bool casesensitive = true, long long start = 0, long long end = -1) {

		long long l, j, k;
		k = strlen(text);
		if (end == -1) {
			end = strlen(str);
		}
		l = end - k;

		if (k <= 0 || (l - start) < 0) return -1;

		// if (text == str) { return 0; }

		if (casesensitive) {
			const char sample = text[0];
			if (!sample) {
				return start;
			}
			for (; start <= l; start++) { // starting at the search position ... 
				if (str[start] == sample) { // found a match for the first character ...
					for (j = 1; ; j++) { // for the remaining parts of the search text ... 
						if (!text[j]) { // we reached the end of the search text without breaking -- therefore success.
							return start;
						}
						else {
							if (str[start + j] != text[j]) {
								break;
							}
						}
					}
				}
			}
		}
		else {
			for (; start <= l; start++)
				for (j = 0;; j++) {
					if (!text[j]) return start;
					if (::toupper(str[start + j]) != ::toupper(text[j]))
						break;
				}
		}
		return -1;

	};
	static long long	FindText(const char* str, bpstd::string_view const& text, bool casesensitive = true, long long start = 0, long long end = -1) {
		long long l, j, k;
		k = text.length();
		if (end == -1) {
			end = strlen(str);
		}
		l = end - k;

		if (k <= 0 || (l - start) < 0) return -1;

		if (casesensitive) {
			const char sample = text[0];
			if (!sample) {
				return start;
			}
			for (; start <= l; start++) { // starting at the search position ... 
				if (str[start] == sample) { // found a match for the first character ...
					for (j = 1; ; j++) { // for the remaining parts of the search text ... 
						if (j >= k) return start;
						else {
							if (str[start + j] != text[j]) {
								break;
							}
						}
					}
				}
			}
		}
		else {
			for (; start <= l; start++)
				for (j = 0;; j++) {
					if (j >= k) return start;
					if (::toupper(str[start + j]) != ::toupper(text[j]))
						break;
				}
		}
		return -1;
	};

	static bool			Filter(const char* filter, const char* name, bool casesensitive) {
		cweeStr buf;
		long long i, found, index;

		while (*filter) {
			if (*filter == '*') {
				filter++;
				buf.Empty();
				for (i = 0; *filter; i++) {
					if (*filter == '*' || *filter == '?' || (*filter == '[' && *(filter + 1) != '[')) {
						break;
					}
					buf += *filter;
					if (*filter == '[') {
						filter++;
					}
					filter++;
				}
				if (buf.Length()) {
					index = cweeStr(name).Find(buf.c_str(), casesensitive);
					if (index == -1) {
						return false;
					}
					name += index + strlen(buf);
				}
			}
			else if (*filter == '?') {
				filter++;
				name++;
			}
			else if (*filter == '[') {
				if (*(filter + 1) == '[') {
					if (*name != '[') {
						return false;
					}
					filter += 2;
					name++;
				}
				else {
					filter++;
					found = false;
					while (*filter && !found) {
						if (*filter == ']' && *(filter + 1) != ']') {
							break;
						}
						if (*(filter + 1) == '-' && *(filter + 2) && (*(filter + 2) != ']' || *(filter + 3) == ']')) {
							if (casesensitive) {
								if (*name >= *filter && *name <= *(filter + 2)) {
									found = true;
								}
							}
							else {
								if (::toupper(*name) >= ::toupper(*filter) && ::toupper(*name) <= ::toupper(*(filter + 2))) {
									found = true;
								}
							}
							filter += 3;
						}
						else {
							if (casesensitive) {
								if (*filter == *name) {
									found = true;
								}
							}
							else {
								if (::toupper(*filter) == ::toupper(*name)) {
									found = true;
								}
							}
							filter++;
						}
					}
					if (!found) {
						return false;
					}
					while (*filter) {
						if (*filter == ']' && *(filter + 1) != ']') {
							break;
						}
						filter++;
					}
					filter++;
					name++;
				}
			}
			else {
				if (casesensitive) {
					if (*filter != *name) {
						return false;
					}
				}
				else {
					if (::toupper(*filter) != ::toupper(*name)) {
						return false;
					}
				}
				filter++;
				name++;
			}
		}
		return true;
	};
	static void			StripMediaName(const char* name, cweeStr& mediaName) {
		char c;

		mediaName.Empty();

		for (c = *name; c; c = *(++name)) {
			// truncate at an extension
			if (c == '.') {
				break;
			}
			// convert backslashes to forward slashes
			if (c == '\\') {
				mediaName.Append('/');
			}
			else {
				mediaName.Append(cweeStr::ToLower(c));
			}
		}
	};
	static bool			CheckExtension(const char* name, const char* ext) {
		const char* s1 = name + Length(name) - 1;
		const char* s2 = ext + Length(ext) - 1;
		size_t c1, c2, d;

		do {
			c1 = *s1--;
			c2 = *s2--;

			d = c1 - c2;
			while (d) {
				if (c1 <= 'Z' && c1 >= 'A') {
					d += ('a' - 'A');
					if (!d) {
						break;
					}
				}
				if (c2 <= 'Z' && c2 >= 'A') {
					d -= ('a' - 'A');
					if (!d) {
						break;
					}
				}
				return false;
			}
		} while (s1 > name && s2 > ext);

		return (s1 >= name);
	};
	static const char*	CStyleQuote(const char* str) {
		static size_t index = 0;
		static char buffers[4][16384];	// in case called by nested functions
		unsigned int i;
		char* buf;

		buf = buffers[index];
		index = (index + 1) & 3;

		buf[0] = '\"';
		for (i = 1; i < sizeof(buffers[0]) - 2; i++) {
			size_t c = *str++;
			switch (c) {
			case '\0': buf[i++] = '\"'; buf[i] = '\0'; return buf;
			case '\\': buf[i++] = '\\'; buf[i] = '\\'; break;
			case '\n': buf[i++] = '\\'; buf[i] = 'n'; break;
			case '\r': buf[i++] = '\\'; buf[i] = 'r'; break;
			case '\t': buf[i++] = '\\'; buf[i] = 't'; break;
			case '\v': buf[i++] = '\\'; buf[i] = 'v'; break;
			case '\b': buf[i++] = '\\'; buf[i] = 'b'; break;
			case '\f': buf[i++] = '\\'; buf[i] = 'f'; break;
			case '\a': buf[i++] = '\\'; buf[i] = 'a'; break;
			case '\'': buf[i++] = '\\'; buf[i] = '\''; break;
			case '\"': buf[i++] = '\\'; buf[i] = '\"'; break;
			case '\?': buf[i++] = '\\'; buf[i] = '\?'; break;
			default: buf[i] = c; break;
			}
		}
		buf[i++] = '\"';
		buf[i] = '\0';
		return buf;
	};
	static const char*	CStyleUnQuote(const char* str) {
		if (str[0] != '\"') {
			return str;
		}

		static size_t index = 0;
		static char buffers[4][16384];	// in case called by nested functions
		unsigned int i;
		char* buf;

		buf = buffers[index];
		index = (index + 1) & 3;

		str++;
		for (i = 0; i < sizeof(buffers[0]) - 1; i++) {
			size_t c = *str++;
			if (c == '\0') {
				break;
			}
			else if (c == '\\') {
				c = *str++;
				switch (c) {
				case '\\': buf[i] = '\\'; break;
				case 'n': buf[i] = '\n'; break;
				case 'r': buf[i] = '\r'; break;
				case 't': buf[i] = '\t'; break;
				case 'v': buf[i] = '\v'; break;
				case 'b': buf[i] = '\b'; break;
				case 'f': buf[i] = '\f'; break;
				case 'a': buf[i] = '\a'; break;
				case '\'': buf[i] = '\''; break;
				case '\"': buf[i] = '\"'; break;
				case '\?': buf[i] = '\?'; break;
				}
			}
			else {
				buf[i] = c;
			}
		}
		assert(buf[i - 1] == '\"');
		buf[i - 1] = '\0';
		return buf;
	};

	// hash keys
	
	constexpr static size_t	Hash(const char* s) {
		constexpr AUTO A = 54059; /* a prime */
		constexpr AUTO B = 76963; /* another prime */
		constexpr AUTO C = 86969; /* yet another prime */
		constexpr AUTO FIRSTH = 37; /* also prime */

		size_t h = FIRSTH;
		while (*s) {
			h = (h * A) ^ (s[0] * B);
			s++;
		}
		AUTO result =  h % C; 
		return result;
	};
	constexpr static size_t	hash(const char* s) { return Hash(s); };
	size_t				Hash() const { return cweeStr::Hash(c_str()); };
	static size_t		Hash(const char* string, size_t length) {
		return Hash(cweeStr(string, 0, length));
	};
	static size_t		IHash(const char* string) {
		AUTO s = cweeStr(string);
		s.ToLower();
		return Hash(s);
	};					// case insensitive
	static size_t		IHash(const char* string, size_t length) {
		return IHash(cweeStr(string, 0, length));
	};		// case insensitive

	// character methods
	static char			ToLower(char c) {
		if (c <= 'Z' && c >= 'A') {
			return (c + ('a' - 'A'));
		}
		return c;
	};
	static char			ToUpper(char c) {
		if (c >= 'a' && c <= 'z') {
			return (c - ('a' - 'A'));
		}
		return c;
	};
	static bool			CharIsPrsize_table(size_t c) {
		return (c >= 0x20 && c <= 0x7E) || (c >= 0xA1 && c <= 0xFF);
	};
	static bool			CharIsLower(size_t c) {
		return (c >= 'a' && c <= 'z') || (c >= 0xE0 && c <= 0xFF);
	};
	static bool			CharIsUpper(size_t c) {
		return (c <= 'Z' && c >= 'A') || (c >= 0xC0 && c <= 0xDF);
	};
	static bool			CharIsAlpha(size_t c) {
		return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
			(c >= 0xC0 && c <= 0xFF));
	};
	static bool			CharIsNumeric(size_t c) {
		return (c <= '9' && c >= '0');
	};
	static bool			CharIsNewLine(char c) {
		return (c == '\n' || c == '\r' || c == '\v');
	};
	static bool			CharIsTab(char c) {
		return (c == '\t');
	};

	static cweeStr		printf(const char* fmt, ...) {
		va_list argptr;

		AUTO buffer = new char[128000];		
		buffer[128000 - 1] = '\0';

		// char buffer[128000];

		va_start(argptr, fmt);
		cweeStr::vsnPrintf(buffer, 128000 - 1 /*sizeof(buffer) - 1*/, fmt, argptr);
		va_end(argptr);
		buffer[128000 /*sizeof(buffer)*/ - 1] = '\0';

		cweeStr out(buffer);
		
		delete[] buffer;
		return out;
	};

	static cweeStr		print(const char* format) { return format; }
	template<typename T, typename... Targs> static cweeStr print(const char* format, const T& value, Targs... Fargs) // recursive function
	{
		cweeStr out;

		for (; *format != '\0'; format++) {
			if (*format == '%') {
				out += value;
				out += cweeStr::print(format + 1, Fargs...); // recursive call
				return out;
			}
			out += *format;
		}

		return out;
	}

	void				ReAllocate(size_t amount, bool keepold) { // main cost when saving to file as cweeStr
		char* newbuffer;
		size_t		newsize;
		size_t		mod;

		//assert( data );
		assert(amount > 0);
		size_t alloc_granularity = amount < 1024 ? STR_ALLOC_GRAN : ::Max(STR_ALLOC_GRAN, ::Min(::Max(STR_ALLOC_GRAN, (size_t)(amount * 1.5f)), (size_t)(GetAlloced() * 2.0f))); // rg
		mod = amount % alloc_granularity; // mod = amount % STR_ALLOC_GRAN;
		if (!mod) {
			newsize = amount;
		}
		else {
			newsize = amount + alloc_granularity - mod; // newsize = amount + STR_ALLOC_GRAN - mod;
		}
		if (newsize < (len + 1)) newsize = (len + 1);

		SetAlloced(newsize);

#ifdef USE_STRING_DATA_ALLOCATOR
		stringDataAllocator.Lock();
		auto ptr = stringDataAllocator.UnsafeRead();
		newbuffer = ptr->Alloc(GetAlloced());
		stringDataAllocator.Unlock();
#else
		newbuffer = new (TAG_STRING) char[GetAlloced()];
#endif
		if (GetAlloced() < newsize) { // not optimum but a fix for issues re: bitwise mods for allocation buffer > ~1GB in size (i.e. one string with >4 billion characters)
			delete[] newbuffer;
			newbuffer = new (TAG_STRING) char[newsize + 1];
			SetAlloced(newsize);
		}
		if (keepold && data) {
			data[len] = '\0';
			cweeStr::Copynz(newbuffer, data, len + 1);
			newbuffer[len] = '\0';
		}
		if (data && data != baseBuffer) {
#ifdef USE_STRING_DATA_ALLOCATOR
			stringDataAllocator.Lock();
			auto ptr = stringDataAllocator.UnsafeRead();
			ptr->Free(data);
			stringDataAllocator.Unlock();
#else
			delete[] data;
#endif
		}
		if (!data || newbuffer) {
			data = newbuffer;
		}
	}
	void				FreeData() {
		if (IsStatic()) {
			return;
		}

		if (data && data != baseBuffer) {
			delete[] data;
			data = (char*)baseBuffer; // = baseBuffer;
		}
	};

	static int			cweeLevenshteinDistance(const cweeStr& a, const cweeStr& b, bool caseSensitive = false)
	{
		std::string s1;
		std::string s2;

		if (caseSensitive == true) {
			s1 = a.c_str();
			s2 = b.c_str();
		}
		else {
			cweeStr c = a;
			cweeStr d = b;
			c.ToLower();
			d.ToLower();
			s1 = c.c_str();
			s2 = d.c_str();
		}

		const long long m(s1.size());
		const long long n(s2.size());

		if (m == 0) return n;
		if (n == 0) return m;

		int* costs = new int[n + 1];

		for (int k = 0; k <= n; k++) costs[k] = k;

		int i = 0;
		for (std::string::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1, ++i)
		{
			costs[0] = i + 1;
			int corner = i;

			int j = 0;
			for (std::string::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++j)
			{
				int upper = costs[j + 1];
				if (*it1 == *it2)
				{
					costs[j + 1] = corner;
				}
				else
				{
					int t(upper < corner ? upper : corner);
					costs[j + 1] = (costs[j] < t ? costs[j] : t) + 1;
				}

				corner = upper;
			}
		}

		int result = costs[n];
		delete[] costs;

		return result;
	}

	int					Levenshtein(const cweeStr& other, bool caseSensitive = false) const {
		return cweeLevenshteinDistance(*this, other, caseSensitive);
	};

	static cweeStr  FindLargestSharedSubstring(const cweeStr& strA, const cweeStr& strB, bool caseSensitive = true) {
		cweeStr 
			str1 = strA, 
			str2 = strB;
		bpstd::string_view 
			largestSubstring, 
			currentSubstring;
		int 
			maxLength{ 0 }, 
			i{ 0 }, 
			j{ 0 }, 
			currentLength{ 0 };

		if (!caseSensitive) {
			str1.ToUpper();
			str2.ToUpper();
		}

		for (i = 0; i < str1.Length(); ++i) {
			for (j = i + 1; j <= str1.Length(); ++j) {
				currentSubstring = str1.Mid_Ref(i, j - i); // reference to sub-string, rather than a copy of the underlying text data.
				if (str2.Find(currentSubstring, true) >= 0) {
					currentLength = j - i;
					if (currentLength > maxLength) {
						maxLength = currentLength;
						largestSubstring = strA.Mid_Ref(i, j - i);
					}
				}
			}
		}
		if (largestSubstring.length() > 0) {
			return cweeStr(largestSubstring.c_str(), 0, largestSubstring.length());
		}
		else {
			return cweeStr();
		}
	};
	cweeStr  FindLargestSharedSubstring(const cweeStr& str2, bool caseSensitive = true) const {
		return cweeStr::FindLargestSharedSubstring(*this, str2, caseSensitive);
	};
	cweeStr				BestMatch(std::vector<cweeStr> const& list) const {
		cweeStr out;
		int minL = std::numeric_limits<int>::max();
		const cweeStr& target = *this;
		for (auto& x : list) {
			int i = x.Levenshtein(target);
			if (i <= minL) {
				minL = i;
				out = x;
				if (i == 0) return out;
			}
		}
		return out;
	};
	cweeStr				BestMatch(std::vector<std::string> const& list) const {
		cweeStr out;
		int minL = std::numeric_limits<int>::max();
		const cweeStr& target = *this;
		for (auto& x : list) {
			int i = cweeStr(x).Levenshtein(target);
			if (i <= minL) {
				minL = i;
				out = x.c_str();
				if (i == 0) return out;
			}
		}
		return out;
	};

protected:
	size_t				len;
	char*				data;
	size_t				allocedAndFlag;	// top bit is used to store a flag that indicates if the string data is static or not
	char				baseBuffer[STR_ALLOC_BASE];
	void				EnsureAlloced(size_t amount, bool keepold = true) {
		if (IsStatic()) {
			return;
		}
		if (amount > GetAlloced()) {
			ReAllocate(amount, keepold);
		}
	};	// ensure string data buffer is large anough
	void				SetStaticBuffer(char* buffer, const size_t bufferLength) {
		data = buffer;
		len = 0;
		SetAlloced(bufferLength);
		SetStatic(true);
	};

private:
	void		Construct() {
		SetStatic(false);
		SetAlloced(STR_ALLOC_BASE);
		data = baseBuffer;
		len = 0;
		data[0] = '\0';
	};

	static const size_t	STATIC_BIT = 31;
	static const size_t	STATIC_MASK = 1u << STATIC_BIT;
	static const size_t	ALLOCED_MASK = STATIC_MASK - 1;

	size_t		GetAlloced() const { return allocedAndFlag & ALLOCED_MASK; }
	void		SetAlloced(const size_t a) { allocedAndFlag = (allocedAndFlag & STATIC_MASK) | (a & ALLOCED_MASK); }

	bool		IsStatic() const { return (allocedAndFlag & STATIC_MASK) != 0; }
	void		SetStatic(const bool isStatic) { allocedAndFlag = (allocedAndFlag & ALLOCED_MASK) | (isStatic << STATIC_BIT); }
};

class cweeStrView {
public:
	cweeStrView() : view() {};
	cweeStrView(const cweeStrView& text) : view(text.view) {};
	cweeStrView(const cweeStrView& text, size_t start, size_t end) : view(text.view) { view.mid(start, end); };
	cweeStrView(cweeStr& text) : view(text.c_str()) {};
	cweeStrView(cweeStr& text, size_t start, size_t end) : view(text.c_str()) { view.mid(start, end); };
	~cweeStrView() {};

	size_t				Size() const { return view.length(); };
	cweeStr				c_str() const { return cweeStr(view.c_str(), 0, view.length()); };
	operator cweeStr () const { return cweeStr(view.c_str(), 0, view.length()); };
	operator cweeStr () { return cweeStr(view.c_str(), 0, view.length()); };
	explicit operator float() const { return ReturnNumericD(); };
	explicit operator float() { return ReturnNumericD(); };
	explicit operator double() const { return ReturnNumericD(); };
	explicit operator double() { return ReturnNumericD(); };
	explicit operator u64() const {
		if (view.length() > 0) {
			AUTO ptr = new char[view.length() + 2];
			ptr[view.length() + 1] = '\0';
			std::memcpy(ptr, view.c_str(), view.length());
			u64 out = atof(ptr);
			delete[] ptr;
			return out;
		}
		else {
			return 0;
		}
	};
	explicit operator u64() {
		if (view.length() > 0) {
			AUTO ptr = new char[view.length() + 2];
			ptr[view.length() + 1] = '\0';
			std::memcpy(ptr, view.c_str(), view.length());
			u64 out = atof(ptr);
			delete[] ptr;
			return out;
		}
		else {
			return 0;
		}
	};
	explicit operator int() const { return ReturnNumericD(); };
	explicit operator int() { return ReturnNumericD(); };
	explicit operator bool() const { return ReturnNumericD(); };
	explicit operator bool() { return ReturnNumericD(); };

	char				operator[](size_t index) const { return view.operator[](index); };

	void				operator=(const cweeStrView& text) { view = text.view; };

	friend bool			operator<(const cweeStrView& a, const cweeStrView& b) {
		return a.c_str() < b.c_str();
	};

	// case sensitive compare
	friend bool			operator==(const cweeStrView& a, const cweeStrView& b) { return a.view == b.view; };
	friend bool			operator==(const cweeStrView& a, const char* b) { return a.view == b; };
	friend bool			operator==(const char* a, const cweeStrView& b) { return a == b.view; };

	// case sensitive compare
	friend bool			operator!=(const cweeStrView& a, const cweeStrView& b) { return a.view == b.view; };
	friend bool			operator!=(const cweeStrView& a, const char* b) { return a.view == b; };
	friend bool			operator!=(const char* a, const cweeStrView& b) { return a == b.view; };

	size_t				Length() const { return view.length(); };
	void				Empty() { view = bpstd::string_view(); };
	bool				IsEmpty() const { return view == ""; };
	void				Clear() { view = bpstd::string_view(); };
	bool				IsNumeric() const {
		size_t		i;
		bool	dot, finalSpace;

		auto s = &view.at(0);

		if (*s == '-') {
			s++;
		}

		dot = false;
		finalSpace = false;
		for (i = 0; view[i] && i < view.length(); i++) {
			if (!std::isdigit((const unsigned char)s[i])) {
				if (s[i] == ' ') {
					finalSpace = true;
					continue;
				}
				else {
					if (finalSpace)
						return false;
				}
				if ((s[i] == '.') && !dot) {
					dot = true;
					continue;
				}
				return false;
			}
		}
		return true;
	};
	double				ReturnNumericD() const {
		double r = 0.0;

		AUTO p = view.c_str();
		AUTO len = view.length();
		AUTO len2 = view.length();

		{
			if (len > 0 && p) {
				while (*p == ' ' && len2 >= 1) { ++p; --len2; }
				bool neg = false;
				if (*p == '-' && len2 >= 1) {
					neg = true;
					++p;
					--len2;
				}
				while (*p >= '0' && *p <= '9' && len2 >= 1) {
					r = (r * 10.0) + (*p - '0');
					++p;
					--len2;
				}
				if (*p == '.' && len2 >= 1) {
					double f = 0.0;
					int n = 0;
					++p; --len2;
					while (*p >= '0' && *p <= '9' && len2 >= 1) {
						f = (f * 10.0) + (*p - '0');
						++p; --len2;
						++n;
					}
					r += f / std::pow(10.0, n);
				}
				if (neg) {
					r = -r;
				}
			}
		}
		return r;
	};
	float				ReturnNumeric() const {
		return static_cast<float>(ReturnNumericD());
	};

	long long			Find(const char c, long long start = 0, long long end = -1) const {
		return Find(cweeStr(c), true, start, end);
	};
	long long			Find(const char* text, bool casesensitive = true, long long start = 0, long long end = -1) const {
		if (end == -1) {
			end = view.length();
		}
		return cweeStr::FindText(view.c_str(), text, casesensitive, start, end);
	};
	long long			rFind(const char* text, bool casesensitive = true, long long start = 0, long long end = -1) const {
		if (std::strlen(text) == 0 || this->Length() == 0) return -1;

		long long found = Find(text, casesensitive, start, end);
		long long prevFound = found;
		long long l = ::Max((size_t)1, std::strlen(text));
		while (found != -1) {
			prevFound = found;
			found = Find(text, casesensitive, prevFound + l, end); // if fails to find (-1) then returns the last good find
		}
		return prevFound;
	};

	cweeStrView			Left(size_t len) const {
		cweeStrView out = *this;
		out.view.mid(0, len);
		return out;
	};
	cweeStrView			Right(size_t len) const {
		cweeStrView out = *this;
		out.view.mid(Length() - len, Length());
		return out;
	};
	cweeStrView			Mid(const size_t& start, long long l) const {
		cweeStrView out = *this;
		out.view.mid(start, start + l);
		return out;
	};

	void				StripLeading(const char c) {
		while (view[0] == c) {
			view.mid(1, view.length());
		}
	};
	void				StripLeading(const char* string) {
		long long l;
		l = std::strlen(string);
		if (l > 0) {
			while (!Cmpn(string, l)) {
				view.mid(l, view.length());
			}
		}
	};
	bool				StripLeadingOnce(const char* string) {
		long long l;

		l = std::strlen(string);
		if (l > 0) {
			while (!Cmpn(string, l)) {
				view.mid(l, view.length());
				return true;
			}
		}
		return false;
	};
	void				StripTrailing(const char c) {
		long long i;

		for (i = Length(); i > 0 && view[i - 1] == c; i--) {
			view.mid(0, view.length() - 1);
		}
	};
	void				StripTrailing(const char* string) {
		long long l;

		l = std::strlen(string);
		if (l > 0) {
			while (((long long)view.length() >= l) && !Cmpn(string, view.data() + view.length() - l, l)) {
				view.mid(0, (long long)view.length() - l);
			}
		}
	};
	bool				StripTrailingOnce(const char* string) {
		long long l;

		l = std::strlen(string);
		if (l > 0) {
			while (((long long)view.length() >= l) && !Cmpn(string, view.data() + view.length() - l, l)) {
				view.mid(0, view.length() - l);
				return true;
			}
		}
		return false;
	};

	int					Levenshtein(const cweeStr& other) const;
	cweeStr				BestMatch(std::vector<cweeStr> list) const;
	
	size_t				Cmp(const char* text) const {
		return Cmpn(view.c_str(), text, view.length() < std::strlen(text) ? view.length() : std::strlen(text));
	};
	size_t				Cmpn(const char* text, long long n) const {
		return Cmpn(view.c_str(), text, view.length() < std::strlen(text) ? view.length() : std::strlen(text));
	};
	size_t				Icmp(const char* text)  const {
		return Icmpn(view.c_str(), text, view.length() < std::strlen(text) ? view.length() : std::strlen(text));
	};
	size_t				Icmpn(const char* text, size_t n)  const {
		return Icmpn(view.c_str(), text, view.length() < std::strlen(text) ? view.length() : std::strlen(text));
	};
	static long long	Icmpn(const char* s1, const char* s2, long long n) {
		return cweeStr::Icmpn(s1, s2, n);
	};
	static long long	Cmpn(const char* s1, const char* s2, long long n) {
		return cweeStr::Cmpn(s1, s2, n);
	};

protected:
	bpstd::string_view view;

};

INLINE cweeStrView	cweeStr::View() const {
	return cweeStrView(const_cast<cweeStr&>(*this));
};

namespace std {
	template <>
	struct hash<::cweeStr>
	{
		std::size_t operator()(const ::cweeStr& k) const
		{
			using ::cweeStr;
			return cweeStr::Hash(k);
		}
	};
}

template<typename betterEnumType>
INLINE AUTO		GetBetterEnum(cweeStr const& a) {
	std::vector<cweeStr> list; 
	for (auto& x : betterEnumType::_values()) {
		list.push_back(x.ToString());
	}
	return betterEnumType::_from_string(a.BestMatch(list));
};