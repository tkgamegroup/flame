#pragma once

#include "hud.h"

#include "../../graphics/canvas.h"

namespace flame
{
	struct HudLayout
	{
		HudLayoutType type;
		Rect rect;
		vec2 cursor;
		vec2 pivot;
		vec2 item_spacing;
		vec4 border;
		vec2 item_max;
		bool auto_size;
	};

	struct Hud
	{
		uint id;
		vec2 pos;
		vec2 size;
		cvec4 color;
		vec2 pivot;
		vec4 border;
		bool stencil;
		graphics::Canvas::DrawVert* bg_verts;
		uint bg_vert_count;
		int translate_cmd_idx;

		std::vector<HudLayout> layouts;
	};

	struct sHudPrivate : sHud
	{
		std::vector<std::stack<vec2>> style_vars;
		std::vector<std::stack<cvec4>> style_colors;
		std::vector<Hud> huds;
		Rect last_rect;

		uint current_modal = 0;
		uint modal_frames = 0;

		sHudPrivate();

		HudLayout& add_layout(HudLayoutType type);
		void finish_layout(HudLayout& layout);

		void begin(uint id, const vec2& pos, const vec2& size, const cvec4& col, const vec2& pivot, const graphics::ImageDesc& image, const vec4& border, bool is_modal) override;
		void end() override;
		vec2 get_cursor() override;
		void set_cursor(const vec2& pos) override;
		Rect wnd_rect() const override;
		Rect item_rect() const override;
		vec2 screen_size() const override;
		void push_style_var(HudStyleVar idx, const vec2& value) override;
		void pop_style_var(HudStyleVar idx) override;
		void push_style_color(HudStyleColor idx, const cvec4& color) override;
		void pop_style_color(HudStyleColor idx) override;
		void begin_layout(HudLayoutType type, const vec2& size, const vec2& item_spacing, const vec4& border) override;
		void end_layout() override;
		void new_line() override;
		void begin_stencil_write() override;
		void end_stencil_write() override;
		void begin_stencil_compare() override;
		void end_stencil_compare() override;
		Rect add_rect(const vec2& sz);
		void rect(const vec2& size, const cvec4& col) override;
		void text(std::wstring_view text, uint font_size, const cvec4& col) override;
		void image(const vec2& size, const graphics::ImageDesc& image, const cvec4& col) override;
		void image_stretched(const vec2& size, const graphics::ImageDesc& image, const vec4& border, const cvec4& col) override;
		void image_rotated(const vec2& size, const graphics::ImageDesc& image, const cvec4& col, float angle) override;
		bool button(std::wstring_view label, uint font_size) override;
		bool image_button(const vec2& size, const graphics::ImageDesc& image, const vec4& border) override;
		void stroke_item(float thickness, const cvec4& col) override;
		bool item_hovered() override;
		bool item_clicked() override;

		bool is_modal() override;

		void update() override;
	};
}
