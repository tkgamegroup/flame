#pragma once

#include "../../audio/source.h"
#include "../../graphics/image.h"
#include "../../graphics/canvas.h"
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
		HudStyleVarFontSize,
		HudStyleVarBorder,
		HudStyleVarButtonBorder,
		HudStyleVarFrame,
		HudStyleVarButtonFrame,
		HudStyleVarSpacing,
		HudStyleVarWindowBorder,
		HudStyleVarWindowFrame,

		HudStyleVarCount
	};

	enum HudStyleColor
	{
		HudStyleColorBackground,
		HudStyleColorFrame,
		HudStyleColorWindowBackground,
		HudStyleColorWindowFrame,
		HudStyleColorText,
		HudStyleColorTextDisabled,
		HudStyleColorImage,
		HudStyleColorButton,
		HudStyleColorButtonHovered,
		HudStyleColorButtonActive,
		HudStyleColorButtonDisabled,
		HudStyleColorButtonFrame,
		HudStyleColorButtonFrameHovered,
		HudStyleColorButtonFrameActive,
		HudStyleColorButtonFrameDisabled,

		HudStyleColorCount
	};

	enum HudStyleImage
	{
		HudStyleImageBackground,
		HudStyleImageWindowBackground,
		HudStyleImageButton,
		HudStyleImageButtonHovered,
		HudStyleImageButtonActive,
		HudStyleImageButtonDisabled,

		HudStyleImageCount
	};

	enum HudStyleSound
	{
		HudStyleSoundButtonHover,
		HudStyleSoundButtonClicked,

		HudStyleSoundCount
	};

	// Reflect ctor
	struct sHud : System
	{
		graphics::CanvasPtr canvas = nullptr;

		Listeners<void()> callbacks;

		virtual void bind_window(graphics::WindowPtr window) = 0;

		virtual void begin(uint id, const vec2& pos, const vec2& size = vec2(0.f) /* (0,0) means auto size */, const vec2& pivot = vec2(0.f), bool is_modal = false) = 0;
		virtual void begin_popup() = 0;
		virtual void end() = 0;

		virtual vec2 get_cursor() = 0;
		virtual void set_cursor(const vec2& pos) = 0;
		virtual Rect wnd_rect() const = 0;
		virtual Rect item_rect() const = 0;
		inline vec2 screen_size() const
		{
			return canvas->size;
		}
		virtual void stroke_item(float thickness = 1.f, const cvec4& col = cvec4(255)) = 0;
		virtual bool item_hovered() = 0;
		virtual bool item_clicked() = 0;
		virtual bool is_modal() = 0;
		// styles:
		virtual void push_style_var(HudStyleVar idx, const vec4& value, uint num_styles = 1) = 0;
		virtual void pop_style_var(HudStyleVar idx, uint num_styles = 1) = 0;
		virtual void push_style_color(HudStyleColor idx, const cvec4& color, uint num_styles = 1) = 0;
		virtual void pop_style_color(HudStyleColor idx, uint num_styles = 1) = 0;
		virtual void push_style_image(HudStyleImage idx, const graphics::ImageDesc& image, uint num_styles = 1) = 0;
		virtual void pop_style_image(HudStyleImage idx, uint num_styles = 1) = 0;
		virtual void push_style_sound(HudStyleSound idx, audio::SourcePtr sound, uint num_styles = 1) = 0;
		virtual void pop_style_sound(HudStyleSound idx, uint num_styles = 1) = 0;
		virtual void push_enable(bool v) = 0;
		virtual void pop_enable() = 0;
		// layout:
		virtual void begin_layout(HudLayoutType type, const vec2& size = vec2(0.f)) = 0;
		virtual void end_layout() = 0;
		virtual void newline() = 0;
		// stencil:
		virtual void begin_stencil_write() = 0;
		virtual void end_stencil_write() = 0;
		virtual void begin_stencil_compare() = 0;
		virtual void end_stencil_compare() = 0;
		// widgets:
		virtual void rect(const vec2& size, const cvec4& col) = 0;
		virtual void text(std::wstring_view text) = 0;
		virtual bool checkbox(bool* v, std::wstring_view label) = 0;
		virtual void image(const vec2& size, const graphics::ImageDesc& image) = 0;
		virtual void image_stretched(const vec2& size, const graphics::ImageDesc& image) = 0;
		virtual void image_rotated(const vec2& size, const graphics::ImageDesc& image, float angle) = 0;
		virtual bool button(std::wstring_view label, uint id = 0) = 0;
		virtual void progress_bar(const vec2& size, float progress, const cvec4& color, const cvec4& background_color, std::wstring_view label = L"") = 0;

		virtual void render(int idx, graphics::CommandBufferPtr cb, bool cleanup) = 0;

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
