#ifndef VECTOR_HPP
#define VECTOR_HPP 1

#define PI 3.14159265

#include <vector>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>

//
//	Vector Class
//	Not complete, only has functions needed for this sample.
//
template <size_t D, class T>
class Vec {
public:
	// Data
	T data[3];

	// Constructors
	Vec();
	Vec(T x_, T y_, T z_);

	// Functions
	T operator[](int i) const;
	T& operator[](int i);
	Vec<D, T>& operator+=(const Vec<D, T>& x);
	Vec<D, T>& operator=(const Vec<D, T>& x);
	Vec<D, T> operator-();

	double len2() const;// squared length
	double len() const;// length
	T dot(const Vec<D, T>& v) const;
	Vec<3, T> cross(const Vec<3, T>& v) const;
	void normalize();
};

typedef Vec<3, float> Vec3f;
typedef Vec<3, int> Vec3i;
typedef Vec<2, float> Vec2f;
typedef Vec<2, int> Vec2i;

template <size_t D, class T> static inline
const Vec<D, T> operator-(const Vec<D, T>& v1, const Vec<D, T>& v2) {
	Vec<D, T> r;
	for (size_t i = 0; i < D; ++i) { r[i] = v1[i] - v2[i]; }
	return r;
}

template <size_t D, class T> static inline
const Vec<D, T> operator*(const Vec<D, T>& v, const T& x) {
	Vec<D, T> r;
	for (size_t i = 0; i < D; ++i) { r[i] = v[i] * x; }
	return r;
}

static inline
void rotate(Vec3f& v, float xtheta, float ytheta, float ztheta)
{
	float xrad = xtheta * PI / 180.0;
	float yrad = ytheta * PI / 180.0;
	float zrad = ztheta * PI / 180.0;

	float temp1_1 = v[0] * cos(yrad) * cos(zrad);
	float temp1_2_1 = sin(xrad) * sin(yrad) * cos(zrad);
	float temp1_2_2 = cos(xrad) * sin(zrad);
	float temp1_2 = v[1] * (temp1_2_1 - temp1_2_2);
	float temp1_3_1 = cos(xrad) * sin(yrad) * cos(zrad);
	float temp1_3_2 = sin(xrad) * sin(zrad);
	float temp1_3 = v[2] * (temp1_3_1 + temp1_3_2);
	float temp1 = temp1_1 + temp1_2 + temp1_3;

	float temp2_1 = v[0] * cos(yrad) * sin(zrad);
	float temp2_2_1 = sin(xrad) * sin(yrad) * sin(zrad);
	float temp2_2_2 = cos(xrad) * cos(zrad);
	float temp2_2 = v[1] * (temp2_2_1 + temp2_2_2);
	float temp2_3_1 = cos(xrad) * sin(yrad) * sin(zrad);
	float temp2_3_2 = sin(xrad) * cos(zrad);
	float temp2_3 = v[2] * (temp2_3_1 - temp2_3_2);
	float temp2 = temp2_1 + temp2_2 + temp2_3;

	float temp3_1 = v[0] * (-sin(yrad));
	float temp3_2 = v[1] * sin(xrad) * cos(yrad);
	float temp3_3 = v[2] * cos(xrad) * cos(yrad);
	float temp3 = temp3_1 + temp3_2 + temp3_3;

	v[0] = temp1;
	v[1] = temp2;
	v[2] = temp3;
}

#endif 