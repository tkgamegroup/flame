#include "../../foundation/window.h"
#include "../../graphics/window.h"
#include "hud_private.h"
#include "input_private.h"

namespace flame
{
	sHudPrivate::sHudPrivate()
	{
		style_vars.resize(HudStyleVarCount);
		style_vars[HudStyleVarScaling].push(vec2(1.f));
		style_vars[HudStyleVarAlpha].push(vec2(1.f));

		style_colors.resize(HudStyleColorCount);
		style_colors[HudStyleColorTextBg].push(cvec4(0, 0, 0, 0));
		style_colors[HudStyleColorButton].push(cvec4(35, 69, 109, 255));
		style_colors[HudStyleColorButtonHovered].push(cvec4(66, 150, 250, 255));
	}

	HudLayout& sHudPrivate::add_layout(HudLayoutType type)
	{
		auto& hud = huds.back();
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
		auto scaling = style_vars[HudStyleVarScaling].top();

		if (layout.auto_size)
		{
			if (layout.rect.b.x > layout.rect.a.x && layout.rect.b.y > layout.rect.a.y)
				layout.rect.b -= layout.item_spacing * scaling;
			layout.rect.b += (layout.border.xy() + layout.border.zw()) * scaling;
		}
	}

	void sHudPrivate::bind_window(graphics::WindowPtr window)
	{
		bound_window = window;
		if (canvas)
			canvas->bind_window(window);
	}

	void sHudPrivate::begin(uint id, const vec2& pos, const vec2& size, const cvec4& col, const vec2& pivot, const graphics::ImageDesc& image, const vec4& border, bool is_modal)
	{
		auto& hud = huds.emplace_back();
		hud.id = id;

		if (is_modal)
		{
			current_modal = id;
			modal_frames = 2;
		}

		auto scaling = style_vars[HudStyleVarScaling].top();
		auto alpha = style_vars[HudStyleVarAlpha].top().x;
		auto auto_sizing = size.x == 0.f && size.y == 0.f;

		hud.pos = pos;
		if (pos.x < 0.f)
			hud.pos.x += canvas->size.x;
		if (pos.y < 0.f)
			hud.pos.y += canvas->size.y;
		hud.size = size * scaling;
		if (!auto_sizing)
			hud.pos -= hud.size * pivot;
		hud.color = col;
		hud.color.a *= alpha;
		hud.pivot = pivot;
		hud.border = border * vec4(scaling, scaling);

		hud.layouts.clear();
		auto& layout = hud.layouts.emplace_back();
		layout.type = HudVertical;
		layout.rect.a = layout.rect.b = hud.pos + hud.border.xy();
		layout.cursor = layout.rect.a;
		layout.auto_size = auto_sizing;

		if (!image.view)
		{
			hud.bg_verts = canvas->draw_rect_filled(vec2(0.f), vec2(100.f), hud.color); // 100 for temporary size
			hud.bg_vert_count = 4;
		}
		else
		{
			auto size = (vec2)image.view->image->extent.xy() * (image.uvs.zw() - image.uvs.xy());
			hud.bg_verts = canvas->draw_image_stretched(image.view, vec2(0.f), vec2(border.xy() + border.zw()) + vec2(1.f), image.uvs, border, image.border_uvs, hud.color);
			hud.bg_vert_count = 9 * 4;
		}
		if ((pivot.x != 0.f || pivot.y != 0.f) && auto_sizing)
			hud.translate_cmd_idx = canvas->set_translate(vec2(0.f));
		else
			hud.translate_cmd_idx = -1;
	}

