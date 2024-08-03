#pragma once

#include "type.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/color_space.hpp>
#include <glm/gtx/easing.hpp>

using namespace glm;

namespace flame
{
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

	typedef vec<2, uchar> cvec2;
	typedef vec<3, uchar> cvec3;
	typedef vec<4, uchar> cvec4;

	// short variant
	union sVariant
	{
		cvec4 c;
		int i;
		uint u;
		float f;
	};

	// long variant
	union lVariant
	{
		cvec4 c;
		ivec4 i;
		uvec4 u;
		vec4 f;
		void* p;
	};

	inline float map_01(float x, float v0, float v1)
	{
		return (x - v0) / (v1 - v0);
	}

	inline vec2 map_01(vec2 x, float v0, float v1)
	{
		return vec2(map_01(x.x, v0, v1), map_01(x.y, v0, v1));
	}

	inline vec3 map_01(vec3 x, float v0, float v1)
	{
		return vec3(map_01(x.x, v0, v1), map_01(x.y, v0, v1), map_01(x.z, v0, v1));
	}

	inline vec4 map_01(vec4 x, float v0, float v1)
	{
		return vec4(map_01(x.x, v0, v1), map_01(x.y, v0, v1), map_01(x.z, v0, v1), map_01(x.w, v0, v1));
	}

	inline int log2i(int v)
	{
		int ret = 0;
		while (v >>= 1) ++ret;
		return ret;
	}

	template<uint N, typename T>
	inline T sum(const vec<N, T>& v)
	{
		T ret = 0;
		for (auto i = 0; i < N; i++)
			ret += v[i];
		return ret;
	}

	template<uint N, typename T>
	inline T product(const vec<N, T>& v)
	{
		T ret = v[0];
		for (auto i = 1; i < N; i++)
			ret *= v[i];
		return ret;
	}

	template<typename T, typename ...Args>
	T minN(T a, T b, Args... args)
	{
		return minN(min(a, b), args...);
	}

	template<typename T, typename ...Args>
	T maxN(T a, T b, Args... rest)
	{
		return maxN(max(a, b), rest...);
	}

	inline float square(float v)
	{
		return v * v;
	}

	inline float sign_min(float a, float b)
	{
		return sign(a) * min(abs(a), abs(b));
	}

	template<typename T>
	T weighted_random(const std::vector<std::pair<T, uint>>& cands)
	{
		int sum_of_weight = 0;
		for (int i = 0; i < cands.size(); i++) {
			sum_of_weight += cands[i].second;
		}
		int rnd = linearRand(0, sum_of_weight - 1);
		for (int i = 0; i < cands.size(); i++)
		{
			if (rnd < cands[i].second)
				return cands[i].first;
			rnd -= cands[i].second;
		}
	}

	inline float cross2(const vec2& a, const vec2& b)
	{
		return a.x * b.y - a.y * b.x;
	}

	inline float angle(const vec2& d)
	{
		return -degrees(atan2(d.y, d.x));
	}

	inline float angle_xz(const vec3& d)
	{
		return angle(d.xz());
	}

	inline float angle_xz(const vec3& p0, const vec3& p1)
	{
		return angle_xz(p1 - p0);
	}

	inline vec2 dir(float ang)
	{
		auto rad = radians(ang);
		return vec2(cos(rad), -sin(rad));
	}

	inline vec3 dir_xz(float ang)
	{
		vec3 ret = vec3(dir(ang), 0.f);
		return ret.xzy();
	}

	inline float angle_diff(float ang0, float ang1)
	{
		ang0 = mod(ang0, 360.f); if (ang0 < 0.f) ang0 += 360.f;
		ang1 = mod(ang1, 360.f); if (ang1 < 0.f) ang1 += 360.f;
		auto diff1 = ang0 - ang1; if (diff1 < 0.f) diff1 += 360.f;
		auto diff2 = ang1 - ang0; if (diff2 < 0.f) diff2 += 360.f;
		if (diff1 < diff2)
			return -diff1;
		return diff2;
	}

	inline void dist_ang_diff(const vec3& pos0, const vec3& pos1, float ang0, float& dist, float& ang_diff)
	{
		auto d = pos1 - pos0;
		dist = length(d);
		ang_diff = angle_diff(ang0, angle_xz(d));
	}

