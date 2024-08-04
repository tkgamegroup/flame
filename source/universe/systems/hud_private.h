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
		vec2 spacing;
		vec4 border;
		vec2 item_max;
		bool auto_size;
	};

	struct Hud
	{
		uint id;
		vec2 pos;
		vec2 size;
		vec4 border;
		vec2 suggested_size = vec2(0.f);

		std::vector<HudLayout> layouts;
	};

	struct sHudPrivate : sHud
	{
		graphics::WindowPtr bound_window = nullptr;
		sInputPtr input = nullptr;

		std::vector<std::stack<vec4>>					style_vars;
		std::vector<std::stack<cvec4>>					style_colors;
		std::vector<std::stack<graphics::ImageDesc>>	style_images;
		std::stack<bool>								enables;
		std::unordered_map<uint, Hud> huds;
		Hud* last_hud = nullptr;
		Rect last_rect;

		uint current_modal = 0;
		uint modal_frames = 0;

		sHudPrivate();
		sHudPrivate(int); // dummy

		HudLayout& add_layout(HudLayoutType type);
		void finish_layout(HudLayout& layout);

		void bind_window(graphics::WindowPtr window) override;

		void begin(uint id, const vec2& pos, const vec2& size, const vec2& pivot, bool is_modal) override;
		void begin_popup() override;
		void end() override;
		vec2 get_cursor() override;
		void set_cursor(const vec2& pos) override;
		Rect wnd_rect() const override;
		Rect item_rect() const override;
		void stroke_item(float thickness, const cvec4& col) override;
		bool item_hovered() override;
		bool item_clicked() override;
		bool is_modal() override;
		void push_style_var(HudStyleVar idx, const vec4& value) override;
		void pop_style_var(HudStyleVar idx) override;
		void push_style_color(HudStyleColor idx, const cvec4& color) override;
		void pop_style_color(HudStyleColor idx) override;
		void push_style_image(HudStyleImage idx, const graphics::ImageDesc& image) override;
		void pop_style_image(HudStyleImage idx) override;
		void push_enable(bool v) override;
		void pop_enable() override;
		void begin_layout(HudLayoutType type, const vec2& size) override;
		void end_layout() override;
		void newline() override;
		void begin_stencil_write() override;
		void end_stencil_write() override;
		void begin_stencil_compare() override;
		void end_stencil_compare() override;
		Rect add_rect(const vec2& sz);
		void rect(const vec2& size, const cvec4& col) override;
		void text(std::wstring_view text) override;
		void image(const vec2& size, const graphics::ImageDesc& image) override;
		void image_stretched(const vec2& size, const graphics::ImageDesc& image) override;
		void image_rotated(const vec2& size, const graphics::ImageDesc& image, float angle) override;
		bool button(std::wstring_view label) override;
		void progress_bar(const vec2& size, float progress, const cvec4& color, const cvec4& background_color, std::wstring_view label) override;

		void start() override;
		void update() override;
		void render(int idx, graphics::CommandBufferPtr cb) override;
	};
}
