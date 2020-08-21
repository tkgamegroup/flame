#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include "../world_private.h"
#include "aligner_private.h"
#include "text_private.h"

namespace flame
{
	void cTextPrivate::set_text(const std::wstring& _text)
	{
		text = _text;
		mark_text_changed();
	}

	void cTextPrivate::set_size(uint s)
	{
		if (size == s)
			return;
		size = s;
		if (element)
			element->mark_drawing_dirty();
		mark_size_dirty();
		Entity::report_data_changed(this, S<ch("size")>::v);
	}

	void cTextPrivate::set_color(const Vec4c& col)
	{
		if (color == col)
			return;
		color = col;
		if (element)
			element->mark_drawing_dirty();
		mark_size_dirty();
		Entity::report_data_changed(this, S<ch("color")>::v);
	}

	void cTextPrivate::on_gain_element()
	{
		element->after_drawers.push_back(this);
		element->mark_drawing_dirty();
	}

	void cTextPrivate::on_lost_element()
	{
		std::erase_if(element->after_drawers, [&](const auto& i) {
			return i == this;
		});
		element->mark_drawing_dirty();
	}

	void cTextPrivate::on_gain_type_setting()
	{
		mark_size_dirty();
	}

	void cTextPrivate::on_gain_canvas()
	{
		atlas = canvas->get_resource(0)->get_font_atlas();
	}

	void cTextPrivate::on_lost_canvas()
	{
		atlas = nullptr;
	}

	void cTextPrivate::mark_text_changed()
	{
		if (element)
			element->mark_drawing_dirty();
		mark_size_dirty();
		Entity::report_data_changed(this, S<ch("text")>::v);
	}

	void cTextPrivate::mark_size_dirty()
	{
		if (type_setting && (auto_width || auto_height))
			type_setting->add_to_sizing_list(this, (EntityPrivate*)entity);
	}

	void cTextPrivate::draw(graphics::Canvas* canvas)
	{
		canvas->add_text(0, text.c_str(), nullptr, size, color, element->points[4], element->axes);
	}

	Vec2f cTextPrivate::measure()
	{
		if (!atlas)
			return Vec2f(0.f);
		return Vec2f(atlas->text_size(size, text.c_str()));
	}

	void cTextPrivate::on_added()
	{
		mark_size_dirty();
	}

	void cTextPrivate::on_local_message(Message msg, void* p)
	{
		switch (msg)
		{
		case MessageElementSizeDirty:
		case MessageVisibilityChanged:
			mark_size_dirty();
			break;
		}
	}

	cText* cText::create()
	{
		return f_new<cTextPrivate>();
	}
}
