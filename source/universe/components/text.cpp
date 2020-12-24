#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
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
			entity->data_changed(this, S<"size"_h>);
	}

	void cTextPrivate::set_color(const cvec4& col)
	{
		if (color == col)
			return;
		color = col;
		if (element)
		{
			element->mark_drawing_dirty();
			element->mark_size_dirty();
		}
		if (entity)
			entity->data_changed(this, S<"color"_h>);
	}

	void cTextPrivate::mark_text_changed()
	{
		if (element)
		{
			element->mark_drawing_dirty();
			element->mark_size_dirty();
		}
		if (entity)
			entity->data_changed(this, S<"text"_h>);
	}

	void cTextPrivate::draw(graphics::Canvas* canvas)
	{
		canvas->draw_text(res_id, text.c_str(), nullptr, font_size, color, element->points[4], element->axes);
	}

	void cTextPrivate::measure(vec2& ret)
	{
		if (!atlas)
		{
			ret = vec2(-1.f);
			return;
		}
		ret = vec2(atlas->text_size(font_size, text.c_str()));
	}

	void cTextPrivate::on_added()
	{
		element = entity->get_component_t<cElementPrivate>();
		fassert(element);

		element->drawers[1].emplace_back(this, (void(*)(Component*, graphics::Canvas*))f2a(&cTextPrivate::draw));
		element->measurables.emplace_back(this, (void(*)(Component*, vec2&))f2a(&cTextPrivate::measure));
		element->mark_drawing_dirty();
		element->mark_size_dirty();
	}

	void cTextPrivate::on_removed()
	{
		std::erase_if(element->drawers[0], [&](const auto& i) {
			return i.first == this;
		});
		std::erase_if(element->measurables, [&](const auto& i) {
			return i.first == this;
		});
		element->mark_drawing_dirty();
		element->mark_size_dirty();
	}

	void cTextPrivate::on_entered_world()
	{
		canvas = entity->world->get_system_t<sRendererPrivate>()->canvas;
		fassert(canvas);
		res_id = canvas->find_element_resource("default_font");
		if (res_id != -1)
		{
			atlas = canvas->get_element_resource(res_id).fa;
			if (!atlas)
				res_id = -1;
		}
	}

	void cTextPrivate::on_left_world()
	{
		res_id = -1;
		atlas = nullptr;
	}

	cText* cText::create()
	{
		return f_new<cTextPrivate>();
	}
}
