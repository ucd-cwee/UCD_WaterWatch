#pragma once

//===============================================================
//	vec2d - 2D vector
//===============================================================
class vec2d {
public:
	double			x;
	double			y;

	vec2d();
	explicit vec2d(const double x, const double y);

	void 			Set(const double x, const double y);
	void			Zero();

	double			first() { return x; };
	double			second() { return y; };

	double			operator[](int index) const;
	double& operator[](int index);
	vec2d			operator-() const;
	double			operator*(const vec2d& a) const;
	vec2d			operator*(const double a) const;
	vec2d			operator/(const double a) const;
	vec2d			operator+(const vec2d& a) const;
	vec2d			operator-(const vec2d& a) const;
	vec2d& operator+=(const vec2d& a);
	vec2d& operator-=(const vec2d& a);
	vec2d& operator/=(const vec2d& a);
	vec2d& operator/=(const double a);
	vec2d& operator*=(const double a);

	friend vec2d	operator*(const double a, const vec2d b);

	vec2d			Scale(const vec2d& a) const;

	bool			Compare(const vec2d& a) const;							// exact compare, no epsilon
	bool			operator==(const vec2d& a) const;						// exact compare, no epsilon
	bool			operator!=(const vec2d& a) const;						// exact compare, no epsilon

	double			Length() const;
	double			LengthFast() const;
	double			LengthSqr() const;
	double			Normalize();			// returns length
	double			NormalizeFast();		// returns length
	vec2d			Truncate(double length) const;	// cap length
	void			Clamp(const vec2d& min, const vec2d& max);
	void			Snap();				// snap to closest integer value
	void			SnapInt();			// snap towards integer (floor)

	double			Dot(const vec2d& a) const {
		vec2d temp = *this;
		vec2d tempA = a;

		temp.Normalize();
		tempA.Normalize();

		return temp.x * tempA.x + temp.y * tempA.y;
	};

	int				GetDimension() const;

	const char* ToString(int precision = 2) const;
	vec2d& FromString(const cweeStr& in);

	operator			const char* () const {
		return ToString();
	};
	operator			const char* () {
		return ToString();
	};

	double			Distance(const vec2d& a) const;

	void			Lerp(const vec2d& v1, const vec2d& v2, const double l);
};

INLINE vec2d::vec2d() {
}

INLINE vec2d::vec2d(const double x, const double y) {
	this->x = x;
	this->y = y;
}

INLINE void vec2d::Set(const double x, const double y) {
	this->x = x;
	this->y = y;
}

INLINE double vec2d::Distance(const vec2d& a) const {
	double out(0);
	out += (this->x - a.x) * (this->x - a.x);
	out += (this->y - a.y) * (this->y - a.y);
	return ::sqrt(out);
}

INLINE void vec2d::Zero() {
	x = y = 0.0f;
}

INLINE bool vec2d::Compare(const vec2d& a) const {
	return ((x == a.x) && (y == a.y));
}

INLINE const char* vec2d::ToString(int precision) const {
	return cweeStr(x) + "," + cweeStr(y);
}

INLINE vec2d& vec2d::FromString(const cweeStr& in) {
	int finder(0); cweeStr left; cweeStr right;
	finder = in.Find(',');
	if (finder != -1) {
		in.Mid(0, finder, left);
		in.Mid(finder + 1, in.Length(), right);
		x = (double)left;
		y = (double)right;
	}
	return *this;
};

INLINE void vec2d::Lerp(const vec2d& v1, const vec2d& v2, const double l) {
	if (l <= 0.0f) {
		(*this) = v1;
	}
	else if (l >= 1.0f) {
		(*this) = v2;
	}
	else {
		(*this) = v1 + l * (v2 - v1);
	}
}

INLINE bool vec2d::operator==(const vec2d& a) const {
	return Compare(a);
}

INLINE bool vec2d::operator!=(const vec2d& a) const {
	return !Compare(a);
}

INLINE double vec2d::operator[](int index) const {
	return (&x)[index];
}

INLINE double& vec2d::operator[](int index) {
	return (&x)[index];
}

INLINE double vec2d::Length() const {
	return (double)cweeMath::Sqrt(x * x + y * y);
}

INLINE double vec2d::LengthFast() const {
	double sqrLength;

	sqrLength = x * x + y * y;
	return sqrLength * cweeMath::InvSqrt(sqrLength);
}

INLINE double vec2d::LengthSqr() const {
	return (x * x + y * y);
}

INLINE double vec2d::Normalize() {
	double sqrLength, invLength;

	sqrLength = x * x + y * y;
	invLength = cweeMath::InvSqrt(sqrLength);
	x *= invLength;
	y *= invLength;
	return invLength * sqrLength;
}

INLINE double vec2d::NormalizeFast() {
	double lengthSqr, invLength;

	lengthSqr = x * x + y * y;
	invLength = cweeMath::InvSqrt(lengthSqr);
	x *= invLength;
	y *= invLength;
	return invLength * lengthSqr;
}

INLINE vec2d vec2d::Truncate(double length) const {
	if (length < cweeMath::EPSILON) {
		return vec2d();
	}
	else {
		double length2 = LengthSqr();
		if (length2 > length * length) {
			double ilength = length * cweeMath::InvSqrt(length2);
			return *this * ilength;
		}
	}
	return *this;
}

