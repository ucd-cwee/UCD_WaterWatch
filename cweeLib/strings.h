/*
=======================================================================
Energy Demand Management System for Water Distribution Systems
Developed by the Center for Water-Energy Efficiency, 2019 - 2020
=======================================================================
*/

#ifndef __STR_H__
#define __STR_H__

#define ASSERT_ENUM_STRING( string, index )		( 1 / (size_t)!( string - index ) ) ? #string : ""

class cweeParser;

enum utf8Encoding_t {
	UTF8_PURE_ASCII,		// no characters with values > 127
	UTF8_ENCODED_BOM,		// characters > 128 encoded with UTF8, but no byte-order-marker at the beginning
	UTF8_ENCODED_NO_BOM,	// characters > 128 encoded with UTF8, with a byte-order-marker at the beginning
	UTF8_INVALID,			// has values > 127 but isn't valid UTF8 
	UTF8_INVALID_BOM		// has a byte-order-marker at the beginning, but isn't valuid UTF8 -- it's messed up
};


#ifndef FILE_HASH_SIZE
#define FILE_HASH_SIZE		1024
#endif

const size_t STR_ALLOC_BASE = 20;
constexpr size_t STR_ALLOC_GRAN = 32; // was 32. Increasing to see if this helps with performance of saving to file. 

typedef enum {
	MEASURE_SIZE = 0,
	MEASURE_BANDWIDTH
} Measure_t;

class cweeAny; class cweeStrView;
class cweeStr {
public:
	cweeStr();
	cweeStr(const cweeStr& text);
	cweeStr(const cweeStr& text, size_t start, size_t end);
	cweeStr(const char* text);
	cweeStr(const char* text, size_t start, size_t end);
	explicit cweeStr(const bool b);
	explicit cweeStr(const char c);
	explicit cweeStr(const int i);
	explicit cweeStr(const unsigned u);
	explicit cweeStr(const float f);
	explicit cweeStr(const double f);
	explicit cweeStr(const time_t time);
	explicit cweeStr(const u64 time);
	explicit cweeStr(const std::string in);
	explicit cweeStr(const std::pair<u64, float> in);
	~cweeStr();

	size_t				Size() const;
	const char* c_str() const;
	operator			const char* () const;
	operator			const char* ();
	explicit operator float() const;
	explicit operator float();
	explicit operator double() const;
	explicit operator double();
	explicit operator u64() const;
	explicit operator u64();
	explicit operator int() const;
	explicit operator int();
	explicit operator bool() const;
	explicit operator bool();

	char				operator[](size_t index) const;
	char& operator[](size_t index);

	void				operator=(const cweeStr& text);
	void				operator=(const char* text);

	friend cweeStr		operator+(const cweeStr& a, const cweeStr& b);
	friend cweeStr		operator+(const cweeStr& a, const char* b);
	friend cweeStr		operator+(const char* a, const cweeStr& b);

	friend cweeStr		operator+(const cweeStr& a, const float b);
	friend cweeStr		operator+(const cweeStr& a, const double b);
	friend cweeStr		operator+(const cweeStr& a, const int b);
	friend cweeStr		operator+(const cweeStr& a, const unsigned b);
	friend cweeStr		operator+(const cweeStr& a, const bool b);
	friend cweeStr		operator+(const cweeStr& a, const char b);

	friend bool			operator<(const cweeStr& a, const cweeStr& b) {
		return a.c_str() < b.c_str();
	};

	cweeStr& operator+=(const cweeStr& a);
	cweeStr& operator+=(const char* a);
	cweeStr& operator+=(const float a);
	cweeStr& operator+=(const double a);
	cweeStr& operator+=(const char a);
	cweeStr& operator+=(const int a);
	cweeStr& operator+=(const unsigned a);
	cweeStr& operator+=(const bool a);
	cweeStr& ReplaceInline(const char* _old, const char* _new) {
		Replace(_old, _new);
		return *this;
	};

	// case sensitive compare
	friend bool			operator==(const cweeStr& a, const cweeStr& b);
	friend bool			operator==(const cweeStr& a, const char* b);
	friend bool			operator==(const char* a, const cweeStr& b);

	// case sensitive compare
	friend bool			operator!=(const cweeStr& a, const cweeStr& b);
	friend bool			operator!=(const cweeStr& a, const char* b);
	friend bool			operator!=(const char* a, const cweeStr& b);

	// split based on a delimiter
	cweeParser			Split(const cweeStr& delim) const;

	// case sensitive compare
	size_t					Cmp(const char* text) const;
	size_t					Cmpn(const char* text, size_t n) const;
	size_t					CmpPrefix(const char* text) const;

