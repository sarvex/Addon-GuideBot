//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIDEBOT_MATH_H
#define _GUIDEBOT_MATH_H

#include <math.h>
#include <stdlib.h>
#include "guideBot/platform.h"

namespace GuideBot
{
////////////////////////////////////////////////////
////////***********Vector3***********///////////////
////////////////////////////////////////////////////
class Vector3
{
public:
	float x;
	float y;
	float z;

	Vector3();
	Vector3(const Vector3&);
	Vector3(float new_x, float new_y, float new_z);

	void set(float new_x, float new_y, float new_z);
	void set(const Vector3&);

	float   length()const;
	float   lengthSq() const;

	bool	isUnit() const;
	bool	isZero() const;

	void	normalize();
	void	normalizeSafe();

	bool operator==(const Vector3&) const;
	bool operator!=(const Vector3&) const;

	Vector3  operator+(const Vector3&) const;
	Vector3  operator-(const Vector3&) const;
	Vector3& operator+=(const Vector3&);
	Vector3& operator-=(const Vector3&);

	Vector3  operator*(float) const;
	Vector3  operator/(float) const;
	Vector3& operator*=(float);
	Vector3& operator/=(float);

	Vector3  operator*(const Vector3&) const;
	Vector3& operator*=(const Vector3&);
	Vector3  operator/(const Vector3&) const;
	Vector3& operator/=(const Vector3&);

	Vector3 operator-() const;

	Vector3& operator=(const Vector3&);

	const static Vector3 ZERO;
	const static Vector3 X;
	const static Vector3 Y;
	const static Vector3 Z;
	const static Vector3 ONE;
};

////////////////////////////////////////////////////
////////***********Matrix***********////////////////
////////////////////////////////////////////////////

class Matrix
{
private:
	float m[16];     

public:

	explicit Matrix(bool initialize = false);

	Matrix& identity();
    Matrix& inverse();
	float determinant();

	void getColumn(size_t col, Vector3 *vec) const;
	Vector3 getColumn(size_t col) const;

	void setColumn(size_t col, const Vector3& vec);

	Vector3 getPosition() const;
	void setPosition( const Vector3 &pos );

	Matrix&  mul(const Matrix &a);                    ///< M * a -> M
	Matrix&  mul(const Matrix &a, const Matrix &b);  ///< a * b -> M

	Matrix&  mul(const float a);                         ///< M * a -> M
	Matrix&  mul(const Matrix &a, const float b);       ///< a * b -> M

	void mulP( Vector3& p ) const;                      ///< M * p -> p (assume w = 1.0f)
	void mulP( const Vector3 &p, Vector3 *d) const;     ///< M * p -> d (assume w = 1.0f)
	void mulV( Vector3& p ) const;                      ///< M * v -> v (assume w = 0.0f)
	void mulV( const Vector3 &p, Vector3 *d) const;     ///< M * v -> d (assume w = 0.0f)

	Vector3 getRight() const;
	Vector3 getForward() const;   
	Vector3 getUp() const;

	friend Matrix operator * ( const Matrix &m1, const Matrix &m2 );
	Matrix& operator *= ( const Matrix &m );
	Matrix& operator=(const Matrix&);

