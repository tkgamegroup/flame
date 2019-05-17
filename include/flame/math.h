// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <flame/type.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>

#undef min
#undef max

namespace flame
{
	const float RAD_ANG     = 180.f / M_PI;  // rad to angle
	const float ANG_RAD     = M_PI / 180.f;  // angle to rad

	const float EPS = 0.000001f;

	class F16
	{
		union Bits
		{
			float f;
			int32_t si;
			uint32_t ui;
		};

		static int const shift = 13;
		static int const shiftSign = 16;

		static int32_t const infN = 0x7F800000; // flt32 infinity
		static int32_t const maxN = 0x477FE000; // max flt16 normal as a flt32
		static int32_t const minN = 0x38800000; // min flt16 normal as a flt32
		static int32_t const signN = 0x80000000; // flt32 sign bit

		static int32_t const infC = infN >> shift;
		static int32_t const nanN = (infC + 1) << shift; // minimum flt16 nan as a flt32
		static int32_t const maxC = maxN >> shift;
		static int32_t const minC = minN >> shift;
		static int32_t const signC = signN >> shiftSign; // flt16 sign bit

		static int32_t const mulN = 0x52000000; // (1 << 23) / minN
		static int32_t const mulC = 0x33800000; // minN / (1 << (23 - shift))

		static int32_t const subC = 0x003FF; // max flt32 subnormal down shifted
		static int32_t const norC = 0x00400; // min flt32 normal down shifted

		static int32_t const maxD = infC - maxC - 1;
		static int32_t const minD = minC - subC - 1;

	public:

		static uint16_t compress(float value)
		{
			Bits v, s;
			v.f = value;
			uint32_t sign = v.si & signN;
			v.si ^= sign;
			sign >>= shiftSign; // logical shift
			s.si = mulN;
			s.si = s.f * v.f; // correct subnormals
			v.si ^= (s.si ^ v.si) & -(minN > v.si);
			v.si ^= (infN ^ v.si) & -((infN > v.si) & (v.si > maxN));
			v.si ^= (nanN ^ v.si) & -((nanN > v.si) & (v.si > infN));
			v.ui >>= shift; // logical shift
			v.si ^= ((v.si - maxD) ^ v.si) & -(v.si > maxC);
			v.si ^= ((v.si - minD) ^ v.si) & -(v.si > subC);
			return v.ui | sign;
		}

		static float decompress(uint16_t value)
		{
			Bits v;
			v.ui = value;
			int32_t sign = v.si & signC;
			v.si ^= sign;
			sign <<= shiftSign;
			v.si ^= ((v.si + minD) ^ v.si) & -(v.si > subC);
			v.si ^= ((v.si + maxD) ^ v.si) & -(v.si > maxC);
			Bits s;
			s.si = mulC;
			s.f *= v.si;
			int32_t mask = -(norC > v.si);
			v.si <<= shift;
			v.si ^= (s.si ^ v.si) & mask;
			v.si |= sign;
			return v.f;
		}
	};

	inline float linear_depth_ortho(float z, float depth_near, float depth_far)
	{
		z = z * 0.5 + 0.5;
		return z * (depth_far - depth_near) + depth_near;
	}

	inline float linear_depth_perspective(float z, float depth_near, float depth_far)
	{
		float a = (1.f - depth_far / depth_near) * 0.5f / depth_far;
		float b = (1.f + depth_far / depth_near) * 0.5f / depth_far;
		return 1.f / (a * z + b);
	}

	enum Axis
	{
		AxisPosX = 1 << 0,
		AxisNegX = 1 << 1,
		AxisPosY = 1 << 2,
		AxisNegY = 1 << 3,
		AxisPosZ = 1 << 4,
		AxisNegZ = 1 << 5
	};

	template<uint N, class T>
	struct Vec
	{
		T v_[N];

		Vec() 
		{
		}

		T& x()
		{
			static_assert(N > 0);
			return v_[0];
		}

		T& y()
		{
			static_assert(N > 1);
			return v_[1];
		}

		T& z()
		{
			static_assert(N > 2);
			return v_[2];
		}

		T& w()
		{
			static_assert(N > 3);
			return v_[3];
		}

		T& r()
		{
			static_assert(N > 0);
			return v_[0];
		}

		T& g()
		{
			static_assert(N > 1);
			return v_[1];
		}

		T& b()
		{
			static_assert(N > 2);
			return v_[2];
		}

		T& a()
		{
			static_assert(N > 3);
			return v_[3];
		}

		T& s()
		{
			static_assert(N > 0);
			return v_[0];
		}

		T& t()
		{
			static_assert(N > 1);
			return v_[1];
		}

		T& p()
		{
			static_assert(N > 2);
			return v_[2];
		}

		T& q()
		{
			static_assert(N > 3);
			return v_[3];
		}

		T operator[](uint i) const
		{
			return v_[i];
		}

		T& operator[](uint i)
		{
			return v_[i];
		}

		explicit Vec(T rhs)
		{
			for (auto i = 0; i < N; i++)
				v_[i] = v;
		}

		template<uint M, class U>
		explicit Vec(const Vec<M, U>& rhs)
		{
			static_assert(N <= M);
			for (auto i = 0; i < N; i++)
				v_[i] = rhs[i];
		}

		Vec(const T* rhs)
		{
			for (auto i = 0; i < N; i++)
				v_[i] = rhs[i];
		}

		Vec(T _x, T _y)
		{
			static_assert(N == 2);
			x() = _x;
			y() = _y;
		}

		Vec(T _x, T _y, T _z)
		{
			static_assert(N == 2);
			x() = _x;
			y() = _y;
			z() = _z;
		}

		template<class U>
		Vec(const Vec<2, U>& v1, T _z)
		{
			static_assert(N == 3);
			x() = v1.x();
			y() = v1.y();
			z() = _z;
		}

		template<class U>
		Vec(T _x, const Vec<2, U>& v1)
		{
			static_assert(N == 3);
			x() = _x;
			y() = v1.x();
			z() = v1.y();
		}

		Vec(T _x, T _y, T _z, T _w)
		{
			static_assert(N == 4);
			x() = _x;
			y() = _y;
			z() = _z;
			w() = _w;
		}

		template<class U>
		Vec(const Vec<2, U>& v1, T _z, T _w)
		{
			static_assert(N == 4);
			x() = v1.x();
			y() = v1.y();
			z() = _z;
			w() = _w;
		}

		template<class U>
		Vec(T _x, const Vec<2, U>& v1, T _w)
		{
			static_assert(N == 4);
			x() = _x;
			y() = v1.x();
			z() = v1.y();
			w() = _w;
		}

		template<class U>
		Vec(T _x, T _y, const Vec<2, U>& v1)
		{
			static_assert(N == 4);
			x() = _x;
			y() = _y;
			z() = v1.x();
			w() = v1.y();
		}

		template<class U>
		Vec(const Vec<2, U>& v1, const Vec<2, U>& v2)
		{
			static_assert(N == 4);
			x() = v1.x();
			y() = v1.y();
			z() = v2.x();
			w() = v2.y();
		}

		template<class U>
		Vec(const Vec<3, U>& v1, T _w)
		{
			static_assert(N == 4);
			x() = v1.x();
			y() = v1.y();
			z() = v1.z();
			w() = w;
		}

		template<class U>
		Vec(T _x, const Vec<3, U>& v1)
		{
			static_assert(N == 4);
			x() = _x;
			y() = v1.x();
			z() = v1.y();
			w() = v1.z();
		}

		Vec<N, T>& operator=(T rhs)
		{
			for (auto i = 0; i < N; i++)
				v_[i] = rhs;
			return *this;
		}

		template<uint M, class U>
		Vec<N, T>& operator=(const Vec<M, U>& rhs) 
		{
			static_assert(N <= M);
			for (auto i = 0; i < N; i++)
				v_[i] = rhs;
			return *this;
		}

		Vec<N, T> operator-()
		{
			Vec<N, T> ret;
			for (auto i = 0; i < N; i++)
				ret[i] = -v_[i];
			return ret;
		}

		Vec<N, T>& operator+=(T rhs)
		{
			for (auto i = 0; i < N; i++)
				v_[i] += rhs;
			return *this;
		}

