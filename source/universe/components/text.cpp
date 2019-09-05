#include <flame/graphics/canvas.h>
#include <flame/universe/components/element.h>
#include "text_private.h"
#include <flame/universe/components/aligner.h>
#include <flame/universe/default_style.h>

namespace flame
{
	cTextPrivate::cTextPrivate(graphics::FontAtlas* _font_atlas)
	{
		element = nullptr;
		aligner = nullptr;

		font_atlas = _font_atlas;
		color = default_style.text_color_normal;
		sdf_scale = default_style.sdf_scale;
		right_align = false;
		auto_size = true;
	}

	void cTextPrivate::start()
	{
		element = (cElement*)(entity->find_component(cH("Element")));
		assert(element);
		aligner = (cAligner*)(entity->find_component(cH("Aligner")));
	}

	void cTextPrivate::update()
	{
		if (!right_align)
		{
			auto rect = element->canvas->add_text(font_atlas, Vec2f(element->global_x, element->global_y) +
				Vec2f(element->inner_padding[0], element->inner_padding[1]) * element->global_scale,
				alpha_mul(color, element->alpha), text.c_str(), sdf_scale * element->global_scale);
			if (auto_size)
			{
				auto w = rect.x() + element->inner_padding[0] + element->inner_padding[2];
				if (!aligner || aligner->width_policy != SizeGreedy || w > aligner->min_width)
					element->width = w;
				auto h = rect.y() + element->inner_padding[1] + element->inner_padding[3];
				if (!aligner || aligner->height_policy != SizeGreedy || h > aligner->min_height)
					element->height = h;
			}
		}
		else
		{
			element->canvas->add_text_right_align(font_atlas, Vec2f(element->global_x, element->global_y) +
				Vec2f(element->inner_padding[0], element->inner_padding[1]) * element->global_scale,
				alpha_mul(color, element->alpha), text.c_str(), sdf_scale * element->global_scale);
		}
	}

	Component* cTextPrivate::copy()
	{
		auto copy = new cTextPrivate(font_atlas);

		copy->color = color;
		copy->sdf_scale = sdf_scale;
		copy->right_align = right_align;
		copy->auto_size = auto_size;
		copy->text = text;

		return copy;
	}

	const std::wstring& cText::text() const
	{
		return ((cTextPrivate*)this)->text;
	}

	void cText::set_text(const std::wstring& text)
	{
		((cTextPrivate*)this)->text = text;
	}

	void cText::start()
	{
		((cTextPrivate*)this)->start();
	}

	void cText::update()
	{
		((cTextPrivate*)this)->update();
	}

	Component* cText::copy()
	{
		return ((cTextPrivate*)this)->copy();
	}

	cText* cText::create(graphics::FontAtlas* font_atlas)
	{
		return new cTextPrivate(font_atlas);
	}

	struct cTextA$
	{
		uint font_atlas_index$;
		Vec4c color$;
		float sdf_scale$;

		FLAME_UNIVERSE_EXPORTS static cText* create()
		{
			//font_atlas = nullptr; // TODO
			//color = src->color$;
			//sdf_scale = src->sdf_scale$;
			return nullptr;
		}

		FLAME_UNIVERSE_EXPORTS static void save(cText* t)
		{

		}
	};
}