INLINE void vec2d::Clamp(const vec2d& min, const vec2d& max) {
	if (x < min.x) {
		x = min.x;
	}
	else if (x > max.x) {
		x = max.x;
	}
	if (y < min.y) {
		y = min.y;
	}
	else if (y > max.y) {
		y = max.y;
	}
}

INLINE void vec2d::Snap() {
	x = floor(x + 0.5f);
	y = floor(y + 0.5f);
}

INLINE void vec2d::SnapInt() {
	x = double(int(x));
	y = double(int(y));
}

INLINE vec2d vec2d::operator-() const {
	return vec2d(-x, -y);
}

INLINE vec2d vec2d::operator-(const vec2d& a) const {
	return vec2d(x - a.x, y - a.y);
}

INLINE double vec2d::operator*(const vec2d& a) const {
	return x * a.x + y * a.y;
}

INLINE vec2d vec2d::operator*(const double a) const {
	return vec2d(x * a, y * a);
}

INLINE vec2d vec2d::operator/(const double a) const {
	double inva = 1.0f / a;
	return vec2d(x * inva, y * inva);
}

INLINE vec2d operator*(const double a, const vec2d b) {
	return vec2d(b.x * a, b.y * a);
}

INLINE vec2d vec2d::operator+(const vec2d& a) const {
	return vec2d(x + a.x, y + a.y);
}

INLINE vec2d& vec2d::operator+=(const vec2d& a) {
	x += a.x;
	y += a.y;

	return *this;
}

INLINE vec2d& vec2d::operator/=(const vec2d& a) {
	x /= a.x;
	y /= a.y;

	return *this;
}

INLINE vec2d& vec2d::operator/=(const double a) {
	double inva = 1.0f / a;
	x *= inva;
	y *= inva;

	return *this;
}

INLINE vec2d& vec2d::operator-=(const vec2d& a) {
	x -= a.x;
	y -= a.y;

	return *this;
}

INLINE vec2d& vec2d::operator*=(const double a) {
	x *= a;
	y *= a;

	return *this;
}

INLINE vec2d vec2d::Scale(const vec2d& a) const {
	return vec2d(x * a.x, y * a.y);
}

INLINE int vec2d::GetDimension() const {
	return 2;
}


//===============================================================
//	vec2 - 2D vector
//===============================================================
class vec2 {
public:
	float			x;
	float			y;

	vec2();
	explicit vec2(const float x, const float y);

	void 			Set(const float x, const float y);
	void			Zero();

	float			first() { return x; };
	float			second() { return y; };

	float			operator[](int index) const;
	float& operator[](int index);
	vec2			operator-() const;
	float			operator*(const vec2& a) const;
	vec2			operator*(const float a) const;
	vec2			operator/(const float a) const;
	vec2			operator+(const vec2& a) const;
	vec2			operator-(const vec2& a) const;
	vec2& operator+=(const vec2& a);
	vec2& operator-=(const vec2& a);
	vec2& operator/=(const vec2& a);
	vec2& operator/=(const float a);
	vec2& operator*=(const float a);

	friend vec2	operator*(const float a, const vec2 b);

	vec2			Scale(const vec2& a) const;

	bool			Compare(const vec2& a) const;							// exact compare, no epsilon
	bool			operator==(const vec2& a) const;						// exact compare, no epsilon
	bool			operator!=(const vec2& a) const;						// exact compare, no epsilon

	float			Length() const;
	float			LengthFast() const;
	float			LengthSqr() const;
	float			Normalize();			// returns length
	float			NormalizeFast();		// returns length
	vec2			Truncate(float length) const;	// cap length
	void			Clamp(const vec2& min, const vec2& max);
	void			Snap();				// snap to closest integer value
	void			SnapInt();			// snap towards integer (floor)

	float			Dot(const vec2& a) const {
		vec2 temp = *this;
		vec2 tempA = a;

		temp.Normalize();
		tempA.Normalize();

		return temp.x * tempA.x + temp.y * tempA.y;
	};

	int				GetDimension() const;

	const float* ToFloatPtr() const;
	float* ToFloatPtr();
	const char* ToString(int precision = 2) const;

	operator			const char* () const {
		return ToString();
	};
	operator			const char* () {
		return ToString();
	};

	float			Distance(const vec2& a);

	void			Lerp(const vec2& v1, const vec2& v2, const float l);
};

INLINE vec2::vec2() {
}

INLINE vec2::vec2(const float x, const float y) {
	this->x = x;
	this->y = y;
}

INLINE void vec2::Set(const float x, const float y) {
	this->x = x;
	this->y = y;
}

INLINE float vec2::Distance(const vec2& a) {
	float out(0);
	out += (this->x - a.x) * (this->x - a.x);
	out += (this->y - a.y) * (this->y - a.y);
	return ::sqrt(out);
}

INLINE void vec2::Zero() {
	x = y = 0.0f;
}

INLINE bool vec2::Compare(const vec2& a) const {
	return ((x == a.x) && (y == a.y));
}

INLINE const char* vec2::ToString(int precision) const {
	return cweeStr(x) + "," + cweeStr(y);
}

INLINE void vec2::Lerp(const vec2& v1, const vec2& v2, const float l) {
	if (l <= 0.0f) {
		(*this) = v1;
	}
	else if (l >= 1.0f) {
		(*this) = v2;
	}
	else {
		(*this) = v1 + l * (v2 - v1);
	}
}

