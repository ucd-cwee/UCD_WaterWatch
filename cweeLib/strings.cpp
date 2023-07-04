/*
=======================================================================
Energy Demand Management System for Water Distribution Systems
Developed by the Center for Water-Energy Efficiency, 2019 - 2020
=======================================================================
*/


#include "precompiled.h"
#pragma hdrstop

//#define USE_STRING_DATA_ALLOCATOR

#ifdef USE_STRING_DATA_ALLOCATOR
static cweeUnpooledInterlocked<cweeDynamicBlockAlloc<char, 1 << 18, 128, TAG_STRING>>	stringDataAllocator;
//static cweeDynamicBlockAlloc<char, 1 << 18, 128, TAG_STRING>	stringDataAllocator;
#endif

const char* units[2][4] =
{
	{ "B", "KB", "MB", "GB" },
	{ "B/s", "KB/s", "MB/s", "GB/s" }
};

/*
============
cweeStr::ReAllocate
============
*/
void cweeStr::ReAllocate(size_t amount, bool keepold) { // main cost when saving to file as cweeStr
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
		// strcpy(newbuffer, data);
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

/*
============
cweeStr::FreeData
============
*/
void cweeStr::FreeData() {
	if (IsStatic()) {
		return;
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
		data = (char*)baseBuffer; // = baseBuffer;
	}
}

/*
============
cweeStr::operator=
============
*/
void cweeStr::operator=(const char* text) {
	size_t l;
	size_t diff;
	size_t i;

	if (!text) {
		// safe behavior if NULL
		EnsureAlloced(1, false);
		data[0] = '\0';
		len = 0;
		return;
	}

	if (text == data) {
		return; // copying same thing
	}

	// check if we're aliasing
	if (text >= data && text <= data + len) {
		diff = text - data;

		assert(strlen(text) < (unsigned)len);

		for (i = 0; text[i]; i++) {
			data[i] = text[i];
		}

		data[i] = '\0';

		len -= diff;

		return;
	}

	l = strlen(text);
	EnsureAlloced(l + 1, false);
	cweeStr::Copynz(data, text, l + 1);
	len = l;
}

/*
============
cweeStr::Filter

Returns true if the string conforms the given filter.
Several metacharacter may be used in the filter.

*          match any string of zero or more characters
?          match any single character
[abc...]   match any of the enclosed characters; a hyphen can
		   be used to specify a range (e.g. a-z, A-Z, 0-9)

============
*/
bool cweeStr::Filter(const char* filter, const char* name, bool casesensitive) {
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
}

/*
=============
cweeStr::StripMediaName

  makes the string lower case, replaces backslashes with forward slashes, and removes extension
=============
*/
void cweeStr::StripMediaName(const char* name, cweeStr& mediaName) {
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
}

/*
=============
cweeStr::CheckExtension
=============
*/
bool cweeStr::CheckExtension(const char* name, const char* ext) {
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
}

/*
========================
cweeStr::CStyleQuote
========================
*/
const char* cweeStr::CStyleQuote(const char* str) {
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
}

/*
========================
cweeStr::CStyleUnQuote
========================
*/
const char* cweeStr::CStyleUnQuote(const char* str) {
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
}

/*
============
cweeStr::Last

returns -1 if not found otherwise the index of the char
============
*/
long long cweeStr::Last(const char c) const {
	long long i;

	for (i = Length(); i > 0; i--) {
		if (data[i - 1] == c) {
			return i - 1;
		}
	}

	return -1;
}

/*
============
cweeStr::ReduceSpaces
============
*/
void cweeStr::ReduceSpaces(bool keepNewLine) {
	//ReplaceChar('\t', ' '); // may be worth to keep tabs...
	if (!keepNewLine) ReplaceChar('\n', ' ');
	StripLeading(' ');
	StripTrailing(' ');
	while (Find("  ") >= 0) {
		// Replace(cweeStr("  ").c_str(), cweeStr(" ").c_str());
		Replace("  ", " ");
	}
}

/*
============
cweeStr::StripLeading
============
*/
void cweeStr::StripLeading(const char c) {
	while (data[0] == c) {
		memmove(&data[0], &data[1], len);
		len--;
	}
}

/*
============
cweeStr::StripLeading
============
*/
void cweeStr::StripLeading(const char* string) {
	long long l;

	l = strlen(string);
	if (l > 0) {
		while (!Cmpn(string, l)) {
			memmove(data, data + l, len - l + 1);
			len -= l;
		}
	}
}

/*
============
cweeStr::StripLeadingOnce
============
*/
bool cweeStr::StripLeadingOnce(const char* string) {
	long long l;

	l = strlen(string);
	if ((l > 0) && !Cmpn(string, l)) {
		memmove(data, data + l, len - l + 1);
		len -= l;
		return true;
	}
	return false;
}

/*
============
cweeStr::StripTrailing
============
*/
void cweeStr::StripTrailing(const char c) {
	long long i;

	for (i = Length(); i > 0 && data[i - 1] == c; i--) {
		data[i - 1] = '\0';
		len--;
	}
}

/*
============
cweeStr::StripLeading
============
*/
void cweeStr::StripTrailing(const char* string) {
	long long l;

	l = strlen(string);
	if (l > 0) {
		while ((len >= l) && !Cmpn(string, data + len - l, l)) {
			len -= l;
			data[len] = '\0';
		}
	}
}

/*
============
cweeStr::StripTrailingOnce
============
*/
bool cweeStr::StripTrailingOnce(const char* string) {
	long long l;

	l = strlen(string);
	if ((l > 0) && (len >= l) && !Cmpn(string, data + len - l, l)) {
		len -= l;
		data[len] = '\0';
		return true;
	}
	return false;
}

/*
============
cweeStr::Replace
============
*/
bool  cweeStr::ReplaceChar(const char old, const char nw) {
	bool replaced = false;
	for (long long i = 0; i < Length(); i++) {
		if (data[i] == old) {
			data[i] = nw;
			replaced = true;
		}
	}
	return replaced;
}

/*
============
cweeStr::Replace
============
*/
bool cweeStr::Replace(const char* old, const char* nw) {
	long long oldLen = strlen(old);
	long long newLen = strlen(nw);
	long long prevlength = Length();

	if (oldLen <= 0) return false;
#if 0
	// Work out how big the new string will be
	long long count = 0;
	for (long long i = 0; i < prevlength; i++) {
		if (cweeStr::Cmpn(&data[i], old, oldLen) == 0) {
			count++;
			i += oldLen - 1;
		}
		else if (((i + oldLen) < prevlength) && (data[i + oldLen] != old[oldLen - 1])) {
			i++;
		}
	}

#if 0
	if (count) {
		cweeStr oldString(data);

		EnsureAlloced(len + ((newLen - oldLen) * count) + 2, false);

		// Replace the old data with the new data
		size_t j = 0;
		for (long long i = 0; i < oldString.Length(); i++) {
			if (cweeStr::Cmpn(&oldString[(size_t)i], old, oldLen) == 0) {
				memcpy(data + j, nw, newLen);
				i += oldLen - 1;
				j += newLen;
			}
			else {
				data[j] = oldString[i];
				j++;
			}
		}
		data[j] = 0;
		len = strlen(data);
		return true;
	}
#else

	if (count) {
		// inline replace without copying the data. 
		long long finalLen = len + ((newLen - oldLen) * count);

		if (finalLen > len)
		{
			EnsureAlloced(finalLen + 2, true);
			long long diff = finalLen - len;

			// data[] is now the size of "finalLen + 2"
			// move all of our data to the right-most edge. 
			long long i, j, k;
			for (i = finalLen; i >= diff; i--) data[i] = data[i - diff];
			// for (; i >= 0; i--) data[i] = ' ';

			// "old\0" 
			// "   old\0"

			j = 0;
			// Replace the old data with the new data
			for (i = diff; i < finalLen; i++) {
				if (cweeStr::Cmpn(&data[i], old, oldLen) == 0) {
					while (j + newLen >= finalLen) {
						EnsureAlloced(j + newLen + 2, true);
						finalLen = j + newLen + 2;
					}
					memcpy(data + j, nw, newLen);

					i += oldLen - 1;
					j += newLen;
				}
				else if (((i + oldLen) < finalLen) && (data[i + oldLen] != old[oldLen - 1])) {
					memcpy(data + j, data + i, 2);
					j += 2;
					i++;
				}
				else {
					data[j] = data[i];
					j++;
				}
			}
			data[j] = 0;
			len = strlen(data);
			return true;
		}
		else {
#if 0
			// same length or shorter. instead of 'copying' the data, we should just move it over as we go. 
			cweeStr oldString(data);
			EnsureAlloced(len + ((newLen - oldLen) * count) + 2, false);
			// Replace the old data with the new data
			size_t j = 0;
			for (long long i = 0; i < oldString.Length(); i++) {
				if (cweeStr::Cmpn(&oldString[(size_t)i], old, oldLen) == 0) {
					memcpy(data + j, nw, newLen);
					i += oldLen - 1;
					j += newLen;
				}
				else {
					data[j] = oldString[i];
					j++;
				}
			}
			data[j] = 0;
			len = strlen(data);
			return true;
#else 			
			long long buffer = ::Max(oldLen, newLen) - ::Min((long long)0.0f, finalLen - (long long)len);
			long long diff = (finalLen + buffer) - len;
			EnsureAlloced(finalLen + 2 + buffer, true);

			// data[] is now the size of "finalLen + 2"
			// move all of our data to the right-most edge. 
			long long i, j, k;
			for (i = finalLen + buffer; i >= diff; i--) data[i] = data[i - diff];
			// for (; i >= 0; i--) data[i] = ' ';

			// "old\0" 
			// "   old\0"

			j = 0;
			// Replace the old data with the new data
			for (i = diff; i < (finalLen + buffer); i++) {
				if (cweeStr::Cmpn(&data[i], old, oldLen) == 0) {
					while (j + newLen >= (finalLen + buffer)) {
						EnsureAlloced(j + newLen + 2 + buffer, true);
						finalLen = j + newLen + 2;
					}
					memcpy(data + j, nw, newLen);

					i += oldLen - 1;
					j += newLen;
				}
				else if (((i + oldLen) < (finalLen + buffer)) && (data[i + oldLen] != old[oldLen - 1])) {
					memcpy(data + j, data + i, 2);
					j += 2;
					i++;
				}
				else {
					data[j] = data[i];
					j++;
				}
			}
			data[j] = 0;
			len = strlen(data);
			return true;
#endif
		}
	}

#endif
#else

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
			for (long long i = 0; i < oldString.Length(); i++) {
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

		size_t capacity = 16;
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
			if (finalLen > len)
			{
				EnsureAlloced(finalLen + 2, true); // data[] is now the size of "finalLen + 2"
				long long diff = finalLen - len;
				long long i, j, k;
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
				long long i, j, k;
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
#endif
	return false;
}

/*
============
cweeStr::Replace
============
*/
void cweeStr::Replace(const std::vector<cweeStr>& olds, const std::vector<cweeStr>& news) {
	const size_t numOlds = olds.size();
	std::vector<long long> counts; for (auto& old : olds) counts.push_back(0); size_t overall = 0;
	long long oldLen = 0, newLen = 0;
	long long i = 0, j = 0, k = 0;

	// Work out how big the new string will be
	for (i = 0; i < Length(); i++) {
		for (k = 0; k < numOlds; k++) {
			if (cweeStr::Cmpn(&data[i], olds[k], olds[k].Length()) == 0) {
				counts[k]++;
				overall++;
				i += olds[k].Length() - 1;
				break; // early exit to the first for-loop
			}
		}
	}

#if 0
	if (overall > 0) {
		cweeStr oldString(data);

		size_t finalLen = len;
		for (long long k = 0; k < numOlds; k++) {
			finalLen += ((news[k].Length() - olds[k].Length()) * counts[k]);
		}
		EnsureAlloced(finalLen + 2, false);

		j = 0;
		// Replace the old data with the new data
		for (i = 0; i < oldString.Length(); i++) {
			bool found = false;
			for (k = 0; k < numOlds; k++) {
				oldLen = olds[k].Length();
				if (cweeStr::Cmpn(&oldString[(size_t)i], olds[k], oldLen) == 0) {
					newLen = news[k].Length();
					found = true;
					while (j + newLen >= finalLen) {
						EnsureAlloced(j + newLen + (oldString.Length() - i) + 2, true);
						finalLen = j + newLen + (oldString.Length() - i) + 2;
					}

					memcpy(data + j, news[k], newLen);

					i += oldLen - 2;
					j += newLen - 1;
					break; // exit to i-based for-loop
				}
			}

			if (!found) {
				data[j] = oldString[(size_t)i];
				j++;
			}
		}
		data[j] = 0;
		len = strlen(data);
	}
#else
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
#if 0
			cweeStr oldString(data);
			EnsureAlloced(finalLen + 2, true);
			j = 0;
			// Replace the old data with the new data
			for (i = 0; i < oldString.Length(); i++) {
				bool found = false;
				for (k = 0; k < numOlds; k++) {
					oldLen = olds[k].Length();
					if (cweeStr::Cmpn(&oldString[(size_t)i], olds[k], oldLen) == 0) {
						newLen = news[k].Length();
						found = true;
						while (j + newLen >= finalLen) {
							EnsureAlloced(j + newLen + (oldString.Length() - i) + 2, true);
							finalLen = j + newLen + (oldString.Length() - i) + 2;
						}

						memcpy(data + j, news[k], newLen);

						i += oldLen - 2;
						j += newLen - 1;
						break; // exit to i-based for-loop
					}
				}

				if (!found) {
					data[j] = oldString[(size_t)i];
					j++;
				}
			}
			data[j] = 0;
			len = strlen(data);
#else
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
#endif

		}





	}
#endif


};

/*
============
cweeStr::StripTrailingWhitespace
============
*/
void cweeStr::StripTrailingWhitespace() {
	long long i;

	// cast to unsigned char to prevent stripping off high-ASCII characters
	for (i = Length(); i > 0 && (unsigned char)(data[i - 1]) <= ' '; i--) {
		data[i - 1] = '\0';
		len--;
	}
}

/*
============
cweeStr::StripQuotes

Removes the quotes from the beginning and end of the string
============
*/
cweeStr& cweeStr::StripQuotes()
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
}