	// case insensitive compare
	size_t					Icmp(const char* text) const;
	size_t					Icmpn(const char* text, size_t n) const;
	size_t					IcmpPrefix(const char* text) const;

	// compares paths and makes sure folders come first
	size_t					IcmpPath(const char* text) const;
	size_t					IcmpnPath(const char* text, size_t n) const;
	size_t					IcmpPrefixPath(const char* text) const;

	template <typename T>	static cweeStr			ToString(T any) {
		throw(
			cweeStr(
				cweeStr("Attempted to cast ") + typeid(any).name() + cweeStr(" to cweeStr.")
			).c_str()
			);
	};
	template <>				static cweeStr			ToString<bool>(bool any) { return cweeStr(any); };
	template <>				static cweeStr			ToString<float>(float any) { return cweeStr(any); };
	template <>				static cweeStr			ToString<double>(double any) { return cweeStr(any); };
	template <>				static cweeStr			ToString<int>(int any) { return cweeStr(any); };
	template <>				static cweeStr			ToString<u64>(u64 any) { return cweeStr(any); };
	template <>				static cweeStr			ToString<time_t>(time_t any) { return cweeStr(any); };
	template <>				static cweeStr			ToString<const char*>(const char* any) { return any; };
	template <>				static cweeStr			ToString<char>(char any) { return cweeStr(any); };
	template <>				static cweeStr			ToString<std::string>(std::string any) { return any.c_str(); };
	template <>				static cweeStr			ToString<cweeStr>(cweeStr any) { return any; };