INLINE bool vec2::operator==(const vec2& a) const {
	return Compare(a);
}

INLINE bool vec2::operator!=(const vec2& a) const {
	return !Compare(a);
}

INLINE float vec2::operator[](int index) const {
	return (&x)[index];
}

INLINE float& vec2::operator[](int index) {
	return (&x)[index];
}

INLINE float vec2::Length() const {
	return (float)cweeMath::Sqrt(x * x + y * y);
}

INLINE float vec2::LengthFast() const {
	float sqrLength;

	sqrLength = x * x + y * y;
	return sqrLength * cweeMath::InvSqrt(sqrLength);
}

INLINE float vec2::LengthSqr() const {
	return (x * x + y * y);
}

INLINE float vec2::Normalize() {
	float sqrLength, invLength;

	sqrLength = x * x + y * y;
	invLength = cweeMath::InvSqrt(sqrLength);
	x *= invLength;
	y *= invLength;
	return invLength * sqrLength;
}

INLINE float vec2::NormalizeFast() {
	float lengthSqr, invLength;

	lengthSqr = x * x + y * y;
	invLength = cweeMath::InvSqrt(lengthSqr);
	x *= invLength;
	y *= invLength;
	return invLength * lengthSqr;
}

INLINE vec2 vec2::Truncate(float length) const {
	if (length < cweeMath::EPSILON) {
		return vec2();
	}
	else {
		float length2 = LengthSqr();
		if (length2 > length * length) {
			float ilength = length * cweeMath::InvSqrt(length2);
			return *this * ilength;
		}
	}
	return *this;
}

INLINE void vec2::Clamp(const vec2& min, const vec2& max) {
	if (x < min.x) {
		x = min.x;
	}
	else if (x > max.x) {
		x = max.x;
	}
	if (y < min.y) {
		y = min.y;
	}
	else if (y > max.y) {
		y = max.y;
	}
}

INLINE void vec2::Snap() {
	x = floor(x + 0.5f);
	y = floor(y + 0.5f);
}

INLINE void vec2::SnapInt() {
	x = float(int(x));
	y = float(int(y));
}

INLINE vec2 vec2::operator-() const {
	return vec2(-x, -y);
}

INLINE vec2 vec2::operator-(const vec2& a) const {
	return vec2(x - a.x, y - a.y);
}

INLINE float vec2::operator*(const vec2& a) const {
	return x * a.x + y * a.y;
}

INLINE vec2 vec2::operator*(const float a) const {
	return vec2(x * a, y * a);
}

INLINE vec2 vec2::operator/(const float a) const {
	float inva = 1.0f / a;
	return vec2(x * inva, y * inva);
}

INLINE vec2 operator*(const float a, const vec2 b) {
	return vec2(b.x * a, b.y * a);
}

INLINE vec2 vec2::operator+(const vec2& a) const {
	return vec2(x + a.x, y + a.y);
}

INLINE vec2& vec2::operator+=(const vec2& a) {
	x += a.x;
	y += a.y;

	return *this;
}

INLINE vec2& vec2::operator/=(const vec2& a) {
	x /= a.x;
	y /= a.y;

	return *this;
}

INLINE vec2& vec2::operator/=(const float a) {
	float inva = 1.0f / a;
	x *= inva;
	y *= inva;

	return *this;
}

INLINE vec2& vec2::operator-=(const vec2& a) {
	x -= a.x;
	y -= a.y;

	return *this;
}

INLINE vec2& vec2::operator*=(const float a) {
	x *= a;
	y *= a;

	return *this;
}

INLINE vec2 vec2::Scale(const vec2& a) const {
	return vec2(x * a.x, y * a.y);
}

INLINE int vec2::GetDimension() const {
	return 2;
}

INLINE const float* vec2::ToFloatPtr() const {
	return &x;
}

INLINE float* vec2::ToFloatPtr() {
	return &x;
}

//===============================================================
//	vec3 - 3D vector
//===============================================================
class vec3 {
public:
	float			x = 0;
	float			y = 0;
	float			z = 0;
	vec3(void) {
	};

	explicit		vec3(const cweeStr& a) {
		cweeParser obj(a, ",", true);
		for (int i = 0; i < obj.getNumVars() && i < 3; i++) {
			this->operator[](i) = (float)obj[i];
		}
	};
	explicit		vec3(float a) {
		x = y = z = a;
	};
	explicit		vec3(const float x, const float y, const float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	};

	void 			Set(const float x, const float y, const float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	};
	void			Zero(void) {
		x = y = z = 0.0f;
	};
	float			operator[](const int index) const {
		return (&x)[index];
	};
	float& operator[](const int index) {
		return (&x)[index];
	};

	explicit operator float() const {
		return (x + y + z) * 0.3333333333333f;
	};
	explicit operator float() {
		return (x + y + z) * 0.3333333333333f;
	};