/*
=====================================================================

  filename methods

=====================================================================
*/

/*
============
cweeStr::FileNameHash
============
*/
size_t cweeStr::FileNameHash() const {
	size_t		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (data[i] != '\0') {
		letter = cweeStr::ToLower(data[i]);
		if (letter == '.') {
			break;				// don't include extension
		}
		if (letter == '\\') {
			letter = '/';
		}
		hash += (long)(letter) * (i + 119);
		i++;
	}
	hash &= (FILE_HASH_SIZE - 1);
	return hash;
}

/*
============
cweeStr::BackSlashesToSlashes
============
*/
cweeStr& cweeStr::BackSlashesToSlashes() {
	size_t i;

	for (i = 0; i < len; i++) {
		if (data[i] == '\\') {
			data[i] = '/';
		}
	}
	return *this;
}

/*
============
cweeStr::SlashesToBackSlashes
============
*/
cweeStr& cweeStr::SlashesToBackSlashes() {
	size_t i;

	for (i = 0; i < len; i++) {
		if (data[i] == '/') {
			data[i] = '\\';
		}
	}
	return *this;
}

/*
============
cweeStr::SetFileExtension
============
*/
cweeStr& cweeStr::SetFileExtension(const char* extension) {
	StripFileExtension();
	if (*extension != '.') {
		Append('.');
	}
	Append(extension);
	return *this;
}

