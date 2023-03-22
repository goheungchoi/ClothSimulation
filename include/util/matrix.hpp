#ifndef MATRIX_HPP
#define MATRIX_HPP 1

#include "vector.hpp"
#include <iostream>

class Mat4x4 {
public:
	float m[16];

	Mat4x4();
	void make_identity();
	void print();
	void make_scale(float x, float y, float z);
	Mat4x4 operator*(const Mat4x4& dest);
};

static inline const Vec3f operator*(const Mat4x4& m, const Vec3f& v) {
	Vec3f r(m.m[0] * v[0] + m.m[4] * v[1] + m.m[8] * v[2] + m.m[12] * 1.f,
			m.m[1] * v[0] + m.m[5] * v[1] + m.m[9] * v[2] + m.m[13] * 1.f,
			m.m[2] * v[0] + m.m[6] * v[1] + m.m[10] * v[2] + m.m[14] * 1.f);
	return r;
}

#endif