	vec3			operator-() const { return vec3(-x, -y, -z); };
	//vec3			operator-(const vec3& a) const { return vec3(x - a.x, y - a.y, z - a.z); };
	//vec3			operator+(const vec3& a) const { return vec3(x + a.x, y + a.y, z + a.z); };
	//vec3			operator/(const vec3& a) const { return vec3(x / a.x, y / a.y, z / a.z); };
	//vec3			operator*(const vec3& a) const { return vec3(x * a.x, y * a.y, z * a.z); };
	vec3& operator-(const vec3& a) { x -= a.x; y -= a.y; z -= a.z; return *this; };
	vec3& operator+(const vec3& a) { x += a.x; y += a.y; z += a.z; return *this; };
	vec3& operator/(const vec3& a) { x /= a.x; y /= a.y; z /= a.z; return *this; };
	vec3& operator*(const vec3& a) { x *= a.x; y *= a.y; z *= a.z; return *this; };
	vec3& operator-=(const vec3& a) { x -= a.x; y -= a.y; z -= a.z; return *this; };
	vec3& operator+=(const vec3& a) { x += a.x; y += a.y; z += a.z; return *this; };
	vec3& operator/=(const vec3& a) { x /= a.x; y /= a.y; z /= a.z; return *this; };
	vec3& operator*=(const vec3& a) { x *= a.x; y *= a.y; z *= a.z; return *this; };

	//vec3			operator-(float a) const { return vec3(x - a, y - a, z - a); };
	//vec3			operator+(float a) const { return vec3(x + a, y + a, z + a); };
	//vec3			operator/(float a) const { return vec3(x / a, y / a, z / a); };
	//vec3			operator*(float a) const { return vec3(x * a, y * a, z * a); };
	vec3& operator-(float a) { x -= a; y -= a; z -= a; return *this; };
	vec3& operator+(float a) { x += a; y += a; z += a; return *this; };
	vec3& operator/(float a) { x /= a; y /= a; z /= a; return *this; };
	vec3& operator*(float a) { x *= a; y *= a; z *= a; return *this; };
	vec3& operator-=(float a) { x -= a; y -= a; z -= a; return *this; };
	vec3& operator+=(float a) { x += a; y += a; z += a; return *this; };
	vec3& operator/=(float a) { x /= a; y /= a; z /= a; return *this; };
	vec3& operator*=(float a) { x *= a; y *= a; z *= a; return *this; };

	friend vec3		operator+(const vec3& a, const vec3& b) {
		vec3 result(a);
		result += b;
		return result;
	};
	friend vec3		operator+(const vec3& a, float b) {
		vec3 result(a);
		result += b;
		return result;
	};
	friend vec3		operator+(float a, const vec3& b) {
		vec3 result(b);
		result += a;
		return result;
	};

	friend vec3		operator-(const vec3& a, const vec3& b) {
		vec3 result(a);
		result -= b;
		return result;
	};
	friend vec3		operator-(const vec3& a, float b) {
		vec3 result(a);
		result -= b;
		return result;
	};
	friend vec3		operator-(float a, const vec3& b) {
		vec3 result(-b);
		result += a;
		return result;
	};

	friend vec3		operator*(const vec3& a, const vec3& b) {
		vec3 result(a);
		result *= b;
		return result;
	};
	friend vec3		operator*(const vec3& a, float b) {
		vec3 result(a);
		result *= b;
		return result;
	};
	friend vec3		operator*(float a, const vec3& b) {
		vec3 result(b);
		result *= a;
		return result;
	};

	friend vec3		operator/(const vec3& a, const vec3& b) {
		vec3 result(a);
		result /= b;
		return result;
	};
	friend vec3		operator/(const vec3& a, float b) {
		vec3 result(a);
		result /= b;
		return result;
	};
	friend vec3		operator/(float a, const vec3& b) {
		vec3 result;
		result.x = 1 / b.x;
		result.y = 1 / b.y;
		result.z = 1 / b.z;

		result *= a;
		return result;
	};
	vec3& operator=(const vec3& a) {
		this->x = a.x;
		this->y = a.y;
		this->z = a.z;

		return *this;
	};
	vec3& operator=(float a) {
		x = y = z = a;
		return *this;
	};

	bool			Compare(const vec3& a) const {
		return ((x == a.x) && (y == a.y) && (z == a.z));
	};							// exact compare, no epsilon
	bool			operator==(const vec3& a) const {
		return Compare(a);
	};						// exact compare, no epsilon
	bool			operator!=(const vec3& a) const {
		return !Compare(a);
	};						// exact compare, no epsilon
	bool			operator<(const vec3& a) const {
		return ((float)*this) < ((float)a);
	};
	bool			operator<=(const vec3& a) const {
		return ((float)*this) <= ((float)a);
	};
	bool			operator>(const vec3& a) const {
		return ((float)*this) > ((float)a);
	};
	bool			operator>=(const vec3& a) const {
		return ((float)*this) >= ((float)a);
	};

	float			ScalarProjectionOnto(const vec3& B) const {
		return (float)((x * B.x + y * B.y) * cweeMath::InvSqrt(B.x * B.x + B.y * B.y));
	};

	vec2			GetVec2() const {
		return vec2(x, y);
	};

	float			Distance2d(const vec3& a) const {
		float out(0);
		out += ((this->x - a.x) * (this->x - a.x));
		out += ((this->y - a.y) * (this->y - a.y));
		return cweeMath::Sqrt(out);
	};

	float			Distance(const vec3& a) const {
		float out(0);
		out += pow(this->x - a.x, 2);
		out += pow(this->y - a.y, 2);
		out += pow(this->z - a.z, 2);
		return cweeMath::Sqrt(out);
	};