/*
============
cweeStr::StripFileExtension
============
*/
cweeStr& cweeStr::StripFileExtension() {
	long long i;

	for (i = ((long long)len) - 1; i >= 0; i--) {
		if (data[i] == '.') {
			data[i] = '\0';
			len = i;
			break;
		}
	}
	return *this;
}

/*
============
cweeStr::StripAbsoluteFileExtension
============
*/
cweeStr& cweeStr::StripAbsoluteFileExtension() {
	long long i;

	for (i = 0; i < len; i++) {
		if (data[i] == '.') {
			data[i] = '\0';
			len = i;
			break;
		}
	}

	return *this;
}

/*
==================
cweeStr::DefaultFileExtension
==================
*/
cweeStr& cweeStr::DefaultFileExtension(const char* extension) {
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
}

/*
==================
cweeStr::DefaultPath
==================
*/
cweeStr& cweeStr::DefaultPath(const char* basepath) {
	if (((*this)[0] == '/') || ((*this)[0] == '\\')) {
		// absolute path location
		return *this;
	}

	*this = basepath + *this;
	return *this;
}

/*
====================
cweeStr::AppendPath
====================
*/
void cweeStr::AppendPath(const char* text) {
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
}

/*
==================
cweeStr::StripFilename
==================
*/
cweeStr& cweeStr::StripFilename() {
	int pos;

	pos = Length() - 1;
	while ((pos > 0) && ((*this)[pos] != '/') && ((*this)[pos] != '\\')) {
		pos--;
	}

	if (pos < 0) {
		pos = 0;
	}

	CapLength(pos);
	return *this;
}

