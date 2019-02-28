// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/descriptor.h>
#include <flame/graphics/swapchain.h>
#include "ui_private.h"

namespace flame
{
	FLAME_PACKAGE_BEGIN_1(UIEventData, UIPrivatePtr, ui, p)
	FLAME_PACKAGE_END_1

	void ui_key_event$(Window::KeyListenerParm& p)
	{
		p.get_capture<UIEventData>().ui()->on_key(p.action(), p.value());
	}

	void ui_mouse_event$(Window::MouseListenerParm& p)
	{
		p.get_capture<UIEventData>().ui()->on_mouse(p.action(), p.key(), p.pos());
	}

	void ui_resize_event$(Window::ResizeListenerParm& p)
	{
		p.get_capture<UIEventData>().ui()->on_resize(p.size());
	}

	FLAME_ELEMENT_BEGIN_0(wDebug, wDialog)
		FLAME_UNIVERSE_EXPORTS void init();
	FLAME_ELEMENT_END

	void wDebug::init()
	{
		init_data_types();

		align$ = AlignRightTop;
		visible$ = false;
	}

	UIPrivate::UIPrivate(graphics::Canvas* _canvas, Window * w)
	{
		canvas = _canvas;

		set_default_style(DefaultStyleDark);

		root_ = std::unique_ptr<Element>(Element::create(this));
		root_->name$ = "root";
		root_->size_policy_hori$ = SizeFitLayout;
		root_->size_policy_vert$ = SizeFitLayout;
		root_->event_attitude$ = EventAccept;
		root_->want_key_focus$ = true;

		hovering_element_ = nullptr;
		focus_element_ = nullptr;
		key_focus_element_ = root_.get();
		dragging_element_ = nullptr;
		popup_element_ = nullptr;
		popup_element_modual_ = false;
		potential_doubleclick_element_ = nullptr;
		doubleclick_timer_ = 0.f;
		char_input_compelete_ = true;

		for (auto i = 0; i < FLAME_ARRAYSIZE(key_states); i++)
			key_states[i] = KeyStateUp;

		mouse_pos = Ivec2(0);
		mouse_prev_pos_ = Ivec2(0);
		mouse_disp = Ivec2(0);
		mouse_scroll = 0;

		for (auto i = 0; i < FLAME_ARRAYSIZE(mouse_buttons); i++)
			mouse_buttons[i] = KeyStateUp;

		elp_time_ = 0.f;
		total_time_ = 0.f;

		if (w)
		{
			root_->size$ = w->size;

			w->add_key_listener(Function<Window::KeyListenerParm>(ui_key_event$, { this }));
			w->add_mouse_listener(Function<Window::MouseListenerParm>(ui_mouse_event$, { this }));
			w->add_resize_listener(Function<Window::ResizeListenerParm>(ui_resize_event$, { this }));
		}

		auto w_debug = Element::createT<wDebug>(this);
		root_->add_child(w_debug, 1);
	}

	inline void UIPrivate::set_default_style(DefaultStyle s)
	{
		switch (s)
		{
		case DefaultStyleDark:
			default_text_col = Bvec4(255, 255, 255, 255);
			default_text_col_hovering_or_active = Bvec4(180, 180, 180, 255);
			default_window_col = Colorf(0.06f, 0.06f, 0.06f, 0.94f);
			default_frame_col = HSV(55.f, 0.67f, 0.47f, 0.54f);
			default_frame_col_hovering = HSV(52.f, 0.73f, 0.97f, 0.40f);
			default_frame_col_active = HSV(52.f, 0.73f, 0.97f, 0.67f);
			default_button_col = HSV(52.f, 0.73f, 0.97f, 0.40f);
			default_button_col_hovering = HSV(52.f, 0.73f, 0.97f, 1.00f);
			default_button_col_active = HSV(49.f, 0.93f, 0.97f, 1.00f);
			default_header_col = HSV(52.f, 0.73f, 0.97f, 0.31f);
			default_header_col_hovering = HSV(52.f, 0.73f, 0.97f, 0.80f);
			default_header_col_active = HSV(52.f, 0.73f, 0.97f, 1.00f);
			break;
		case DefaultStyleLight:
			default_text_col = Bvec4(0, 0, 0, 255);
			default_text_col_hovering_or_active = Bvec4(255, 255, 255, 255);
			default_window_col = Colorf(0.94f, 0.94f, 0.94f, 1.00f);
			default_frame_col = Colorf(1.00f, 1.00f, 1.00f, 1.00f);
			default_frame_col_hovering = HSV(52.f, 0.73f, 0.97f, 0.40f);
			default_frame_col_active = HSV(52.f, 0.73f, 0.97f, 0.67f);
			default_button_col = HSV(52.f, 0.73f, 0.97f, 0.40f);
			default_button_col_hovering = HSV(52.f, 0.73f, 0.97f, 1.00f);
			default_button_col_active = HSV(45.f, 0.73f, 0.97f, 1.00f);
			default_header_col = HSV(52.f, 0.73f, 0.97f, 0.31f);
			default_header_col_hovering = HSV(52.f, 0.73f, 0.97f, 0.80f);
			default_header_col_active = HSV(52.f, 0.73f, 0.97f, 1.00f);
			break;
		}
		default_sdf_scale = 1.f;
	}