	void Normalize() {
		float sqrLength, invLength;

		sqrLength = x * x + y * y + z * z;
		invLength = cweeMath::InvSqrt(sqrLength);
		x *= invLength;
		y *= invLength;
		z *= invLength;
	}

	cweeStr			ToString(void) const {
		return cweeStr::printf("%s,%s,%s",
			cweeStr(x).c_str(),
			cweeStr(y).c_str(),
			cweeStr(z).c_str()
		);


		// return ((cweeStr)x + "," + (cweeStr)y + "," + (cweeStr)z);
		// return cweeStr::printf("%f,%f,%f", x, y, z);
	};

	vec3& FromString(const cweeStr& v) {
		int finder(0); cweeStr left; cweeStr right;
		finder = v.Find(',');
		if (finder != -1) {
			v.Mid(0, finder, left);
			v.Mid(finder + 1, v.Length(), right);
			x = (float)left;
		}
		finder = right.Find(',');
		if (finder != -1) {
			right.Mid(0, finder, left);
			right.Mid(finder + 1, right.Length(), right);
			y = (float)left;
			z = (float)right;
		}

		//cweeParser p(v, ",");
		//if (p.getNumVars() >= 3) {
		//	x = (float)p[0];
		//	y = (float)p[1];
		//	z = (float)p[2];
		//}
		//else {
		//	Zero();
		//}

		return *this;
	};

	operator		const char* () const {
		return ToString();
	};
	operator		const char* () {
		return ToString();
	};

};
class vec3d {
public:
	double			x = 0;
	double			y = 0;
	double			z = 0;
	vec3d(void) {
	};

	explicit		vec3d(const cweeStr& a) {
		cweeParser obj(a, ",", true);
		for (int i = 0; i < obj.getNumVars() && i < 3; i++) {
			this->operator[](i) = (double)obj[i];
		}
	};
	explicit		vec3d(const double& a) {
		x = y = z = a;
	};
	explicit		vec3d(const double x, const double y, const double z) {
		this->x = x;
		this->y = y;
		this->z = z;
	};

	void 			Set(const double x, const double y, const double z) {
		this->x = x;
		this->y = y;
		this->z = z;
	};
	void			Zero(void) {
		x = y = z = 0.0f;
	};
	double			operator[](const int index) const {
		return (&x)[index];
	};
	double& operator[](const int index) {
		return (&x)[index];
	};

	explicit operator double() const {
		return (x + y + z) * 0.3333333333333f;
	};
	explicit operator double() {
		return (x + y + z) * 0.3333333333333f;
	};

	vec3d			operator-() const { return vec3d(-x, -y, -z); };
	//vec3d			operator-(const vec3d& a) const { return vec3d(x - a.x, y - a.y, z - a.z); };
	//vec3d			operator+(const vec3d& a) const { return vec3d(x + a.x, y + a.y, z + a.z); };
	//vec3d			operator/(const vec3d& a) const { return vec3d(x / a.x, y / a.y, z / a.z); };
	//vec3d			operator*(const vec3d& a) const { return vec3d(x * a.x, y * a.y, z * a.z); };
	vec3d& operator-(const vec3d& a) { x -= a.x; y -= a.y; z -= a.z; return *this; };
	vec3d& operator+(const vec3d& a) { x += a.x; y += a.y; z += a.z; return *this; };
	vec3d& operator/(const vec3d& a) { x /= a.x; y /= a.y; z /= a.z; return *this; };
	vec3d& operator*(const vec3d& a) { x *= a.x; y *= a.y; z *= a.z; return *this; };
	vec3d& operator-=(const vec3d& a) { x -= a.x; y -= a.y; z -= a.z; return *this; };
	vec3d& operator+=(const vec3d& a) { x += a.x; y += a.y; z += a.z; return *this; };
	vec3d& operator/=(const vec3d& a) { x /= a.x; y /= a.y; z /= a.z; return *this; };
	vec3d& operator*=(const vec3d& a) { x *= a.x; y *= a.y; z *= a.z; return *this; };

	//vec3d			operator-(const double& a) const { return vec3d(x - a, y - a, z - a); };
	//vec3d			operator+(const double& a) const { return vec3d(x + a, y + a, z + a); };
	//vec3d			operator/(const double& a) const { return vec3d(x / a, y / a, z / a); };
	//vec3d			operator*(const double& a) const { return vec3d(x * a, y * a, z * a); };
	vec3d& operator-(const double& a) { x -= a; y -= a; z -= a; return *this; };
	vec3d& operator+(const double& a) { x += a; y += a; z += a; return *this; };
	vec3d& operator/(const double& a) { x /= a; y /= a; z /= a; return *this; };
	vec3d& operator*(const double& a) { x *= a; y *= a; z *= a; return *this; };
	vec3d& operator-=(const double& a) { x -= a; y -= a; z -= a; return *this; };
	vec3d& operator+=(const double& a) { x += a; y += a; z += a; return *this; };
	vec3d& operator/=(const double& a) { x /= a; y /= a; z /= a; return *this; };
	vec3d& operator*=(const double& a) { x *= a; y *= a; z *= a; return *this; };

	friend vec3d	operator+(const vec3d& a, const vec3d& b) {
		vec3d result(a);
		result += b;
		return result;
	};
	friend vec3d	operator+(const vec3d& a, const double& b) {
		vec3d result(a);
		result += b;
		return result;
	};
	friend vec3d	operator+(const double& a, const vec3d& b) {
		vec3d result(b);
		result += a;
		return result;
	};

