#include "../../graphics/font.h"
#include "../../graphics/canvas.h"
#include "element_private.h"
#include "text_private.h"

namespace flame
{
	cTextPrivate::~cTextPrivate()
	{
		element->drawers.remove("text"_h);
	}

	void cTextPrivate::on_init()
	{
		element->drawers.add([this](graphics::CanvasPtr canvas) {
			if (!text.empty() && col.a > 0)
				canvas->add_text(font_atlas, font_size, element->global_pos0(), text, col, thickness, border);
		}, "text"_h);
	}

	void cTextPrivate::on_active()
	{
		get_font_atlas();
		if (auto_size)
			element->set_ext(calc_text_size());
		element->mark_drawing_dirty();
	}

	void cTextPrivate::set_auto_size(bool v)
	{
		if (auto_size == v)
			return;
		auto_size = v;
		element->mark_drawing_dirty();
		if (auto_size)
			element->set_ext(calc_text_size());
		data_changed("auto_size"_h);
	}

	void cTextPrivate::set_text(const std::wstring& _str)
	{
		std::wstring str; int length = _str.size();
		for (auto i = 0; i < length; i++)
		{
			auto ch = _str[i];
			if (ch == L'\\')
			{
				if (i + 1 < length)
				{
					i++;
					auto ch2 = _str[i];
					if (ch2 == L'\\')
					{
						str += L'\\';
						continue;
					}
					else if (ch2 == L'n')
					{
						str += L'\n';
						continue;
					}
				}
			}

			str += ch;
		}

		if (text == str)
			return;
		text = str;
		if (auto_size)
			element->set_ext(calc_text_size());
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
		if (auto_size)
			element->set_ext(calc_text_size());
		element->mark_drawing_dirty();
		data_changed("font_size"_h);
	}

	void cTextPrivate::set_font_names(const std::vector<std::filesystem::path>& names)
	{
		if (font_names == names)
			return;
		font_names = names;
		get_font_atlas();
		if (auto_size)
			element->set_ext(calc_text_size());
		element->mark_drawing_dirty();
		data_changed("font_names"_h);
	}

	void cTextPrivate::set_sdf(bool v)
	{
		if (sdf == v)
			return;
		sdf = v;
		get_font_atlas();
		if (auto_size)
			element->set_ext(calc_text_size());
		element->mark_drawing_dirty();
		data_changed("sdf"_h);
	}

	void cTextPrivate::set_thickness(float _thickness)
	{
		if (thickness == _thickness)
			return;
		thickness = _thickness;
		element->mark_drawing_dirty();
		data_changed("thickness"_h);
	}

	void cTextPrivate::set_border(float _border)
	{
		if (border == _border)
			return;
		border = _border;
		element->mark_drawing_dirty();
		data_changed("border"_h);
	}

	vec2 cTextPrivate::calc_text_size()
	{
		if (!font_atlas)
			return vec2(0.f);

		auto ret = vec2(0.f, font_size);
		auto scale = font_atlas->get_scale(font_size);
		auto x = 0.f;
		for (auto ch : text)
		{
			if (ch == L'\n')
			{
				ret.y += font_size;
				x = 0.f;
				continue;
			}

			auto& g = font_atlas->get_glyph(ch, font_size);
			auto s = vec2(g.size) * scale;
			ret.x = max(ret.x, x + s.x);
			x += g.advance * scale;
		}
		return ret;
	}

	void cTextPrivate::get_font_atlas()
	{
		auto _font_names = font_names;
		if (_font_names.empty())
			_font_names.push_back(L"flame\\fonts\\OpenSans-Regular.ttf");
		auto _font_atlas = graphics::FontAtlas::get(_font_names, sdf ? graphics::FontAtlasSDF : graphics::FontAtlasBitmap);
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