	void sHudPrivate::end()
	{
		if (huds.empty())
			return;
		auto& hud = huds.back();
		if (hud.layouts.empty())
			return;
		auto& layout = hud.layouts[0];

		auto input = sInput::instance();

		finish_layout(layout);
		if (layout.auto_size)
			hud.size = layout.rect.size() + hud.border.xy() + hud.border.zw();

		if (hud.translate_cmd_idx != -1)
		{
			auto translate = -hud.size * hud.pivot;
			hud.pos += translate;
			canvas->draw_cmds[hud.translate_cmd_idx].data.translate = translate;
			canvas->set_translate(vec2(0.f));
		}

		if (hud.bg_verts)
		{
			if (hud.bg_vert_count <= 4)
			{
				auto scl = hud.size / 100.f; // 100 is temporary size
				for (auto i = 0; i < hud.bg_vert_count; i++)
					hud.bg_verts[i].pos = hud.bg_verts[i].pos * scl + hud.pos;
			}
			else // is a stretched image
			{
				auto sz = hud.size - hud.bg_verts[1].pos;
				// hud.bg_verts[0] stays the same
				// hud.bg_verts[1] stays the same
				hud.bg_verts[2].pos.x += sz.x - 1.f;
				hud.bg_verts[3].pos.x += sz.x - 1.f;

				hud.bg_verts[4].pos.y += sz.y - 1.f;
				hud.bg_verts[5].pos.y += sz.y - 1.f;
				hud.bg_verts[6].pos += sz - 1.f;
				hud.bg_verts[7].pos += sz - 1.f;

				// hud.bg_verts[8] stays the same
				hud.bg_verts[9].pos.y += sz.y - 1.f;
				hud.bg_verts[10].pos.y += sz.y - 1.f;
				// hud.bg_verts[11] stays the same

				hud.bg_verts[12].pos.x += sz.x - 1.f;
				hud.bg_verts[13].pos += sz - 1.f;
				hud.bg_verts[14].pos += sz - 1.f;
				hud.bg_verts[15].pos.x += sz.x - 1.f;

				// hud.bg_verts[16] stays the same
				// hud.bg_verts[17] stays the same
				// hud.bg_verts[18] stays the same
				// hud.bg_verts[19] stays the same

				hud.bg_verts[20].pos.x += sz.x - 1.f;
				hud.bg_verts[21].pos.x += sz.x - 1.f;
				hud.bg_verts[22].pos.x += sz.x - 1.f;
				hud.bg_verts[23].pos.x += sz.x - 1.f;

				hud.bg_verts[24].pos.y += sz.y - 1.f;
				hud.bg_verts[25].pos.y += sz.y - 1.f;
				hud.bg_verts[26].pos.y += sz.y - 1.f;
				hud.bg_verts[27].pos.y += sz.y - 1.f;

				hud.bg_verts[28].pos += sz - 1.f;
				hud.bg_verts[29].pos += sz - 1.f;
				hud.bg_verts[30].pos += sz - 1.f;
				hud.bg_verts[31].pos += sz - 1.f;

				// hud.bg_verts[32] stays the same
				hud.bg_verts[33].pos.y += sz.y - 1.f;
				hud.bg_verts[34].pos += sz - 1.f;
				hud.bg_verts[35].pos.x += sz.x - 1.f;

				for (auto i = 0; i < hud.bg_vert_count; i++)
					hud.bg_verts[i].pos += hud.pos;
			}
		}
		Rect rect(hud.pos, hud.pos + hud.size);
		if (hud.color.a > 0 && rect.contains(input->mpos))
			input->mouse_used = true;

		huds.pop_back();
	}

	vec2 sHudPrivate::get_cursor()
	{
		if (huds.empty())
			return vec2(-1.f);
		auto& hud = huds.back();
		if (hud.layouts.empty())
			return vec2(-1.f);
		auto& layout = hud.layouts.back();

		return layout.cursor;
	}

	void sHudPrivate::set_cursor(const vec2& pos)
	{
		if (huds.empty())
			return;
		auto& hud = huds.back();
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
		auto& hud = huds.back();
		return Rect(hud.pos, hud.pos + hud.size);
	}

	Rect sHudPrivate::item_rect() const
	{
		return last_rect;
	}

	vec2 sHudPrivate::screen_size() const
	{
		return canvas->size;
	}

	void sHudPrivate::push_style_var(HudStyleVar idx, const vec2& value)
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

