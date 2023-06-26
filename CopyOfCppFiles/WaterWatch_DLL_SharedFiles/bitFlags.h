
#ifndef __BITFLAGS_H__
#define __BITFLAGS_H__

#pragma region "INCLUDES"
#pragma hdrstop
#include "Precompiled.h"
#pragma endregion

/*! Bitwise boolean flag tool. Efficiently get, set, or toggle boolean values for 8 flags (0 - 7). */
class flags {
private: // components
	class BitFlags {
	public:
		typedef unsigned char flag;

		/*!
		Create a new flag object with bits 0 - 7 set to zero.
		*/
		static flag	NewFlag() {
			flag obj{ 0b0000'0000 };
			nullAllFlags(obj);
			return obj;
		};

		/*!
		Get selected bit of flag.
		*/
		static bool	getFlag(const flag& in, int which) {
			if (which > 7 || which < 0) return false;
			bool result = false;
			switch (which) {
			case 0: {
				((in & mask0) ? result = true : result = false);
				break;
			}
			case 1: {
				((in & mask1) ? result = true : result = false);
				break;
			}
			case 2: {
				((in & mask2) ? result = true : result = false);
				break;
			}
			case 3: {
				((in & mask3) ? result = true : result = false);
				break;
			}
			case 4: {
				((in & mask4) ? result = true : result = false);
				break;
			}
			case 5: {
				((in & mask5) ? result = true : result = false);
				break;
			}
			case 6: {
				((in & mask6) ? result = true : result = false);
				break;
			}
			case 7: {
				((in & mask7) ? result = true : result = false);
				break;
			}
			}
			return result;
		};

		/*!
		Set selected bit of flag object to 1.
		*/
		static void	setFlag(flag& in, int which) {
			if (which > 7 || which < 0) return;
			switch (which) {
			case 0: {
				in |= mask0;
				break;
			}
			case 1: {
				in |= mask1;
				break;
			}
			case 2: {
				in |= mask2;
				break;
			}
			case 3: {
				in |= mask3;
				break;
			}
			case 4: {
				in |= mask4;
				break;
			}
			case 5: {
				in |= mask5;
				break;
			}
			case 6: {
				in |= mask6;
				break;
			}
			case 7: {
				in |= mask7;
				break;
			}
			}
		};

		/*!
		Set selected bit of flag object to 0.
		*/
		static void	nullFlag(flag& in, int which) {
			if (which > 7 || which < 0) return;
			switch (which) {
			case 0: {
				in &= ~mask0;
				break;
			}
			case 1: {
				in &= ~mask1;
				break;
			}
			case 2: {
				in &= ~mask2;
				break;
			}
			case 3: {
				in &= ~mask3;
				break;
			}
			case 4: {
				in &= ~mask4;
				break;
			}
			case 5: {
				in &= ~mask5;
				break;
			}
			case 6: {
				in &= ~mask6;
				break;
			}
			case 7: {
				in &= ~mask7;
				break;
			}
			}
		};

		/*!
		Reverse setting of a flag. (0 becomes 1, and 1 becoems 0)
		*/
		static void	toggleFlag(flag& in, int which) {
			if (which > 7 || which < 0) return;
			switch (which) {
			case 0: {
				in ^= mask0;
				break;
			}
			case 1: {
				in ^= mask1;
				break;
			}
			case 2: {
				in ^= mask2;
				break;
			}
			case 3: {
				in ^= mask3;
				break;
			}
			case 4: {
				in ^= mask4;
				break;
			}
			case 5: {
				in ^= mask5;
				break;
			}
			case 6: {
				in ^= mask6;
				break;
			}
			case 7: {
				in ^= mask7;
				break;
			}
			}
		};

		/*!
		Set all Flags to 1.
		*/
		static void	setAllFlags(flag& in) {
			for (int i = 0; i < 8; i++)	setFlag(in, i);
		};

		/*!
		Set all Flags to 0.
		*/
		static void	nullAllFlags(flag& in) {
			for (int i = 0; i < 8; i++)	nullFlag(in, i);
		};

		/*!
		Toggle all Flags.
		*/
		static void	toggleAllFlags(flag& in) {
			for (int i = 0; i < 8; i++)	toggleFlag(in, i);
		};

		static cweeStr Serialize(const flag& in) {
			cweeStr delim = ",";
			cweeStr out;
			out.AddToDelimiter(getFlag(in, 0), delim);
			out.AddToDelimiter(getFlag(in, 1), delim);
			out.AddToDelimiter(getFlag(in, 2), delim);
			out.AddToDelimiter(getFlag(in, 3), delim);
			out.AddToDelimiter(getFlag(in, 4), delim);
			out.AddToDelimiter(getFlag(in, 5), delim);
			out.AddToDelimiter(getFlag(in, 6), delim);
			out.AddToDelimiter(getFlag(in, 7), delim);
			return out;
		};

		static flag Deserialize(const cweeStr& in) {
			cweeParser obj(in, ",", true);
			flag out{ 0b0000'0000 };

			if ((int)obj[0] > 0) setFlag(out, 0);
			if ((int)obj[1] > 0) setFlag(out, 1);
			if ((int)obj[2] > 0) setFlag(out, 2);
			if ((int)obj[3] > 0) setFlag(out, 3);
			if ((int)obj[4] > 0) setFlag(out, 4);
			if ((int)obj[5] > 0) setFlag(out, 5);
			if ((int)obj[6] > 0) setFlag(out, 6);
			if ((int)obj[7] > 0) setFlag(out, 7);

			return out;
		};

	private:
		constexpr const static unsigned char mask0{ 0b0000'0001 }; // represents bit 0
		constexpr const static unsigned char mask1{ 0b0000'0010 }; // represents bit 1
		constexpr const static unsigned char mask2{ 0b0000'0100 }; // represents bit 2 
		constexpr const static unsigned char mask3{ 0b0000'1000 }; // represents bit 3
		constexpr const static unsigned char mask4{ 0b0001'0000 }; // represents bit 4
		constexpr const static unsigned char mask5{ 0b0010'0000 }; // represents bit 5
		constexpr const static unsigned char mask6{ 0b0100'0000 }; // represents bit 6
		constexpr const static unsigned char mask7{ 0b1000'0000 }; // represents bit 7
	};

public: // methods
	/*! get setting of flag (0 - 7). */
	const bool get(int which) const {
		return BitFlags::getFlag(data, which);
	};
	/*! get setting of flag (0 - 7). */
	const bool operator[](int which) const {
		return get(which);
	};
	/*! set setting of flag (0 - 7). */
	void set(int which, bool value) {
		if (value) BitFlags::setFlag(data, which);
		else BitFlags::nullFlag(data, which);
	};
	/*! set setting of flag (0 - 7). */
	const void operator()(int which, bool value) {
		set(which, value);
	};
	/*! toggle setting of flag (0 - 7) to inverse of previous setting. */
	bool toggle(int which) {
		BitFlags::toggleFlag(data, which);
		return get(which);
	};
	/*! set all settings of flags (0 - 7) simultaneously. */
	void setAll(bool value) {
		if (value) BitFlags::setAllFlags(data);
		else BitFlags::nullAllFlags(data);
	};
	/*! toggle all settings of flags (0 - 7) simultaneously to their individual inverse settings. */
	void toggleAll() {
		BitFlags::toggleAllFlags(data);
	};
	/*! get string representation of flags (0 - 7). */
	cweeStr Serialize() const {
		return BitFlags::Serialize(data);
	};
	/*! return values to match the serialized source. */
	void Deserialize(const cweeStr& in) {
		data = BitFlags::Deserialize(in);
	};

private: // data
	/*! data source. */
	BitFlags::flag data = BitFlags::NewFlag();

private: // example
	/*! Programmer's demonstration of the use of this class. */
	void example() {
		flags info;

		if (info[0]) assert("Flag should initialize to 0");

		info.set(0, true);
		if (!info[0]) assert("Flag should have been set to 1");

		if (info.toggle(0)) assert("Flag should have been toggled to 0");

		info.setAll(true);
		if (!info[0]) assert("All Flags should have been toggled to 1");

		info.Deserialize(info.Serialize());
		if (!info[0]) assert("All Flags should have maintained values==1 after serialization/deserialization");

		info = flags();
		if (info[0]) assert("Flag should initialize to 0");
	};

};

#endif