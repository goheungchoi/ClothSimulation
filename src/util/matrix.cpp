#include "matrix.hpp"

Mat4x4::Mat4x4() { // Default: Identity
	m[0] = 1.f;  m[4] = 0.f;  m[8] = 0.f;  m[12] = 0.f;
	m[1] = 0.f;  m[5] = 1.f;  m[9] = 0.f;  m[13] = 0.f;
	m[2] = 0.f;  m[6] = 0.f;  m[10] = 1.f;  m[14] = 0.f;
	m[3] = 0.f;  m[7] = 0.f;  m[11] = 0.f;  m[15] = 1.f;
}

void Mat4x4::make_identity() {
	m[0] = 1.f;  m[4] = 0.f;  m[8] = 0.f;  m[12] = 0.f;
	m[1] = 0.f;  m[5] = 1.f;  m[9] = 0.f;  m[13] = 0.f;
	m[2] = 0.f;  m[6] = 0.f;  m[10] = 1.f;  m[14] = 0.f;
	m[3] = 0.f;  m[7] = 0.f;  m[11] = 0.f;  m[15] = 1.f;
}

void Mat4x4::print() {
	std::cout << m[0] << ' ' << m[4] << ' ' << m[8] << ' ' << m[12] << "\n";
	std::cout << m[1] << ' ' << m[5] << ' ' << m[9] << ' ' << m[13] << "\n";
	std::cout << m[2] << ' ' << m[6] << ' ' << m[10] << ' ' << m[14] << "\n";
	std::cout << m[3] << ' ' << m[7] << ' ' << m[11] << ' ' << m[15] << "\n";
}

void Mat4x4::make_scale(float x, float y, float z) {
	make_identity();
	m[0] = x; m[5] = y; m[10] = z;
}

Mat4x4 Mat4x4::operator*(const Mat4x4& dest) {
	Mat4x4 temp;
	int N = 4;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			float num = 0;
			for (int k = 0; k < N; k++) {
				num += m[i + k * 4] * dest.m[k + j * 4];
			}
			temp.m[i + j * 4] = num;
		}
	}
	return temp;
}
