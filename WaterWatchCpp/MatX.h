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
#include "VecX.h"

/*
===============================================================================
cweeMatX - arbitrary sized dense real matrix
The matrix lives on 16 byte aligned and 16 byte padded memory.
NOTE: due to the temporary memory pool cweeMatX cannot be used by multiple threads.
===============================================================================
*/
#define MATRIX_INVERSE_EPSILON		1e-14
#define MATRIX_EPSILON				1e-6
#define MATX_MAX_TEMP		1024
#define MATX_QUAD( x )		( ( ( ( x ) + 3 ) & ~3 ) * sizeof( float ) )
#define MATX_CLEAREND()		int s = numRows * numColumns; while( s < ( ( s + 3 ) & ~3 ) ) { mat[s++] = 0.0f; }
#define MATX_ALLOCA( n )	( (float *) _alloca16( MATX_QUAD( n ) ) )
#define MATX_ALLOCA_CACHE_LINES( n )	( (float *) _alloca128( ( ( n ) * sizeof( float ) + CACHE_LINE_SIZE - 1 ) & ~ ( CACHE_LINE_SIZE - 1 ) ) )


#define CACHE_LINE_SIZE						128

INLINE void Prefetch(const void* ptr, int offset) {}
INLINE void ZeroCacheLine(void* ptr, int offset)
{
	byte* bytePtr = (byte*)((((uintptr_t)(ptr)) + (offset)) & ~(CACHE_LINE_SIZE - 1));
	memset(bytePtr, 0, CACHE_LINE_SIZE);
}
INLINE void FlushCacheLine(const void* ptr, int offset) {}

template< typename _type_ > INLINE void SwapValues(_type_& a, _type_& b) {
	_type_ c = a;
	a = b;
	b = c;
}

class cweeMatX
{
public:
	INLINE					cweeMatX();
	INLINE					cweeMatX(const cweeMatX& other);
	INLINE					explicit cweeMatX(int rows, int columns);
	INLINE					explicit cweeMatX(int rows, int columns, float* src);
	~cweeMatX() {
		int i = 0;
		if (mat != NULL && alloced != -1)
		{
			i++;
			free(mat);
		}
		i++;
	};

	void			Set(int rows, int columns, const float* src)
	{
		SetSize(rows, columns);
		memcpy(this->mat, src, rows * columns * sizeof(float));
	};

	INLINE	const float* operator[](int index) const;
	INLINE	float* operator[](int index);
	cweeMatX& operator=(const cweeMatX& a) {
		// numRows = numColumns = alloced = 0;
		// mat = NULL;
		// Set(a.GetNumRows(), a.GetNumColumns(), a.ToFloatPtr());

		int s = a.numRows * a.numColumns;

		if (s <= 0) return *this;

		SetSize(a.numRows, a.numColumns, false);

#ifdef MATX_SIMD
		for (int i = 0; i < s; i += 4)
		{
			_mm_store_ps(mat + i, _mm_load_ps(a.mat + i));
		}
#else
		for (int i = 0; i < numRows; i++) {
			for (int j = 0; j < numRows; j++) {
				this->operator[](i)[j] = a[i][j];
			}
		}

		// memcpy(mat, a.mat, s * sizeof(float));

#endif
		return *this;
	};
	INLINE	cweeMatX			operator*(const float a) const;
	INLINE	cweeVecX			operator*(const cweeVecX& vec) const;
	INLINE	cweeMatX			operator*(const cweeMatX& a) const;
	INLINE	cweeMatX			operator+(const cweeMatX& a) const;
	INLINE	cweeMatX			operator-(const cweeMatX& a) const;
	INLINE	cweeMatX& operator*=(const float a);
	INLINE	cweeMatX& operator*=(const cweeMatX& a);
	INLINE	cweeMatX& operator+=(const cweeMatX& a);
	INLINE	cweeMatX& operator-=(const cweeMatX& a);

	friend INLINE	cweeMatX	operator*(const float a, const cweeMatX& m);
	friend INLINE	cweeVecX	operator*(const cweeVecX& vec, const cweeMatX& m);
	friend INLINE	cweeVecX& operator*=(cweeVecX& vec, const cweeMatX& m);

	INLINE	bool			Compare(const cweeMatX& a) const;									// exact compare, no epsilon
	INLINE	bool			Compare(const cweeMatX& a, const float epsilon) const;				// compare with epsilon
	INLINE	bool			operator==(const cweeMatX& a) const;								// exact compare, no epsilon
	INLINE	bool			operator!=(const cweeMatX& a) const;								// exact compare, no epsilon

