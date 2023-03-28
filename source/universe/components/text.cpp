#include "../../graphics/font.h"
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

	void cTextPrivate::set_font_names(const std::vector<std::filesystem::path>& names)
	{
		if (font_names == names)
			return;
		font_names = names;
		get_font_atlas();
		element->mark_drawing_dirty();
		data_changed("font_names"_h);
	}

	void cTextPrivate::set_sdf(bool v)
	{
		if (sdf == v)
			return;
		sdf = v;
		get_font_atlas();
		element->mark_drawing_dirty();
		data_changed("sdf"_h);
	}

	void cTextPrivate::get_font_atlas()
	{
		auto _font_names = font_names;
		if (_font_names.empty())
			_font_names.push_back(L"flame\\fonts\\OpenSans-Regular.ttf");
		auto _font_atlas = graphics::FontAtlas::get(font_names, sdf ? graphics::FontAtlasSDF : graphics::FontAtlasBitmap);
		if (font_atlas != _font_atlas)
		{
			if (font_atlas)
				graphics::FontAtlas::release(font_atlas);
			font_atlas = _font_atlas;
		}
		else if (_font_atlas)
			graphics::FontAtlas::release(_font_atlas);
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
