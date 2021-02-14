#pragma once

#include <flame/type.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
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
		vec2 LT;
		vec2 RB;

		Rect()
		{
			reset();
		}

		Rect(float LT_x, float LT_y, float RB_x, float RB_y)
		{
			LT.x = LT_x;
			LT.y = LT_y;
			RB.x = RB_x;
			RB.y = RB_y;
		}

		Rect(const vec2& LT, const vec2& RB) :
			Rect(LT.x, LT.y, RB.x, RB.y)
		{
		}

		void reset()
		{
			LT = vec2(10000.f);
			RB = vec2(-10000.f);
		}

		bool operator==(const Rect& rhs)
		{
			return LT.x == rhs.LT.x && LT.y == rhs.LT.y &&
				RB.x == rhs.RB.x && RB.y == rhs.RB.y;
		}

		void expand(float length)
		{
			LT.x -= length;
			LT.y += length;
			RB.x -= length;
			RB.y += length;
		}

		void expand(const vec2& p)
		{
			LT.x = min(LT.x, p.x);
			LT.y = min(LT.y, p.y);
			RB.x = max(RB.x, p.x);
			RB.y = max(RB.y, p.y);
		}

		bool contains(const vec2& p)
		{
			return p.x > LT.x && p.x < RB.x &&
				p.y > LT.y && p.y < RB.y;
		}

		bool overlapping(const Rect& rhs)
		{
			return !(rhs.LT.x > RB.x || rhs.LT.y > RB.y);
		}
	};
}

