
#ifndef __MATH_VECX_H__
#define __MATH_VECX_H__

/*
===============================================================================
cweeVecX - arbitrary sized vector
The vector lives on 16 byte aligned and 16 byte padded memory.
NOTE: due to the temporary memory pool cweeVecX cannot be used by multiple threads
===============================================================================
*/

#define VECX_MAX_TEMP		1024
#define VECX_QUAD( x )		( ( ( ( x ) + 3 ) & ~3 ) * sizeof( float ) )
#define VECX_CLEAREND()		int s = size; while( s < ( ( s + 3) & ~3 ) ) { p[s++] = 0.0f; }
#define VECX_ALLOCA( n )	( (float *) _alloca16( VECX_QUAD( n ) ) )
#define VECX_SIMD

class cweeVecX {
	friend class cweeMatX;

public:
	INLINE					cweeVecX();
	INLINE					explicit cweeVecX(int length);
	INLINE					explicit cweeVecX(int length, float* data);
	INLINE					~cweeVecX();

	INLINE	float			Get(int index) const;
	INLINE	float&			Get(int index);

	INLINE	float			operator[](const int index) const;
	INLINE	float&			operator[](const int index);
	INLINE	cweeVecX			operator-() const;
	INLINE	cweeVecX&			operator=(const cweeVecX& a);
	INLINE	cweeVecX			operator*(const float a) const;
	INLINE	cweeVecX			operator/(const float a) const;
	INLINE	float			operator*(const cweeVecX& a) const;
	INLINE	cweeVecX			operator-(const cweeVecX& a) const;
	INLINE	cweeVecX			operator+(const cweeVecX& a) const;
	INLINE	cweeVecX&			operator*=(const float a);
	INLINE	cweeVecX&			operator/=(const float a);
	INLINE	cweeVecX&			operator+=(const cweeVecX& a);
	INLINE	cweeVecX&			operator-=(const cweeVecX& a);

	friend INLINE	cweeVecX	operator*(const float a, const cweeVecX& b);

	INLINE	bool			Compare(const cweeVecX& a) const;							// exact compare, no epsilon
	INLINE	bool			Compare(const cweeVecX& a, const float epsilon) const;		// compare with epsilon
	INLINE	bool			operator==(const cweeVecX& a) const;						// exact compare, no epsilon
	INLINE	bool			operator!=(const cweeVecX& a) const;						// exact compare, no epsilon

	INLINE	void			SetSize(int size);
	INLINE	void			ChangeSize(int size, bool makeZero = false);
	INLINE	int				GetSize() const { return size; }
	INLINE	void			SetData(int length, float* data);
	INLINE	void			Zero();
	INLINE	void			Zero(int length);
	INLINE	void			Random(int seed, float l = 0.0f, float u = 1.0f);
	INLINE	void			Random(int length, int seed, float l = 0.0f, float u = 1.0f);
	INLINE	void			Negate();
	INLINE	void			Clamp(float min, float max);
	INLINE	cweeVecX&			SwapElements(int e1, int e2);

	INLINE	float			Length() const;
	INLINE	float			LengthSqr() const;
	INLINE	cweeVecX			Normalize() const;
	INLINE	float			NormalizeSelf();

	INLINE	int				GetDimension() const;

	INLINE	void			AddScaleAdd(const float scale, const cweeVecX& v0, const cweeVecX& v1);

	INLINE	const float*	ToFloatPtr() const;
	INLINE	float*			ToFloatPtr();

private:
	int						size;					// size of the vector
	int						alloced;				// if -1 p points to data set with SetData
	float*					p;						// memory the vector is stored

	static float			temp[VECX_MAX_TEMP + 4];	// used to store intermediate results
	static float*			tempPtr;				// pointer to 16 byte aligned temporary memory
	static int				tempIndex;				// index into memory pool, wraps around

	INLINE void				SetTempSize(int size);
};

/*
========================
cweeVecX::cweeVecX
========================
*/
INLINE cweeVecX::cweeVecX() {
	size = alloced = 0;
	p = NULL;
}

