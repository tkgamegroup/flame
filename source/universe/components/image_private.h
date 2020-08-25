#pragma once

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

	struct cImagePrivate : cImageBridge // R ~ on_*
	{
		int res_id = -1;
		int tile_id = -1;
		Vec2f uv0 = Vec2f(0.f);
		Vec2f uv1 = Vec2f(1.f);
		Vec3c color = Vec3c(255);

		std::string src;

		bool auto_size = true;

		cElementPrivate* element = nullptr; // R ref
		graphics::Canvas* canvas = nullptr; // R ref

		int get_res_id() const override { return res_id; }
		void set_res_id(int id) override;
		int get_tile_id() const override { return tile_id; }
		void set_tile_id(int id) override;

		const char* get_src() const override { return src.c_str(); }
		void set_src(const std::string& src);

		bool get_auto_size() const override { return auto_size; }
		void set_auto_size(bool a) override { auto_size = a; }

		void on_gain_canvas();

		void apply_src();

		void draw(graphics::Canvas* canvas); // R

		void measure(Vec2f& ret); // R
	};

	inline void cImageBridge::set_src(const char* src)
	{
		((cImagePrivate*)this)->set_src(src);
	}
}