	friend vec3d	operator-(const vec3d& a, const vec3d& b) {
		vec3d result(a);
		result -= b;
		return result;
	};
	friend vec3d	operator-(const vec3d& a, const double& b) {
		vec3d result(a);
		result -= b;
		return result;
	};
	friend vec3d	operator-(const double& a, const vec3d& b) {
		vec3d result(-b);
		result += a;
		return result;
	};

	friend vec3d	operator*(const vec3d& a, const vec3d& b) {
		vec3d result(a);
		result *= b;
		return result;
	};
	friend vec3d	operator*(const vec3d& a, const double& b) {
		vec3d result(a);
		result *= b;
		return result;
	};
	friend vec3d	operator*(const double& a, const vec3d& b) {
		vec3d result(b);
		result *= a;
		return result;
	};

	friend vec3d	operator/(const vec3d& a, const vec3d& b) {
		vec3d result(a);
		result /= b;
		return result;
	};
	friend vec3d	operator/(const vec3d& a, const double& b) {
		vec3d result(a);
		result /= b;
		return result;
	};
	friend vec3d	operator/(const double& a, const vec3d& b) {
		vec3d result;
		result.x = 1 / b.x;
		result.y = 1 / b.y;
		result.z = 1 / b.z;

		result *= a;
		return result;
	};
	vec3d& operator=(const vec3d& a) {
		this->x = a.x;
		this->y = a.y;
		this->z = a.z;

		return *this;
	};
	vec3d& operator=(const double& a) {
		x = y = z = a;
		return *this;
	};

	bool			Compare(const vec3d& a) const {
		return ((x == a.x) && (y == a.y) && (z == a.z));
	};							// exact compare, no epsilon
	bool			operator==(const vec3d& a) const {
		return Compare(a);
	};						// exact compare, no epsilon
	bool			operator!=(const vec3d& a) const {
		return !Compare(a);
	};						// exact compare, no epsilon
	bool			operator<(const vec3d& a) const {
		return ((double)*this) < ((double)a);
	};
	bool			operator<=(const vec3d& a) const {
		return ((double)*this) <= ((double)a);
	};
	bool			operator>(const vec3d& a) const {
		return ((double)*this) > ((double)a);
	};
	bool			operator>=(const vec3d& a) const {
		return ((double)*this) >= ((double)a);
	};

	double			ScalarProjectionOnto(const vec3d& B) const {
		return (x * B.x + y * B.y) / ::sqrt(B.x * B.x + B.y * B.y);
	};

	vec2d			GetVec2() const {
		return vec2d(x, y);
	};

	double			Distance2d(const vec3d& a) const {
		double out(0);
		out += ((this->x - a.x) * (this->x - a.x));
		out += ((this->y - a.y) * (this->y - a.y));
		return ::sqrt(out);
	};

	double			Distance(const vec3d& a) const {
		double out(0);
		out += ::pow(this->x - a.x, 2);
		out += ::pow(this->y - a.y, 2);
		out += ::pow(this->z - a.z, 2);
		return ::sqrt(out);
	};

	void Normalize() {
		double sqrLength, invLength;

		sqrLength = x * x + y * y + z * z;
		invLength = sqrLength == 0 ? 1.0 : 1.0 / ::sqrt(sqrLength);
		x *= invLength;
		y *= invLength;
		z *= invLength;
	}

	void Normalize2D() {
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

	vec3d& FromString(const cweeStr& v) {
		int finder(0); cweeStr left; cweeStr right;
		finder = v.Find(',');
		if (finder != -1) {
			v.Mid(0, finder, left);
			v.Mid(finder + 1, v.Length(), right);
			x = (double)left;
		}
		finder = right.Find(',');
		if (finder != -1) {
			right.Mid(0, finder, left);
			right.Mid(finder + 1, right.Length(), right);
			y = (double)left;
			z = (double)right;
		}

		//cweeParser p(v, ",");
		//if (p.getNumVars() >= 3) {
		//	x = (double)p[0];
		//	y = (double)p[1];
		//	z = (double)p[2];
		//}
		//else {
		//	Zero();
		//}

		return *this;
	};

	operator		const char* () const {
		return ToString();
	};
	operator		const char* () {
		return ToString();
	};

};

//===============================================================
//	vec4 - 4D vector
//===============================================================
class vec4 {
public:
	float			x = 0;
	float			y = 0;
	float			z = 0;
	float			w = 0;
	vec4(void) {
	};
	explicit		vec4(const float x, const float y, const float z, const float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	};

