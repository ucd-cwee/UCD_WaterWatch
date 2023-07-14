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
#include "Iterator.h"
#include "List.h"

class cweeParser {
public:
	struct it_state {
		int pos;
		inline void begin(const cweeParser* ref) { pos = 0; }
		inline void next(const cweeParser* ref) { ++pos; }
		inline void end(const cweeParser* ref) { pos = ref->argV.Num(); }
		inline cweeStr& get(cweeParser* ref) { return ref->argV[pos]; }
		inline bool cmp(const it_state& s) const { return pos != s.pos; }
		inline long long distance(const it_state& s) const { return pos - s.pos; };
		// Optional to allow operator--() and reverse iterators:
		inline void prev(const cweeParser* ref) { --pos; }
		// Optional to allow `const_iterator`:
		inline const cweeStr& get(const cweeParser* ref) const { return ref->argV[pos]; }
	};
	SETUP_STL_ITERATOR(cweeParser, cweeStr, it_state);


	void Clear() { argV.Clear(); };




	cweeParser() {};
	~cweeParser() {};
	cweeParser(const cweeStr& text) {
		processText(text);
	};
	//cweeParser(cweeStr& text, char delimiter) { 
	//	processText(text, cweeStr(delimiter)); 
	//};
	cweeParser(const cweeStr& text, const cweeStr& delimiter, const bool& noCopy = false) {
		if (noCopy == false) processText(text, delimiter);
		else processTextFast(text, delimiter);
	};
	cweeParser(const cweeStr& text, const char& delimiter, const bool& noCopy = false) {
		if (noCopy == false) processText(text, cweeStr(delimiter));
		else processTextFast(text, cweeStr(delimiter));
	};
	cweeParser(const char* text, const cweeStr& delimiter, const bool& noCopy = false) {
		if (noCopy == false) processText(text, delimiter);
		else processTextFast(text, delimiter);
	};

	friend bool			operator==(const cweeParser& a, const cweeParser& b) {
		if (!(a.argV.Num() == b.argV.Num()))
			return false;

		for (int i = 0; i < a.argV.Num(); i++) {
			for (int j = 0; j < b.argV.Num(); j++) {
				if (a.argV[i] != b.argV[j])
					return false;
			}
		}

		return true;
	};

	cweeStr				operator[](int index) const {
		return getVar(index);
	};
	cweeStr& operator[](int index) {
		return argV[index];
	};

	operator cweeThreadedList<cweeStr>() {
		return argV;
	};
	operator cweeThreadedList<cweeStr>() const {
		return argV;
	};

	void Parse(const cweeStr& text, const cweeStr& delimiter, const bool& noCopy = false) {
		if (noCopy == false) processText(text, delimiter);
		else processTextFast(text, delimiter);
	};
	void ParseFirstDelimiterOnly(const cweeStr& text, const cweeStr& delimiter) {
		processTextFast_FirstDelimiterOnly(text, delimiter);
	};
	std::vector<std::string> GetEveryOtherVar() const {
		std::vector<std::string> out;
		for (int i = 1; i < getNumVars(); i += 2) {
			out.push_back(argV[i].c_str());
		}
		return out;
	};

	cweeParser& Trim(char c) {
		for (auto& x : argV) {
			x.Strip(c);
		}
		return *this;
	};
	cweeParser& ReplaceInline(cweeStr old, cweeStr n) {
		for (auto& x : argV) {
			x.ReplaceInline(old, n);
		}
		return *this;
	};
	cweeParser SplitAgain(cweeStr delim) const {
		cweeParser out;
		for (auto& x : *this) {
			cweeParser temp;
			temp.Parse(x, delim, true);
			for (auto& y : temp) {
				out.argV.Append(y);
			}
		}
		return out;
	};
	int getNumVars() { return argV.Num(); }
	int getNumVars() const { return argV.Num(); }
	//cweeStr getVar(int i) { if (i < argV.Num()) return argV[i]; else return ""; }
	//cweeStr getVar(int i) const { if (i < argV.Num()) return argV[i]; else return ""; }
	cweeStr& getVar(int i) { return argV[i]; }
	cweeStr getVar(int i) const { return argV[i]; }

