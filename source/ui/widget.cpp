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

#include <flame/font.h>
#include <flame/file.h>
#include <flame/system.h>
#include <flame/typeinfo.h>

#include <flame/ui/canvas.h>
#include <flame/ui/style.h>
#include <flame/ui/icon.h>
#include "instance_private.h"
#include "widget_private.h"

#include <Windows.h>
#include <stdarg.h>
#include <assert.h>

namespace flame
{
	namespace ui
	{
		inline WidgetPrivate::WidgetPrivate(Instance *ui_)
		{
			class_hash$ = cH("widget");

			pos$ = Vec2(0.f);
			size$ = Vec2(0.f);

			alpha$ = 1.f;
			scale$ = 1.f;

			inner_padding$ = Vec4(0.f);
			layout_padding$ = 0.f;

			background_offset$ = Vec4(0.f);
			background_round_radius$ = 0.f;
			background_round_flags$ = 0;
			background_frame_thickness$ = 0.f;
			background_col$ = Bvec4(0);
			background_frame_col$ = Bvec4(255);
			background_shaow_thickness$ = 0.f;

			size_policy_hori$ = SizeFixed;
			size_policy_vert$ = SizeFixed;

			align$ = AlignFree;

			layout_type$ = LayoutFree;
			item_padding$ = 0.f;
			grid_hori_count$ = 1;
			clip$ = false;

			scroll_offset$ = 0.f;

			event_attitude$ = EventAccept;
			want_key_focus$ = false;

			visible$ = true;

			global_pos = Vec2(0.f);
			global_scale = 1.f;

			cliped = false;
			content_size = 0.f;
			showed = false;
			state = StateNormal;

			instance = ui_;
			parent = nullptr;

			closet_id = 0;
			
			add_default_draw_command();
		}

		inline WidgetPrivate::~WidgetPrivate()
		{
			for (auto &s : styles)
				Function::destroy(s);
			for (auto &a : animations)
				Function::destroy(a);

			if (this == instance->dragging_widget())
				instance->set_dragging_widget(nullptr);
			if (this == instance->focus_widget())
				instance->set_focus_widget(nullptr);
			if (this == instance->hovering_widget())
				instance->set_hovering_widget(nullptr);
		}

		inline void WidgetPrivate::set_width(float x, Widget *sender)
		{
			if (size$.x == x)
				return;
			size$.x = x;
			if (sender != this)
				arrange();
			if (parent && parent != sender)
				parent->arrange();
		}

		inline void WidgetPrivate::set_height(float y, Widget *sender)
		{
			if (size$.y == y)
				return;
			size$.y = y;
			if (sender != this)
				arrange();
			if (parent && parent != sender)
				parent->arrange();
		}

		inline void WidgetPrivate::set_size(const Vec2 &v, Widget *sender)
		{
			auto changed = false;
			auto do_arrange = false;
			if (size$.x != v.x)
			{
				size$.x = v.x;
				do_arrange |= (size_policy_hori$ == SizeFitLayout && sender != this);
				changed = true;
			}
			if (size$.y != v.y)
			{
				size$.y = v.y;
				do_arrange |= (size_policy_vert$ == SizeFitLayout && sender != this);
				changed = true;
			}
			if (!changed)
				return;
			if (do_arrange)
				arrange();
			if (parent && parent != sender)
				parent->arrange();
		}

		inline void WidgetPrivate::set_visibility(bool v)
		{
			if (visible$ == v)
				return;

			visible$ = v;
			if (!visible$)
				remove_animations();
			arrange();
			if (parent)
				parent->arrange();
		}

		void WidgetPrivate::remove_animations()
		{
			for (auto &a : animations)
				Function::destroy(a);
			animations.clear();
			for (auto i = 0; i < 2; i++)
			{
				for (auto &c : children_[i])
					c->remove_animations();
			}
		}

		inline void WidgetPrivate::add_child(WidgetPrivate *w, int layer, int pos, bool delay, void(*func)(CommonData*), char *capture_fmt, va_list ap)
		{
			if (delay)
			{
				delay_adds.emplace_back(w, layer, pos, func ? Function::create(func, "", capture_fmt, ap) : nullptr);
				return;
			}

			if (pos < 0)
				pos = children_[layer].size() + pos + 1;
			children_[layer].emplace(children_[layer].begin() + pos, w);

			w->parent = this;
			w->layer = layer;

			arrange();

			for (auto f : addchild_listeners)
			{
				f->datas[0].p = w;
				f->exec();
			}
		}

		inline void WidgetPrivate::remove_child(int layer, int idx, bool delay)
		{
			if (delay)
			{
				delay_removes_by_idx.emplace_back(layer, idx);
				return;
			}

			children_[layer].erase(children_[layer].begin() + idx);

			arrange();
		}

		inline void WidgetPrivate::remove_child(WidgetPrivate *w, bool delay)
		{
			if (delay)
			{
				delay_removes_by_ptr.push_back(w);
				return;
			}

			for (auto it = children_[w->layer].begin(); it != children_[w->layer].end(); it++)
			{
				if (it->get() == w)
				{
					children_[w->layer].erase(it);

					arrange();
					return;
				}
			}
		}

		inline void WidgetPrivate::take_child(int layer, int idx, bool delay)
		{
			if (delay)
			{
				delay_takes_by_idx.emplace_back(layer, idx);
				return;
			}

			children_[layer][idx].release();
			children_[layer].erase(children_[layer].begin() + idx);

			remove_animations();

			arrange();
		}

		inline void WidgetPrivate::take_child(WidgetPrivate *w, bool delay)
		{
			if (delay)
			{
				delay_takes_by_ptr.push_back(w);
				return;
			}

			for (auto it = children_[w->layer].begin(); it != children_[w->layer].end(); it++)
			{
				if (it->get() == w)
				{
					it->release();
					children_[w->layer].erase(it);
					w->remove_animations();

					arrange();
					return;
				}
			}
		}

		inline void WidgetPrivate::clear_children(int layer, int begin, int end, bool delay)
		{
			if (delay)
			{
				delay_clears.emplace_back(layer, begin, end);
				return;
			}

			if (end < 0)
				end = children_[layer].size() + end + 1;
			children_[layer].erase(children_[layer].begin() + begin, children_[layer].begin() + end);

			arrange();
		}

		inline void WidgetPrivate::take_children(int layer, int begin, int end, bool delay)
		{
			if (delay)
			{
				delay_takes.emplace_back(layer, begin, end);
				return;
			}

			if (end == -1)
				end = children_[layer].size();
			for (auto i = begin; i < end; i++)
				children_[layer][i].release()->remove_animations();
			children_[layer].erase(children_[layer].begin() + begin, children_[layer].begin() + end);

			arrange();
		}

		inline void WidgetPrivate::remove_from_parent(bool delay)
		{
			if (delay)
			{
				if (parent)
					parent->remove_child(this, true);
				return;
			}

			if (parent)
				parent->remove_child(this);
		}

		inline void WidgetPrivate::take_from_parent(bool delay)
		{
			if (delay)
			{
				if (parent)
					parent->take_child(this, true);
				return;
			}

			if (parent)
				parent->take_child(this);
		}

		inline int WidgetPrivate::find_child(WidgetPrivate *w)
		{
			for (auto i = 0; i < children_[w->layer].size(); i++)
			{
				if (children_[layer][i].get() == w)
					return i;
			}
			return -1;
		}