	void 			Set(const float x, const float y, const float z, const float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	};
	void			Zero(void) {
		x = y = z = w = 0.0f;
	};
	float			operator[](const int index) const {
		return (&x)[index];
	};
	float& operator[](const int index) {
		return (&x)[index];
	};
	vec4			operator-() const {
		return vec4(-x, -y, -z, -w);
	};
	vec4& operator=(const vec4& a) {
		x = a.x;
		y = a.y;
		z = a.z;
		w = a.w;
		return *this;
	};
	float			operator*(const vec4& a) const {
		return x * a.x + y * a.y + z * a.z + w * a.w;
	};
	vec4			operator*(const float a) const {
		return vec4(x * a, y * a, z * a, w * a);
	};
	vec4			operator/(const float a) const {
		float inva = 1.0f / a;
		return vec4(x * inva, y * inva, z * inva, w * inva);
	};
	vec4			operator+(const vec4& a) const {
		return vec4(x + a.x, y + a.y, z + a.z, w + a.w);
	};
	vec4			operator-(const vec4& a) const {
		return vec4(x - a.x, y - a.y, z - a.z, w - a.w);
	};
	vec4& operator+=(const vec4& a) {
		x += a.x;
		y += a.y;
		z += a.z;
		w += a.w;
		return *this;
	};
	vec4& operator-=(const vec4& a) {
		x -= a.x;
		y -= a.y;
		z -= a.z;
		w -= a.w;
		return *this;
	};
	vec4& operator/=(const vec4& a) {
		x /= a.x;
		y /= a.y;
		z /= a.z;
		w /= a.w;
		return *this;
	};
	vec4& operator/=(const float a) {
		float inva = 1.0f / a;
		x *= inva;
		y *= inva;
		z *= inva;
		w *= inva;
		return *this;
	};
	vec4& operator*=(const float a) {
		x *= a;
		y *= a;
		z *= a;
		w *= a;
		return *this;
	};
	bool			Compare(const vec4& a) const {
		return ((x == a.x) && (y == a.y) && (z == a.z) && (w == a.w));
	};							// exact compare, no epsilon
	bool			operator==(const vec4& a) const {
		return Compare(a);
	};						// exact compare, no epsilon
	bool			operator!=(const vec4& a) const {
		return !Compare(a);
	};						// exact compare, no epsilon

	float			Distance(const vec4& a) {
		float out(0);
		out += pow(this->x - a.x, 2);
		out += pow(this->y - a.y, 2);
		out += pow(this->z - a.z, 2);
		out += pow(this->w - a.w, 2);
		return cweeMath::Sqrt(out);
	};

	cweeStr			ToString(void) const {
		return cweeStr::printf("%f,%f,%f,%f", x, y, z, w);
	};

	operator			const char* () const {
		return ToString();
	};
	operator			const char* () {
		return ToString();
	};

	cweeStr			Serialize(void) const {
		const cweeStr bufferText(":vec4:");
		cweeStr out;
		out.AddToDelimiter((float)x, bufferText);
		out.AddToDelimiter((float)y, bufferText);
		out.AddToDelimiter((float)z, bufferText);
		out.AddToDelimiter((float)w, bufferText);
		return out;
	};

	void			Deserialize(const cweeStr& source) {
		const cweeStr bufferText(":vec4:");
		cweeParser temp(source, bufferText, true);
		x = (float)temp[0];
		y = (float)temp[1];
		z = (float)temp[2];
		w = (float)temp[3];
	};
};

//===============================================================
//	vec5 - 5D vector
//===============================================================
class vec5 {
public:
	float			x = 0;
	float			y = 0;
	float			z = 0;
	float			w = 0;
	float			a = 0;
	vec5(void) {
	};
	explicit		vec5(const float x, const float y, const float z, const float w, const float a) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
		this->a = a;
	};

	void 			Set(const float x, const float y, const float z, const float w, const float a) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
		this->a = a;
	};
	void			Zero(void) {
		x = y = z = w = a = 0.0f;
	};
	float			operator[](const int index) const {
		return (&x)[index];
	};
	float& operator[](const int index) {
		return (&x)[index];
	};
	vec5			operator-() const {
		return vec5(-x, -y, -z, -w, -a);
	};
	vec5& operator=(const vec5& a) {
		x = a.x;
		y = a.y;
		z = a.z;
		w = a.w;
		this->a = a.a;
		return *this;
	};
	float			operator*(const vec5& a) const {
		return x * a.x + y * a.y + z * a.z + w * a.w + this->a * a.a;
	};
	vec5			operator*(const float a) const {
		return vec5(x * a, y * a, z * a, w * a, this->a * a);
	};
	vec5			operator/(const float a) const {
		float inva = 1.0f / a;
		return vec5(x * inva, y * inva, z * inva, w * inva, this->a * inva);
	};
	vec5			operator+(const vec5& a) const {
		return vec5(x + a.x, y + a.y, z + a.z, w + a.w, this->a + a.a);
	};
	vec5			operator-(const vec5& a) const {
		return vec5(x - a.x, y - a.y, z - a.z, w - a.w, this->a - a.a);
	};
	vec5& operator+=(const vec5& a) {
		x += a.x;
		y += a.y;
		z += a.z;
		w += a.w;
		this->a += a.a;
		return *this;
	};
	vec5& operator-=(const vec5& a) {
		x -= a.x;
		y -= a.y;
		z -= a.z;
		w -= a.w;
		this->a -= a.a;
		return *this;
	};
	vec5& operator/=(const vec5& a) {
		x /= a.x;
		y /= a.y;
		z /= a.z;
		w /= a.w;
		this->a /= a.a;
		return *this;
	};
	vec5& operator/=(const float a) {
		float inva = 1.0f / a;
		x *= inva;
		y *= inva;
		z *= inva;
		w *= inva;
		this->a *= inva;
		return *this;
	};
	vec5& operator*=(const float a) {
		x *= a;
		y *= a;
		z *= a;
		w *= a;
		this->a *= a;
		return *this;
	};
	bool			Compare(const vec5& a) const {
		return ((x == a.x) && (y == a.y) && (z == a.z) && (w == a.w) && (this->a == a.a));
	};							// exact compare, no epsilon
	bool			operator==(const vec5& a) const {
		return Compare(a);
	};						// exact compare, no epsilon
	bool			operator!=(const vec5& a) const {
		return !Compare(a);
	};						// exact compare, no epsilon

	float			Distance(const vec5& a) {
		float out(0);
		out += pow(this->x - a.x, 2);
		out += pow(this->y - a.y, 2);
		out += pow(this->z - a.z, 2);
		out += pow(this->w - a.w, 2);
		out += pow(this->a - a.a, 2);
		return cweeMath::Sqrt(out);
	};

	cweeStr			ToString(void) const {
		return cweeStr::printf("%f,%f,%f,%f,%f", x, y, z, w, a);
	};

	operator			const char* () const {
		return ToString();
	};
	operator			const char* () {
		return ToString();
	};

	cweeStr			Serialize(void) const {
		const cweeStr bufferText(":vec5:");
		cweeStr out;
		out.AddToDelimiter((float)x, bufferText);
		out.AddToDelimiter((float)y, bufferText);
		out.AddToDelimiter((float)z, bufferText);
		out.AddToDelimiter((float)w, bufferText);
		out.AddToDelimiter((float)a, bufferText);
		return out;
	};

	void			Deserialize(const cweeStr& source) {
		const cweeStr bufferText(":vec5:");
		cweeParser temp(source, bufferText, true);
		x = (float)temp[0];
		y = (float)temp[1];
		z = (float)temp[2];
		w = (float)temp[3];
		a = (float)temp[4];
	};
};