	cweeThreadedList<cweeStr>& getVars() { return argV; }
private:
	void			processTextFast_FirstDelimiterOnly(const cweeStr& text, const cweeStr& delimiter) { // used for complex delimiters like ':cweeJunctionDelimiter:'
		argV.Clear();
		argV.SetGranularity(64);

		int i1(0), i2(0), i3(0);
		const int delimLen = delimiter.Length();

		cweeStr currentWord;
		while (1) {
			i2 = text.Find(delimiter, true, i1);
			text.Mid(i1, ((i2 <= 0 ? text.Length() : i2) - i1), currentWord);

			i3 = currentWord.Find(delimiter, true);
			if (i3 >= 0) currentWord = currentWord.Left(i3);

			if ((argV.Num() + 1) >= argV.GetGranularity()) argV.SetGranularity(argV.GetGranularity() * GRANULARITY_SCALER);
			argV.Append(currentWord);

			if (i2 < 0) break;
			i1 = i2 + delimLen;

			argV.Append(text.Mid(i1, text.Length()));
			break;
		}
	};

	void			processTextFast(const cweeStr& text, const cweeStr& delimiter) { // used for complex delimiters like ':cweeJunctionDelimiter:'
#if 1
		if (delimiter.Length() <= 0) {
			argV.Clear();
			argV.Append(text);
			return;
		}

		if (argV.Num() == 0) {
			argV.SetGranularity(64);

			long long i1, i2, i3;
			i1 = 0;
			i2 = 0;
			i3 = 0;
			const long long delimLen = delimiter.Length();

			cweeStr currentWord;
			while (1) {
				i2 = text.Find(delimiter, true, i1);
				text.Mid(i1, ((i2 <= 0 ? text.Length() : i2) - i1), currentWord);

				i3 = currentWord.Find(delimiter, true);
				if (i3 >= 0) currentWord = currentWord.Left(i3);

				if ((argV.Num() + 1) >= argV.GetGranularity()) argV.SetGranularity(argV.GetGranularity() * GRANULARITY_SCALER);
				argV.Append(currentWord);

				if (i2 < 0) break;
				i1 = i2 + delimLen;
			}
		}
		else {
			// we have initialized before. Take advantage of this. 
			long long i1(0), i2(0), i3(0);
			const long long delimLen = delimiter.Length();
			cweeStr currentWord; int append(0);
			while (1) {
				i2 = text.Find(delimiter, true, i1);
				text.Mid(i1, ((i2 <= 0 ? text.Length() : i2) - i1), currentWord);

				i3 = currentWord.Find(delimiter, true);
				if (i3 >= 0) currentWord = currentWord.Left(i3);

				if (append >= argV.Num()) {
					if ((argV.Num() + 1) >= argV.GetGranularity()) argV.SetGranularity(argV.GetGranularity() * GRANULARITY_SCALER);
					argV.Append(currentWord);
					append++;
				}
				else {
					argV[append] = currentWord;
					append++;
				}

				if (i2 < 0) break;
				i1 = i2 + delimLen;
			}
			// ensure we don't leave strings in the list from a previous parsing. 
			argV.Resize(append);
		}

#else
		argV.Clear();
		argV.SetGranularity(64);

		int i1(0), i2(0), i3(0);
		const int delimLen = delimiter.Length();

		cweeStr currentWord;
		while (1) {
			i2 = text.Find(delimiter, true, i1);
			text.Mid(i1, ((i2 <= 0 ? text.Length() : i2) - i1), currentWord);

			i3 = currentWord.Find(delimiter, true);
			if (i3 >= 0) currentWord = currentWord.Left(i3);

			if ((argV.Num() + 1) >= argV.GetGranularity()) argV.SetGranularity(argV.GetGranularity() * GRANULARITY_SCALER);
			argV.Append(currentWord);

			if (i2 < 0) break;
			i1 = i2 + delimLen;
		}
#endif
	};
	void			processText(const cweeStr& text, const cweeStr& delimiter) { // used for complex delimiters like ':cweeJunctionDelimiter:'
		if (delimiter.Length() <= 0) {
			argV.Clear();
			argV.Append(text);
			return;
		}

		argV.Clear();
		argV.SetGranularity(10000);

		int i2(0);
		const int delimLen = delimiter.Length();
		cweeStr currentWord;
		cweeStr remainingWord(text);
		remainingWord.ReduceSpaces(true);

		while (1) {
			i2 = remainingWord.Find(delimiter);
			currentWord.CopyRange(remainingWord, 0, i2);
			argV.Append(currentWord);
			remainingWord.CopyRange(remainingWord, i2 + delimLen, remainingWord.Length()); // instead of plus one, do range of delim
			if (i2 < 0) break;
		}

		// need to go through and fix a glitch.
		int i3(0);
		int limit = argV.Num();
		for (int i = 0; i < limit; i++) {
			i3 = argV[i].Find(delimiter);
			if (i3 >= 0) argV[i] = argV[i].Left(i3);
		}
	};
	void			processText(const cweeStr& text, const char& delimiter) {
		argV.Clear();
		argV.SetGranularity(1000);

		bool escape(false);
		int i1(0);
		int i2(0);
		cweeStr currentWord(text), remainingWord(text);
		remainingWord.ReduceSpaces((delimiter == '\n'));

		while (!escape) {
			i2 = remainingWord.Find(delimiter);
			if (i2 < 0)
				escape = true;
			currentWord.CopyRange(remainingWord.c_str(), 0, i2);
			currentWord.ReduceSpaces((delimiter == '\n'));
			argV.Append(currentWord);
			remainingWord.CopyRange(remainingWord.c_str(), i2 + 1, remainingWord.Length());
		}

		// need to go through and fix a glitch.
		for (int i = 0; i < argV.Num(); i++) {
			int i3 = argV[i].Find(delimiter);
			if (!(i3 < 0)) argV[i] = argV[i].Left(i3);
		}
	};
	void			processText(const cweeStr& text) {
		argV.Clear();
		argV.SetGranularity(1000);

		bool escape(false);
		int i1(0);
		int i2(0);
		cweeStr currentWord(text), remainingWord(text);
		remainingWord.ReduceSpaces();

		while (!escape) {
			i2 = remainingWord.Find(' ');
			if (i2 < 0)
				escape = true;
			currentWord.CopyRange(remainingWord.c_str(), 0, i2);
			argV.Append(currentWord);
			remainingWord.CopyRange(remainingWord.c_str(), i2 + 1, remainingWord.Length());
		}

		// need to go through and fix a glitch.
		for (int i = 0; i < argV.Num(); i++) {
			int i3 = argV[i].Find(' ');
			if (!(i3 < 0)) argV[i] = argV[i].Left(i3);
		}
	};
	cweeThreadedList<cweeStr> argV;
};