	cweeStrView			View() const;



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
			if (copy[i] == '>') {
				start = i + 1;
				break;
			}
		}

		// Remove the blank space
		while (copy[start] == ' ') {
			start++;
		}

		// Traverse the string
		for (int i = start; i < n; i++) {
			// If S[i] is '<', update
			// end to i-1 and break
			if (copy[i] == '<') {
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
#if 0
	static std::map<std::string, cweeAny>		ParseXML(cweeStr xml);
#endif

	size_t				Length() const;
	size_t				Allocated() const;
	void				Empty();
	bool				IsEmpty() const;
	void				Clear();
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
	cweeStr&			Append(const char* text, const size_t& l) {
		long long newLen; long long i;// = long long(l) - 1;
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
	void				Insert(const char a, long long index);
	void				Insert(const char* text, long long index);
	void				ToLower();
	void				ToUpper();
	bool				IsNumeric() const;
	bool				HasLower() const;
	bool				HasUpper() const;
	void				CapLength(size_t);
	void				Fill(const char ch, size_t newlen);

	INLINE size_t		UTF8Length();
	INLINE uint32		UTF8Char(size_t& idx);
	static size_t		UTF8Length(const byte* s);
	static INLINE uint32 UTF8Char(const char* s, size_t& idx);
	static uint32		UTF8Char(const byte* s, size_t& idx);
	void				AppendUTF8Char(uint32 c);
	INLINE void			ConvertToUTF8();
	static bool			IsValidUTF8(const uint8* s, const size_t maxLen, utf8Encoding_t& encoding);
	static INLINE bool	IsValidUTF8(const char* s, const size_t maxLen, utf8Encoding_t& encoding) { return IsValidUTF8((const uint8*)s, maxLen, encoding); }
	static INLINE bool	IsValidUTF8(const uint8* s, const size_t maxLen);
	static INLINE bool	IsValidUTF8(const char* s, const size_t maxLen) { return IsValidUTF8((const uint8*)s, maxLen); }

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
	bool				Filter(const char* filter, bool casesensitive) const;
	long long			Last(const char c) const;						// return the index to the last occurance of 'c', returns -1 if not found
	const char*			Left(size_t len, cweeStr& result) const;			// store the leftmost 'len' characters in the result
	const char*			Right(size_t len, cweeStr& result) const;			// store the rightmost 'len' characters in the result
	const char*			Mid(const size_t& start, long long l, cweeStr& result) const {
		long long i = Length();

		if (i == 0 || l <= 0 || start >= i) {
			result.Empty();
			return nullptr;
		}

		if (start + l >= i) {
			l = i - start;
		}

		cweeStr temp; 
		result = temp.Append(&data[start], l);
		return result;
	};	// store 'len' characters starting at 'start' in result
	cweeStr				Left(size_t len) const;							// return the leftmost 'len' characters
	cweeStr				Right(size_t len) const;							// return the rightmost 'len' characters
	cweeStr				Mid(const size_t& start, long long l) const {
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
	void				ReduceSpaces(bool keepNewLine = false);								// strip \t, \n, and ' ' from string until only one ' ' is between each word, and there is no trailing/leading spaces
	void				StripLeading(const char c);					// strip char from front as many times as the char occurs
	void				StripLeading(const char* string);			// strip string from front as many times as the string occurs
	bool				StripLeadingOnce(const char* string);		// strip string from front just once if it occurs
	void				StripTrailing(const char c);				// strip char from end as many times as the char occurs
	void				StripTrailing(const char* string);			// strip string from end as many times as the string occurs
	bool				StripTrailingOnce(const char* string);		// strip string from end just once if it occurs
	void				Strip(const char c);						// strip char from front and end as many times as the char occurs
	void				Strip(const char* string);					// strip string from front and end as many times as the string occurs
	void				StripOnce(const char* string);					// strip string from front and end as many times as the string occurs
	void				StripTrailingWhitespace();					// strip trailing white space characters
	cweeStr&			StripQuotes();								// strip quotes around string
	bool				Replace(const char* old, const char* nw);
	bool				ReplaceChar(const char old, const char nw);
	void				Replace(const std::vector<cweeStr>& olds, const std::vector<cweeStr>& news);
	INLINE void			CopyRange(const char* text, long long start, long long end);

	// file name methods
	size_t				FileNameHash() const;						// hash key for the filename (skips extension)
	cweeStr&			BackSlashesToSlashes();					// convert slashes
	cweeStr&			SlashesToBackSlashes();					// convert slashes
	cweeStr&			SetFileExtension(const char* extension);		// set the given file extension
	cweeStr&			StripFileExtension();						// remove any file extension
	cweeStr&			StripAbsoluteFileExtension();				// remove any file extension looking from front (useful if there are multiple .'s)
	cweeStr&			DefaultFileExtension(const char* extension);	// if there's no file extension use the default
	cweeStr&			DefaultPath(const char* basepath);			// if there's no path use the default
	void				AppendPath(const char* text);					// append a partial path
	cweeStr&			StripFilename();							// remove the filename from a path
	cweeStr&			StripPath();								// remove the path from the filename
	void				ExtractFilePath(cweeStr& dest) const;			// copy the file path to another string
	void				ExtractFileName(cweeStr& dest) const;			// copy the filename to another string
	void				ExtractFileBase(cweeStr& dest) const;			// copy the filename minus the extension to another string
	void				ExtractFileExtension(cweeStr& dest) const;		// copy the file extension to another string
	bool				CheckExtension(const char* ext);

	float				ReturnNumeric();
	float				ReturnNumeric() const;
	double				ReturnNumericD();
	double				ReturnNumericD() const;
	static std::wstring	toWideString(cweeStr in);
	static cweeStr		fromTime(time_t in);
	cweeStr&			AddToDelimiter(cweeStr& in, const cweeStr& delimiter) {
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
	cweeStr&			AddToDelimiter(const char* in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(in);
		return out;
	};
	cweeStr&			AddToDelimiter(char in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr(in));
		return out;
	};
	cweeStr&			AddToDelimiter(int in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((int)in));
		return out;
	};
	cweeStr&			AddToDelimiter(float in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr(in));
		return out;
	};
	cweeStr&			AddToDelimiter(bool in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr(in));
		return out;
	};
	cweeStr&			AddToDelimiter(double in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((float)in));
		return out;
	};
	cweeStr&			AddToDelimiter(time_t in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((float)in));
		return out;
	};
	cweeStr&			AddToDelimiter(u64 in, const cweeStr& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((float)in));
		return out;
	};

	cweeStr&			AddToDelimiter(cweeStr& in, const char& delimiter) {
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
	cweeStr&			AddToDelimiter(const char* in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr(in));
		return out;
	};
	cweeStr&			AddToDelimiter(char in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr(in));
		return out;
	};
	cweeStr&			AddToDelimiter(int in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((int)in));
		return out;
	};
	cweeStr&			AddToDelimiter(float in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr(in));
		return out;
	};
	cweeStr&			AddToDelimiter(bool in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((int)in));
		return out;
	};
	cweeStr&			AddToDelimiter(double in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((float)in));
		return out;
	};
	cweeStr&			AddToDelimiter(time_t in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((float)in));
		return out;
	};
	cweeStr&			AddToDelimiter(u64 in, const char& delimiter) {
		cweeStr& out = *this;
		if (out.IsEmpty() == false) {
			out += delimiter;
		}
		out.Append(cweeStr((float)in));
		return out;
	};


	// char * methods to replace library functions
	static size_t		Length(const char* s);
	static char*		ToLower(char* s);
	static char*		ToUpper(char* s);
	static bool			IsNumeric(const char* s);
	static bool			HasLower(const char* s);
	static bool			HasUpper(const char* s);
	static long long	Cmp(const char* s1, const char* s2);
	static long long	Cmpn(const char* s1, const char* s2, long long n);
	static long long	Icmp(const char* s1, const char* s2);
	static long long	Icmpn(const char* s1, const char* s2, long long n);
	static long long	IcmpPath(const char* s1, const char* s2);			// compares paths and makes sure folders come first
	static long long	IcmpnPath(const char* s1, const char* s2, long long n);	// compares paths and makes sure folders come first
	static void			Copynz(char* dest, const char* src, size_t destsize);
	static size_t		vsnPrintf(char* dest, size_t size, const char* fmt, va_list argptr);
	static long long	FindChar(const char* str, const char c, long long start = 0, long long end = -1);
	static long long	FindText(const char* str, const char* text, bool casesensitive = true, long long start = 0, long long end = -1);
	static bool			Filter(const char* filter, const char* name, bool casesensitive);
	static void			StripMediaName(const char* name, cweeStr& mediaName);
	static bool			CheckExtension(const char* name, const char* ext);
	static const char*	CStyleQuote(const char* str);
	static const char*	CStyleUnQuote(const char* str);

	// hash keys
	static size_t		Hash(const char* string);
	static size_t		Hash(const char* string, size_t length);
	static size_t		IHash(const char* string);					// case insensitive
	static size_t		IHash(const char* string, size_t length);		// case insensitive

	// character methods
	static char			ToLower(char c);
	static char			ToUpper(char c);
	static bool			CharIsPrsize_table(size_t c);
	static bool			CharIsLower(size_t c);
	static bool			CharIsUpper(size_t c);
	static bool			CharIsAlpha(size_t c);
	static bool			CharIsNumeric(size_t c);
	static bool			CharIsNewLine(char c);
	static bool			CharIsTab(char c);

	friend size_t		sprintf(cweeStr& dest, const char* fmt, ...);
	static cweeStr		printf(const char* fmt, ...);

	static constexpr unsigned int hash(const char* s, int off = 0) {
		return !s[off] ? 5381 : (hash(s, off + 1) * 33) ^ s[off];
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





	friend size_t		vsprintf(cweeStr& dest, const char* fmt, va_list ap);
	static size_t		odsprintf(cweeStr in);

	void				ReAllocate(size_t amount, bool keepold);
	void				FreeData();

	size_t				BestUnit(const char* format, float value, Measure_t measure);
	void				SetUnit(const char* format, float value, size_t unit, Measure_t measure);

	static void			InitMemory();
	static void			ShutdownMemory();
	static void			PurgeMemory();

	size_t				DynamicMemoryUsed() const;
	static cweeStr		FormatNumber(size_t number);
	int					Levenshtein(const cweeStr& other) const;
	cweeStr				BestMatch(std::vector<cweeStr> list) const;
	cweeStr				iBestMatch(std::vector<cweeStr> list) const;

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
	INLINE void		SetStaticBuffer(char* buffer, const size_t bufferLength);

private:
	INLINE void		Construct();

	static const size_t	STATIC_BIT = 31;
	static const size_t	STATIC_MASK = 1u << STATIC_BIT;
	static const size_t	ALLOCED_MASK = STATIC_MASK - 1;


	INLINE size_t		GetAlloced() const { return allocedAndFlag & ALLOCED_MASK; }
	INLINE void		SetAlloced(const size_t a) { allocedAndFlag = (allocedAndFlag & STATIC_MASK) | (a & ALLOCED_MASK); }

	INLINE bool		IsStatic() const { return (allocedAndFlag & STATIC_MASK) != 0; }
	INLINE void		SetStatic(const bool isStatic) { allocedAndFlag = (allocedAndFlag & ALLOCED_MASK) | (isStatic << STATIC_BIT); }

	//public:
		// static const int	INVALID_POSITION = -1;
};









