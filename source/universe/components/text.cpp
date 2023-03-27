#include "../../graphics/canvas.h"
#include "element_private.h"
#include "text_private.h"

namespace flame
{
	void cTextPrivate::on_init()
	{
		element->drawers.add([this](graphics::CanvasPtr canvas) {
			if (!text.empty() && col.a > 0)
				canvas->add_text(font_atlas, font_size, element->global_pos0(), text, col);
		});
	}

	void cTextPrivate::set_text(const std::wstring& str)
	{
		if (text == str)
			return;
		text = str;
		element->mark_drawing_dirty();
		data_changed("text"_h);

	}

	void cTextPrivate::set_col(const cvec4& _col)
	{
		if (col == _col)
			return;
		col = _col;
		element->mark_drawing_dirty();
		data_changed("col"_h);
	}

	void cTextPrivate::set_font_size(uint size)
	{
		if (font_size == size)
			return;
		font_size = size;
		element->mark_drawing_dirty();
		data_changed("font_size"_h);
	}

	void cTextPrivate::set_font_names(const std::vector<std::wstring>& names)
	{

	}

	void cTextPrivate::set_sdf(bool v)
	{

	}

	struct cTextCreate : cText::Create
	{
		cTextPtr operator()(EntityPtr) override
		{
			return new cTextPrivate();
		}
	}cText_create;
	cText::Create& cText::create = cText_create;
}