		inline void WidgetPrivate::arrange()
		{
			switch (layout_type$)
			{
			case LayoutVertical:
			{
				if (size_policy_hori$ == SizeFitChildren || size_policy_hori$ == SizeGreedy)
				{
					auto width = 0;
					for (auto &c : children_[0])
					{
						if (!c->visible$)
							continue;

						width = max(width, c->size$.x);
					}

					width += inner_padding$[0] + inner_padding$[1];
					if (size_policy_hori$ == SizeFitChildren)
						set_width(width, this);
					else if (size_policy_hori$ == SizeGreedy)
					{
						if (width > size$.x)
							set_width(width, this);
					}
				}
				if (size_policy_vert$ == SizeFitChildren || size_policy_vert$ == SizeGreedy)
				{
					auto height = 0;
					for (auto &c : children_[0])
					{
						if (!c->visible$)
							continue;

						height += c->size$.y + item_padding$;
					}
					height -= item_padding$;
					content_size = height;

					height += inner_padding$[2] + inner_padding$[3];
					if (size_policy_vert$ == SizeFitChildren)
						set_height(height, this);
					else if (size_policy_vert$ == SizeGreedy)
					{
						if (height > size$.y)
							set_height(height, this);
					}
				}
				else if (size_policy_vert$ == SizeFitLayout)
				{
					auto cnt = 0;
					auto height = 0;
					for (auto &c : children_[0])
					{
						if (!c->visible$)
							continue;

						if (c->size_policy_vert$ == SizeFitLayout)
							cnt++;
						else
							height += c->size$.y;
						height += item_padding$;
					}
					height -= item_padding$;
					content_size = height;

					height = max(0, (size$.y - inner_padding$[2] - inner_padding$[3] - height) / cnt);

					for (auto &c : children_[0])
					{
						if (!c->visible$)
							continue;

						if (c->size_policy_vert$ == SizeFitLayout)
							c->set_height(height, this);
					}
				}

				auto width = size$.x - inner_padding$[0] - inner_padding$[1];
				auto height = size$.y - inner_padding$[2] - inner_padding$[3];

				auto content_size = get_content_size();
				scroll_offset$ = content_size > height ? clamp((float)scroll_offset$, height - content_size, 0.f) : 0.f;

				auto y = inner_padding$[2] + scroll_offset$;

				for (auto &c : children_[0])
				{
					if (!c->visible$)
						continue;

					if (c->size_policy_hori$ == SizeFitLayout)
						c->set_width(width, this);
					else if (c->size_policy_hori$ == SizeGreedy)
					{
						if (width > c->size$.x)
							c->set_width(width, this);
					}

					switch (c->align$)
					{
					case AlignLittleEnd:
						c->pos$ = Vec2(inner_padding$[0] + c->layout_padding$, y);
						break;
					case AlignLargeEnd:
						c->pos$ = Vec2(size$.x - inner_padding$[1] - c->size$.x - c->layout_padding$, y);
						break;
					case AlignMiddle:
						c->pos$ = Vec2((size$.x - inner_padding$[0] - inner_padding$[1] - c->size$.x) * 0.5f + inner_padding$[0], y);
						break;
					}

					y += c->size$.y + item_padding$;
				}
			}
				break;
			case LayoutHorizontal:
			{
				if (size_policy_hori$ == SizeFitChildren || size_policy_hori$ == SizeGreedy)
				{
					auto width = 0;
					for (auto &c : children_[0])
					{
						if (!c->visible$)
							continue;

						width += c->size$.x + item_padding$;
					}
					width -= item_padding$;
					content_size = width;

					width += inner_padding$[0] + inner_padding$[1];
					if (size_policy_hori$ == SizeFitChildren)
						set_width(width, this);
					else if (size_policy_hori$ == SizeGreedy)
					{
						if (width > size$.x)
							set_width(width, this);
					}
				}
				else if (size_policy_hori$ == SizeFitLayout)
				{
					auto cnt = 0;
					auto width = 0;
					for (auto &c : children_[0])
					{
						if (!c->visible$)
							continue;

						if (c->size_policy_hori$ == SizeFitLayout)
							cnt++;
						else
							width += c->size$.x;
						width += item_padding$;
					}
					width -= item_padding$;
					content_size = width;

					width = max(0, (size$.x - inner_padding$[0] - inner_padding$[1] - width) / cnt);

					for (auto &c : children_[0])
					{
						if (!c->visible$)
							continue;

						if (c->size_policy_hori$ == SizeFitLayout)
							c->set_width(width, this);
					}
				}
				if (size_policy_vert$ == SizeFitChildren || size_policy_vert$ == SizeGreedy)
				{
					auto height = 0;
					for (auto &c : children_[0])
					{
						if (!c->visible$)
							continue;

						height = max(height, c->size$.y);
					}

					height += inner_padding$[2] + inner_padding$[3];
					if (size_policy_vert$ == SizeFitChildren)
						set_height(height, this);
					else if (size_policy_vert$ == SizeGreedy)
					{
						if (height > size$.y)
							set_height(height, this);
					}
				}

				auto height = size$.y - inner_padding$[2] - inner_padding$[3];
				auto x = inner_padding$[0];
				for (auto &c : children_[0])
				{
					if (!c->visible$)
						continue;

					if (c->size_policy_vert$ == SizeFitLayout)
						c->set_height(height, this);
					else if (c->size_policy_vert$ == SizeGreedy)
					{
						if (height > c->size$.y)
							c->set_height(height, this);
					}

					switch (c->align$)
					{
					case AlignLittleEnd:
						c->pos$ = Vec2(x, inner_padding$[2] + c->layout_padding$);
						break;
					case AlignLargeEnd:
						c->pos$ = Vec2(x, size$.y - inner_padding$[3] - c->size$.y - c->layout_padding$);
						break;
					case AlignMiddle:
						c->pos$ = Vec2(x, (size$.y - inner_padding$[2] - inner_padding$[3] - c->size$.y) * 0.5f + inner_padding$[2]);
						break;
					}

					x += c->size$.x + item_padding$;
				}
			}
				break;
			case LayoutGrid:
			{
				auto pos = Vec2(inner_padding$[0], inner_padding$[2]);

				auto cnt = 0;
				auto line_height = 0.f;
				auto max_width = 0.f;
				for (auto &c : children_[0])
				{
					c->pos$ = pos;
					line_height = max(line_height, c->size$.y);

					pos.x += c->size$.x + item_padding$;
					max_width = max(max_width, pos.x);
					cnt++;
					if (cnt >= grid_hori_count$)
					{
						pos.x = inner_padding$[0];
						pos.y += line_height + item_padding$;
						cnt = 0;
						line_height = 0.f;
					}
				}

				if (size_policy_hori$ == SizeFitChildren)
					set_width(max(max_width - item_padding$, 0.f), this);
				if (size_policy_vert$ == SizeFitChildren)
					set_height(max(pos.y - item_padding$, 0.f), this);
			}
				break;
			}

			for (auto &c : children_[1])
			{
				switch (c->align$)
				{
				case AlignLeft:
					c->pos$ = Vec2(inner_padding$[0] + c->layout_padding$,
						(size$.y - inner_padding$[2] - inner_padding$[3] - c->size$.y) * 0.5f + inner_padding$[2]);
					break;
				case AlignRight:
					if (c->size_policy_vert$ == SizeFitLayout)
						c->size$.y = size$.y - inner_padding$[2] - inner_padding$[3];
					c->pos$ = Vec2(size$.x - inner_padding$[1] - c->size$.x - c->layout_padding$,
						(size$.y - inner_padding$[2] - inner_padding$[3] - c->size$.y) * 0.5f + inner_padding$[2]);
					break;
				case AlignTop:
					c->pos$ = Vec2((size$.x - inner_padding$[0] - inner_padding$[1] - c->size$.x) * 0.5f + inner_padding$[0],
						inner_padding$[2] + c->layout_padding$);
					break;
				case AlignBottom:
					c->pos$ = Vec2((size$.x - inner_padding$[0] - inner_padding$[1] - c->size$.x) * 0.5f + inner_padding$[0],
						size$.y - inner_padding$[3] - c->size$.y - c->layout_padding$);
					break;
				case AlignLeftTop:
					c->pos$ = Vec2(inner_padding$[0] + c->layout_padding$,
						inner_padding$[2] + c->layout_padding$);
					break;
				case AlignLeftBottom:
					c->pos$ = Vec2(inner_padding$[0] + c->layout_padding$,
						size$.y - inner_padding$[3] - c->size$.y - c->layout_padding$);
					break;
				case AlignRightTop:
					c->pos$ = Vec2(size$.x - inner_padding$[1] - c->size$.x - c->layout_padding$,
						inner_padding$[2] + c->layout_padding$);
					break;
				case AlignRightBottom:
					c->pos$ = Vec2(size$.x - inner_padding$[1] - c->size$.x - c->layout_padding$,
						size$.y - inner_padding$[3] - c->size$.y - c->layout_padding$);
					break;
				case AlignLeftNoPadding:
					c->pos$ = Vec2(c->layout_padding$, (size$.y - c->size$.y) * 0.5f);
					break;
				case AlignRightNoPadding:
					c->pos$ = Vec2(size$.x - c->size$.x - c->layout_padding$, (size$.y - c->size$.y) * 0.5f);
					break;
				case AlignTopNoPadding:
					c->pos$ = Vec2((size$.x - c->size$.x) * 0.5f, c->layout_padding$);
					break;
				case AlignBottomNoPadding:
					c->pos$ = Vec2((size$.x - c->size$.x) * 0.5f, size$.y - c->size$.y - c->layout_padding$);
					break;
				case AlignLeftTopNoPadding:
					c->pos$ = Vec2(c->layout_padding$, c->layout_padding$);
					break;
				case AlignLeftBottomNoPadding:
					c->pos$ = Vec2(c->layout_padding$, size$.y - c->size$.y - c->layout_padding$);
					break;
				case AlignRightTopNoPadding:
					c->pos$ = Vec2(size$.x - c->size$.x - c->layout_padding$, c->layout_padding$);
					break;
				case AlignRightBottomNoPadding:
					c->pos$ = size$ - c->size$ - Vec2(c->layout_padding$);
					break;
				case AlignCenter:
					c->pos$ = (size$ - c->size$) * 0.5f;
					break;
				case AlignLeftOutside:
					c->pos$ = Vec2(-c->size$.x, 0.f);
					break;
				case AlignRightOutside:
					c->pos$ = Vec2(size$.x, 0.f);
					break;
				case AlignTopOutside:
					c->pos$ = Vec2(0.f, -c->size$.y);
					break;
				case AlignBottomOutside:
					c->pos$ = Vec2(0.f, size$.y);
					break;
				}
			}
		}

		inline void WidgetPrivate::add_style(PF pf, char *capture_fmt, va_list ap)
		{
			styles.emplace_back(Function::create(pf, "p", capture_fmt, ap));
		}

		inline void WidgetPrivate::add_animation(float total_time, bool looping, PF pf, char *capture_fmt, va_list ap)
		{
			auto f = Function::create(pf, "p f f i", capture_fmt, ap);
			f->datas[1].f[0] = 0.f;
			f->datas[2].f[0] = total_time;
			f->datas[3].i[0] = looping;
			animations.emplace_back(f);
		}

		inline void WidgetPrivate::on_draw(Canvas *c, const Vec2 &off, float scl)
		{
			for (auto i = 0; i < draw_commands$.size; i++)
			{
				auto f = draw_commands$[i];
				f->datas[0].p = c;
				*(Vec2*)f->datas[1].f = off;
				f->datas[2].f[0] = scl;
				f->exec();
			}
		}

		inline void WidgetPrivate::on_mouseenter()
		{
			for (auto f : mouseenter_listeners)
				f->exec();
		}

		inline void WidgetPrivate::on_mouseleave()
		{
			for (auto f : mouseleave_listeners)
				f->exec();
		}

		inline void WidgetPrivate::on_lmousedown(const Vec2 &mpos)
		{
			for (auto f : lmousedown_listeners)
			{
				*(Vec2*)f->datas[0].f = mpos;
				f->exec();
			}
		}

		inline void WidgetPrivate::on_rmousedown(const Vec2 &mpos)
		{
			for (auto f : rmousedown_listeners)
			{
				*(Vec2*)f->datas[0].f = mpos;
				f->exec();
			}
		}

		inline void WidgetPrivate::on_mousemove(const Vec2 &disp)
		{
			for (auto f : mousemove_listeners)
			{
				*(Vec2*)f->datas[0].f = disp;
				f->exec();
			}
		}

		inline void WidgetPrivate::on_clicked()
		{
			for (auto f : clicked_listeners)
				f->exec();
		}

		inline void WidgetPrivate::on_doubleclicked()
		{
			for (auto f : doubleclicked_listeners)
				f->exec();
		}

		inline void WidgetPrivate::on_mousescroll(int scroll)
		{
			for (auto f : mousescroll_listeners)
			{
				f->datas[0].i[0] = scroll;
				f->exec();
			}
		}

		inline void WidgetPrivate::on_keydown(int code)
		{
			for (auto f : keydown_listeners)
			{
				f->datas[0].i[0] = code;
				f->exec();
			}
		}

		inline void WidgetPrivate::on_keyup(int code)
		{
			for (auto f : keyup_listeners)
			{
				f->datas[0].i[0] = code;
				f->exec();
			}
		}

		inline void WidgetPrivate::on_char(wchar_t ch)
		{
			if (ch != '\b' && ch != 22 && ch != 27)
			{
				for (auto f : char_filters)
				{
					f->datas[0].i[0] = ch;
					f->exec();
					if (!f->datas[1].i[0])
						return;
				}
			}

			for (auto f : char_listeners)
			{
				f->datas[0].i[0] = ch;
				f->exec();
			}
		}

		inline void WidgetPrivate::on_drop(Widget *src)
		{
			for (auto f : drop_listeners)
			{
				f->datas[0].p = src;
				f->exec();
			}
		}

		inline void WidgetPrivate::report_changed() const
		{
			for (auto f : changed_listeners)
				f->exec();
		}

		inline void WidgetPrivate::add_draw_command(PF pf, char *capture_fmt, va_list ap)
		{
			auto f = Function::create(pf, "p f2 f", capture_fmt, ap);
			draw_commands$.push_back(f);
		}

		inline Function *WidgetPrivate::add_listener(unsigned int type, PF pf, char *capture_fmt, va_list ap)
		{
			const char *parm_fmt;
			std::vector<Function*> *list;

			switch (type)
			{
			case cH("mouse enter"):
				parm_fmt = "";
				list = &mouseenter_listeners;
				break;
			case cH("mouse leave"):
				parm_fmt = "";
				list = &mouseleave_listeners;
				break;
			case cH("left mouse down"):
				parm_fmt = "f2";
				list = &lmousedown_listeners;
				break;
			case cH("right mouse down"):
				parm_fmt = "f2";
				list = &rmousedown_listeners;
				break;
			case cH("mouse move"):
				parm_fmt = "f2";
				list = &mousemove_listeners;
				break;
			case cH("mouse scroll"):
				parm_fmt = "i";
				list = &mousescroll_listeners;
				break;
			case cH("clicked"):
				parm_fmt = "";
				list = &clicked_listeners;
				break;
			case cH("double clicked"):
				parm_fmt = "";
				list = &doubleclicked_listeners;
				break;
			case cH("key down"):
				parm_fmt = "i";
				list = &keydown_listeners;
				break;
			case cH("key up"):
				parm_fmt = "i";
				list = &keyup_listeners;
				break;
			case cH("char"):
				parm_fmt = "i";
				list = &char_listeners;
				break;
			case cH("char filter"):
				parm_fmt = "i i";
				list = &char_filters;
				break;
			case cH("drop"):
				parm_fmt = "p";
				list = &drop_listeners;
				break;
			case cH("changed"):
				parm_fmt = "";
				list = &changed_listeners;
				break;
			case cH("add child"):
				parm_fmt = "p";
				list = &addchild_listeners;
				break;
			default:
				assert(0);
				return nullptr;
			}

			auto f = Function::create(pf, parm_fmt, capture_fmt, ap);
			list->emplace_back(f);
			return f;
		}

