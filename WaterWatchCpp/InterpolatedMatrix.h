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
#include "Parser.h"
#include "BalancedPattern.h"
#include "Pattern.h"
#include "Mutex.h"
#include "InterlockedValues.h"

template <typename T> class cweeInterpolatedMatrix {
public:
	class xyContainer {
	public:
		double			x = 0;
		double			y = 0;
		T				z = 0;

		xyContainer(void) {
		};
		xyContainer(const xyContainer& inbound) {
			this->x = inbound.x;
			this->y = inbound.y;
			this->z = inbound.z;
		};

		explicit		xyContainer(const cweeStr& a) {
			cweeParser obj(a, ",", true);
			if (obj.getNumVars() >= 3)
			{
				x = (double)obj[0];
				y = (double)obj[1];
				z = T(obj[2]);
			}
		};
		explicit		xyContainer(const double& a) {
			z = a;
		};
		explicit		xyContainer(const double& x, const double& y, const T& z) {
			this->x = x;
			this->y = y;
			this->z = z;
		};

		void 			Set(const double& x, const double& y, const T& z) {
			this->x = x;
			this->y = y;
			this->z = z;
		};
		void			Zero(void) {
			x = 0.0;
			y = 0.0;
			z = 0.0;
		};

		explicit operator T() const {
			return z;
		};
		explicit operator T() {
			return z;
		};

		xyContainer			operator-() const { return xyContainer(-x, -y, -1.0 * z); };

		xyContainer& operator-(const xyContainer& a) { x -= a.x; y -= a.y; z -= a.z; return *this; };
		xyContainer& operator+(const xyContainer& a) { x += a.x; y += a.y; z += a.z; return *this; };
		xyContainer& operator/(const xyContainer& a) { x /= a.x; y /= a.y; z /= a.z; return *this; };
		xyContainer& operator*(const xyContainer& a) { x *= a.x; y *= a.y; z *= a.z; return *this; };
		xyContainer& operator-=(const xyContainer& a) { x -= a.x; y -= a.y; z -= a.z; return *this; };
		xyContainer& operator+=(const xyContainer& a) { x += a.x; y += a.y; z += a.z; return *this; };
		xyContainer& operator/=(const xyContainer& a) { x /= a.x; y /= a.y; z /= a.z; return *this; };
		xyContainer& operator*=(const xyContainer& a) { x *= a.x; y *= a.y; z *= a.z; return *this; };

		xyContainer& operator-(const double& a) { x -= a; y -= a; z -= a; return *this; };
		xyContainer& operator+(const double& a) { x += a; y += a; z += a; return *this; };
		xyContainer& operator/(const double& a) { x /= a; y /= a; z /= a; return *this; };
		xyContainer& operator*(const double& a) { x *= a; y *= a; z *= a; return *this; };
		xyContainer& operator-=(const double& a) { x -= a; y -= a; z -= a; return *this; };
		xyContainer& operator+=(const double& a) { x += a; y += a; z += a; return *this; };
		xyContainer& operator/=(const double& a) { x /= a; y /= a; z /= a; return *this; };
		xyContainer& operator*=(const double& a) { x *= a; y *= a; z *= a; return *this; };

		friend xyContainer	operator+(const xyContainer& a, const xyContainer& b) {
			xyContainer result(a);
			result += b;
			return result;
		};
		friend xyContainer	operator+(const xyContainer& a, const double& b) {
			xyContainer result(a);
			result += b;
			return result;
		};
		friend xyContainer	operator+(const double& a, const xyContainer& b) {
			xyContainer result(b);
			result += a;
			return result;
		};

		friend xyContainer	operator-(const xyContainer& a, const xyContainer& b) {
			xyContainer result(a);
			result -= b;
			return result;
		};
		friend xyContainer	operator-(const xyContainer& a, const double& b) {
			xyContainer result(a);
			result -= b;
			return result;
		};
		friend xyContainer	operator-(const double& a, const xyContainer& b) {
			xyContainer result(-b);
			result += a;
			return result;
		};

		friend xyContainer	operator*(const xyContainer& a, const xyContainer& b) {
			xyContainer result(a);
			result *= b;
			return result;
		};
		friend xyContainer	operator*(const xyContainer& a, const double& b) {
			xyContainer result(a);
			result *= b;
			return result;
		};
		friend xyContainer	operator*(const double& a, const xyContainer& b) {
			xyContainer result(b);
			result *= a;
			return result;
		};

		friend xyContainer	operator/(const xyContainer& a, const xyContainer& b) {
			xyContainer result(a);
			result /= b;
			return result;
		};
		friend xyContainer	operator/(const xyContainer& a, const double& b) {
			xyContainer result(a);
			result /= b;
			return result;
		};
		friend xyContainer	operator/(const double& a, const xyContainer& b) {
			xyContainer result;
			result.x = 1 / b.x;
			result.y = 1 / b.y;
			result.z = 1 / b.z;

			result *= a;
			return result;
		};
		xyContainer& operator=(const xyContainer& a) {
			this->x = a.x;
			this->y = a.y;
			this->z = a.z;

			return *this;
		};
		xyContainer& operator=(const double& a) {
			x = y = z = a;
			return *this;
		};

		bool			Compare(const xyContainer& a) const {
			return ((x == a.x) && (y == a.y) && (z == a.z));
		};							// exact compare, no epsilon
		bool			operator==(const xyContainer& a) const {
			return Compare(a);
		};						// exact compare, no epsilon
		bool			operator!=(const xyContainer& a) const {
			return !Compare(a);
		};						// exact compare, no epsilon
		bool			operator<(const xyContainer& a) const {
			return ((double)*this) < ((double)a);
		};
		bool			operator<=(const xyContainer& a) const {
			return ((double)*this) <= ((double)a);
		};
		bool			operator>(const xyContainer& a) const {
			return ((double)*this) > ((double)a);
		};
		bool			operator>=(const xyContainer& a) const {
			return ((double)*this) >= ((double)a);
		};

		double			ScalarProjectionOnto(const xyContainer& B) const {
			return (x * B.x + y * B.y) / ::sqrt(B.x * B.x + B.y * B.y);
		};

		vec2d			GetVec2() const {
			return vec2d(x, y);
		};

		double			Distance2d(const xyContainer& a) const {
			double out(0);
			out += ((this->x - a.x) * (this->x - a.x));
			out += ((this->y - a.y) * (this->y - a.y));
			return ::sqrt(out);
		};

		void			Normalize2D() {
			double sqrLength, invLength;

			sqrLength = x * x + y * y;
			invLength = sqrLength == 0 ? 1.0 : 1.0 / ::sqrt(sqrLength);
			x *= invLength;
			y *= invLength;
			z = 0;
		}

		cweeStr			ToString(void) const {
			cweeStr out;
			out = cweeStr::printf("%s,%s,%s",
				cweeStr(x).c_str(),
				cweeStr(y).c_str(),
				cweeStr(z).c_str()
			);
			return out;
		};

		xyContainer& FromString(const cweeStr& v) {
			xyContainer a = xyContainer(v);
			return operator=(a);
		};

		operator		const char* () const {
			return ToString();
		};
		operator		const char* () {
			return ToString();
		};

	};
	using sourceType = cweeBalancedCurve<xyContainer>;

	cweeInterpolatedMatrix() {
		hilbertContainer.SetBoundaryType(boundary_t::BT_CLOSED);
		hilbertContainer.SetInterpolationType(interpolation_t::LINEAR);
		Tag = nullptr;
	};
	cweeInterpolatedMatrix(const cweeInterpolatedMatrix<T>& s) {
		hilbertContainer.SetBoundaryType(boundary_t::BT_CLOSED);
		hilbertContainer.SetInterpolationType(interpolation_t::LINEAR);

		sourceType& sD = s.UnsafeGetSource();
		sD.Lock();
		for (auto* ptr : sD.UnsafeGetKnotSeries()) {
			if (ptr && ptr->object) {
				InsertValue(ptr->object->x, ptr->object->y, ptr->object->z);
			}
		}
		sD.Unlock();

		Tag = s.Tag;
	};
	cweeInterpolatedMatrix& operator=(const cweeInterpolatedMatrix<T>& s) {
		Clear();

		hilbertContainer.SetBoundaryType(boundary_t::BT_CLOSED);
		hilbertContainer.SetInterpolationType(interpolation_t::LINEAR);

		sourceType& sD = s.UnsafeGetSource();
		sD.Lock();
		for (auto* ptr : sD.UnsafeGetKnotSeries()) {
			if (ptr && ptr->object) {
				InsertValue(ptr->object->x, ptr->object->y, ptr->object->z);
			}
		}
		sD.Unlock();

		Tag = s.Tag;

		return *this;
	};
	bool operator==(const cweeInterpolatedMatrix<T>& s) {
		ValidateData(); s.ValidateData();
		return this->hilbertContainer.operator==(s.hilbertContainer);
	};
	bool operator!=(const cweeInterpolatedMatrix<T>& obj) { return !operator==(obj); };
	void    Reserve(long long num) {
		hilbertContainer.SetGranularity(num);
		source.Reserve(num);
	};
	u64		GetMinX() const {
		u64 out;
		ValidateData();
		mut.Read_Lock();
		{
			out = minX / compressionFactor;
		}
		mut.Read_Unlock();
		return out;
	};
	u64		GetMaxX() const {
		u64 out;
		ValidateData();
		mut.Read_Lock();
		{
			out = maxX / compressionFactor;
		}
		mut.Read_Unlock();
		return out;
	};
	u64		GetMinY() const {
		u64 out;
		ValidateData();
		mut.Read_Lock();
		{
			out = minY / compressionFactor;
		}
		mut.Read_Unlock();
		return out;
	};
	u64		GetMaxY() const {
		u64 out;
		ValidateData();
		mut.Read_Lock();
		{
			out = maxY / compressionFactor;
		}
		mut.Read_Unlock();
		return out;
	};
	T		GetMinValue() const {
		T out;
		ValidateData();
		mut.Read_Lock();
		{
			out = minV;
		}
		mut.Read_Unlock();
		return out;
	};
	T		GetMaxValue() const {
		T out;
		ValidateData();
		mut.Read_Lock();
		{
			out = maxV;
		}
		mut.Read_Unlock();
		return out;
	};

	T		GetValue(const u64& column, const u64& row) const {
		T out = 0;
		ValidateData();
		mut.Read_Lock();
		{
			long long x = std::floor((double)(column * compressionFactor - minX) + 0.5);
			long long y = std::floor((double)(row * compressionFactor - minY) + 0.5);
			if (hilbertN > 0) {
				long long pos = xy2d(x, y, hilbertN);
				out = hilbertContainer.GetCurrentValue(pos)();
			}
			else {
				if (hilbertContainer.GetNumValues() > 0) {
					out = hilbertContainer.GetCurrentValue(0)();
				}
			}
		}
		mut.Read_Unlock();
		return out;
	};
	T		GetCurrentValue(const u64& column, const u64& row) const {
		return GetValue(column, row);
	};

	void	RemoveUnnecessaryKnots() {
		ValidateData();		
		hilbertContainer.RemoveUnnecessaryKnots();
	};
	void	ReduceMemory(float percentToRemove) {
		ValidateData();
		hilbertContainer.ReduceMemory(percentToRemove);
	};

	/*! Row1Column1, Row1Column2, ... Row1ColumnN, Row2Column1 ... etc. */
	std::vector<T> GetMatrix(const u64& Left, const u64& Top, const u64& Right, const u64& Bottom, int numColumns, int numRows) const {
		std::vector<T> out;
		if (numColumns <= 0 || numRows <= 0) {
			return out;
		}
		out.reserve(numRows * numColumns);

		T v = 0; long long pos, x, y; u64 col = Left, columnStep = (Right - Left) / numColumns, rowStep = (Top - Bottom) / numRows, row = Top; int R, C;
		ValidateData(); 
		mut.Read_Lock();
		for (R = 0; R < numRows; R++) {
			col = Left;
			for (C = 0; C < numColumns; C++) {
				{
					x = std::floor((double)(col * compressionFactor - minX) + 0.5);
					y = std::floor((double)(row * compressionFactor - minY) + 0.5);
					if (hilbertN > 0) {
						pos = xy2d(x, y, hilbertN);
						v = hilbertContainer.GetCurrentValue(pos)();
					}
					else if (hilbertContainer.GetNumValues() > 0) {
						v = hilbertContainer.GetCurrentValue(0)();
					}
				}
				out.push_back(v);

				col += columnStep;
			}
			row -= rowStep;
		}
		mut.Read_Unlock();
		return out;
	};
	std::vector<T> GetMatrix(int numColumns, int numRows) const {
		std::vector<T> out;
		if (numColumns <= 0 || numRows <= 0) {
			return out;
		}
		out.reserve(numRows * numColumns);

		const u64& Left = GetMinX(), Top = GetMaxY(), Right = GetMaxX(), Bottom = GetMinY();

		T v = 0; long long pos, x, y; u64 col = Left, columnStep = (Right - Left) / numColumns, rowStep = (Top - Bottom) / numRows, row = Top; int R, C;
		ValidateData(); 
		mut.Read_Lock();
		for (R = 0; R < numRows; R++) {
			col = Left;
			for (C = 0; C < numColumns; C++) {
				{
					x = std::floor((double)(col * compressionFactor - minX) + 0.5);
					y = std::floor((double)(row * compressionFactor - minY) + 0.5);
					if (hilbertN > 0) {
						pos = xy2d(x, y, hilbertN);
						v = hilbertContainer.GetCurrentValue(pos)();
					}
					else if (hilbertContainer.GetNumValues() > 0) {
						v = hilbertContainer.GetCurrentValue(0)();
					}
				}
				out.push_back(v);

				col += columnStep;
			}
			row -= rowStep;
		}
		mut.Read_Unlock();
		return out;
	};
	void	InsertValue(const u64& column, const u64& row, const T& value, bool AddUnique = true) {
		long long 
			x = std::floor((double)(column * compressionFactor) + 0.5),
			y = std::floor((double)(row * compressionFactor) + 0.5);
		xyContainer val(column, row, value);
		u64 hash = uniqueHash(x, y);
		if (AddUnique)
			source.AddUniqueValue(hash, val);
		else 
			source.AddValue(hash, val);
		invalidated.store(true);
	};
	cweeInterpolatedMatrix<T>& AddValue(const u64& column, const u64& row, const T& value, bool AddUnique = true) {
		this->InsertValue(column, row, value, AddUnique);
		return *this;
	};
	bool ContainsPosition(const u64& column, const u64& row) const {
		long long
			x = std::floor((double)(column * compressionFactor) + 0.5),
			y = std::floor((double)(row * compressionFactor) + 0.5);
		return source.ValueExists(uniqueHash(x, y));
	};

	void	Clear() {
		mut.Write_Lock();
		UnsafeClear();
		mut.Write_Unlock();
	};
	void	UnsafeClear() {
		source.Clear();
		hilbertContainer.Clear();
		invalidated.store(false);
		minX = std::numeric_limits<long long>::max();
		maxX = -std::numeric_limits<long long>::max();
		minY = std::numeric_limits<long long>::max();
		maxY = -std::numeric_limits<long long>::max();
		minV = std::numeric_limits<T>::max();
		maxV = -std::numeric_limits<T>::max();

		hilbertN = 0;
	};
	void	Lock() const {
		mut.Write_Lock();
	};
	void	Unlock() const {
		mut.Write_Unlock();
	};
	sourceType& UnsafeGetSource() const {
		return source;
	};

	cweeStr			ToString(void) const {
		return source.Serialize();
	};
	cweeInterpolatedMatrix& FromString(const cweeStr& v) {
		Lock();
		UnsafeClear();

		cweeStr v0 = v;
		source.Deserialize(v0);

		invalidated.store(true);

		Unlock();

		return *this;
	};
	operator		const char* () const {
		return ToString();
	};
	operator		const char* () {
		return ToString();
	};

	int		Num() {
		return source.GetNumValues();
	};

	u64 AverageDistanceBetweenKnots() const {
		u64 Distance = 0; int count = 0; long long x, y;
		ValidateData();
		AUTO knots = hilbertContainer.GetKnotSeries();
		cweeList<cweePair<double, double>> knotsXY(knots.Num() + 16);
		mut.Read_Lock();
		for (auto& knot : knots) {
			d2xy(knot.first, x, y, hilbertN);

			x += minX;
			y += minY;

			x /= compressionFactor;
			y /= compressionFactor;

			knotsXY.Append(cweePair<double, double>(x, y));
		}
		mut.Read_Unlock();
		for (auto& x : knotsXY) {
			for (auto& y : knotsXY) {
				cweeMath::rollingAverageRef<u64>(
					Distance
					,
					cweeMath::RSqrtFast(
						((x.first - y.first) * (x.first - y.first))
						+ ((x.second - y.second) * (x.second - y.second))
					)
					, count
				);
			}
		}

		if (Distance > 0)
			return 1.0 / Distance;
		else
			return Distance;
	};

	u64 EstimateDistanceBetweenKnots() const {
		u64 width, height;
		ValidateData();
		int num = source.GetNumValues();
		mut.Read_Lock(); {
			width = (maxX - minX) / compressionFactor;
			height = (maxY - minY) / compressionFactor;
		} mut.Read_Unlock();
		if (width <= 0 || height <= 0 || num <= 0) return 0;
		else return cweeMath::Sqrt(width*height / num);
	};

	u64 MinHilbertPosition() const  {
		ValidateData();
		return hilbertContainer.GetMinTime();
	};
	u64 MaxHilbertPosition() const  {
		ValidateData();
		return hilbertContainer.GetMaxTime();
	};
	T	HilbertPositionToValue(const u64& hilbertPos) const  {
		ValidateData();
		return hilbertContainer.GetCurrentValue(hilbertPos)();
	};
	std::pair<u64, u64>	HilbertPositionToXY(const u64& hilbertPos) const {
		std::pair<u64, u64> out; long long x, y;
		ValidateData();

		mut.Read_Lock();
		{
			d2xy(hilbertPos, x, y, hilbertN);
			out.first = (x + minX) / compressionFactor;
			out.second = (y + minY) / compressionFactor;
		}
		mut.Read_Unlock();

		return out;
	};

	mutable cweeSharedPtr<void> Tag;
	
protected: // data
	mutable sourceType source; // ordered by a hash, NOT ordered by the hilbert scaler
	// mutable cweePattern_CatmullRomSpline<T> hilbertContainer; // x-position is the length along the hilbert line 
	mutable cweeBalancedPattern<units::dimensionless::scalar_t> hilbertContainer; // x-position is the length along the hilbert line 

	mutable std::atomic_bool invalidated = false;
	mutable long long minX = std::numeric_limits<long long>::max();
	mutable long long maxX = -std::numeric_limits<long long>::max();
	mutable long long minY = std::numeric_limits<long long>::max();
	mutable long long maxY = -std::numeric_limits<long long>::max();
	mutable T minV = std::numeric_limits<T>::max();
	mutable T maxV = -std::numeric_limits<T>::max();

	mutable long long hilbertN = 0;
	constexpr static u64 compressionFactor = 100000.0f;
	mutable cweeReadWriteMutex mut;

protected: // protected member methods
	void ValidateData() const {
		if (invalidated.load()) {
			mut.Write_Lock();
			if (invalidated.load()) {
				Tag = nullptr;

				long long x, y;

				source.Lock();
				for (auto& ptr : source.UnsafeGetValues()) {
					if (ptr.object) {
						auto& value = ptr.object->z;

						x = std::floor((double)(ptr.object->x * compressionFactor) + 0.5);
						y = std::floor((double)(ptr.object->y * compressionFactor) + 0.5);

						if (value < minV) minV = value;
						if (value > maxV) maxV = value;

						// does this new value invalidate the current hilbert formula?
						if (x < minX || (x - minX) >= hilbertN || y < minY || (y - minY) >= hilbertN) {
							// the current formula doesn't cover the needed range  - the hilbert must be re-calculated from scratch for the current 'source'
							if (x < minX) minX = x;
							if (x > maxX) maxX = x;
							if (y < minY) minY = y;
							if (y > maxY) maxY = y;
						}
					}
				}
				source.Unlock();

				long long width = next_pow2(maxX - minX); // i.e. 1,2,4,16,128,256,1024
				long long height = next_pow2(maxY - minY); // i.e. 1,2,4,16,128,256,1024
				hilbertN = ::Max(width, height);

				hilbertContainer.Clear();
				hilbertContainer.SetBoundaryType(boundary_t::BT_CLOSED);
				hilbertContainer.SetInterpolationType(interpolation_t::LINEAR);
				hilbertContainer.SetGranularity(source.GetNumValues() + 12);

				source.Lock();
				for (auto* ptr : source.UnsafeGetKnotSeries()) {
					if (ptr && ptr->object) {
						hilbertContainer.AddUniqueValue(
							xy2d(
								std::floor(((double)(ptr->object->x * compressionFactor) - minX) + 0.5),
								std::floor(((double)(ptr->object->y * compressionFactor) - minY) + 0.5),
								hilbertN
							),
							ptr->object->z
						);
					}
				}
				source.Unlock();

				invalidated.store(false);
			}
			mut.Write_Unlock();
		}
	};

private: // private static methods
	static long long uniqueHash(long long a, long long b) {
		return ((std::hash<long long>{ } (a)+2) ^ (std::hash<long long>{ } (b) << 1));
	};
	static constexpr uint64_t next_pow2(uint64_t x) {
		x |= x >> 1;
		x |= x >> 2;
		x |= x >> 4;
		x |= x >> 8;
		x |= x >> 16;
		x |= x >> 32;
		return x + 1;
	};
	//****************************************************************************
	//  Purpose:
	//
	//    ROT rotates and flips a quadrant appropriately
	//
	//  Modified:
	//
	//    24 December 2015
	//
	//  Parameters:
	//
	//    Input, long long N, the length of a side of the square.  N must be a power of 2.
	//
	//    Input/output, long long &X, &Y, the input and output coordinates of a polong long.
	//
	//    Input, long long RX, RY, ???
	static void rot(long long n, long long& x, long long& y, long long rx, long long ry) {
		long long t;
		if (ry == 0)
		{
			//  Reflect.			
			if (rx == 1)
			{
				x = n - 1 - x;
				y = n - 1 - y;
			}

			//  Flip.			
			t = x;
			x = y;
			y = t;
		}
	};
	//****************************************************************************80
	//
	//  Purpose:
	//
	//    D2XY converts a 1D Hilbert coordinate to a 2D Cartesian coordinate.
	//
	//  Modified:
	//
	//    24 December 2015
	//
	//  Parameters:
	//
	//    Input, long long M, the index of the Hilbert curve.
	//    The number of cells is N=2^M.
	//    0 < M.
	//
	//    Input, long long D, the Hilbert coordinate of the cell.
	//    0 <= D < N * N.
	//
	//    Output, long long &X, &Y, the Cartesian coordinates of the cell.
	//    0 <= X, Y < N.
	//
	static void d2xy(long long d, long long& x, long long& y, long long _hilbertN) {
		long long rx;
		long long ry;
		long long s;
		long long t = d;

		x = 0;
		y = 0;
		for (s = 1; s < _hilbertN; s = s * 2)
		{
			rx = (1 & (t / 2));
			ry = (1 & (t ^ rx));
			rot(s, x, y, rx, ry);
			x = x + s * rx;
			y = y + s * ry;
			t = t / 4;
		}
		return;
	}
	//****************************************************************************
	//  Purpose:
	//
	//    XY2D converts a 2D Cartesian coordinate to a 1D Hilbert coordinate.
	//
	//  Discussion:
	//
	//    It is assumed that a square has been divided into an NxN array of cells,
	//    where N is a power of 2.
	//
	//    Cell (0,0) is in the lower left corner, and (N-1,N-1) in the upper 
	//    right corner.
	//
	//  Modified:
	//
	//    24 December 2015
	//
	//  Parameters:
	//
	//    Input, long long M, the index of the Hilbert curve.
	//    The number of cells is N=2^M.
	//    0 < M.
	//
	//    Input, long long X, Y, the Cartesian coordinates of a cell.
	//    0 <= X, Y < N.
	//
	//    Output, long long XY2D, the Hilbert coordinate of the cell.
	//    0 <= D < N * N.
	static long long xy2d(long long x, long long y, long long _hilbertN) {
		long long d = 0;
		long long rx;
		long long ry;
		long long s;

		for (s = _hilbertN / 2; s > 0; s = s / 2)
		{
			rx = (x & s) > 0;
			ry = (y & s) > 0;
			d += s * s * ((3 * rx) ^ ry);
			rot(s, x, y, rx, ry);
		}
		return d;
	}

public:
	static long long Encode_2D_to_1D(long long x, long long y, long long _hilbertN) {
		return xy2d(x, y, _hilbertN);
	};
	static std::pair < u64, u64> Decode_1D_to_2D(long long x, long long _hilbertN) {
		std::pair < long long, long long> out = std::pair < long long, long long>(0, 0);
		d2xy(x, out.first, out.second, _hilbertN);
		return out;
	};
};
