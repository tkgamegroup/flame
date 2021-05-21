#pragma once

#include "type.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
using namespace glm;

namespace flame
{
	typedef vec<2, uchar> cvec2;
	typedef vec<3, uchar> cvec3;
	typedef vec<4, uchar> cvec4;

	template <class T, class ...Args>
	T minN(T a, T b, Args... args)
	{
		return minN(min(a, b), args...);
	}

	template <class T, class ...Args>
	T maxN(T a, T b, Args... rest)
	{
		return maxN(max(a, b), rest...);
	}

	inline float cross2(const vec2& a, const vec2& b)
	{
		return a.x * b.y - a.y * b.x;
	}

	inline uint image_pitch(uint b)
	{
		return (uint)ceil((b / 4.f)) * 4U;
	}

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

	union CommonValue
	{
		cvec4 c;
		ivec4 i;
		uvec4 u;
		vec4 f;
		void* p;
	};

	template <uint N>
	CommonValue common(const vec<N, uint, lowp>& v)
	{
		CommonValue cv;
		for (auto i = 0; i < N; i++)
			cv.c[i] = v[i];
		return cv;
	}

	template <uint N>
	CommonValue common(const vec<N, int, highp>& v)
	{
		CommonValue cv;
		for (auto i = 0; i < N; i++)
			cv.i[i] = v[i];
		return cv;
	}

	template <uint N>
	CommonValue common(const vec<N, uint, highp>& v)
	{
		CommonValue cv;
		for (auto i = 0; i < N; i++)
			cv.u[i] = v[i];
		return cv;
	}

	template <uint N>
	CommonValue common(const vec<N, float, highp>& v)
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

	inline float segment_intersect(const vec2& a, const vec2& b, const vec2& c, const vec2& d)
	{
		auto ab = b - a;
		auto dc = c - d;
		return cross2(ab, c - a) * cross2(ab, d - a) <= 0.f &&
			cross2(dc, a - d) * cross2(dc, b - d) <= 0.f;
	}

	inline bool convex_contains(const vec2& p, std::span<vec2> points)
	{
		if (points.size() < 3)
			return false;

		if (cross(vec3(p - points[0], 0), vec3(points[1] - p, 0)).z > 0.f)
			return false;
		if (cross(vec3(p - points[points.size() - 1], 0), vec3(points[0] - p, 0)).z > 0.f)
			return false;

		for (auto i = 1; i < points.size() - 1; i++)
		{
			if (cross(vec3(p - points[i], 0), vec3(points[i + 1] - p, 0)).z > 0.f)
				return false;
		}

		return true;
	}

	struct Rect
	{
		vec2 a;
		vec2 b;

		Rect() = default;

		Rect(float LT_x, float LT_y, float RB_x, float RB_y)
		{
			a.x = LT_x;
			a.y = LT_y;
			b.x = RB_x;
			b.y = RB_y;
		}

		Rect(const vec2& a, const vec2& b) :
			Rect(a.x, a.y, b.x, b.y)
		{
		}

		Rect(const vec4& v) :
			Rect(v.x, v.y, v.z, v.w)
		{
		}

		void reset()
		{
			a = vec2(10000.f);
			b = vec2(-10000.f);
		}

		operator vec4() const
		{
			return vec4(a.x, a.y, b.x, b.y);
		}

		bool operator==(const Rect& rhs)
		{
			return a.x == rhs.a.x && a.y == rhs.a.y &&
				b.x == rhs.b.x && b.y == rhs.b.y;
		}

		void expand(float length)
		{
			a.x -= length;
			a.y += length;
			b.x -= length;
			b.y += length;
		}

		void expand(const vec2& p)
		{
			a = min(a, p);
			b = max(b, p);
		}

		bool contains(const vec2& p)
		{
			return p.x > a.x && p.x < b.x &&
				p.y > a.y && p.y < b.y;
		}

		bool overlapping(const Rect& rhs)
		{
			return !(rhs.a.x > b.x || rhs.a.y > b.y);
		}
	};

	struct AABB
	{
		vec3 a;
		vec3 b;

		AABB() = default;

		AABB(const vec3 & a, const vec3& b) :
			a(a),
			b(b)
		{
		}

		AABB(const vec3& center, float size)
		{
			auto hf_size = size * 0.5f;
			a = center - hf_size;
			b = center + hf_size;
		}

		void reset()
		{
			a = vec3(10000.f);
			b = vec3(-10000.f);
		}

		vec3 center() const
		{
			return (a + b) * 0.5f;
		}

		void get_points(vec3* dst) const
		{
			dst[0] = vec3(a.x, a.y, a.z);
			dst[1] = vec3(b.x, a.y, a.z);
			dst[2] = vec3(a.x, a.y, b.z);
			dst[3] = vec3(a.x, b.y, a.z);
			dst[4] = vec3(a.x, b.y, b.z);
			dst[5] = vec3(b.x, b.y, a.z);
			dst[6] = vec3(b.x, a.y, b.z);
			dst[7] = vec3(b.x, b.y, b.z);
		}

		void expand(const vec3& p)
		{
			a = min(a, p);
			b = max(b, p);
		}

		void expand(const AABB& oth)
		{
			a = min(a, oth.a);
			b = max(b, oth.b);
		}

		bool contains(const vec3& p)
		{
			return all(greaterThan(p, a)) && all(greaterThan(b, p));
		}

		bool contains(const AABB& oth)
		{
			return contains(oth.a) && contains(oth.b);
		}

		bool intersects(const AABB& oth)
		{
			return any(greaterThan(oth.a, b)) || any(lessThan(oth.b, a));
		}
	};

	struct Plane
	{
		vec3 n;
		float d;

		Plane() = default;

		Plane(const vec3 &n, float d) :
			n(n),
			d(d)
		{
		}

		Plane(const vec3& a, const vec3& b, const vec3& c)
		{
			n = normalize(cross(b - a, c - a));
			d = -dot(n, a);
		}

		float distance(const vec3& p) const
		{
			return dot(n, p) - d;
		}
	};

	inline bool is_AABB_in_frustum(const Plane* planes, const AABB& bounds)
	{
		vec3 ps[8];
		bounds.get_points(ps);
		for (auto i = 0; i < 6; i++)
		{
			auto outside = true;
			for (auto j = 0; j < 8; j++)
			{
				if (planes[i].distance(ps[j]) > 0.f)
				{
					outside = false;
					break;
				}
			}
			if (outside)
				return false;
		}
		return true;
	}
}

