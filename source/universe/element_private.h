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

#include <flame/universe/element.h>
#include <flame/universe/style.h>
#include <flame/universe/animation.h>
#include <flame/universe/ui.h>

#include <tuple>
#include <vector>

namespace flame
{
	struct ElementPrivate : Element
	{
		Instance* instance;
		ElementPrivate* parent;
		int layer;

		std::vector<std::tuple<ElementPrivate*, int, int, bool>> delay_adds;
		std::vector<std::pair<int, int>> delay_removes_by_idx;
		std::vector<ElementPrivate*> delay_removes_by_ptr;
		std::vector<std::pair<int, int>> delay_takes_by_idx;
		std::vector<ElementPrivate*> delay_takes_by_ptr;
		std::vector<std::tuple<int, int, int>> delay_takes;
		std::vector<std::tuple<int, int, int>> delay_clears;

		std::vector<std::pair<Listener, Function*>> delay_listener_remove;

		ElementPrivate(Instance* ui);
		~ElementPrivate();

		void set_width(float x, Element* sender = nullptr);
		void set_height(float y, Element* sender = nullptr);
		void set_size(const Vec2& v, Element* sender = nullptr);

		void set_visibility(bool v);

		void add_child(ElementPrivate* w, int layer, int pos, bool delay, bool modual);
		void remove_child(int layer, int idx, bool delay);
		void remove_child(ElementPrivate* w, bool delay);
		void take_child(int layer, int idx, bool delay);
		void take_child(ElementPrivate* w, bool delay);
		void clear_children(int layer, int begin, int end, bool delay);
		void take_children(int layer, int begin, int end, bool delay);
		void remove_from_parent(bool delay);
		void take_from_parent(bool delay);
		int find_child(int layer, ElementPrivate* w);
		void set_to_foreground();

		void arrange();

		void add_extra_draw(PF pf, const std::vector<CommonData>& capt);

		void add_style(int closet_id, PF pf, const std::vector<CommonData>& capt, int pos = -1);
		void remove_style(int idx);

		void add_animation(float duration, int looping, PF pf, const std::vector<CommonData>& capt);

		void remove_animations();

		void on_draw(Canvas* c, const Vec2& off, float scl);
		void on_focus(FocusType type, int focus_or_keyfocus);
		void on_key(KeyState action, int value);
		void on_mouse(KeyState action, MouseKey key, const Vec2& value);
		void on_drop(Element* src);
		void on_changed();

		int add_focus_listener(Function<FoucusListenerParm>& listener);
		int add_key_listener(Function<KeyListenerParm>& listener);
		int add_mouse_listener(Function<MouseListenerParm>& listener);
		int add_drop_listener(Function<DropListenerParm>& listener);
		int add_changed_listener(Function<ChangedListenerParm>& listener);
		int add_child_listener(Function<ChildListenerParm>& listener);

		void remove_focus_listener(int idx, bool delay);
		void remove_key_listener(int idx, bool delay);
		void remove_mouse_listener(int idx, bool delay);
		void remove_drop_listener(int idx, bool delay);
		void remove_changed_listener(int idx, bool delay);
		void remove_child_listener(int idx, bool delay);
	};
}

