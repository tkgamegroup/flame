#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct s2DRenderer;

	struct cElement : Component
	{
		s2DRenderer* renderer;

		Vec2f pos_;
		Vec2f size_;
		float scale_;
		Vec2f pivot_;
		Vec4f padding_; // L T R B
		float alpha_;
		Vec4f roundness_;
		uint roundness_lod;
		float frame_thickness_;
		Vec4c color_;
		Vec4c frame_color_;
		ClipFlags clip_flags;

		Vec2f global_pos;
		float global_scale;
		Vec2f global_size;
		bool clipped;
		Vec4f clipped_rect;

		float padding_h() const
		{
			return padding_[0] + padding_[2];
		}

		float padding_v() const
		{
			return padding_[1] + padding_[3];
		}

		Vec2f center() const
		{
			return global_pos + global_size * 0.5f;
		}

		Vec2f content_min() const
		{
			return global_pos + Vec2f(padding_[0], padding_[1]) * global_scale;
		}

		Vec2f content_max() const
		{
			return global_pos + global_size - Vec2f(padding_[2], padding_[3]) * global_scale;
		}

		Vec2f content_size() const
		{
			return global_size - Vec2f(padding_[0] + padding_[2], padding_[1] + padding_[3]) * global_scale;
		}

		cElement() :
			Component("cElement")
		{
		}

		ListenerHub<bool(void* c, graphics::Canvas * canvas)> cmds;

		FLAME_UNIVERSE_EXPORTS void set_x(float x, bool add = false, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_y(float y, bool add = false, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_pos(const Vec2f& p, bool add = false, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_scale(float s, bool add = false, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_width(float w, bool add = false, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_height(float h, bool add = false, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_size(const Vec2f& s, bool add = false, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_alpha(float a, bool add = false, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_roundness(const Vec4f& r, bool add = false, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_frame_thickness(float t, bool add = false, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_color(const Vec4c& c, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_frame_color(const Vec4c& c, bool add = false, void* sender = nullptr);

		FLAME_UNIVERSE_EXPORTS static cElement* create();
	};
}
