#pragma once

#include "image.h"
#include "element_private.h"

namespace flame
{
	struct cImagePrivate : cImage, ElementDrawer, ElementMeasurer
	{
		std::filesystem::path src;
		std::string tile_name;

		vec2 uv0 = vec2(0.f);
		vec2 uv1 = vec2(1.f);
		cvec3 color = cvec3(255);

		cElementPrivate* element = nullptr;

		sRenderer* s_renderer = nullptr;

		int res_id = -1;
		int tile_id = -1;
		vec4 tile_uv;
		vec2 tile_sz;
		graphics::ImageView* iv = nullptr;
		graphics::ImageAtlas* atlas = nullptr;

		const wchar_t* get_src() const override { return src.c_str(); }
		void set_src(const std::filesystem::path& src);
		void set_src(const wchar_t* src) override { set_src(std::filesystem::path(src)); }
		const char* get_tile_name() const override { return tile_name.c_str(); }
		void set_tile_name(const std::string& name);
		void set_tile_name(const char* name) override { set_tile_name(std::string(name)); }

		vec4 get_uv() const override { return vec4(uv0, uv1); }
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
