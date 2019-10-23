#include <flame/graphics/font.h>
#include <flame/universe/world.h>
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include "text_private.h"
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>

#include "../renderpath/canvas_make_cmd/canvas.h"

namespace flame
{
	cTextPrivate::cTextPrivate(graphics::FontAtlas* _font_atlas)
	{
		element = nullptr;

		font_atlas = _font_atlas;
		color = default_style.text_color_normal;
		sdf_scale_ = default_style.sdf_scale;
		auto_width_ = true;
		auto_height_ = true;
	}

	void cTextPrivate::draw(graphics::Canvas* canvas)
	{
		canvas->add_text(font_atlas, element->global_pos +
			Vec2f(element->inner_padding_[0], element->inner_padding_[1]) * element->global_scale,
			alpha_mul(color, element->alpha), text.c_str(), sdf_scale_ * element->global_scale);
	}

	void cTextPrivate::on_component_added(Component* c)
	{
		if (c->name_hash == cH("Element"))
			element = (cElement*)c;
	}

	Component* cTextPrivate::copy()
	{
		auto copy = new cTextPrivate(font_atlas);

		copy->color = color;
		copy->sdf_scale_ = sdf_scale_;
		copy->auto_width_ = auto_width_;
		copy->auto_height_ = auto_height_;
		copy->text = text;

		return copy;
	}

	const std::wstring& cText::text() const
	{
		return ((cTextPrivate*)this)->text;
	}

	void cText::set_text(const std::wstring& text, void* sender)
	{
		((cTextPrivate*)this)->text = text;
		data_changed(cH("text"), sender);
	}

	void cText::insert_char(wchar_t ch, uint pos, void* sender)
	{
		auto& str = ((cTextPrivate*)this)->text;
		str.insert(str.begin() + pos, ch);
		data_changed(cH("text"), sender);
	}

	void cText::erase_char(uint pos, void* sender)
	{
		auto& str = ((cTextPrivate*)this)->text;
		str.erase(str.begin() + pos);
		data_changed(cH("text"), sender);
	}

	void cText::set_sdf_scale(float s, void* sender)
	{
		if (s == sdf_scale_)
			return;
		sdf_scale_ = s;
		data_changed(cH("sdf_scale"), sender);
	}

	void cText::set_auto_width(bool v, void* sender)
	{
		if (v == auto_width_)
			return;
		auto_width_ = v;
		data_changed(cH("auto_width"), sender);
	}

	void cText::set_auto_height(bool v, void* sender)
	{
		if (v == auto_height_)
			return;
		auto_height_ = v;
		data_changed(cH("auto_height"), sender);
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
			c_element->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
			e_button->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->sdf_scale_ = sdf_scale;
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
			c_text->sdf_scale_ = sdf_scale;
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

		FLAME_UNIVERSE_EXPORTS Component* create$(Universe* u)
		{
			auto c = new cTextPrivate((graphics::FontAtlas*)u->bank_get(FONT_ATLAS_PREFIX + std::to_string(font_atlas_index$)));

			c->color = color$;
			c->sdf_scale_ = sdf_scale$;
			c->auto_width_ = auto_width$;
			c->auto_height_ = auto_height$;
			c->set_text(text$);

			return c;
		}

		FLAME_UNIVERSE_EXPORTS void serialize$(Component* _c, int offset)
		{
			auto c = (cText*)_c;
			auto u = c->entity->world_->universe_;

			if (offset == -1)
			{
				{
					auto& name = u->bank_find(c->font_atlas);
					assert(name.compare(0, strlen(FONT_ATLAS_PREFIX), FONT_ATLAS_PREFIX) == 0);
					font_atlas_index$ = std::stoul(name.c_str() + strlen(FONT_ATLAS_PREFIX));
				}
				color$ = c->color;
				sdf_scale$ = c->sdf_scale_;
				auto_width$ = c->auto_width_;
				auto_height$ = c->auto_height_;
				text$ = c->text();
			}
			else
			{
				switch (offset)
				{
				case offsetof(ComponentText$, font_atlas_index$):
				{
					auto& name = u->bank_find(c->font_atlas);
					assert(name.compare(0, strlen(FONT_ATLAS_PREFIX), FONT_ATLAS_PREFIX) == 0);
					font_atlas_index$ = std::stoul(name.c_str() + strlen(FONT_ATLAS_PREFIX));
				}
					break;
				case offsetof(ComponentText$, color$):
					color$ = c->color;
					break;
				case offsetof(ComponentText$, sdf_scale$):
					sdf_scale$ = c->sdf_scale_;
					break;
				case offsetof(ComponentText$, auto_width$):
					auto_width$ = c->auto_width_;
					break;
				case offsetof(ComponentText$, auto_height$):
					auto_height$ = c->auto_height_;
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
			auto u = c->entity->world_->universe_;

			if (offset == -1)
			{
				c->font_atlas = (graphics::FontAtlas*)u->bank_get(FONT_ATLAS_PREFIX + std::to_string(font_atlas_index$));
				c->color = color$;
				c->sdf_scale_ = sdf_scale$;
				c->auto_width_ = auto_width$;
				c->auto_height_ = auto_height$;
				c->set_text(text$);
			}
			else
			{
				switch (offset)
				{
				case offsetof(ComponentText$, font_atlas_index$):
					c->font_atlas = (graphics::FontAtlas*)u->bank_get(FONT_ATLAS_PREFIX + std::to_string(font_atlas_index$));
					break;
				case offsetof(ComponentText$, color$):
					c->color = color$;
					break;
				case offsetof(ComponentText$, sdf_scale$):
					c->sdf_scale_ = sdf_scale$;
					break;
				case offsetof(ComponentText$, auto_width$):
					c->auto_width_ = auto_width$;
					break;
				case offsetof(ComponentText$, auto_height$):
					c->auto_height_ = auto_height$;
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
