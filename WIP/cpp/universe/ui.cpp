#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/descriptor.h>
#include "ui_private.h"

namespace flame
{
	UIPrivate::UIPrivate(graphics::Canvas* _canvas, Window * w)
	{
		auto w_debug = Element::createT<wDebug>(this);
		root_->add_child(w_debug, 1);
	}

	inline void UIPrivate::set_hovering_element(Element * w)
	{
		if (w == hovering_element_)
			return;
		if (hovering_element_)
			hovering_element_->on_mouse(KeyStateUp, Mouse_Null, Vec2(0.f));
		hovering_element_ = w;
		if (hovering_element_)
			hovering_element_->on_mouse(KeyStateDown, Mouse_Null, Vec2(0.f));
	}

	inline void UIPrivate::set_focus_element(Element * w)
	{
		auto old = focus_element_;
		focus_element_ = w;
		if (old)
			old->on_focus(Focus_Lost, 0);
		if (focus_element_)
			focus_element_->on_focus(Focus_Gain, 0);
		set_key_focus_element(w);
	}

	struct _Package
	{
		Rect curr_scissor;
	};

	void UIPrivate::preprocessing(void* __p, Element * w, bool visible, const Vec2 & off, float scl)
	{
		switch (w->flag)
		{
		case Element::FlagJustCreated:
			w->flag = Element::FlagNull;
			break;
		}

		if (visible && w->event_attitude$ != EventIgnore)
		{
			auto mhover = (Rect(w->pos$ * scl, (w->pos$ + w->size$) * scl * w->scale$) + off).contains(p.mpos);
			if (w->event_attitude$ == EventBlackHole || mhover)
			{
				if (p.mljustup)
				{
					if (p.temp_dragging_element&& w != p.temp_dragging_element)
						w->on_drop(p.temp_dragging_element);
				}
			}
		}
	}

	void UIPrivate::show_children(void* __p, Element * w, const Array<Element*> & children, bool visible, const Vec2 & off, float scl)
	{
		if (w->clip$)
		{
			p.curr_scissor = Rect(Vec2(0.f), w->size$ * w->global_scale) + w->global_pos;
			canvas->set_scissor(p.curr_scissor);
		}

		if (w->clip$)
		{
			p.curr_scissor = Rect(Vec2(0.f), p.surface_size);
			canvas->set_scissor(p.curr_scissor);
		}
	}

	void UIPrivate::show(void* __p, Element * e, bool visible, const Vec2 & off, float scl)
	{
		for (auto i_a = 0; i_a < e->animations$.size; )
		{
			auto& a = e->animations$[i_a];
			a.time += elp_time_;
			a.f$.p.thiz() = &a;
			a.f$.p.e() = e;

			if (a.time >= a.duration$)
			{
				a.time = -1.f;
				a.f$.exec();
				e->animations$.remove(i_a);
			}
			else
			{
				a.f$.exec();
				i_a++;
			}
		}

		if (visible && ((e->size$.x == 0.f && e->size$.y == 0.f) || (Rect(Vec2(0.f), e->size$ * e->global_scale) + e->global_pos).overlapping(p.curr_scissor)))
			e->on_draw(canvas, off + p.show_off, scl);
	}

	void UIPrivate::postprocessing(Element * w)
	{
		if (w->flag == Element::FlagNeedToRemoveFromParent)
		{
			w->remove_from_parent();
			return;
		}
		if (w->flag == Element::FlagNeedToTakeFromParent)
		{
			w->take_from_parent();
			w->flag = Element::FlagNull;
		}
	}

	//void Drawlist::draw_grid(const Vec2 &wnd_off, const Vec2 &off, const Vec2 &size)
	//{
	//	for (auto i = mod((int)off.x, 100); i.y < size.x; i.y += 100, i.x--)
	//	{
	//		if (i.y < 0)
	//			continue;
	//		add_line(Vec2(i.y, 0.f) + wnd_off, Vec2(i.y, size.y) + wnd_off, Vec4(1.f));
	//		add_text_stroke(Vec2(i.y + 4, 0.f) + wnd_off, Vec4(1.f), "%d", i.x * -100);
	//	}
	//	for (auto i = mod((int)off.y, 100); i.y < size.y; i.y += 100, i.x--)
	//	{
	//		if (i.y < 0)
	//			continue;
	//		add_line(Vec2(0.f, i.y) + wnd_off, Vec2(size.x, i.y) + wnd_off, Vec4(1.f));
	//		add_text_stroke(Vec2(4.f, i.y) + wnd_off, Vec4(1.f), "%d", i.x * -100);
	//	}
	//}
}