		inline void WidgetPrivate::remove_listener(unsigned int type, Function *f, bool delay)
		{
			if (delay)
			{
				delay_listener_remove.emplace_back(type, f);
				return;
			}

			std::vector<Function*> *list;

			switch (type)
			{
			case cH("mouse enter"):
				list = &mouseenter_listeners;
				break;
			case cH("mouse leave"):
				list = &mouseleave_listeners;
				break;
			case cH("left mouse down"):
				list = &lmousedown_listeners;
				break;
			case cH("right mouse down"):
				list = &rmousedown_listeners;
				break;
			case cH("mouse move"):
				list = &mousemove_listeners;
				break;
			case cH("mouse scroll"):
				list = &mousescroll_listeners;
				break;
			case cH("clicked"):
				list = &clicked_listeners;
				break;
			case cH("double clicked"):
				list = &doubleclicked_listeners;
				break;
			case cH("key down"):
				list = &keydown_listeners;
				break;
			case cH("key up"):
				list = &keyup_listeners;
				break;
			case cH("char"):
				list = &char_listeners;
				break;
			case cH("char filter"):
				list = &char_filters;
				break;
			case cH("drop"):
				list = &drop_listeners;
				break;
			case cH("changed"):
				list = &changed_listeners;
				break;
			case cH("add child"):
				list = &addchild_listeners;
				break;
			default:
				assert(0);
				return;
			}

			for (auto it = list->begin(); it != list->end(); it++)
			{
				if (*it == f)
				{
					Function::destroy(f);
					list->erase(it);
					return;
				}
			}
		}

		inline void WidgetPrivate::remove_draw_command(int idx)
		{
			Function::destroy(draw_commands$[idx]);
			draw_commands$.remove(idx);
		}

		inline void WidgetPrivate::resize_data_storage(int count)
		{
			data_storage_.resize(count);
		}

		inline void WidgetPrivate::resize_string_storage(int count)
		{
			string_storage_.resize(count);
		}

		inline void WidgetPrivate::set_string_storage(int idx, const wchar_t *str)
		{
			if (idx >= string_storage_.size())
				string_storage_.resize(idx + 1);
			string_storage_[idx] = str;
		}

		void Widget::set_width(float x, Widget *sender)
		{
			((WidgetPrivate*)this)->set_width(x, sender);
		}

		void Widget::set_height(float y, Widget *sender)
		{
			((WidgetPrivate*)this)->set_height(y, sender);
		}

		void Widget::set_size(const Vec2 &v, Widget *sender)
		{
			((WidgetPrivate*)this)->set_size(v, sender);
		}

		void Widget::set_visibility(bool v)
		{
			((WidgetPrivate*)this)->set_visibility(v);
		}

		Instance *Widget::instance() const
		{
			return ((WidgetPrivate*)this)->instance;
		}

		Widget *Widget::parent() const
		{
			return ((WidgetPrivate*)this)->parent;
		}

		int Widget::layer() const
		{
			return ((WidgetPrivate*)this)->layer;
		}

		void Widget::add_child(Widget *w, int layer, int pos, bool delay, void(*func)(CommonData*), char *capture_fmt, ...)
		{
			va_list ap;
			va_start(ap, capture_fmt);
			((WidgetPrivate*)this)->add_child((WidgetPrivate*)w, layer, pos, delay, func, capture_fmt, ap);
			va_end(ap);
		}

		void Widget::remove_child(int layer, int idx, bool delay)
		{
			((WidgetPrivate*)this)->remove_child(idx, delay);
		}

		void Widget::remove_child(Widget *w, bool delay)
		{
			((WidgetPrivate*)this)->remove_child((WidgetPrivate*)w, delay);
		}

		void Widget::take_child(int layer, int idx, bool delay)
		{
			((WidgetPrivate*)this)->take_child(layer, idx, delay);
		}

		void Widget::take_child(Widget *w, bool delay)
		{
			((WidgetPrivate*)this)->take_child((WidgetPrivate*)w, delay);
		}

		void Widget::clear_children(int layer, int begin, int end, bool delay)
		{
			((WidgetPrivate*)this)->clear_children(layer, begin, end, delay);
		}

		void Widget::take_children(int layer, int begin, int end, bool delay)
		{
			((WidgetPrivate*)this)->take_children(layer, begin, end, delay);
		}

		void Widget::remove_from_parent(bool delay)
		{
			((WidgetPrivate*)this)->remove_from_parent(delay);
		}

		void Widget::take_from_parent(bool delay)
		{
			((WidgetPrivate*)this)->take_from_parent(delay);
		}

		int Widget::children_count(int layer) const
		{
			return ((WidgetPrivate*)this)->children_[layer].size();
		}

		Widget *Widget::child(int layer, int idx)
		{
			return ((WidgetPrivate*)this)->children_[layer][idx].get();
		}

		int Widget::find_child(Widget *w)
		{
			return ((WidgetPrivate*)this)->find_child((WidgetPrivate*)w);
		}

		const auto scroll_spare_spacing = 20.f;

		float Widget::get_content_size() const
		{
			return content_size + scroll_spare_spacing;
		}

		void Widget::arrange()
		{
			((WidgetPrivate*)this)->arrange();
		}

		void Widget::add_style(PF pf, char *capture_fmt, ...)
		{
			va_list ap;
			va_start(ap, capture_fmt);
			((WidgetPrivate*)this)->add_style(pf, capture_fmt, ap);
			va_end(ap);
		}

		void Widget::add_animation(float total_time, bool looping, PF pf, char *capture_fmt, ...)
		{
			va_list ap;
			va_start(ap, capture_fmt);
			((WidgetPrivate*)this)->add_animation(total_time, looping, pf, capture_fmt, ap);
			va_end(ap);
		}

		void Widget::on_draw(Canvas *c, const Vec2 &off, float scl)
		{
			((WidgetPrivate*)this)->on_draw(c, off, scl);
		}

		void Widget::on_mouseenter()
		{
			((WidgetPrivate*)this)->on_mouseenter();
		}

		void Widget::on_mouseleave()
		{
			((WidgetPrivate*)this)->on_mouseleave();
		}

		void Widget::on_lmousedown(const Vec2 &mpos)
		{
			((WidgetPrivate*)this)->on_lmousedown(mpos);
		}

		void Widget::on_rmousedown(const Vec2 &mpos)
		{
			((WidgetPrivate*)this)->on_rmousedown(mpos);
		}

		void Widget::on_mousemove(const Vec2 &disp)
		{
			((WidgetPrivate*)this)->on_mousemove(disp);
		}

		void Widget::on_clicked()
		{
			((WidgetPrivate*)this)->on_clicked();
		}

		void Widget::on_doubleclicked()
		{
			((WidgetPrivate*)this)->on_doubleclicked();
		}

		void Widget::on_mousescroll(int scroll)
		{
			((WidgetPrivate*)this)->on_mousescroll(scroll);
		}

		void Widget::on_keydown(int code)
		{
			((WidgetPrivate*)this)->on_keydown(code);
		}

		void Widget::on_keyup(int code)
		{
			((WidgetPrivate*)this)->on_keyup(code);
		}

		void Widget::on_char(wchar_t ch)
		{
			((WidgetPrivate*)this)->on_char(ch);
		}

		void Widget::on_drop(Widget *src)
		{
			((WidgetPrivate*)this)->on_drop(src);
		}

		void Widget::report_changed() const
		{
			((WidgetPrivate*)this)->report_changed();
		}

		void Widget::add_draw_command(PF pf, char *capture_fmt, ...)
		{
			va_list ap;
			va_start(ap, capture_fmt);
			((WidgetPrivate*)this)->add_draw_command(pf, capture_fmt, ap);
			va_end(ap);
		}

		FLAME_REGISTER_FUNCTION_BEG(Widget_defaultdraw, FLAME_GID(8350), "p f2 f")
			auto c = (Canvas*)d[0].p;
			auto off = Vec2(d[1].f);
			auto scl = d[2].f[0];
			auto thiz = (Widget*)d[3].p;

			auto p = (thiz->pos$ - Vec2(thiz->background_offset$[0], thiz->background_offset$[1])) * scl + off;
			auto ss = scl * thiz->scale$;
			auto s = (thiz->size$ + Vec2(thiz->background_offset$[0] + thiz->background_offset$[2], thiz->background_offset$[1] + thiz->background_offset$[3])) * ss;
			auto rr = thiz->background_round_radius$ * ss;

			if (thiz->background_shaow_thickness$ > 0.f)
			{
				c->add_rect_col2(p - Vec2(thiz->background_shaow_thickness$ * 0.5f), s + Vec2(thiz->background_shaow_thickness$), Bvec4(0, 0, 0, 128), Bvec4(0),
					thiz->background_shaow_thickness$, rr, thiz->background_round_flags$);
			}
			if (thiz->alpha$ > 0.f)
			{
				if (thiz->background_col$.w > 0)
					c->add_rect_filled(p, s, Bvec4(thiz->background_col$, thiz->alpha$), rr, thiz->background_round_flags$);
				if (thiz->background_frame_thickness$ > 0.f && thiz->background_frame_col$.w > 0)
					c->add_rect(p, s, Bvec4(thiz->background_frame_col$, thiz->alpha$), thiz->background_frame_thickness$, rr, thiz->background_round_flags$);
			}
		FLAME_REGISTER_FUNCTION_END(Widget_defaultdraw)

		void Widget::add_default_draw_command()
		{
			add_draw_command(Widget_defaultdraw::v, "p", this);
		}

		void Widget::remove_draw_command(int idx)
		{
			((WidgetPrivate*)this)->remove_draw_command(idx);
		}

		Function *Widget::add_listener(unsigned int type, PF pf, char *capture_fmt, ...)
		{
			va_list ap;
			va_start(ap, capture_fmt);
			auto f = ((WidgetPrivate*)this)->add_listener(type, pf, capture_fmt, ap);
			va_end(ap);
			return f;
		}

		void Widget::remove_listener(unsigned int type, Function *f, bool delay)
		{
			((WidgetPrivate*)this)->remove_listener(type, f, delay);
		}

		void Widget::resize_data_storage(int count)
		{
			((WidgetPrivate*)this)->resize_data_storage(count);
		}

		CommonData &Widget::data_storage(int idx)
		{
			return ((WidgetPrivate*)this)->data_storage_[idx];
		}

		void Widget::resize_string_storage(int count)
		{
			((WidgetPrivate*)this)->resize_string_storage(count);
		}

		const wchar_t *Widget::string_storage(int idx)
		{
			return ((WidgetPrivate*)this)->string_storage_[idx].c_str();
		}

		int Widget::string_storage_len(int idx)
		{
			return ((WidgetPrivate*)this)->string_storage_[idx].size();
		}

		void Widget::set_string_storage(int idx, const wchar_t *str)
		{
			((WidgetPrivate*)this)->set_string_storage(idx, str);
		}

		Widget *Widget::create(Instance *ui)
		{
			return new WidgetPrivate(ui);
		}

		void Widget::destroy(Widget *w)
		{
			delete(WidgetPrivate*)w;
		}

		void wLayout::init()
		{
			class_hash$ = cH("layout");

			event_attitude$ = EventIgnore;
			size_policy_hori$ = SizeFitChildren;
			size_policy_vert$ = SizeFitChildren;
		}

		wLayout *wLayout::create(Instance *ui)
		{
			auto w = (wLayout*)Widget::create(ui);
			w->init();

			return w;
		}

		FLAME_REGISTER_FUNCTION_BEG(Checkbox_clicked, FLAME_GID(15432), "")
			auto thiz = (wCheckbox*)d[0].p;

			thiz->checked() = !thiz->checked();
			thiz->report_changed();
		FLAME_REGISTER_FUNCTION_END(Checkbox_clicked)

		FLAME_REGISTER_FUNCTION_BEG(Checkbox_draw, FLAME_GID(8818), "p f2 f")
			auto c = (Canvas*)d[0].p;
			auto off = Vec2(d[1].f);
			auto scl = d[2].f[0];
			auto thiz = (wCheckbox*)d[3].p;

			c->add_rect(thiz->pos$ * scl + off, thiz->size$ * scl, thiz->background_col$, 2.f * scl);
			if (thiz->checked())
				c->add_rect_filled((thiz->pos$ + 3.f) * scl + off, (thiz->size$ - 6.f) * scl, thiz->background_col$);
		FLAME_REGISTER_FUNCTION_END(Checkbox_draw)