template < int size >
class vecN {
public:
	cweeThreadedList<float> list;
	vecN(void) {
		list.Resize(size);
	};
	explicit		vecN(const std::vector<float>& source) {
		for (int i = 0; i < size; i++)
			this->list[i] = source[i];
	};
	void 			Set(const std::vector<float>& source) {
		for (int i = 0; i < size; i++)
			this->list[i] = source[i];
	};
	void			Zero(void) {
		for (int i = 0; i < size; i++)
			this->list[i] = 0.0f;
	};
	float			operator[](const int index) const {
		return list[index];
	};
	float& operator[](const int index) {
		return list[index];
	};
	vecN			operator-() const {
		auto temp = vecN(list);
		for (auto& x : temp.list) x *= -1.0f;
		return temp;
	};
	vecN& operator=(const vecN& a) {
		list = a.list;
		return *this;
	};
	float			operator*(const vecN& a) const {
		float out = 0;
		for (int i = 0; i < size; i++)
			out += (list[i] * a.list[i]);

		return out;
	};
	vecN			operator*(const float a) const {
		auto temp = vecN(list);
		for (int i = 0; i < size; i++) temp.list[i] *= a;
		return temp;
	};
	vecN			operator/(const float a) const {
		float inva = 1.0f / a;
		return operator*(inva);
	};
	vecN			operator+(const vecN& a) const {
		auto temp = vecN(list);
		for (int i = 0; i < size; i++) temp.list[i] += a.list[i];
		return temp;
	};
	vecN			operator-(const vecN& a) const {
		auto temp = vecN(list);
		for (int i = 0; i < size; i++) temp.list[i] -= a.list[i];
		return temp;
	};
	vecN& operator+=(const vecN& a) {
		for (int i = 0; i < size; i++) list[i] += a.list[i];
		return *this;
	};
	vecN& operator-=(const vecN& a) {
		for (int i = 0; i < size; i++) list[i] -= a.list[i];
		return *this;
	};
	vecN& operator/=(const vecN& a) {
		for (int i = 0; i < size; i++) list[i] /= a.list[i];
		return *this;
	};
	vecN& operator/=(const float a) {
		float inva = 1.0f / a;
		for (int i = 0; i < size; i++) list[i] /= inva;
		return *this;
	};
	vecN& operator*=(const float a) {
		for (int i = 0; i < size; i++) list[i] *= a;
		return *this;
	};
	bool			Compare(const vecN& a) const {
		for (int i = 0; i < size; i++)
			if (list[i] != a.list[i])
				return false;
		return true;
	};							// exact compare, no epsilon
	bool			operator==(const vecN& a) const {
		return Compare(a);
	};						// exact compare, no epsilon
	bool			operator!=(const vecN& a) const {
		return !Compare(a);
	};						// exact compare, no epsilon

	float			Distance(const vecN& a) {
		float out(0);
		for (int i = 0; i < size; i++)
			out += pow(list[i] - a.list[i], 2);

		return cweeMath::Sqrt(out);
	};

	cweeStr			ToString(void) const {
		cweeStr out;
		for (int i = 0; i < size; i++) {
			if (i > 0) out += ',';
			out += list[i];
		}
		return out;
	};

	operator			const char* () const {
		return ToString();
	};
	operator			const char* () {
		return ToString();
	};

	cweeStr			Serialize(void) const {
		const cweeStr bufferText(":vecN:");
		cweeStr out;
		for (int i = 0; i < size; i++) {
			out.AddToDelimiter((float)list[i], bufferText);
		}
		return out;
	};

	void			Deserialize(const cweeStr& source) {
		const cweeStr bufferText(":vecN:");
		cweeParser temp(source, bufferText, true);
		for (int i = 0; i < size; i++) {
			list[i] = (float)temp[i];
		}
	};
};