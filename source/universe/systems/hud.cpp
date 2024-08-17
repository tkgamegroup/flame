#include "../../foundation/window.h"
#include "../../graphics/window.h"
#include "hud_private.h"
#include "input_private.h"

namespace flame
{
	sHudPrivate::sHudPrivate()
	{
		canvas = graphics::Canvas::create();
		canvas->clear_framebuffer = false;

		style_vars.resize(HudStyleVarCount);
		style_vars[HudStyleVarScaling].push(vec4(1.f, 1.f, 0.f, 0.f));
		style_vars[HudStyleVarAlpha].push(vec4(1.f, 0.f, 0.f, 0.f));
		style_vars[HudStyleVarFontSize].push(vec4(24.f, 0.f, 0.f, 0.f));
		style_vars[HudStyleVarBorder].push(vec4(2.f, 2.f, 2.f, 2.f));
		style_vars[HudStyleVarFrame].push(vec4(0.f, 0.f, 0.f, 0.f));
		style_vars[HudStyleVarSpacing].push(vec4(2.f, 2.f, 0.f, 0.f));
		style_vars[HudStyleVarWindowBorder].push(vec4(2.f, 2.f, 2.f, 2.f));
		style_vars[HudStyleVarWindowFrame].push(vec4(0.f, 0.f, 0.f, 0.f));

		style_colors.resize(HudStyleColorCount);
		style_colors[HudStyleColorBackground].push(cvec4(0, 0, 0, 0));
		style_colors[HudStyleColorFrame].push(cvec4(255, 255, 255, 255));
		style_colors[HudStyleColorWindowBackground].push(cvec4(20, 20, 20, 255));
		style_colors[HudStyleColorWindowFrame].push(cvec4(255, 255, 255, 255));
		style_colors[HudStyleColorText].push(cvec4(255, 255, 255, 255));
		style_colors[HudStyleColorTextDisabled].push(cvec4(84, 84, 84, 255));
		style_colors[HudStyleColorImage].push(cvec4(255, 255, 255, 255));
		style_colors[HudStyleColorButton].push(cvec4(46, 46, 46, 255));
		style_colors[HudStyleColorButtonHovered].push(cvec4(61, 61, 61, 255));
		style_colors[HudStyleColorButtonActive].push(cvec4(100, 100, 100, 255));
		style_colors[HudStyleColorButtonDisabled].push(cvec4(46, 46, 46, 255));
		style_colors[HudStyleColorButtonFrame].push(cvec4(255, 255, 255, 255));
		style_colors[HudStyleColorButtonFrameHovered].push(cvec4(255, 255, 255, 255));
		style_colors[HudStyleColorButtonFrameActive].push(cvec4(255, 255, 255, 255));
		style_colors[HudStyleColorButtonFrameDisabled].push(cvec4(255, 255, 255, 255));

		style_images.resize(HudStyleImageCount);
		style_images[HudStyleImageBackground].emplace();
		style_images[HudStyleImageWindowBackground].emplace();
		style_images[HudStyleImageButton].emplace();
		style_images[HudStyleImageButtonHovered].emplace();
		style_images[HudStyleImageButtonActive].emplace();
		style_images[HudStyleImageButtonDisabled].emplace();

		style_sounds.resize(HudStyleSoundCount);
		style_sounds[HudStyleSoundButtonHover].emplace();
		style_sounds[HudStyleSoundButtonClicked].emplace();

		enables.push(true);
	}

	sHudPrivate::sHudPrivate(int)
	{
	}

	HudLayout& sHudPrivate::add_layout(HudLayoutType type)
	{
		auto& hud = *last_hud;
		auto pos = hud.layouts.back().cursor;
		auto& layout = hud.layouts.emplace_back();
		layout.type = type;
		layout.rect.a = layout.rect.b = pos;
		layout.cursor = pos;
		layout.auto_size = true;
		return layout;
	}