		void wCheckbox::init()
		{
			class_hash$ = cH("checkbox");
			resize_data_storage(1);

			size$ = Vec2(share_data.font_atlas->pixel_height);
			background_col$ = Bvec4(255);

			checked() = 0;

			add_listener(cH("clicked"), Checkbox_clicked::v, "p", this);

			remove_draw_command(0);
			add_draw_command(Checkbox_draw::v, "p", this);
		}

		int &wCheckbox::checked()
		{
			return data_storage(0).i[0];
		}

		wCheckbox *wCheckbox::create(Instance *ui)
		{
			auto w = (wCheckbox*)Widget::create(ui);
			w->init();

			return w;
		}

		FLAME_REGISTER_FUNCTION_BEG(Text_draw, FLAME_GID(9510), "p f2 f")
			auto c = (Canvas*)d[0].p;
			auto off = Vec2(d[1].f);
			auto scl = d[2].f[0];
			auto thiz = (wText*)d[3].p;

			if (thiz->alpha$ > 0.f && thiz->text_col().w > 0.f)
			{
				auto _pos = (thiz->pos$ + Vec2(thiz->inner_padding$[0], thiz->inner_padding$[2])) * scl + off;
				if (thiz->sdf_scale() < 0.f)
					c->add_text_stroke(_pos, Bvec4(thiz->text_col(), thiz->alpha$), thiz->string_storage(0));
				else
					c->add_text_sdf(_pos, Bvec4(thiz->text_col(), thiz->alpha$), thiz->string_storage(0), thiz->sdf_scale() * scl);
			}
		FLAME_REGISTER_FUNCTION_END(Text_draw)

		void wText::init()
		{
			class_hash$ = cH("text");
			resize_data_storage(2);
			resize_string_storage(1);

			event_attitude$ = EventIgnore;

			text_col() = Bvec4(0, 0, 0, 255);
			sdf_scale() = -1.f;
			set_text(L"");

			add_draw_command(Text_draw::v, "p", this);
		}

		Bvec4 &wText::text_col()
		{
			return *((Bvec4*)data_storage(0).b);
		}

		float &wText::sdf_scale()
		{
			return data_storage(1).f[0];
		}

		const wchar_t *wText::text()
		{
			return string_storage(0);
		}

		int wText::text_len()
		{
			return string_storage_len(0);
		}

		void wText::set_text(const wchar_t *str)
		{
			set_string_storage(0, str);
		}

		void wText::set_size_auto(const wchar_t *str)
		{
			auto v = Vec2(share_data.font_atlas->get_text_width(str), share_data.font_atlas->pixel_height);
			if (sdf_scale() > 0.f)
				v *= sdf_scale();
			v.x += inner_padding$[0] + inner_padding$[1];
			v.y += inner_padding$[2] + inner_padding$[3];
			set_size(v);
		}

		void wText::set_size_auto()
		{
			set_size_auto(string_storage(0));
		}

		void wText::set_text_and_size(const wchar_t *str)
		{
			set_text(str);
			set_size_auto();
		}

		wText *wText::create(Instance *ui)
		{
			auto w = (wText*)Widget::create(ui);
			w->init();

			return w;
		}

		void wButton::init()
		{
			((wText*)this)->init();

			class_hash$ = cH("button");

			background_col$ = Bvec4(255, 255, 255, 255 * 0.7f);
			event_attitude$ = EventAccept;
		}

		void wButton::set_classic(const wchar_t *text, float _sdf_scale, float alpha)
		{
			inner_padding$ += Vec4(4.f, 4.f, 2.f, 2.f);
			sdf_scale() = _sdf_scale;
			set_text_and_size(text);
			background_col$.w *= alpha;
			add_style_color(this, 0, Vec3(0.f, 0.f, 1.f));
		}

		wButton *wButton::create(Instance *ui)
		{
			auto w = (wButton*)Widget::create(ui);
			w->init();

			return w;
		}

		FLAME_REGISTER_FUNCTION_BEG(Toggle_clicked, FLAME_GID(23140), "")
			auto thiz = (wToggle*)d[0].p;

			thiz->set_toggle(!thiz->toggled());
		FLAME_REGISTER_FUNCTION_END(Toggle_clicked)

		void wToggle::init()
		{
			((wText*)this)->init();

			class_hash$ = cH("toggle");
			resize_data_storage(3);

			background_col$ = Bvec4(255, 255, 255, 255 * 0.7f);
			background_round_radius$ = share_data.font_atlas->pixel_height * 0.5f;
			background_offset$ = Vec4(background_round_radius$, 0.f, background_round_radius$, 0.f);
			background_round_flags$ = Rect::SideNW | Rect::SideNE | Rect::SideSW | Rect::SideSE;

			event_attitude$ = EventAccept;

			toggled() = 0;

			add_listener(cH("clicked"), Toggle_clicked::v, "p", this);
		}

		int &wToggle::toggled()
		{
			return data_storage(2).i[0];
		}

		void wToggle::set_toggle(bool v)
		{
			toggled() = v;
			if (!v)
				closet_id = 0;
			else
				closet_id = 1;

			report_changed();
		}

		wToggle *wToggle::create(Instance *ui)
		{
			auto w = (wToggle*)Widget::create(ui);
			w->init();

			return w;
		}

		static void menu_add_rarrow(wMenu *w)
		{
			if (w->w_rarrow())
				return;

			if (w->parent() && w->parent()->class_hash$ == cH("menubar"))
				return;
			if (!w->sub() && w->class_hash$ != cH("combo"))
				return;

			w->w_btn()->inner_padding$[1] += w->w_btn()->size$.y * 0.6f;
			w->w_btn()->set_size_auto();

			w->w_rarrow() = wText::create(w->instance());
			w->w_rarrow()->align$ = AlignRightNoPadding;
			w->w_rarrow()->sdf_scale() = w->w_btn()->sdf_scale();
			w->w_rarrow()->set_text_and_size(w->sub() ? Icon_CARET_RIGHT : Icon_ANGLE_DOWN);
			w->add_child(w->w_rarrow(), 1);
		}

		FLAME_REGISTER_FUNCTION_BEG(MenuItem_clicked, FLAME_GID(11216), "")
			auto thiz = (wToggle*)d[0].p;

			thiz->instance()->close_popup();
		FLAME_REGISTER_FUNCTION_END(MenuItem_clicked)

		void wMenuItem::init(const wchar_t *title)
		{
			((wButton*)this)->init();

			class_hash$ = cH("menuitem");

			set_classic(title);
			size_policy_hori$ = SizeFitLayout;
			align$ = AlignLittleEnd;

			add_listener(cH("clicked"), MenuItem_clicked::v, "p", this);
		}

		wMenuItem *wMenuItem::create(Instance *ui, const wchar_t *title)
		{
			auto w = (wMenuItem*)Widget::create(ui);
			w->init(title);

			return w;
		}

		FLAME_REGISTER_FUNCTION_BEG(Menu_btn_mousemove, FLAME_GID(10376), "f2")
			auto thiz = (wMenu*)d[1].p;

			if (thiz->instance()->popup_widget())
				thiz->open();
		FLAME_REGISTER_FUNCTION_END(Menu_btn_mousemove)

		FLAME_REGISTER_FUNCTION_BEG(Menu_items_addchild, FLAME_GID(21018), "p")
			auto w = (Widget*)d[0].p;
			auto thiz = (wMenu*)d[1].p;

			switch (w->class_hash$)
			{
			case cH("menuitem"):
				menu_add_rarrow(thiz);
				break;
			case cH("menu"):
			{
				auto menu = (wMenu*)w;

				menu->sub() = 1;
				menu->size_policy_hori$ = SizeGreedy;
				menu->w_items()->align$ = AlignRightOutside;

				menu_add_rarrow(thiz);
			}
				break;
			}
		FLAME_REGISTER_FUNCTION_END(Menu_items_addchild)

		void wMenu::init(const wchar_t *title, float alpha)
		{
			((wLayout*)this)->init();

			class_hash$ = cH("menu");
			resize_data_storage(5);

			sub() = 0;
			opened() = 0;

			align$ = AlignLittleEnd;
			layout_type$ = LayoutVertical;

			w_btn() = wButton::create(instance());
			w_btn()->set_classic(title, -1.f, alpha);
			w_btn()->size_policy_hori$ = SizeGreedy;
			w_btn()->align$ = AlignLittleEnd;
			add_child(w_btn());

			w_btn()->add_listener(cH("mouse move"), Menu_btn_mousemove::v, "p", this);

			w_rarrow() = nullptr;

			w_items() = wLayout::create(instance());
			w_items()->class_hash$ = cH("menu items");
			w_items()->layout_type$ = LayoutVertical;
			w_items()->align$ = AlignBottomOutside;
			w_items()->visible$ = false;
			add_child(w_items(), 1);

			w_items()->add_listener(cH("add child"), Menu_items_addchild::v, "p", this);
		}

		int &wMenu::sub()
		{
			return data_storage(0).i[0];
		}

		int &wMenu::opened()
		{
			return data_storage(1).i[0];
		}

		wButtonPtr &wMenu::w_btn()
		{
			return (*(wButtonPtr*)&data_storage(2).p);
		}

		wTextPtr &wMenu::w_rarrow()
		{
			return (*(wTextPtr*)&data_storage(3).p);
		}

		wLayoutPtr &wMenu::w_items()
		{
			return (*(wLayoutPtr*)&data_storage(4).p);
		}

		void wMenu::open()
		{
			if (opened())
				return;

			if (parent() && (parent()->class_hash$ == cH("menubar") || parent()->class_hash$ == cH("menu items")))
			{
				for (auto i = 0; i < parent()->children_count(0); i++)
				{
					auto c = parent()->child(0, i);
					if (c->class_hash$ == cH("menu"))
						((wMenu*)c)->close();
				}
			}

			w_items()->set_visibility(true);
			for (auto i = 0; i < w_items()->children_count(0); i++)
			{
				auto w = w_items()->child(0, i);
				add_animation_fade(w, 0.2f, 0.f, w->alpha$);
			}

			opened() = 1;
		}

		void wMenu::popup(const Vec2 &pos)
		{
			if (opened() || instance()->popup_widget())
				return;

			open();

			instance()->set_popup_widget(w_items());
			w_items()->pos$ += pos - w_items()->global_pos;
		}

		void wMenu::close()
		{
			if (!opened())
				return;

			for (auto i = 0; i < w_items()->children_count(0); i++)
			{
				auto c = w_items()->child(0, i);
				if (c->class_hash$ == cH("menu"))
					((wMenu*)c)->close();
			}

			w_items()->set_visibility(false);

			opened() = 0;
		}

		wMenu *wMenu::create(Instance *ui, const wchar_t *title, float alpha)
		{
			auto w = (wMenu*)Widget::create(ui);
			w->init(title, alpha);

			return w;
		}

		FLAME_REGISTER_FUNCTION_BEG(Menu_btn_clicked, FLAME_GID(24104), "")
			auto thiz = (wMenu*)d[0].p;
			auto menu = (wMenu*)d[1].p;

			if (!menu->opened())
			{
				if (!thiz->instance()->popup_widget())
				{
					menu->open();

					thiz->instance()->set_popup_widget(thiz);
				}
			}
			else
				thiz->instance()->close_popup();
		FLAME_REGISTER_FUNCTION_END(Menu_btn_clicked)

		FLAME_REGISTER_FUNCTION_BEG(MenuBar_addchild, FLAME_GID(10208), "p")
			auto w = (Widget*)d[0].p;
			auto thiz = (wMenuBar*)d[1].p;

			if (w->class_hash$ == cH("menu"))
			{
				auto menu = (wMenu*)w;

				menu->w_btn()->add_listener(cH("clicked"), Menu_btn_clicked::v, "p p", thiz, menu);
			}
		FLAME_REGISTER_FUNCTION_END(MenuBar_addchild)

		void wMenuBar::init()
		{
			((wLayout*)this)->init();

			class_hash$ = cH("menubar");

			layout_type$ = LayoutHorizontal;

			add_listener(cH("add child"), MenuBar_addchild::v, "p", this);
		}