	const static Matrix IDENTITY;
};

////////////////////////////////////////////////////
///////******Vector3 implementation********/////////
////////////////////////////////////////////////////

inline Vector3::Vector3()
{
}


inline Vector3::Vector3(const Vector3& vec)
:	x(vec.x) 
,	y(vec.y)
,	z(vec.z)
{
}

inline Vector3::Vector3(float new_x, float new_y, float new_z)
:	x(new_x)
,	y(new_y)
,	z(new_z)
{
}

inline void Vector3::set(float new_x, float new_y, float new_z)
{
	x = new_x;
	y = new_y;
	z = new_z;
}

inline void Vector3::set(const Vector3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
}

inline float Vector3::length() const
{
	return (float) sqrtf(x*x + y*y + z*z);
}

inline float Vector3::lengthSq() const
{
	return (x * x) + (y * y) + (z * z);
}

inline bool Vector3::isZero() const
{
	return lengthSq() < GB_EPSILON;
}

inline bool Vector3::isUnit() const
{
	return (fabs(1.0f-lengthSq()) < GB_EPSILON);
}

inline void Vector3::normalize()
{
	GB_ASSERT(!isZero(), "Error! Can't normalize zero vector");

	float length = sqrtf(x*x + y*y + z*z);
	GB_ASSERT(length != 0.0f, "Error, division by zero");
	float lengthInv = 1.f / length;
	x *= lengthInv;
	y *= lengthInv;
	z *= lengthInv;
}

inline void Vector3::normalizeSafe()
{
	if (isZero())
	{
		*this = ZERO;
	}
	else
		normalize();	
}

inline bool Vector3::operator==(const Vector3& vec) const
{
	return (x == vec.x) && (y == vec.y) && (z == vec.z);
}

inline bool Vector3::operator!=(const Vector3& vec) const
{
	return operator==(vec) == false;
}

inline Vector3 Vector3::operator+(const Vector3& vec) const
{
	return Vector3(x + vec.x, y + vec.y,  z + vec.z);
}

inline Vector3 Vector3::operator-(const Vector3& vec) const
{
	return Vector3(x - vec.x, y - vec.y, z - vec.z);
}

inline Vector3& Vector3::operator+=(const Vector3& vec)
{
	x += vec.x;
	y += vec.y;
	z += vec.z;

	return *this;
}

inline Vector3& Vector3::operator-=(const Vector3& vec)
{
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;

	return *this;
}

inline Vector3 Vector3::operator*(float vec) const
{
	return Vector3(x * vec, y * vec, z * vec);
}

inline Vector3 Vector3::operator/(float scalar) const
{
	GB_ASSERT(scalar != 0.0f, "Error, division by zero");

	float scalarInv = 1.0f / scalar;

	return Vector3(x * scalarInv, y * scalarInv, z * scalarInv);
}

inline Vector3& Vector3::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;

	return *this;
}

inline Vector3& Vector3::operator/=(float scalar)
{
	GB_ASSERT(scalar != 0.0f, "Error, division by zero");

	float scalarInv = 1.0f / scalar;
	x *= scalarInv;
	y *= scalarInv;
	z *= scalarInv;

	return *this;
}

inline Vector3 Vector3::operator*(const Vector3 &vec) const
{
	return Vector3(x * vec.x, y * vec.y, z * vec.z);
}

inline Vector3& Vector3::operator*=(const Vector3 &vec)
{
	x *= vec.x;
	y *= vec.y;
	z *= vec.z;
	return *this;
}

inline Vector3 Vector3::operator/(const Vector3 &vec) const
{
	return Vector3(x / vec.x, y / vec.y, z / vec.z);
}

inline Vector3& Vector3::operator/=(const Vector3 &vec)
{
	x /= vec.x;
	y /= vec.y;
	z /= vec.z;
	return *this;
}

inline Vector3 Vector3::operator-() const
{
	return Vector3(-x, -y, -z);
}

inline Vector3& Vector3::operator=(const Vector3& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return *this;	
}
////////////////////////////////////////////////////
///////******Matrix implementation********//////////
////////////////////////////////////////////////////

inline Matrix::Matrix(bool initialize /*= false*/)
{
	if (initialize)
		identity();
}

inline Matrix& Matrix::identity()
{
	m[0]  = 1.0f;
	m[1]  = 0.0f;
	m[2]  = 0.0f;
	m[3]  = 0.0f;
	m[4]  = 0.0f;
	m[5]  = 1.0f;
	m[6]  = 0.0f;
	m[7]  = 0.0f;
	m[8]  = 0.0f;
	m[9]  = 0.0f;
	m[10] = 1.0f;
	m[11] = 0.0f;
	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;
	return (*this);
}