	void sHudPrivate::finish_layout(HudLayout& layout)
	{
		if (layout.auto_size)
		{
			if (layout.rect.b.x > layout.rect.a.x && layout.rect.b.y > layout.rect.a.y)
				layout.rect.b -= layout.spacing;
			layout.rect.b += (layout.border.xy() + layout.border.zw());
		}
	}

	void sHudPrivate::bind_window(graphics::WindowPtr window)
	{
		bound_window = window;
		if (!canvas->bound_window)
			canvas->bind_window(window);
	}
	
	uint fhash(float x)
	{
		union
		{
			float f;
			uint u;
		};
		f = x;
		return u;
	}

	void sHudPrivate::begin(uint id, const vec2& pos, const vec2& size, const vec2& pivot, bool is_modal)
	{
		auto& hud = huds[id];
		hud.id = id;
		last_hud = &hud;

		if (is_modal)
		{
			current_modal = id;
			modal_frames = 2;
		}

		auto scaling = style_vars[HudStyleVarScaling].top().xy();
		auto alpha = style_vars[HudStyleVarAlpha].top().x;
		auto border = style_vars[HudStyleVarBorder].top();
		border *= vec4(scaling, scaling);
		auto window_border = style_vars[HudStyleVarWindowBorder].top();
		window_border *= vec4(scaling, scaling);
		auto frame = style_vars[HudStyleVarWindowFrame].top().x;
		auto spacing = style_vars[HudStyleVarSpacing].top().xy();
		spacing *= scaling;
		auto color = style_colors[HudStyleColorWindowBackground].top();
		color.a *= alpha;
		auto& image = style_images[HudStyleImageWindowBackground].top();
		auto auto_sizing = size.x == 0.f && size.y == 0.f;

		hud.pos = pos;
		if (pos.x < 0.f)
			hud.pos.x += canvas->size.x;
		if (pos.y < 0.f)
			hud.pos.y += canvas->size.y;
		if (auto_sizing)
			hud.size = hud.suggested_size;
		else
			hud.size = size * scaling;
		hud.pos -= hud.size * pivot;
		hud.border = window_border;

		hud.layouts.clear();
		auto& layout = hud.layouts.emplace_back();
		layout.type = HudVertical;
		layout.spacing = spacing;
		layout.border = border;
		layout.rect.a = layout.rect.b = hud.pos + hud.border.xy() + border.xy();
		layout.cursor = layout.rect.a;
		layout.auto_size = auto_sizing;

		Rect rect(hud.pos, hud.pos + hud.size);
		if (!image.view)
		{
			if (color.a > 0)
				canvas->draw_rect_filled(rect.a, rect.b, color);
		}
		else
		{
			if (color.a > 0)
				canvas->draw_image_stretched(image.view, rect.a, rect.b, image.uvs, window_border, image.border_uvs, color);
		}

		if (frame > 0.f)
		{
			auto color = style_colors[HudStyleColorWindowFrame].top();
			color.a *= alpha;
			if (color.a > 0)
				canvas->draw_rect(rect.a, rect.b, frame, color);
		}

		if (color.a > 0 && rect.contains(input->mpos))
			input->mouse_used = true;
	}

