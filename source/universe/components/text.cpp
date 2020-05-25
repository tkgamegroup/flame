#include <flame/graphics/canvas.h>
#include <flame/universe/world.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/components/element.h>
#include "text_private.h"
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/aligner.h>

namespace flame
{
	cTextPrivate::cTextPrivate()
	{
		element = nullptr;
		aligner = nullptr;

		font_atlas = nullptr;
		text.resize(1);
		text.v[0] = 0;
		font_size = 0;
		color = 0;
		auto_size = true;

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
			canvas->add_text(font_atlas, text.v, nullptr, font_size * element->global_scale, element->content_min(),
				color.copy().factor_w(element->alpha));
		}
	}

	void cTextPrivate::auto_set_size()
	{
		if (!element)
			return;
		auto s = Vec2f(font_atlas->text_size(font_size, text.v, nullptr)) + Vec2f(element->padding.xz().sum(), element->padding.yw().sum());
		element->set_size(s, this);
		if (aligner)
		{
			aligner->set_min_width(s.x(), this);
			aligner->set_min_height(s.y(), this);
		}
	}

	void cTextPrivate::on_component_added(Component* c)
	{
		if (c->name_hash == FLAME_CHASH("cElement"))
		{
			element = (cElement*)c;
			draw_cmd = element->cmds.add([](Capture& c, graphics::Canvas* canvas) {
				c.thiz<cTextPrivate>()->draw(canvas);
				return true;
			}, Capture().set_thiz(this));
			if (element->management && auto_size)
				element->management->add_to_sizing_list(element);
		}
		else if (c->name_hash == FLAME_CHASH("cAligner"))
			aligner = (cAligner*)c;
	}

	void cTextPrivate::on_visibility_changed()
	{
		if (element->management && auto_size)
			element->management->add_to_sizing_list(element);
	}

	void cText::set_text(const wchar_t* text, int length, void* sender)
	{
		auto thiz = (cTextPrivate*)this;
		if (text)
		{
			if (length == -1)
				length = wcslen(text);
			if (thiz->text.compare(text, length))
				return;
			thiz->text.assign(text, length);
		}
		if (element)
		{
			element->mark_dirty();
			if (element->management && auto_size)
				element->management->add_to_sizing_list(element);
		}
		data_changed(FLAME_CHASH("text"), sender);
	}

	void cText::set_font_size(uint s, void* sender)
	{
		if (s == font_size)
			return;
		font_size = s;
		if (element)
		{
			element->mark_dirty();
			if (element->management && auto_size)
				element->management->add_to_sizing_list(element);
		}
		data_changed(FLAME_CHASH("font_size"), sender);
	}

	void cText::set_color(const Vec4c& c, void* sender)
	{
		if (c == color)
			return;
		color = c;
		if (element)
		{
			element->mark_dirty();
			if (element->management && auto_size)
				element->management->add_to_sizing_list(element);
		}
		data_changed(FLAME_CHASH("color"), sender);
	}

	void cText::set_auto_size(bool v, void* sender)
	{
		if (v == auto_size)
			return;
		auto_size = v;
		auto thiz = (cTextPrivate*)this;
		if (thiz->auto_size)
		{
			if (element)
			{
				element->mark_dirty();
				if (element->management)
					element->management->add_to_sizing_list(element);
			}
		}
		data_changed(FLAME_CHASH("auto_size"), sender);
	}

	cText* cText::create()
	{
		return new cTextPrivate();
	}
}
