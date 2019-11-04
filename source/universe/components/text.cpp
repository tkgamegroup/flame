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
		font_size_ = default_style.font_size;
		scale_ = 1.f;
		auto_width_ = true;
		auto_height_ = true;

		draw_font_size = 0.f;
	}

	void cTextPrivate::update_glyphs()
	{
		glyphs.resize(text.size());
		for (auto i = 0; i < text.size(); i++)
			glyphs[i] = font_atlas->get_glyph(text[i], draw_font_size);
	}

	void cTextPrivate::draw(graphics::Canvas* canvas)
	{
		auto is_sdf = font_atlas->draw_type == graphics::FontDrawSdf;
		auto global_scale = element->global_scale;
		auto fs = font_size_;
		if (!is_sdf)
			fs *= global_scale;
		if (fs != draw_font_size)
		{
			draw_font_size = fs;
			update_glyphs();
		}
		canvas->add_text(font_atlas, glyphs, draw_font_size, scale_ * (is_sdf ? global_scale : 1.f), element->global_pos +
			Vec2f(element->inner_padding_[0], element->inner_padding_[1]) * global_scale,
			alpha_mul(color, element->alpha));
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
		copy->font_size_ = font_size_;
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
		auto thiz = (cTextPrivate*)this;
		thiz->text = text;
		thiz->update_glyphs();
		data_changed(cH("text"), sender);
	}

	void cText::insert_char(wchar_t ch, uint pos, void* sender)
	{
		auto thiz = (cTextPrivate*)this;
		auto& str = thiz->text;
		str.insert(str.begin() + pos, ch);
		thiz->update_glyphs();
		data_changed(cH("text"), sender);
	}

	void cText::erase_char(uint pos, void* sender)
	{
		auto thiz = (cTextPrivate*)this;
		auto& str = thiz->text;
		str.erase(str.begin() + pos);
		thiz->update_glyphs();
		data_changed(cH("text"), sender);
	}

	void cText::set_font_size(uint s, void* sender)
	{
		if (s == font_size_)
			return;
		font_size_ = s;
		((cTextPrivate*)this)->update_glyphs();
		data_changed(cH("font_size"), sender);
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

	Entity* create_standard_button(graphics::FontAtlas* font_atlas, float font_size_scale, const std::wstring& text)
	{
		auto e_button = Entity::create();
		{
			auto c_element = cElement::create();
			c_element->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
			e_button->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->font_size_ = default_style.font_size * font_size_scale;
			c_text->set_text(text);
			e_button->add_component(c_text);

			e_button->add_component(cEventReceiver::create());

			e_button->add_component(cStyleColor::create(default_style.button_color_normal, default_style.button_color_hovering, default_style.button_color_active));
		}

		return e_button;
	}

	Entity* wrap_standard_text(Entity* e, bool before, graphics::FontAtlas* font_atlas, float font_size_scale, const std::wstring& text)
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
			c_text->font_size_ = default_style.font_size * font_size_scale;
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
		uint font_size$;
		bool right_align$;
		bool auto_width$;
		bool auto_height$;
		std::wstring text$;

		FLAME_UNIVERSE_EXPORTS ComponentText$()
		{
			font_atlas_index$ = 1;
			color$ = default_style.text_color_normal;
			font_size$ = default_style.font_size;
			right_align$ = false;
			auto_width$ = false;
			auto_height$ = false;
		}

		FLAME_UNIVERSE_EXPORTS ~ComponentText$()
		{
		}

		FLAME_UNIVERSE_EXPORTS Component* create$(World* w)
		{
			auto c = new cTextPrivate((graphics::FontAtlas*)w->find_object(cH("FontAtlas"), std::to_string(font_atlas_index$)));

			c->color = color$;
			c->font_size_ = font_size$;
			c->auto_width_ = auto_width$;
			c->auto_height_ = auto_height$;
			c->set_text(text$);

			return c;
		}

		FLAME_UNIVERSE_EXPORTS void serialize$(Component* _c, int offset)
		{
			auto c = (cText*)_c;
			auto w = c->entity->world_;

			if (offset == -1)
			{
				font_atlas_index$ = std::stoul(*w->find_id(c->font_atlas));
				color$ = c->color;
				font_size$ = c->font_size_;
				auto_width$ = c->auto_width_;
				auto_height$ = c->auto_height_;
				text$ = c->text();
			}
			else
			{
				switch (offset)
				{
				case offsetof(ComponentText$, font_atlas_index$):
					font_atlas_index$ = std::stoul(*w->find_id(c->font_atlas));
					break;
				case offsetof(ComponentText$, color$):
					color$ = c->color;
					break;
				case offsetof(ComponentText$, font_size$):
					font_size$ = c->font_size_;
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
			auto w = c->entity->world_;

			if (offset == -1)
			{
				c->font_atlas = (graphics::FontAtlas*)w->find_object(cH("FontAtlas"), std::to_string(font_atlas_index$));
				c->color = color$;
				c->font_size_ = font_size$;
				c->auto_width_ = auto_width$;
				c->auto_height_ = auto_height$;
				c->set_text(text$);
			}
			else
			{
				switch (offset)
				{
				case offsetof(ComponentText$, font_atlas_index$):
					c->font_atlas = (graphics::FontAtlas*)w->find_object(cH("FontAtlas"), std::to_string(font_atlas_index$));
					break;
				case offsetof(ComponentText$, color$):
					c->color = color$;
					break;
				case offsetof(ComponentText$, font_size$):
					c->font_size_ = font_size$;
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
	};
}