	void sHudPrivate::begin_popup()
	{
		auto id = "popup"_h;
		auto& hud = huds[id];
		hud.id = id;
		last_hud = &hud;
		
		auto scaling = style_vars[HudStyleVarScaling].top().xy();
		auto alpha = style_vars[HudStyleVarAlpha].top().x;
		auto border = style_vars[HudStyleVarBorder].top();
		border *= vec4(scaling, scaling);
		auto window_border = style_vars[HudStyleVarWindowBorder].top();
		window_border *= vec4(scaling, scaling);
		auto frame = style_vars[HudStyleVarWindowFrame].top().x;
		auto spacing = style_vars[HudStyleVarSpacing].top().xy();
		spacing *= scaling;
		auto color = style_colors[HudStyleColorWindowBackground].top();
		color.a *= alpha;
		auto& image = style_images[HudStyleImageWindowBackground].top();

		hud.pos = input->mpos + vec2(16.f, 0.f);
		hud.size = hud.suggested_size;
		auto pivot = vec2(0.f);
		if (hud.pos.x + hud.size.x > canvas->size.x)
		{
			hud.pos = input->mpos + vec2(-4.f, 0.f);
			pivot.x = 1.f;
		}
		if (hud.pos.y + hud.size.y > canvas->size.y)
			pivot.y = 1.f;
		hud.pos -= hud.size * pivot;
		hud.border = window_border;

		hud.layouts.clear();
		auto& layout = hud.layouts.emplace_back();
		layout.type = HudVertical;
		layout.spacing = spacing;
		layout.border = border;
		layout.rect.a = layout.rect.b = hud.pos + hud.border.xy() + border.xy();
		layout.cursor = layout.rect.a;
		layout.auto_size = true;

		Rect rect(hud.pos, hud.pos + hud.size);
		if (!image.view)
		{
			if (color.a > 0)
				canvas->draw_rect_filled(rect.a, rect.b, color);
		}
		else
		{
			if (color.a > 0)
				canvas->draw_image_stretched(image.view, rect.a, rect.b, image.uvs, window_border, image.border_uvs, color);
		}

		if (frame > 0.f)
		{
			auto color = style_colors[HudStyleColorWindowFrame].top();
			color.a *= alpha;
			if (color.a > 0)
				canvas->draw_rect(rect.a, rect.b, frame, color);
		}

		if (color.a > 0 && rect.contains(input->mpos))
			input->mouse_used = true;
	}

	void sHudPrivate::end()
	{
		if (huds.empty())
			return;
		auto& hud = *last_hud;
		if (hud.layouts.empty())
			return;
		auto& layout = hud.layouts[0];
		finish_layout(layout);
		if (layout.auto_size)
			hud.suggested_size = layout.rect.size() + hud.border.xy() + hud.border.zw();
	}

	vec2 sHudPrivate::get_cursor()
	{
		if (huds.empty())
			return vec2(-1.f);
		auto& hud = *last_hud;
		if (hud.layouts.empty())
			return vec2(-1.f);
		auto& layout = hud.layouts.back();

		return layout.cursor;
	}

	void sHudPrivate::set_cursor(const vec2& pos)
	{
		if (huds.empty())
			return;
		auto& hud = *last_hud;
		if (hud.layouts.empty())
			return;
		auto& layout = hud.layouts.back();

		layout.cursor = pos;
		if (layout.auto_size)
			layout.rect.b = max(layout.rect.b, pos);
	}

	Rect sHudPrivate::wnd_rect() const
	{
		if (huds.empty())
			return Rect(0.f, 0.f, 0.f, 0.f);
		auto& hud = *last_hud;
		return Rect(hud.pos, hud.pos + hud.size);
	}

	Rect sHudPrivate::item_rect() const
	{
		return last_rect;
	}

	void sHudPrivate::stroke_item(float thickness, const cvec4& col)
	{
		auto alpha = style_vars[HudStyleVarAlpha].top().x;

		auto rect = last_rect;
		canvas->draw_rect(rect.a, rect.b, thickness, cvec4(col.xyz(), col.a * alpha));
	}

	bool sHudPrivate::item_hovered()
	{
		if (huds.empty())
			return false;
		auto& hud = *last_hud;

		if (current_modal && hud.id != current_modal)
			return false;
		return last_rect.contains(sInput::instance()->mpos);
	}

	bool sHudPrivate::item_clicked()
	{
		if (huds.empty())
			return false;
		auto& hud = *last_hud;

		if (current_modal && hud.id != current_modal)
			return false;
		return last_rect.contains(sInput::instance()->mpos) && input->mpressed(Mouse_Left);
	}

	bool sHudPrivate::is_modal()
	{
		return current_modal != 0;
	}

	void sHudPrivate::push_style_var(HudStyleVar idx, const vec4& value)
	{
		style_vars[idx].push(value);
	}