/*
==================
cweeStr::StripPath
==================
*/
cweeStr& cweeStr::StripPath() {
	int pos;

	pos = Length();
	while ((pos > 0) && ((*this)[pos - 1] != '/') && ((*this)[pos - 1] != '\\')) {
		pos--;
	}

	*this = Right(Length() - pos);
	return *this;
}

/*
====================
cweeStr::ExtractFilePath
====================
*/
void cweeStr::ExtractFilePath(cweeStr& dest) const {
	int pos;

	//
	// back up until a \ or the start
	//
	pos = Length();
	while ((pos > 0) && ((*this)[pos - 1] != '/') && ((*this)[pos - 1] != '\\')) {
		pos--;
	}

	Left(pos, dest);
}

/*
====================
cweeStr::ExtractFileName
====================
*/
void cweeStr::ExtractFileName(cweeStr& dest) const {
	int pos;

	//
	// back up until a \ or the start
	//
	pos = Length() - 1;
	while ((pos > 0) && ((*this)[pos - 1] != '/') && ((*this)[pos - 1] != '\\')) {
		pos--;
	}

	Right(Length() - pos, dest);
}

/*
====================
cweeStr::ExtractFileBase
====================
*/
void cweeStr::ExtractFileBase(cweeStr& dest) const {
	int pos;
	long long start;

	//
	// back up until a \ or the start
	//
	pos = Length() - 1;
	while ((pos > 0) && ((*this)[pos - 1] != '/') && ((*this)[pos - 1] != '\\')) {
		pos--;
	}

	start = pos;
	while ((pos < Length()) && ((*this)[pos] != '.')) {
		pos++;
	}

	Mid(start, pos - start, dest);
}

