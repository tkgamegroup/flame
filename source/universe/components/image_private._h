#pragma once

#include "image.h"
#include "element_private.h"

namespace flame
{
	struct cImagePrivate : cImage, ElementDrawer, ElementMeasurer
	{
		std::filesystem::path img_src;
		std::string tile_name;

		vec4 uvs = vec4(0.f, 0.f, 1.f, 1.f);
		cvec3 color = cvec3(255);

		cElementPrivate* element = nullptr;

		sRenderer* s_renderer = nullptr;

		int res_id = -1;
		int tile_id = -1;
		vec4 tile_uv;
		vec2 tile_sz;
		graphics::ImageView* iv = nullptr;
		graphics::ImageAtlas* atlas = nullptr;

		const wchar_t* get_img() const override { return img_src.c_str(); }
		void set_img(const std::filesystem::path& src);
		void set_img(const wchar_t* src) override { set_img(std::filesystem::path(src)); }
		const char* get_tile() const override { return tile_name.c_str(); }
		void set_tile(std::string_view name);
		void set_tile(const char* name) override { set_tile(std::string(name)); }

		vec4 get_uv() const override { return uvs; }
		void set_uv(const vec4& uv) override;

		void apply_src();

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;

		bool measure(vec2* s) override;

		uint draw(uint layer, sRendererPtr s_renderer) override;
	};
}