	void sHudPrivate::pop_style_var(HudStyleVar idx)
	{
		style_vars[idx].pop();
	}

	void sHudPrivate::push_style_color(HudStyleColor idx, const cvec4& color)
	{
		style_colors[idx].push(color);
	}

	void sHudPrivate::pop_style_color(HudStyleColor idx)
	{
		style_colors[idx].pop();
	}

	void sHudPrivate::push_style_image(HudStyleImage idx, const graphics::ImageDesc& image)
	{
		style_images[idx].push(image);
	}

	void sHudPrivate::pop_style_image(HudStyleImage idx)
	{
		style_images[idx].pop();
	}

	void sHudPrivate::push_style_sound(HudStyleSound idx, audio::SourcePtr sound)
	{
		style_sounds[idx].push(sound);
	}

	void sHudPrivate::pop_style_sound(HudStyleSound idx)
	{
		style_sounds[idx].pop();
	}

	void sHudPrivate::push_enable(bool v)
	{
		enables.push(v);
	}

	void sHudPrivate::pop_enable()
	{
		enables.pop();
	}

	void sHudPrivate::begin_layout(HudLayoutType type, const vec2& size)
	{
		auto scaling = style_vars[HudStyleVarScaling].top().xy();
		auto border = style_vars[HudStyleVarBorder].top();
		border *= vec4(scaling, scaling);
		auto spacing = style_vars[HudStyleVarSpacing].top().xy();
		spacing *= scaling;

		auto& layout = add_layout(type);
		if (size.x != 0.f || size.y != 0.f)
		{
			layout.auto_size = false;
			layout.rect.b = layout.rect.a + size * scaling;
		}
		layout.spacing = spacing;
		layout.border = border;
		layout.cursor += border.xy();
	}

	void sHudPrivate::end_layout()
	{
		if (huds.empty())
			return;
		auto& hud = *last_hud;
		auto& layout = hud.layouts.back();
		finish_layout(layout);
		auto size = layout.rect.size();
		hud.layouts.pop_back();
		add_rect(size);
	}

	void sHudPrivate::newline()
	{
		if (huds.empty())
			return;
		auto& hud = *last_hud;
		if (hud.layouts.empty())
			return;
		auto& layout = hud.layouts.back();

		auto scaling = style_vars[HudStyleVarScaling].top().xy();

		if (layout.type == HudHorizontal)
		{
			layout.cursor.x = layout.rect.a.x;
			layout.cursor.y += layout.item_max.y + layout.spacing.y * scaling.y;
			if (layout.auto_size)
				layout.rect.b = max(layout.rect.b, layout.cursor);
		}
	}

	void sHudPrivate::begin_stencil_write()
	{
		canvas->begin_stencil_write();
	}

	void sHudPrivate::end_stencil_write()
	{
		canvas->end_stencil_write();
	}

	void sHudPrivate::begin_stencil_compare()
	{
		canvas->begin_stencil_compare();
	}

	void sHudPrivate::end_stencil_compare()
	{
		canvas->end_stencil_compare();
	}

	Rect sHudPrivate::add_rect(const vec2& _sz)
	{
		if (huds.empty())
			return Rect();
		auto& hud = *last_hud;
		if (hud.layouts.empty())
			return Rect();
		auto& layout = hud.layouts.back();

		auto scaling = style_vars[HudStyleVarScaling].top().xy();
		auto spacing = layout.spacing;
		auto size = _sz;
		size *= scaling;

		Rect rect(layout.cursor, layout.cursor + size);
		layout.item_max = max(layout.item_max, size);
		if (layout.type == HudHorizontal)
		{
			layout.cursor.x += size.x + spacing.x;
			if (layout.auto_size)
				layout.rect.b = max(layout.rect.b, layout.cursor + vec2(0.f, layout.item_max.y));
		}
		else
		{
			if (layout.auto_size)
				layout.rect.b.x = max(layout.rect.b.x, layout.cursor.x + size.x + spacing.x);
			layout.cursor.x = layout.rect.a.x;
			layout.cursor.y += size.y + spacing.y;
			if (layout.auto_size)
				layout.rect.b.y = layout.cursor.y;
		}
		last_rect = rect;
		return rect;
	}