	void					SetSize(int rows, int columns, bool zero = true) {
		if (rows != numRows || columns != numColumns || mat == NULL)
		{
			if (rows * columns == 0) {
				return;
			}

			int alloc = (rows * columns + 3) & ~3;
			if (alloc > alloced && alloced != -1)
			{
				if (mat != NULL)
				{
					free(mat);
					mat = NULL;
				}

				mat = (float*)malloc((size_t)(alloc * sizeof(float)));


				// mat = (float*)Mem_Alloc((size_t)(alloc * sizeof(float)), TAG_MATH);
				alloced = alloc;
			}
			numRows = rows;
			numColumns = columns;
			if (zero) {
				int s = numRows * numColumns;
				while (s < ((s + 3) & ~3)) {
					mat[s++] = 0.0f;
				}
			}
		}
	};									// set the number of rows/columns
	void					ChangeSize(int rows, int columns, bool makeZero = false);		// change the size keeping data intact where possible
	INLINE	void			ChangeNumRows(int rows)
	{
		ChangeSize(rows, numColumns);	   // set the number of rows/columns
	}
	int						GetNumRows() const
	{
		return numRows;    // get the number of rows
	}
	int						GetNumColumns() const
	{
		return numColumns;    // get the number of columns
	}
	INLINE	void			SetData(int rows, int columns, float* data);						// set float array pointer
	INLINE	void			SetDataCacheLines(int rows, int columns, float* data, bool clear);// set float array pointer
	INLINE	void			Zero();																// clear matrix
	INLINE	void			Zero(int rows, int columns);										// set size and clear matrix
	INLINE	void			Identity();															// clear to identity matrix
	INLINE	void			Identity(int rows, int columns);									// set size and clear to identity matrix
	INLINE	void			Diag(const cweeVecX& v);											// create diagonal matrix from vector
	INLINE	void			Random(int seed, float l = 0.0f, float u = 1.0f);					// fill matrix with random values
	INLINE	void			Random(int rows, int columns, int seed, float l = 0.0f, float u = 1.0f);
	INLINE	void			Negate();															// (*this) = - (*this)
	INLINE	void			Clamp(float min, float max);										// clamp all values
	INLINE	cweeMatX& SwapRows(int r1, int r2);											// swap rows
	INLINE	cweeMatX& SwapColumns(int r1, int r2);										// swap columns
	INLINE	cweeMatX& SwapRowsColumns(int r1, int r2);									// swap rows and columns
	cweeMatX& RemoveRow(int r);															// remove a row
	cweeMatX& RemoveColumn(int r);														// remove a column
	cweeMatX& RemoveRowColumn(int r);													// remove a row and column
	INLINE	void			ClearUpperTriangle();												// clear the upper triangle
	INLINE	void			ClearLowerTriangle();												// clear the lower triangle
	void					CopyLowerToUpperTriangle();													// copy the lower triangle to the upper triangle
	INLINE	void			SquareSubMatrix(const cweeMatX& m, int size);						// get square sub-matrix from 0,0 to size,size
	INLINE	float			MaxDifference(const cweeMatX& m) const;								// return maximum element difference between this and m

	INLINE	bool			IsSquare() const { return (numRows == numColumns); }
	INLINE	bool			IsZero(const float epsilon = MATRIX_EPSILON) const;
	INLINE	bool			IsIdentity(const float epsilon = MATRIX_EPSILON) const;
	INLINE	bool			IsDiagonal(const float epsilon = MATRIX_EPSILON) const;
	INLINE	bool			IsTriDiagonal(const float epsilon = MATRIX_EPSILON) const;
	INLINE	bool			IsSymmetric(const float epsilon = MATRIX_EPSILON) const;
	bool					IsOrthogonal(const float epsilon = MATRIX_EPSILON) const;
	bool					IsOrthonormal(const float epsilon = MATRIX_EPSILON) const;
	bool					IsPMatrix(const float epsilon = MATRIX_EPSILON) const;
	bool					IsZMatrix(const float epsilon = MATRIX_EPSILON) const;
	bool					IsPositiveDefinite(const float epsilon = MATRIX_EPSILON) const;
	bool					IsSymmetricPositiveDefinite(const float epsilon = MATRIX_EPSILON) const;
	bool					IsPositiveSemiDefinite(const float epsilon = MATRIX_EPSILON) const;
	bool					IsSymmetricPositiveSemiDefinite(const float epsilon = MATRIX_EPSILON) const;

	INLINE	float			Trace() const;													// returns product of diagonal elements
	INLINE	float			Determinant() const;											// returns determinant of matrix
	INLINE	cweeMatX		Transpose() const;												// returns transpose
	INLINE	cweeMatX& TransposeSelf();												// transposes the matrix itself
	INLINE	void			Transpose(cweeMatX& dst) const;								// stores transpose in 'dst'
	INLINE	cweeMatX		Inverse() const;												// returns the inverse ( m * m.Inverse() = identity )
	INLINE	bool			InverseSelf();													// returns false if determinant is zero
	INLINE	cweeMatX		InverseFast() const;											// returns the inverse ( m * m.Inverse() = identity )
	INLINE	bool			InverseFastSelf();												// returns false if determinant is zero
	INLINE	void			Inverse(cweeMatX& dst) const;									// stores the inverse in 'dst' ( m * m.Inverse() = identity )

	bool					LowerTriangularInverse();									// in-place inversion, returns false if determinant is zero
	bool					UpperTriangularInverse();									// in-place inversion, returns false if determinant is zero

	INLINE	void			Subtract(const cweeMatX& a);									// (*this) -= a;

	INLINE	cweeVecX		Multiply(const cweeVecX& vec) const;							// (*this) * vec
	INLINE	cweeVecX		TransposeMultiply(const cweeVecX& vec) const;					// this->Transpose() * vec

	INLINE	cweeMatX		Multiply(const cweeMatX& a) const;								// (*this) * a
	INLINE	cweeMatX		TransposeMultiply(const cweeMatX& a) const;						// this->Transpose() * a

	INLINE	void			Multiply(cweeVecX& dst, const cweeVecX& vec) const;				// dst = (*this) * vec
	INLINE	void			MultiplyAdd(cweeVecX& dst, const cweeVecX& vec) const;			// dst += (*this) * vec
	INLINE	void			MultiplySub(cweeVecX& dst, const cweeVecX& vec) const;			// dst -= (*this) * vec
	INLINE	void			TransposeMultiply(cweeVecX& dst, const cweeVecX& vec) const;		// dst = this->Transpose() * vec
	INLINE	void			TransposeMultiplyAdd(cweeVecX& dst, const cweeVecX& vec) const;	// dst += this->Transpose() * vec
	INLINE	void			TransposeMultiplySub(cweeVecX& dst, const cweeVecX& vec) const;	// dst -= this->Transpose() * vec

	INLINE	void			Multiply(cweeMatX& dst, const cweeMatX& a) const;					// dst = (*this) * a
	INLINE	void			TransposeMultiply(cweeMatX& dst, const cweeMatX& a) const;		// dst = this->Transpose() * a

	INLINE	int				GetDimension() const;											// returns total number of values in matrix

	INLINE	const cweeVecX	SubVecX(int row) const;										// interpret complete row as a const cweeVecX
	INLINE	cweeVecX		SubVecX(int row);												// interpret complete row as an cweeVecX
	INLINE	const float* ToFloatPtr() const;												// pointer to const matrix float array
	INLINE	float* ToFloatPtr();													// pointer to matrix float array