		wMenuBar *wMenuBar::create(Instance *ui)
		{
			auto w = (wMenuBar*)Widget::create(ui);
			w->init();

			return w;
		}
		
		FLAME_REGISTER_FUNCTION_BEG(Combo_btn_clicked, FLAME_GID(10368), "")
			auto thiz = (wMenu*)d[0].p;

			if (!thiz->opened())
			{
				if (!thiz->instance()->popup_widget())
				{
					thiz->open();

					thiz->instance()->set_popup_widget(thiz);
				}
			}
			else
				thiz->instance()->close_popup();
		FLAME_REGISTER_FUNCTION_END(Combo_btn_clicked)

		FLAME_REGISTER_FUNCTION_BEG(ComboItem_clicked, FLAME_GID(22268), "")
			auto thiz = (wCombo*)d[0].p;
			auto idx = d[1].i[0];

			thiz->set_sel(idx);
		FLAME_REGISTER_FUNCTION_END(ComboItem_clicked)

		FLAME_REGISTER_FUNCTION_BEG(Combo_items_addchild, FLAME_GID(7524), "p")
			auto w = (Widget*)d[0].p;
			auto thiz = (wMenu*)d[1].p;

			if (w->class_hash$ == cH("menuitem"))
			{
				auto i = (wMenuItem*)w;

				thiz->set_width(thiz->inner_padding$[0] + thiz->inner_padding$[1] + thiz->w_btn()->inner_padding$[0] + thiz->w_btn()->inner_padding$[1] + thiz->w_items()->size$.x);
				auto idx = thiz->w_items()->children_count(0) - 1;

				i->add_listener(cH("clicked"), ComboItem_clicked::v, "p i", thiz, idx);
			}
		FLAME_REGISTER_FUNCTION_END(Combo_items_addchild)

		void wCombo::init()
		{
			((wMenu*)this)->init(L"");

			class_hash$ = cH("combo");
			resize_data_storage(6);

			background_frame_thickness$ = 1.f;
			size_policy_hori$ = SizeGreedy;

			w_btn()->size_policy_hori$ = SizeFitLayout;

			sel() = -1;

			w_btn()->add_listener(cH("clicked"), Combo_btn_clicked::v, "p", this);

			w_items()->add_listener(cH("add child"), Combo_items_addchild::v, "p", this);
		}

		int &wCombo::sel()
		{
			return data_storage(5).i[0];
		}

		void wCombo::set_sel(int idx)
		{
			sel() = idx;
			auto i = (wMenuItem*)w_items()->child(0, idx);
			w_btn()->set_text(i->text());

			report_changed();
		}

		wCombo *wCombo::create(Instance *ui)
		{
			auto w = (wCombo*)Widget::create(ui);
			w->init();

			return w;
		}

		FLAME_REGISTER_FUNCTION_BEG(Edit_keydown, FLAME_GID(27590), "i")
			auto code = d[0].i[0];
			auto thiz = (wEdit*)d[1].p;

			auto &text = ((WidgetPrivate*)thiz)->string_storage_[0]; // since we need the full control of string

			switch (code)
			{
			case Key_Left:
				if (thiz->cursor() > 0)
					thiz->cursor()--;
				break;
			case Key_Right:
				if (thiz->cursor() < text.size())
					thiz->cursor()++;
				break;
			case Key_Home:
				thiz->cursor() = 0;
				break;
			case Key_End:
				thiz->cursor() = text.size();
				break;
			case Key_Del:
				if (thiz->cursor() < text.size())
				{
					text.erase(text.begin() + thiz->cursor());
					thiz->report_changed();
				}
				break;
			}
		FLAME_REGISTER_FUNCTION_END(Edit_keydown)

		FLAME_REGISTER_FUNCTION_BEG(Combo_char, FLAME_GID(5693), "i")
			auto ch = d[0].i[0];
			auto thiz = (wEdit*)d[1].p;

			auto &text = ((WidgetPrivate*)thiz)->string_storage_[0]; // since we need the full control of string

			switch (ch)
			{
			case L'\b':
				if (thiz->cursor() > 0)
				{
					thiz->cursor()--;
					text.erase(text.begin() + thiz->cursor());
					thiz->report_changed();
				}
				break;
			case 22:
			{
				auto str = get_clipboard();

				thiz->cursor() = 0;
				text = str.v;
				thiz->report_changed();
			}
				break;
			case 27:
				break;
			default:
				text.insert(text.begin() + thiz->cursor(), ch);
				thiz->cursor()++;
				thiz->report_changed();
			}
		FLAME_REGISTER_FUNCTION_END(Combo_char)

		FLAME_REGISTER_FUNCTION_BEG(Combo_draw, FLAME_GID(9908), "p f2 f")
			auto c = (Canvas*)d[0].p;
			auto off = Vec2(d[1].f);
			auto scl = d[2].f[0];
			auto thiz = (wEdit*)d[3].p;

			if (thiz->instance()->key_focus_widget() == thiz && int(thiz->instance()->total_time() * 2) % 2 == 0)
			{
				auto len = share_data.font_atlas->get_text_width(thiz->text(), thiz->text() + thiz->cursor());
				if (thiz->sdf_scale() < 0.f)
				{
					c->add_char_stroke((thiz->pos$ + Vec2(thiz->inner_padding$[0], thiz->inner_padding$[2])
						+ Vec2(len - 1.f, 0.f)) * scl + off, thiz->text_col(), '|');
				}
				else
				{
					c->add_char_sdf((thiz->pos$ + Vec2(thiz->inner_padding$[0], thiz->inner_padding$[2])
						+ Vec2(len - 1.f, 0.f)) * scl + off, thiz->text_col(), '|', thiz->sdf_scale() * scl);
				}
			}
		FLAME_REGISTER_FUNCTION_END(Combo_draw)

		void wEdit::init()
		{
			((wText*)this)->init();

			class_hash$ = cH("edit");
			resize_data_storage(3);

			inner_padding$ = Vec4(4.f, 4.f, 2.f, 2.f);
			background_col$ = Colorf(0.3f, 0.3f, 0.3f, 1.f);
			event_attitude$ = EventAccept;
			want_key_focus$ = true;

			cursor() = 0;

			add_listener(cH("key down"), Edit_keydown::v, "p", this);

			add_listener(cH("char"), Combo_char::v, "p", this);

			add_draw_command(Combo_draw::v, "p", this);
		}

		int &wEdit::cursor()
		{
			return data_storage(2).i[0];
		}

		void wEdit::set_size_by_width(float width)
		{
			set_size(Vec2(width + inner_padding$[0] + inner_padding$[1],
				share_data.font_atlas->pixel_height * (sdf_scale() > 0.f ? sdf_scale() : 1.f) +
				inner_padding$[2] + inner_padding$[3]));
		}

		FLAME_REGISTER_FUNCTION_BEG(Edit_charfilter_int, FLAME_GID(5037), "i i")
			auto ch = d[0].i[0];

			d[1].i[0] = ch >= L'0' && ch <= L'9';
		FLAME_REGISTER_FUNCTION_END(Edit_charfilter_int)

		FLAME_REGISTER_FUNCTION_BEG(Edit_charfilter_float, FLAME_GID(18387), "i i")
			auto ch = d[0].i[0];
			auto thiz = (WidgetPrivate*)d[1].p;

			if (ch == L'.')
			{
				auto it = thiz->string_storage_[0].find(L'.');
				if (it != std::string::npos)
				{
					d[1].i[0] = 0;
					return;
				}
			}
			d[1].i[0] = ch >= '0' && ch <= '9';
		FLAME_REGISTER_FUNCTION_END(Edit_charfilter_float)

		void wEdit::add_char_filter_int()
		{
			add_listener(cH("char filter"), Edit_charfilter_int::v, "");
		}

		void wEdit::add_char_filter_float()
		{
			add_listener(cH("char filter"), Edit_charfilter_float::v, "p", this);
		}

		wEdit *wEdit::create(Instance *ui)
		{
			auto w = (wEdit*)Widget::create(ui);
			w->init();

			return w;
		}

		FLAME_REGISTER_FUNCTION_BEG(Image_draw, FLAME_GID(30624), "p f2 f")
			auto c = (Canvas*)d[0].p;
			auto off = Vec2(d[1].f);
			auto scl = d[2].f[0];
			auto thiz = (wImage*)d[3].p;

			auto p = (thiz->pos$ + Vec2(thiz->inner_padding$[0], thiz->inner_padding$[2])) * scl + off;
			auto s = (thiz->size$ - Vec2(thiz->inner_padding$[0] + thiz->inner_padding$[1], thiz->inner_padding$[2] + thiz->inner_padding$[3])) * scl * thiz->scale$;
			if (!thiz->stretch())
				c->add_image(p, s, thiz->id(), thiz->uv0(), thiz->uv1());
			else
				c->add_image_stretch(p, s, thiz->id(), thiz->border());
		FLAME_REGISTER_FUNCTION_END(Image_draw)

		void wImage::init()
		{
			class_hash$ = cH("image");
			resize_data_storage(5);

			id() = 0;
			uv0() = Vec2(0.f);
			uv1() = Vec2(1.f);
			stretch() = 0;
			border() = Vec4(0.f);

			add_draw_command(Image_draw::v, "p", this);
		}

		int &wImage::id()
		{
			return data_storage(0).i[0];
		}

		Vec2 &wImage::uv0()
		{
			return *((Vec2*)data_storage(1).f);
		}

		Vec2 &wImage::uv1()
		{
			return *((Vec2*)data_storage(2).f);
		}

		int &wImage::stretch()
		{
			return data_storage(3).i[0];
		}

		Vec4 &wImage::border()
		{
			return *((Vec4*)data_storage(4).f);
		}

		wImage *wImage::create(Instance *ui)
		{
			auto w = (wImage*)Widget::create(ui);
			w->init();

			return w;
		}

		FLAME_REGISTER_FUNCTION_BEG(SizeDrag_mousemove, FLAME_GID(2863), "f2")
			auto disp = Vec2(d[0].f);
			auto thiz = (wSizeDrag*)d[1].p;
			auto target = (Widget*)d[2].p;

			if (thiz == thiz->instance()->dragging_widget())
			{
				auto changed = false;
				auto d = disp / thiz->parent()->scale$;
				auto new_size = target->size$;

				if (new_size.x + d.x > thiz->min_size().x)
				{
					new_size.x += d.x;
					changed = true;
				}
				if (new_size.y + d.y > thiz->min_size().y)
				{
					new_size.y += d.y;
					changed = true;
				}

				if (changed)
					target->set_size(new_size);
			}
		FLAME_REGISTER_FUNCTION_END(SizeDrag_mousemove)

		FLAME_REGISTER_FUNCTION_BEG(SizeDrag_draw, FLAME_GID(4242), "p f2 f")
			auto c = (Canvas*)d[0].p;
			auto off = Vec2(d[1].f);
			auto scl = d[2].f[0];
			auto thiz = (wSizeDrag*)d[3].p;

			c->add_triangle_filled(
				(thiz->pos$ + Vec2(thiz->size$.x, 0.f)) * scl + off,
				(thiz->pos$ + Vec2(0.f, thiz->size$.y)) * scl + off,
				(thiz->pos$ + Vec2(thiz->size$)) * scl + off,
				thiz->background_col$);
		FLAME_REGISTER_FUNCTION_END(SizeDrag_draw)

		void wSizeDrag::init(Widget *target)
		{
			class_hash$ = cH("sizedrag");
			resize_data_storage(1);

			size$ = Vec2(10.f);
			background_col$ = Bvec4(140, 225, 15, 255 * 0.5f);
			align$ = AlignRightBottomNoPadding;
			add_style_color(this, 0, Vec3(0.f, 0.f, 0.7f));

			min_size() = Vec2(0.f);

			add_listener(cH("mouse move"), SizeDrag_mousemove::v, "p p", this, target);

			remove_draw_command(0);
			add_draw_command(SizeDrag_draw::v, "p", this);
		}

		Vec2 &wSizeDrag::min_size()
		{
			return *((Vec2*)data_storage(0).f);
		}

