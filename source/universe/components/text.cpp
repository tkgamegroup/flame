#include <flame/graphics/canvas.h>
#include <flame/universe/world.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/components/element.h>
#include "text_private.h"
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/utils/style.h>

namespace flame
{
	cTextPrivate::cTextPrivate(graphics::FontAtlas* _font_atlas)
	{
		element = nullptr;

		font_atlas = _font_atlas;
		font_size_ = utils::style_1u(utils::FontSize);
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
		if (!element->clipped)
		{
			assert(font_atlas->canvas_slot_ != -1);
			canvas->add_text(font_atlas, text.c_str(), text.c_str() + text.size(), font_size_ * element->global_scale, element->content_min(),
				color_.new_proply<3>(element->alpha));
		}
	}

	void cTextPrivate::on_text_changed(void* sender)
	{
		if (element)
			element->mark_dirty();
		data_changed(FLAME_CHASH("text"), sender);
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
		thiz->on_text_changed(sender);
	}

	void cText::set_font_size(uint s, void* sender)
	{
		if (s == font_size_)
			return;
		font_size_ = s;
		if (element)
			element->mark_dirty();
		data_changed(FLAME_CHASH("font_size"), sender);
	}

	void cText::set_color(const Vec4c& c, void* sender)
	{
		if (c == color_)
			return;
		color_ = c;
		if (element)
			element->mark_dirty();
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
}