		template<class U>
		Vec<N, T>& operator+=(const Vec<N, U>& rhs)
		{
			for (auto i = 0; i < N; i++)
				v_[i] += rhs[i];
			return *this;
		}

		Vec<N, T>& operator-=(T rhs)
		{
			for (auto i = 0; i < N; i++)
				v_[i] -= rhs;
			return *this;
		}

		template<class U>
		Vec<N, T>& operator-=(const Vec<N, U>& rhs)
		{
			for (auto i = 0; i < N; i++)
				v_[i] -= rhs[i];
			return *this;
		}

		Vec<N, T>& operator*=(T rhs)
		{
			for (auto i = 0; i < N; i++)
				v_[i] *= rhs;
			return *this;
		}

		template<class U>
		Vec<N, T>& operator*=(const Vec<N, U>& rhs)
		{
			for (auto i = 0; i < N; i++)
				v_[i] *= rhs[i];
			return *this;
		}

		Vec<N, T>& operator/=(T rhs)
		{
			rhs = 1 / rhs;
			for (auto i = 0; i < N; i++)
				v_[i] *= rhs;
			return *this;
		}

		template<class U>
		Vec<N, T>& operator/=(const Vec<N, U>& rhs)
		{
			for (auto i = 0; i < N; i++)
				v_[i] /= rhs[i];
			return *this;
		}
	};

	template<uint N, class T>
	Vec<N, T> operator+(T lhs, const Vec<N, T>& rhs)
	{
		Vec<N, T> ret(rhs);
		for (auto i = 0; i < N; i++)
			ret[i] += lhs;
		return ret;
	}

	template<uint N, class T>
	Vec<N, T> operator+(const Vec<N, T>& lhs, T rhs)
	{
		Vec<N, T> ret(lhs);
		for (auto i = 0; i < N; i++)
			ret[i] += rhs;
		return ret;
	}

	template<uint N, class T, class U>
	Vec<N, T> operator+(const Vec<N, T>& lhs, const Vec<N, U>& rhs)
	{
		Vec<N, T> ret(lhs);
		for (auto i = 0; i < N; i++)
			ret[i] += rhs[i];
		return ret;
	}

	template<uint N, class T>
	Vec<N, T> operator-(T lhs, const Vec<N, T>& rhs)
	{
		Vec<N, T> ret(rhs);
		for (auto i = 0; i < N; i++)
			ret[i] -= lhs;
		return ret;
	}

	template<uint N, class T>
	Vec<N, T> operator-(const Vec<N, T> & lhs, T rhs)
	{
		Vec<N, T> ret(lhs);
		for (auto i = 0; i < N; i++)
			ret[i] -= rhs;
		return ret;
	}

	template<uint N, class T, class U>
	Vec<N, T> operator-(const Vec<N, T> & lhs, const Vec<N, U> & rhs)
	{
		Vec<N, T> ret(lhs);
		for (auto i = 0; i < N; i++)
			ret[i] -= rhs[i];
		return ret;
	}

	template<uint N, class T>
	Vec<N, T> operator*(T lhs, const Vec<N, T>& rhs)
	{
		Vec<N, T> ret(rhs);
		for (auto i = 0; i < N; i++)
			ret[i] *= lhs;
		return ret;
	}

	template<uint N, class T>
	Vec<N, T> operator*(const Vec<N, T> & lhs, T rhs)
	{
		Vec<N, T> ret(lhs);
		for (auto i = 0; i < N; i++)
			ret[i] *= rhs;
		return ret;
	}

	template<uint N, class T, class U>
	Vec<N, T> operator*(const Vec<N, T> & lhs, const Vec<N, U> & rhs)
	{
		Vec<N, T> ret(lhs);
		for (auto i = 0; i < N; i++)
			ret[i] *= rhs[i];
		return ret;
	}

	template<uint N, class T>
	Vec<N, T> operator/(T lhs, const Vec<N, T>& rhs)
	{
		Vec<N, T> ret(rhs);
		for (auto i = 0; i < N; i++)
			ret[i] /= lhs;
		return ret;
	}

	template<uint N, class T>
	Vec<N, T> operator/(const Vec<N, T> & lhs, T rhs)
	{
		Vec<N, T> ret(lhs);
		for (auto i = 0; i < N; i++)
			ret[i] /= rhs;
		return ret;
	}

	template<uint N, class T, class U>
	Vec<N, T> operator/(const Vec<N, T> & lhs, const Vec<N, U> & rhs)
	{
		Vec<N, T> ret(lhs);
		for (auto i = 0; i < N; i++)
			ret[i] /= rhs[i];
		return ret;
	}

	using Vec3f = Vec<3, float>;

	template<uint N, uint M, class T>
	struct Mat
	{
		Vec<N, T> v_[M];

		T operator[](uint i) const
		{
			return v_[i];
		}

		T& operator[](uint i)
		{
			return v_[i];
		}

		Mat()
		{
		}

		explicit Mat(T rhs)
		{
			for (auto i = 0; i < M; i++)
				v_[i] = Vec<N, T>(0);
			auto n = min(N, M);
			for (auto i = 0; i < n; i++)
				v_[i][i] = v;
		}

		template<uint O, uint P, class U>
		explicit Mat(const Mat<O, P, U>& rhs)
		{
			static_assert(N <= O && M <= P);
			for (auto i = 0; i < M; i++)
			{
				for (auto j = 0; j < N; j++)
					v_[i][j] = rhs[i][j];
			}
		}

		Mat(const T * rhs)
		{
			for (auto i = 0; i < M; i++)
			{
				for (auto j = 0; j < N; j++)
					v_[i][j] = *rhs++;
			}
		}

		template<uint O, class U>
		Mat(const Vec<O, U>& _v1, const Vec<O, U>& _v2)
		{
			static_assert(N == O && M == 2);
			v_[0] = _v1;
			v_[1] = _v2;
		}

		template<uint O, class U>
		Mat(const Vec<O, U>& _v1, const Vec<O, U>& _v2, const Vec<O, U>& _v3)
		{
			static_assert(N == O && M == 3);
			v_[0] = _v1;
			v_[1] = _v2;
			v_[2] = _v3;
		}

		template<uint O, class U>
		Mat(const Mat<O, 2, U>& _m1, const Vec<O, U>& _v1)
		{
			static_assert(N == O && M == 3);
			v_[0] = _m1[0];
			v_[1] = _m1[1];
			v_[2] = _v1;
		}

		template<uint O, class U>
		Mat(const Vec<O, U>& _v1, const Mat<O, 2, U>& _m1)
		{
			static_assert(N == O && M == 3);
			v_[0] = _v1;
			v_[1] = _m1[0];
			v_[2] = _m1[1];
		}

		template<uint O, class U>
		Mat(const Vec<O, U>& _v1, const Vec<O, U>& _v2, const Vec<O, U>& _v3, const Vec<O, U>& _v4)
		{
			static_assert(N == O && M == 4);
			v_[0] = _v1;
			v_[1] = _v2;
			v_[2] = _v3;
			v_[3] = _v4;
		}

		template<uint O, class U>
		Mat(const Mat<O, 2, U>& _m1, const Vec<O, U>& _v1, const Vec<O, U>& _v2)
		{
			static_assert(N == O && M == 4);
			v_[0] = _m1[0];
			v_[1] = _m1[1];
			v_[2] = _v1;
			v_[3] = _v2;
		}

		template<uint O, class U>
		Mat(const Vec<O, U>& _v1, const Mat<O, 2, U>& _m1, const Vec<O, U>& _v2)
		{
			static_assert(N == O && M == 4);
			v_[0] = _v1;
			v_[1] = _m1[0];
			v_[2] = _m1[1];
			v_[3] = _v2;
		}

		template<uint O, class U>
		Mat(const Vec<O, U>& _v1, const Vec<O, U>& _v2, const Mat<O, 2, U>& _m1)
		{
			static_assert(N == O && M == 4);
			v_[0] = _v1;
			v_[1] = _v2;
			v_[2] = _m1[0];
			v_[3] = _m1[1];
		}