/*
========================
cweeVecX::cweeVecX
========================
*/
INLINE cweeVecX::cweeVecX(int length) {
	size = alloced = 0;
	p = NULL;
	SetSize(length);
}

/*
========================
cweeVecX::cweeVecX
========================
*/
INLINE cweeVecX::cweeVecX(int length, float* data) {
	size = alloced = 0;
	p = NULL;
	SetData(length, data);
}

/*
========================
cweeVecX::~cweeVecX
========================
*/
INLINE cweeVecX::~cweeVecX() {
	// if not temp memory
	if (p && (p < cweeVecX::tempPtr || p >= cweeVecX::tempPtr + VECX_MAX_TEMP) && alloced != -1) {
		Mem_Free16(p);
	}
}

/*
========================
cweeVecX::Get
========================
*/
INLINE float cweeVecX::Get(int index) const {
	assert(index >= 0 && index < size);
	return p[index];
}

/*
========================
cweeVecX::Get
========================
*/
INLINE float& cweeVecX::Get(int index) {
	assert(index >= 0 && index < size);
	return p[index];
}

/*
========================
cweeVecX::operator[]
========================
*/
INLINE float cweeVecX::operator[](int index) const {
	return Get(index);
}

/*
========================
cweeVecX::operator[]
========================
*/
INLINE float& cweeVecX::operator[](int index) {
	return Get(index);
}

/*
========================
cweeVecX::operator-
========================
*/
INLINE cweeVecX cweeVecX::operator-() const {
	cweeVecX m;

	m.SetTempSize(size);
#if defined(ID_WIN_X86_SSE_INTRIN) && defined(VECX_SIMD)
	ALIGN16(unsigned int signBit[4]) = { IEEE_FLT_SIGN_MASK, IEEE_FLT_SIGN_MASK, IEEE_FLT_SIGN_MASK, IEEE_FLT_SIGN_MASK };
	for (int i = 0; i < size; i += 4) {
		_mm_store_ps(m.p + i, _mm_xor_ps(_mm_load_ps(p + i), (__m128&) signBit[0]));
	}
#else
	for (int i = 0; i < size; i++) {
		m.p[i] = -p[i];
	}
#endif
	return m;
}

/*
========================
cweeVecX::operator=
========================
*/
INLINE cweeVecX& cweeVecX::operator=(const cweeVecX& a) {
	SetSize(a.size);
#if defined(ID_WIN_X86_SSE_INTRIN) && defined(VECX_SIMD)
	for (int i = 0; i < a.size; i += 4) {
		_mm_store_ps(p + i, _mm_load_ps(a.p + i));
	}
#else
	memcpy(p, a.p, a.size * sizeof(float));
#endif
	cweeVecX::tempIndex = 0;
	return *this;
}

/*
========================
cweeVecX::operator+
========================
*/
INLINE cweeVecX cweeVecX::operator+(const cweeVecX& a) const {
	cweeVecX m;

	assert(size == a.size);
	m.SetTempSize(size);
#if defined(ID_WIN_X86_SSE_INTRIN) && defined(VECX_SIMD)
	for (int i = 0; i < size; i += 4) {
		_mm_store_ps(m.p + i, _mm_add_ps(_mm_load_ps(p + i), _mm_load_ps(a.p + i)));
	}
#else
	for (int i = 0; i < size; i++) {
		m.p[i] = p[i] + a.p[i];
	}
#endif
	return m;
}

/*
========================
cweeVecX::operator-
========================
*/
INLINE cweeVecX cweeVecX::operator-(const cweeVecX& a) const {
	cweeVecX m;

	assert(size == a.size);
	m.SetTempSize(size);
#if defined(ID_WIN_X86_SSE_INTRIN) && defined(VECX_SIMD)
	for (int i = 0; i < size; i += 4) {
		_mm_store_ps(m.p + i, _mm_sub_ps(_mm_load_ps(p + i), _mm_load_ps(a.p + i)));
	}
#else
	for (int i = 0; i < size; i++) {
		m.p[i] = p[i] - a.p[i];
	}
#endif
	return m;
}

