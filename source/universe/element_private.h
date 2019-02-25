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
		UI* ui;
		ElementPrivate* parent;
		int layer;

		std::vector<std::tuple<ElementPrivate*, int, int, bool>> delay_adds;
		std::vector<std::pair<int, int>> delay_removes_by_idx;
		std::vector<ElementPrivate*> delay_removes_by_ptr;
		std::vector<std::pair<int, int>> delay_takes_by_idx;
		std::vector<ElementPrivate*> delay_takes_by_ptr;
		std::vector<std::tuple<int, int, int>> delay_takes;
		std::vector<std::tuple<int, int, int>> delay_clears;

		ElementPrivate(UI* ui);
		~ElementPrivate();

		void set_width(float x, Element* sender = nullptr);
		void set_height(float y, Element* sender = nullptr);
		void set_size(const Vec2& v, Element* sender = nullptr);

		void set_visibility(bool v);

		void add_child(ElementPrivate* w, int layer = 0, int pos = -1, bool delay = false, bool modual = false);
		void remove_child(int layer, int idx, bool delay = false);
		void remove_child(ElementPrivate* w, bool delay = false);
		void take_child(int layer, int idx, bool delay = false);
		void take_child(ElementPrivate* w, bool delay = false);
		void clear_children(int layer, int begin, int end, bool delay = false);
		void take_children(int layer, int begin, int end, bool delay = false);
		void remove_from_parent(bool delay = false);
		void take_from_parent(bool delay = false);
		int find_child(int layer, ElementPrivate* w);
		void set_to_foreground();

		void arrange();

		void remove_animations();

		void on_draw(graphics::Canvas* c, const Vec2& off, float scl);
		void on_focus(FocusType type, int focus_or_keyfocus);
		void on_key(KeyState action, int value);
		void on_mouse(KeyState action, MouseKey key, const Vec2& value);
		void on_drop(Element* src);
		void on_changed();
	};
}