	void					Update_RankOne(const cweeVecX& v, const cweeVecX& w, float alpha);
	void					Update_RankOneSymmetric(const cweeVecX& v, float alpha);
	void					Update_RowColumn(const cweeVecX& v, const cweeVecX& w, int r);
	void					Update_RowColumnSymmetric(const cweeVecX& v, int r);
	void					Update_Increment(const cweeVecX& v, const cweeVecX& w);
	void					Update_IncrementSymmetric(const cweeVecX& v);
	void					Update_Decrement(int r);

	bool					Inverse_GaussJordan();					// invert in-place with Gauss-Jordan elimination
	bool					Inverse_UpdateRankOne(const cweeVecX& v, const cweeVecX& w, float alpha);
	bool					Inverse_UpdateRowColumn(const cweeVecX& v, const cweeVecX& w, int r);
	bool					Inverse_UpdateIncrement(const cweeVecX& v, const cweeVecX& w);
	bool					Inverse_UpdateDecrement(const cweeVecX& v, const cweeVecX& w, int r);
	void					Inverse_Solve(cweeVecX& x, const cweeVecX& b) const;

	bool					LU_Factor(int* index, float* det = NULL);		// factor in-place: L * U
	bool					LU_UpdateRankOne(const cweeVecX& v, const cweeVecX& w, float alpha, int* index);
	bool					LU_UpdateRowColumn(const cweeVecX& v, const cweeVecX& w, int r, int* index);
	bool					LU_UpdateIncrement(const cweeVecX& v, const cweeVecX& w, int* index);
	bool					LU_UpdateDecrement(const cweeVecX& v, const cweeVecX& w, const cweeVecX& u, int r, int* index);
	void					LU_Solve(cweeVecX& x, const cweeVecX& b, const int* index) const;
	void					LU_Inverse(cweeMatX& inv, const int* index) const;
	void					LU_UnpackFactors(cweeMatX& L, cweeMatX& U) const;
	void					LU_MultiplyFactors(cweeMatX& m, const int* index) const;

	bool					QR_Factor(cweeVecX& c, cweeVecX& d);				// factor in-place: Q * R
	bool					QR_UpdateRankOne(cweeMatX& R, const cweeVecX& v, const cweeVecX& w, float alpha);
	bool					QR_UpdateRowColumn(cweeMatX& R, const cweeVecX& v, const cweeVecX& w, int r);
	bool					QR_UpdateIncrement(cweeMatX& R, const cweeVecX& v, const cweeVecX& w);
	bool					QR_UpdateDecrement(cweeMatX& R, const cweeVecX& v, const cweeVecX& w, int r);
	void					QR_Solve(cweeVecX& x, const cweeVecX& b, const cweeVecX& c, const cweeVecX& d) const;
	void					QR_Solve(cweeVecX& x, const cweeVecX& b, const cweeMatX& R) const;
	void					QR_Inverse(cweeMatX& inv, const cweeVecX& c, const cweeVecX& d) const;
	void					QR_UnpackFactors(cweeMatX& Q, cweeMatX& R, const cweeVecX& c, const cweeVecX& d) const;
	void					QR_MultiplyFactors(cweeMatX& m, const cweeVecX& c, const cweeVecX& d) const;

	bool					SVD_Factor(cweeVecX& w, cweeMatX& V);				// factor in-place: U * Diag(w) * V.Transpose()
	void					SVD_Solve(cweeVecX& x, const cweeVecX& b, const cweeVecX& w, const cweeMatX& V) const;
	void					SVD_Inverse(cweeMatX& inv, const cweeVecX& w, const cweeMatX& V) const;
	void					SVD_MultiplyFactors(cweeMatX& m, const cweeVecX& w, const cweeMatX& V) const;

	bool					Cholesky_Factor();						// factor in-place: L * L.Transpose()
	bool					Cholesky_UpdateRankOne(const cweeVecX& v, float alpha, int offset = 0);
	bool					Cholesky_UpdateRowColumn(const cweeVecX& v, int r);
	bool					Cholesky_UpdateIncrement(const cweeVecX& v);
	bool					Cholesky_UpdateDecrement(const cweeVecX& v, int r);
	void					Cholesky_Solve(cweeVecX& x, const cweeVecX& b) const;
	void					Cholesky_Inverse(cweeMatX& inv) const;
	void					Cholesky_MultiplyFactors(cweeMatX& m) const;

	bool					LDLT_Factor();							// factor in-place: L * D * L.Transpose()
	bool					LDLT_UpdateRankOne(const cweeVecX& v, float alpha, int offset = 0);
	bool					LDLT_UpdateRowColumn(const cweeVecX& v, int r);
	bool					LDLT_UpdateIncrement(const cweeVecX& v);
	bool					LDLT_UpdateDecrement(const cweeVecX& v, int r);
	void					LDLT_Solve(cweeVecX& x, const cweeVecX& b) const;
	void					LDLT_Inverse(cweeMatX& inv) const;
	void					LDLT_UnpackFactors(cweeMatX& L, cweeMatX& D) const;
	void					LDLT_MultiplyFactors(cweeMatX& m) const;

	void					TriDiagonal_ClearTriangles();
	bool					TriDiagonal_Solve(cweeVecX& x, const cweeVecX& b) const;
	void					TriDiagonal_Inverse(cweeMatX& inv) const;

	bool					Eigen_SolveSymmetricTriDiagonal(cweeVecX& eigenValues);
	bool					Eigen_SolveSymmetric(cweeVecX& eigenValues);
	bool					Eigen_Solve(cweeVecX& realEigenValues, cweeVecX& imaginaryEigenValues);
	void					Eigen_SortIncreasing(cweeVecX& eigenValues);
	void					Eigen_SortDecreasing(cweeVecX& eigenValues);

private:
	int				numRows;				// number of rows
	int				numColumns;				// number of columns
	int				alloced;				// floats allocated, if -1 then mat points to data set with SetData
	float* mat;					// memory the matrix is stored

private:
	float			DeterminantGeneric() const;
	bool			InverseSelfGeneric();
	void			QR_Rotate(cweeMatX& R, int i, float a, float b);
	float			Pythag(float a, float b) const;
	void			SVD_BiDiag(cweeVecX& w, cweeVecX& rv1, float& anorm);
	void			SVD_InitialWV(cweeVecX& w, cweeMatX& V, cweeVecX& rv1);
	void			HouseholderReduction(cweeVecX& diag, cweeVecX& subd);
	bool			QL(cweeVecX& diag, cweeVecX& subd);
	void			HessenbergReduction(cweeMatX& H);
	void			ComplexDivision(float xr, float xi, float yr, float yi, float& cdivr, float& cdivi);
	bool			HessenbergToRealSchur(cweeMatX& H, cweeVecX& realEigenValues, cweeVecX& imaginaryEigenValues);
};

