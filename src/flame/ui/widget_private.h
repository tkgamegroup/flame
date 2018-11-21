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

#include "instance.h"
#include "widget.h"
#include "style.h"
#include "animation.h"

#include <tuple>
#include <list>

namespace flame
{
	namespace ui
	{
		struct WidgetPrivate : Widget
		{
			std::string name;

			Instance *instance;

			std::vector<Function*> draw_list;

			std::vector<Function*> mouseenter_listeners;
			std::vector<Function*> mouseleave_listeners;
			std::vector<Function*> lmousedown_listeners;
			std::vector<Function*> rmousedown_listeners;
			std::vector<Function*> mousemove_listeners;
			std::vector<Function*> mousescroll_listeners;
			std::vector<Function*> clicked_listeners;
			std::vector<Function*> doubleclicked_listeners;
			std::vector<Function*> keydown_listeners;
			std::vector<Function*> keyup_listeners;
			std::vector<Function*> char_listeners;
			std::vector<Function*> char_filters;
			std::vector<Function*> drop_listeners;
			std::vector<Function*> changed_listeners;
			std::vector<Function*> addchild_listeners;

			std::vector<std::pair<unsigned int, Function*>> delay_listener_remove;

			WidgetPrivate *parent;
			int layer;
			std::vector<std::unique_ptr<WidgetPrivate>> children_[2];

			std::vector<std::tuple<WidgetPrivate*, int, int, Function*>> delay_adds;
			std::vector<std::pair<int, int>> delay_removes_by_idx;
			std::vector<WidgetPrivate*> delay_removes_by_ptr;
			std::vector<std::pair<int, int>> delay_takes_by_idx;
			std::vector<WidgetPrivate*> delay_takes_by_ptr;
			std::vector<std::tuple<int, int, int>> delay_takes;
			std::vector<std::tuple<int, int, int>> delay_clears;

			typedef std::vector<Function*> closet;
			std::vector<closet> closets;
			closet *curr_closet;

			std::vector<std::tuple<float, float, bool, Function*>> animations;

			std::vector<CommonData> data_storage_;
			std::vector<std::wstring> string_storage_;

			WidgetPrivate(Instance *ui);
			WidgetPrivate(Instance *ui, XmlNode *src);
			~WidgetPrivate();

			void set_name(const char *_name);

			void set_width(float x, Widget *sender = nullptr);
			void set_height(float y, Widget *sender = nullptr);
			void set_size(const Vec2 &v, Widget *sender = nullptr);

			void set_visibility(bool v);

			void remove_animations();

			void add_child(WidgetPrivate *w, int layer = 0, int pos = -1, bool delay = false, void(*func)(CommonData*) = nullptr, char *capture_fmt = nullptr, va_list = 0);
			void remove_child(int layer, int idx, bool delay = false);
			void remove_child(WidgetPrivate *w, bool delay = false);
			void take_child(int layer, int idx, bool delay = false);
			void take_child(WidgetPrivate *w, bool delay = false);
			void clear_children(int layer, int begin, int end, bool delay = false);
			void take_children(int layer, int begin, int end, bool delay = false);
			void remove_from_parent(bool delay = false);
			void take_from_parent(bool delay = false);
			int find_child(WidgetPrivate *w);

			void resize_closet(int count);
			void set_curr_closet(int idx);

			void arrange();

			void add_style(int closet_idx, PF pf, char *capture_fmt, va_list ap);
			void add_animation(float total_time, bool looping, PF pf, char *capture_fmt, va_list ap);

			void on_draw(Canvas *c, const Vec2 &off, float scl);
			void on_mouseenter();
			void on_mouseleave();
			void on_lmousedown(const Vec2 &mpos);
			void on_rmousedown(const Vec2 &mpos);
			void on_mousemove(const Vec2 &disp);
			void on_clicked();
			void on_doubleclicked();
			void on_mousescroll(int scroll);
			void on_keydown(int code);
			void on_keyup(int code);
			void on_char(wchar_t ch);
			void on_drop(Widget *src);

			void report_changed() const;

			void add_draw_command(PF pf, char *capture_fmt, va_list ap);
			void remove_draw_command(int idx);

			Function *add_listener(unsigned int type, PF pf, char *capture_fmt, va_list ap);
			void remove_listener(unsigned int type, Function *f, bool delay = false);

			void resize_data_storage(int count);

			void resize_string_storage(int count);
			void set_string_storage(int idx, const wchar_t *str);

			void save(XmlNode *dst);
		};
	}
}

