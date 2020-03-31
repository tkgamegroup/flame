#include <flame/graphics/font.h>
#include <flame/universe/world.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/components/element.h>
#include "text_private.h"
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/utils/style.h>

#include "../renderpath/canvas/canvas.h"

#include <flame/reflect_macros.h>

namespace flame
{
	cTextPrivate::cTextPrivate(graphics::FontAtlas* _font_atlas)
	{
		element = nullptr;

		font_atlas = _font_atlas;
		font_size_ = utils::style_1u(utils::FontSize);
		scale_ = 1.f;
		color_ = utils::style_4c(utils::TextColorNormal);
		auto_width_ = true;
		auto_height_ = true;

		draw_cmd = nullptr;
	}

	cTextPrivate::cTextPrivate::~cTextPrivate()
	{
		if (!entity->dying_)
			element->cmds.remove(draw_cmd);
	}

	void cTextPrivate::draw(graphics::Canvas* canvas)
	{
		if (!element->cliped)
		{
			if (font_atlas->draw_type == graphics::FontDrawSdf)
			{
				canvas->add_text(font_atlas, text.c_str(), 0, scale_ * element->global_scale, element->global_pos +
					Vec2f(element->inner_padding_[0], element->inner_padding_[1]) * element->global_scale,
					color_.new_proply<3>(element->alpha_));
			}
			else
			{
				canvas->add_text(font_atlas, text.c_str(), font_size_ * element->global_scale, 1.f, element->global_pos +
					Vec2f(element->inner_padding_[0], element->inner_padding_[1]) * element->global_scale,
					color_.new_proply<3>(element->alpha_));
			}
		}
	}

	void cTextPrivate::on_component_added(Component* c)
	{
		if (c->name_hash == FLAME_CHASH("cElement"))
		{
			element = (cElement*)c;
			draw_cmd = element->cmds.add([](void* c, graphics::Canvas* canvas) {
				(*(cTextPrivate**)c)->draw(canvas);
				return true;
			}, Mail::from_p(this));
		}
	}

	Component* cTextPrivate::copy()
	{
		auto copy = new cTextPrivate(font_atlas);

		copy->text = text;
		copy->font_size_ = font_size_;
		copy->scale_ = scale_;
		copy->color_ = color_;
		copy->auto_width_ = auto_width_;
		copy->auto_height_ = auto_height_;

		return copy;
	}

	uint cText::text_length() const
	{
		return ((cTextPrivate*)this)->text.size();
	}

	const wchar_t* cText::text() const
	{
		return ((cTextPrivate*)this)->text.c_str();
	}

	void cText::set_text(const wchar_t* text, void* sender)
	{
		auto thiz = (cTextPrivate*)this;
		if (thiz->text == text)
			return;
		thiz->text = text;
		if (element && element->renderer)
			element->renderer->pending_update = true;
		data_changed(FLAME_CHASH("text"), sender);
	}

	void cText::set_font_size(uint s, void* sender)
	{
		if (s == font_size_)
			return;
		font_size_ = s;
		if (element && element->renderer)
			element->renderer->pending_update = true;
		data_changed(FLAME_CHASH("font_size"), sender);
	}

	void cText::set_scale(float s, void* sender)
	{
		if (s == scale_)
			return;
		scale_ = s;
		if (element && element->renderer)
			element->renderer->pending_update = true;
		data_changed(FLAME_CHASH("scale"), sender);
	}

	void cText::set_color(const Vec4c& c, void* sender)
	{
		if (c == color_)
			return;
		color_ = c;
		if (element && element->renderer)
			element->renderer->pending_update = true;
		data_changed(FLAME_CHASH("color"), sender);
	}

	void cText::set_auto_width(bool v, void* sender)
	{
		if (v == auto_width_)
			return;
		auto_width_ = v;
		data_changed(FLAME_CHASH("auto_width"), sender);
	}