/*
========================
cweeMatX::cweeMatX
========================
*/
INLINE cweeMatX::cweeMatX()
{
	numRows = numColumns = alloced = 0;
	mat = NULL;
}

/*
========================
cweeMatX::~cweeMatX
========================
*/
//INLINE cweeMatX::~cweeMatX()
//{
//	if (mat != NULL && alloced != -1)
//	{
//		Mem_Free(mat);
//	}
//}

/*
========================
cweeMatX::cweeMatX
========================
*/
INLINE cweeMatX::cweeMatX(int rows, int columns)
{
	numRows = numColumns = alloced = 0;
	mat = NULL;
	SetSize(rows, columns);
}

/*
========================
cweeMatX::cweeMatX
========================
*/
INLINE cweeMatX::cweeMatX(const cweeMatX& other)
{
	numRows = numColumns = alloced = 0;
	mat = NULL;
	Set(other.GetNumRows(), other.GetNumColumns(), other.ToFloatPtr());
}

/*
========================
cweeMatX::cweeMatX
========================
*/
INLINE cweeMatX::cweeMatX(int rows, int columns, float* src)
{
	numRows = numColumns = alloced = 0;
	mat = NULL;
	SetData(rows, columns, src);
}

/*
========================
cweeMatX::Set
========================
*/
//INLINE void cweeMatX::Set(int rows, int columns, const float* src)
//{
//	SetSize(rows, columns);
//	memcpy(this->mat, src, rows * columns * sizeof(float));
//}

/*
========================
cweeMatX::operator[]
========================
*/
INLINE const float* cweeMatX::operator[](int index) const
{
	assert((index >= 0) && (index < numRows));
	return mat + index * numColumns;
}

/*
========================
cweeMatX::operator[]
========================
*/
INLINE float* cweeMatX::operator[](int index)
{
	assert((index >= 0) && (index < numRows));
	return mat + index * numColumns;
}

/*
========================
cweeMatX::operator=
========================
*/
//INLINE cweeMatX& cweeMatX::operator=(const cweeMatX& a)
//{
//	SetSize(a.numRows, a.numColumns);
//	int s = a.numRows * a.numColumns;
//#ifdef MATX_SIMD
//	for (int i = 0; i < s; i += 4)
//	{
//		_mm_store_ps(mat + i, _mm_load_ps(a.mat + i));
//	}
//#else
//	memcpy(mat, a.mat, s * sizeof(float));
//#endif
//	return *this;
//}

/*
========================
cweeMatX::operator*
========================
*/
INLINE cweeMatX cweeMatX::operator*(const float a) const
{
	cweeMatX m(numRows, numColumns);
	int s = numRows * numColumns;
#ifdef MATX_SIMD
	__m128 va = _mm_load1_ps(&a);
	for (int i = 0; i < s; i += 4)
	{
		_mm_store_ps(m.mat + i, _mm_mul_ps(_mm_load_ps(mat + i), va));
	}
#else
	for (int i = 0; i < s; i++)
	{
		m.mat[i] = mat[i] * a;
	}
#endif
	return m;
}

/*
========================
cweeMatX::operator*
========================
*/
INLINE cweeVecX cweeMatX::operator*(const cweeVecX& vec) const
{
	assert(numColumns == vec.GetSize());

	cweeVecX dst;
	dst.SetSize(numRows);
	Multiply(dst, vec);
	return dst;
}

/*
========================
cweeMatX::operator*
========================
*/
INLINE cweeMatX cweeMatX::operator*(const cweeMatX& a) const
{
	assert(numColumns == a.numRows);

	cweeMatX dst(numRows, a.numColumns);
	Multiply(dst, a);
	return dst;
}

/*
========================
cweeMatX::operator+
========================
*/
INLINE cweeMatX cweeMatX::operator+(const cweeMatX& a) const
{
	cweeMatX m(numRows, numColumns);
	int s = numRows * numColumns;
#ifdef MATX_SIMD
	for (int i = 0; i < s; i += 4)
	{
		_mm_store_ps(m.mat + i, _mm_add_ps(_mm_load_ps(mat + i), _mm_load_ps(a.mat + i)));
	}
#else
	for (int i = 0; i < s; i++)
	{
		m.mat[i] = mat[i] + a.mat[i];
	}
#endif
	return m;
}

/*
========================
cweeMatX::operator-
========================
*/
INLINE cweeMatX cweeMatX::operator-(const cweeMatX& a) const
{
	cweeMatX m(numRows, numColumns);
	int s = numRows * numColumns;
#ifdef MATX_SIMD
	for (int i = 0; i < s; i += 4)
	{
		_mm_store_ps(m.mat + i, _mm_sub_ps(_mm_load_ps(mat + i), _mm_load_ps(a.mat + i)));
	}
#else
	for (int i = 0; i < s; i++)
	{
		m.mat[i] = mat[i] - a.mat[i];
	}
#endif
	return m;
}

/*
========================
cweeMatX::operator*=
========================
*/
INLINE cweeMatX& cweeMatX::operator*=(const float a)
{
	int s = numRows * numColumns;
	for (int i = 0; i < s; i++)
	{
		mat[i] *= a;
	}
	return *this;
}

/*
========================
cweeMatX::operator*=
========================
*/
INLINE cweeMatX& cweeMatX::operator*=(const cweeMatX& a)
{
	*this = *this * a;
	return *this;
}