	inline void UIPrivate::on_key(KeyState action, int value)
	{
		if (action == KeyStateNull)
		{
			if (!char_input_compelete_ && !char_inputs_.empty())
			{
				std::string ansi;
				ansi += char_inputs_.back();
				ansi += value;
				auto wstr = a2w(ansi);
				char_inputs_.back() = wstr[0];
				char_input_compelete_ = true;
			}
			else
			{
				char_inputs_.push_back(value);
				if (value >= 0x80)
					char_input_compelete_ = false;
			}
		}
		else
		{
			key_states[value] = action | KeyStateJust;
			if (action == KeyStateDown)
				keydown_inputs_.push_back(value);
			else if (action == KeyStateUp)
				keyup_inputs_.push_back(value);
		}
	}

	inline void UIPrivate::on_mouse(KeyState action, MouseKey key, const Ivec2 & pos)
	{
		if (action == KeyStateNull)
		{
			if (key == Mouse_Middle)
				mouse_scroll = pos.x;
			else if (key == Mouse_Null)
				mouse_pos = pos;
		}
		else
		{
			mouse_buttons[key] = action | KeyStateJust;
			mouse_pos = pos;
		}
	}

	inline void UIPrivate::on_resize(const Ivec2 & size)
	{
		root_->set_size(Vec2(size));
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

	void UIPrivate::set_key_focus_element(Element * w)
	{
		if (w == nullptr)
			w = root_.get();
		if (w->want_key_focus$)
		{
			auto old = key_focus_element_;
			key_focus_element_ = w;
			if (old)
				old->on_focus(Focus_Lost, 1);
			if (key_focus_element_)
				key_focus_element_->on_focus(Focus_Gain, 1);
			return;
		}
		set_key_focus_element(w->parent);
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

	inline void UIPrivate::set_dragging_element(Element * w)
	{
		dragging_element_ = w;
	}

	inline void UIPrivate::set_popup_element(Element * w, bool modual)
	{
		popup_element_ = w;
		popup_element_modual_ = modual;
	}

	inline void UIPrivate::close_popup()
	{
		if (popup_element_ && !popup_element_modual_)
		{
			switch (popup_element_->class$.hash)
			{
			case cH("wMenuBar"):
				for (auto i_c = 0; i_c < popup_element_->children_1$.size; i_c++)
				{
					auto c = popup_element_->children_1$[i_c];

					if (c->class$.hash == cH("wMenu"))
						((wMenu*)c)->close();
				}
				break;
			case cH("menu items"):
				((wMenu*)popup_element_->parent)->close();
				break;
			case cH("wCombo"):
				((wMenu*)popup_element_)->close();
				break;
			}
			popup_element_ = nullptr;
		}
	}

	inline void UIPrivate::begin(float elp_time)
	{
		processed_mouse_input = false;
		processed_keyboard_input = false;

		elp_time_ = elp_time;
		total_time_ += elp_time;
	}

	struct _Package
	{
		Vec2 mpos;
		bool mljustdown;
		bool mljustup;
		bool mrjustdown;
		int mscroll;
		Ivec2 mdisp;
		Element* temp_dragging_element;
		Rect curr_scissor;
		Vec2 surface_size;
		bool hovering_any_element;
		bool clicking_nothing;
		Vec2 popup_off;
		float popup_scl;
		bool meet_popup_first;
		bool ban_event;
		Vec2 show_off;
	};

	void UIPrivate::preprocessing_children(void* __p, Element * w, const Array<Element*> & children, const Vec2 & off, float scl)
	{
		auto& p = *(_Package*)__p;

		if (children.size == 0)
			return;

		if (w->clip$)
			p.curr_scissor = Rect(Vec2(0.f), w->size$ * w->global_scale) + w->global_pos;

		auto _off = w->pos$ * scl + off;
		auto _scl = w->scale$ * scl;

		for (auto i_c = children.size - 1; i_c >= 0; i_c--)
			preprocessing(&p, children[i_c], w->showed, _off, _scl);

		if (w->clip$)
			p.curr_scissor = Rect(Vec2(0.f), p.surface_size);
	}

	void UIPrivate::preprocessing(void* __p, Element * w, bool visible, const Vec2 & off, float scl)
	{
		auto& p = *(_Package*)__p;

		switch (w->flag)
		{
		case Element::FlagJustCreatedNeedModual:
			set_popup_element(w, true);
		case Element::FlagJustCreated:
			w->flag = Element::FlagNull;
			break;
		}

		if (w == popup_element_ && p.meet_popup_first)
		{
			p.popup_off = off;
			p.popup_scl = scl;
			p.meet_popup_first = false;
			return;
		}

		w->global_pos = w->pos$ * scl + off;
		w->global_scale = w->scale$ * scl;
		w->showed = w->visible$ && visible;

		preprocessing_children(__p, w, w->children_2$, off, scl);
		preprocessing_children(__p, w, w->children_1$, off, scl);

		if (!p.ban_event && visible && w->event_attitude$ != EventIgnore)
		{
			auto mhover = p.curr_scissor.contains(p.mpos) &&
				(Rect(w->pos$ * scl, (w->pos$ + w->size$) * scl * w->scale$) + off).contains(p.mpos);
			if (w->event_attitude$ == EventBlackHole || mhover)
			{
				if (!p.hovering_any_element)
				{
					set_hovering_element(w);
					p.hovering_any_element = true;
				}
				if (p.mdisp.x != 0 || p.mdisp.y != 0)
				{
					w->on_mouse(KeyStateNull, Mouse_Null, Vec2(p.mdisp));
					p.mdisp = Ivec2(0);
				}
				if (p.mljustdown)
				{
					p.clicking_nothing = false;
					set_focus_element(w);
					if (mhover)
						dragging_element_ = w;
					w->on_mouse(KeyStateDown, Mouse_Left, p.mpos);
					p.mljustdown = false;
				}
				if (p.mrjustdown)
				{
					w->on_mouse(KeyStateDown, Mouse_Right, p.mpos);
					p.mrjustdown = false;
				}
				if (p.mljustup)
				{
					if (focus_element_ == w)
					{
						w->on_mouse(KeyState(KeyStateDown | KeyStateUp), Mouse_Null, Vec2(0.f));
						if (potential_doubleclick_element_ == w)
						{
							w->on_mouse(KeyState(KeyStateDown | KeyStateUp | KeyStateDouble), Mouse_Null, Vec2(0.f));
							potential_doubleclick_element_ = nullptr;
							doubleclick_timer_ = 0.f;
						}
						else
							potential_doubleclick_element_ = w;
					}
					if (p.temp_dragging_element&& w != p.temp_dragging_element)
						w->on_drop(p.temp_dragging_element);
					p.mljustup = false;
				}
				if (p.mscroll)
				{
					w->on_mouse(KeyStateNull, Mouse_Middle, Vec2(p.mscroll, 0.f));
					p.mscroll = 0;
				}
			}
		}
	}

	void UIPrivate::show_children(void* __p, Element * w, const Array<Element*> & children, bool visible, const Vec2 & off, float scl)
	{
		auto& p = *(_Package*)__p;

		if (children.size == 0)
			return;

		if (w->clip$)
		{
			p.curr_scissor = Rect(Vec2(0.f), w->size$ * w->global_scale) + w->global_pos;
			canvas->set_scissor(p.curr_scissor);
		}

		auto _off = w->pos$ * scl + off;
		auto _scl = w->scale$ * scl;

		for (auto i_c = 0; i_c < children.size; i_c++)
		{
			auto c = (Element*)children[i_c];
			show(&p, c, c->visible$ && visible, _off, _scl);
		}

		if (w->clip$)
		{
			p.curr_scissor = Rect(Vec2(0.f), p.surface_size);
			canvas->set_scissor(p.curr_scissor);
		}
	}

	void UIPrivate::show(void* __p, Element * e, bool visible, const Vec2 & off, float scl)
	{
		auto& p = *(_Package*)__p;

		if (e->need_arrange)
		{
			e->do_arrange();
			e->need_arrange = false;
		}

		if (e->class$.hash == cH("wToggle"))
			int cut = 1;
		e->style_level = -1;
		for (auto i_s = 0; i_s < e->styles$.size; i_s++)
		{
			auto& s = e->styles$[i_s];
			if (e->closet_id$ != s.closet_id$)
				continue;
			if (e->style_level <= s.level$)
			{
				e->style_level = s.level$;
				s.f$.p.thiz() = &s;
				s.f$.p.e() = e;
				s.f$.exec();
			}
		}

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
		{
			if (e == popup_element_ && p.meet_popup_first)
			{
				p.popup_off = off;
				p.popup_scl = scl;
				p.meet_popup_first = false;
				return;
			}
			else
				e->on_draw(canvas, off + p.show_off, scl);
		}

		show_children(__p, e, e->children_1$, visible, off, scl);
		show_children(__p, e, e->children_2$, visible, off, scl);
	}

	void UIPrivate::postprocessing_children(const Array<Element*> & children)
	{
		if (children.size == 0)
			return;

		for (auto i_c = children.size - 1; i_c >= 0; i_c--)
			postprocessing(children[i_c]);
	}

	void UIPrivate::postprocessing(Element * w)
	{
		postprocessing_children(w->children_2$);
		postprocessing_children(w->children_1$);

		w->state = StateNormal;
		if (dragging_element_)
		{
			if (dragging_element_ == w && hovering_element_ == w)
				w->state = StateActive;
		}
		else
		{
			if (hovering_element_ == w)
				w->state = StateHovering;
		}

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

	inline void UIPrivate::end(const Vec2 & show_off)
	{
		mouse_disp = mouse_pos - mouse_prev_pos_;

		_Package p;
		p.mpos = Vec2(mouse_pos);
		p.mljustdown = just_down_M(0);
		p.mljustup = just_up_M(0);
		p.mrjustdown = just_down_M(1);
		p.mscroll = mouse_scroll;
		p.mdisp = mouse_disp;
		p.temp_dragging_element = dragging_element_;

		if (dragging_element_)
		{
			if (!dragging_element_->visible$ || !pressing_M(0))
				dragging_element_ = nullptr;
			else if (dragging_element_->event_attitude$ != EventIgnore)
			{
				dragging_element_->on_mouse(KeyStateNull, Mouse_Null, Vec2(p.mdisp));
				p.mdisp = Ivec2(0);
			}
		}

		if (focus_element_)
		{
			if (!focus_element_->visible$)
				focus_element_ = nullptr;
		}
		if (key_focus_element_)
		{
			if (!key_focus_element_->visible$)
				key_focus_element_ = nullptr;
			else if (key_focus_element_->event_attitude$ != EventIgnore)
			{
				for (auto& code : keydown_inputs_)
					key_focus_element_->on_key(KeyStateDown, code);
				for (auto& code : keyup_inputs_)
					key_focus_element_->on_key(KeyStateUp, code);
				for (auto& ch : char_inputs_)
					key_focus_element_->on_key(KeyStateNull, ch);
			}
		}
		keydown_inputs_.clear();
		keyup_inputs_.clear();
		char_inputs_.clear();

		if (p.mljustdown)
		{
			set_key_focus_element(nullptr);
			set_focus_element(nullptr);
		}

		p.surface_size = root_->size$;
		p.curr_scissor = Rect(Vec2(0.f), p.surface_size);
		p.hovering_any_element = false;
		p.clicking_nothing = p.mljustdown;
		p.popup_off = popup_element_ ? popup_element_->pos$ : Vec2(0.f);
		p.popup_scl = 1.f;
		p.meet_popup_first = true;
		p.ban_event = popup_element_;
		p.show_off = show_off;

		preprocessing(&p, root_.get(), true, Vec2(0.f), 1.f);
		p.ban_event = false;
		if (popup_element_)
			preprocessing(&p, popup_element_, true, p.popup_off, p.popup_scl);
		if (!p.hovering_any_element)
			set_hovering_element(nullptr);
		if (p.clicking_nothing&& popup_element_)
			close_popup();

		if (dragging_element_)
		{
			if (!dragging_element_->visible$ || !pressing_M(0))
				dragging_element_ = nullptr;
		}

		if (potential_doubleclick_element_)
		{
			doubleclick_timer_ += elp_time_;
			if (doubleclick_timer_ > 0.5f)
			{
				potential_doubleclick_element_ = nullptr;
				doubleclick_timer_ = 0.f;
			}
		}

		p.curr_scissor = Rect(Vec2(0.f), p.surface_size);

		p.meet_popup_first = true;
		show(&p, root_.get(), true, Vec2(0.f), 1.f);
		if (popup_element_)
		{
			if (popup_element_modual_)
				canvas->add_rect_filled(Vec2(0.f), p.surface_size, Bvec4(0, 0, 0, 100));
			show(&p, popup_element_, true, p.popup_off, p.popup_scl);
		}

		postprocessing(root_.get());

		for (int i = 0; i < FLAME_ARRAYSIZE(key_states); i++)
			key_states[i] &= ~KeyStateJust;

		for (auto i = 0; i < FLAME_ARRAYSIZE(mouse_buttons); i++)
			mouse_buttons[i] &= ~KeyStateJust;

		mouse_prev_pos_ = mouse_pos;
		mouse_scroll = 0;
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

	void UI::set_default_style(DefaultStyle s)
	{
		((UIPrivate*)this)->set_default_style(s);
	}

	graphics::Canvas* UI::canvas()
	{
		return ((UIPrivate*)this)->canvas;
	}

	Ivec2 UI::size() const
	{
		return Ivec2(((UIPrivate*)this)->root_->size$);
	}

	void UI::on_key(KeyState action, int value)
	{
		((UIPrivate*)this)->on_key(action, value);
	}

	void UI::on_mouse(KeyState action, MouseKey key, const Ivec2 & pos)
	{
		((UIPrivate*)this)->on_mouse(action, key, pos);
	}

	void UI::on_resize(const Ivec2 & size)
	{
		((UIPrivate*)this)->on_resize(size);
	}

	Element* UI::root()
	{
		return ((UIPrivate*)this)->root_.get();
	}

	Element* UI::hovering_element()
	{
		return ((UIPrivate*)this)->hovering_element_;
	}

	Element* UI::focus_element()
	{
		return ((UIPrivate*)this)->focus_element_;
	}

	Element* UI::key_focus_element()
	{
		return ((UIPrivate*)this)->key_focus_element_;
	}

	Element* UI::dragging_element()
	{
		return ((UIPrivate*)this)->dragging_element_;
	}

	Element* UI::popup_element()
	{
		return ((UIPrivate*)this)->popup_element_;
	}

	void UI::set_hovering_element(Element * w)
	{
		((UIPrivate*)this)->set_hovering_element((Element*)w);
	}

	void UI::set_focus_element(Element * w)
	{
		((UIPrivate*)this)->set_focus_element((Element*)w);
	}

	void UI::set_key_focus_element(Element * w)
	{
		((UIPrivate*)this)->set_key_focus_element((Element*)w);
	}

	void UI::set_dragging_element(Element * w)
	{
		((UIPrivate*)this)->set_dragging_element((Element*)w);
	}

	void UI::set_popup_element(Element * w, bool modual)
	{
		((UIPrivate*)this)->set_popup_element((Element*)w, modual);
	}

	void UI::close_popup()
	{
		((UIPrivate*)this)->close_popup();
	}

	void UI::begin(float elp_time)
	{
		((UIPrivate*)this)->begin(elp_time);
	}

	void UI::end(const Vec2 & show_off)
	{
		((UIPrivate*)this)->end(show_off);
	}

	float UI::total_time() const
	{
		return ((UIPrivate*)this)->total_time_;
	}

	UI* UI::create(graphics::Canvas* canvas, Window * w)
	{
		return new UIPrivate(canvas, w);
	}

	void UI::destroy(UI * i)
	{
		delete (UIPrivate*)i;
	}
}