	inline vec2 arc_point(const vec2& center, float angle, float radius)
	{
		auto rad = radians(angle);
		return center + vec2(cos(rad) * radius, sin(rad) * radius);
	}

	inline float triangle_area(const vec2& a, const vec2& b, const vec2& c)
	{
		return ((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)) / 2.f;
	}

	inline vec2 convex_centroid(std::span<vec2> points)
	{
		vec2 ret(0.f); float area = 0.f;
		if (points.size() < 3)
			return ret;

		for (auto i = 1; i < points.size() - 1; i++)
		{
			auto m = (points[0] + points[i] + points[i + 1]) / 3.f;
			auto a = triangle_area(points[0], points[1], points[2]);
			ret = (ret * area + m * a) / (area + a);
			area += a;
		}

		return ret;
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

	inline float segment_intersect(const vec2& a, const vec2& b, const vec2& c, const vec2& d)
	{
		auto ab = b - a;
		auto dc = c - d;
		return cross2(ab, c - a) * cross2(ab, d - a) <= 0.f &&
			cross2(dc, a - d) * cross2(dc, b - d) <= 0.f;
	}

	// line segment start, line segment end, circle center, circle rarius
	inline bool segment_circle_overlap(const vec2& p, const vec2& q, const vec2& o, float r)
	{
		auto pq = q - p; auto op = p - o; auto oq = q - o;
		if (dot(op, pq) < 0.f && dot(oq, pq) > 0.f)
			return abs(cross(vec3(pq, 0.f), vec3(op, 0.f)).z) / length(pq) < r;
		return sqrt(min(dot(op, op), dot(oq, oq))) < r;
	}

	// circle center, circle rarius, sector center, sector radius start, sector radius end, sector half central angle, sector direction angle
	inline bool circle_sector_overlap(const vec2& co, float cr, const vec2& so, float sr0, float sr1, float sa, float sd)
	{
		auto vec_two_circles = co - so;
		auto dist_two_circles = length(vec_two_circles);
		if (dist_two_circles > cr + sr1 || dist_two_circles < sr0 - cr)
			return false;
		if (abs(angle_diff(angle(vec_two_circles), sd)) < sa)
			return true;
		auto dir0 = dir(-sd + sa); auto dir1 = dir(-sd - sa);
		if (segment_circle_overlap(so + dir0 * sr0, so + dir0 * sr1, co, cr) ||
			segment_circle_overlap(so + dir1 * sr0, so + dir1 * sr1, co, cr))
			return true;
		return false;
	}

	inline bool oriented_box_circle_overlap(const vec2& bc, const vec2& be, float ba, const vec2& co, float cr)
	{
		auto d = dir(-ba);
		auto p0 = bc + d * be.x;
		auto p1 = bc - d * be.x;
		auto p2 = bc + d * be.y;
		auto p3 = bc - d * be.y;
		if (segment_circle_overlap(p0, p1, co, cr) ||
			segment_circle_overlap(p1, p2, co, cr) ||
			segment_circle_overlap(p2, p3, co, cr) ||
			segment_circle_overlap(p3, p0, co, cr))
			return true;
		return false;
	}

	template <uint N>
	inline void make_line_strips(const vec<N, float>& pt0, const vec<N, float>& pt1, std::vector<vec<N, float>>& strips)
	{
		constexpr auto SEPARATOR = std::numeric_limits<float>::quiet_NaN();

		auto it0 = std::find_if(strips.begin(), strips.end(), [&](const auto& i) {
			return distance(i, pt0) < 0.01f;
		});
		auto it1 = std::find_if(strips.begin(), strips.end(), [&](const auto& i) {
			return distance(i, pt1) < 0.01f;
		});

		if (it0 != strips.end() && it1 != strips.end())
		{
			if (it1 == strips.begin())
				strips.push_back(strips[0]);
			else
			{
				auto n = 0;
				for (auto it = it1; it != strips.end(); it++)
				{
					if (isnan(it->x))
						break;
					n++;
				}

				auto copied = std::vector<vec<N, float>>(it1, it1 + n);
				{
					auto b = it1;
					auto e = it1 + n;
					if (b != strips.begin())
					{
						if (isnan((*(b - 1)).x))
							b = b - 1;
					}
					strips.erase(b, e);
				}
				it0 = std::find_if(strips.begin(), strips.end(), [&](const auto& i) {
					return distance(i, pt0) < 0.01f;
				});
				if (it0 == strips.end())
					int cut = 1; // why?
				else
					strips.insert(it0 + 1, copied.begin(), copied.end());
			}
		}
		else if (it0 != strips.end())
			strips.emplace(it0 + 1, pt1);
		else if (it1 != strips.end())
			strips.emplace(it1, pt0);
		else
		{
			if (!strips.empty())
				strips.push_back(vec<N, float>(SEPARATOR));
			strips.push_back(pt0);
			strips.push_back(pt1);
		}
	}

	inline vec3 smooth_damp(const vec3& current, const vec3& _target, vec3& current_velocity, float smooth_time, float max_speed, float delta_time)
	{
		auto target = _target;

		vec3 output(0.f);

		// Based on Game Programming Gems 4 Chapter 1.10
		smooth_time = max(0.0001f, smooth_time);
		float omega = 2.f / smooth_time;

		float x = omega * delta_time;
		float exp = 1.f / (1.f + x + 0.48f * x * x + 0.235f * x * x * x);

		vec3 change(current - target);
		auto original_to = target;

		// Clamp maximum speed
		float max_change = max_speed * smooth_time;

		float max_change_sq = max_change * max_change;
		float sqrmag = dot(change, change);
		if (sqrmag > max_change_sq)
			change = change / sqrt(sqrmag) * max_change;

		target = current - change;

		vec3 temp((current_velocity + omega * change) * delta_time);

		current_velocity = (current_velocity - omega * temp) * exp;

		output = target + (change + temp) * exp;

		// Prevent overshooting
		vec3 origMinusCurrent = original_to - current;
		vec3 outMinusOrig = output - original_to;

		if (dot(origMinusCurrent, outMinusOrig) > 0)
		{
			output = original_to;
			current_velocity = (output - original_to) / delta_time;
		}

		return output;
	}

	struct Rect
	{
		vec2 a;
		vec2 b;

		Rect()
		{
			reset();
		}

		Rect(float LT_x, float LT_y, float RB_x, float RB_y) :
			a(LT_x, LT_y),
			b(RB_x, RB_y)
		{
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
			a = vec2(+10000.f);
			b = vec2(-10000.f);
		}

		bool invalid() const
		{
			return any(greaterThan(a, b));
		}

		operator vec4() const
		{
			return vec4(a.x, a.y, b.x, b.y);
		}

		vec2 size() const
		{
			return b - a;
		}

		void expand(float length)
		{
			a.x -= length;
			a.y -= length;
			b.x += length;
			b.y += length;
		}

		void expand(const vec2& p)
		{
			a = min(a, p);
			b = max(b, p);
		}

		void expand(const Rect& oth)
		{
			a = min(a, oth.a);
			b = max(b, oth.b);
		}

		void offset(const vec2& p)
		{
			a += p;
			b += p;
		}

		bool contains(const vec2& p) const
		{
			return p.x > a.x && p.x < b.x &&
				p.y > a.y && p.y < b.y;
		}

		bool contains(const Rect& oth) const
		{
			return oth.a.x > a.x && oth.a.y > a.y &&
				oth.b.x < b.x && oth.b.y < b.y;
		}

		bool overlapping(const Rect& rhs) const
		{
			return !(rhs.a.x > b.x || rhs.a.y > b.y || rhs.b.x < a.x || rhs.b.y < a.y);
		}

		std::vector<vec2> get_points() const
		{
			std::vector<vec2> ret;
			ret.resize(4);
			ret[0] = a;
			ret[1] = vec2(a.x, b.y);
			ret[2] = b;
			ret[3] = vec2(b.x, a.y);
			return ret;
		}
	};

	inline bool operator==(const Rect& lhs, const Rect& rhs)
	{
		return lhs.a.x == rhs.a.x && lhs.a.y == rhs.a.y &&
			lhs.b.x == rhs.b.x && lhs.b.y == rhs.b.y;
	}

	struct AABB
	{
		vec3 a;
		vec3 b;

		AABB()
		{
			reset();
		}

		AABB(const vec3& a, const vec3& b) :
			a(a),
			b(b)
		{
		}

		AABB(const vec3& center, float size) :
			a(center - size),
			b(center + size)
		{
		}

		AABB(uint count, const vec3* points, const mat3& mat = mat3(1.f))
		{
			reset();
			for (auto i = 0; i < count; i++)
				expand(mat * points[i]);
		}

		AABB(const std::vector<vec3>& points, const mat3& mat = mat3(1.f)) :
			AABB(points.size(), points.data(), mat)
		{
		}

		void reset()
		{
			a = vec3(+10000.f);
			b = vec3(-10000.f);
		}

		bool invalid() const
		{
			return any(greaterThan(a, b));
		}

		vec3 center() const
		{
			return (a + b) * 0.5f;
		}

		float radius() const
		{
			return distance(a, b) * 0.5f;
		}

		void get_points(vec3* dst, const mat4& m = mat4(1.f)) const
		{
			dst[0] = m * vec4(a.x, a.y, a.z, 1.f);
			dst[1] = m * vec4(b.x, a.y, a.z, 1.f);
			dst[2] = m * vec4(b.x, a.y, b.z, 1.f);
			dst[3] = m * vec4(a.x, a.y, b.z, 1.f);
			dst[4] = m * vec4(a.x, b.y, a.z, 1.f);
			dst[5] = m * vec4(b.x, b.y, a.z, 1.f);
			dst[6] = m * vec4(b.x, b.y, b.z, 1.f);
			dst[7] = m * vec4(a.x, b.y, b.z, 1.f);
		}

		std::vector<vec3> get_points(const mat4& m = mat4(1.f)) const
		{
			std::vector<vec3> ret;
			ret.resize(8);
			get_points(ret.data(), m);
			return ret;
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

		bool contains(const vec3& p) const
		{
			return all(greaterThan(p, a)) && all(greaterThan(b, p));
		}

		bool contains(const AABB& oth) const
		{
			return contains(oth.a) && contains(oth.b);
		}

		bool intersects(const AABB& oth) const
		{
			return !(any(greaterThan(oth.a, b)) || any(lessThan(oth.b, a)));
		}

		bool intersects(const vec3& center, float radius) const
		{
			auto d = 0.f;
			if		(center.x < a.x) d += square(center.x - a.x);
			else if (center.x > b.x) d += square(center.x - b.x);
			if		(center.y < a.y) d += square(center.y - a.y);
			else if (center.y > b.y) d += square(center.y - b.y);
			if		(center.z < a.z) d += square(center.z - a.z);
			else if (center.z > b.z) d += square(center.z - b.z);
			return square(radius) > d;
		}

		bool intersects(const vec2& center, float radius) const
		{
			auto d = 0.f;
			if		(center.x < a.x) d += square(center.x - a.x);
			else if (center.x > b.x) d += square(center.x - b.x);
			if		(center.y < a.z) d += square(center.y - a.z);
			else if (center.y > b.z) d += square(center.y - b.z);
			return square(radius) > d;
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

		Plane(const vec3& n, const vec3& p) :
			n(n),
			d(-dot(n, p))
		{
		}

		Plane(const vec3& a, const vec3& b, const vec3& c) :
			n(normalize(cross(b - a, c - a))),
			d(-dot(n, a))
		{
		}

		float distance(const vec3& p) const
		{
			return dot(n, p) + d;
		}

		bool intersects(const vec3& segment_p0, const vec3& segment_p1, vec3* out_p = nullptr, float* out_t = nullptr) const
		{
			auto seg_dir = segment_p1 - segment_p0;
			auto dot_plane_seg = abs(dot(n, seg_dir));
			if (dot_plane_seg < 1e-6)
				return false;

			float t = abs(dot(n, segment_p0) + d) / dot_plane_seg;
			if (t < 0.f || t > 1.f)
				return false;

			if (out_p)
				*out_p = segment_p0 + t * seg_dir;
			if (out_t)
				*out_t = t;

			return true;
		}
	};

	struct Frustum
	{
		Plane planes[6];

		Frustum() = default;

		void set(const vec3* points)
		{
			planes[0] = Plane(points[0], points[1], points[2]); // near
			planes[1] = Plane(points[4], points[7], points[5]); // far
			planes[2] = Plane(points[3], points[7], points[4]); // left
			planes[3] = Plane(points[1], points[5], points[6]); // right
			planes[4] = Plane(points[0], points[4], points[5]); // top
			planes[5] = Plane(points[2], points[6], points[7]); // bottom
		}

		static std::vector<vec3> get_points(const mat4& inv, float n = 0.f, float f = 1.f)
		{
			std::vector<vec3> ret;
			ret.resize(8);
			auto trans_point = [&](const vec3& p) {
				auto ret = inv * vec4(p, 1.f);
				ret /= ret.w;
				return ret;
			};
			ret[0] = trans_point(vec3(-1.f, -1.f, n));
			ret[1] = trans_point(vec3(+1.f, -1.f, n));
			ret[2] = trans_point(vec3(+1.f, +1.f, n));
			ret[3] = trans_point(vec3(-1.f, +1.f, n));
			ret[4] = trans_point(vec3(-1.f, -1.f, f));
			ret[5] = trans_point(vec3(+1.f, -1.f, f));
			ret[6] = trans_point(vec3(+1.f, +1.f, f));
			ret[7] = trans_point(vec3(-1.f, +1.f, f));
			return ret;
		}

		static std::vector<vec3> points_to_lines(const vec3* points)
		{
			std::vector<vec3> ret;
			ret.resize(24);
			auto p = ret.data();
			*p++ = points[0]; *p++ = points[1];
			*p++ = points[1]; *p++ = points[2];
			*p++ = points[2]; *p++ = points[3];
			*p++ = points[3]; *p++ = points[0];
			*p++ = points[0]; *p++ = points[4];
			*p++ = points[1]; *p++ = points[5];
			*p++ = points[2]; *p++ = points[6];
			*p++ = points[3]; *p++ = points[7];
			*p++ = points[4]; *p++ = points[5];
			*p++ = points[5]; *p++ = points[6];
			*p++ = points[6]; *p++ = points[7];
			*p++ = points[7]; *p++ = points[4];
			return ret;
		}

		static std::vector<vec3> points_to_quads(const vec3* points)
		{
			std::vector<vec3> ret;
			ret.resize(24);
			auto p = ret.data();
			*p++ = points[0]; *p++ = points[1]; *p++ = points[2]; *p++ = points[3];
			*p++ = points[5]; *p++ = points[4]; *p++ = points[7]; *p++ = points[6];
			*p++ = points[4]; *p++ = points[0]; *p++ = points[3]; *p++ = points[7];
			*p++ = points[1]; *p++ = points[5]; *p++ = points[6]; *p++ = points[2];
			*p++ = points[3]; *p++ = points[2]; *p++ = points[6]; *p++ = points[7];
			*p++ = points[4]; *p++ = points[5]; *p++ = points[1]; *p++ = points[0];
			return ret;
		}

		Frustum(const vec3* points)
		{
			set(points);
		}

		Frustum(const mat4& inv)
		{
			set(get_points(inv).data());
		}
	};

	inline bool AABB_frustum_check(const Frustum& frustum, const AABB& bounds)
	{
		vec3 ps[8];
		bounds.get_points(ps);
		for (auto i = 0; i < 6; i++)
		{
			auto outside = true;
			for (auto j = 0; j < 8; j++)
			{
				if (frustum.planes[i].distance(ps[j]) > 0.f)
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

	inline vec3 fit_camera_to_object(const mat3& camera_rotation, float fovy, float zNear, float aspect, const AABB& object_bounds)
	{
		auto transformed_bounds = AABB(object_bounds.get_points(inverse(camera_rotation)));
		auto xext = transformed_bounds.b.x - transformed_bounds.a.x;
		auto yext = transformed_bounds.b.y - transformed_bounds.a.y;
		if (xext / aspect > yext)
			yext = xext / aspect;
		auto l = (yext * 0.5f) / tan(radians(fovy * 0.5f)) + (transformed_bounds.b.z - transformed_bounds.a.z) * 0.5f;
		l = max(l, zNear + 0.05f);
		return object_bounds.center() + camera_rotation[2] * l;
	}

	template <uint N>
	struct Curve
	{
		float segment_length = 1.f; // how many segments per unit
		float curvedness = 0.5f;	// how much do the curve along the normals
		std::vector<vec<N, float>> ctrl_points;
		std::vector<vec<N, float>> vertices;

		inline static vec<N, float> interpolate(const vec<N, float>& p0, const vec<N, float>& p1, const vec<N, float>& p2, const vec<N, float>& p3, float u, float curvedness)
		{
			auto c0 = p1;
			auto c1 = -curvedness * p0 + curvedness * p2;
			auto c2 = 2.f * curvedness * p0 + (curvedness - 3.f) * p1 + (3.f - 2.f * curvedness) * p2 + -curvedness * p3;
			auto c3 = -curvedness * p0 + (2.f - curvedness) * p1 + (curvedness - 2.f) * p2 + curvedness * p3;
			return c0 + c1 * u + c2 * u * u + c3 * u * u * u;
		}

		inline void update()
		{
			vertices.clear();
			if (ctrl_points.size() < 2)
			{
				vertices = ctrl_points;
				return;
			}

			std::vector<vec<N, float>> _ctrl_points;
			_ctrl_points.push_back(2.f * ctrl_points[0] - ctrl_points[1]);
			_ctrl_points.insert(_ctrl_points.end(), ctrl_points.begin(), ctrl_points.end());
			_ctrl_points.push_back(2.f * ctrl_points.rbegin()[0] - ctrl_points.rbegin()[1]);
			for (auto i = 2; i < _ctrl_points.size() - 1; i++)
			{
				auto pi_2 = _ctrl_points[i - 2];
				auto pi_1 = _ctrl_points[i - 1];
				auto pi = _ctrl_points[i];
				auto pi1 = _ctrl_points[i + 1];

				auto c0 = pi_1;
				auto c1 = -curvedness * pi_2 + curvedness * pi;
				auto c2 = 2.f * curvedness * pi_2 + (curvedness - 3.f) * pi_1 + (3.f - 2.f * curvedness) * pi + -curvedness * pi1;
				auto c3 = -curvedness * pi_2 + (2.f - curvedness) * pi_1 + (curvedness - 2.f) * pi + curvedness * pi1;

				auto segments = max(1U, uint(distance(pi_1, pi) / segment_length));
				auto num = segments + (i == _ctrl_points.size() - 2 ? 1 : 0);
				for (auto j = 0; j < num; j++)
				{
					auto u = (float)j / (float)segments;
					vertices.push_back(c0 + c1 * u + c2 * u * u + c3 * u * u * u);
				}
			}
		}
	};

	struct PerspectiveProjector
	{
		vec2 screen_sz;
		float aspect;
		float fovy;
		float zNear;
		float zFar;
		float tanfovy;

		mat4 matp;

		void set(const vec2& sz, float fov, float n, float f)
		{
			screen_sz = sz;
			aspect = screen_sz.x / screen_sz.y;
			fovy = radians(fov);
			zNear = n;
			zFar = f;
			tanfovy = tan(fovy);

			matp = perspective(fovy, aspect, zNear, zFar);
		}

		vec2 proj(const vec3& p)
		{
			auto pp = matp * vec4(p, 1.f);
			pp /= pp.w;
			return (pp.xy() * 0.5f + 0.5f) * screen_sz;
		}

	};

	using basic_math_types = type_list<ivec2, ivec3, ivec4, uvec2, uvec3, uvec4, cvec2, cvec3, cvec4, vec2, vec3, vec4, mat2, mat3, mat4, quat, 
		Rect, AABB, Plane, Frustum, Curve<1>, Curve<2>, Curve<3>, Curve<4>>;

	template<typename T>
	concept basic_math_type = is_one_of_t<T>(basic_math_types());

	inline uint image_pitch(uint b)
	{
		return (uint)ceil((b / 4.f)) * 4U;
	}
}

