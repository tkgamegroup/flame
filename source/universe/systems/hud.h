#pragma once

#include "../../graphics/image.h"
#include "../system.h"

namespace flame
{
	enum HudLayoutType
	{
		HudVertical,
		HudHorizontal
	};

	enum HudStyleVar
	{
		HudStyleVarScaling,
		HudStyleVarAlpha,
		HudStyleVarCount
	};

	enum HudStyleColor
	{
		HudStyleColorButton,
		HudStyleColorButtonHovered,
		HudStyleColorCount
	};

	// Reflect ctor
	struct sHud : System
	{
		graphics::CanvasPtr canvas = nullptr;

		virtual void begin(const vec2& pos, const vec2& size = vec2(0.f) /* 0 size means auto layout */, const cvec4& col = cvec4(0, 0, 0, 255), const vec2& pivot = vec2(0.f),
			const graphics::ImageDesc& image = {}, const vec4& border = vec4(4.f)) = 0;
		virtual void end() = 0;
		virtual vec2 get_cursor() = 0;
		virtual void set_cursor(const vec2& pos) = 0;
		virtual Rect wnd_rect() const = 0;
		virtual Rect item_rect() const = 0;
		virtual vec2 screen_size() const = 0;
		virtual void push_style_var(HudStyleVar idx, const vec2& value) = 0;
		virtual void pop_style_var(HudStyleVar idx) = 0;
		virtual void push_style_color(HudStyleColor idx, const cvec4& color) = 0;
		virtual void pop_style_color(HudStyleColor idx) = 0;
		virtual void begin_layout(HudLayoutType type, const vec2& size = vec2(0.f), const vec2& item_spacing = vec2(2.f), const vec4& border = vec4(0.f)) = 0;
		virtual void end_layout() = 0;
		virtual void new_line() = 0;
		virtual void begin_stencil_write() = 0;
		virtual void end_stencil_write() = 0;
		virtual void begin_stencil_compare() = 0;
		virtual void end_stencil_compare() = 0;
		virtual void rect(const vec2& size, const cvec4& col) = 0;
		virtual void text(std::wstring_view text, uint font_size = 24, const cvec4& col = cvec4(255)) = 0;
		virtual void image(const vec2& size, const graphics::ImageDesc& img, const cvec4& col = cvec4(255)) = 0;
		inline void image(const vec2& size, graphics::ImagePtr img, const cvec4& col = cvec4(255))
		{
			graphics::ImageDesc desc;
			desc.view = img->get_view();
			desc.uvs = vec4(0.f, 0.f, 1.f, 1.f);
			desc.border_uvs = vec4(0.f);
			image(size, desc, col);
		}
		virtual void image_stretched(const vec2& size, const graphics::ImageDesc& img, const vec4& border = vec4(0.f), const cvec4& col = cvec4(255)) = 0;
		inline void image_stretched(const vec2& size, graphics::ImagePtr img, const vec4& border = vec4(0.f), const cvec4& col = cvec4(255))
		{
			graphics::ImageDesc desc;
			desc.view = img->get_view();
			desc.uvs = vec4(0.f, 0.f, 1.f, 1.f);
			desc.border_uvs = vec4(0.f);
			image_stretched(size, desc, border, col);
		}
		virtual void image_rotated(const vec2& size, const graphics::ImageDesc& img, const cvec4& col, float angle) = 0;
		inline void image_rotated(const vec2& size, graphics::ImagePtr img, const cvec4& col, float angle)
		{
			graphics::ImageDesc desc;
			desc.view = img->get_view();
			desc.uvs = vec4(0.f, 0.f, 1.f, 1.f);
			desc.border_uvs = vec4(0.f);
			image_rotated(size, desc, col, angle);
		}
		virtual bool button(std::wstring_view label, uint font_size = 24) = 0;
		virtual bool image_button(const vec2& size, const graphics::ImageDesc& img = {}, const vec4& border = vec4(0.f)) = 0;
		inline bool image_button(const vec2& size, graphics::ImagePtr img, const vec4& border = vec4(0.f))
		{
			graphics::ImageDesc desc;
			desc.view = img->get_view();
			desc.uvs = vec4(0.f, 0.f, 1.f, 1.f);
			desc.border_uvs = vec4(0.f);
			return image_button(size, desc, border);
		}
		virtual void stroke_item(float thickness = 1.f, const cvec4& col = cvec4(255)) = 0;
		virtual bool item_hovered() = 0;
		virtual bool item_clicked() = 0;

		struct Instance
		{
			virtual sHudPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;

		struct Create
		{
			virtual sHudPtr operator()(WorldPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