inline Matrix& Matrix::inverse()
{
	float det = determinant();
	GB_ASSERT( det != 0.0f, "Matrix::inverse: can't invert matrix, determinant is zero.");

	float invDet = 1.0f/det;
	float temp[16];

	temp[0] = (m[5] * m[10]- m[6] * m[9]) * invDet;
	temp[1] = (m[9] * m[2] - m[10]* m[1]) * invDet;
	temp[2] = (m[1] * m[6] - m[2] * m[5]) * invDet;

	temp[4] = (m[6] * m[8] - m[4] * m[10])* invDet;
	temp[5] = (m[10]* m[0] - m[8] * m[2]) * invDet;
	temp[6] = (m[2] * m[4] - m[0] * m[6]) * invDet;

	temp[8] = (m[4] * m[9] - m[5] * m[8]) * invDet;
	temp[9] = (m[8] * m[1] - m[9] * m[0]) * invDet;
	temp[10]= (m[0] * m[5] - m[1] * m[4]) * invDet;

	m[0] = temp[0];
	m[1] = temp[1];
	m[2] = temp[2];

	m[4] = temp[4];
	m[5] = temp[5];
	m[6] = temp[6];

	m[8] = temp[8];
	m[9] = temp[9];
	m[10] = temp[10];

	temp[0] = -m[3];
	temp[1] = -m[7];
	temp[2] = -m[11];

	//m_matF_x_Vector3(m, temp, &temp[4]);
	temp[4] = m[0]*temp[0] + m[1]*temp[1] + m[2]*temp[2];
	temp[5] = m[4]*temp[0] + m[5]*temp[1] + m[6]*temp[2];
	temp[6] = m[8]*temp[0] + m[9]*temp[1] + m[10]*temp[2];

	m[3] = temp[4];
	m[7] = temp[5];
	m[11]= temp[6];
	return (*this);
}

inline float Matrix::determinant()
{
	return m[0] * (m[5] * m[10] - m[6] * m[9])  +
		m[4] * (m[2] * m[9]  - m[1] * m[10]) +
		m[8] * (m[1] * m[6]  - m[2] * m[5])  ;
}

inline void Matrix::getColumn(size_t col, Vector3 *vec) const
{
	vec->x = m[col];
	vec->y = m[col+4];
	vec->z = m[col+8];
}

inline Vector3 Matrix::getColumn(size_t col) const 
{ 
	Vector3 ret; 
	getColumn(col,&ret); 
	return ret; 
}

inline void Matrix::setColumn(size_t col, const Vector3 &vec)
{
	m[col]   = vec.x;
	m[col+4] = vec.y;
	m[col+8] = vec.z;
}

inline Vector3 Matrix::getPosition() const
{
	Vector3 pos;
	getColumn( 3, &pos );
	return pos;
}

inline void Matrix::setPosition( const Vector3 &pos )
{ 
	setColumn( 3, pos ); 
}

inline Matrix& Matrix::mul( const Matrix &mat )
{
	GB_ASSERT(&mat != this, "Matrix::mul - mat.mul(mat) is invalid!");

	Matrix temp(*this);

	return mul(temp,mat);
}

inline Matrix& Matrix::mul( const Matrix &a, const Matrix &b )
{
	GB_ASSERT((&a != this) && (&b != this), "Matrix::mul - a.mul(a, b) a.mul(b, a) a.mul(a, a) is invalid!");

	m[0] = a.m[0]*b.m[0] + a.m[1]*b.m[4] + a.m[2]*b.m[8]  + a.m[3]*b.m[12];
	m[1] = a.m[0]*b.m[1] + a.m[1]*b.m[5] + a.m[2]*b.m[9]  + a.m[3]*b.m[13];
	m[2] = a.m[0]*b.m[2] + a.m[1]*b.m[6] + a.m[2]*b.m[10] + a.m[3]*b.m[14];
	m[3] = a.m[0]*b.m[3] + a.m[1]*b.m[7] + a.m[2]*b.m[11] + a.m[3]*b.m[15];

	m[4] = a.m[4]*b.m[0] + a.m[5]*b.m[4] + a.m[6]*b.m[8]  + a.m[7]*b.m[12];
	m[5] = a.m[4]*b.m[1] + a.m[5]*b.m[5] + a.m[6]*b.m[9]  + a.m[7]*b.m[13];
	m[6] = a.m[4]*b.m[2] + a.m[5]*b.m[6] + a.m[6]*b.m[10] + a.m[7]*b.m[14];
	m[7] = a.m[4]*b.m[3] + a.m[5]*b.m[7] + a.m[6]*b.m[11] + a.m[7]*b.m[15];

	m[8] = a.m[8]*b.m[0] + a.m[9]*b.m[4] + a.m[10]*b.m[8] + a.m[11]*b.m[12];
	m[9] = a.m[8]*b.m[1] + a.m[9]*b.m[5] + a.m[10]*b.m[9] + a.m[11]*b.m[13];
	m[10]= a.m[8]*b.m[2] + a.m[9]*b.m[6] + a.m[10]*b.m[10]+ a.m[11]*b.m[14];
	m[11]= a.m[8]*b.m[3] + a.m[9]*b.m[7] + a.m[10]*b.m[11]+ a.m[11]*b.m[15];

	m[12]= a.m[12]*b.m[0]+ a.m[13]*b.m[4]+ a.m[14]*b.m[8] + a.m[15]*b.m[12];
	m[13]= a.m[12]*b.m[1]+ a.m[13]*b.m[5]+ a.m[14]*b.m[9] + a.m[15]*b.m[13];
	m[14]= a.m[12]*b.m[2]+ a.m[13]*b.m[6]+ a.m[14]*b.m[10]+ a.m[15]*b.m[14];
	m[15]= a.m[12]*b.m[3]+ a.m[13]*b.m[7]+ a.m[14]*b.m[11]+ a.m[15]*b.m[15];

	return (*this);
}