/*
====================
cweeStr::ExtractFileExtension
====================
*/
void cweeStr::ExtractFileExtension(cweeStr& dest) const {
	int pos;

	//
	// back up until a . or the start
	//
	pos = Length() - 1;
	while ((pos > 0) && ((*this)[pos - 1] != '.')) {
		pos--;
	}

	if (!pos) {
		// no extension
		dest.Empty();
	}
	else {
		Right(Length() - pos, dest);
	}
}


/*
=====================================================================

  char * methods to replace library functions

=====================================================================
*/

/*
============
cweeStr::IsNumeric

Checks a string to see if it contains only numerical values.
============
*/
bool cweeStr::IsNumeric(const char* s) {
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
}

/*
============
cweeStr::HasLower

Checks if a string has any lowercase chars
============
*/
bool cweeStr::HasLower(const char* s) {
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
}

/*
============
cweeStr::HasUpper

Checks if a string has any uppercase chars
============
*/
bool cweeStr::HasUpper(const char* s) {
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
}

/*
================
cweeStr::Cmp
================
*/
long long cweeStr::Cmp(const char* s1, const char* s2) {
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
}

/*
================
cweeStr::Cmpn
================
*/
long long cweeStr::Cmpn(const char* s1, const char* s2, long long n) {
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
}

/*
================
cweeStr::Icmp
================
*/
long long cweeStr::Icmp(const char* s1, const char* s2) {
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
}

/*
================
cweeStr::Icmpn
================
*/
long long cweeStr::Icmpn(const char* s1, const char* s2, long long n) {
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
}

/*
================
cweeStr::IcmpPath
================
*/
long long cweeStr::IcmpPath(const char* s1, const char* s2) {
	long long c1, c2, d;

#if 0
	//#if !defined( ID_PC_WIN )
	idLib::common->Printf("WARNING: IcmpPath used on a case-sensitive filesystem?\n");
#endif
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
}

/*
================
cweeStr::IcmpnPath
================
*/
long long cweeStr::IcmpnPath(const char* s1, const char* s2, long long n) {
	long long c1, c2, d;

#if 0
	//#if !defined( ID_PC_WIN )
	idLib::common->Printf("WARNING: IcmpPath used on a case-sensitive filesystem?\n");
#endif

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
}

/*
=============
cweeStr::Copynz

Safe strncpy that ensures a trailing zero
=============
*/
void cweeStr::Copynz(char* dest, const char* src, size_t destsize) {
	if (!src) {
		return;
	}
	if (destsize < 1) {
		return;
	}

	strncpy(dest, src, destsize - 1);
	dest[destsize - 1] = 0;
}

