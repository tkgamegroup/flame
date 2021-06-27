#include "../../graphics/device.h"
#include "../../graphics/font.h"
#include "../world_private.h"
#include "element_private.h"
#include "text_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cTextPrivate::set_text(const std::wstring& _text)
	{
		text = _text;
		mark_text_changed();
	}

	void cTextPrivate::set_font_size(uint s)
	{
		if (font_size == s)
			return;
		font_size = s;
		if (element)
		{
			element->mark_drawing_dirty();
			element->mark_size_dirty();
		}
		if (entity)
			entity->component_data_changed(this, S<"font_size"_h>);
	}

	void cTextPrivate::set_font_color(const cvec4& col)
	{
		if (font_color == col)
			return;
		font_color = col;
		if (element)
			element->mark_drawing_dirty();
		if (entity)
			entity->component_data_changed(this, S<"font_color"_h>);
	}

	void cTextPrivate::set_text_align(Align a)
	{
		if (text_align == a)
			return;
		text_align = a;
		if (element)
			element->mark_drawing_dirty();
		if (entity)
			entity->component_data_changed(this, S<"text_align"_h>);
	}

	void cTextPrivate::row_layout(int offset, vec2& size, uint& num_chars)
	{
		auto width = 0.f;

		auto beg = text.c_str() + offset;
		auto end = text.c_str() + text.size();
		auto s = beg;
		while (s < end)
		{
			auto c = *s++;
			if (c == L'\n')
				break;
			if (c == L'\r')
				continue;

			width += atlas->get_glyph(c, font_size).advance;
		}

		width = max(width, (float)font_size);

		size = vec2(width, font_size);
		num_chars = s - beg;
	}

	void cTextPrivate::mark_text_changed()
	{
		if (element)
		{
			element->mark_drawing_dirty();
			element->mark_size_dirty();
		}
		if (entity)
			entity->component_data_changed(this, S<"text"_h>);
	}

	uint cTextPrivate::draw(uint layer, sRenderer* s_renderer)
	{
		if (res_id != -1)
		{
			layer++;
			auto pos = element->points[4];
			auto axes = element->axes;
			switch (text_align)
			{
			case AlignMin:
				break;
			case AlignMiddle:
				pos += axes[0] * (element->size.x - element->desired_size.x) * 0.5f;
				break;
			case AlignMax:
				pos += axes[0] * (element->size.x - element->desired_size.x);
				break;
			}
			auto glyphs = atlas->get_draw_glyphs(font_size, text.c_str(), text.c_str() + text.size(), pos, axes);
			s_renderer->draw_glyphs(layer, glyphs.size(), glyphs.data(), res_id, font_color);
		}
		return layer;
	}

	bool cTextPrivate::measure(vec2* s)
	{
		if (!atlas)
			return false;
		*s = vec2(atlas->text_size(font_size, text.c_str()));
		return true;
	}

	void cTextPrivate::on_added()
	{
		element = entity->get_component_i<cElementPrivate>(0);
		fassert(element);

		drawer = element->add_drawer([](Capture& c, uint layer, sRendererPtr s_renderer) {
			auto thiz = c.thiz<cTextPrivate>();
			return thiz->draw(layer, s_renderer);
		}, Capture().set_thiz(this));
		measurer = element->add_measurer([](Capture& c, vec2* s) {
			auto thiz = c.thiz<cTextPrivate>();
			return thiz->measure(s);
		}, Capture().set_thiz(this));
		element->mark_drawing_dirty();
		element->mark_size_dirty();
	}

	void cTextPrivate::on_removed()
	{
		element->remove_drawer(drawer);
		element->remove_measurer(measurer);
		element->mark_drawing_dirty();
		element->mark_size_dirty();
	}

	void cTextPrivate::on_entered_world()
	{
		s_renderer = entity->world->get_system_t<sRenderer>();
		fassert(s_renderer);

		atlas = graphics::FontAtlas::get(graphics::Device::get_default(), atlas_name.c_str());
		fassert(atlas);
		auto iv = atlas->get_view();
		res_id = s_renderer->find_element_res(iv);
		if (res_id == -1)
			res_id = s_renderer->set_element_res(-1, iv);
	}

	void cTextPrivate::on_left_world()
	{
		s_renderer = nullptr;
		res_id = -1;
		atlas = nullptr;
	}

	cText* cText::create(void* parms)
	{
		return f_new<cTextPrivate>();
	}
}
