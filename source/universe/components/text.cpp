#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include "../world_private.h"
#include "element_private.h"
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
		{
			element->mark_drawing_dirty();
			element->mark_size_dirty();
		}
		Entity::report_data_changed(this, S<ch("size")>::v);
	}

	void cTextPrivate::set_color(const Vec4c& col)
	{
		if (color == col)
			return;
		color = col;
		if (element)
		{
			element->mark_drawing_dirty();
			element->mark_size_dirty();
		}
		Entity::report_data_changed(this, S<ch("color")>::v);
	}

	void cTextPrivate::on_gain_canvas()
	{
		res_id = canvas->find_element_resource("default_font");
		if (res_id != -1)
		{
			atlas = canvas->get_element_resource(res_id).fa;
			if (!atlas)
				res_id = -1;
		}
	}

	void cTextPrivate::on_lost_canvas()
	{
		res_id = -1;
		atlas = nullptr;
	}

	void cTextPrivate::mark_text_changed()
	{
		if (element)
		{
			element->mark_drawing_dirty();
			element->mark_size_dirty();
		}
		Entity::report_data_changed(this, S<ch("text")>::v);
	}

	void cTextPrivate::draw(graphics::Canvas* canvas)
	{
		canvas->draw_text(res_id, text.c_str(), nullptr, size, color, element->points[4], element->axes);
	}

	void cTextPrivate::measure(Vec2f& ret)
	{
		if (!atlas || (!auto_width && !auto_height))
		{
			ret = Vec2f(-1.f);
			return;
		}
		ret = Vec2f(atlas->text_size(size, text.c_str()));
		if (!auto_width)
			ret.x() = -1.f;
		if (!auto_height)
			ret.y() = -1.f;
	}

	cText* cText::create()
	{
		return f_new<cTextPrivate>();
	}
}
