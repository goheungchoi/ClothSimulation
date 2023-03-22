#include "vector.hpp"

template <size_t D, class T>
Vec<D, T>::Vec() { data[0] = T(0); data[1] = T(0); data[2] = T(0); }
template <size_t D, class T>
Vec<D, T>::Vec(T x_, T y_, T z_) { data[0] = x_; data[1] = y_; data[2] = z_; }

// Functions
template <size_t D, class T>
T Vec<D, T>::operator[](int i) const { return data[i]; }

template <size_t D, class T>
T& Vec<D, T>::operator[](int i) { return data[i]; }

template <size_t D, class T>
Vec<D, T>& Vec<D, T>::operator+=(const Vec<D, T>& x) {
	for (size_t i = 0; i < D; ++i) { data[i] += x[i]; }
	return *this;
}

template <size_t D, class T>
Vec<D, T>& Vec<D, T>::operator=(const Vec<D, T>& x) {
	// Guard self assignment
	if (this == &x)
		return *this;
	for (size_t i = 0; i < D; ++i) { data[i] = x[i]; }
	return *this;
}

template <size_t D, class T>
Vec<D, T> Vec<D, T>::operator-() {
	Vec<D, T> r;
	for (size_t i = 0; i < D; ++i) { r[i] = -data[i]; }
	return r;
}

template <size_t D, class T>
double Vec<D, T>::len2() const { return this->dot(*this); } // squared length

template <size_t D, class T>
double Vec<D, T>::len() const { return sqrt(len2()); } // length

template <size_t D, class T>
T Vec<D, T>::dot(const Vec<D, T>& v) const {
	T r(0);
	for (size_t i = 0; i < D; ++i) { r += v[i] * data[i]; }
	return r;
}

template <size_t D, class T>
Vec<3, T> Vec<D, T>::cross(const Vec<3, T>& v) const {
	assert(D == 3); // only defined for 3 dims
	return Vec<3, T>(data[1] * v[2] - data[2] * v[1], data[2] * v[0] - data[0] * v[2], data[0] * v[1] - data[1] * v[0]);
}

template <size_t D, class T>
void Vec<D, T>::normalize() {
	double l = len(); if (l <= 0.0) { return; }
	for (size_t i = 0; i < D; ++i) { data[i] = data[i] / l; }
}