/*
========================
cweeMatX::operator+=
========================
*/
INLINE cweeMatX& cweeMatX::operator+=(const cweeMatX& a)
{
	assert(numRows == a.numRows && numColumns == a.numColumns);
	int s = numRows * numColumns;
	for (int i = 0; i < s; i++)
	{
		mat[i] += a.mat[i];
	}
	return *this;
}

/*
========================
cweeMatX::operator-=
========================
*/
INLINE cweeMatX& cweeMatX::operator-=(const cweeMatX& a)
{
	assert(numRows == a.numRows && numColumns == a.numColumns);
	int s = numRows * numColumns;
	for (int i = 0; i < s; i++)
	{
		mat[i] -= a.mat[i];
	}
	return *this;
}

/*
========================
operator*
========================
*/
INLINE cweeMatX operator*(const float a, cweeMatX const& m)
{
	return m * a;
}

/*
========================
operator*
========================
*/
INLINE cweeVecX operator*(const cweeVecX& vec, const cweeMatX& m)
{
	return m * vec;
}

/*
========================
operator*=
========================
*/
INLINE cweeVecX& operator*=(cweeVecX& vec, const cweeMatX& m)
{
	vec = m * vec;
	return vec;
}

/*
========================
cweeMatX::Compare
========================
*/
INLINE bool cweeMatX::Compare(const cweeMatX& a) const
{
	assert(numRows == a.numRows && numColumns == a.numColumns);

	int s = numRows * numColumns;
	for (int i = 0; i < s; i++)
	{
		if (mat[i] != a.mat[i])
		{
			return false;
		}
	}
	return true;
}

/*
========================
cweeMatX::Compare
========================
*/
INLINE bool cweeMatX::Compare(const cweeMatX& a, const float epsilon) const
{
	assert(numRows == a.numRows && numColumns == a.numColumns);

	int s = numRows * numColumns;
	for (int i = 0; i < s; i++)
	{
		if (cweeMath::Fabs(mat[i] - a.mat[i]) > epsilon)
		{
			return false;
		}
	}
	return true;
}

/*
========================
cweeMatX::operator==
========================
*/
INLINE bool cweeMatX::operator==(const cweeMatX& a) const
{
	return Compare(a);
}

/*
========================
cweeMatX::operator!=
========================
*/
INLINE bool cweeMatX::operator!=(const cweeMatX& a) const
{
	return !Compare(a);
}

/*
========================
cweeMatX::SetSize
========================
*/
//INLINE void cweeMatX::SetSize(int rows, int columns)
//{
//	if (rows != numRows || columns != numColumns || mat == NULL)
//	{
//		assert(mat < cweeMatX::tempPtr || mat > cweeMatX::tempPtr + MATX_MAX_TEMP);
//		int alloc = (rows * columns + 3) & ~3;
//		if (alloc > alloced && alloced != -1)
//		{
//			if (mat != NULL)
//			{
//				Mem_Free(mat);
//			}
//			mat = (float*)Mem_Alloc((size_t)(alloc * sizeof(float)), TAG_MATH);
//			alloced = alloc;
//		}
//		numRows = rows;
//		numColumns = columns;
//		MATX_CLEAREND();
//	}
//}

/*
========================
cweeMatX::SetData
========================
*/
INLINE void cweeMatX::SetData(int rows, int columns, float* data)
{
	if (mat != NULL && alloced != -1)
	{
		free(mat);
	}
	// RB: changed UINT_PTR to uintptr_t
	// RB end
	mat = data;
	alloced = -1;
	numRows = rows;
	numColumns = columns;
	MATX_CLEAREND();
}

/*
========================
cweeMatX::SetDataCacheLines
========================
*/
INLINE void cweeMatX::SetDataCacheLines(int rows, int columns, float* data, bool clear)
{
	if (mat != NULL && alloced != -1)
	{
		free(mat);
	}

	// RB: changed UINT_PTR to uintptr_t
	assert((((uintptr_t)data) & 127) == 0); // data must be 128 byte aligned
	// RB end

	mat = data;
	alloced = -1;
	numRows = rows;
	numColumns = columns;

	if (clear)
	{
		int size = numRows * numColumns * sizeof(float);
		for (int i = 0; i < size; i += CACHE_LINE_SIZE)
		{
			ZeroCacheLine(mat, i);
		}
	}
	else
	{
		MATX_CLEAREND();
	}
}

/*
========================
cweeMatX::Zero
========================
*/
INLINE void cweeMatX::Zero()
{
	int s = numRows * numColumns;
#ifdef MATX_SIMD
	for (int i = 0; i < s; i += 4)
	{
		_mm_store_ps(mat + i, _mm_setzero_ps());
	}
#else
	memset(mat, 0, numRows * numColumns * sizeof(float));
#endif
}

/*
========================
cweeMatX::Zero
========================
*/
INLINE void cweeMatX::Zero(int rows, int columns)
{
	SetSize(rows, columns);
	Zero();
}

/*
========================
cweeMatX::Identity
========================
*/
INLINE void cweeMatX::Identity()
{
	assert(numRows == numColumns);
	Zero();
	for (int i = 0; i < numRows; i++)
	{
		mat[i * numColumns + i] = 1.0f;
	}
}

/*
========================
cweeMatX::Identity
========================
*/
INLINE void cweeMatX::Identity(int rows, int columns)
{
	assert(rows == columns);
	SetSize(rows, columns);
	cweeMatX::Identity();
}

/*
========================
cweeMatX::Diag
========================
*/
INLINE void cweeMatX::Diag(const cweeVecX& v)
{
	Zero(v.GetSize(), v.GetSize());
	for (int i = 0; i < v.GetSize(); i++)
	{
		mat[i * numColumns + i] = v[i];
	}
}

/*
========================
cweeMatX::Random
========================
*/
INLINE void cweeMatX::Random(int seed, float l, float u)
{
	float c = u - l;
	int s = numRows * numColumns;
	for (int i = 0; i < s; i++)
	{
		mat[i] = l + cweeRandomFloat(0, 1) * c;
	}
}