/*
========================
cweeVecX::operator+=
========================
*/
INLINE cweeVecX& cweeVecX::operator+=(const cweeVecX& a) {
	assert(size == a.size);
#if defined(ID_WIN_X86_SSE_INTRIN) && defined(VECX_SIMD)
	for (int i = 0; i < size; i += 4) {
		_mm_store_ps(p + i, _mm_add_ps(_mm_load_ps(p + i), _mm_load_ps(a.p + i)));
	}
#else
	for (int i = 0; i < size; i++) {
		p[i] += a.p[i];
	}
#endif
	cweeVecX::tempIndex = 0;
	return *this;
}

/*
========================
cweeVecX::operator-=
========================
*/
INLINE cweeVecX& cweeVecX::operator-=(const cweeVecX& a) {
	assert(size == a.size);
#if defined(ID_WIN_X86_SSE_INTRIN) && defined(VECX_SIMD)
	for (int i = 0; i < size; i += 4) {
		_mm_store_ps(p + i, _mm_sub_ps(_mm_load_ps(p + i), _mm_load_ps(a.p + i)));
	}
#else
	for (int i = 0; i < size; i++) {
		p[i] -= a.p[i];
	}
#endif
	cweeVecX::tempIndex = 0;
	return *this;
}

/*
========================
cweeVecX::operator*
========================
*/
INLINE cweeVecX cweeVecX::operator*(const float a) const {
	cweeVecX m;

	m.SetTempSize(size);
#if defined(ID_WIN_X86_SSE_INTRIN) && defined(VECX_SIMD)
	__m128 va = _mm_load1_ps(&a);
	for (int i = 0; i < size; i += 4) {
		_mm_store_ps(m.p + i, _mm_mul_ps(_mm_load_ps(p + i), va));
	}
#else
	for (int i = 0; i < size; i++) {
		m.p[i] = p[i] * a;
	}
#endif
	return m;
}

/*
========================
cweeVecX::operator*=
========================
*/
INLINE cweeVecX& cweeVecX::operator*=(const float a) {
#if defined(ID_WIN_X86_SSE_INTRIN) && defined(VECX_SIMD)
	__m128 va = _mm_load1_ps(&a);
	for (int i = 0; i < size; i += 4) {
		_mm_store_ps(p + i, _mm_mul_ps(_mm_load_ps(p + i), va));
	}
#else
	for (int i = 0; i < size; i++) {
		p[i] *= a;
	}
#endif
	return *this;
}

/*
========================
cweeVecX::operator/
========================
*/
INLINE cweeVecX cweeVecX::operator/(const float a) const {
	assert(fabs(a) > cweeMath::EPSILON);
	return (*this) * (1.0f / a);
}

/*
========================
cweeVecX::operator/=
========================
*/
INLINE cweeVecX& cweeVecX::operator/=(const float a) {
	assert(fabs(a) > cweeMath::EPSILON);
	(*this) *= (1.0f / a);
	return *this;
}

/*
========================
operator*
========================
*/
INLINE cweeVecX operator*(const float a, const cweeVecX& b) {
	return b * a;
}

/*
========================
cweeVecX::operator*
========================
*/
INLINE float cweeVecX::operator*(const cweeVecX& a) const {
	assert(size == a.size);
	float sum = 0.0f;
	for (int i = 0; i < size; i++) {
		sum += p[i] * a.p[i];
	}
	return sum;
}

/*
========================
cweeVecX::Compare
========================
*/
INLINE bool cweeVecX::Compare(const cweeVecX& a) const {
	assert(size == a.size);
	for (int i = 0; i < size; i++) {
		if (p[i] != a.p[i]) {
			return false;
		}
	}
	return true;
}

/*
========================
cweeVecX::Compare
========================
*/
INLINE bool cweeVecX::Compare(const cweeVecX& a, const float epsilon) const {
	assert(size == a.size);
	for (int i = 0; i < size; i++) {
		if (cweeMath::Fabs(p[i] - a.p[i]) > epsilon) {
			return false;
		}
	}
	return true;
}

