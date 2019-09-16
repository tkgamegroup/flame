#include <flame/graphics/canvas.h>
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include "text_private.h"
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/aligner.h>

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
		auto_width = true;
		auto_height = true;
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
			if (auto_width)
			{
				auto w = rect.x() + element->inner_padding_horizontal() * element->global_scale;
				if (!aligner || aligner->width_policy != SizeGreedy || w > aligner->min_width)
					element->width = w;
			}
			if (auto_height)
			{
				auto h = rect.y() + element->inner_padding_vertical() * element->global_scale;
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
		copy->auto_width = auto_width;
		copy->auto_height = auto_height;
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

	Entity* create_standard_button(graphics::FontAtlas* font_atlas, float sdf_scale, const std::wstring& text)
	{
		auto e_button = Entity::create();
		{
			auto c_element = cElement::create();
			c_element->inner_padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			e_button->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->sdf_scale = sdf_scale;
			c_text->set_text(text);
			e_button->add_component(c_text);

			e_button->add_component(cEventReceiver::create());

			e_button->add_component(cStyleBackgroundColor::create(default_style.button_color_normal, default_style.button_color_hovering, default_style.button_color_active));
		}

		return e_button;
	}
}
