#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct FLAME_RU(cElement : Component, all)
	{
		inline static auto type_name = "cElement";
		inline static auto type_hash = ch(type_name);

		struct Drawer
		{
			virtual void draw(graphics::Canvas* canvas) = 0;
		};

		cElement() :
			Component(type_name, type_hash, true)
		{
		}

		virtual float get_x() const = 0;
		virtual void set_x(float x) = 0;

		virtual float get_y() const = 0;
		virtual void set_y(float y) = 0;

		virtual float get_width() const = 0;
		virtual void set_width(float w) = 0;

		virtual float get_height() const = 0;
		virtual void set_height(float h) = 0;

		// 0 - 1
		virtual float get_pivotx() const = 0;
		// 0 - 1
		virtual void set_pivotx(float p) = 0;

		// 0 - 1
		virtual float get_pivoty() const = 0;
		// 0 - 1
		virtual void set_pivoty(float p) = 0;

		virtual float get_scalex() const = 0;
		virtual void set_scalex(float s) = 0;

		virtual float get_scaley() const = 0;
		virtual void set_scaley(float s) = 0;

		virtual float get_rotation() const = 0;
		virtual void set_rotation(float angle) = 0;

		virtual float get_skewx() const = 0;
		virtual void set_skewx(float angle) = 0;

		virtual float get_skewy() const = 0;
		virtual void set_skewy(float angle) = 0;

		virtual Vec2f get_p00() = 0;
		virtual Vec2f get_p10() = 0;
		virtual Vec2f get_p11() = 0;
		virtual Vec2f get_p01() = 0;

		virtual Vec3c get_fill_color() = 0;
		virtual void set_fill_color(const Vec3c& c) = 0;

		virtual void mark_transform_dirty() = 0;
		virtual void mark_drawing_dirty() = 0;

		inline bool contains(const Vec2f& p)
		{
			Vec2f points[] = { get_p00(), get_p10(), get_p11(), get_p01() };
			return convex_contains<float>(p, points);
		}

		//FLAME_RV(Vec4f, padding); // L T R B
		//FLAME_RV(float, alpha);
		//FLAME_RV(Vec4f, roundness);
		//FLAME_RV(uint, roundness_lod);
		//FLAME_RV(float, frame_thickness);
		//FLAME_RV(Vec4c, frame_color);
		//FLAME_RV(ClipFlag, clip_flags, m);

		//bool clipped;
		//Vec4f clipped_rect;

		//void add_x(float v)
		//{
		//	set_pos(Vec2f(pos.x() + v, pos.y()));
		//}

		//void add_y(float v)
		//{
		//	set_pos(Vec2f(pos.x(), pos.y() + v));
		//}

		//void add_width(float v)
		//{
		//	set_size(Vec2f(size.x() + v, size.y()));
		//}

		//void add_height(float v)
		//{
		//	set_size(Vec2f(size.x(), size.y() + v));
		//}

		//void add_pos(const Vec2f& p)
		//{
		//	set_pos(pos + p);
		//}

		//void add_size(const Vec2f& p)
		//{
		//	set_size(size + p);
		//}

		//Vec2f center() const
		//{
		//	return global_pos + global_size * 0.5f;
		//}

		//Vec2f content_min() const
		//{
		//	return global_pos + padding.xy() * global_scale;
		//}

		//Vec2f content_max() const
		//{
		//	return global_pos + global_size - padding.zw() * global_scale;
		//}

		//Vec2f content_size() const
		//{
		//	return global_size - Vec2f(padding.xz().sum(), padding.yw().sum()) * global_scale;
		//}

		//void move_to(Schedule* s, float delay, float duration, const Vec2f& target)
		//{
		//	struct Capturing
		//	{
		//		Vec2f a;
		//		Vec2f b;
		//	}capture;
		//	capture.a = 0.f;
		//	capture.b = target;
		//	s->add_event(delay, duration, [](Capture& c, float time, float duration) {
		//		auto& capture = c.data<Capturing>();
		//		auto thiz = c.thiz<cElement>();
		//		if (time == -1.f)
		//		{
		//			capture.a = thiz->pos;
		//			return;
		//		}
		//		auto t = time / duration;
		//		thiz->set_pos(capture.a * (1.f - t) + capture.b * t);
		//	}, Capture().set_data(&capture).set_thiz(this));
		//}

		//void scale_to(Schedule* s, float delay, float duration, float target)
		//{
		//	struct Capturing
		//	{
		//		float a;
		//		float b;
		//	}capture;
		//	capture.a = 0.f;
		//	capture.b = target;
		//	s->add_event(delay, duration, [](Capture& c, float time, float duration) {
		//		auto& capture = c.data<Capturing>();
		//		auto thiz = c.thiz<cElement>();
		//		if (time == -1.f)
		//		{
		//			capture.a = thiz->scale;
		//			return;
		//		}
		//		auto t = time / duration;
		//		thiz->set_scale(capture.a * (1.f - t) + capture.b * t);
		//	}, Capture().set_data(&capture).set_thiz(this));
		//}

		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_alpha)(float a);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_roundness)(const Vec4f& r);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_frame_thickness)(float t);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_frame_color)(const Vec4c& c);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_clip_flags)(uint f);

		FLAME_UNIVERSE_EXPORTS static cElement* create();
	};
}