		wSizeDrag *wSizeDrag::create(Instance *ui, Widget *target)
		{
			auto w = (wSizeDrag*)Widget::create(ui);
			w->init(target);

			return w;
		}

		FLAME_REGISTER_FUNCTION_BEG(Scrollbar_mousemove, FLAME_GID(1385), "f2")
			auto disp = Vec2(d[0].f);
			auto thiz = (wScrollbar*)d[1].p;

			if (thiz->w_btn() == thiz->instance()->dragging_widget())
			{
				thiz->w_target()->scroll_offset$ -= (disp.y / thiz->size$.y) * thiz->w_target()->get_content_size();
				thiz->w_target()->arrange();
			}
		FLAME_REGISTER_FUNCTION_END(Scrollbar_mousemove)

		FLAME_REGISTER_FUNCTION_BEG(Scrollbar_mousescroll, FLAME_GID(2126), "i")
			auto scroll = d[0].i[0];
			auto thiz = (wScrollbar*)d[1].p;

			thiz->scroll(scroll);
		FLAME_REGISTER_FUNCTION_END(Scrollbar_mousescroll)

		FLAME_REGISTER_FUNCTION_BEG(Scrollbar_style, FLAME_GID(18956), "p")
			auto thiz = (wScrollbar*)d[0].p;

			auto s = thiz->w_target()->size$.y - thiz->w_target()->inner_padding$[2] - thiz->w_target()->inner_padding$[3];
			auto content_size = thiz->w_target()->get_content_size();
			if (content_size > s)
			{
				thiz->w_btn()->pos$.y = thiz->size$.y * (-thiz->w_target()->scroll_offset$ / content_size);
				thiz->w_btn()->size$.y = thiz->size$.y * (s / content_size);
			}
			else
			{
				thiz->w_btn()->pos$.y = 0.f;
				thiz->w_btn()->size$.y = thiz->size$.y;
			}
		FLAME_REGISTER_FUNCTION_END(Scrollbar_style)

		void wScrollbar::init(Widget *target)
		{
			((wLayout*)this)->init();

			class_hash$ = cH("scrollbar");
			resize_data_storage(2);

			size$ = Vec2(10.f);
			size_policy_vert$ = SizeFitLayout;
			align$ = AlignRight;
			event_attitude$ = EventAccept;

			w_btn() = wButton::create(instance());
			w_btn()->size$ = size$;
			add_style_color(w_btn(), 0, Vec3(0.f, 1.f, 1.f));
			add_child(w_btn());

			w_target() = target;

			w_btn()->add_listener(cH("mouse move"), Scrollbar_mousemove::v, "p", this);

			add_listener(cH("mouse scroll"), Scrollbar_mousescroll::v, "p", this);

			add_style(Scrollbar_style::v, "");
		}

		wButtonPtr &wScrollbar::w_btn()
		{
			return (*(wButtonPtr*)&data_storage(0).p);
		}

		WidgetPtr &wScrollbar::w_target()
		{
			return (*(WidgetPtr*)&data_storage(1).p);
		}

		void wScrollbar::scroll(int v)
		{
			w_target()->scroll_offset$ += v * 20.f;
			w_target()->arrange();
		}

		wScrollbar *wScrollbar::create(Instance *ui, Widget *target)
		{
			auto w = (wScrollbar*)Widget::create(ui);
			w->init(target);

			return w;
		}

		FLAME_REGISTER_FUNCTION_BEG(ListItem_btn_mousescroll, FLAME_GID(6526), "i")
			auto scroll = d[0].i[0];
			auto thiz = (wList*)d[1].p;

			if (thiz->parent())
				thiz->parent()->on_mousescroll(scroll);
		FLAME_REGISTER_FUNCTION_END(ListItem_btn_mousescroll)

		void wListItem::init()
		{
			((wLayout*)this)->init();

			class_hash$ = cH("listitem");
			resize_data_storage(1);

			size_policy_hori$ = SizeFitLayout;
			align$ = AlignLittleEnd;
			layout_type$ = LayoutHorizontal;

			w_btn() = wButton::create(instance());
			w_btn()->background_col$.w = 40;
			w_btn()->inner_padding$ = Vec4(4.f, 4.f, 2.f, 2.f);
			w_btn()->size_policy_hori$ = SizeFitLayout;
			w_btn()->align$ = AlignLittleEnd;
			add_child(w_btn());

			w_btn()->add_listener(cH("mouse scroll"), ListItem_btn_mousescroll::v, "p", this);
		}

		wButtonPtr &wListItem::w_btn()
		{
			return *((wButtonPtr*)&data_storage(0).p);
		}

		wListItem *wListItem::create(Instance *ui)
		{
			auto w = (wListItem*)Widget::create(ui);
			w->init();

			return w;
		}

		FLAME_REGISTER_FUNCTION_BEG(List_mousescroll, FLAME_GID(20822), "i")
			auto scroll = d[0].i[0];
			auto thiz = (wList*)d[1].p;

			thiz->w_scrollbar()->scroll(scroll);
		FLAME_REGISTER_FUNCTION_END(List_mousescroll)

		FLAME_REGISTER_FUNCTION_BEG(List_leftmousedown, FLAME_GID(19124), "f2")
			auto thiz = (wList*)d[1].p;

			thiz->w_sel() = nullptr;
			thiz->report_changed();
		FLAME_REGISTER_FUNCTION_END(List_leftmousedown)

		FLAME_REGISTER_FUNCTION_BEG(ListItem_btn_style, FLAME_GID(408), "p")
			auto w = (Widget*)d[0].p;
			auto thiz = (wList*)d[1].p;

			if (thiz->w_sel() && thiz->w_sel()->w_btn() == w && w->state == StateNormal)
				w->background_col$ = Bvec4(120, 120, 20, 255);
		FLAME_REGISTER_FUNCTION_END(ListItem_btn_style)

		FLAME_REGISTER_FUNCTION_BEG(ListItem_btn_leftmousedown, FLAME_GID(8928), "f2")
			auto thiz = (wList*)d[1].p;
			auto i = (wListItem*)d[2].p;

			thiz->w_sel() = i;
			thiz->report_changed();
		FLAME_REGISTER_FUNCTION_END(ListItem_btn_leftmousedown)

		FLAME_REGISTER_FUNCTION_BEG(List_addchild, FLAME_GID(8288), "p")
			auto w = (Widget*)d[0].p;
			auto thiz = (wList*)d[1].p;

			if (w->class_hash$ == cH("listitem"))
			{
				auto i = (wListItem*)w;

				add_style_color(i->w_btn(), 0, Vec3(260.f, 0.8f, 1.f));
				i->w_btn()->add_style(ListItem_btn_style::v, "p", thiz);

				i->w_btn()->add_listener(cH("left mouse down"), ListItem_btn_leftmousedown::v, "p p", thiz, i);
			}
		FLAME_REGISTER_FUNCTION_END(List_addchild)

		void wList::init()
		{
			((wLayout*)this)->init();

			class_hash$ = cH("list");
			resize_data_storage(2);

			inner_padding$ = Vec4(4.f);
			background_offset$ = Vec4(0.f, 1.f, 0.f, 1.f);
			background_frame_thickness$ = 1.f;
			size_policy_hori$ = SizeFitLayout;
			size_policy_vert$ = SizeFitLayout;
			event_attitude$ = EventAccept;
			layout_type$ = LayoutVertical;
			clip$ = true;

			add_listener(cH("mouse scroll"), List_mousescroll::v, "p", this);

			add_listener(cH("left mouse down"), List_leftmousedown::v, "p", this);

			w_scrollbar() = wScrollbar::create(instance(), this);
			w_scrollbar()->background_col$ = Bvec4(255, 255, 255, 40);

			add_child(w_scrollbar(), 1);

			add_listener(cH("add child"), List_addchild::v, "p", this);
		}

		wListItemPtr &wList::w_sel()
		{
			return *((wListItemPtr*)&data_storage(0).p);
		}

		wScrollbarPtr &wList::w_scrollbar()
		{
			return *((wScrollbarPtr*)&data_storage(1).p);
		}

		wList *wList::create(Instance *ui)
		{
			auto w = (wList*)Widget::create(ui);
			w->init();

			return w;
		}

		FLAME_REGISTER_FUNCTION_BEG(TreeNode_doubleclicked, FLAME_GID(10825), "")
			auto thiz = (wTreeNode*)d[0].p;

			thiz->w_larrow()->on_clicked();
		FLAME_REGISTER_FUNCTION_END(TreeNode_doubleclicked)

		FLAME_REGISTER_FUNCTION_BEG(TreeNode_larrow_clicked, FLAME_GID(20989), "")
			auto thiz = (wTreeNode*)d[0].p;

			auto v = !thiz->w_items()->visible$;
			thiz->w_items()->set_visibility(v);

			thiz->w_larrow()->set_text(v ? Icon_CARET_DOWN : Icon_CARET_RIGHT);
		FLAME_REGISTER_FUNCTION_END(TreeNode_larrow_clicked)

		void wTreeNode::init(const wchar_t *title, const Bvec4 &normal_col, const Bvec4 &else_col)
		{
			((wLayout*)this)->init();

			class_hash$ = cH("treenode");
			resize_data_storage(3);

			layout_type$ = LayoutVertical;
			align$ = AlignLittleEnd;

			w_btn() = wButton::create(instance());
			w_btn()->inner_padding$[0] = share_data.font_atlas->pixel_height * 0.8f;
			w_btn()->inner_padding$ += Vec4(4.f, 4.f, 2.f, 2.f);
			w_btn()->background_col$.w = 0;
			w_btn()->set_text_and_size(title);
			add_style_textcolor(w_btn(), 0, normal_col, else_col);
			w_btn()->align$ = AlignLittleEnd;
			add_child(w_btn());

			w_btn()->add_listener(cH("double clicked"), TreeNode_doubleclicked::v, "p", this);

			w_items() = wLayout::create(instance());
			w_items()->layout_padding$ = w_btn()->inner_padding$[0];
			w_items()->align$ = AlignLittleEnd;
			w_items()->visible$ = false;
			w_items()->layout_type$ = LayoutVertical;
			add_child(w_items());

			w_larrow() = wText::create(instance());
			w_larrow()->inner_padding$ = Vec4(4.f, 0.f, 4.f, 0.f);
			w_larrow()->background_col$ = Bvec4(255, 255, 255, 0);
			w_larrow()->align$ = AlignLeftTopNoPadding;
			w_larrow()->event_attitude$ = EventAccept;
			w_larrow()->set_size(Vec2(w_btn()->inner_padding$[0], w_btn()->size$.y));
			w_larrow()->set_text(Icon_CARET_RIGHT);
			add_style_textcolor(w_larrow(), 0, normal_col, else_col);

			w_larrow()->add_listener(cH("clicked"), TreeNode_larrow_clicked::v, "p", this);

			add_child(w_larrow(), 1);
		}

		wButtonPtr &wTreeNode::w_btn()
		{
			return *((wButtonPtr*)&data_storage(0).p);
		}

		wLayoutPtr &wTreeNode::w_items()
		{
			return *((wLayoutPtr*)&data_storage(1).p);
		}

		wTextPtr &wTreeNode::w_larrow()
		{
			return *((wTextPtr*)&data_storage(2).p);
		}

		wTreeNode *wTreeNode::create(Instance *ui, const wchar_t *title, const Bvec4 &normal_col, const Bvec4 &else_col)
		{
			auto w = (wTreeNode*)Widget::create(ui);
			w->init(title, normal_col, else_col);

			return w;
		}

		//void wDialog::init(const wchar_t *title, float sdf_scale, bool resize)
		//{
		//	((wLayout*)this)->init();

		//	resize_data_storage(3);

		//	auto radius = 8.f;
		//	if (sdf_scale > 0.f)
		//		radius *= sdf_scale;

		//	inner_padding = Vec4(radius);
		//	background_col = Colorf(0.5f, 0.5f, 0.5f, 0.9f);
		//	event_attitude = EventAccept;
		//	if (resize)
		//	{
		//		size_policy_hori = SizeFitLayout;
		//		size_policy_vert = SizeFitLayout;
		//	}
		//	layout_type = LayoutVertical;
		//	item_padding = radius;