class cweeInlineParser {
public:
	struct it_state {
		int pos;
		inline void begin(const cweeInlineParser* ref) { pos = 0; }
		inline void next(const cweeInlineParser* ref) { ++pos; }
		inline void end(const cweeInlineParser* ref) { pos = ref->argV.Num(); }
		inline cweeStrView& get(cweeInlineParser* ref) { return ref->argV[pos]; }
		inline bool cmp(const it_state& s) const { return pos != s.pos; }
		inline long long distance(const it_state& s) const { return pos - s.pos; };
		// Optional to allow operator--() and reverse iterators:
		inline void prev(const cweeInlineParser* ref) { --pos; }
		// Optional to allow `const_iterator`:
		inline const cweeStrView& get(const cweeInlineParser* ref) const { return ref->argV[pos]; }
	};
	SETUP_STL_ITERATOR(cweeInlineParser, cweeStrView, it_state);

	void Clear() { argV.Clear(); };

	cweeInlineParser() {};
	~cweeInlineParser() {};
	cweeInlineParser(cweeStr& text) {
		processTextFast(text.View(), " ");
	};
	cweeInlineParser(cweeStr& text, const cweeStr& delimiter) {
		processTextFast(text.View(), delimiter);
	};
	cweeInlineParser(cweeStr& text, const char& delimiter) {
		processTextFast(text.View(), cweeStr(delimiter));
	};
	cweeInlineParser(const cweeStrView& text) {
		processTextFast(text, " ");
	};
	cweeInlineParser(const cweeStrView& text, const cweeStr& delimiter) {
		processTextFast(text, delimiter);
	};
	cweeInlineParser(const cweeStrView& text, const char& delimiter) {
		processTextFast(text, cweeStr(delimiter));
	};

	friend bool			operator==(const cweeInlineParser& a, const cweeInlineParser& b) {
		if (!(a.argV.Num() == b.argV.Num()))
			return false;

		for (int i = 0; i < a.argV.Num(); i++) {
			for (int j = 0; j < b.argV.Num(); j++) {
				if (a.argV[i] != b.argV[j])
					return false;
			}
		}

		return true;
	};

