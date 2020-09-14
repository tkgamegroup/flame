#pragma once

#include <flame/type.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

#undef min
#undef max

namespace flame
{
	constexpr float RAD_ANG     = 180.f / M_PI;  // rad to angle
	constexpr float ANG_RAD     = M_PI / 180.f;  // angle to rad

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

	inline uint image_pitch(uint b)
	{
		return (uint)ceil((b / 4.f)) * 4U;
	}

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

	enum AxisFlags
	{
		AxisPosX = 1 << 0,
		AxisNegX = 1 << 1,
		AxisPosY = 1 << 2,
		AxisNegY = 1 << 3,
		AxisPosZ = 1 << 4,
		AxisNegZ = 1 << 5
	};

	inline AxisFlags operator| (AxisFlags a, AxisFlags b) { return (AxisFlags)((int)a | (int)b); }

	enum SideFlags
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
		SideCenter = 1 << 8,
		Inside
	};

	inline SideFlags operator| (SideFlags a, SideFlags b) { return (SideFlags)((int)a | (int)b); }

	template <uint N, class T>
	struct Vec
	{
		T v_[N];

		Vec() = default;

		static const char* coord_name(uint idx)
		{
			const char* names[] = {
				"x",
				"y",
				"z",
				"w"
			};
			return names[idx];
		}

		T& x()
		{
			static_assert(N > 0);
			return v_[0];
		}

		T x() const
		{
			static_assert(N > 0);
			return v_[0];
		}

		T& y()
		{
			static_assert(N > 1);
			return v_[1];
		}

		T y() const
		{
			static_assert(N > 1);
			return v_[1];
		}

		T& z()
		{
			static_assert(N > 2);
			return v_[2];
		}

		T z() const
		{
			static_assert(N > 2);
			return v_[2];
		}

		T& w()
		{
			static_assert(N > 3);
			return v_[3];
		}

		T w() const
		{
			static_assert(N > 3);
			return v_[3];
		}

		T& r()
		{
			static_assert(N > 0);
			return v_[0];
		}

		T r() const
		{
			static_assert(N > 0);
			return v_[0];
		}

		T& g()
		{
			static_assert(N > 1);
			return v_[1];
		}

		T g() const
		{
			static_assert(N > 1);
			return v_[1];
		}

		T& b()
		{
			static_assert(N > 2);
			return v_[2];
		}

		T b() const
		{
			static_assert(N > 2);
			return v_[2];
		}

		T& a()
		{
			static_assert(N > 3);
			return v_[3];
		}

		T a() const
		{
			static_assert(N > 3);
			return v_[3];
		}

		T& s()
		{
			static_assert(N > 0);
			return v_[0];
		}

		T s() const
		{
			static_assert(N > 0);
			return v_[0];
		}

		T& t()
		{
			static_assert(N > 1);
			return v_[1];
		}

		T t() const
		{
			static_assert(N > 1);
			return v_[1];
		}

		T& p()
		{
			static_assert(N > 2);
			return v_[2];
		}

		T p() const
		{
			static_assert(N > 2);
			return v_[2];
		}

		T& q()
		{
			static_assert(N > 3);
			return v_[3];
		}

		T q() const
		{
			static_assert(N > 3);
			return v_[3];
		}

		Vec<2, T>& xy() const
		{
			static_assert(N > 1);
			return (Vec<2, T>&)v_[0];
		}

		Vec<2, T>& zw() const
		{
			static_assert(N > 3);
			return (Vec<2, T>&)v_[2];
		}

		Vec<2, T> xz() const
		{
			static_assert(N > 2);
			return Vec<2, T>(v_[0], v_[2]);
		}

		Vec<2, T> yw() const
		{
			static_assert(N > 3);
			return Vec<2, T>(v_[1], v_[3]);
		}

		Vec<N, T> copy() const
		{
			return *this;
		}

		Vec<N, T>& set_x(T v)
		{
			static_assert(N > 0);
			v_[0] = v;
			return *this;
		}

		Vec<N, T>& set_y(T v)
		{
			static_assert(N > 1);
			v_[1] = v;
			return *this;
		}

		Vec<N, T>& set_z(T v)
		{
			static_assert(N > 2);
			v_[2] = v;
			return *this;
		}

		Vec<N, T>& set_w(T v)
		{
			static_assert(N > 3);
			v_[3] = v;
			return *this;
		}

		Vec<N, T>& add_x(T v)
		{
			static_assert(N > 0);
			v_[0] += v;
			return *this;
		}

		Vec<N, T>& add_y(T v)
		{
			static_assert(N > 1);
			v_[1] += v;
			return *this;
		}

		Vec<N, T>& add_z(T v)
		{
			static_assert(N > 2);
			v_[2] += v;
			return *this;
		}

		Vec<N, T>& add_w(T v)
		{
			static_assert(N > 3);
			v_[3] += v;
			return *this;
		}

		Vec<N, T>& factor_x(float v)
		{
			static_assert(N > 0);
			v_[0] *= v;
			return *this;
		}

		Vec<N, T>& factor_y(float v)
		{
			static_assert(N > 1);
			v_[1] *= v;
			return *this;
		}

		Vec<N, T>& factor_z(float v)
		{
			static_assert(N > 2);
			v_[2] *= v;
			return *this;
		}

		Vec<N, T>& factor_w(float v)
		{
			static_assert(N > 3);
			v_[3] *= v;
			return *this;
		}

		T operator[](uint i) const
		{
			return v_[i];
		}

		T& operator[](uint i)
		{
			return v_[i];
		}

		explicit Vec(T v)
		{
			for (auto i = 0; i < N; i++)
				v_[i] = v;
		}

		template <uint M, class U>
		explicit Vec(const Vec<M, U>& rhs)
		{
			static_assert(N <= M);
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
			static_assert(N == 3);
			x() = _x;
			y() = _y;
			z() = _z;
		}

		template <class U>
		Vec(const Vec<2, U>& v1, T _z)
		{
			static_assert(N == 3);
			x() = v1.x();
			y() = v1.y();
			z() = _z;
		}

		template <class U>
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

		template <class U>
		Vec(const Vec<2, U>& v1, T _z, T _w)
		{
			static_assert(N == 4);
			x() = v1.x();
			y() = v1.y();
			z() = _z;
			w() = _w;
		}

		template <class U>
		Vec(T _x, const Vec<2, U>& v1, T _w)
		{
			static_assert(N == 4);
			x() = _x;
			y() = v1.x();
			z() = v1.y();
			w() = _w;
		}

		template <class U>
		Vec(T _x, T _y, const Vec<2, U>& v1)
		{
			static_assert(N == 4);
			x() = _x;
			y() = _y;
			z() = v1.x();
			w() = v1.y();
		}

		template <class U>
		Vec(const Vec<2, U>& v1, const Vec<2, U>& v2)
		{
			static_assert(N == 4);
			x() = v1.x();
			y() = v1.y();
			z() = v2.x();
			w() = v2.y();
		}

		template <class U>
		Vec(const Vec<3, U>& v1, T _w)
		{
			static_assert(N == 4);
			x() = v1.x();
			y() = v1.y();
			z() = v1.z();
			w() = _w;
		}

		template <class U>
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

		template <uint M, class U>
		Vec<N, T>& operator=(const Vec<M, U>& rhs) 
		{
			static_assert(N <= M);
			for (auto i = 0; i < N; i++)
				v_[i] = rhs[i];
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

		template <class U>
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

		template <class U>
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

		template <class U>
		Vec<N, T>& operator*=(const Vec<N, U>& rhs)
		{
			for (auto i = 0; i < N; i++)
				v_[i] *= rhs[i];
			return *this;
		}

		Vec<N, T>& operator/=(T rhs)
		{
			rhs = T(1) / rhs;
			for (auto i = 0; i < N; i++)
				v_[i] *= rhs;
			return *this;
		}

		template <class U>
		Vec<N, T>& operator/=(const Vec<N, U>& rhs)
		{
			for (auto i = 0; i < N; i++)
				v_[i] /= rhs[i];
			return *this;
		}

		T sum() const
		{
			auto ret = T(0);
			for (auto i = 0; i < N; i++)
				ret += v_[i];
			return ret;
		}
	};

	template <uint N, class T>
	Vec<N, T> operator+(T lhs, const Vec<N, T>& rhs)
	{
		Vec<N, T> ret(rhs);
		for (auto i = 0; i < N; i++)
			ret[i] += lhs;
		return ret;
	}

	template <uint N, class T>
	Vec<N, T> operator+(const Vec<N, T>& lhs, T rhs)
	{
		Vec<N, T> ret(lhs);
		for (auto i = 0; i < N; i++)
			ret[i] += rhs;
		return ret;
	}

	template <uint N, class T, class U>
	Vec<N, T> operator+(const Vec<N, T>& lhs, const Vec<N, U>& rhs)
	{
		Vec<N, T> ret(lhs);
		for (auto i = 0; i < N; i++)
			ret[i] += rhs[i];
		return ret;
	}

	template <uint N, class T>
	Vec<N, T> operator-(T lhs, const Vec<N, T>& rhs)
	{
		Vec<N, T> ret(rhs);
		for (auto i = 0; i < N; i++)
			ret[i] -= lhs;
		return ret;
	}

	template <uint N, class T>
	Vec<N, T> operator-(const Vec<N, T> & lhs, T rhs)
	{
		Vec<N, T> ret(lhs);
		for (auto i = 0; i < N; i++)
			ret[i] -= rhs;
		return ret;
	}

	template <uint N, class T, class U>
	Vec<N, T> operator-(const Vec<N, T> & lhs, const Vec<N, U> & rhs)
	{
		Vec<N, T> ret(lhs);
		for (auto i = 0; i < N; i++)
			ret[i] -= rhs[i];
		return ret;
	}

	template <uint N, class T>
	Vec<N, T> operator*(T lhs, const Vec<N, T>& rhs)
	{
		Vec<N, T> ret(rhs);
		for (auto i = 0; i < N; i++)
			ret[i] *= lhs;
		return ret;
	}

	template <uint N, class T>
	Vec<N, T> operator*(const Vec<N, T> & lhs, T rhs)
	{
		Vec<N, T> ret(lhs);
		for (auto i = 0; i < N; i++)
			ret[i] *= rhs;
		return ret;
	}

	template <uint N, class T, class U>
	Vec<N, T> operator*(const Vec<N, T> & lhs, const Vec<N, U> & rhs)
	{
		Vec<N, T> ret(lhs);
		for (auto i = 0; i < N; i++)
			ret[i] *= rhs[i];
		return ret;
	}

	template <uint N, class T>
	Vec<N, T> operator/(T lhs, const Vec<N, T>& rhs)
	{
		Vec<N, T> ret;
		for (auto i = 0; i < N; i++)
			ret[i] = lhs / rhs[i];
		return ret;
	}

	template <uint N, class T>
	Vec<N, T> operator/(const Vec<N, T> & lhs, T rhs)
	{
		Vec<N, T> ret(lhs);
		for (auto i = 0; i < N; i++)
			ret[i] /= rhs;
		return ret;
	}

	template <uint N, class T, class U>
	Vec<N, T> operator/(const Vec<N, T> & lhs, const Vec<N, U> & rhs)
	{
		Vec<N, T> ret(lhs);
		for (auto i = 0; i < N; i++)
			ret[i] /= rhs[i];
		return ret;
	}

	template <uint N, class T>
	bool operator<(T lhs, const Vec<N, T>& rhs)
	{
		for (auto i = 0; i < N; i++)
		{
			if (lhs >= rhs[i])
				return false;
		}
		return true;
	}

	template <uint N, class T>
	bool operator<(const Vec<N, T>& lhs, T rhs)
	{
		for (auto i = 0; i < N; i++)
		{
			if (lhs[i] >= rhs)
				return false;
		}
		return true;
	}

	template <uint N, class T, class U>
	bool operator<(const Vec<N, T>& lhs, const Vec<N, U>& rhs)
	{
		for (auto i = 0; i < N; i++)
		{
			if (lhs[i] >= rhs[i])
				return false;
		}
		return true;
	}

	template <uint N, class T>
	bool operator<=(T lhs, const Vec<N, T>& rhs)
	{
		for (auto i = 0; i < N; i++)
		{
			if (lhs > rhs[i])
				return false;
		}
		return true;
	}

	template <uint N, class T>
	bool operator<=(const Vec<N, T>& lhs, T rhs)
	{
		for (auto i = 0; i < N; i++)
		{
			if (lhs[i] > rhs)
				return false;
		}
		return true;
	}

	template <uint N, class T, class U>
	bool operator<=(const Vec<N, T>& lhs, const Vec<N, U>& rhs)
	{
		for (auto i = 0; i < N; i++)
		{
			if (lhs[i] > rhs[i])
				return false;
		}
		return true;
	}

	template <uint N, class T>
	bool operator>(T lhs, const Vec<N, T>& rhs)
	{
		for (auto i = 0; i < N; i++)
		{
			if (lhs <= rhs[i])
				return false;
		}
		return true;
	}

	template <uint N, class T>
	bool operator>(const Vec<N, T>& lhs, T rhs)
	{
		for (auto i = 0; i < N; i++)
		{
			if (lhs[i] <= rhs)
				return false;
		}
		return true;
	}

	template <uint N, class T, class U>
	bool operator>(const Vec<N, T>& lhs, const Vec<N, U>& rhs)
	{
		for (auto i = 0; i < N; i++)
		{
			if (lhs[i] <= rhs[i])
				return false;
		}
		return true;
	}

	template <uint N, class T>
	bool operator>=(T lhs, const Vec<N, T>& rhs)
	{
		for (auto i = 0; i < N; i++)
		{
			if (lhs < rhs[i])
				return false;
		}
		return true;
	}

	template <uint N, class T>
	bool operator>=(const Vec<N, T>& lhs, T rhs)
	{
		for (auto i = 0; i < N; i++)
		{
			if (lhs[i] < rhs)
				return false;
		}
		return true;
	}

	template <uint N, class T, class U>
	bool operator>=(const Vec<N, T>& lhs, const Vec<N, U>& rhs)
	{
		for (auto i = 0; i < N; i++)
		{
			if (lhs[i] < rhs[i])
				return false;
		}
		return true;
	}

	template <uint N, class T>
	bool operator==(T lhs, const Vec<N, T>& rhs)
	{
		for (auto i = 0; i < N; i++)
		{
			if (lhs != rhs[i])
				return false;
		}
		return true;
	}

	template <uint N, class T>
	bool operator==(const Vec<N, T>& lhs, T rhs)
	{
		for (auto i = 0; i < N; i++)
		{
			if (lhs[i] != rhs)
				return false;
		}
		return true;
	}

	template <uint N, class T, class U>
	bool operator==(const Vec<N, T>& lhs, const Vec<N, U>& rhs)
	{
		for (auto i = 0; i < N; i++)
		{
			if (lhs[i] != rhs[i])
				return false;
		}
		return true;
	}

	template <uint N, class T>
	bool operator!=(T lhs, const Vec<N, T>& rhs)
	{
		return !(lhs == rhs);
	}

	template <uint N, class T>
	bool operator!=(const Vec<N, T>& lhs, T rhs)
	{
		return !(lhs == rhs);
	}

	template <uint N, class T, class U>
	bool operator!=(const Vec<N, T>& lhs, const Vec<N, U>& rhs)
	{
		return !(lhs == rhs);
	}

	using Vec1b = Vec<1, bool>;
	using Vec2b = Vec<2, bool>;
	using Vec3b = Vec<3, bool>;
	using Vec4b = Vec<4, bool>;

	using Vec1c = Vec<1, uchar>;
	using Vec2c = Vec<2, uchar>;
	using Vec3c = Vec<3, uchar>;
	using Vec4c = Vec<4, uchar>;

	using Vec1i = Vec<1, int>;
	using Vec2i = Vec<2, int>;
	using Vec3i = Vec<3, int>;
	using Vec4i = Vec<4, int>;

	using Vec1u = Vec<1, uint>;
	using Vec2u = Vec<2, uint>;
	using Vec3u = Vec<3, uint>;
	using Vec4u = Vec<4, uint>;

	using Vec1f = Vec<1, float>;
	using Vec2f = Vec<2, float>;
	using Vec3f = Vec<3, float>;
	using Vec4f = Vec<4, float>;

	union CommonValue
	{
		Vec4c c;
		Vec4i i;
		Vec4u u;
		Vec4f f;
		void* p;
	};

	template<uint N>
	CommonValue common(const Vec<N, uchar>& v)
	{
		CommonValue cv;
		for (auto i = 0; i < N; i++)
			cv.c[i] = v[i];
		return cv;
	}

	template<uint N>
	CommonValue common(const Vec<N, int>& v)
	{
		CommonValue cv;
		for (auto i = 0; i < N; i++)
			cv.i[i] = v[i];
		return cv;
	}

	template<uint N>
	CommonValue common(const Vec<N, uint>& v)
	{
		CommonValue cv;
		for (auto i = 0; i < N; i++)
			cv.u[i] = v[i];
		return cv;
	}

	template<uint N>
	CommonValue common(const Vec<N, float>& v)
	{
		CommonValue cv;
		for (auto i = 0; i < N; i++)
			cv.f[i] = v[i];
		return cv;
	}

	inline CommonValue common(void* p)
	{
		CommonValue cv;
		cv.p = p;
		return cv;
	}

	template <uint M, uint N, class T>
	struct Mat
	{
		Vec<N, T> v_[M];

		Vec<N, T> operator[](uint i) const
		{
			return v_[i];
		}

		Vec<N, T>& operator[](uint i)
		{
			return v_[i];
		}

		Mat() = default;

		explicit Mat(T v)
		{
			for (auto i = 0; i < M; i++)
				v_[i] = Vec<N, T>(0);
			auto n = min(N, M);
			for (auto i = 0; i < n; i++)
				v_[i][i] = v;
		}

		explicit Mat(const Vec<N, T>& v)
		{
			static_assert(N == M);
			for (auto i = 0; i < N; i++)
				v_[i] = Vec<N, T>(0);
			for (auto i = 0; i < N; i++)
				v_[i][i] = v[i];
		}

		template <uint O, uint P, class U>
		explicit Mat(const Mat<O, P, U>& rhs)
		{
			static_assert(M <= O && N <= P);
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

		template <uint N, class U>
		Mat(const Vec<N, U>& _v1, const Vec<N, U>& _v2)
		{
			static_assert(M == 2);
			v_[0] = _v1;
			v_[1] = _v2;
		}

		template <uint N, class U>
		Mat(const Vec<N, U>& _v1, const Vec<N, U>& _v2, const Vec<N, U>& _v3)
		{
			static_assert(M == 3);
			v_[0] = _v1;
			v_[1] = _v2;
			v_[2] = _v3;
		}

		template <class U>
		Mat(const Vec<N, U>& _v1, const Vec<N, U>& _v2, const Vec<N, U>& _v3, const Vec<N, U>& _v4)
		{
			static_assert(M == 4);
			v_[0] = _v1;
			v_[1] = _v2;
			v_[2] = _v3;
			v_[3] = _v4;
		}

		template <class U>
		Mat(const Mat<M - 1, N, U>& _m, const Vec<N, U>& _v)
		{
			for (auto i = 0; i < M - 1; i++)
				v_[i] = _m[i];
			v_[M - 1] = _v;
		}

		template <class U>
		Mat(const Mat<M, N - 1, U>& _m, const Vec<M, U>& _v)
		{
			for (auto i = 0; i < M; i++)
				v_[i] = Vec<N, T>(_m[i], _v[i]);
		}

		template <class U>
		Mat(const Mat<M - 1, N - 1, U>& _m)
		{
			for (auto i = 0; i < M - 1; i++)
				v_[i] = Vec<N, T>(_m[i], 0.f);
			for (auto i = 0; i < N - 1; i++)
				v_[M - 1][i] = 0.f;
			v_[M - 1][N - 1] = 1.f;
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

		template <uint O, uint P, class U>
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

		template <class U>
		Mat<N, M, T>& operator+=(const Mat<N, M, U>& rhs)
		{
			for (auto i = 0; i < M; i++)
				v_[i] += rhs[i];
			return *this;
		}

		template <class U>
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

	template <uint N, uint M, class T, class U>
	Mat<N, M, T> operator+(const Mat<N, M, T>& lhs, const Mat<N, M, U>& rhs)
	{
		Mat<N, M, T> ret(lhs);
		for (auto i = 0; i < M; i++)
			ret[i] += rhs[i];
		return ret;
	}

	template <uint N, uint M, class T, class U>
	Mat<N, M, T> operator-(const Mat<N, M, T>& lhs, const Mat<N, M, U>& rhs)
	{
		Mat<N, M, T> ret(lhs);
		for (auto i = 0; i < M; i++)
			ret[i] -= rhs[i];
		return ret;
	}

	template <uint N, uint M, class T>
	Mat<N, M, T> operator*(const Mat<N, M, T>& lhs, T rhs)
	{
		Mat<N, M, T> ret(lhs);
		for (auto i = 0; i < M; i++)
			ret[i] *= rhs;
		return ret;
	}

	template <uint N, uint M, class T>
	Mat<N, M, T> operator*(T lhs, const Mat<N, M, T>& rhs)
	{
		Mat<N, M, T> ret(rhs);
		for (auto i = 0; i < M; i++)
			ret[i] *= lhs;
		return ret;
	}

	template <uint N, uint M, uint O, class T>
	Mat<N, O, T> operator*(const Mat<N, M, T>& lhs, const Mat<M, O, T>& rhs)
	{
		Mat<N, O, T> ret;
		for (auto i = 0; i < O; i++)
		{
			for (auto j = 0; j < N; j++)
			{
				ret[i][j] = 0;
				for (auto k = 0; k < M; k++)
					ret[i][j] += lhs[k][j] * rhs[i][k];
			}
		}
		return ret;
	}

	template <uint N, uint M, class T>
	Vec<M, T> operator*(const Mat<N, M, T>& lhs, const Vec<M, T>& rhs)
	{
		Vec<M, T> ret;
		for (auto i = 0; i < N; i++)
		{
			ret[i] = 0;
			for (auto j = 0; j < M; j++)
				ret[i] += lhs[j][i] * rhs[j];
		}
		return ret;
	}

	template <uint N, uint M, class T>
	Mat<N, M, T> operator/(const Mat<N, M, T>& lhs, T rhs)
	{
		Mat<N, M, T> ret(lhs);
		for (auto i = 0; i < M; i++)
			ret[i] /= rhs;
		return ret;
	}

	using Mat2i = Mat<2, 2, int>;
	using Mat3i = Mat<3, 3, int>;
	using Mat4i = Mat<4, 4, int>;

	using Mat2u = Mat<2, 2, uint>;
	using Mat3u = Mat<3, 3, uint>;
	using Mat4u = Mat<4, 4, uint>;

	using Mat2f = Mat<2, 2, float>;
	using Mat3f = Mat<3, 3, float>;
	using Mat4f = Mat<4, 4, float>;
	using Mat23f = Mat<2, 3, float>;

	template <class T>
	Vec<3, T> x_axis()
	{
		return Vec<3, T>(1, 0, 0);
	}

	template <class T>
	Vec<3, T> y_axis()
	{
		return Vec<3, T>(0, 1, 0);
	}

	template <class T>
	Vec<3, T> z_axis()
	{
		return Vec<3, T>(0, 0, 1);
	}

	template <class T>
	Vec<3, T> axis(uint idx)
	{
		static Vec<3, T> axes[] = {
			x_axis<T>(),
			y_axis<T>(),
			z_axis<T>()
		};
		return axes[idx];
	}

	template <uint N, class T>
	T length(const Vec<N, T>& v)
	{
		T s = 0;
		for (auto i = 0; i < N; i++)
			s += v[i] * v[i];
		return sqrt(s);
	}

	template <uint N, class T>
	Vec<N, T> normalize(const Vec<N, T>& v)
	{
		Vec<N, T> ret(v);
		ret /= length(v);
		return ret;
	}

	template <class T>
	float sign(T v)
	{
		return v >= T(0) ? 1.f : -1.f;
	}

	template <class T>
	T abs(T v)
	{
		return v >= T(0) ? v : -v;
	}

	template <class T>
	T min(T a, T b)
	{
		return a < b ? a : b;
	}

	template <uint N, class T>
	Vec<N, T> min(const Vec<N, T>& a, const Vec<N, T>& b)
	{
		Vec<N, T> ret;
		for (auto i = 0; i < N; i++)
			ret[i] = min(a[i], b[i]);
		return ret;
	}

	template <class T, class ...Args>
	T minN(T a, T b, Args... args)
	{
		return minN(min(a, b), args...);
	}

	template <class T>
	T max(T a, T b)
	{
		return a > b ? a : b;
	}

	template <uint N, class T>
	Vec<N, T> max(const Vec<N, T>& a, const Vec<N, T>& b)
	{
		Vec<N, T> ret;
		for (auto i = 0; i < N; i++)
			ret[i] = max(a[i], b[i]);
		return ret;
	}

	template <class T, class ...Args>
	T maxN(T a, T b, Args... rest)
	{
		return maxN(max(a, b), rest...);
	}

	template <class T>
	T clamp(T v, T a, T b)
	{
		if (v < a)
			return a;
		if (v > b)
			return b;
		return v;
	}

	template <uint N, class T>
	Vec<N, T> clamp(const Vec<N, T>& v, const Vec<N, T>& a, const Vec<N, T>& b)
	{
		Vec<N, T> ret;
		for (auto i = 0; i < N; i++)
			ret[i] = clamp(v[i], a[i], b[i]);
		return ret;
	}

	template <uint N>
	Vec<N, float> floor(const Vec<N, float>& v)
	{
		Vec<N, float> ret;
		for (auto i = 0; i < N; i++)
			ret[i] = ::floor(v[i]);
		return ret;
	}

	inline float fract(float v)
	{
		return v - ::floor(v);
	}

	template <uint N>
	Vec<N, float> fract(const Vec<N, float>& v)
	{
		Vec<N, float> ret;
		for (auto i = 0; i < N; i++)
			ret[i] = fract(v[i]);
		return ret;
	}

	template <class T>
	Vec<2, T> mod(T a, T b)
	{
		return Vec<2, T>(a / b, a % b);
	}

	template <class T>
	T mix(T v0, T v1, T q)
	{
		return v0 + q * (v1 - v0);
	}

	template <uint N, class T>
	Vec<N, T> mix(const Vec<N, T>& v0, const Vec<N, T>& v1, T q)
	{
		Vec<N, T> ret;
		for (auto i = 0; i < N; i++)
			ret[i] = mix(v0[i], v1[i], q);
		return ret;
	}
	
	template <uint N, class T>
	T dot(const Vec<N, T>& lhs, const Vec<N, T>& rhs)
	{
		T ret = 0;
		for (auto i = 0; i < N; i++)
			ret += lhs.v_[i] * rhs.v_[i];
		return ret;
	}

	template <class T>
	T cross(const Vec<2, T>& lhs, const Vec<2, T>& rhs)
	{
		return lhs.x() * rhs.y() - lhs.y() * rhs.x();
	}

	template <class T>
	Vec<3, T> cross(const Vec<3, T>& lhs, const Vec<3, T>& rhs)
	{
		return Vec<3, T>(
			lhs.v_[1] * rhs.v_[2] - rhs.v_[1] * lhs.v_[2],
			lhs.v_[2] * rhs.v_[0] - rhs.v_[2] * lhs.v_[0],
			lhs.v_[0] * rhs.v_[1] - rhs.v_[0] * lhs.v_[1]);
	}

	template <uint N, class T>
	T distance_square(const Vec<N, T>& lhs, const Vec<N, T>& rhs)
	{
		auto d = lhs - rhs;
		return dot(d, d);
	}

	template <uint N, class T>
	T distance(const Vec<N, T>& lhs, const Vec<N, T>& rhs)
	{
		return sqrt(distance_square(lhs, rhs));
	}

	template <uint N, class T>
	Mat<N, N, T> transpose(const Mat<N, N, T>& m)
	{
		Mat<N, N, T> ret;
		for (auto i = 0; i < N; i++)
		{
			for (auto j = 0; j < N; j++)
				ret[i][j] = m[j][i];
		}
		return ret;
	}

	template <class T>
	Mat<2, 2, T> inverse(const Mat<2, 2, T>& m)
	{
		auto det_inv = T(1) / (
			m[0][0] * m[1][1] -
			m[1][0] * m[0][1]);

		return Mat<2, 2, T>(
			Vec<2, T>(m[1][1] * det_inv, -m[0][1] * det_inv),
			Vec<2, T>(-m[1][0] * det_inv, m[0][0] * det_inv));
	}

	template <class T>
	Mat<3, 3, T> inverse(const Mat<3, 3, T>& m)
	{
		auto det_inv = T(1) / (
			m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
			- m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
			+ m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]));

		Mat<3, 3, T> ret;
		ret[0][0] = +(m[1][1] * m[2][2] - m[2][1] * m[1][2]) * det_inv;
		ret[1][0] = -(m[1][0] * m[2][2] - m[2][0] * m[1][2]) * det_inv;
		ret[2][0] = +(m[1][0] * m[2][1] - m[2][0] * m[1][1]) * det_inv;
		ret[0][1] = -(m[0][1] * m[2][2] - m[2][1] * m[0][2]) * det_inv;
		ret[1][1] = +(m[0][0] * m[2][2] - m[2][0] * m[0][2]) * det_inv;
		ret[2][1] = -(m[0][0] * m[2][1] - m[2][0] * m[0][1]) * det_inv;
		ret[0][2] = +(m[0][1] * m[1][2] - m[1][1] * m[0][2]) * det_inv;
		ret[1][2] = -(m[0][0] * m[1][2] - m[1][0] * m[0][2]) * det_inv;
		ret[2][2] = +(m[0][0] * m[1][1] - m[1][0] * m[0][1]) * det_inv;

		return ret;
	}

	template <class T>
	Mat<4, 4, T> inverse(const Mat<4, 4, T>& m)
	{
		auto coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
		auto coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
		auto coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

		auto coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
		auto coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
		auto coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

		auto coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
		auto coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
		auto coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

		auto coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
		auto coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
		auto coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

		auto coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
		auto coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
		auto coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

		auto coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
		auto coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
		auto coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

		Vec<4, T> fac0(coef00, coef00, coef02, coef03);
		Vec<4, T> fac1(coef04, coef04, coef06, coef07);
		Vec<4, T> fac2(coef08, coef08, coef10, coef11);
		Vec<4, T> fac3(coef12, coef12, coef14, coef15);
		Vec<4, T> fac4(coef16, coef16, coef18, coef19);
		Vec<4, T> fac5(coef20, coef20, coef22, coef23);

		Vec<4, T> vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
		Vec<4, T> vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
		Vec<4, T> vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
		Vec<4, T> vec3(m[1][3], m[0][3], m[0][3], m[0][3]);

		Vec<4, T> inv0(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
		Vec<4, T> inv1(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
		Vec<4, T> inv2(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
		Vec<4, T> inv3(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

		Vec<4, T> signA(+1, -1, +1, -1);
		Vec<4, T> signB(-1, +1, -1, +1);
		Mat<4, 4, T> inv(inv0 * signA, inv1 * signB, inv2 * signA, inv3 * signB);

		Vec<4, T> row0(inv[0][0], inv[1][0], inv[2][0], inv[3][0]);

		Vec<4, T> dot0(m[0] * row0);
		return inv / ((dot0.x() + dot0.y()) + (dot0.z() + dot0.w()));
	}

	template <class T>
	Mat<2, 2, T> make_rotation_matrix(T rad)
	{
		auto c = cos(rad);
		auto s = sin(rad);

		return Mat<2, 2, T>(Vec<2, T>(c, s), Vec<2, T>(-s, c));
	}

	template <class T>
	Mat<3, 3, T> make_rotation_matrix(const Vec<3, T>& axis, T rad)
	{
		auto c = cos(rad);
		auto s = sin(rad);

		Vec<3, T> temp((T(1) - c) * axis);

		Mat<3, 3, T> ret;

		ret[0][0] = c + temp[0] * axis[0];
		ret[0][1] = 0 + temp[0] * axis[1] + s * axis[2];
		ret[0][2] = 0 + temp[0] * axis[2] - s * axis[1];

		ret[1][0] = 0 + temp[1] * axis[0] - s * axis[2];
		ret[1][1] = c + temp[1] * axis[1];
		ret[1][2] = 0 + temp[1] * axis[2] + s * axis[0];

		ret[2][0] = 0 + temp[2] * axis[0] + s * axis[1];
		ret[2][1] = 0 + temp[2] * axis[1] - s * axis[0];
		ret[2][2] = c + temp[2] * axis[2];

		return ret;
	}

	template <class T>
	Mat<4, 4, T> make_view_matrix(const Vec<3, T>& eye, const Vec<3, T>& center, const Vec<3, T>& up)
	{
		auto f = normalize(center - eye);
		auto s = normalize(cross(f, up));
		auto u = normalize(cross(s, f));

		Mat<4, 4, T> ret(1);
		ret[0][0] = s.x();
		ret[1][0] = s.y();
		ret[2][0] = s.z();
		ret[0][1] = u.x();
		ret[1][1] = u.y();
		ret[2][1] = u.z();
		ret[0][2] = -f.x();
		ret[1][2] = -f.y();
		ret[2][2] = -f.z();
		ret[3][0] = -dot(s, eye);
		ret[3][1] = -dot(u, eye);
		ret[3][2] = dot(f, eye);
		return ret;
	}

	template <class T>
	Mat<4, 4, T> make_project_matrix(T fovy, T aspect, T zNear, T zFar)
	{
		auto t = tan(fovy / 2.f);

		Mat<4, 4, T> ret(T(0));
		ret[0][0] = T(1) / (aspect * t);
		ret[1][1] = T(-1) / (t);
		ret[2][2] = zFar / (zNear - zFar);
		ret[2][3] = T(-1);
		ret[3][2] = -(zFar * zNear) / (zFar - zNear);
		return ret;
	}

	struct PerspectiveProjector
	{
		float _screen_width;
		float _screen_height;
		float _screen_ratio;
		float _fovy;
		float _tan_fovy;
		float _tan_fovy_near2;
		float _near;
		float _far;

		PerspectiveProjector(float screen_width, float screen_height, float fovy, float near, float far) :
			_screen_width(screen_width),
			_screen_height(screen_height),
			_fovy(fovy),
			_near(near),
			_far(far)
		{
			_screen_ratio = _screen_width / _screen_height;
			_tan_fovy = tan(_fovy * ANG_RAD);
			_tan_fovy_near2 = _tan_fovy * _near * _near;
		}

		Vec2f project(const Vec3f& p)
		{
			return Vec2f((p.x() / p.z() * _tan_fovy_near2 * _screen_ratio + 1.f) * 0.5f * _screen_width,
				(-p.y() / p.z() * _tan_fovy_near2 + 1.f) * 0.5f * _screen_height);
		}
	};

	template <class T>
	T segment_intersect(const Vec<2, T>& a, const Vec<2, T>& b, const Vec<2, T>& c, const Vec<2, T>& d)
	{
		auto ab = b - a;
		auto dc = c - d;
		return cross(ab, c -a) * cross(ab, d - a) <= 0.f &&
			cross(dc, a - d) * cross(dc, b - d) <= 0.f;
	}

	template <class T>
	T segment_distance(const Vec<2, T>& a, const Vec<2, T>& b, const Vec<2, T>& p)
	{
		auto l2 = distance_square(a, b);
		auto l = sqrt(l2);
		auto pa2 = distance_square(p, a);
		auto pb2 = distance_square(p, b);
		auto x = (pa2 - pb2 + l2) * 0.5f / l;
		if (x < 0.f)
			return sqrt(pa2);
		if (x > l)
			return sqrt(pb2);
		return sqrt(pa2 - x * x);
	}

	template <uint N, class T>
	Vec<N, T> bezier(float t, const Vec<N, T>& p0, const Vec<N, T>& p1, const Vec<N, T>& p2, const Vec<N, T>& p3)
	{
		return (T(1) - t) * (T(1) - t) * (T(1) - t) * p0 +
			T(3) * (T(1) - t) * (T(1) - t) * t * p1 +
			T(3) * (T(1) - t) * t * t * p2 +
			t * t * t * p3;
	}

	template <uint N, class T>
	T bezier_closest(int iters, const Vec<N, T>& pos, float start, float end, int slices,
		const Vec<N, T>& p0, const Vec<N, T>& p1, const Vec<N, T>& p2, const Vec<N, T>& p3)
	{
		if (iters <= 0)
			return (start + end) / 2;

		float tick = (end - start) / float(slices);
		float best = 0;
		float best_distance = T(1000000);
		float t = start;

		while (t <= end)
		{
			auto v = bezier(t, p0, p1, p2, p3);
			auto d = v - pos;
			float current_distance = d.x() * d.x() + d.y() * d.y();
			if (current_distance < best_distance)
			{
				best_distance = current_distance;
				best = t;
			}
			t += tick;
		}

		return bezier_closest(iters - 1, pos, max(best - tick, 0.f), min(best + tick, T(1)), slices, p0, p1, p2, p3);
	}

	template <uint N, class T>
	Vec<N, T> bezier_closest_point(const Vec<N, T>& pos, 
		const Vec<N, T>& p0, const Vec<N, T>& p1, const Vec<N, T>& p2, const Vec<N, T>& p3,
		int slices, int iters)
	{
		return bezier(bezier_closest(iters, pos, 0.f, T(1), slices, p0, p1, p2, p3), p0, p1, p2, p3);
	}

	inline float random()
	{
		return (float)::rand() / (float)RAND_MAX;
	}

	template <class T>
	T rand(const Vec<2, T>& v)
	{
		return fract(cos(v.x() * (12.9898) + v.y() * (4.1414)) * 43758.5453);
	}

	template <class T>
	T noise(const Vec<2, T>& _v)
	{
		const auto SC = 250;

		auto v = _v / SC;
		auto vf = fract(v);
		auto vi = floor(v);

		auto r0 = rand(vi);
		auto r1 = rand(vi + Vec<2, T>(T(1), T(0)));
		auto r2 = rand(vi + Vec<2, T>(T(0), T(1)));
		auto r3 = rand(vi + Vec<2, T>(T(1), T(1)));

		auto vs = T(3) * vf * vf - T(2) * vf * vf * vf;

		return mix(
			mix(r0, r1, vs.x()),
			mix(r2, r3, vs.x()),
			vs.y());
	}

	template <class T>
	T fbm(const Vec<2, T>& _v)
	{
		auto v = _v;
		auto r = T(0);

		auto a = T(1) / T(3);
		for (auto i = 0; i < 4; i++)
		{
			r += noise(v) * a;
			a /= T(3);
			v *= T(3);
		}

		return r;
	}

	// Vec4f as Rect
	// (x, y) - min, (z, w) - max
	template <class T>
	Vec<4, T> rect(const Vec<2, T>& _min, const Vec<2, T>& _max)
	{
		return Vec<4, T>(_min, _max);
	}

	template <class T>
	void rect_expand(Vec<4, T>& rect, T length)
	{
		rect.x() -= length;
		rect.y() -= length;
		rect.z() += length;
		rect.w() += length;
	}

	template <class T>
	void rect_expand(Vec<4, T>& rect, const Vec<2, T>& p)
	{
		rect.x() = min(rect.x(), p.x());
		rect.y() = min(rect.y(), p.y());
		rect.z() = max(rect.z(), p.x());
		rect.w() = max(rect.w(), p.y());
	}

	template <class T, class ...Args>
	Vec<4, T> rect_from_points(std::span<Vec<2, T>> points)
	{
		auto ret = Vec<4, T>(0);
		for (auto& p : points)
		{
			ret.x() = min(ret.x(), p.x());
			ret.y() = min(ret.y(), p.y());
			ret.z() = max(ret.z(), p.x());
			ret.w() = max(ret.w(), p.y());
		}
		return ret;
	}

	template <class T>
	bool rect_contains(const Vec<4, T>& rect, const Vec<2, T>& p)
	{
		return p.x() > rect.x() && p.x() < rect.z() &&
			p.y() > rect.y() && p.y() < rect.w();
	}

	template <class T>
	bool rect_overlapping(const Vec<4, T>& lhs, const Vec<4, T>& rhs)
	{
		return lhs.x() <= rhs.z() && lhs.z() >= rhs.x() &&
			lhs.y() <= rhs.w() && lhs.w() >= rhs.y();
	}

	template <class T>
	bool convex_contains(const Vec<2, T>& p, std::span<Vec<2, T>> points)
	{
		if (points.size() < 3)
			return false;

		if (cross(Vec3f(p - points[0], 0), Vec3f(points[1] - p, 0)).z() > 0.f)
			return false;
		if (cross(Vec3f(p - points[points.size() - 1], 0), Vec3f(points[0] - p, 0)).z() > 0.f)
			return false;

		for (auto i = 1; i < points.size() - 1; i++)
		{
			if (cross(Vec3f(p - points[i], 0), Vec3f(points[i + 1] - p, 0)).z() > 0.f)
				return false;
		}

		return true;
	}

	template <class T>
	SideFlags rect_side(const Vec<4, T>& rect, const Vec<4, T>& p, T threshold)
	{
		if (p.x() < rect.z() && p.x() > rect.z() - threshold &&
			p.y() > rect.y() && p.y() < rect.y() + threshold)
			return SideNE;
		if (p.x() > rect.x() && p.x() < rect.x() + threshold &&
			p.y() > rect.y() && p.y() < rect.y() + threshold)
			return SideNW;
		if (p.x() < rect.z() && p.x() > rect.z() - threshold &&
			p.y() < rect.w() && p.y() > rect.w() - threshold)
			return SideSE;
		if (p.x() > rect.x() && p.x() < rect.x() + threshold &&
			p.y() < rect.w() && p.y() > rect.w() - threshold)
			return SideSW;
		if (p.y() > rect.y() - threshold && p.y() < rect.y() + threshold &&
			p.x() > rect.x() && p.x() < rect.z())
			return SideN;
		if (p.y() < rect.w() && p.y() > rect.w() - threshold &&
			p.x() > rect.x() && p.x() < rect.z())
			return SideS;
		if (p.x() < rect.z() && p.x() > rect.z() - threshold &&
			p.y() > rect.y() && p.y() < rect.w())
			return SideE;
		if (p.x() > rect.x() && p.x() < rect.x() + threshold &&
			p.y() > rect.y() && p.y() < rect.w())
			return SideW;
		if (rect_contains(rect, p))
			return Inside;
		return Outside;
	}

	template <class T>
	Vec<2, T> side_dir(SideFlags s)
	{
		switch (s)
		{
		case SideN:
			return Vec<2, T>(T(0), T(-1));
		case SideS:
			return Vec<2, T>(T(0), T(1));
		case SideW:
			return Vec<2, T>(T(-1), T(0));
		case SideE:
			return Vec<2, T>(T(1), T(0));
		case SideNW:
			return Vec<2, T>(T(-1), T(-1));
		case SideNE:
			return Vec<2, T>(T(1), T(-1));
		case SideSW:
			return Vec<2, T>(T(-1), T(1));
		case SideSE:
			return Vec<2, T>(T(1), T(1));
		case Outside:
			return Vec<2, T>(T(2));
		case Inside:
			return Vec<2, T>(T(0));
		}
	}

	template <class T>
	Vec<4, T> fit_rect(const Vec<2, T>& desired_size, float xy_aspect)
	{
		if (desired_size.x() <= T(0) || desired_size.y() <= T(0))
			return Vec<4, T>(T(0), T(0), T(1), T(1));
		Vec<4, T> ret;
		if (desired_size.x() / desired_size.y() > xy_aspect)
		{
			ret.z() = xy_aspect * desired_size.y();
			ret.w() = desired_size.y();
			ret.x() = (desired_size.x() - ret.w()) * 0.5f;
			ret.y() = 0;
			ret.z() += ret.x();
		}
		else
		{
			ret.z() = desired_size.x();
			ret.w() = desired_size.x() / xy_aspect;
			ret.x() = 0;
			ret.y() = (desired_size.y() - ret.w()) * 0.5f;
			ret.w() += ret.x();
		}
		return ret;
	}

	template <class T>
	Vec<4, T> fit_rect_no_zoom_in(const Vec<2, T>& desired_size, const Vec<2, T>& size)
	{
		if (desired_size.x() <= T(0) || desired_size.y() <= T(0))
			return Vec<4, T>(T(0), T(0), T(1), T(1));
		if (size.x() <= desired_size.x() && size.y() <= desired_size.y())
		{
			Vec<4, T> ret;
			ret.z() = size.x();
			ret.w() = size.y();
			ret.x() = (desired_size.x() - size.x()) * 0.5f;
			ret.y() = (desired_size.y() - size.y()) * 0.5f;
			ret.z() += ret.x();
			ret.w() += ret.y();
			return ret;
		}
		else
			return fit_rect(desired_size, size.x() / size.y());
	}

	inline Vec3c col3_inv(const Vec3c& col)
	{
		return Vec3c(255 - col.r(), 255 - col.g(), 255 - col.b());
	}

	inline Vec4c col3_inv(const Vec4c& col)
	{
		return Vec4c(255 - col.r(), 255 - col.g(), 255 - col.b(), col.a());
	}

	inline Vec3c hsv_2_col3(const Vec3f& hsv)
	{
		auto h = hsv.x();
		auto s = hsv.y();
		auto v = hsv.z();
		auto hdiv60 = h / 60.f;
		auto hi = int(hdiv60) % 6;
		auto f = hdiv60 - hi;
		auto p = v * (1.f - s);
		float q, t;

		switch (hi)
		{
		case 0:
			t = v * (1.f - (1.f - f) * s);
			return Vec3c(v * 255.f, t * 255.f, p * 255.f);
		case 1:
			q = v * (1.f - f * s);
			return Vec3c(q * 255.f, v * 255.f, p * 255.f);
		case 2:
			t = v * (1.f - (1.f - f) * s);
			return Vec3c(p * 255.f, v * 255.f, t * 255.f);
		case 3:
			q = v * (1.f - f * s);
			return Vec3c(p * 255.f, q * 255.f, v * 255.f);
		case 4:
			t = v * (1.f - (1.f - f) * s);
			return Vec3c(t * 255.f, p * 255.f, v * 255.f);
		case 5:
			q = v * (1.f - f * s);
			return Vec3c(v * 255.f, p * 255.f, q * 255.f);
		default:
			return Vec3c(0.f);
		}
	}

	inline Vec3c hsv_2_col3(float h, float s, float v)
	{
		return hsv_2_col3(Vec3f(h, s, v));
	}

	inline Vec4c hsv_2_col4(const Vec3f& hsv, float a)
	{
		return Vec4c(hsv_2_col3(hsv), a);
	}

	inline Vec4c hsv_2_col4(float h, float s, float v, float a)
	{
		return Vec4c(hsv_2_col3(h, s, v), a);
	}

	inline Vec3f col_2_hsv(const Vec3c& rgb)
	{
		auto r = rgb.x() / 255.f;
		auto g = rgb.y() / 255.f;
		auto b = rgb.z() / 255.f;

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

		return Vec3f(h, cmax == 0.f ? 0.f : delta / cmax, cmax);
	}

	// Vec4f as Quat
	// (x, y, z) - vector, (w) - scalar

	// q = [sin(q /2)v, cos(q / 2)] (where q is an angle and v is an axis)
	template <class T>
	Vec<4, T> make_quat(float q, const Vec<3, T>& v)
	{
		auto rad = q * 0.5f * ANG_RAD;
		return normalize(Vec<4, T>(sin(rad) * v, cos(rad)));
	}

	template <class T>
	Vec<4, T> make_quat(const Mat<3, 3, T>& m)
	{
		T s;
		T tq[4];
		int i, j;
		// Use tq to store the largest trace
		tq[0] = T(1) + m[0][0] + m[1][1] + m[2][2];
		tq[1] = T(1) + m[0][0] - m[1][1] - m[2][2];
		tq[2] = T(1) - m[0][0] + m[1][1] - m[2][2];
		tq[3] = T(1) - m[0][0] - m[1][1] + m[2][2];
		// Find the maximum (could also use stacked if's later)
		j = 0;
		for (i = 1; i < 4; i++)
			j = (tq[i] > tq[j]) ? i : j;

		Vec<4, T> ret;
		// check the diagonal
		if (j == 0)
		{
			/* perform instant calculation */
			ret.w() = tq[0];
			ret.x() = m[1][2] - m[2][1];
			ret.y() = m[2][0] - m[0][2];
			ret.z() = m[0][1] - m[1][0];
		}
		else if (j == 1)
		{
			ret.w() = m[1][2] - m[2][1];
			ret.x() = tq[1];
			ret.y() = m[0][1] + m[1][0];
			ret.z() = m[2][0] + m[0][2];
		}
		else if (j == 2)
		{
			ret.w() = m[2][0] - m[0][2];
			ret.x() = m[0][1] + m[1][0];
			ret.y() = tq[2];
			ret.z() = m[1][2] + m[2][1];
		}
		else /* if (j==3) */
		{
			ret.w() = m[0][1] - m[1][0];
			ret.x() = m[2][0] + m[0][2];
			ret.y() = m[1][2] + m[2][1];
			ret.z() = tq[3];
		}
		ret *= sqrt(T(0.25) / tq[j]);
		return normalize(ret);
	}

	template <class T>
	Vec<4, T> quat_mul_nn(const Vec<4, T>& q1, const Vec<4, T>& q2)
	{
		Vec<4, T> ret;
		auto v1 = Vec<3, T>(q1);
		auto v2 = Vec<3, T>(q2);
		ret = Vec<4, T>(q1.w() * v2 + q2.w() * v1 + cross(v1, v2), q1.w() * q2.w() - dot(v1, v2));
		return ret;
	}
	
	template <class T>
	Vec<4, T> quat_mul(const Vec<4, T>& q1, const Vec<4, T>& q2)
	{
		return normalize(quat_mul_nn(q1, q2));
	}

	template <class T>
	Vec<3, T> quat_mul(const Vec<4, T>& q, const Vec<3, T>& v)
	{
		return Vec<3, T>(quat_mul_nn(quat_mul_nn(q, Vec<4, T>(v, T(0))), Vec4f(-q.x(), -q.y(), -q.z(), q.w())));
	}

	template <class T>
	Mat<3, 3, T> make_rotation_matrix(const Vec<4, T>& q)
	{
		T wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

		x2 = T(2) * q.x();
		y2 = T(2) * q.y();
		z2 = T(2) * q.z();

		xx = q.x() * x2;
		xy = q.x() * y2;
		xz = q.x() * z2;

		yy = q.y() * y2;
		yz = q.y() * z2;
		zz = q.z() * z2;

		wx = q.w() * x2;
		wy = q.w() * y2;
		wz = q.w() * z2;

		Mat<3, 3, T> ret;

		ret[0][0] = T(1) - (yy + zz);
		ret[1][0] = xy - wz;
		ret[2][0] = xz + wy;

		ret[0][1] = xy + wz;
		ret[1][1] = T(1) - (xx + zz);
		ret[2][1] = yz - wx;

		ret[0][2] = xz - wy;
		ret[1][2] = yz + wx;
		ret[2][2] = T(1) - (xx + yy);

		ret[0] = normalize(ret[0]);
		ret[1] = normalize(ret[1]);
		ret[2] = normalize(ret[2]);
		return ret;
	}

	// Vec3f as Euler:
	// (x, y z) - (yaw pitch roll)

	template <class T>
	Vec<3, T> eulerYPR(const Vec<4, T>& q)
	{
		Vec<3, T> ret;

		T sinr_cosp = 2 * (q.w() * q.x() + q.y() * q.z());
		T cosr_cosp = 1 - 2 * (q.x() * q.x() + q.y() * q.y());
		ret.z() = std::atan2(sinr_cosp, cosr_cosp); 

		T sinp = 2 * (q.w() * q.y() - q.z() * q.x());
		if (std::abs(sinp) >= 1)
			ret.y() = std::copysign(M_PI / 2, sinp); // use 90 degrees if out of range
		else
			ret.y() = std::asin(sinp);

		T siny_cosp = 2 * (q.w() * q.z() + q.x() * q.y());
		T cosy_cosp = 1 - 2 * (q.y() * q.y() + q.z() * q.z());
		ret.x() = std::atan2(siny_cosp, cosy_cosp);

		return ret;
	}

	template <class T>
	Mat<3, 3, T> rotation(const Vec<3, T>& e /* yaw pitch roll */)
	{
		Mat<3, 3, T> ret(T(1));

		Mat<3, 3, T> mat_yaw(ret[1], e.x() * ANG_RAD);
		ret[0] = mat_yaw * ret[0];
		ret[2] = mat_yaw * ret[2];
		Mat<3, 3, T> mat_pitch(ret[0], e.y() * ANG_RAD);
		ret[2] = mat_pitch * ret[2];
		ret[1] = mat_pitch * ret[1];
		Mat<3, 3, T> mat_roll(ret[2], e.z() * ANG_RAD);
		ret[1] = mat_roll * ret[1];
		ret[0] = mat_roll * ret[0];

		return ret;
	}

	// Vec4f as Plane:
	// (x, y, z) - normal, w - d

	template <class T>
	T plane_intersect(const Vec<4, T>& plane, const Vec<3, T>& origin, const Vec<3, T>& dir)
	{
		auto normal = Vec<3, T>(plane);
		auto numer = dot(normal, origin) - plane.w();
		auto denom = dot(normal, dir);

		if (abs(denom) < EPS)
			return -1.f;

		return -(numer / denom);
	}

	// Mat2x3 as AABB:
	// v_[0] - min, v_[1] - max

	template <class T>
	void AABB_offset(Mat<2, 3, T>& AABB, const Vec<3, T>& off)
	{
		AABB.v_[0] += off;
		AABB.v_[1] += off;
	}

	template <class T>
	void AABB_merge(Mat<2, 3, T>& AABB, const Vec<3, T>& p)
	{
		auto& v1 = AABB.v_[0], v2 = AABB.v_[1];
		v1.x() = min(v1.x(), p.x());
		v1.y() = min(v1.y(), p.y());
		v1.z() = min(v1.z(), p.z());
		v2.x() = max(v2.x(), p.x());
		v2.y() = max(v2.y(), p.y());
		v2.z() = max(v2.z(), p.z());
	}

	template <class T>
	void AABB_points(const Mat<2, 3, T>& AABB, Vec<3, T>* dst)
	{
		auto& v1 = AABB.v_[0], v2 = AABB.v_[1];
		dst[0] = v1;
		dst[1] = Vec<3, T>(v2.x(), v1.y(), v1.z());
		dst[2] = Vec<3, T>(v2.x(), v1.y(), v2.z());
		dst[3] = Vec<3, T>(v1.x(), v1.y(), v2.z());
		dst[4] = Vec<3, T>(v1.x(), v2.y(), v1.z());
		dst[5] = Vec<3, T>(v2.x(), v2.y(), v1.z());
		dst[6] = v2;
		dst[7] = Vec<3, T>(v1.x(), v2.y(), v2.z());
	}
}