/*
========================
cweeMatX::Random
========================
*/
INLINE void cweeMatX::Random(int rows, int columns, int seed, float l, float u)
{

	SetSize(rows, columns);
	float c = u - l;
	int s = numRows * numColumns;
	for (int i = 0; i < s; i++)
	{
		mat[i] = l + cweeRandomFloat(0, 1) * c;
	}
}

/*
========================
cweeMatX::Negate
========================
*/
INLINE void cweeMatX::Negate()
{
	int s = numRows * numColumns;
	for (int i = 0; i < s; i++)
	{
		mat[i] = -mat[i];
	}
}

/*
========================
cweeMatX::Clamp
========================
*/
INLINE void cweeMatX::Clamp(float min, float max)
{
	int s = numRows * numColumns;
	for (int i = 0; i < s; i++)
	{
		if (mat[i] < min)
		{
			mat[i] = min;
		}
		else if (mat[i] > max)
		{
			mat[i] = max;
		}
	}
}

/*
========================
cweeMatX::SwapRows
========================
*/
INLINE cweeMatX& cweeMatX::SwapRows(int r1, int r2)
{
	float* ptr1 = mat + r1 * numColumns;
	float* ptr2 = mat + r2 * numColumns;
	for (int i = 0; i < numColumns; i++)
	{
		SwapValues(ptr1[i], ptr2[i]);
	}
	return *this;
}

/*
========================
cweeMatX::SwapColumns
========================
*/
INLINE cweeMatX& cweeMatX::SwapColumns(int r1, int r2)
{
	float* ptr = mat;
	for (int i = 0; i < numRows; i++, ptr += numColumns)
	{
		SwapValues(ptr[r1], ptr[r2]);
	}
	return *this;
}

/*
========================
cweeMatX::SwapRowsColumns
========================
*/
INLINE cweeMatX& cweeMatX::SwapRowsColumns(int r1, int r2)
{
	SwapRows(r1, r2);
	SwapColumns(r1, r2);
	return *this;
}

/*
========================
cweeMatX::ClearUpperTriangle
========================
*/
INLINE void cweeMatX::ClearUpperTriangle()
{
	assert(numRows == numColumns);
	for (int i = numRows - 2; i >= 0; i--)
	{
		memset(mat + i * numColumns + i + 1, 0, (numColumns - 1 - i) * sizeof(float));
	}
}

/*
========================
cweeMatX::ClearLowerTriangle
========================
*/
INLINE void cweeMatX::ClearLowerTriangle()
{
	assert(numRows == numColumns);
	for (int i = 1; i < numRows; i++)
	{
		memset(mat + i * numColumns, 0, i * sizeof(float));
	}
}

/*
========================
cweeMatX::SquareSubMatrix
========================
*/
INLINE void cweeMatX::SquareSubMatrix(const cweeMatX& m, int size)
{
	assert(size <= m.numRows && size <= m.numColumns);
	SetSize(size, size);
	for (int i = 0; i < size; i++)
	{
		memcpy(mat + i * numColumns, m.mat + i * m.numColumns, size * sizeof(float));
	}
}

/*
========================
cweeMatX::MaxDifference
========================
*/
INLINE float cweeMatX::MaxDifference(const cweeMatX& m) const
{
	assert(numRows == m.numRows && numColumns == m.numColumns);

	float maxDiff = -1.0f;
	for (int i = 0; i < numRows; i++)
	{
		for (int j = 0; j < numColumns; j++)
		{
			float diff = cweeMath::Fabs(mat[i * numColumns + j] - m[i][j]);
			if (maxDiff < 0.0f || diff > maxDiff)
			{
				maxDiff = diff;
			}
		}
	}
	return maxDiff;
}

/*
========================
cweeMatX::IsZero
========================
*/
INLINE bool cweeMatX::IsZero(const float epsilon) const
{
	// returns true if (*this) == Zero
	for (int i = 0; i < numRows; i++)
	{
		for (int j = 0; j < numColumns; j++)
		{
			if (cweeMath::Fabs(mat[i * numColumns + j]) > epsilon)
			{
				return false;
			}
		}
	}
	return true;
}

/*
========================
cweeMatX::IsIdentity
========================
*/
INLINE bool cweeMatX::IsIdentity(const float epsilon) const
{
	// returns true if (*this) == Identity
	assert(numRows == numColumns);
	for (int i = 0; i < numRows; i++)
	{
		for (int j = 0; j < numColumns; j++)
		{
			if (cweeMath::Fabs(mat[i * numColumns + j] - (float)(i == j)) > epsilon)
			{
				return false;
			}
		}
	}
	return true;
}

/*
========================
cweeMatX::IsDiagonal
========================
*/
INLINE bool cweeMatX::IsDiagonal(const float epsilon) const
{
	// returns true if all elements are zero except for the elements on the diagonal
	assert(numRows == numColumns);
	for (int i = 0; i < numRows; i++)
	{
		for (int j = 0; j < numColumns; j++)
		{
			if (i != j && cweeMath::Fabs(mat[i * numColumns + j]) > epsilon)
			{
				return false;
			}
		}
	}
	return true;
}

/*
========================
cweeMatX::IsTriDiagonal
========================
*/
INLINE bool cweeMatX::IsTriDiagonal(const float epsilon) const
{
	// returns true if all elements are zero except for the elements on the diagonal plus or minus one column

	if (numRows != numColumns)
	{
		return false;
	}
	for (int i = 0; i < numRows - 2; i++)
	{
		for (int j = i + 2; j < numColumns; j++)
		{
			if (cweeMath::Fabs((*this)[i][j]) > epsilon)
			{
				return false;
			}
			if (cweeMath::Fabs((*this)[j][i]) > epsilon)
			{
				return false;
			}
		}
	}
	return true;
}

/*
========================
cweeMatX::IsSymmetric
========================
*/
INLINE bool cweeMatX::IsSymmetric(const float epsilon) const
{
	// (*this)[i][j] == (*this)[j][i]
	if (numRows != numColumns)
	{
		return false;
	}
	for (int i = 0; i < numRows; i++)
	{
		for (int j = 0; j < numColumns; j++)
		{
			if (cweeMath::Fabs(mat[i * numColumns + j] - mat[j * numColumns + i]) > epsilon)
			{
				return false;
			}
		}
	}
	return true;
}