	void cText::set_auto_height(bool v, void* sender)
	{
		if (v == auto_height_)
			return;
		auto_height_ = v;
		data_changed(FLAME_CHASH("auto_height"), sender);
	}


	cText* cText::create(graphics::FontAtlas* font_atlas)
	{
		return new cTextPrivate(font_atlas);
	}

	struct R(Serializer_cText)
	{
		RV(uint, font_atlas_id, n);
		RV(StringW, text, n);
		RV(uint, font_size, n);
		RV(float, scale, n);
		RV(Vec4c, color, n);
		RV(bool, auto_width, n);
		RV(bool, auto_height, n);

		FLAME_UNIVERSE_EXPORTS RF(Serializer_cText)()
		{
			color = utils::style_4c(utils::TextColorNormal);
			font_size = utils::style_1u(utils::FontSize);
			auto_width = false;
			auto_height = false;
		}

		FLAME_UNIVERSE_EXPORTS RF(~Serializer_cText)()
		{
		}

		FLAME_UNIVERSE_EXPORTS Component* RF(create)(World* w)
		{
			auto c = new cTextPrivate((graphics::FontAtlas*)w->find_object(FLAME_CHASH("FontAtlas"), font_atlas_id));

			c->set_text(text.v);
			c->font_size_ = font_size;
			c->scale_ = scale;
			c->color_ = color;
			c->auto_width_ = auto_width;
			c->auto_height_ = auto_height;

			return c;
		}

		FLAME_UNIVERSE_EXPORTS void RF(serialize)(Component* _c, int offset)
		{
			auto c = (cTextPrivate*)_c;

			if (offset == -1)
			{
				font_atlas_id = c->font_atlas->id;
				text = c->text;
				font_size = c->font_size_;
				scale = c->scale_;
				color = c->color_;
				auto_width = c->auto_width_;
				auto_height = c->auto_height_;
			}
			else
			{
				switch (offset)
				{
				case offsetof(Serializer_cText, font_atlas_id):
					font_atlas_id = c->font_atlas->id;
					break;
				case offsetof(Serializer_cText, text):
					text = c->text;
					break;
				case offsetof(Serializer_cText, font_size):
					font_size = c->font_size_;
					break;
				case offsetof(Serializer_cText, scale):
					scale = c->scale_;
					break;
				case offsetof(Serializer_cText, color):
					color = c->color_;
					break;
				case offsetof(Serializer_cText, auto_width):
					auto_width = c->auto_width_;
					break;
				case offsetof(Serializer_cText, auto_height):
					auto_height = c->auto_height_;
					break;
				}
			}
		}

		FLAME_UNIVERSE_EXPORTS void  RF(unserialize)(Component* _c, int offset)
		{
			auto c = (cTextPrivate*)_c;
			auto w = c->entity->world();

			if (offset == -1)
			{
				c->font_atlas = (graphics::FontAtlas*)w->find_object(FLAME_CHASH("FontAtlas"), font_atlas_id);
				c->set_text(text.v);
				c->font_size_ = font_size;
				c->scale_ = scale;
				c->color_ = color;
				c->auto_width_ = auto_width;
				c->auto_height_ = auto_height;
			}
			else
			{
				switch (offset)
				{
				case offsetof(Serializer_cText, font_atlas_id):
					c->font_atlas = (graphics::FontAtlas*)w->find_object(FLAME_CHASH("FontAtlas"), font_atlas_id);
					break;
				case offsetof(Serializer_cText, text):
					c->set_text(text.v);
					break;
				case offsetof(Serializer_cText, font_size):
					c->font_size_ = font_size;
					break;
				case offsetof(Serializer_cText, scale):
					c->scale_ = scale;
					break;
				case offsetof(Serializer_cText, color):
					c->color_ = color;
					break;
				case offsetof(Serializer_cText, auto_width):
					c->auto_width_ = auto_width;
					break;
				case offsetof(Serializer_cText, auto_height):
					c->auto_height_ = auto_height;
					break;
				}
			}
		}
	};
}