/*
========================
cweeVecX::operator==
========================
*/
INLINE bool cweeVecX::operator==(const cweeVecX& a) const {
	return Compare(a);
}

/*
========================
cweeVecX::operator!=
========================
*/
INLINE bool cweeVecX::operator!=(const cweeVecX& a) const {
	return !Compare(a);
}

/*
========================
cweeVecX::SetSize
========================
*/
INLINE void cweeVecX::SetSize(int newSize) {
	//assert( p < cweeVecX::tempPtr || p > cweeVecX::tempPtr + VECX_MAX_TEMP );
	if (newSize != size || p == NULL) {
		int alloc = (newSize + 3) & ~3;
		if (alloc > alloced && alloced != -1) {
			if (p) {
				Mem_Free16(p);
			}
			p = (float*)Mem_Alloc16((size_t)(alloc * sizeof(float)), TAG_MATH);
			alloced = alloc;
		}
		size = newSize;
		VECX_CLEAREND();
	}
}

/*
========================
cweeVecX::ChangeSize
========================
*/
INLINE void cweeVecX::ChangeSize(int newSize, bool makeZero) {
	if (newSize != size) {
		int alloc = (newSize + 3) & ~3;
		if (alloc > alloced && alloced != -1) {
			float* oldVec = p;
			p = (float*)Mem_Alloc16((size_t)(alloc * sizeof(float)), TAG_MATH);
			alloced = alloc;
			if (oldVec) {
				for (int i = 0; i < size; i++) {
					p[i] = oldVec[i];
				}
				Mem_Free16(oldVec);
			}
			if (makeZero) {
				// zero any new elements
				for (int i = size; i < newSize; i++) {
					p[i] = 0.0f;
				}
			}
		}
		size = newSize;
		VECX_CLEAREND();
	}
}

/*
========================
cweeVecX::SetTempSize
========================
*/
INLINE void cweeVecX::SetTempSize(int newSize) {
	size = newSize;
	alloced = (newSize + 3) & ~3;
	assert(alloced < VECX_MAX_TEMP);
	if (cweeVecX::tempIndex + alloced > VECX_MAX_TEMP) {
		cweeVecX::tempIndex = 0;
	}
	p = cweeVecX::tempPtr + cweeVecX::tempIndex;
	cweeVecX::tempIndex += alloced;
	VECX_CLEAREND();
}

/*
========================
cweeVecX::SetData
========================
*/
INLINE void cweeVecX::SetData(int length, float* data) {
	if (p != NULL && (p < cweeVecX::tempPtr || p >= cweeVecX::tempPtr + VECX_MAX_TEMP) && alloced != -1) {
		Mem_Free16(p);
	}
	assert_16_byte_aligned(data); // data must be 16 byte aligned
	p = data;
	size = length;
	alloced = -1;
	VECX_CLEAREND();
}

/*
========================
cweeVecX::Zero
========================
*/
INLINE void cweeVecX::Zero() {
#if defined(ID_WIN_X86_SSE_INTRIN) && defined(VECX_SIMD)
	for (int i = 0; i < size; i += 4) {
		_mm_store_ps(p + i, _mm_setzero_ps());
	}
#else
	memset(p, 0, size * sizeof(float));
#endif
}

/*
========================
cweeVecX::Zero
========================
*/
INLINE void cweeVecX::Zero(int length) {
	SetSize(length);
#if defined(ID_WIN_X86_SSE_INTRIN) && defined(VECX_SIMD)
	for (int i = 0; i < length; i += 4) {
		_mm_store_ps(p + i, _mm_setzero_ps());
	}
#else
	memset(p, 0, length * sizeof(float));
#endif
}

/*
========================
cweeVecX::Random
========================
*/
INLINE void cweeVecX::Random(int seed, float l, float u) {
	
	float c = u - l;
	for (int i = 0; i < size; i++) {
		p[i] = l + cweeRandomFloat(0,1) * c;
	}
}