		//	add_listener(ListenerMouseMove, [this](const Vec2 &disp) {
		//		if (this == instance()->dragging_widget())
		//			pos += disp / parent()->scale;
		//	});

		//	if (title[0])
		//	{
		//		w_title() = wText::create(instance());
		//		w_title()->inner_padding = Vec4(0.f, 0.f, 0.f, radius * 0.5f);
		//		if (sdf_scale > 0.f)
		//			w_title()->sdf_scale() = sdf_scale;
		//		w_title()->background_offset = Vec4(radius, radius, radius, 0.f);
		//		w_title()->background_round_radius = radius;
		//		w_title()->background_round_flags = Rect::SideNW | Rect::SideNE;
		//		w_title()->set_text_and_size(title);
		//		w_title()->size_policy_hori = SizeFitLayout;
		//		w_title()->align = AlignLittleEnd;
		//		add_child(w_title());

		//		background_offset[1] = -w_title()->size.y - inner_padding[2];
		//	}
		//	else
		//	{
		//		background_offset[1] = 0.f;
		//		background_round_radius = radius;
		//		background_round_flags = Rect::SideNW | Rect::SideNE | Rect::SideSW | Rect::SideSE;
		//		background_shaow_thickness = 8.f;

		//		w_title() = nullptr;
		//	}

		//	w_content() = wLayout::create(instance());
		//	w_content()->align = AlignLittleEnd;
		//	if (resize)
		//	{
		//		w_content()->size_policy_hori = SizeFitLayout;
		//		w_content()->size_policy_vert = SizeFitLayout;
		//	}
		//	add_child(w_content());

		//	if (resize)
		//	{
		//		w_sizedrag() = wSizeDrag::create(instance(), this);
		//		add_child(w_sizedrag(), 1);
		//	}
		//	else
		//		w_sizedrag() = nullptr;
		//}

		//wTextPtr &wDialog::w_title()
		//{
		//	return *((wTextPtr*)&data_storage(0).p);
		//}

		//wLayoutPtr &wDialog::w_content()
		//{
		//	return *((wLayoutPtr*)&data_storage(1).p);
		//}

		//wSizeDragPtr &wDialog::w_sizedrag()
		//{
		//	return *((wSizeDragPtr*)&data_storage(2).p);
		//}

		//wDialog *wDialog::create(Instance *ui, const wchar_t *title, float sdf_scale, bool resize)
		//{
		//	auto w = (wDialog*)Widget::create(ui);
		//	w->init(title, sdf_scale, resize);

		//	return w;
		//}

		//void wMessageDialog::init(const wchar_t *title, float sdf_scale, const wchar_t *text)
		//{
		//	((wDialog*)this)->init(title, sdf_scale, false);

		//	resize_data_storage(5);

		//	event_attitude = EventBlackHole;
		//	want_key_focus = true;

		//	if (w_title())
		//		w_title()->background_col = Bvec4(200, 40, 20, 255);

		//	w_content()->layout_type = LayoutVertical;
		//	w_content()->item_padding = 8.f;
		//	if (sdf_scale > 0.f)
		//		w_content()->item_padding *= sdf_scale;

		//	if (text[0])
		//	{
		//		w_text() = wText::create(instance());
		//		w_text()->align = AlignLittleEnd;
		//		w_text()->sdf_scale() = sdf_scale;
		//		w_text()->set_text_and_size(text);
		//		w_content()->add_child(w_text());
		//	}
		//	else
		//		w_text() = nullptr;

		//	w_ok() = wButton::create(instance());
		//	w_ok()->set_classic(L"OK", sdf_scale);
		//	w_ok()->align = AlignMiddle;
		//	w_content()->add_child(w_ok());

		//	w_ok()->add_listener(ListenerClicked, [this]() {
		//		remove_from_parent(true);
		//	});

		//	pos = (Vec2(instance()->root()->size) - size) * 0.5f;

		//	instance()->root()->add_child(this, 0, -1, true, [](CommonData *d) {
		//		auto thiz = (Widget*)d[0].p;

		//		thiz->instance()->set_focus_widget(thiz);
		//		thiz->instance()->set_dragging_widget(nullptr);
		//	}, "p", this);
		//}

		//wTextPtr &wMessageDialog::w_text()
		//{
		//	return *((wTextPtr*)&data_storage(3).p);
		//}

		//wButtonPtr &wMessageDialog::w_ok()
		//{
		//	return *((wButtonPtr*)&data_storage(4).p);
		//}

		//wMessageDialog *wMessageDialog::create(Instance *ui, const wchar_t *title, float sdf_scale, const wchar_t *text)
		//{
		//	auto w = (wMessageDialog*)Widget::create(ui);
		//	w->init(title, sdf_scale, text);

		//	return w;
		//}

		//void wYesNoDialog::init(const wchar_t *title, float sdf_scale, const wchar_t *text, const wchar_t *yes_text, const wchar_t *no_text, const std::function<void(bool)> &callback)
		//{
		//	((wDialog*)this)->init(title, sdf_scale, false);

		//	resize_data_storage(7);

		//	event_attitude = EventBlackHole;
		//	want_key_focus = true;

		//	add_listener(ListenerKeyDown, [this](int key) {
		//		switch (key)
		//		{
		//		case Key_Enter:
		//			w_yes()->on_clicked();
		//			break;
		//		case Key_Esc:
		//			w_no()->on_clicked();
		//			break;
		//		}
		//	});

		//	if (w_title())
		//		w_title()->background_col = Bvec4(200, 40, 20, 255);

		//	w_content()->layout_type = LayoutVertical;
		//	w_content()->item_padding = 8.f;
		//	if (sdf_scale > 0.f)
		//		w_content()->item_padding *= sdf_scale;

		//	if (text[0])
		//	{
		//		w_text() = wText::create(instance());
		//		w_text()->align = AlignMiddle;
		//		w_text()->sdf_scale() = sdf_scale;
		//		w_text()->set_text_and_size(text);
		//		w_content()->add_child(w_text());
		//	}
		//	else
		//		w_text() = nullptr;

		//	w_buttons() = wLayout::create(instance());
		//	w_buttons()->align = AlignMiddle;
		//	w_buttons()->layout_type = LayoutHorizontal;
		//	w_buttons()->item_padding = 4.f;
		//	if (sdf_scale > 0.f)
		//		w_buttons()->item_padding *= sdf_scale;

		//	w_yes() = wButton::create(instance());
		//	w_yes()->set_classic(yes_text, sdf_scale);
		//	w_yes()->align = AlignLittleEnd;
		//	w_buttons()->add_child(w_yes());

		//	w_no() = wButton::create(instance());
		//	w_no()->set_classic(no_text, sdf_scale);
		//	w_no()->align = AlignLittleEnd;
		//	w_buttons()->add_child(w_no());

		//	w_yes()->add_listener(ListenerClicked, [this, callback]() {
		//		callback(true);
		//		remove_from_parent(true);
		//	});

		//	w_no()->add_listener(ListenerClicked, [this, callback]() {
		//		callback(false);
		//		remove_from_parent(true);
		//	});

		//	w_content()->add_child(w_buttons());

		//	pos = (Vec2(instance()->root()->size) - size) * 0.5f;

		//	instance()->root()->add_child(this, 0, -1, true, [](CommonData *d) {
		//		auto thiz = (Widget*)d[0].p;

		//		thiz->instance()->set_focus_widget(thiz);
		//		thiz->instance()->set_dragging_widget(nullptr);
		//	}, "p", this);
		//}

		//wTextPtr &wYesNoDialog::w_text()
		//{
		//	return *((wTextPtr*)&data_storage(3).p);
		//}

		//wLayoutPtr &wYesNoDialog::w_buttons()
		//{
		//	return *((wLayoutPtr*)&data_storage(4).p);
		//}

		//wButtonPtr &wYesNoDialog::w_yes()
		//{
		//	return *((wButtonPtr*)&data_storage(5).p);
		//}

		//wButtonPtr &wYesNoDialog::w_no()
		//{
		//	return *((wButtonPtr*)&data_storage(6).p);
		//}

		//wYesNoDialog *wYesNoDialog::create(Instance *ui, const wchar_t *title, float sdf_scale, const wchar_t *text, const wchar_t *yes_text, const wchar_t *no_text, const std::function<void(bool)> &callback)
		//{
		//	auto w = (wYesNoDialog*)Widget::create(ui);
		//	w->init(title, sdf_scale, text, yes_text, no_text, callback);

		//	return w;
		//}

		//void wInputDialog::init(const wchar_t *title, float sdf_scale, const std::function<void(bool ok, const wchar_t *input)> &callback)
		//{
		//	((wDialog*)this)->init(title, sdf_scale, false);

		//	resize_data_storage(7);

		//	event_attitude = EventBlackHole;
		//	want_key_focus = true;

		//	if (w_title())
		//		w_title()->background_col = Bvec4(200, 40, 20, 255);

		//	w_content()->layout_type = LayoutVertical;
		//	w_content()->item_padding = 8.f;
		//	if (sdf_scale > 0.f)
		//		w_content()->item_padding *= sdf_scale;

		//	w_input() = wEdit::create(instance());
		//	w_input()->align = AlignLittleEnd;
		//	w_input()->sdf_scale() = sdf_scale;
		//	w_input()->set_size_by_width(100.f);
		//	w_content()->add_child(w_input());

		//	w_buttons() = wLayout::create(instance());
		//	w_buttons()->align = AlignMiddle;
		//	w_buttons()->layout_type = LayoutHorizontal;
		//	w_buttons()->item_padding = 4.f;
		//	if (sdf_scale > 0.f)
		//		w_buttons()->item_padding *= sdf_scale;

		//	w_ok() = wButton::create(instance());
		//	w_ok()->set_classic(L"OK", sdf_scale);
		//	w_ok()->align = AlignLittleEnd;
		//	w_buttons()->add_child(w_ok());

		//	w_cancel() = wButton::create(instance());
		//	w_cancel()->set_classic(L"Cancel", sdf_scale);
		//	w_cancel()->align = AlignLittleEnd;
		//	w_buttons()->add_child(w_cancel());

		//	w_content()->add_child(w_buttons());

		//	pos = (Vec2(instance()->root()->size) - size) * 0.5f;

		//	w_ok()->add_listener(ListenerClicked, [this, callback]() {
		//		callback(true, w_input()->text());
		//		remove_from_parent(true);
		//	});

		//	w_cancel()->add_listener(ListenerClicked, [this, callback]() {
		//		callback(false, nullptr);
		//		remove_from_parent(true);
		//	});

		//	instance()->root()->add_child(this, 0, -1, true, [](CommonData *d) {
		//		auto thiz = (Widget*)d[0].p;

		//		thiz->instance()->set_focus_widget(thiz);
		//		thiz->instance()->set_dragging_widget(nullptr);
		//	}, "p", this);
		//}

		//wEditPtr &wInputDialog::w_input()
		//{
		//	return *((wEditPtr*)&data_storage(3).p);
		//}

		//wLayoutPtr &wInputDialog::w_buttons()
		//{
		//	return *((wLayoutPtr*)&data_storage(4).p);
		//}

		//wButtonPtr &wInputDialog::w_ok()
		//{
		//	return *((wButtonPtr*)&data_storage(5).p);
		//}

		//wButtonPtr &wInputDialog::w_cancel()
		//{
		//	return *((wButtonPtr*)&data_storage(6).p);
		//}

		//wInputDialog *wInputDialog::create(Instance *ui, const wchar_t *title, float sdf_scale, const std::function<void(bool ok, const wchar_t *input)> &callback)
		//{
		//	auto w = (wInputDialog*)Widget::create(ui);
		//	w->init(title, sdf_scale, callback);

		//	return w;
		//}

		//void wFileDialog::init(const wchar_t *title, int io, const std::function<void(bool ok, const wchar_t *filename)> &callback, const wchar_t *exts)
		//{
		//	((wDialog*)this)->init(title, -1.f, true);

