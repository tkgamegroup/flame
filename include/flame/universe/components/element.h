#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct s2DRenderer;

	struct FLAME_R(cElement) : Component
	{
		s2DRenderer* renderer;

		FLAME_RV(Vec2f, pos, 0);
		Vec2f size;
		float scale;
		Vec2f pivot;
		Vec4f padding; // L T R B
		float alpha;
		Vec4f roundness;
		uint roundness_lod;
		float frame_thickness;
		Vec4c color;
		Vec4c frame_color;
		ClipFlags clip_flags;

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
			return padding[0] + padding[2];
		}

		float padding_v() const
		{
			return padding[1] + padding[3];
		}

		Vec2f center() const
		{
			return global_pos + global_size * 0.5f;
		}

		Vec2f content_min() const
		{
			return global_pos + Vec2f(padding[0], padding[1]) * global_scale;
		}

		Vec2f content_max() const
		{
			return global_pos + global_size - Vec2f(padding[2], padding[3]) * global_scale;
		}

		Vec2f content_size() const
		{
			return global_size - Vec2f(padding[0] + padding[2], padding[1] + padding[3]) * global_scale;
		}

		cElement() :
			Component("cElement")
		{
		}

		ListenerHub<bool(void* c, graphics::Canvas * canvas)> cmds;

		FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_pos)(const Vec2f& p, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_scale(float s, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_size(const Vec2f& s, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_alpha(float a, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_roundness(const Vec4f& r, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_frame_thickness(float t, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_color(const Vec4c& c, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_frame_color(const Vec4c& c, void* sender = nullptr);

		FLAME_UNIVERSE_EXPORTS static cElement* create();
	};
}
