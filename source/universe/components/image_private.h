#pragma once

#include "../entity_private.h"
#include <flame/universe/components/image.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cElementPrivate;

	struct cImageBridge : cImage
	{
		void set_src(const char* src) override;
	};

	struct cImagePrivate : cImageBridge
	{
		std::string src;

		cElementPrivate* element = nullptr;
		graphics::Canvas* canvas = nullptr;

		vec2 uv0 = vec2(0.f);
		vec2 uv1 = vec2(1.f);
		cvec3 color = cvec3(255);

		int res_id = -1;
		int tile_id = -1;
		graphics::ImageView* iv = nullptr;
		graphics::ImageAtlas* atlas = nullptr;

		int get_res_id() const override { return res_id; }
		void set_res_id(int id) override;
		int get_tile_id() const override { return tile_id; }
		void set_tile_id(int id) override;

		const char* get_src() const override { return src.c_str(); }
		void set_src(const std::string& src);

		void apply_src();

		void on_gain_canvas();
		void on_lost_canvas();

		void measure(vec2& ret);

		void draw(graphics::Canvas* canvas);
	};

	inline void cImageBridge::set_src(const char* src)
	{
		((cImagePrivate*)this)->set_src(src);
	}
}
