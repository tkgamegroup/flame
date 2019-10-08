#include <flame/graphics/canvas.h>
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include "text_private.h"
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>

namespace flame
{
	cTextPrivate::cTextPrivate(graphics::FontAtlas* _font_atlas)
	{
		element = nullptr;
		aligner = nullptr;

		font_atlas = _font_atlas;
		color = default_style.text_color_normal;
		sdf_scale = default_style.sdf_scale;
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
		auto rect = element->canvas->add_text(font_atlas, element->global_pos +
			Vec2f(element->inner_padding[0], element->inner_padding[1]) * element->global_scale,
			alpha_mul(color, element->alpha), text.c_str(), sdf_scale * element->global_scale);
		if (auto_width)
		{
			auto w = rect.x() * sdf_scale + element->inner_padding_horizontal();
			element->size.x() = w;
			if (aligner && aligner->width_policy == SizeGreedy)
				aligner->min_size.x() = w;
		}
		if (auto_height)
		{
			auto h = rect.y() * sdf_scale + element->inner_padding_vertical();
			element->size.y() = h;
			if (aligner && aligner->width_policy == SizeGreedy)
				aligner->min_size.y() = h;
		}
	}

	Component* cTextPrivate::copy()
	{
		auto copy = new cTextPrivate(font_atlas);

		copy->color = color;
		copy->sdf_scale = sdf_scale;
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

			e_button->add_component(cStyleColor::create(default_style.button_color_normal, default_style.button_color_hovering, default_style.button_color_active));
		}

		return e_button;
	}

	Entity* wrap_standard_text(Entity* e, bool before, graphics::FontAtlas* font_atlas, float sdf_scale, const std::wstring& text)
	{
		auto e_layout = Entity::create();
		{
			e_layout->add_component(cElement::create());

			auto c_layout = cLayout::create(LayoutHorizontal);
			c_layout->item_padding = 4.f;
			e_layout->add_component(c_layout);
		}

		if (!before)
			e_layout->add_child(e);

		auto e_text = Entity::create();
		e_layout->add_child(e_text);
		{
			e_text->add_component(cElement::create());

			auto c_text = cText::create(font_atlas);
			c_text->sdf_scale = sdf_scale;
			c_text->set_text(text);
			e_text->add_component(c_text);
		}

		if (before)
			e_layout->add_child(e);

		return e_layout;
	}

	struct ComponentText$
	{
		uint font_atlas_index$;
		Vec4c color$;
		float sdf_scale$;
		bool right_align$;
		bool auto_width$;
		bool auto_height$;
		std::wstring text$;

#define FONT_ATLAS_PREFIX "font_atlas"

		FLAME_UNIVERSE_EXPORTS ComponentText$()
		{
			font_atlas_index$ = 1;
			color$ = default_style.text_color_normal;
			sdf_scale$ = default_style.sdf_scale;
			right_align$ = false;
			auto_width$ = false;
			auto_height$ = false;
		}

		FLAME_UNIVERSE_EXPORTS ~ComponentText$()
		{
		}

		FLAME_UNIVERSE_EXPORTS Component* create$()
		{
			auto c = new cTextPrivate((graphics::FontAtlas*)universe_serialization_get_data(FONT_ATLAS_PREFIX + std::to_string(font_atlas_index$)));

			c->color = color$;
			c->sdf_scale = sdf_scale$;
			c->auto_width = auto_width$;
			c->auto_height = auto_height$;
			c->set_text(text$);

			return c;
		}

		FLAME_UNIVERSE_EXPORTS void serialize$(Component* _c, int offset)
		{
			auto c = (cText*)_c;

			if (offset == -1)
			{
				{
					auto& name = universe_serialization_find_data(c->font_atlas);
					assert(name.compare(0, strlen(FONT_ATLAS_PREFIX), FONT_ATLAS_PREFIX) == 0);
					font_atlas_index$ = std::stoul(name.c_str() + strlen(FONT_ATLAS_PREFIX));
				}
				color$ = c->color;
				sdf_scale$ = c->sdf_scale;
				auto_width$ = c->auto_width;
				auto_height$ = c->auto_height;
				text$ = c->text();
			}
			else
			{
				switch (offset)
				{
				case offsetof(ComponentText$, font_atlas_index$):
				{
					auto& name = universe_serialization_find_data(c->font_atlas);
					assert(name.compare(0, strlen(FONT_ATLAS_PREFIX), FONT_ATLAS_PREFIX) == 0);
					font_atlas_index$ = std::stoul(name.c_str() + strlen(FONT_ATLAS_PREFIX));
				}
					break;
				case offsetof(ComponentText$, color$):
					color$ = c->color;
					break;
				case offsetof(ComponentText$, sdf_scale$):
					sdf_scale$ = c->sdf_scale;
					break;
				case offsetof(ComponentText$, auto_width$):
					auto_width$ = c->auto_width;
					break;
				case offsetof(ComponentText$, auto_height$):
					auto_height$ = c->auto_height;
					break;
				case offsetof(ComponentText$, text$):
					text$ = c->text();
					break;
				}
			}
		}

		FLAME_UNIVERSE_EXPORTS void unserialize$(Component* _c, int offset)
		{
			auto c = (cText*)_c;

			if (offset == -1)
			{
				c->font_atlas = (graphics::FontAtlas*)universe_serialization_get_data(FONT_ATLAS_PREFIX + std::to_string(font_atlas_index$));
				c->color = color$;
				c->sdf_scale = sdf_scale$;
				c->auto_width = auto_width$;
				c->auto_height = auto_height$;
				c->set_text(text$);
			}
			else
			{
				switch (offset)
				{
				case offsetof(ComponentText$, font_atlas_index$):
					c->font_atlas = (graphics::FontAtlas*)universe_serialization_get_data(FONT_ATLAS_PREFIX + std::to_string(font_atlas_index$));
					break;
				case offsetof(ComponentText$, color$):
					c->color = color$;
					break;
				case offsetof(ComponentText$, sdf_scale$):
					c->sdf_scale = sdf_scale$;
					break;
				case offsetof(ComponentText$, auto_width$):
					c->auto_width = auto_width$;
					break;
				case offsetof(ComponentText$, auto_height$):
					c->auto_height = auto_height$;
					break;
				case offsetof(ComponentText$, text$):
					c->set_text(text$);
					break;
				}
			}
		}

#undef FONT_ATLAS_PREFIX
	};
}