/*
========================
cweeMatX::Trace
========================
*/
INLINE float cweeMatX::Trace() const
{
	float trace = 0.0f;

	assert(numRows == numColumns);

	// sum of elements on the diagonal
	for (int i = 0; i < numRows; i++)
	{
		trace += mat[i * numRows + i];
	}
	return trace;
}

/*
========================
cweeMatX::Determinant
========================
*/
INLINE float cweeMatX::Determinant() const
{
	assert(numRows == numColumns);
	return DeterminantGeneric();
}

/*
========================
cweeMatX::Transpose
========================
*/
INLINE cweeMatX cweeMatX::Transpose() const
{
	cweeMatX transpose(numColumns, numRows);

	for (int i = 0; i < numRows; i++)
	{
		for (int j = 0; j < numColumns; j++)
		{
			transpose.mat[j * transpose.numColumns + i] = mat[i * numColumns + j];
		}
	}

	return transpose;
}

/*
========================
cweeMatX::TransposeSelf
========================
*/
INLINE cweeMatX& cweeMatX::TransposeSelf()
{
	*this = Transpose();
	return *this;
}

/*
========================
cweeMatX::Transpose
========================
*/
INLINE void cweeMatX::Transpose(cweeMatX& dst) const
{
	dst = Transpose();
}

/*
========================
cweeMatX::Inverse
========================
*/
INLINE cweeMatX cweeMatX::Inverse() const
{
	cweeMatX invMat(numRows, numColumns);
	memcpy(invMat.mat, mat, numRows * numColumns * sizeof(float));
	verify(invMat.InverseSelf());
	return invMat;
}

/*
========================
cweeMatX::InverseSelf
========================
*/
INLINE bool cweeMatX::InverseSelf()
{
	assert(numRows == numColumns);
	return InverseSelfGeneric();
}

/*
========================
cweeMatX::InverseFast
========================
*/
INLINE cweeMatX cweeMatX::InverseFast() const
{
	cweeMatX invMat(numRows, numColumns);
	memcpy(invMat.mat, mat, numRows * numColumns * sizeof(float));
	verify(invMat.InverseFastSelf());
	return invMat;
}

/*
========================
cweeMatX::InverseFastSelf
========================
*/
INLINE bool cweeMatX::InverseFastSelf()
{
	assert(numRows == numColumns);
	return InverseSelfGeneric();
}

/*
========================
cweeMatX::Inverse
========================
*/
INLINE void cweeMatX::Inverse(cweeMatX& dst) const
{
	dst = InverseFast();
}

/*
========================
cweeMatX::Subtract
========================
*/
INLINE void cweeMatX::Subtract(const cweeMatX& a)
{
	(*this) -= a;
}

/*
========================
cweeMatX::Multiply
========================
*/
INLINE cweeVecX cweeMatX::Multiply(const cweeVecX& vec) const
{
	assert(numColumns == vec.GetSize());

	cweeVecX dst;
	dst.SetSize(numRows);
	Multiply(dst, vec);
	return dst;
}

/*
========================
cweeMatX::Multiply
========================
*/
INLINE cweeMatX cweeMatX::Multiply(const cweeMatX& a) const
{
	assert(numColumns == a.numRows);

	cweeMatX dst(numRows, a.numColumns);
	Multiply(dst, a);
	return dst;
}

/*
========================
cweeMatX::TransposeMultiply
========================
*/
INLINE cweeVecX cweeMatX::TransposeMultiply(const cweeVecX& vec) const
{
	assert(numRows == vec.GetSize());

	cweeVecX dst;
	dst.SetSize(numColumns);
	TransposeMultiply(dst, vec);
	return dst;
}

/*
========================
cweeMatX::TransposeMultiply
========================
*/
INLINE cweeMatX cweeMatX::TransposeMultiply(const cweeMatX& a) const
{
	assert(numRows == a.numRows);

	cweeMatX dst(numColumns, a.numColumns);
	TransposeMultiply(dst, a);
	return dst;
}

/*
========================
cweeMatX::Multiply
========================
*/
INLINE void cweeMatX::Multiply(cweeVecX& dst, const cweeVecX& vec) const
{
	dst.SetSize(numRows);
	const float* mPtr = mat;
	const float* vPtr = vec.ToFloatPtr();
	float* dstPtr = dst.ToFloatPtr();
	float* temp = (float*)_alloca16(numRows * sizeof(float));
	for (int i = 0; i < numRows; i++)
	{
		float sum = mPtr[0] * vPtr[0];
		for (int j = 1; j < numColumns; j++)
		{
			sum += mPtr[j] * vPtr[j];
		}
		temp[i] = sum;
		mPtr += numColumns;
	}
	for (int i = 0; i < numRows; i++)
	{
		dstPtr[i] = temp[i];
	}
}

/*
========================
cweeMatX::MultiplyAdd
========================
*/
INLINE void cweeMatX::MultiplyAdd(cweeVecX& dst, const cweeVecX& vec) const
{
	assert(dst.GetSize() == numRows);
	const float* mPtr = mat;
	const float* vPtr = vec.ToFloatPtr();
	float* dstPtr = dst.ToFloatPtr();
	float* temp = (float*)_alloca16(numRows * sizeof(float));
	for (int i = 0; i < numRows; i++)
	{
		float sum = mPtr[0] * vPtr[0];
		for (int j = 1; j < numColumns; j++)
		{
			sum += mPtr[j] * vPtr[j];
		}
		temp[i] = dstPtr[i] + sum;
		mPtr += numColumns;
	}
	for (int i = 0; i < numRows; i++)
	{
		dstPtr[i] = temp[i];
	}
}