/*
========================
cweeVecX::Random
========================
*/
INLINE void cweeVecX::Random(int length, int seed, float l, float u) {

	SetSize(length);
	float c = u - l;
	for (int i = 0; i < size; i++) {
		p[i] = l + cweeRandomFloat(0, 1) * c;
	}
}

/*
========================
cweeVecX::Negate
========================
*/
INLINE void cweeVecX::Negate() {
#if defined(ID_WIN_X86_SSE_INTRIN) && defined(VECX_SIMD)
	ALIGN16(const unsigned int signBit[4]) = { IEEE_FLT_SIGN_MASK, IEEE_FLT_SIGN_MASK, IEEE_FLT_SIGN_MASK, IEEE_FLT_SIGN_MASK };
	for (int i = 0; i < size; i += 4) {
		_mm_store_ps(p + i, _mm_xor_ps(_mm_load_ps(p + i), (__m128&) signBit[0]));
	}
#else
	for (int i = 0; i < size; i++) {
		p[i] = -p[i];
	}
#endif
}

/*
========================
cweeVecX::Clamp
========================
*/
INLINE void cweeVecX::Clamp(float min, float max) {
	for (int i = 0; i < size; i++) {
		if (p[i] < min) {
			p[i] = min;
		}
		else if (p[i] > max) {
			p[i] = max;
		}
	}
}

/*
========================
cweeVecX::SwapElements
========================
*/
INLINE cweeVecX& cweeVecX::SwapElements(int e1, int e2) {
	float tmp;
	tmp = p[e1];
	p[e1] = p[e2];
	p[e2] = tmp;
	return *this;
}

/*
========================
cweeVecX::Length
========================
*/
INLINE float cweeVecX::Length() const {
	float sum = 0.0f;
	for (int i = 0; i < size; i++) {
		sum += p[i] * p[i];
	}
	return cweeMath::Sqrt(sum);
}

/*
========================
cweeVecX::LengthSqr
========================
*/
INLINE float cweeVecX::LengthSqr() const {
	float sum = 0.0f;
	for (int i = 0; i < size; i++) {
		sum += p[i] * p[i];
	}
	return sum;
}

/*
========================
cweeVecX::Normalize
========================
*/
INLINE cweeVecX cweeVecX::Normalize() const {
	cweeVecX m;

	m.SetTempSize(size);
	float sum = 0.0f;
	for (int i = 0; i < size; i++) {
		sum += p[i] * p[i];
	}
	float invSqrt = cweeMath::InvSqrt(sum);
	for (int i = 0; i < size; i++) {
		m.p[i] = p[i] * invSqrt;
	}
	return m;
}

/*
========================
cweeVecX::NormalizeSelf
========================
*/
INLINE float cweeVecX::NormalizeSelf() {
	float sum = 0.0f;
	for (int i = 0; i < size; i++) {
		sum += p[i] * p[i];
	}
	float invSqrt = cweeMath::InvSqrt(sum);
	for (int i = 0; i < size; i++) {
		p[i] *= invSqrt;
	}
	return invSqrt * sum;
}

/*
========================
cweeVecX::GetDimension
========================
*/
INLINE int cweeVecX::GetDimension() const {
	return size;
}

/*
========================
cweeVecX::ToFloatPtr
========================
*/
INLINE const float* cweeVecX::ToFloatPtr() const {
	return p;
}

/*
========================
cweeVecX::ToFloatPtr
========================
*/
INLINE float* cweeVecX::ToFloatPtr() {
	return p;
}

/*
========================
cweeVecX::AddScaleAdd
========================
*/
INLINE void cweeVecX::AddScaleAdd(const float scale, const cweeVecX& v0, const cweeVecX& v1) {
	assert(GetSize() == v0.GetSize());
	assert(GetSize() == v1.GetSize());

	const float* v0Ptr = v0.ToFloatPtr();
	const float* v1Ptr = v1.ToFloatPtr();
	float* dstPtr = ToFloatPtr();

	for (int i = 0; i < size; i++) {
		dstPtr[i] += scale * (v0Ptr[i] + v1Ptr[i]);
	}
}

#endif
