#pragma once

#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct s2DRenderer;

	struct FLAME_R(cElement : Component)
	{
		s2DRenderer* renderer;

		FLAME_RV(Vec2f, pos);
		FLAME_RV(Vec2f, size);
		FLAME_RV(float, scale);
		FLAME_RV(Vec2f, pivot);
		FLAME_RV(Vec4f, padding); // L T R B
		FLAME_RV(float, alpha);
		FLAME_RV(Vec4f, roundness);
		FLAME_RV(uint, roundness_lod);
		FLAME_RV(float, frame_thickness);
		FLAME_RV(Vec4c, color);
		FLAME_RV(Vec4c, frame_color);
		FLAME_RV(ClipFlag, clip_flags, m);

		Vec2f global_pos;
		float global_scale;
		Vec2f global_size;
		bool clipped;
		Vec4f clipped_rect;

		void set_x(float v, bool add = false, void* sender = nullptr)
		{
			auto t = pos;
			t.x() = add ? t.x() + v : v;
			set_pos(t, sender);
		}

		void set_y(float v, bool add = false, void* sender = nullptr)
		{
			auto t = pos;
			t.y() = add ? t.y() + v : v;
			set_pos(t, sender);
		}

		void set_width(float v, bool add = false, void* sender = nullptr)
		{
			auto t = size;
			t.x() = add ? t.x() + v : v;
			set_size(t, sender);
		}

		void set_height(float v, bool add = false, void* sender = nullptr)
		{
			auto t = size;
			t.y() = add ? t.y() + v : v;
			set_size(t, sender);
		}

		void add_pos(const Vec2f& p, void* sender = nullptr)
		{
			set_pos(pos + p, sender);
		}

		void add_size(const Vec2f& p, void* sender = nullptr)
		{
			set_size(size + p, sender);
		}

		float padding_h() const
		{
			return padding.xz().sum();
		}

		float padding_v() const
		{
			return padding.yw().sum();
		}

		Vec2f padding_hv() const
		{
			return Vec2f(padding_h(), padding_v());
		}

		Vec2f center() const
		{
			return global_pos + global_size * 0.5f;
		}

		Vec2f content_min() const
		{
			return global_pos + padding.xy() * global_scale;
		}

		Vec2f content_max() const
		{
			return global_pos + global_size - padding.zw() *global_scale;
		}

		Vec2f content_size() const
		{
			return global_size - Vec2f(padding.xz().sum(), padding.yw().sum()) * global_scale;
		}

		void mark_dirty()
		{
			if (renderer)
				renderer->pending_update = true;
		}

		cElement() :
			Component("cElement")
		{
		}

		ListenerHub<bool(void* c, graphics::Canvas * canvas)> cmds;

		FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_pos)(const Vec2f& p, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_scale)(float s, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_size)(const Vec2f& s, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_alpha)(float a, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_roundness)(const Vec4f& r, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_frame_thickness)(float t, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_color)(const Vec4c& c, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_frame_color)(const Vec4c& c, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_clip_flags)(uint f, void* sender = nullptr);

		FLAME_UNIVERSE_EXPORTS static void set_current(cElement* e);
		FLAME_UNIVERSE_EXPORTS static cElement* FLAME_RF(current)();
		FLAME_UNIVERSE_EXPORTS static cElement* FLAME_RF(create)();
	};
}