inline Matrix& Matrix::mul(const float a)
{
	for (size_t i = 0; i < 16; i++)
		m[i] *= a;

	return *this;
}


inline Matrix& Matrix::mul(const Matrix &a, const float b)
{
	*this = a;
	mul(b);

	return *this;
}

inline void Matrix::mulP( Vector3& p) const
{
	Vector3 temp;
	mulP(p,&temp);
	p = temp;
}

inline void Matrix::mulP( const Vector3 &p, Vector3 *d) const
{
	d->x = m[0]*p.x + m[1]*p.y + m[2]*p.z  + m[3];
	d->y = m[4]*p.x + m[5]*p.y + m[6]*p.z  + m[7];
	d->z = m[8]*p.x + m[9]*p.y + m[10]*p.z + m[11];
}

inline void Matrix::mulV( Vector3& v) const
{
	Vector3 temp;
	mulV(v,&temp);
	v = temp;
}

inline void Matrix::mulV( const Vector3 &v, Vector3 *d) const
{
	d->x = m[0]*v.x + m[1]*v.y + m[2]*v.z;
	d->y = m[4]*v.x + m[5]*v.y + m[6]*v.z;
	d->z = m[8]*v.x + m[9]*v.y + m[10]*v.z;
}

inline Vector3 Matrix::getForward() const
{
	return getColumn(1);
}

inline Vector3 Matrix::getRight() const
{
	return getColumn(0);
}

inline Vector3 Matrix::getUp() const
{
	return getColumn(2);
}

inline Matrix operator * ( const Matrix &m1, const Matrix &m2 )
{
	Matrix temp;
	temp.mul(m1,m2);
	return temp;
}

inline Matrix& Matrix::operator *= ( const Matrix &m )
{
	Matrix temp(*this);
	mul(temp,m);
	return (*this);
}

inline Matrix& Matrix::operator = ( const Matrix &other )
{
	for (size_t i = 0; i<16; i++)
	{
		m[i] = other.m[i];
	}
	return (*this);
}

////////////////////////////////////////////////////
///////*********Math functions************//////////
////////////////////////////////////////////////////
inline Vector3 operator*(float scalar, const Vector3& vector)
{
	return vector*scalar;
}

inline Vector3 crossProduct(const Vector3 &a, const Vector3 &b)
{
	return Vector3((a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x));
}

inline float dotProtuct(const Vector3 &a, const Vector3 &b)
{
	return (a.x*b.x + a.y*b.y + a.z*b.z);
}

inline float minValue(float value1, float value2)
{
	return value1 < value2 ? value1 : value2;
}

inline float maxValue(float value1, float value2)
{
	return value1 > value2 ? value1 : value2;
}

inline float clamp(float value, float low, float high)
{
	return (float) maxValue(minValue(value, high), low);
}

inline float lerp( float value1, float value2, float factor )
{
	return value1 + factor*(value2 - value1);
}


////////////////////////////////////////////////////
////////***********Randomizer*************//////////
////////////////////////////////////////////////////
class Randomizer
{
public:
	Randomizer();
	int getRand();
};
extern Randomizer gRandomizer;
inline int Randomizer::getRand()
{
	return rand();
}

inline int randI()
{
	//return random int from 0 to RAND_MAX
	return gRandomizer.getRand();
}

inline int randI(int a, int b)
{
	//return random int from a to b
	return (a + (randI() % (b - a + 1)) );
}

inline float randF()
{
	//return random float from 0 to 1
	return (float) gRandomizer.getRand() / RAND_MAX;
}

inline float randF(float a, float b)
{
	//return random float from a to b
	float rand = randF();
	return a + rand*(b-a);
}



}

#endif