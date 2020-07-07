#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct FLAME_R(cElement : Component, all)
	{
		cElement() :
			Component("cElement", true)
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

		//FLAME_RV(Vec2f, pos);
		//FLAME_RV(Vec2f, size);
		//FLAME_RV(float, scale);
		//FLAME_RV(Vec2f, pivot);
		//FLAME_RV(Vec4f, padding); // L T R B
		//FLAME_RV(float, alpha);
		//FLAME_RV(Vec4f, roundness);
		//FLAME_RV(uint, roundness_lod);
		//FLAME_RV(float, frame_thickness);
		//FLAME_RV(Vec4c, color);
		//FLAME_RV(Vec4c, frame_color);
		//FLAME_RV(ClipFlag, clip_flags, m);

		//Vec2f global_pos;
		//float global_scale;
		//Vec2f global_size;
		//bool clipped;
		//Vec4f clipped_rect;

		//void set_x(float v)
		//{
		//	set_pos(Vec2f(v, pos.y()));
		//}

		//void set_y(float v)
		//{
		//	set_pos(Vec2f(pos.x(), v));
		//}

		//void set_width(float v)
		//{
		//	set_size(Vec2f(v, size.y()));
		//}

		//void set_height(float v)
		//{
		//	set_size(Vec2f(size.x(), v));
		//}
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

		//void mark_dirty()
		//{
		//	if (renderer)
		//		renderer->pending_update = true;
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

		//ListenerHub<bool(Capture& c, graphics::Canvas * canvas)> cmds;

		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_pos)(const Vec2f& p);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_scale)(float s);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_size)(const Vec2f& s);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_alpha)(float a);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_roundness)(const Vec4f& r);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_frame_thickness)(float t);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_color)(const Vec4c& c);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_frame_color)(const Vec4c& c);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_clip_flags)(uint f);

		FLAME_UNIVERSE_EXPORTS static cElement* FLAME_RF(create)();
	};
}