/*
========================
cweeStr::IsValidUTF8
========================
*/
bool cweeStr::IsValidUTF8(const uint8* s, const size_t maxLen, utf8Encoding_t& encoding) {
	struct local_t {
		static size_t GetNumEncodedUTF8Bytes(const uint8 c) {
			if (c < 0x80) {
				return 1;
			}
			else if ((c >> 5) == 0x06) {
				// 2 byte encoding - the next byte must begin with
				return 2;
			}
			else if ((c >> 4) == 0x0E) {
				// 3 byte encoding
				return 3;
			}
			else if ((c >> 5) == 0x1E) {
				// 4 byte encoding
				return 4;
			}
			// this isnt' a valid UTF-8 precursor character
			return 0;
		}
		static bool RemainingCharsAreUTF8FollowingBytes(const uint8* s, const size_t curChar, const size_t maxLen, const size_t num) {
			if (maxLen - curChar < num) {
				return false;
			}
			for (long long i = curChar + 1; i <= curChar + num; i++) {
				if (s[i] == '\0') {
					return false;
				}
				if ((s[i] >> 6) != 0x02) {
					return false;
				}
			}
			return true;
		}
	};

	// check for byte-order-marker
	encoding = UTF8_PURE_ASCII;
	utf8Encoding_t utf8Type = UTF8_ENCODED_NO_BOM;
	if (maxLen > 3 && s[0] == 0xEF && s[1] == 0xBB && s[2] == 0xBF) {
		utf8Type = UTF8_ENCODED_BOM;
	}

	for (long long i = 0; s[i] != '\0' && i < maxLen; i++) {
		size_t numBytes = local_t::GetNumEncodedUTF8Bytes(s[i]);
		if (numBytes == 1) {
			continue;	// just low ASCII
		}
		else if (numBytes == 2) {
			// 2 byte encoding - the next byte must begin with bit pattern 10
			if (!local_t::RemainingCharsAreUTF8FollowingBytes(s, i, maxLen, 1)) {
				return false;
			}
			// skip over UTF-8 character
			i += 1;
			encoding = utf8Type;
		}
		else if (numBytes == 3) {
			// 3 byte encoding - the next 2 bytes must begin with bit pattern 10
			if (!local_t::RemainingCharsAreUTF8FollowingBytes(s, i, maxLen, 2)) {
				return false;
			}
			// skip over UTF-8 character
			i += 2;
			encoding = utf8Type;
		}
		else if (numBytes == 4) {
			// 4 byte encoding - the next 3 bytes must begin with bit pattern 10
			if (!local_t::RemainingCharsAreUTF8FollowingBytes(s, i, maxLen, 3)) {
				return false;
			}
			// skip over UTF-8 character
			i += 3;
			encoding = utf8Type;
		}
		else {
			// this isnt' a valid UTF-8 character
			if (utf8Type == UTF8_ENCODED_BOM) {
				encoding = UTF8_INVALID_BOM;
			}
			else {
				encoding = UTF8_INVALID;
			}
			return false;
		}
	}
	return true;
}

/*
========================
cweeStr::UTF8Length
========================
*/
size_t cweeStr::UTF8Length(const byte* s) {
	size_t mbLen = 0;
	size_t charLen = 0;
	while (s[mbLen] != '\0') {
		uint32 cindex;
		cindex = s[mbLen];
		if (cindex < 0x80) {
			mbLen++;
		}
		else {
			size_t trailing = 0;
			if (cindex >= 0xc0) {
				static const byte trailingBytes[64] = {
					1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
					2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
				};
				trailing = trailingBytes[cindex - 0xc0];
			}
			mbLen += trailing + 1;
		}
		charLen++;
	}
	return charLen;
}


/*
========================
cweeStr::AppendUTF8Char
========================
*/
void cweeStr::AppendUTF8Char(uint32 c) {
	if (c < 0x80) {
		Append((char)c);
	}
	else if (c < 0x800) { // 11 bits
		Append((char)(0xC0 | (c >> 6)));
		Append((char)(0x80 | (c & 0x3F)));
	}
	else if (c < 0x10000) { // 16 bits
		Append((char)(0xE0 | (c >> 12)));
		Append((char)(0x80 | ((c >> 6) & 0x3F)));
		Append((char)(0x80 | (c & 0x3F)));
	}
	else if (c < 0x200000) {	// 21 bits
		Append((char)(0xF0 | (c >> 18)));
		Append((char)(0x80 | ((c >> 12) & 0x3F)));
		Append((char)(0x80 | ((c >> 6) & 0x3F)));
		Append((char)(0x80 | (c & 0x3F)));
	}
	else {
		// UTF-8 can encode up to 6 bytes. Why don't we support that?
		// This is an invalid Unicode character
		Append('?');
	}
}

/*
========================
cweeStr::UTF8Char
========================
*/
uint32 cweeStr::UTF8Char(const byte* s, size_t& idx) {
	if (idx >= 0) {
		while (s[idx] != '\0') {
			uint32 cindex = s[idx];
			if (cindex < 0x80) {
				idx++;
				return cindex;
			}
			int trailing = 0;
			if (cindex >= 0xc0) {
				static const byte trailingBytes[64] = {
					1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
					2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
				};
				trailing = trailingBytes[cindex - 0xc0];
			}
			static const uint32 trailingMask[6] = { 0x0000007f, 0x0000001f, 0x0000000f, 0x00000007, 0x00000003, 0x00000001 };
			cindex &= trailingMask[trailing];
			while (trailing-- > 0) {
				cindex <<= 6;
				cindex += s[++idx] & 0x0000003f;
			}
			idx++;
			return cindex;
		}
	}
	idx++;
	return 0;	// return a null terminator if out of range
}