/*
========================
cweeMatX::MultiplySub
========================
*/
INLINE void cweeMatX::MultiplySub(cweeVecX& dst, const cweeVecX& vec) const
{
	assert(dst.GetSize() == numRows);
	const float* mPtr = mat;
	const float* vPtr = vec.ToFloatPtr();
	float* dstPtr = dst.ToFloatPtr();
	float* temp = (float*)_alloca16(numRows * sizeof(float));
	for (int i = 0; i < numRows; i++)
	{
		float sum = mPtr[0] * vPtr[0];
		for (int j = 1; j < numColumns; j++)
		{
			sum += mPtr[j] * vPtr[j];
		}
		temp[i] = dstPtr[i] - sum;
		mPtr += numColumns;
	}
	for (int i = 0; i < numRows; i++)
	{
		dstPtr[i] = temp[i];
	}
}

/*
========================
cweeMatX::TransposeMultiply
========================
*/
INLINE void cweeMatX::TransposeMultiply(cweeVecX& dst, const cweeVecX& vec) const
{
	dst.SetSize(numColumns);
	const float* vPtr = vec.ToFloatPtr();
	float* dstPtr = dst.ToFloatPtr();
	float* temp = (float*)_alloca16(numColumns * sizeof(float));
	for (int i = 0; i < numColumns; i++)
	{
		const float* mPtr = mat + i;
		float sum = mPtr[0] * vPtr[0];
		for (int j = 1; j < numRows; j++)
		{
			mPtr += numColumns;
			sum += mPtr[0] * vPtr[j];
		}
		temp[i] = sum;
	}
	for (int i = 0; i < numColumns; i++)
	{
		dstPtr[i] = temp[i];
	}
}

/*
========================
cweeMatX::TransposeMultiplyAdd
========================
*/
INLINE void cweeMatX::TransposeMultiplyAdd(cweeVecX& dst, const cweeVecX& vec) const
{
	assert(dst.GetSize() == numColumns);
	const float* vPtr = vec.ToFloatPtr();
	float* dstPtr = dst.ToFloatPtr();
	float* temp = (float*)_alloca16(numColumns * sizeof(float));
	for (int i = 0; i < numColumns; i++)
	{
		const float* mPtr = mat + i;
		float sum = mPtr[0] * vPtr[0];
		for (int j = 1; j < numRows; j++)
		{
			mPtr += numColumns;
			sum += mPtr[0] * vPtr[j];
		}
		temp[i] = dstPtr[i] + sum;
	}
	for (int i = 0; i < numColumns; i++)
	{
		dstPtr[i] = temp[i];
	}
}

/*
========================
cweeMatX::TransposeMultiplySub
========================
*/
INLINE void cweeMatX::TransposeMultiplySub(cweeVecX& dst, const cweeVecX& vec) const
{
	assert(dst.GetSize() == numColumns);
	const float* vPtr = vec.ToFloatPtr();
	float* dstPtr = dst.ToFloatPtr();
	float* temp = (float*)_alloca16(numColumns * sizeof(float));
	for (int i = 0; i < numColumns; i++)
	{
		const float* mPtr = mat + i;
		float sum = mPtr[0] * vPtr[0];
		for (int j = 1; j < numRows; j++)
		{
			mPtr += numColumns;
			sum += mPtr[0] * vPtr[j];
		}
		temp[i] = dstPtr[i] - sum;
	}
	for (int i = 0; i < numColumns; i++)
	{
		dstPtr[i] = temp[i];
	}
}

/*
========================
cweeMatX::Multiply
========================
*/
INLINE void cweeMatX::Multiply(cweeMatX& dst, const cweeMatX& a) const
{
	assert(numColumns == a.numRows);
	assert(&dst != &a && &dst != this);

	dst.SetSize(numRows, a.numColumns);
	float* dstPtr = dst.ToFloatPtr();
	const float* m1Ptr = ToFloatPtr();
	int k = numRows;
	int l = a.GetNumColumns();
	for (int i = 0; i < k; i++)
	{
		for (int j = 0; j < l; j++)
		{
			const float* m2Ptr = a.ToFloatPtr() + j;
			float sum = m1Ptr[0] * m2Ptr[0];
			for (int n = 1; n < numColumns; n++)
			{
				m2Ptr += l;
				sum += m1Ptr[n] * m2Ptr[0];
			}
			*dstPtr++ = sum;
		}
		m1Ptr += numColumns;
	}
}

/*
========================
cweeMatX::TransposeMultiply
========================
*/
INLINE void cweeMatX::TransposeMultiply(cweeMatX& dst, const cweeMatX& a) const
{
	assert(numRows == a.numRows);
	assert(&dst != &a && &dst != this);

	dst.SetSize(numColumns, a.numColumns);
	float* dstPtr = dst.ToFloatPtr();
	int k = numColumns;
	int l = a.numColumns;
	for (int i = 0; i < k; i++)
	{
		for (int j = 0; j < l; j++)
		{
			const float* m1Ptr = ToFloatPtr() + i;
			const float* m2Ptr = a.ToFloatPtr() + j;
			float sum = m1Ptr[0] * m2Ptr[0];
			for (int n = 1; n < numRows; n++)
			{
				m1Ptr += numColumns;
				m2Ptr += a.numColumns;
				sum += m1Ptr[0] * m2Ptr[0];
			}
			*dstPtr++ = sum;
		}
	}
}

/*
========================
cweeMatX::GetDimension
========================
*/
INLINE int cweeMatX::GetDimension() const
{
	return numRows * numColumns;
}

/*
========================
cweeMatX::SubVecX
========================
*/
INLINE const cweeVecX cweeMatX::SubVecX(int row) const
{
	cweeVecX v;
	assert(row >= 0 && row < numRows);
	v.SetData(numColumns, mat + row * numColumns);
	return v;
}

/*
========================
cweeMatX::SubVecX
========================
*/
INLINE cweeVecX cweeMatX::SubVecX(int row)
{
	cweeVecX v;
	assert(row >= 0 && row < numRows);
	v.SetData(numColumns, mat + row * numColumns);
	return v;
}

/*
========================
cweeMatX::ToFloatPtr
========================
*/
INLINE const float* cweeMatX::ToFloatPtr() const
{
	return mat;
}

/*
========================
cweeMatX::ToFloatPtr
========================
*/
INLINE float* cweeMatX::ToFloatPtr()
{
	return mat;
}