	void sHudPrivate::begin_layout(HudLayoutType type, const vec2& size, const vec2& item_spacing, const vec4& border)
	{
		auto scaling = style_vars[HudStyleVarScaling].top();

		auto& layout = add_layout(type);
		if (size.x != 0.f || size.y != 0.f)
		{
			layout.auto_size = false;
			layout.rect.b = layout.rect.a + size * scaling;
		}
		layout.item_spacing = item_spacing;
		layout.border = border;
		layout.cursor += border.xy() * scaling;
	}

	void sHudPrivate::end_layout()
	{
		if (huds.empty())
			return;
		auto& hud = huds.back();
		auto& layout = hud.layouts.back();
		finish_layout(layout);
		auto size = layout.rect.size();
		hud.layouts.pop_back();
		add_rect(size);
	}

	void sHudPrivate::new_line()
	{
		if (huds.empty())
			return;
		auto& hud = huds.back();
		if (hud.layouts.empty())
			return;
		auto& layout = hud.layouts.back();

		auto scaling = style_vars[HudStyleVarScaling].top();

		if (layout.type == HudHorizontal)
		{
			layout.cursor.x = layout.rect.a.x;
			layout.cursor.y += layout.item_max.y + layout.item_spacing.y * scaling.y;
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
		auto& hud = huds.back();
		if (hud.layouts.empty())
			return Rect();
		auto& layout = hud.layouts.back();

		auto item_spacing = layout.item_spacing;
		auto scaling = style_vars[HudStyleVarScaling].top();
		auto sz = _sz * scaling;
		item_spacing *= scaling;

		Rect rect(layout.cursor, layout.cursor + sz);
		layout.item_max = max(layout.item_max, sz);
		if (layout.type == HudHorizontal)
		{
			layout.cursor.x += sz.x + item_spacing.x;
			if (layout.auto_size)
				layout.rect.b = max(layout.rect.b, layout.cursor + vec2(0.f, layout.item_max.y));
		}
		else
		{
			if (layout.auto_size)
				layout.rect.b.x = max(layout.rect.b.x, layout.cursor.x + sz.x + item_spacing.x);
			layout.cursor.x = layout.rect.a.x;
			layout.cursor.y += sz.y + item_spacing.y;
			if (layout.auto_size)
				layout.rect.b.y = layout.cursor.y;
		}
		last_rect = rect;
		return rect;
	}

	void sHudPrivate::text(std::wstring_view text, uint font_size, const cvec4& col)
	{
		auto sz = canvas->calc_text_size(canvas->default_font_atlas, font_size, text);
		auto rect = add_rect(sz);
		auto bg_col = style_colors[HudStyleColorTextBg].top();
		if (bg_col.a > 0)
			canvas->draw_rect_filled(rect.a, rect.b, bg_col);
		canvas->draw_text(canvas->default_font_atlas, font_size, rect.a, text, col, 0.5f, 0.2f);
	}

	void sHudPrivate::rect(const vec2& size, const cvec4& col)
	{
		auto input = sInput::instance();
		auto alpha = style_vars[HudStyleVarAlpha].top().x;

		auto rect = add_rect(size);
		canvas->draw_rect_filled(rect.a, rect.b, cvec4(col.xyz(), col.a * alpha));
	}

	void sHudPrivate::image(const vec2& size, const graphics::ImageDesc& image, const cvec4& col)
	{
		auto input = sInput::instance();
		auto alpha = style_vars[HudStyleVarAlpha].top().x;

		auto rect = add_rect(size);
		canvas->draw_image(image.view, rect.a, rect.b, image.uvs, cvec4(col.xyz(), col.a * alpha));
	}

	void sHudPrivate::image_stretched(const vec2& size, const graphics::ImageDesc& image, const vec4& border, const cvec4& col)
	{
		auto input = sInput::instance();
		auto alpha = style_vars[HudStyleVarAlpha].top().x;

		auto rect = add_rect(size);
		canvas->draw_image_stretched(image.view, rect.a, rect.b, image.uvs, border, image.border_uvs, cvec4(col.xyz(), col.a * alpha));
	}

	void sHudPrivate::image_rotated(const vec2& size, const graphics::ImageDesc& image, const cvec4& col, float angle)
	{
		auto input = sInput::instance();
		auto alpha = style_vars[HudStyleVarAlpha].top().x;

		auto rect = add_rect(size);
		canvas->draw_image_rotated(image.view, rect.a, rect.b, image.uvs, cvec4(col.xyz(), col.a * alpha), angle);
	}

	bool sHudPrivate::button(std::wstring_view label, uint font_size)
	{
		if (huds.empty())
			return false;
		auto& hud = huds.back();
		auto input = sInput::instance();

		vec4 border(2.f);
		auto sz = canvas->calc_text_size(canvas->default_font_atlas, font_size, label);
		sz += border.xy() + border.zw();

		auto rect = add_rect(sz);
		auto state = 0;
		if (!current_modal || hud.id == current_modal)
		{
			if (rect.contains(input->mpos))
			{
				state = 1;
				if (input->mpressed(Mouse_Left))
					state = 2;

				input->mouse_used = true;
			}
		}
		canvas->draw_rect_filled(rect.a, rect.b, state == 0 ? style_colors[HudStyleColorButton].top() : style_colors[HudStyleColorButtonHovered].top());
		canvas->draw_text(canvas->default_font_atlas, font_size, rect.a + border.xy(), label, cvec4(255), 0.5f, 0.2f);
		return state == 2;
	}

	bool sHudPrivate::image_button(const vec2& size, const graphics::ImageDesc& image, const vec4& border)
	{
		if (huds.empty())
			return false;
		auto& hud = huds.back();
		auto input = sInput::instance();

		auto sz = size;
		sz += border.xy() + border.zw();

		auto rect = add_rect(sz);
		auto state = 0;
		if (!current_modal || hud.id == current_modal)
		{
			if (rect.contains(input->mpos))
			{
				state = 1;
				if (input->mpressed(Mouse_Left))
					state = 2;

				input->mouse_used = true;
			}
		}
		if (image.view)
		{
			if (border.x > 0.f || border.y > 0.f || border.z > 0.f || border.w > 0.f)
				canvas->draw_image_stretched(image.view, rect.a, rect.b, image.uvs, border, image.border_uvs, state == 0 ? cvec4(255) : cvec4(200));
			else
				canvas->draw_image(image.view, rect.a, rect.b, image.uvs, state == 0 ? cvec4(255) : cvec4(200));
		}
		else
			canvas->draw_rect_filled(rect.a, rect.b, state == 0 ? style_colors[HudStyleColorButton].top() : style_colors[HudStyleColorButtonHovered].top());
		return state == 2;
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
		auto& hud = huds.back();

		if (current_modal && hud.id != current_modal)
			return false;
		return last_rect.contains(sInput::instance()->mpos);
	}

	bool sHudPrivate::item_clicked()
	{
		if (huds.empty())
			return false;
		auto& hud = huds.back();
		auto input = sInput::instance();

		if (current_modal && hud.id != current_modal)
			return false;
		return last_rect.contains(sInput::instance()->mpos) && input->mpressed(Mouse_Left);
	}

	bool sHudPrivate::is_modal()
	{
		return current_modal != 0;
	}

	void sHudPrivate::start()
	{
		canvas = graphics::Canvas::create();
		canvas->clear_framebuffer = false;
		if (bound_window)
			canvas->bind_window(bound_window);
	}

	void sHudPrivate::update()
	{
		if (modal_frames > 0)
		{
			modal_frames--;
			if (modal_frames == 0)
				current_modal = 0;
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
				return new sHudPrivate();

			assert(!_instance);

			_instance = new sHudPrivate();
			return _instance;
		}
	}sHud_create;
	sHud::Create& sHud::create = sHud_create;
}