		template<uint O, class U>
		Mat(const Mat<O, 2, U>& _m1, const Mat<O, 2, U>& _m2)
		{
			static_assert(N == O && M == 4);
			v_[0] = _m1[0];
			v_[1] = _m1[1];
			v_[2] = _m1[0];
			v_[3] = _m1[1];
		}

		template<uint O, class U>
		Mat(const Mat<O, 3, U>& _m1, const Vec<O, U>& _v1)
		{
			static_assert(N == O && M == 4);
			v_[0] = _m1[0];
			v_[1] = _m1[1];
			v_[2] = _m1[2];
			v_[3] = _v1;
		}

		template<uint O, class U>
		Mat(const Vec<O, U>& _v1, const Mat<O, 3, U>& _m1)
		{
			static_assert(N == O && M == 4);
			v_[0] = _v1;
			v_[1] = _m1[0];
			v_[2] = _m1[1];
			v_[3] = _m1[2];
		}

		Mat<N, M, T>& operator=(T rhs)
		{
			for (auto i = 0; i < M; i++)
				v_[i] = Vec<N, T>(0);
			auto n = min(N, M);
			for (auto i = 0; i < n; i++)
				v_[i][i] = rhs;
			return *this;
		}

		template<uint O, uint P, class U>
		Mat<N, M, T>& operator=(const Mat<O, P, U> & rhs)
		{
			static_assert(N <= O && M <= P);
			for (auto i = 0; i < M; i++)
			{
				for (auto j = 0; j < N; j++)
					v_[i][j] = rhs[i][j];
			}
			return *this;
		}

		template<class U>
		Mat<N, M, T>& operator+=(const Mat<N, M, U>& rhs)
		{
			for (auto i = 0; i < M; i++)
				v_[i] += rhs[i];
			return *this;
		}

		template<class U>
		Mat<N, M, T>& operator-=(const Mat<N, M, U>& rhs)
		{
			for (auto i = 0; i < M; i++)
				v_[i] -= rhs[i];
			return *this;
		}

		Mat<N, M, T>& operator*=(T rhs)
		{
			for (auto i = 0; i < M; i++)
				v_[i] *= rhs;
			return *this;
		}

		Mat<N, M, T>& operator/=(T rhs)
		{
			for (auto i = 0; i < M; i++)
				v_[i] /= rhs;
			return *this;
		}
	};

	template<uint N, uint M, class T, class U>
	Mat<N, M, T> operator+(const Mat<N, M, T>& lhs, const Mat<N, M, U>& rhs)
	{
		Mat<N, M, T> ret(lhs);
		for (auto i = 0; i < M; i++)
			ret[i] += rhs[i];
		return ret;
	}

	template<uint N, uint M, class T, class U>
	Mat<N, M, T> operator-(const Mat<N, M, T>& lhs, const Mat<N, M, U>& rhs)
	{
		Mat<N, M, T> ret(lhs);
		for (auto i = 0; i < M; i++)
			ret[i] -= rhs[i];
		return ret;
	}

	template<uint N, uint M, class T>
	Mat<N, M, T> operator*(const Mat<N, M, T>& lhs, T rhs)
	{
		Mat<N, M, T> ret(lhs);
		for (auto i = 0; i < M; i++)
			ret[i] *= rhs;
		return ret;
	}

	template<uint N, uint M, class T>
	Mat<N, M, T> operator*(T lhs, const Mat<N, M, T>& rhs)
	{
		Mat<N, M, T> ret(rhs);
		for (auto i = 0; i < M; i++)
			ret[i] *= lhs;
		return ret;
	}

	template<uint N, uint M, uint O, class T>
	Mat<N, O, T> operator*(const Mat<N, M, T>& lhs, const Mat<M, O, T>& rhs)
	{
		Mat<N, O, T> ret;
		for (auto i = 0; i < O; i++)
		{
			ret[i][j] = 0;
			for (auto j = 0; j < N; j++)
				ret[i][j] += lhs[j][i] * rhs[i][j];
		}
		return ret;
	}

	template<uint N, uint M, class T>
	Vec<M, T> operator*(const Mat<N, M, T>& lhs, const Vec<M, T>& rhs)
	{
		Vec<M, T> ret;
		for (auto i = 0; i < M; i++)
		{
			ret[i] = 0;
			for (auto j = 0; j < N; j++)
				ret[i] += lhs[j][i] * rhs[i];
		}
		return ret;
	}

	template<uint N, uint M, class T>
	Mat<N, M, T> operator/(const Mat<N, M, T>& lhs, T rhs)
	{
		Mat<N, M, T> ret(lhs);
		for (auto i = 0; i < M; i++)
			ret[i] /= rhs;
		return ret;
	}

	template<uint N, uint M, class T>
	Mat<N, M, T> operator/(T lhs, const Mat<N, M, T>& rhs)
	{
		Mat<N, M, T> ret(rhs);
		for (auto i = 0; i < M; i++)
			ret[i] /= lhs;
		return ret;
	}

	template<class T>
	inline Vec<3, T> x_axis()
	{
		return Vec<3, T>(1.f, 0.f, 0.f);
	}

	template<class T>
	inline Vec<3, T> y_axis()
	{
		return Vec<3, T>(0.f, 1.f, 0.f);
	}

	template<class T>
	inline Vec<3, T> z_axis()
	{
		return Vec<3, T>(0.f, 0.f, 1.f);
	}

	template<class T>
	inline Vec<3, T> axis(int idx)
	{
		static Vec<3, T> axes[] = {
			x_axis(),
			y_axis(),
			z_axis()
		};
		return axes[idx];
	}

	template<uint N, class T>
	inline float length(const Vec<N, T>& v)
	{
		float s = 0.f;
		for (auto i = 0; i < N; i++)
			s += v[i] * v[i];
		return sqrt(s);
	}

	template<uint N, class T>
	Vec<N, T> normalize(const Vec<N, T>& v)
	{
		Vec<N, T> ret(v);
		ret /= length(v);
		return ret;
	}

	template<class T>
	inline T min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template<uint N, class T>
	inline Vec<N, T> min(const Vec<N, T>& a, const Vec<N, T>& b)
	{
		Vec<N, T> ret;
		for (auto i = 0; i < N; i++)
			ret[i] = min(a[i], b[i]);
		return ret;
	}