/*
============
cweeStr::vsnPrintf

vsnprintf portability:

C99 standard: vsnprintf returns the number of characters (excluding the trailing
'\0') which would have been written to the final string if enough space had been available
snprintf and vsnprintf do not write more than size bytes (including the trailing '\0')

win32: _vsnprintf returns the number of characters written, not including the terminating null character,
or a negative value if an output error occurs. If the number of characters to write exceeds count, then count
characters are written and -1 is returned and no trailing '\0' is added.

cweeStr::vsnPrintf: always appends a trailing '\0', returns number of characters written (not including terminal \0)
or returns -1 on failure or if the buffer would be overflowed.
============
*/
size_t cweeStr::vsnPrintf(char* dest, size_t size, const char* fmt, va_list argptr) {
	size_t ret;

#undef _vsnprintf
	ret = _vsnprintf(dest, size - 1, fmt, argptr);
#define _vsnprintf	use_cweeStr_vsnPrintf
	dest[size - 1] = '\0';
	if (ret < 0 || ret >= size) {
		return -1;
	}
	return ret;
}

long long			cweeStr::FindChar(const char* str, const char c, long long start, long long end) {
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
}

long long			cweeStr::FindText(const char* str, const char* text, bool casesensitive, long long start, long long end) {
#if 1
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
#elif 1
	size_t l, i, j;
	if (end == -1) {
		end = strlen(str);
	}
	l = end - strlen(text);

	if (casesensitive) {
		for (i = start; i <= l; i++) {
			for (j = 0;; j++) {
				if (!text[j]) return i;
				if (str[i + j] != text[j])
					break;
			}
		}
	}
	else {
		for (i = start; i <= l; i++) {
			for (j = 0;; j++) {
				if (!text[j]) return i;
				if (::toupper(str[i + j]) != ::toupper(text[j])) {
					break;
				}
			}
		}
	}
	return -1;
#elif 1
	size_t l, i, j;

	if (end == -1) {
		end = strlen(str);
	}
	l = end - strlen(text);

	if (casesensitive) {
		for (i = start; i <= l; i++) {
			for (j = 0;; j++) {
				if (!text[j]) return i;
				if (str[i + j] != text[j]) {
					break;
				}
			}
		}
	}
	else {
		for (i = start; i <= l; i++) {
			for (j = 0;; j++) {
				if (!text[j]) return i;
				if (::toupper(str[i + j]) != ::toupper(text[j])) {
					break;
				}
			}
		}
	}
	return -1;
#else
	size_t l, i, j;

	if (end == -1) {
		end = strlen(str);
	}
	l = end - strlen(text);
	for (i = start; i <= l; i++) {
		if (casesensitive) {
			for (j = 0; text[j]; j++) {
				if (str[i + j] != text[j]) {
					break;
				}
			}
		}
		else {
			for (j = 0; text[j]; j++) {
				if (::toupper(str[i + j]) != ::toupper(text[j])) {
					break;
				}
			}
		}
		if (!text[j]) {
			return i;
		}
	}
	return -1;
#endif
}

/*
============
sprintf

Sets the value of the string using a printf size_terface.
============
*/
size_t sprintf(cweeStr& string, const char* fmt, ...) {
	size_t l;
	va_list argptr;
	char buffer[32000];

	va_start(argptr, fmt);
	l = cweeStr::vsnPrintf(buffer, sizeof(buffer) - 1, fmt, argptr);
	va_end(argptr);
	buffer[sizeof(buffer) - 1] = '\0';

	string = buffer;
	return l;
}

/*
============
sprintf

Sets the value of the string using a printf size_terface.
============
*/
cweeStr cweeStr::printf(const char* fmt, ...) {
	size_t l;
	va_list argptr;
	char buffer[128000];

	va_start(argptr, fmt);
	l = cweeStr::vsnPrintf(buffer, sizeof(buffer) - 1, fmt, argptr);
	va_end(argptr);
	buffer[sizeof(buffer) - 1] = '\0';

	return cweeStr(buffer);
}

/*
============
odsprintf

Sets the value of the string using a printf size_terface then prsize_ts to output console
============
*/
size_t  cweeStr::odsprintf(cweeStr in) {
	char buffer[32000];
	std::strcpy(buffer, in.c_str());
	OutputDebugString(buffer);
	return 1;
}

/*
============
vsprintf

Sets the value of the string using a vprintf size_terface.
============
*/
size_t vsprintf(cweeStr& string, const char* fmt, va_list argptr) {
	size_t l;
	char buffer[32000];

	l = cweeStr::vsnPrintf(buffer, sizeof(buffer) - 1, fmt, argptr);
	buffer[sizeof(buffer) - 1] = '\0';

	string = buffer;
	return l;
}

/*
============
va

does a varargs printf size_to a temp buffer
NOTE: not thread safe
============
*/
char* va(const char* fmt, ...) {
	va_list argptr;
	static size_t index = 0;
	static char string[4][16384];	// in case called by nested functions
	char* buf;

	buf = string[index];
	index = (index + 1) & 3;

	va_start(argptr, fmt);
	vsprintf(buf, fmt, argptr);
	va_end(argptr);

	return buf;
}



