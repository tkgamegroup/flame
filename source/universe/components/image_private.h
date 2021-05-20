#pragma once

#include "image.h"

namespace flame
{
	struct cImagePrivate : cImage
	{
		std::string src;

		vec2 uv0 = vec2(0.f);
		vec2 uv1 = vec2(1.f);
		cvec3 color = cvec3(255);

		cElementPrivate* element = nullptr;
		void* drawer = nullptr;
		void* measurable = nullptr;

		sRenderer* renderer = nullptr;

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
		void set_src(const char* src) override { set_src(std::string(src)); }

		vec4 get_uv() const override { return vec4(uv0, uv1); }
		void set_uv(const vec4& uv) override;

		void refresh_res() override;

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;

		bool measure(vec2* s);

		uint draw(uint layer, sRenderer* renderer);
	};
}