	template<class T>
	inline T max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}

	template<uint N, class T>
	inline Vec<N, T> max(const Vec<N, T>& a, const Vec<N, T>& b)
	{
		Vec<N, T> ret;
		for (auto i = 0; i < N; i++)
			ret[i] = max(a[i], b[i]);
		return ret;
	}

	template<class T>
	inline T clamp(const T& v, const T& a, const T& b)
	{
		if (v < a)
			return a;
		if (v > b)
			return b;
		return v;
	}

	template<uint N, class T>
	inline Vec<N, T> clamp(const Vec<N, T>& v, const Vec<N, T>& a, const Vec<N, T>& b)
	{
		Vec<N, T> ret;
		for (auto i = 0; i < N; i++)
			ret[i] = clamp(v[i], a[i], b[i]);
		return ret;
	}

	template<class T>
	inline T fract(T v)
	{
		return v - floor(v);
	}

	template<uint N, class T>
	inline Vec<N, T> fract(const Vec<N, T>& v)
	{
		Vec<N, T> ret;
		for (auto i = 0; i < N; i++)
			ret[i] = fract(v[i]);
		return ret;
	}

	template<class T>
	inline Vec<2, T> mod(T a, T b)
	{
		return Vec<2, T>(a / b, a % b);
	}

	template<class T>
	inline T mix(T v0, T v1, T q)
	{
		return v0 + q * (v1 - v0);
	}

	template<uint N, class T>
	inline Vec<N, T> mix(const Vec<N, T>& v0, const Vec<N, T>& v1, T q)
	{
		Vec<N, T> ret;
		for (auto i = 0; i < N; i++)
			ret[i] = mix(v0[i], v1[i], q);
		return ret;
	}
	
	template<uint N, class T>
	inline T dot(const Vec<N, T>& lhs, const Vec<N, T>& rhs)
	{
		T ret = 0;
		for (auto i = 0; i < N; i++)
			ret += lhs.v[i] * rhs.v[i];
		return ret;
	}

	template<class T>
	inline Vec<3, T> cross(const Vec<3, T>& lhs, const Vec<3, T>& rhs)
	{
		return Vec<3, T>(
			lhs.v[1] * rhs.v[2] - rhs.v[1] * lhs.v[2],
			lhs.v[2] * rhs.v[0] - rhs.v[2] * lhs.v[0],
			lhs.v[0] * rhs.v[1] - rhs.v[0] * lhs.v[1]);
	}

	template<uint N, class T>
	inline T distance(const Vec<N, T>& lhs, const Vec<N, T>& rhs)
	{
		auto d = lhs - rhs;
		return dot(d, d);
	}

	template<uint N, class T>
	inline Mat<N, N, T> transpose(const Mat<N, N, T>& m)
	{
		Mat<N, N, T> ret;
		for (auto i = 0; i < N; i++)
		{
			for (auto j = 0; j < N; j++)
				ret[i][j] = m[j][i];
		}
	}

	template<uint N, class T>
	inline Mat<N, N, T> inverse(const Mat<N, N, T>& m)
	{
		Mat<N, N, T> ret;

	}

	template<class T>
	inline Vec<2, T> rotate(const Vec<2, T>& p, const Vec<2, T>& c, float rad)
	{
		Vec<2, T> ret(p - c);

		auto sin_v = sin(rad);
		auto cos_v = cos(rad);

		auto xnew = ret.x * cos_v - ret.y * sin_v;
		auto ynew = ret.x * sin_v + ret.y * cos_v;

		ret.x = xnew + c.x;
		ret.y = ynew + c.y;

		return ret;
	}

	inline Mat<4, 4, float> view_mat(const Vec3f& eye, const Vec3f& center, const Vec3f& up)
	{

	}

	inline Mat<4, 4, float> proj_mat(float fovy, float aspect, float zNear, float zFar)
	{

	}

	Vec2 bezier(float t, const Vec2 &p0, const Vec2 &p1, const Vec2 &p2, const Vec2 &p3);
	float closet_to_bezier(int iters, const Vec2 &pos, float start, float end, 
		int slices, const Vec2 &p0, const Vec2 &p1, const Vec2 &p2, const Vec2 &p3);
	Vec2 closet_point_to_bezier(const Vec2 &pos, const Vec2 &p0, const Vec2 &p1, const Vec2 &p2, const Vec2 &p3,
		int slices, int iters);

	float rand(const Vec2 &v);
	float noise(const Vec2 &v);
	float fbm(const Vec2 &v);

	Rect get_fit_rect(const Vec2 &desired_size, float xy_aspect);
	Rect get_fit_rect_no_zoom_in(const Vec2 &desired_size, const Vec2 &size);

	Bvec4 Color(uchar R, uchar G, uchar B, uchar A = 255);
	Bvec4 Colorf(float R, float G, float B, float A = 1.f);
	Bvec4 HSV(float h, float s, float v, float A = 1.f);
	Vec3 to_HSV(const Bvec4 &rgb);

	struct Mat2
	{
		void inverse();
		Mat2 get_inversed() const;
	};

	struct Mat3
	{
		Mat3 &operator=(const EulerYPR &e);
		Mat3 &operator=(const Quat &q);
		void inverse();
		Mat3 get_inversed() const;
	};

	struct Mat4
	{
		void inverse();
		Mat4 get_inversed() const;
	};

	struct Quat
	{
		float x;
		float y;
		float z;
		float w;

		Quat();
		Quat(float _x, float _y, float _z, float _w);
		void normalize();
	};
	
	Quat operator*(const Quat &lhs, const Quat &rhs);

	struct Rect
	{
		Vec2 min;
		Vec2 max;

		enum Side
		{
			Outside = 0,
			SideN = 1 << 0,
			SideS = 1 << 1,
			SideW = 1 << 2,
			SideE = 1 << 3,
			SideNW = 1 << 4,
			SideNE = 1 << 5,
			SideSW = 1 << 6,
			SideSE = 1 << 7,
			Inside
		};

		Rect();
		Rect(const Vec2 &_min, const Vec2 &_max);
		Rect(float min_x, float min_y, float max_x, float max_y);
		Rect(const Rect &v);
		Rect &operator=(const Rect &v);
		Rect &operator+=(const Rect &v);
		Rect &operator-=(const Rect &v);
		Rect &operator+=(const Vec2 &v);
		Rect &operator-=(const Vec2 &v);
		Rect &operator*=(float v);
		float width() const;
		float height() const;
		Vec2 center() const;
		void expand(float length);
		Rect get_expanded(float length);
		bool contains(const Vec2 &p);
		bool overlapping(const Rect &oth);
		Side calc_side(const Vec2 &p, float threshold);

		static Rect b(const Vec2& base, const Vec2& ext);
	};

	Vec2 get_side_dir();

	Rect operator+(const Rect &lhs, const Rect &rhs);
	Rect operator-(const Rect &lhs, const Rect &rhs);
	Rect operator+(const Rect &r, const Vec2 &off);
	Rect operator-(const Rect &r, const Vec2 &off);
	Rect operator*(const Rect &r, float v);

	struct Plane 
	{
		Vec3 normal;
		float d;

		Plane();
		Plane(const Vec3 &n, float _d);
		Plane(const Vec3 &n, const Vec3 &p);
		float intersect(const Vec3 &origin, const Vec3 &dir);
	};

	struct AABB
	{
		Vec3 min;
		Vec3 max;

		AABB();
		AABB(const Vec3 &_min, const Vec3 &_max);
		AABB(const AABB &v);
		AABB &operator=(const AABB &v);
		void reset();
		float width() const;
		float height() const;
		float depth() const;
		Vec3 center() const;
		void merge(const Vec3 &p);
		void merge(const AABB &v);
		void get_points(Vec3 *dst);
	};

	AABB operator+(const AABB &a, const Vec3 &off);
	AABB operator-(const AABB &a, const Vec3 &off);

	namespace math_detail
	{
		void rotate(const Vec3 &axis, float rad, Mat3 &m);
		void mat3_to_quat(const Mat3 &m, Quat &q);
		void euler_to_mat3(const EulerYPR &e, Mat3 &m);
		void quat_to_euler(const Quat &q, EulerYPR &e);
		void quat_to_mat3(const Quat &q, Mat3 &m);
	}

	inline Vec2 bezier(float t, const Vec2 &p0, const Vec2 &p1, const Vec2 &p2, const Vec2 &p3)
	{
		return (1.f - t) * (1.f - t) * (1.f - t) * p0 +
			3.f * (1.f - t) * (1.f - t) * t * p1 +
			3.f * (1.f - t) * t * t * p2 +
			t * t * t * p3;
	}

	inline float bezier_closest(int iters, const Vec2 &pos, float start, float end,
		int slices, const Vec2 &p0, const Vec2 &p1, const Vec2 &p2, const Vec2 &p3) 
	{
		if (iters <= 0)
			return (start + end) / 2;

		float tick = (end - start) / float(slices);
		float best = 0;
		float bestDistance = 1000000.f;
		float t = start;

		while (t <= end)
		{
			auto v = bezier(t, p0, p1, p2, p3);
			auto d = v - pos;
			float currentDistance = d.x * d.x + d.y * d.y;
			if (currentDistance < bestDistance)
			{
				bestDistance = currentDistance;
				best = t;
			}
			t += tick;
		}

		return bezier_closest(iters - 1, pos, max(best - tick, 0.f), min(best + tick, 1.f), slices, p0, p1, p2, p3);
	}

	inline Vec2 bezier_closest_point(const Vec2 &pos, const Vec2 &p0, const Vec2 &p1, const Vec2 &p2, const Vec2 &p3,
		int slices, int iters)
	{
		return bezier(bezier_closest(iters, pos, 0.f, 1.f, slices, p0, p1, p2, p3), p0, p1, p2, p3);
	}

	inline float rand(const Vec2 &v)
	{
		return fract(cos(v.x * (12.9898) + v.y * (4.1414)) * 43758.5453);
	}

	inline float noise(const Vec2 &_v)
	{
		const auto SC = 250;

		auto v = _v / SC;
		Vec2 vf(fract(v.x), fract(v.y));
		Vec2 vi(floor(v.x), floor(v.y));

		auto r0 = rand(vi);
		auto r1 = rand(vi + Vec2(1.f, 0.f));
		auto r2 = rand(vi + Vec2(0.f, 1.f));
		auto r3 = rand(vi + Vec2(1.f, 1.f));

		auto vs = 3.f * vf * vf - 2.f * vf * vf * vf;

		return mix(
			mix(r0, r1, vs.x),
			mix(r2, r3, vs.x),
			vs.y);
	}

	inline float fbm(const Vec2 &_v)
	{
		auto v = _v;
		auto r = 0.f;

		auto a = 1.f / 3.f;
		for (auto i = 0; i < 4; i++)
		{
			r += noise(v) * a;
			a /= 3.f;
			v *= 3.f;
		}

		return r;
	}

	inline Rect get_fit_rect(const Vec2 &desired_size, float xy_aspect)
	{
		if (desired_size.x <= 0.f || desired_size.y <= 0.f)
			return Rect(0.f, 0.f, 1.f, 1.f);
		Rect ret;
		if (desired_size.x / desired_size.y > xy_aspect)
		{
			ret.max.x = xy_aspect * desired_size.y;
			ret.max.y = desired_size.y;
			ret.min.x = (desired_size.x - ret.max.x) * 0.5f;
			ret.min.y = 0;
			ret.max += ret.min;
		}
		else
		{
			ret.max.x = desired_size.x;
			ret.max.y = desired_size.x / xy_aspect;
			ret.min.x = 0;
			ret.min.y = (desired_size.y - ret.max.y) * 0.5f;
			ret.max += ret.min;
		}
		return ret;
	}

	inline Rect get_fit_rect_no_zoom_in(const Vec2 &desired_size, const Vec2 &size)
	{
		if (desired_size.x <= 0.f || desired_size.y <= 0.f)
			return Rect(0.f, 0.f, 1.f, 1.f);
		if (size.x <= desired_size.x && size.y <= desired_size.y)
		{
			Rect ret;
			ret.max.x = size.x;
			ret.max.y = size.y;
			ret.min.x = (desired_size.x - size.x) * 0.5f;
			ret.min.y = (desired_size.y - size.y) * 0.5f;
			ret.max += ret.min;
			return ret;
		}
		else
			return get_fit_rect(desired_size, size.x / size.y);
	}

	inline Hvec4::Hvec4(float _x, float _y, float _z, float _w)
	{
		x = to_f16(_x);
		y = to_f16(_y);
		z = to_f16(_z);
		w = to_f16(_w);
	}

	inline Bvec4 Color(uchar R, uchar G, uchar B, uchar A)
	{
		return Bvec4(R, G, B, A);
	}

	inline Bvec4 Colorf(float R, float G, float B, float A)
	{
		return Bvec4(uchar(R * 255.f), uchar(G * 255.f), uchar(B * 255.f), uchar(A * 255.f));
	}

	inline Bvec4 HSV(float h, float s, float v, float A)
	{
		auto hdiv60 = h / 60.f;
		auto hi = int(hdiv60) % 6;
		auto f = hdiv60 - hi;
		auto p = v * (1.f - s);
		float q, t;

		switch (hi)
		{
		case 0:
			t = v * (1.f - (1.f - f) * s);
			return Bvec4(v * 255.f, t * 255.f, p * 255.f, A * 255.f);
		case 1:
			q = v * (1.f - f * s);
			return Bvec4(q * 255.f, v * 255.f, p * 255.f, A * 255.f);
		case 2:
			t = v * (1.f - (1.f - f) * s);
			return Bvec4(p * 255.f, v * 255.f, t * 255.f, A * 255.f);
		case 3:
			q = v * (1.f - f * s);
			return Bvec4(p * 255.f, q * 255.f, v * 255.f, A * 255.f);
		case 4:
			t = v * (1.f - (1.f - f) * s);
			return Bvec4(t * 255.f, p * 255.f, v * 255.f, A * 255.f);
		case 5:
			q = v * (1.f - f * s);
			return Bvec4(v * 255.f, p * 255.f, q * 255.f, A * 255.f);
		default:
			return Bvec4(0.f);
		}
	}

	inline Vec3 to_HSV(const Bvec4 &rgb)
	{
		auto r = rgb.x / 255.f;
		auto g = rgb.y / 255.f;
		auto b = rgb.z / 255.f;

		auto cmax = max(r, max(g, b));
		auto cmin = min(r, min(g, b));
		auto delta = cmax - cmin;

		float h;
		
		if (delta == 0.f)
			h = 0.f;
		else if (cmax == r)
			h = 60.f * fmod((g - b) / delta + 0.f, 6.f);
		else if (cmax == g)
			h = 60.f * fmod((b - r) / delta + 2.f, 6.f);
		else if (cmax == b)
			h = 60.f * fmod((r - g) / delta + 4.f, 6.f);
		else
			h = 0.f;

		return Vec3(h, cmax == 0.f ? 0.f : delta / cmax, cmax);
	}

	inline Mat2 &Mat2::operator*=(const Mat2 &v)
	{
		return (*this = *this * v);
	}

	inline void Mat2::inverse()
	{
		*this = get_inversed();
	}

	inline Mat2 Mat2::get_inversed() const
	{
		auto det_inv = 1.f / (
			(*this)[0][0] * (*this)[1][1] - 
			(*this)[1][0] * (*this)[0][1]);

		Mat2 ret(
			+(*this)[1][1] * det_inv,
			-(*this)[0][1] * det_inv,
			-(*this)[1][0] * det_inv,
			+(*this)[0][0] * det_inv);

		return ret;
	}

	inline Mat3::Mat3(const EulerYPR &e)
	{
		math_detail::euler_to_mat3(e, *this);
	}

	inline Mat3::Mat3(const Vec3 &axis, float rad)
	{
		auto c = cos(rad);
		auto s = sin(rad);

		Vec3 temp((1.f - c) * axis);

		(*this)[0][0] = c + temp[0] * axis[0];
		(*this)[0][1] = temp[0] * axis[1] + s * axis[2];
		(*this)[0][2] = temp[0] * axis[2] - s * axis[1];

		(*this)[1][0] = temp[1] * axis[0] - s * axis[2];
		(*this)[1][1] = c + temp[1] * axis[1];
		(*this)[1][2] = temp[1] * axis[2] + s * axis[0];

		(*this)[2][0] = temp[2] * axis[0] + s * axis[1];
		(*this)[2][1] = temp[2] * axis[1] - s * axis[0];
		(*this)[2][2] = c + temp[2] * axis[2];
	}

	inline Mat3 &Mat3::operator*=(const Mat3 &v)
	{
		return (*this = *this * v);
	}

	inline void Mat3::inverse()
	{
		*this = get_inversed();
	}

	inline Mat3 Mat3::get_inversed() const
	{
		auto det_inv = 1.f / (
			+(*this)[0][0] * ((*this)[1][1] * (*this)[2][2] - (*this)[2][1] * (*this)[1][2])
			-(*this)[1][0] * ((*this)[0][1] * (*this)[2][2] - (*this)[2][1] * (*this)[0][2])
			+(*this)[2][0] * ((*this)[0][1] * (*this)[1][2] - (*this)[1][1] * (*this)[0][2]));

		Mat3 ret;;
		ret[0][0] = +((*this)[1][1] * (*this)[2][2] - (*this)[2][1] * (*this)[1][2]) * det_inv;
		ret[1][0] = -((*this)[1][0] * (*this)[2][2] - (*this)[2][0] * (*this)[1][2]) * det_inv;
		ret[2][0] = +((*this)[1][0] * (*this)[2][1] - (*this)[2][0] * (*this)[1][1]) * det_inv;
		ret[0][1] = -((*this)[0][1] * (*this)[2][2] - (*this)[2][1] * (*this)[0][2]) * det_inv;
		ret[1][1] = +((*this)[0][0] * (*this)[2][2] - (*this)[2][0] * (*this)[0][2]) * det_inv;
		ret[2][1] = -((*this)[0][0] * (*this)[2][1] - (*this)[2][0] * (*this)[0][1]) * det_inv;
		ret[0][2] = +((*this)[0][1] * (*this)[1][2] - (*this)[1][1] * (*this)[0][2]) * det_inv;
		ret[1][2] = -((*this)[0][0] * (*this)[1][2] - (*this)[1][0] * (*this)[0][2]) * det_inv;
		ret[2][2] = +((*this)[0][0] * (*this)[1][1] - (*this)[1][0] * (*this)[0][1]) * det_inv;

		return ret;
	}

	inline Mat4 &Mat4::operator*=(const Mat4 &v)
	{
		return (*this = *this * v);
	}

	inline void Mat4::inverse()
	{
		*this = get_inversed();
	}

	inline Mat4 Mat4::get_inversed() const
	{
		auto coef00 = (*this)[2][2] * (*this)[3][3] - (*this)[3][2] * (*this)[2][3];
		auto coef02 = (*this)[1][2] * (*this)[3][3] - (*this)[3][2] * (*this)[1][3];
		auto coef03 = (*this)[1][2] * (*this)[2][3] - (*this)[2][2] * (*this)[1][3];

		auto coef04 = (*this)[2][1] * (*this)[3][3] - (*this)[3][1] * (*this)[2][3];
		auto coef06 = (*this)[1][1] * (*this)[3][3] - (*this)[3][1] * (*this)[1][3];
		auto coef07 = (*this)[1][1] * (*this)[2][3] - (*this)[2][1] * (*this)[1][3];

		auto coef08 = (*this)[2][1] * (*this)[3][2] - (*this)[3][1] * (*this)[2][2];
		auto coef10 = (*this)[1][1] * (*this)[3][2] - (*this)[3][1] * (*this)[1][2];
		auto coef11 = (*this)[1][1] * (*this)[2][2] - (*this)[2][1] * (*this)[1][2];

		auto coef12 = (*this)[2][0] * (*this)[3][3] - (*this)[3][0] * (*this)[2][3];
		auto coef14 = (*this)[1][0] * (*this)[3][3] - (*this)[3][0] * (*this)[1][3];
		auto coef15 = (*this)[1][0] * (*this)[2][3] - (*this)[2][0] * (*this)[1][3];

		auto coef16 = (*this)[2][0] * (*this)[3][2] - (*this)[3][0] * (*this)[2][2];
		auto coef18 = (*this)[1][0] * (*this)[3][2] - (*this)[3][0] * (*this)[1][2];
		auto coef19 = (*this)[1][0] * (*this)[2][2] - (*this)[2][0] * (*this)[1][2];

		auto coef20 = (*this)[2][0] * (*this)[3][1] - (*this)[3][0] * (*this)[2][1];
		auto coef22 = (*this)[1][0] * (*this)[3][1] - (*this)[3][0] * (*this)[1][1];
		auto coef23 = (*this)[1][0] * (*this)[2][1] - (*this)[2][0] * (*this)[1][1];

		Vec4 fac0(coef00, coef00, coef02, coef03);
		Vec4 fac1(coef04, coef04, coef06, coef07);
		Vec4 fac2(coef08, coef08, coef10, coef11);
		Vec4 fac3(coef12, coef12, coef14, coef15);
		Vec4 fac4(coef16, coef16, coef18, coef19);
		Vec4 fac5(coef20, coef20, coef22, coef23);

		Vec4 vec0((*this)[1][0], (*this)[0][0], (*this)[0][0], (*this)[0][0]);
		Vec4 vec1((*this)[1][1], (*this)[0][1], (*this)[0][1], (*this)[0][1]);
		Vec4 vec2((*this)[1][2], (*this)[0][2], (*this)[0][2], (*this)[0][2]);
		Vec4 vec3((*this)[1][3], (*this)[0][3], (*this)[0][3], (*this)[0][3]);

		Vec4 inv0(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
		Vec4 inv1(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
		Vec4 inv2(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
		Vec4 inv3(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

		Vec4 signA(+1, -1, +1, -1);
		Vec4 signB(-1, +1, -1, +1);
		Mat4 inverse(inv0 * signA, inv1 * signB, inv2 * signA, inv3 * signB);

		Vec4 row0(inverse[0][0], inverse[1][0], inverse[2][0], inverse[3][0]);

		Vec4 dot0((*this)[0] * row0);
		auto dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

		auto det_inv = 1.f / dot1;

		return inverse * det_inv;
	}

	inline Mat4 get_view_mat(const Vec3 &eye, const Vec3 &target, const Vec3 &up)
	{
		auto f = target - eye;
		f.normalize();
		auto s = cross(f, up);
		s.normalize();
		auto u = cross(s, f);

		Mat4 ret(1.f);
		ret[0][0] = s.x;
		ret[1][0] = s.y;
		ret[2][0] = s.z;
		ret[0][1] = u.x;
		ret[1][1] = u.y;
		ret[2][1] = u.z;
		ret[0][2] = -f.x;
		ret[1][2] = -f.y;
		ret[2][2] = -f.z;
		ret[3][0] = -dot(s, eye);
		ret[3][1] = -dot(u, eye);
		ret[3][2] = dot(f, eye);
		return ret;
	}

	inline Mat4 get_proj_mat(float fovy, float aspect, float zNear, float zFar)
	{
		auto tanHalfFovy = tan(fovy / 2.f);

		Mat4 ret(0.f);
		ret[0][0] = 1.f / (aspect * tanHalfFovy);
		ret[1][1] = 1.f / (tanHalfFovy);
		ret[2][2] = zFar / (zNear - zFar);
		ret[2][3] = -1.f;
		ret[3][2] = -(zFar * zNear) / (zFar - zNear);
		return Mat4(1.f, 0.f, 0.f, 0.f,
			0.f, -1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f) * ret;
	}

	inline EulerYPR::EulerYPR()
	{
	}

	inline EulerYPR::EulerYPR(float y, float p, float r) :
		yaw(y), pitch(p), roll(r)
	{
	}

	inline EulerYPR::EulerYPR(const Quat &q)
	{
		math_detail::quat_to_euler(q, *this);
	}

	inline Quat::Quat()
	{
	}

	inline Quat::Quat(float _x, float _y, float _z, float _w) :
		x(_x),
		y(_y),
		z(_z),
		w(_w)
	{
	}

	inline void Quat::normalize()
	{
		auto l = sqrt(x * x + y * y + z * z + w * w);
		x /= l;
		y /= l;
		z /= l;
		w /= l;
	}

	inline Quat operator*(const Quat &lhs, const Quat &rhs)
	{
		Quat ret;
		ret.x = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y;
		ret.y = lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x;
		ret.z = lhs.w * rhs.z + lhs.x * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x;
		ret.w = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;
		return ret;
	}

	inline Rect::Rect()
	{
	}

	inline Rect::Rect(const Vec2 &_min, const Vec2 &_max) :
		min(_min),
		max(_max)
	{
	}

	inline Rect::Rect(float min_x, float min_y, float max_x, float max_y) :
		min(min_x, min_y),
		max(max_x, max_y)
	{
	}

	inline Rect::Rect(const Rect &v) :
		min(v.min),
		max(v.max)
	{
	}

	inline Rect &Rect::operator=(const Rect &v)
	{
		min = v.min;
		max = v.max;
		return *this;
	}

	inline Rect &Rect::operator+=(const Rect &v)
	{
		min += v.min;
		max += v.max;
		return *this;
	}

	inline Rect &Rect::operator-=(const Rect &v)
	{
		min -= v.min;
		max -= v.max;
		return *this;
	}
	
	inline Rect &Rect::operator+=(const Vec2 &v)
	{
		min += v;
		max += v;
		return *this;
	}

	inline Rect &Rect::operator-=(const Vec2 &v)
	{
		min -= v;
		max -= v;
		return *this;
	}

	inline Rect &Rect::operator*=(float v)
	{
		min *= v;
		max *= v;
		return *this;
	}

	inline float Rect::width() const
	{
		return max.x - min.x;
	}

	inline float Rect::height() const
	{
		return max.y - min.y;
	}

	inline Vec2 Rect::center() const
	{
		return (min + max) / 2;
	}

	inline void Rect::expand(float length)
	{
		min.x -= length;
		min.y -= length;
		max.x += length;
		max.y += length;
	}

	inline Rect Rect::get_expanded(float length)
	{
		Rect ret(*this);
		ret.expand(length);
		return ret;
	}

	inline bool Rect::contains(const Vec2 &p)
	{
		return p.x > min.x && p.x < max.x &&
			p.y > min.y && p.y < max.y;
	}

	inline bool Rect::overlapping(const Rect &oth)
	{
		return min.x <= oth.max.x && max.x >= oth.min.x &&
			   min.y <= oth.max.y && max.y >= oth.min.y;
	}

	inline Rect::Side Rect::calc_side(const Vec2 &p, float threshold)
	{
		if (p.x < max.x && p.x > max.x - threshold &&
			p.y > min.y && p.y < min.y + threshold)
			return SideNE;
		if (p.x > min.x && p.x < min.x + threshold &&
			p.y > min.y && p.y < min.y + threshold)
			return SideNW;
		if (p.x < max.x && p.x > max.x - threshold &&
			p.y < max.y && p.y > max.y - threshold)
			return SideSE;
		if (p.x > min.x && p.x < min.x + threshold &&
			p.y < max.y && p.y > max.y - threshold)
			return SideSW;
		if (p.y > min.y - threshold && p.y < min.y + threshold &&
			p.x > min.x && p.x < max.x)
			return SideN;
		if (p.y < max.y && p.y > max.y - threshold &&
			p.x > min.x && p.x < max.x)
			return SideS;
		if (p.x < max.x && p.x > max.x - threshold &&
			p.y > min.y && p.y < max.y)
			return SideE;
		if (p.x > min.x && p.x < min.x + threshold &&
			p.y > min.y && p.y < max.y)
			return SideW;
		if (contains(p))
			return Inside;
		return Outside;
	}

	inline Rect Rect::b(const Vec2& base, const Vec2& ext)
	{
		return Rect(base, base + ext);
	}

	inline Vec2 get_side_dir(Rect::Side s)
	{
		switch (s)
		{
		case Rect::SideN:
			return Vec2(0.f, -1.f);
		case Rect::SideS:
			return Vec2(0.f, 1.f);
		case Rect::SideW:
			return Vec2(-1.f, 0.f);
		case Rect::SideE:
			return Vec2(1.f, 0.f);
		case Rect::SideNW:
			return Vec2(-1.f, -1.f);
		case Rect::SideNE:
			return Vec2(1.f, -1.f);
		case Rect::SideSW:
			return Vec2(-1.f, 1.f);
		case Rect::SideSE:
			return Vec2(1.f, 1.f);
		case Rect::Outside:
			return Vec2(2.f);
		case Rect::Inside:
			return Vec2(0.f);
		}
	}

	inline Rect operator+(const Rect &lhs, const Rect &rhs)
	{
		Rect ret(lhs);
		ret += rhs;
		return ret;
	}

	inline Rect operator-(const Rect &lhs, const Rect &rhs)
	{
		Rect ret(lhs);
		ret -= rhs;
		return ret;
	}

	inline Rect operator+(const Rect &r, const Vec2 &off)
	{
		Rect ret(r);
		ret += off;
		return ret;
	}

	inline Rect operator-(const Rect &r, const Vec2 &off)
	{
		Rect ret(r);
		ret -= off;
		return ret;
	}

	inline Rect operator*(const Rect &r, float v)
	{
		Rect ret(r);
		ret *= v;
		return ret;
	}

	inline Plane::Plane()
	{
	}

	inline Plane::Plane(const Vec3 &n, float _d) :
		normal(n),
		d(_d)
	{
	}

	inline Plane::Plane(const Vec3 &n, const Vec3 &p) :
		normal(n),
		d(dot(normal, p))
	{
	}

	inline float Plane::intersect(const Vec3 &origin, const Vec3 &dir)
	{
		auto numer = dot(normal, origin) - d;
		auto denom = dot(normal, dir);

		if (abs(denom) < EPS)
			return -1.f;

		return -(numer / denom);
	}

	inline AABB::AABB()
	{
	}

	inline AABB::AABB(const Vec3 &_min, const Vec3 &_max) :
		min(_min),
		max(_max)
	{
	}

	inline AABB::AABB(const AABB &v) :
		min(v.min),
		max(v.max)
	{
	}

	inline AABB &AABB::operator=(const AABB &v)
	{
		min = v.min;
		max = v.max;
		return *this;
	}

	inline void AABB::reset()
	{
		min = Vec3(0.f);
		max = Vec3(0.f);
	}

	inline float AABB::width() const
	{
		return max.x - min.x;
	}

	inline float AABB::height() const
	{
		return max.y - min.y;
	}

	inline float AABB::depth() const
	{
		return max.z - min.z;
	}

	inline Vec3 AABB::center() const
	{
		return (min + max) / 2.f;
	}

	inline void AABB::merge(const Vec3 &p)
	{
		min.x = ::flame::min(min.x, p.x);
		min.y = ::flame::min(min.y, p.y);
		min.z = ::flame::min(min.z, p.z);
		max.x = ::flame::max(max.x, p.x);
		max.y = ::flame::max(max.y, p.y);
		max.z = ::flame::max(max.z, p.z);
	}

	inline void AABB::merge(const AABB &v)
	{
		merge(v.min);
		merge(v.max);
	}

	inline void AABB::get_points(Vec3 *dst)
	{
		dst[0] = min;
		dst[1] = Vec3(max.x, min.y, min.z);
		dst[2] = Vec3(max.x, min.y, max.z);
		dst[3] = Vec3(min.x, min.y, max.z);
		dst[4] = Vec3(min.x, max.y, min.z);
		dst[5] = Vec3(max.x, max.y, min.z);
		dst[6] = max;
		dst[7] = Vec3(min.x, max.y, max.z);
	}

	inline AABB operator+(const AABB &a, const Vec3 &off)
	{
		AABB ret(a);
		ret.min += off;
		ret.max += off;
		return ret;
	}

	inline AABB operator-(const AABB &a, const Vec3 &off)
	{
		AABB ret(a);
		ret.min -= off;
		ret.max -= off;
		return ret;
	}

	namespace math_detail
	{
		inline void rotate(const Vec3 &axis, float rad, Mat3 &m)
		{
			const auto c = cos(rad);
			const auto s = sin(rad);

			Vec3 temp((1.f - c) * axis);

			m[0][0] = c + temp[0] * axis[0];
			m[0][1] = 0 + temp[0] * axis[1] + s * axis[2];
			m[0][2] = 0 + temp[0] * axis[2] - s * axis[1];

			m[1][0] = 0 + temp[1] * axis[0] - s * axis[2];
			m[1][1] = c + temp[1] * axis[1];
			m[1][2] = 0 + temp[1] * axis[2] + s * axis[0];

			m[2][0] = 0 + temp[2] * axis[0] + s * axis[1];
			m[2][1] = 0 + temp[2] * axis[1] - s * axis[0];
			m[2][2] = c + temp[2] * axis[2];
		}

		inline void mat3_to_quat(const Mat3 &m, Quat &q)
		{
			float s;
			float tq[4];
			int   i, j;
			// Use tq to store the largest trace
			tq[0] = 1.f + m[0][0] + m[1][1] + m[2][2];
			tq[1] = 1.f + m[0][0] - m[1][1] - m[2][2];
			tq[2] = 1.f - m[0][0] + m[1][1] - m[2][2];
			tq[3] = 1.f - m[0][0] - m[1][1] + m[2][2];
			// Find the maximum (could also use stacked if's later)
			j = 0;
			for (i = 1; i < 4; i++)
			{
				j = (tq[i] > tq[j]) ? i : j;
			}

			// check the diagonal
			if (j == 0)
			{
				/* perform instant calculation */
				q.w = tq[0];
				q.x = m[1][2] - m[2][1];
				q.y = m[2][0] - m[0][2];
				q.z = m[0][1] - m[1][0];
			}
			else if (j == 1)
			{
				q.w = m[1][2] - m[2][1];
				q.x = tq[1];
				q.y = m[0][1] + m[1][0];
				q.z = m[2][0] + m[0][2];
			}
			else if (j == 2)
			{
				q.w = m[2][0] - m[0][2];
				q.x = m[0][1] + m[1][0];
				q.y = tq[2];
				q.z = m[1][2] + m[2][1];
			}
			else /* if (j==3) */
			{
				q.w = m[0][1] - m[1][0];
				q.x = m[2][0] + m[0][2];
				q.y = m[1][2] + m[2][1];
				q.z = tq[3];
			}
			s = sqrt(0.25f / tq[j]);
			q.w *= s;
			q.x *= s;
			q.y *= s;
			q.z *= s;
			q.normalize();
		}

		inline void euler_to_mat3(const EulerYPR &e, Mat3 &m)
		{
			m[0] = Vec3(1.f, 0.f, 0.f);
			m[1] = Vec3(0.f, 1.f, 0.f);
			m[2] = Vec3(0.f, 0.f, 1.f);
			Mat3 mat_yaw(Vec3(m[1]), e.yaw * ANG_RAD);
			m[0] = mat_yaw * m[0];
			m[2] = mat_yaw * m[2];
			Mat3 mat_pitch(Vec3(m[0]), e.pitch * ANG_RAD);
			m[2] = mat_pitch * m[2];
			m[1] = mat_pitch * m[1];
			Mat3 mat_roll(Vec3(m[2]), e.roll * ANG_RAD);
			m[1] = mat_roll * m[1];
			m[0] = mat_roll * m[0];
		}

		inline void quat_to_euler(const Quat &q, EulerYPR &e)
		{
			auto sqw = q.w * q.w;
			auto sqx = q.x * q.x;
			auto sqy = q.y * q.y;
			auto sqz = q.z * q.z;

			auto unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
			auto test = q.x * q.y + q.z * q.w;
			if (test > 0.499f * unit)
			{ // singularity at north pole
				e.yaw = 2.f * atan2(q.x, q.w);
				e.pitch = M_PI / 2.f;
				e.roll = 0;
				return;
			}
			if (test < -0.499f * unit)
			{ // singularity at south pole
				e.yaw = -2.f * atan2(q.x, q.w);
				e.pitch = -M_PI / 2.f;
				e.roll = 0;
				return;
			}

			e.yaw = atan2(2.f * q.y * q.w - 2.f * q.x * q.z, sqx - sqy - sqz + sqw) * RAD_ANG;
			e.pitch = asin(2.f * test / unit) * RAD_ANG;
			e.roll = atan2(2.f * q.x * q.w - 2.f * q.y * q.z, -sqx + sqy - sqz + sqw) * RAD_ANG;
		}

		inline void quat_to_mat3(const Quat &q, Mat3 &m)
		{
			float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

			x2 = 2.f * q.x;
			y2 = 2.f * q.y;
			z2 = 2.f * q.z;

			xx = q.x * x2;
			xy = q.x * y2;
			xz = q.x * z2;

			yy = q.y * y2;
			yz = q.y * z2;
			zz = q.z * z2;

			wx = q.w * x2;
			wy = q.w * y2;
			wz = q.w * z2;

			m[0][0] = 1.f - (yy + zz);
			m[1][0] = xy - wz;
			m[2][0] = xz + wy;

			m[0][1] = xy + wz;
			m[1][1] = 1.f - (xx + zz);
			m[2][1] = yz - wx;

			m[0][2] = xz - wy;
			m[1][2] = yz + wx;
			m[2][2] = 1.f - (xx + yy);
		}
	}

	typedef char TypeFmt[4];

	inline void typefmt_assign(TypeFmt &lhs, const char *rhs)
	{
		for (auto i = 0; i < 4; i++)
			lhs[i] = rhs[i];
	}

	inline bool typefmt_compare(TypeFmt &lhs, const char *rhs)
	{
		for (auto i = 0;; i++)
		{
			if (lhs[i] != rhs[i])
				return false;
			if (lhs[i] == 0)
				break;
		}
		return true;
	}

	/* fmt:
		i1   - int
		i2   - Ivec2
		i3   - Ivec3
		i4   - Ivec4
		f1   - float
		f2   - Vec2
		f3   - Vec3
		f4   - Vec4
		b1   - uchar
		b2   - Bvec2
		b3   - Bvec3
		b4   - Bvec4
		p    - void*
	*/

	struct CommonData
	{
		TypeFmt fmt;

		union
		{
			uint u;
			Vec4 f;
			Ivec4 i;
			Bvec4 b;
			void *p;
		}v;

		inline uint &u()
		{
			return v.u;
		}

		inline float &f1()
		{
			return v.f[0];
		}

		inline Vec2 &f2()
		{
			return *(Vec2*)&v.f;
		}

		inline Vec3 &f3()
		{
			return *(Vec3*)&v.f;
		}

		inline Vec4 &f4()
		{
			return v.f;
		}

		inline int &i1()
		{
			return v.i[0];
		}

		inline Ivec2 &i2()
		{
			return *(Ivec2*)&v.i;
		}

		inline Ivec3 &i3()
		{
			return *(Ivec3*)&v.i;
		}

		inline Ivec4 &i4()
		{
			return v.i;
		}

		inline uchar &b1()
		{
			return v.b[0];
		}

		inline Bvec2 &b2()
		{
			return *(Bvec2*)&v.b;
		}

		inline Bvec3 &b3()
		{
			return *(Bvec3*)&v.b;
		}

		inline Bvec4 &b4()
		{
			return v.b;
		}

		inline void *&p()
		{
			return v.p;
		}

		CommonData() = default;

		inline CommonData(const CommonData &rhs)
		{
			for (auto i = 0; i < 4; i++)
				fmt[i] = rhs.fmt[i];
			memcpy(&v, &rhs.v, sizeof(v));
		}

		inline CommonData(float f)
		{
			fmt[0] = 'f';
			fmt[1] = '1';
			fmt[2] = 0;
			fmt[3] = 0;

			v.f = f;
		}

		inline CommonData(const Vec2 &f)
		{
			fmt[0] = 'f';
			fmt[1] = '2';
			fmt[2] = 0;
			fmt[3] = 0;

			v.f = f;
		}

		inline CommonData(const Vec3 &f)
		{
			fmt[0] = 'f';
			fmt[1] = '3';
			fmt[2] = 0;
			fmt[3] = 0;

			v.f = f;
		}

		inline CommonData(const Vec4 &f)
		{
			fmt[0] = 'f';
			fmt[1] = '4';
			fmt[2] = 0;
			fmt[3] = 0;

			v.f = f;
		}

		inline CommonData(int i)
		{
			fmt[0] = 'i';
			fmt[1] = '1';
			fmt[2] = 0;
			fmt[3] = 0;

			v.i = i;
		}

		inline CommonData(const Ivec2 &i)
		{
			fmt[0] = 'i';
			fmt[1] = '2';
			fmt[2] = 0;
			fmt[3] = 0;

			v.i = i;
		}

		inline CommonData(const Ivec3 &i)
		{
			fmt[0] = 'i';
			fmt[1] = '3';
			fmt[2] = 0;
			fmt[3] = 0;

			v.i = i;
		}

		inline CommonData(const Ivec4 &i)
		{
			fmt[0] = 'i';
			fmt[1] = '4';
			fmt[2] = 0;
			fmt[3] = 0;

			v.i = i;
		}

		inline CommonData(uchar b)
		{
			fmt[0] = 'b';
			fmt[1] = '1';
			fmt[2] = 0;
			fmt[3] = 0;

			v.b = b;
		}

		inline CommonData(const Bvec2 &b)
		{
			fmt[0] = 'b';
			fmt[1] = '2';
			fmt[2] = 0;
			fmt[3] = 0;

			v.b = b;
		}

		inline CommonData(const Bvec3 &b)
		{
			fmt[0] = 'b';
			fmt[1] = '3';
			fmt[2] = 0;
			fmt[3] = 0;

			v.b = b;
		}

		inline CommonData(const Bvec4 &b)
		{
			fmt[0] = 'b';
			fmt[1] = '4';
			fmt[2] = 0;
			fmt[3] = 0;

			v.b = b;
		}

		inline CommonData(void *p)
		{
			fmt[0] = 'p';
			fmt[1] = 0;
			fmt[2] = 0;
			fmt[3] = 0;

			v.p = p;
		}

		inline CommonData &operator=(const CommonData &rhs)
		{
			typefmt_assign(fmt, rhs.fmt);
			memcpy(&v, &rhs.v, sizeof(v));
			return *this;
		}
	};
}