	void sHudPrivate::text(std::wstring_view text)
	{
		auto scaling = style_vars[HudStyleVarScaling].top().xy();
		auto alpha = style_vars[HudStyleVarAlpha].top().x;
		auto font_size = style_vars[HudStyleVarFontSize].top().x;
		auto color = style_colors[HudStyleColorText].top();
		color.a *= alpha;

		auto sz = canvas->calc_text_size(canvas->default_font_atlas, font_size, text);
		auto rect = add_rect(sz);
		auto background_color = style_colors[HudStyleColorBackground].top();
		if (background_color.a > 0)
			canvas->draw_rect_filled(rect.a, rect.b, background_color);
		if (color.a > 0)
			canvas->draw_text(canvas->default_font_atlas, font_size, rect.a, text, color, 0.5f, 0.2f, scaling);
	}

	void sHudPrivate::rect(const vec2& size, const cvec4& col)
	{
		auto alpha = style_vars[HudStyleVarAlpha].top().x;

		auto rect = add_rect(size);
		canvas->draw_rect_filled(rect.a, rect.b, cvec4(col.xyz(), col.a * alpha));
	}

	void sHudPrivate::image(const vec2& size, const graphics::ImageDesc& image)
	{
		auto alpha = style_vars[HudStyleVarAlpha].top().x;
		auto color = style_colors[HudStyleColorImage].top();
		color.a *= alpha;

		auto rect = add_rect(size);
		if (color.a > 0)
			canvas->draw_image(image.view, rect.a, rect.b, image.uvs, color);
	}

	void sHudPrivate::image_stretched(const vec2& size, const graphics::ImageDesc& image)
	{
		auto scaling = style_vars[HudStyleVarScaling].top().xy();
		auto border = style_vars[HudStyleVarBorder].top();
		border *= vec4(scaling, scaling);
		auto alpha = style_vars[HudStyleVarAlpha].top().x;
		auto color = style_colors[HudStyleColorImage].top();
		color.a *= alpha;

		auto rect = add_rect(size);
		if (color.a > 0)
			canvas->draw_image_stretched(image.view, rect.a, rect.b, image.uvs, border, image.border_uvs, color);
	}

	void sHudPrivate::image_rotated(const vec2& size, const graphics::ImageDesc& image, float angle)
	{
		auto alpha = style_vars[HudStyleVarAlpha].top().x;
		auto color = style_colors[HudStyleColorImage].top();
		color.a *= alpha;

		auto rect = add_rect(size);
		if (color.a > 0)
			canvas->draw_image_rotated(image.view, rect.a, rect.b, image.uvs, color, angle);
	}

