#include <flame/graphics/font.h>
#include <flame/universe/world.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/ui/style_stack.h>

#include "../renderpath/canvas/canvas.h"

namespace flame
{
	struct cTextPrivate : cText
	{
		std::wstring text;
		std::vector<graphics::Glyph*> glyphs;
		void* draw_cmd;

		cTextPrivate(graphics::FontAtlas* _font_atlas)
		{
			element = nullptr;

			font_atlas = _font_atlas;
			color = ui::style_4c(ui::TextColorNormal);
			font_size_ = ui::style_1u(ui::FontSize);
			scale_ = 1.f;
			auto_width_ = true;
			auto_height_ = true;

			last_font_size = 0;
			last_scale = 0.f;

			draw_cmd = nullptr;
		}

		cTextPrivate::~cTextPrivate()
		{
			if (!entity->dying_)
				element->cmds.remove(draw_cmd);
		}

		void update_glyphs()
		{
			glyphs.resize(text.size());
			for (auto i = 0; i < text.size(); i++)
				glyphs[i] = font_atlas->get_glyph(text[i], last_font_size);
		}

		void draw(graphics::Canvas* canvas)
		{
			if (!element->cliped)
			{
				auto is_sdf = font_atlas->draw_type == graphics::FontDrawSdf;
				auto global_scale = element->global_scale;
				auto fs = font_size_;
				if (!is_sdf)
					fs *= global_scale;
				if (fs != last_font_size)
				{
					last_font_size = fs;
					update_glyphs();
				}
				last_scale = scale_ * (is_sdf ? global_scale : 1.f);
				canvas->add_text(font_atlas, glyphs.size(), glyphs.data(), last_font_size, last_scale, element->global_pos +
					Vec2f(element->inner_padding_[0], element->inner_padding_[1]) * global_scale,
					color.new_proply<3>(element->alpha_));
			}
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cElement"))
			{
				element = (cElement*)c;
				draw_cmd = element->cmds.add([](void* c, graphics::Canvas* canvas) {
					(*(cTextPrivate**)c)->draw(canvas);
				}, new_mail_p(this));
			}
		}

		Component* copy() override
		{
			auto copy = new cTextPrivate(font_atlas);

			copy->color = color;
			copy->font_size_ = font_size_;
			copy->auto_width_ = auto_width_;
			copy->auto_height_ = auto_height_;
			copy->text = text;

			return copy;
		}
	};

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
		thiz->text = text;
		thiz->update_glyphs();
		data_changed(FLAME_CHASH("text"), sender);
	}

	void cText::insert_char(wchar_t ch, uint pos, void* sender)
	{
		auto thiz = (cTextPrivate*)this;
		auto& str = thiz->text;
		str.insert(str.begin() + pos, ch);
		thiz->update_glyphs();
		data_changed(FLAME_CHASH("text"), sender);
	}

	void cText::erase_char(uint pos, void* sender)
	{
		auto thiz = (cTextPrivate*)this;
		auto& str = thiz->text;
		str.erase(str.begin() + pos);
		thiz->update_glyphs();
		data_changed(FLAME_CHASH("text"), sender);
	}

	void cText::set_font_size(uint s, void* sender)
	{
		if (s == font_size_)
			return;
		font_size_ = s;
		((cTextPrivate*)this)->update_glyphs();
		data_changed(FLAME_CHASH("font_size"), sender);
	}

	void cText::set_scale(float s, void* sender)
	{
		if (s == scale_)
			return;
		scale_ = s;
		data_changed(FLAME_CHASH("scale"), sender);
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

	struct Serializer_cText$
	{
		uint font_atlas_id$;
		Vec4c color$;
		uint font_size$;
		bool right_align$;
		bool auto_width$;
		bool auto_height$;
		StringW text$;

		FLAME_UNIVERSE_EXPORTS Serializer_cText$()
		{
			color$ = ui::style_4c(ui::TextColorNormal);
			font_size$ = ui::style_1u(ui::FontSize);
			right_align$ = false;
			auto_width$ = false;
			auto_height$ = false;
		}

		FLAME_UNIVERSE_EXPORTS ~Serializer_cText$()
		{
		}

		FLAME_UNIVERSE_EXPORTS Component* create$(World* w)
		{
			auto c = new cTextPrivate((graphics::FontAtlas*)w->find_object(FLAME_CHASH("FontAtlas"), font_atlas_id$));

			c->color = color$;
			c->font_size_ = font_size$;
			c->auto_width_ = auto_width$;
			c->auto_height_ = auto_height$;
			c->set_text(text$.v);

			return c;
		}

		FLAME_UNIVERSE_EXPORTS void serialize$(Component* _c, int offset)
		{
			auto c = (cTextPrivate*)_c;
			auto w = c->entity->world_;

			if (offset == -1)
			{
				font_atlas_id$ = c->font_atlas->id;
				color$ = c->color;
				font_size$ = c->font_size_;
				auto_width$ = c->auto_width_;
				auto_height$ = c->auto_height_;
				text$ = c->text;
			}
			else
			{
				switch (offset)
				{
				case offsetof(Serializer_cText$, font_atlas_id$):
					font_atlas_id$ = c->font_atlas->id;
					break;
				case offsetof(Serializer_cText$, color$):
					color$ = c->color;
					break;
				case offsetof(Serializer_cText$, font_size$):
					font_size$ = c->font_size_;
					break;
				case offsetof(Serializer_cText$, auto_width$):
					auto_width$ = c->auto_width_;
					break;
				case offsetof(Serializer_cText$, auto_height$):
					auto_height$ = c->auto_height_;
					break;
				case offsetof(Serializer_cText$, text$):
					text$ = c->text;
					break;
				}
			}
		}

		FLAME_UNIVERSE_EXPORTS void unserialize$(Component* _c, int offset)
		{
			auto c = (cTextPrivate*)_c;
			auto w = c->entity->world_;

			if (offset == -1)
			{
				c->font_atlas = (graphics::FontAtlas*)w->find_object(FLAME_CHASH("FontAtlas"), font_atlas_id$);
				c->color = color$;
				c->font_size_ = font_size$;
				c->auto_width_ = auto_width$;
				c->auto_height_ = auto_height$;
				c->set_text(text$.v);
			}
			else
			{
				switch (offset)
				{
				case offsetof(Serializer_cText$, font_atlas_id$):
					c->font_atlas = (graphics::FontAtlas*)w->find_object(FLAME_CHASH("FontAtlas"), font_atlas_id$);
					break;
				case offsetof(Serializer_cText$, color$):
					c->color = color$;
					break;
				case offsetof(Serializer_cText$, font_size$):
					c->font_size_ = font_size$;
					break;
				case offsetof(Serializer_cText$, auto_width$):
					c->auto_width_ = auto_width$;
					break;
				case offsetof(Serializer_cText$, auto_height$):
					c->auto_height_ = auto_height$;
					break;
				case offsetof(Serializer_cText$, text$):
					c->set_text(text$.v);
					break;
				}
			}
		}
	};
}