		//	resize_data_storage(10);
		//	resize_string_storage(2);

		//	event_attitude = EventBlackHole;
		//	want_key_focus = true;

		//	if (w_title())
		//		w_title()->background_col = Bvec4(200, 40, 20, 255);

		//	set_string_storage(0, get_curr_path());
		//	set_string_storage(1, L"");

		//	w_sizedrag()->min_size() = Vec2(300.f, 400.f);
		//	set_size(w_sizedrag()->min_size());
		//	event_attitude = EventBlackHole;

		//	w_content()->size_policy_hori = SizeFitLayout;
		//	w_content()->size_policy_vert = SizeFitLayout;
		//	w_content()->layout_type = LayoutVertical;
		//	w_content()->item_padding = 8.f;

		//	w_pathstems() = wMenuBar::create(instance());
		//	w_pathstems()->align = AlignLittleEnd;
		//	w_pathstems()->layout_type = LayoutHorizontal;
		//	w_content()->add_child(w_pathstems());

		//	w_list() = wList::create(instance());
		//	w_list()->align = AlignLittleEnd;

		//	auto upward_item = wListItem::create(instance());
		//	upward_item->w_btn()->set_text_and_size(L"..");
		//	w_list()->add_child(upward_item);

		//	upward_item->w_btn()->add_listener(ListenerDoubleClicked, [this]() {
		//		std::filesystem::path fs_path(string_storage(0));
		//		if (fs_path.root_path() != fs_path)
		//			set_path(fs_path.parent_path().generic_wstring().c_str());
		//	});

		//	w_content()->add_child(w_list());

		//	w_input() = wEdit::create(instance());
		//	w_input()->size_policy_hori = SizeFitLayout;
		//	w_input()->align = AlignLittleEnd;
		//	w_input()->text_col() = Bvec4(255);
		//	w_input()->set_size_by_width(100.f);
		//	w_content()->add_child(w_input());

		//	w_ext() = wCombo::create(instance());
		//	w_ext()->size_policy_hori = SizeFitLayout;
		//	w_ext()->align = AlignLittleEnd;
		//	add_style_buttoncolor(w_ext(), 0, Vec3(0.f, 0.f, 0.7f));
		//	{
		//		if (exts == nullptr)
		//			exts = L"All Files (*.*)\0";
		//		auto sp = doublenull_string_split(exts);
		//		for (auto &s : sp)
		//		{
		//			auto i = wMenuItem::create(instance(), s.c_str());
		//			w_ext()->w_items()->add_child(i);
		//		}
		//	}

		//	w_ext()->add_listener(ListenerChanged, [this]() {
		//		set_string_storage(1, w_ext()->w_btn()->text());
		//		set_path(string_storage(0));
		//	});
		//	w_content()->add_child(w_ext());
		//	w_ext()->set_sel(0);

		//	w_buttons() = wLayout::create(instance());
		//	w_buttons()->align = AlignMiddle;
		//	w_buttons()->layout_type = LayoutHorizontal;
		//	w_buttons()->item_padding = 4.f;

		//	w_ok() = wButton::create(instance());
		//	w_ok()->set_classic(io == 0 ? L"Open" : L"Save");
		//	w_ok()->align = AlignLittleEnd;
		//	w_buttons()->add_child(w_ok());

		//	w_cancel() = wButton::create(instance());
		//	w_cancel()->set_classic(L"Cancel");
		//	w_cancel()->align = AlignLittleEnd;
		//	w_buttons()->add_child(w_cancel());

		//	w_ok()->add_listener(ListenerClicked, [this, io, callback]() {
		//		auto full_filename = std::wstring(string_storage(0)) + L"/" + w_input()->text();
		//		if (io == 0)
		//		{
		//			if (std::filesystem::exists(full_filename))
		//			{
		//				callback(true, full_filename.c_str());
		//				remove_from_parent(true);
		//			}
		//			else
		//				wMessageDialog::create(instance(), L"File doesn't exist.", -1.f, L"");
		//		}
		//		else
		//		{
		//			if (std::filesystem::exists(full_filename))
		//			{
		//				wYesNoDialog::create(instance(), L"", -1.f,
		//					L"File already exists, would you like to cover it?", L"Cover", L"Cancel", [&](bool b)
		//				{
		//					if (b)
		//					{
		//						callback(true, full_filename.c_str());
		//						remove_from_parent(true);
		//					}
		//				});
		//			}
		//			else
		//			{
		//				callback(true, full_filename.c_str());
		//				remove_from_parent(true);
		//			}
		//		}
		//	});

		//	w_cancel()->add_listener(ListenerClicked, [this, callback]() {
		//		callback(false, nullptr);
		//		remove_from_parent(true);
		//	});

		//	w_content()->add_child(w_buttons());

		//	pos = (Vec2(instance()->root()->size) - size) * 0.5f;

		//	instance()->root()->add_child(this, 0, -1, true, [](CommonData *d) {
		//		auto thiz = (Widget*)d[0].p;

		//		thiz->instance()->set_focus_widget(thiz);
		//		thiz->instance()->set_dragging_widget(nullptr);
		//	}, "p", this);
		//}

		//wMenuBarPtr &wFileDialog::w_pathstems()
		//{
		//	return *((wMenuBarPtr*)&data_storage(3).p);
		//}

		//wListPtr &wFileDialog::w_list()
		//{
		//	return *((wListPtr*)&data_storage(4).p);
		//}

		//wEditPtr &wFileDialog::w_input()
		//{
		//	return *((wEditPtr*)&data_storage(5).p);
		//}

		//wComboPtr &wFileDialog::w_ext()
		//{
		//	return *((wComboPtr*)&data_storage(6).p);
		//}

		//wLayoutPtr &wFileDialog::w_buttons()
		//{
		//	return *((wLayoutPtr*)&data_storage(7).p);
		//}

		//wButtonPtr &wFileDialog::w_ok()
		//{
		//	return *((wButtonPtr*)&data_storage(8).p);
		//}

		//wButtonPtr &wFileDialog::w_cancel()
		//{
		//	return *((wButtonPtr*)&data_storage(9).p);
		//}

		//const wchar_t *wFileDialog::curr_path()
		//{
		//	return string_storage(0);
		//}

		//int wFileDialog::curr_path_len()
		//{
		//	return string_storage_len(0);
		//}

		//void wFileDialog::set_curr_path(const wchar_t *path)
		//{
		//	return set_string_storage(0, path);
		//}

		//const wchar_t *wFileDialog::curr_exts()
		//{
		//	return string_storage(1);
		//}

		//int wFileDialog::curr_exts_len()
		//{
		//	return string_storage_len(1);
		//}

		//void wFileDialog::set_curr_exts(const wchar_t *exts)
		//{
		//	return set_string_storage(1, exts);
		//}

		//void wFileDialog::set_path(const wchar_t *path)
		//{
		//	w_pathstems()->clear_children(0, 0, -1, true);
		//	w_list()->clear_children(0, 1, -1, true);
		//	w_list()->w_sel() = nullptr;

		//	set_string_storage(0, path);
		//	std::filesystem::path fs_path(path);
		//	{
		//		std::vector<std::wstring> stems;
		//		auto fs_root_path = fs_path.root_path();
		//		std::filesystem::path p(path);
		//		while (p != fs_root_path)
		//		{
		//			stems.push_back(p.filename().generic_wstring());
		//			p = p.parent_path();
		//		}
		//		auto root_path = fs_root_path.generic_wstring();
		//		if (root_path[root_path.size() - 1] != ':')
		//			root_path = string_cut(root_path, -1);
		//		stems.push_back(root_path);
		//		std::reverse(stems.begin(), stems.end());

		//		std::wstring curr_path(L"");
		//		auto build_btn_pop = [&](const std::wstring &path) {
		//			auto btn_pop = wMenu::create(instance(), Icon_CARET_RIGHT, 0.f);
		//			btn_pop->align = AlignMiddle;
		//			w_pathstems()->add_child(btn_pop, 0, -1, true);

		//			if (path == L"")
		//			{
		//				wchar_t drive_strings[256];
		//				GetLogicalDriveStringsW(FLAME_ARRAYSIZE(drive_strings), drive_strings);

		//				auto drives = doublenull_string_split(drive_strings);
		//				for (auto &d : drives)
		//				{
		//					auto i = wMenuItem::create(instance(), d.c_str());
		//					auto path = string_cut(d, -1);
		//					i->add_listener(ListenerClicked, [this, path]() {
		//						set_path(path.c_str());
		//					});
		//					btn_pop->w_items()->add_child(i, 0, -1, true);
		//				}
		//			}
		//			else
		//			{
		//				for (std::filesystem::directory_iterator end, it(path); it != end; it++)
		//				{
		//					if (std::filesystem::is_directory(it->status()))
		//					{
		//						auto str = it->path().filename().generic_wstring();
		//						auto i = wMenuItem::create(instance(), str.c_str());
		//						str = path + L"/" + str;
		//						i->add_listener(ListenerClicked, [this, str]() {
		//							set_path(str.c_str());
		//						});
		//						btn_pop->w_items()->add_child(i, 0, -1, true);
		//					}
		//				}
		//			}

		//			return btn_pop;
		//		};
		//		for (auto &s : stems)
		//		{
		//			auto btn_pop = build_btn_pop(curr_path);

		//			curr_path += s;

		//			auto btn_path = wButton::create(instance());
		//			btn_path->background_col.w = 0;
		//			btn_path->align = AlignMiddle;
		//			btn_path->set_classic(s.c_str());
		//			w_pathstems()->add_child(btn_path, 0, -1, true);

		//			btn_path->add_listener(ListenerClicked, [this, curr_path]() {
		//				set_path(curr_path.c_str());
		//			});

		//			curr_path += L"/";
		//		}
		//		build_btn_pop(curr_path);
		//	}

		//	std::vector<wListItem*> dir_list;
		//	std::vector<wListItem*> file_list;

		//	std::vector<std::wstring> exts_sp;
		//	auto sp = string_regex_split(std::wstring(string_storage(1)), std::wstring(LR"(\(*(\.\w+)\))"), 1);
		//	for (auto &e : sp)
		//		exts_sp.push_back(e);

		//	for (std::filesystem::directory_iterator end, it(fs_path); it != end; it++)
		//	{
		//		auto filename = it->path().filename().generic_wstring();
		//		auto item = wListItem::create(instance());

		//		if (std::filesystem::is_directory(it->status()))
		//		{
		//			item->w_btn()->set_text_and_size((Icon_FOLDER_O + std::wstring(L" ") + filename).c_str());
		//			dir_list.push_back(item);

		//			item->w_btn()->add_listener(ListenerDoubleClicked, [this, filename]() {
		//				set_path((std::wstring(string_storage(0)) + L"/" + filename).c_str());
		//			});
		//		}
		//		else
		//		{
		//			auto found_ext = false;
		//			for (auto &e : exts_sp)
		//			{
		//				if (e == L".*" || it->path().extension() == e)
		//				{
		//					found_ext = true;
		//					break;
		//				}
		//			}
		//			if (!found_ext)
		//				continue;

		//			item->w_btn()->set_text_and_size((Icon_FILE_O + std::wstring(L" ") + filename).c_str());
		//			file_list.push_back(item);

		//			item->w_btn()->add_listener(ListenerClicked, [this, filename]() {
		//				w_input()->set_text(filename.c_str());
		//			});

		//			item->w_btn()->add_listener(ListenerDoubleClicked, [this]() {
		//				w_ok()->on_clicked();
		//			});
		//		}
		//	}

		//	for (auto &i : dir_list)
		//		w_list()->add_child(i, 0, -1, true);
		//	for (auto &i : file_list)
		//		w_list()->add_child(i, 0, -1, true);
		//}

		//wFileDialog *wFileDialog::create(Instance *ui, const wchar_t *title, int io, const std::function<void(bool ok, const wchar_t *filename)> &callback, const wchar_t *exts)
		//{
		//	auto w = (wFileDialog*)Widget::create(ui);
		//	w->init(title, io, callback, exts);

		//	return w;
		//}
	}
}

