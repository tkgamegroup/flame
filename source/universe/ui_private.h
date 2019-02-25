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

#pragma once

#include <flame/foundation/window.h>
#include <flame/universe/element.h>
#include <flame/universe/ui.h>

namespace flame
{
	namespace ui
	{
		struct UIPrivate : UI
		{
			std::unique_ptr<Element> root_;
			Element *hovering_widget_;
			Element *focus_widget_;
			Element *key_focus_widget_;
			Element *dragging_widget_;
			Element *popup_widget_;
			bool popup_widget_modual_;
			Ivec2 mouse_prev_pos_;
			Element *potential_doubleclick_widget_;
			float doubleclick_timer_;
			std::vector<int> keydown_inputs_;
			std::vector<int> keyup_inputs_;
			std::vector<wchar_t> char_inputs_;
			bool char_input_compelete_;

			float elp_time_; // second
			float total_time_;

			UIPrivate(graphics::Canvas* canvas, Window *w);

			void set_default_style(DefaultStyle s);

			void on_key(KeyState action, int value);
			void on_mouse(KeyState action, MouseKey key, const Ivec2 &pos);
			void on_resize(const Ivec2 &size);

			void set_hovering_widget(Element *w);
			void set_key_focus_widget(Element *w);
			void set_focus_widget(Element *w);
			void set_dragging_widget(Element *w);
			void set_popup_widget(Element *w, bool modual = false);
			void close_popup();

			void begin(float elp_time);
			void preprocessing_children(void *__p, Element *w, const Array<Element*> &children, const Vec2 &off, float scl);
			void preprocessing(void *__p, Element *w, bool visible, const Vec2 &off, float scl);
			void show_children(void *__p, Element *w, const Array<Element*> &children, bool visible, const Vec2 &off, float scl);
			void show(void *__p, Element *w, bool visible, const Vec2 &off, float scl);
			void postprocessing_children(const Array<Element*> &children);
			void postprocessing(Element *w);
			void end(const Vec2 &show_off = Vec2(0.f));
		};
	}
}