	bool sHudPrivate::button(std::wstring_view label, uint id)
	{
		if (huds.empty())
			return false;
		auto& hud = *last_hud;

		auto scaling = style_vars[HudStyleVarScaling].top().xy();
		auto alpha = style_vars[HudStyleVarAlpha].top().x;
		auto font_size = style_vars[HudStyleVarFontSize].top().x;
		auto border = style_vars[HudStyleVarBorder].top();
		border *= vec4(scaling, scaling);
		auto enable = enables.top();

		auto sz = canvas->calc_text_size(canvas->default_font_atlas, font_size, label);
		sz += border.xy() + border.zw();

		auto rect = add_rect(sz);
		auto state = 0;
		auto pressed = false;
		if (enable)
		{
			if (!current_modal || hud.id == current_modal)
			{
				if (rect.contains(input->mpos))
				{
					state = 1;
					if (input->mbtn[Mouse_Left])
						state = 2;
					if (input->mreleased(Mouse_Left))
						pressed = true;

					input->mouse_used = true;
				}
			}
		}
		else
			state = 3;

		auto color = style_colors[HudStyleColorButton + state].top();
		color.a *= alpha;
		auto image = style_images[HudStyleImageButton + state].top();
		auto text_color = style_colors[enable ? HudStyleColorText : HudStyleColorTextDisabled].top();
		text_color.a *= alpha;

		if (color.a > 0)
		{
			if (image.view)
				canvas->draw_image_stretched(image.view, rect.a, rect.b, image.uvs, border, image.border_uvs, color);
			else
				canvas->draw_rect_filled(rect.a, rect.b, color);
		}
		if (text_color.a > 0)
			canvas->draw_text(canvas->default_font_atlas, font_size, rect.a + border.xy(), label, text_color, 0.5f, 0.2f, scaling);

		if (id != 0)
		{
			if (state == 1)
			{
				if (last_hovered != id)
				{
					auto sound = style_sounds[HudStyleSoundButtonHover].top();
					if (sound)
						sound->play();
				}
				last_hovered = id;
				hover_frames = 2;
			}
			else if (state == 2)
			{
				if (last_active != id)
				{
					auto sound = style_sounds[HudStyleSoundButtonClicked].top();
					if (sound)
						sound->play();
				}
				last_hovered = id;
				hover_frames = 2;
				last_active = id;
				active_frames = 2;
			}
		}

		return enable && pressed;
	}

	void sHudPrivate::progress_bar(const vec2& size, float progress, const cvec4& color, const cvec4& background_color, std::wstring_view label)
	{
		if (huds.empty())
			return;
		auto& hud = *last_hud;

		auto scaling = style_vars[HudStyleVarScaling].top().xy();
		auto alpha = style_vars[HudStyleVarAlpha].top().x;
		auto font_size = style_vars[HudStyleVarFontSize].top().x;

		auto rect = add_rect(size);
		if (auto a = background_color.a * alpha; a > 0)
			canvas->draw_rect_filled(rect.a, rect.b, cvec4(background_color.xyz(), a));
		if (auto a = color.a * alpha; a > 0)
			canvas->draw_rect_filled(rect.a, vec2(mix(rect.a.x, rect.b.x, progress), rect.b.y), cvec4(color.xyz(), a));
		if (!label.empty())
		{
			auto text_color = style_colors[HudStyleColorText].top();
			text_color.a *= alpha;
			if (text_color.a > 0)
			{
				auto sz = canvas->calc_text_size(canvas->default_font_atlas, font_size, label);
				sz *= scaling;
				canvas->draw_text(canvas->default_font_atlas, font_size, (rect.a + rect.b) * 0.5f - sz * 0.5f, label, text_color, 0.5f, 0.2f, scaling);
			}
		}
	}

	void sHudPrivate::start()
	{
		if (bound_window && !canvas->bound_window)
			canvas->bind_window(bound_window);
		input = sInput::instance();
	}

	void sHudPrivate::update()
	{
		if (modal_frames > 0)
		{
			modal_frames--;
			if (modal_frames == 0)
				current_modal = 0;
		}
		if (hover_frames > 0)
		{
			hover_frames--;
			if (hover_frames == 0)
				last_hovered = 0;
		}
		if (active_frames > 0)
		{
			active_frames--;
			if (active_frames == 0)
				last_active = 0;
		}
	}

	void sHudPrivate::render(int idx, graphics::CommandBufferPtr cb)
	{
		canvas->render(idx, cb);
	}

	static sHudPtr _instance = nullptr;

	struct sHudInstance : sHud::Instance
	{
		sHudPtr operator()() override
		{
			return _instance;
		}
	}sHud_instance;
	sHud::Instance& sHud::instance = sHud_instance;

	struct sHudCreatePrivate : sHud::Create
	{
		sHudPtr operator()(WorldPtr w) override
		{
			if (!w)
				return new sHudPrivate(0);

			assert(!_instance);

			_instance = new sHudPrivate();
			return _instance;
		}
	}sHud_create;
	sHud::Create& sHud::create = sHud_create;
}