	const cweeStrView& operator[](int index) const {
		return getVar(index);
	};
	cweeStrView& operator[](int index) {
		return argV[index];
	};

	operator cweeThreadedList<cweeStrView>() {
		return argV;
	};
	operator cweeThreadedList<cweeStrView>() const {
		return argV;
	};

	void Parse(cweeStr& text, const cweeStr& delimiter) {
		processTextFast(text.View(), delimiter);
	};
	void ParseFirstDelimiterOnly(cweeStr& text, const cweeStr& delimiter) {
		processTextFast_FirstDelimiterOnly(text.View(), delimiter);
	};
	void Parse(const cweeStrView& text, const cweeStr& delimiter) {
		processTextFast(text, delimiter);
	};
	void ParseFirstDelimiterOnly(const cweeStrView& text, const cweeStr& delimiter) {
		processTextFast_FirstDelimiterOnly(text, delimiter);
	};

	int getNumVars() { return argV.Num(); }
	int getNumVars() const { return argV.Num(); }
	//cweeStr getVar(int i) { if (i < argV.Num()) return argV[i]; else return ""; }
	//cweeStr getVar(int i) const { if (i < argV.Num()) return argV[i]; else return ""; }
	cweeStrView& getVar(int i) { return argV[i]; }
	const cweeStrView& getVar(int i) const { return argV[i]; }

private:
	void			processTextFast_FirstDelimiterOnly(const cweeStrView& text, const cweeStr& delimiter) { // used for complex delimiters like ':cweeJunctionDelimiter:'
		processTextFast(text, delimiter, 1);
	};

	void			processTextFast(const cweeStrView& text, const cweeStr& delimiter, int maxDelim = std::numeric_limits<int>::max()) { // used for complex delimiters like ':cweeJunctionDelimiter:'
		if (delimiter.Length() <= 0) {
			argV.Clear();
			argV.Append(text);
			return;
		}

		if (argV.Num() == 0) {
			argV.SetGranularity(64);

			long long i1, i2, i3;
			i1 = 0;
			i2 = 0;
			i3 = 0;
			const long long delimLen = delimiter.Length();

			cweeStrView currentWord;
			while (1) {
				if (--maxDelim >= 0) {
					i2 = text.Find(delimiter, true, i1);
					currentWord = text.Mid(i1, ((i2 <= 0 ? text.Length() : i2) - i1));
					i3 = currentWord.Find(delimiter, true);
					if (i3 >= 0) currentWord = currentWord.Left(i3);
				}
				else {
					i2 = -1;
					currentWord = text.Mid(i1, ((i2 <= 0 ? text.Length() : i2) - i1));
				}

				if ((argV.Num() + 1) >= argV.GetGranularity()) argV.SetGranularity(argV.GetGranularity() * GRANULARITY_SCALER);
				argV.Append(currentWord);

				if (i2 < 0) break;
				i1 = i2 + delimLen;
			}
		}
		else {
			// we have initialized before. Take advantage of this. 
			long long i1(0), i2(0), i3(0);
			const long long delimLen = delimiter.Length();
			cweeStrView currentWord; int append(0);
			while (1) {
				if (--maxDelim >= 0) {
					i2 = text.Find(delimiter, true, i1);
					currentWord = text.Mid(i1, ((i2 <= 0 ? text.Length() : i2) - i1));
					i3 = currentWord.Find(delimiter, true);
					if (i3 >= 0) currentWord = currentWord.Left(i3);
				}
				else {
					i2 = -1;
					currentWord = text.Mid(i1, ((i2 <= 0 ? text.Length() : i2) - i1));
				}

				if (append >= argV.Num()) {
					if ((argV.Num() + 1) >= argV.GetGranularity()) argV.SetGranularity(argV.GetGranularity() * GRANULARITY_SCALER);
					argV.Append(currentWord);
					append++;
				}
				else {
					argV[append] = currentWord;
					append++;
				}

				if (i2 < 0) break;
				i1 = i2 + delimLen;
			}
			// ensure we don't leave strings in the list from a previous parsing. 
			argV.Resize(append);
		}
	};

private:
	cweeThreadedList<cweeStrView> argV;

};

INLINE cweeParser cweeStr::Split(const cweeStr& delim) const {
	cweeParser out(*this, delim, true);
	return out;
};