/*
============
cweeStr::BestUnit
============
*/
size_t cweeStr::BestUnit(const char* format, float value, Measure_t measure) {
	long long unit = 1;
	while (unit <= 3 && (1 << (unit * 10) < value)) {
		unit++;
	}
	unit--;
	value /= 1 << (unit * 10);
	sprintf(*this, format, value);
	*this += " ";
	*this += units[measure][unit];
	return unit;
}

/*
============
cweeStr::SetUnit
============
*/
void cweeStr::SetUnit(const char* format, float value, size_t unit, Measure_t measure) {
	value /= 1 << (unit * 10);
	sprintf(*this, format, value);
	*this += " ";
	*this += units[measure][unit];
}

/*
================
cweeStr::InitMemory
================
*/
void cweeStr::InitMemory() {
#ifdef USE_STRING_DATA_ALLOCATOR

	stringDataAllocator.Lock();
	auto ptr = stringDataAllocator.UnsafeRead();
	ptr->Init();
	ptr->SetLockMemory(true);
	//ptr->SetLockMemory(false);
	stringDataAllocator.Unlock();
#endif
}

/*
================
cweeStr::ShutdownMemory
================
*/
void cweeStr::ShutdownMemory() {
#ifdef USE_STRING_DATA_ALLOCATOR
	stringDataAllocator.Lock();
	auto ptr = stringDataAllocator.UnsafeRead();
	ptr->Shutdown();
	stringDataAllocator.Unlock();

#endif
}

/*
================
cweeStr::PurgeMemory
================
*/
void cweeStr::PurgeMemory() {
#ifdef USE_STRING_DATA_ALLOCATOR
	stringDataAllocator.Lock();
	auto ptr = stringDataAllocator.UnsafeRead();
	ptr->FreeEmptyBaseBlocks();
	stringDataAllocator.Unlock();
#endif
}

/*
================
cweeStr::FormatNumber
================
*/
struct formatList_t {
	size_t			gran;
	size_t			count;
};

// elements of list need to decend in size
formatList_t formatList[] = {
	{ 1000000000, 0 },
	{ 1000000, 0 },
	{ 1000, 0 }
};

size_t numFormatList = sizeof(formatList) / sizeof(formatList[0]);


cweeStr cweeStr::FormatNumber(size_t number) {
	cweeStr string;
	bool hit;

	// reset
	for (long long i = 0; i < numFormatList; i++) {
		formatList_t* li = formatList + i;
		li->count = 0;
	}

	// main loop
	do {
		hit = false;

		for (long long i = 0; i < numFormatList; i++) {
			formatList_t* li = formatList + i;

			if (number >= li->gran) {
				li->count++;
				number -= li->gran;
				hit = true;
				break;
			}
		}
	} while (hit);

	// prsize_t out
	bool found = false;

	for (long long i = 0; i < numFormatList; i++) {
		formatList_t* li = formatList + i;

		if (li->count) {
			if (!found) {
				string += va("%i,", li->count);
			}
			else {
				string += va("%3.3i,", li->count);
			}
			found = true;
		}
		else if (found) {
			string += va("%3.3i,", li->count);
		}
	}

	if (found) {
		string += va("%3.3i", number);
	}
	else {
		string += va("%i", number);
	}

	// pad to proper size
	long long count = 11 - string.Length();

	for (long long i = 0; i < count; i++) {
		string.Insert(" ", 0);
	}

	return string;
}

int cweeLevenshteinDistance(const cweeStr& a, const cweeStr& b, bool caseSensitive = false)
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

int		cweeStr::Levenshtein(const cweeStr& other) const {
	return cweeLevenshteinDistance(*this, other);
}

cweeStr	cweeStr::BestMatch(std::vector<cweeStr> list) const {
	cweeStr out;
	int minL = std::numeric_limits<int>::max();
	const cweeStr target = *this;
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
cweeStr	cweeStr::iBestMatch(std::vector<cweeStr> list) const {
	cweeStr out;
	int minL = std::numeric_limits<int>::max();
	cweeStr target = *this;
	target.ToLower();
	for (auto& x : list) {
		x.ToLower();
		int i = x.Levenshtein(target);
		if (i <= minL) {
			minL = i;
			out = x;
			if (i == 0) return out;
		}
	}
	return out;
};



int	cweeStrView::Levenshtein(const cweeStr& other) const {
	return cweeLevenshteinDistance(*this, other);
};

cweeStr	cweeStrView::BestMatch(std::vector<cweeStr> list) const {
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