INLINE cweeStr cweeStr::fromTime(time_t in) {
	cweeStr result = cweeStr(ctime(&in));
	result.ReduceSpaces();
	return result;
}

INLINE std::wstring		cweeStr::toWideString(cweeStr in) {
	std::string s = in.c_str();
	size_t len;
	size_t slength = (size_t)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

INLINE void cweeStr::Construct() {
	SetStatic(false);
	SetAlloced(STR_ALLOC_BASE);
	data = baseBuffer;
	len = 0;
	data[0] = '\0';
}

INLINE void cweeStr::SetStaticBuffer(char* buffer, const size_t bufferLength) {
	data = buffer;
	len = 0;
	SetAlloced(bufferLength);
	SetStatic(true);
}

INLINE cweeStr::cweeStr() {
	Construct();
}

INLINE cweeStr::cweeStr(const cweeStr& text) {
	Construct();
	size_t l;
	l = text.Length();
	EnsureAlloced(l + 1);
	strcpy(data, text.data);
	len = l;
}

INLINE cweeStr::cweeStr(const cweeStr& text, size_t start, size_t end) {
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
}

INLINE cweeStr::cweeStr(const char* text) {
	Construct();
	size_t l;
	const char* hold(text);

	if (hold != NULL) {
		l = strlen(hold);
		EnsureAlloced(l + 1);
		strcpy(data, hold);
		len = l;
	}
}

INLINE cweeStr::cweeStr(const char* text, size_t start, size_t end) {
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
}

INLINE cweeStr::cweeStr(const bool b) {
	Construct();
	EnsureAlloced(2);
	data[0] = b ? '1' : '0';
	data[1] = '\0';
	len = 1;
}

INLINE cweeStr::cweeStr(const char c) {
	Construct();
	EnsureAlloced(2);
	data[0] = c;
	data[1] = '\0';
	len = 1;
}

INLINE cweeStr::cweeStr(const int i) {
	int local = 0;
	if (i != NULL) local = i;

	Construct();
	char text[64];
	size_t l;

	l = sprintf(text, "%d", local);
	EnsureAlloced(l + 1);
	strcpy(data, text);
	len = l;
}

INLINE cweeStr::cweeStr(const unsigned u) {
	Construct();
	char text[64];
	size_t l;

	l = sprintf(text, "%u", u);
	EnsureAlloced(l + 1);
	strcpy(data, text);
	len = l;
}

INLINE cweeStr::cweeStr(const float f) {
	float local = 0.0f;
	if (f != NULL) local = f;

	Construct();
	char text[64];
	long long l;

	l = sprintf(text, "%f", local);
	while (l > 0 && text[l - 1] == '0') text[--l] = '\0';
	while (l > 0 && text[l - 1] == '.') text[--l] = '\0';
	EnsureAlloced(l + 1);
	strcpy(data, text);
	len = l;
}

INLINE cweeStr::cweeStr(const double f) {
	Construct();
	this->operator=(std::to_string(f).c_str());
}

INLINE cweeStr::cweeStr(const time_t time) {
	Construct();
	size_t l;
	const char* text = ctime(&time);
	if (text) {
		l = strlen(text);
		EnsureAlloced(l + 1);
		strcpy(data, text);
		len = l;
	}
	ReduceSpaces();
}

INLINE cweeStr::cweeStr(const u64 time) {
	Construct();
	size_t l;
	std::string t = std::to_string(time);
	const char* text = t.c_str();
	if (text) {
		l = strlen(text);
		EnsureAlloced(l + 1);
		strcpy(data, text);
		len = l;
	}
	ReduceSpaces();
	if (Find(".") >= 0) {
		StripTrailing('0');
		StripTrailing('.'); // if the previous call removed all 0's, this will clean-up the final 0
	}
}

INLINE cweeStr::cweeStr(const std::pair<u64, float> in) {
	auto first = cweeStr((time_t)in.first);
	auto second = cweeStr((float)in.second);

	Construct();
	size_t l;
	const char* text = cweeStr::printf("(%s, %s)", first.c_str(), second.c_str());
	if (text) {
		l = strlen(text);
		EnsureAlloced(l + 1);
		strcpy(data, text);
		len = l;
	}
	ReduceSpaces();
}

INLINE cweeStr::cweeStr(const std::string in) {
	Construct();
	size_t l;
	const char* hold(in.c_str());

	if (hold != NULL) {
		l = strlen(hold);
		EnsureAlloced(l + 1);
		strcpy(data, hold);
		len = l;
	}
}

INLINE cweeStr::~cweeStr() {
	FreeData();
}

INLINE size_t cweeStr::Size() const {
	return sizeof(*this) + Allocated();
}

INLINE const char* cweeStr::c_str() const {
	return data;
}

INLINE cweeStr::operator const char* () {
	return c_str();
}

INLINE cweeStr::operator const char* () const {
	return c_str();
}

INLINE cweeStr::operator float() {
	return this->ReturnNumeric();
}

INLINE cweeStr::operator float() const {
	return this->ReturnNumeric();
}

INLINE cweeStr::operator double() {
	return this->ReturnNumericD();
}

INLINE cweeStr::operator double() const {
	return this->ReturnNumericD();
}

INLINE cweeStr::operator int() {
	return this->ReturnNumericD();
}

INLINE cweeStr::operator int() const {
	return this->ReturnNumericD();
}

INLINE cweeStr::operator bool() {
	return this->ReturnNumeric();
}

INLINE cweeStr::operator bool() const {
	return this->ReturnNumeric();
}

INLINE cweeStr::operator u64() {
	return (u64)atof(c_str());
}

INLINE cweeStr::operator u64() const {
	return (u64)atof(c_str());
}

INLINE char cweeStr::operator[](size_t index) const {
	return data[index];
}

INLINE char& cweeStr::operator[](size_t index) {
	return data[index];
}

INLINE void cweeStr::operator=(const cweeStr& text) {
	size_t l;

	try {
		l = text.Length();
		EnsureAlloced(l + 1, false);
		// std::memcpy(data, text.data, l);
		strcpy(data, text.data);
		data[l] = '\0';
		len = l;
	}
	catch (...) {
		const char* hold(text.c_str());

		if (hold != NULL) {
			l = strlen(hold);
			EnsureAlloced(l + 1);
			strcpy(data, hold);
			len = l;
		}
		else {
			Clear();
		}
	}
}

INLINE cweeStr operator+(const cweeStr& a, const cweeStr& b) {
	cweeStr result(a);
	result.Append(b);
	return result;
}

INLINE cweeStr operator+(const cweeStr& a, const char* b) {
	cweeStr result(a);
	result.Append(b);
	return result;
}

INLINE cweeStr operator+(const char* a, const cweeStr& b) {
	cweeStr result(a);
	result.Append(b);
	return result;
}

INLINE cweeStr operator+(const cweeStr& a, const bool b) {
	cweeStr result(a);
	result.Append(b ? "true" : "false");
	return result;
}

INLINE cweeStr operator+(const cweeStr& a, const char b) {
	cweeStr result(a);
	result.Append(b);
	return result;
}

INLINE cweeStr operator+(const cweeStr& a, const float b) {
	char	text[64];
	cweeStr	result(a);

	sprintf(text, "%f", b);
	result.Append(text);

	return result;
}

INLINE cweeStr operator+(const cweeStr& a, const double b) {
	cweeStr	result(a);
	result.Append(std::to_string(b).c_str());
	return result;
}

INLINE cweeStr operator+(const cweeStr& a, const int b) {
	char	text[64];
	cweeStr	result(a);

	sprintf(text, "%d", b);
	result.Append(text);

	return result;
}

INLINE cweeStr operator+(const cweeStr& a, const unsigned b) {
	char	text[64];
	cweeStr	result(a);

	sprintf(text, "%u", b);
	result.Append(text);

	return result;
}

INLINE cweeStr& cweeStr::operator+=(const float a) {
	char text[64];

	sprintf(text, "%f", a);
	Append(text);

	return *this;
}

INLINE cweeStr& cweeStr::operator+=(const double a) {
	Append(std::to_string(a).c_str());
	return *this;
}

INLINE cweeStr& cweeStr::operator+=(const int a) {
	char text[64];

	sprintf(text, "%d", a);
	Append(text);

	return *this;
}

INLINE cweeStr& cweeStr::operator+=(const unsigned a) {
	char text[64];

	sprintf(text, "%u", a);
	Append(text);

	return *this;
}

INLINE cweeStr& cweeStr::operator+=(const cweeStr& a) {
	Append(a);
	return *this;
}

INLINE cweeStr& cweeStr::operator+=(const char* a) {
	Append(a);
	return *this;
}

INLINE cweeStr& cweeStr::operator+=(const char a) {
	Append(a);
	return *this;
}

INLINE cweeStr& cweeStr::operator+=(const bool a) {
	Append(a ? "true" : "false");
	return *this;
}

INLINE bool operator==(const cweeStr& a, const cweeStr& b) {
	return (!cweeStr::Cmp(a.data, b.data));
}

INLINE bool operator==(const cweeStr& a, const char* b) {
	return (!cweeStr::Cmp(a.data, b));
}

INLINE bool operator==(const char* a, const cweeStr& b) {
	return (!cweeStr::Cmp(a, b.data));
}

INLINE bool operator!=(const cweeStr& a, const cweeStr& b) {
	return !(a == b);
}

INLINE bool operator!=(const cweeStr& a, const char* b) {
	return !(a == b);
}

INLINE bool operator!=(const char* a, const cweeStr& b) {
	return !(a == b);
}

INLINE size_t cweeStr::Cmp(const char* text) const {
	return cweeStr::Cmp(data, text);
}

INLINE size_t cweeStr::Cmpn(const char* text, size_t n) const {
	return cweeStr::Cmpn(data, text, n);
}

INLINE size_t cweeStr::CmpPrefix(const char* text) const {
	return cweeStr::Cmpn(data, text, strlen(text));
}

INLINE size_t cweeStr::Icmp(const char* text) const {
	return cweeStr::Icmp(data, text);
}

INLINE size_t cweeStr::Icmpn(const char* text, size_t n) const {
	return cweeStr::Icmpn(data, text, n);
}

INLINE size_t cweeStr::IcmpPrefix(const char* text) const {
	return cweeStr::Icmpn(data, text, strlen(text));
}

INLINE size_t cweeStr::IcmpPath(const char* text) const {
	return cweeStr::IcmpPath(data, text);
}

INLINE size_t cweeStr::IcmpnPath(const char* text, size_t n) const {
	return cweeStr::IcmpnPath(data, text, n);
}

INLINE size_t cweeStr::IcmpPrefixPath(const char* text) const {
	return cweeStr::IcmpnPath(data, text, strlen(text));
}

INLINE size_t cweeStr::Length() const {
	return len;
}

INLINE size_t cweeStr::Allocated() const {
	if (data != baseBuffer) {
		return GetAlloced();
	}
	else {
		return 0;
	}
}

INLINE void cweeStr::Empty() {
	EnsureAlloced(1);
	data[0] = '\0';
	len = 0;
}

INLINE bool cweeStr::IsEmpty() const {
	return (cweeStr::Cmp(data, "") == 0);
}

INLINE void cweeStr::Clear() {
	if (IsStatic()) {
		len = 0;
		data[0] = '\0';
		return;
	}
	FreeData();
	Construct();
}

INLINE void cweeStr::Insert(const char a, long long index) {
	long long i, l;

	if (index < 0) {
		index = 0;
	}
	else if (index > len) {
		index = len;
	}

	l = 1;
	EnsureAlloced(len + l + 1);
	for (i = len; i >= index; i--) {
		data[i + l] = data[i];
	}
	data[index] = a;
	len++;
}

INLINE void cweeStr::Insert(const char* text, long long index) {
	long long i, l;

	if (index < 0) {
		index = 0;
	}
	else if (index > len) {
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
}

INLINE bool cweeStr::CharIsLower(size_t c) {
	return (c >= 'a' && c <= 'z') || (c >= 0xE0 && c <= 0xFF);
}

INLINE void cweeStr::ToLower() {
	for (size_t i = 0; data[i]; i++) {
		if (CharIsUpper(data[i])) {
			data[i] += ('a' - 'A');
		}
	}
}

INLINE void cweeStr::ToUpper() {
	for (size_t i = 0; data[i]; i++) {
		if (CharIsLower(data[i])) {
			data[i] -= ('a' - 'A');
		}
	}
}

INLINE bool cweeStr::CharIsNewLine(char c) {
	return (c == '\n' || c == '\r' || c == '\v');
}

INLINE bool cweeStr::CharIsTab(char c) {
	return (c == '\t');
}

INLINE bool cweeStr::CharIsAlpha(size_t c) {
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
		(c >= 0xC0 && c <= 0xFF));
}

INLINE bool cweeStr::IsNumeric() const {
	return cweeStr::IsNumeric(data);
}

INLINE bool cweeStr::HasLower() const {
	return cweeStr::HasLower(data);
}

INLINE bool cweeStr::HasUpper() const {
	return cweeStr::HasUpper(data);
}

INLINE void cweeStr::CapLength(size_t newlen) {
	if (len <= newlen) {
		return;
	}
	data[newlen] = 0;
	len = newlen;
}

INLINE void cweeStr::Fill(const char ch, size_t newlen) {
	EnsureAlloced(newlen + 1);
	len = newlen;
	memset(data, ch, len);
	data[len] = 0;
}

INLINE size_t cweeStr::UTF8Length() {
	return UTF8Length((byte*)data);
}

INLINE uint32 cweeStr::UTF8Char(size_t& idx) {
	return UTF8Char((byte*)data, idx);
}

INLINE void cweeStr::ConvertToUTF8() {
	cweeStr temp(*this);
	Clear();
	for (size_t index = 0; index < temp.Length(); ++index) {
		AppendUTF8Char(temp[index]);
	}
}

INLINE uint32 cweeStr::UTF8Char(const char* s, size_t& idx) {
	return UTF8Char((byte*)s, idx);
}

INLINE bool cweeStr::IsValidUTF8(const uint8* s, const size_t maxLen) {
	utf8Encoding_t encoding;
	return IsValidUTF8(s, maxLen, encoding);
}

INLINE bool cweeStr::Filter(const char* filter, bool casesensitive) const {
	return cweeStr::Filter(filter, data, casesensitive);
}

INLINE const char* cweeStr::Left(size_t len, cweeStr& result) const {
	return Mid(0, len, result);
}

INLINE const char* cweeStr::Right(size_t len, cweeStr& result) const {
	if (len >= Length()) {
		result = *this;
		return result;
	}
	return Mid(Length() - len, len, result);
}

INLINE cweeStr cweeStr::Left(size_t len) const {
	return Mid(0, len);
}

INLINE cweeStr cweeStr::Right(size_t len) const {
	if (len >= Length()) {
		return *this;
	}
	return Mid(Length() - len, len);
}

INLINE void cweeStr::Strip(const char c) {
	StripLeading(c);
	StripTrailing(c);
}

INLINE void cweeStr::Strip(const char* string) {
	StripLeading(string);
	StripTrailing(string);
}

INLINE void cweeStr::StripOnce(const char* string) {
	StripLeadingOnce(string);
	StripTrailingOnce(string);
}

INLINE bool cweeStr::CheckExtension(const char* ext) {
	return cweeStr::CheckExtension(data, ext);
}

INLINE float cweeStr::ReturnNumeric() {
	return static_cast<float>(ReturnNumericD());
}

INLINE float cweeStr::ReturnNumeric() const {
	return static_cast<float>(ReturnNumericD());
}

INLINE double cweeStr::ReturnNumericD() {
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

	//return atof(c_str());
}

INLINE double cweeStr::ReturnNumericD() const {
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
}

INLINE size_t cweeStr::Length(const char* s) {
	size_t i;
	for (i = 0; s[i]; i++) {}
	return i;
}

INLINE char* cweeStr::ToLower(char* s) {
	for (size_t i = 0; s[i]; i++) {
		if (CharIsUpper(s[i])) {
			s[i] += ('a' - 'A');
		}
	}
	return s;
}

INLINE char* cweeStr::ToUpper(char* s) {
	for (size_t i = 0; s[i]; i++) {
		if (CharIsLower(s[i])) {
			s[i] -= ('a' - 'A');
		}
	}
	return s;
}

INLINE size_t cweeStr::Hash(const char* string) {
	size_t i, hash = 0;
	for (i = 0; *string != '\0'; i++) {
		hash += (*string++) * (i + 119);
	}
	return hash;
}

INLINE size_t cweeStr::Hash(const char* string, size_t length) {
	size_t i, hash = 0;
	for (i = 0; i < length; i++) {
		hash += (*string++) * (i + 119);
	}
	return hash;
}

INLINE size_t cweeStr::IHash(const char* string) {
	size_t i, hash = 0;
	for (i = 0; *string != '\0'; i++) {
		hash += ToLower(*string++) * (i + 119);
	}
	return hash;
}

INLINE size_t cweeStr::IHash(const char* string, size_t length) {
	size_t i, hash = 0;
	for (i = 0; i < length; i++) {
		hash += ToLower(*string++) * (i + 119);
	}
	return hash;
}

INLINE char cweeStr::ToLower(char c) {
	if (c <= 'Z' && c >= 'A') {
		return (c + ('a' - 'A'));
	}
	return c;
}

INLINE char cweeStr::ToUpper(char c) {
	if (c >= 'a' && c <= 'z') {
		return (c - ('a' - 'A'));
	}
	return c;
}

INLINE bool cweeStr::CharIsPrsize_table(size_t c) {
	return (c >= 0x20 && c <= 0x7E) || (c >= 0xA1 && c <= 0xFF);
}



INLINE bool cweeStr::CharIsUpper(size_t c) {
	return (c <= 'Z' && c >= 'A') || (c >= 0xC0 && c <= 0xDF);
}



INLINE bool cweeStr::CharIsNumeric(size_t c) {
	return (c <= '9' && c >= '0');
}


INLINE size_t cweeStr::DynamicMemoryUsed() const {
	return (data == baseBuffer) ? 0 : GetAlloced();
}

INLINE void cweeStr::CopyRange(const char* text, long long start, long long end) {

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
}






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
			delete ptr;
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
			delete ptr;
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
			while ((view.length() >= l) && !Cmpn(string, view.data() + view.length() - l, l)) {
				view.mid(0, view.length() - l);
			}
		}
	};
	bool				StripTrailingOnce(const char* string) {
		long long l;

		l = std::strlen(string);
		if (l > 0) {
			while ((view.length() >= l) && !Cmpn(string, view.data() + view.length() - l, l)) {
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
			return cweeStr::hash(k);
		}
	};
}






#endif
