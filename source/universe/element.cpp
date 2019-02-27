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

#include <flame/foundation/serialize.h>
#include <flame/graphics/font.h>
#include <flame/universe/icon.h>
#include <flame/universe/element.h>
#include "ui_private.h"

namespace flame
{
	const Vec2 hidden_pos(9999.f);

	Element::Element(UI* ui) :
		ui(ui)
	{
		class$ = "";

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

		closet_id$ = 0;

		draw_default$ = true;

		parent = nullptr;
		layer = 0;
		flag = FlagNull;
		need_arrange = false;
	}

	Element::~Element()
	{
		if (this == ui->hovering_element())
			ui->set_hovering_element(nullptr);
		if (this == ui->focus_element())
			ui->set_focus_element(nullptr);
		if (this == ui->key_focus_element())
			ui->set_key_focus_element(nullptr);
		if (this == ui->dragging_element())
			ui->set_dragging_element(nullptr);
		if (this == ui->popup_element())
			ui->set_popup_element(nullptr);
	}

	void Element::set_width(float x, Element * sender)
	{
		if (size$.x == x)
			return;
		size$.x = x;
		if (sender != this)
			need_arrange = true;
		if (parent&& parent != sender)
			parent->need_arrange = true;
	}

	void Element::set_height(float y, Element * sender)
	{
		if (size$.y == y)
			return;
		size$.y = y;
		if (sender != this)
			need_arrange = true;
		if (parent&& parent != sender)
			parent->need_arrange = true;
	}

	void Element::set_size(const Vec2 & v, Element * sender)
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
			need_arrange = true;
		if (parent&& parent != sender)
			parent->need_arrange = true;
	}

	void Element::set_visibility(bool v)
	{
		if (visible$ == v)
			return;

		visible$ = v;
		if (!visible$)
			remove_animations();
		need_arrange = true;
		if (parent)
			parent->need_arrange = true;
	}

	void Element::add_child(Element * w, int layer, int pos, bool modual)
	{
		auto& children = layer == 0 ? children_1$ : children_2$;
		if (pos < 0)
			pos = children.size + pos + 1;
		children.insert(pos, w);

		w->parent = this;
		w->layer = layer;
		w->flag = modual ? FlagJustCreatedNeedModual : FlagJustCreated;

		need_arrange = true;

		for (auto i = 0; i < child_listeners$.size; i++)
		{
			auto& f = child_listeners$[i];
			auto& p = (ChildListenerParm&)f.p;
			p.thiz() = this;
			p.op() = ChildAdd;
			p.src() = w;
			f.exec();
		}
	}

	void Element::remove_child(int layer, int idx, bool delay)
	{
		auto& children = layer == 0 ? children_1$ : children_2$;
		auto w = children[idx];
		if (delay)
		{
			w->flag = FlagNeedToRemoveFromParent;
			return;
		}
		Element::destroy(w);
		children.remove(idx);

		need_arrange = true;
	}

	void Element::take_child(int layer, int idx, bool delay)
	{
		auto& children = layer == 0 ? children_1$ : children_2$;
		auto w = children[idx];
		if (delay)
		{
			w->flag = FlagNeedToTakeFromParent;
			return;
		}
		children.remove(idx);

		remove_animations();

		need_arrange = true;
	}

	void Element::clear_children(int layer, int begin, int end, bool delay)
	{
		auto& children = layer == 0 ? children_1$ : children_2$;
		if (end < 0)
			end = children.size + end + 1;
		if (!delay)
		{
			for (auto i = begin; i < end; i++)
				Element::destroy(children[i]);
			children.remove(begin, end - begin);

			need_arrange = true;
		}
		else
		{
			for (auto i = begin; i < end; i++)
				children[i]->flag = FlagNeedToRemoveFromParent;
		}
	}

	void Element::take_children(int layer, int begin, int end, bool delay)
	{
		auto& children = layer == 0 ? children_1$ : children_2$;
		if (end == -1)
			end = children.size;
		if (!delay)
		{
			for (auto i = begin; i < end; i++)
				children[i]->remove_animations();
			children.remove(begin, end - begin);

			need_arrange = true;
		}
		else
		{
			for (auto i = begin; i < end; i++)
				children[i]->flag = FlagNeedToTakeFromParent;
		}
	}

	void Element::remove_from_parent(bool delay)
	{
		if (delay)
		{
			flag = FlagNeedToRemoveFromParent;
			return;
		}

		if (parent)
			parent->remove_child(layer, parent->find_child(layer, this));
	}

	void Element::take_from_parent(bool delay)
	{
		if (delay)
		{
			flag = FlagNeedToTakeFromParent;
			return;
		}

		if (parent)
			parent->take_child(layer, parent->find_child(layer, this));
	}

	int Element::find_child(int layer, Element * w)
	{
		auto& children = layer == 0 ? children_1$ : children_2$;
		return children.find(w);
	}

	void Element::set_to_foreground()
	{
		auto& list = layer == 0 ? parent->children_1$ : parent->children_2$;
		for (auto i = list.find(this); i < list.size - 1; i++)
			list[i] = list[i + 1];
		list[list.size - 1] = this;
	}

	void Element::do_arrange()
	{
		switch (layout_type$)
		{
		case LayoutVertical:
		{
			if (size_policy_hori$ == SizeFitChildren || size_policy_hori$ == SizeGreedy)
			{
				auto width = 0;
				for (auto i_c = 0; i_c < children_1$.size; i_c++)
				{
					auto c = children_1$[i_c];

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
				for (auto i_c = 0; i_c < children_1$.size; i_c++)
				{
					auto c = children_1$[i_c];

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
				for (auto i_c = 0; i_c < children_1$.size; i_c++)
				{
					auto c = children_1$[i_c];

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

				for (auto i_c = 0; i_c < children_1$.size; i_c++)
				{
					auto c = children_1$[i_c];

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

			for (auto i_c = 0; i_c < children_1$.size; i_c++)
			{
				auto c = children_1$[i_c];

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
				for (auto i_c = 0; i_c < children_1$.size; i_c++)
				{
					auto c = children_1$[i_c];

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
				for (auto i_c = 0; i_c < children_1$.size; i_c++)
				{
					auto c = children_1$[i_c];

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

				for (auto i_c = 0; i_c < children_1$.size; i_c++)
				{
					auto c = children_1$[i_c];

					if (!c->visible$)
						continue;

					if (c->size_policy_hori$ == SizeFitLayout)
						c->set_width(width, this);
				}
			}
			if (size_policy_vert$ == SizeFitChildren || size_policy_vert$ == SizeGreedy)
			{
				auto height = 0;
				for (auto i_c = 0; i_c < children_1$.size; i_c++)
				{
					auto c = children_1$[i_c];

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
			for (auto i_c = 0; i_c < children_1$.size; i_c++)
			{
				auto c = children_1$[i_c];

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
			for (auto i_c = 0; i_c < children_1$.size; i_c++)
			{
				auto c = children_1$[i_c];

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

		for (auto i_c = 0; i_c < children_2$.size; i_c++)
		{
			auto c = children_2$[i_c];

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

	const auto scroll_spare_spacing = 20.f;

	float Element::get_content_size() const
	{
		return content_size + scroll_spare_spacing;
	}

	void Element::remove_animations()
	{
		for (auto i = 0; i < animations$.size; i++)
		{
			auto& a = animations$[i];
			a.time = -1.f;
			a.f$.p.thiz() = &a;
			a.f$.p.e() = this;
			a.f$.exec();
		}
		animations$.resize(0);
		for (auto i = 0; i < children_1$.size; i++)
			children_1$[i]->remove_animations();
		for (auto i = 0; i < children_2$.size; i++)
			children_2$[i]->remove_animations();
	}

	void Element::on_draw(graphics::Canvas * c, const Vec2 & off, float scl)
	{
		if (draw_default$)
		{
			auto p = (pos$ - Vec2(background_offset$[0], background_offset$[1])) * scl + off;
			auto ss = scl * scale$;
			auto s = (size$ + Vec2(background_offset$[0] + background_offset$[2], background_offset$[1] + background_offset$[3])) * ss;
			auto rr = background_round_radius$ * ss;

			if (background_shaow_thickness$ > 0.f)
			{
				c->add_rect_col2(p - Vec2(background_shaow_thickness$ * 0.5f), s + Vec2(background_shaow_thickness$), Bvec4(0, 0, 0, 128), Bvec4(0),
					background_shaow_thickness$, rr, background_round_flags$);
			}
			if (alpha$ > 0.f)
			{
				if (background_col$.w > 0)
					c->add_rect_filled(p, s, Bvec4(background_col$, alpha$), rr, background_round_flags$);
				if (background_frame_thickness$ > 0.f&& background_frame_col$.w > 0)
					c->add_rect(p, s, Bvec4(background_frame_col$, alpha$), background_frame_thickness$, rr, background_round_flags$);
			}
		}
		for (auto i = 0; i < extra_draws$.size; i++)
		{
			auto& f = extra_draws$[i];
			auto& p = (ExtraDrawParm&)f.p;
			p.thiz() = this;
			p.canvas() = c;
			p.off() = off;
			p.scl() = scl;
			f.exec();
		}
	}

	void Element::on_focus(FocusType type, int is_keyfocus)
	{
		for (auto i = 0; i < focus_listeners$.size; i++)
		{
			auto& f = focus_listeners$[i];
			auto& p = (FoucusListenerParm&)f.p;
			p.thiz() = this;
			p.type() = type;
			p.is_keyfocus() = is_keyfocus;
			f.exec();
		}
	}

	void Element::on_key(KeyState action, int value)
	{
		for (auto i = 0; i < key_listeners$.size; i++)
		{
			auto& f = key_listeners$[i];
			auto& p = (KeyListenerParm&)f.p;
			p.thiz() = this;
			p.action() = action;
			p.value() = value;
			f.exec();
		}
	}

	void Element::on_mouse(KeyState action, MouseKey key, const Vec2 & pos)
	{
		for (auto i = 0; i < mouse_listeners$.size; i++)
		{
			auto& f = mouse_listeners$[i];
			auto& p = (MouseListenerParm&)f.p;
			p.thiz() = this;
			p.action() = action;
			p.key() = key;
			p.value() = pos;
			f.exec();
		}
	}

	void Element::on_drop(Element * src)
	{
		for (auto i = 0; i < drop_listeners$.size; i++)
		{
			auto& f = drop_listeners$[i];
			auto& p = (DropListenerParm&)f.p;
			p.thiz() = this;
			p.src() = src;
			f.exec();
		}
	}

	void Element::on_changed()
	{
		for (auto i = 0; i < changed_listeners$.size; i++)
		{
			auto& f = changed_listeners$[i];
			auto& p = (DropListenerParm&)f.p;
			p.thiz() = this;
			f.exec();
		}
	}

	SerializableNode* Element::save()
	{
		return SerializableNode::serialize(find_udt(cH("Element")), this, 1);
	}

	Element* Element::create(UI * ui)
	{
		return new Element(ui);
	}

	void Element::create_from_typeinfo(UI * ui, int font_idx, VaribleInfo * info, void* p, Element * dst)
	{
		switch (info->tag())
		{
		case VariableTagEnumSingle:
		{
			auto c = createT<wCombo>(ui, find_enum(info->type_hash()), (char*)p + info->offset());
			c->align$ = AlignLittleEnd;
			dst->add_child(c, 0, -1, true);
		}
			break;
		case VariableTagEnumMulti:
			break;
		case VariableTagVariable:
		{
			switch (info->type_hash())
			{
			case cH("bool"):
			{
				auto c = createT<wCheckbox>(ui, (char*)p + info->offset());
				c->align$ = AlignLittleEnd;
				dst->add_child(c, 0, -1, true);
			}
				break;
			case cH("uint"):
			{
				auto e = createT<wEdit>(ui, font_idx, wEdit::TypeUint, (char*)p + info->offset());
				e->size_policy_hori$ = SizeFitLayout;
				e->align$ = AlignLittleEnd;
				e->set_size_by_width(10.f);
				dst->add_child(e, 0, -1, true);
			}
				break;
			case cH("int"):
			{
				auto e = createT<wEdit>(ui, font_idx, wEdit::TypeInt, (char*)p + info->offset());
				e->size_policy_hori$ = SizeFitLayout;
				e->align$ = AlignLittleEnd;
				e->set_size_by_width(10.f);
				dst->add_child(e, 0, -1, true);
			}
				break;
			case cH("Ivec2"):
			{
				auto pp = (Ivec2*)((char*)p + info->offset());

				for (auto i_v = 0; i_v < 2; i_v++)
				{
					auto e = createT<wEdit>(ui, font_idx, wEdit::TypeInt, &((*pp)[i_v]));
					e->size_policy_hori$ = SizeFitLayout;
					e->align$ = AlignLittleEnd;
					e->set_size_by_width(10.f);
					dst->add_child(e, 0, -1, true);
				}
			}
				break;
			case cH("Ivec3"):
			{
				auto pp = (Ivec3*)((char*)p + info->offset());

				for (auto i_v = 0; i_v < 3; i_v++)
				{
					auto e = createT<wEdit>(ui, font_idx, wEdit::TypeInt, &((*pp)[i_v]));
					e->size_policy_hori$ = SizeFitLayout;
					e->align$ = AlignLittleEnd;
					e->set_size_by_width(10.f);
					dst->add_child(e, 0, -1, true);
				}
			}
				break;
			case cH("Ivec4"):
			{
				auto pp = (Ivec4*)((char*)p + info->offset());

				for (auto i_v = 0; i_v < 4; i_v++)
				{
					auto e = createT<wEdit>(ui, font_idx, wEdit::TypeInt, &((*pp)[i_v]));
					e->size_policy_hori$ = SizeFitLayout;
					e->align$ = AlignLittleEnd;
					e->set_size_by_width(10.f);
					dst->add_child(e, 0, -1, true);
				}
			}
				break;
			case cH("float"):
			{
				auto e = createT<wEdit>(ui, font_idx, wEdit::TypeFloat, (char*)p + info->offset());
				e->size_policy_hori$ = SizeFitLayout;
				e->align$ = AlignLittleEnd;
				e->set_size_by_width(10.f);
				dst->add_child(e, 0, -1, true);
			}
				break;
			case cH("Vec2"):
			{
				auto pp = (Vec2*)((char*)p + info->offset());

				for (auto i_v = 0; i_v < 2; i_v++)
				{
					auto e = createT<wEdit>(ui, font_idx, wEdit::TypeFloat, &((*pp)[i_v]));
					e->size_policy_hori$ = SizeFitLayout;
					e->align$ = AlignLittleEnd;
					e->set_size_by_width(10.f);
					dst->add_child(e, 0, -1, true);
				}
			}
				break;
			case cH("Vec3"):
			{
				auto pp = (Vec3*)((char*)p + info->offset());

				for (auto i_v = 0; i_v < 3; i_v++)
				{
					auto e = createT<wEdit>(ui, font_idx, wEdit::TypeFloat, &((*pp)[i_v]));
					e->size_policy_hori$ = SizeFitLayout;
					e->align$ = AlignLittleEnd;
					e->set_size_by_width(10.f);
					dst->add_child(e, 0, -1, true);
				}
			}
				break;
			case cH("Vec4"):
			{
				auto pp = (Vec4*)((char*)p + info->offset());

				for (auto i_v = 0; i_v < 4; i_v++)
				{
					auto e = createT<wEdit>(ui, font_idx, wEdit::TypeFloat, &((*pp)[i_v]));
					e->size_policy_hori$ = SizeFitLayout;
					e->align$ = AlignLittleEnd;
					e->set_size_by_width(10.f);
					dst->add_child(e, 0, -1, true);
				}
			}
				break;
			case cH("uchar"):
			{
				auto e = createT<wEdit>(ui, font_idx, wEdit::TypeUchar, (char*)p + info->offset());
				e->size_policy_hori$ = SizeFitLayout;
				e->align$ = AlignLittleEnd;
				e->set_size_by_width(10.f);
				dst->add_child(e, 0, -1, true);
			}
				break;
			case cH("Bvec2"):
			{
				auto pp = (Bvec2*)((char*)p + info->offset());

				for (auto i_v = 0; i_v < 2; i_v++)
				{
					auto e = createT<wEdit>(ui, font_idx, wEdit::TypeUchar, &((*pp)[i_v]));
					e->size_policy_hori$ = SizeFitLayout;
					e->align$ = AlignLittleEnd;
					e->set_size_by_width(10.f);
					dst->add_child(e, 0, -1, true);
				}
			}
				break;
			case cH("Bvec3"):
			{
				auto pp = (Bvec3*)((char*)p + info->offset());

				for (auto i_v = 0; i_v < 3; i_v++)
				{
					auto e = createT<wEdit>(ui, font_idx, wEdit::TypeUchar, &((*pp)[i_v]));
					e->size_policy_hori$ = SizeFitLayout;
					e->align$ = AlignLittleEnd;
					e->set_size_by_width(10.f);
					dst->add_child(e, 0, -1, true);
				}
			}
				break;
			case cH("Bvec4"):
			{
				auto pp = (Bvec4*)((char*)p + info->offset());

				for (auto i_v = 0; i_v < 4; i_v++)
				{
					auto e = createT<wEdit>(ui, font_idx, wEdit::TypeUchar, &((*pp)[i_v]));
					e->size_policy_hori$ = SizeFitLayout;
					e->align$ = AlignLittleEnd;
					e->set_size_by_width(10.f);
					dst->add_child(e, 0, -1, true);
				}
			}
				break;
			}
		}
			break;
		case VariableTagArrayOfVariable:
		{
		}
			break;
		case VariableTagArrayOfPointer:
		{
			auto& arr = *(Array<void*>*)((char*)p + info->offset());

			switch (info->type_hash())
			{
			case cH("Function"):
				for (auto i_i = 0; i_i < arr.size; i_i++)
				{
					arr[i_i];
				}
				break;
			}
		}
			break;
		}
	}

	FLAME_PACKAGE_BEGIN_1(ObjGeneratorData, UIPtr, ui, p)
	FLAME_PACKAGE_END_1

	Element* Element::create_from_file(UI * ui, SerializableNode * src)
	{
		return (Element*)src->unserialize(find_udt(cH("Element")), Function<SerializableNode::ObjGeneratorParm>([](SerializableNode::ObjGeneratorParm & p) {
			auto c = p.get_capture<ObjGeneratorData>();
			auto w = create(c.ui());
			p.out_obj() = w;

			if (p.parent())
			{
				w->parent = (ElementPtr)p.parent();
				if (p.att_hash() == cH("children_1"))
					w->layer = 0;
				else /* if (att_hash == cH("children_2")) */
					w->layer = 1;
			}
		}, { ui }));
	}

	void Element::destroy(Element * w)
	{
		delete w;
	}

	void wLayout::init(LayoutType type, float item_padding)
	{
		init_data_types();

		layout_type$ = type;
		item_padding$ = item_padding;
		event_attitude$ = EventIgnore;
		size_policy_hori$ = SizeFitChildren;
		size_policy_vert$ = SizeFitChildren;
	}

	void checkbox_mouse_event$(Element::MouseListenerParm &p)
	{
		if (!p.is_clicked())
			return;

		auto thiz = (wCheckboxPtr)p.thiz();

		thiz->checked() = !thiz->checked();

		if (thiz->target())
			* (bool*)(thiz->target()) = thiz->checked();

		thiz->on_changed();
	}

	void checkbox_extra_draw$(Element::ExtraDrawParm &p)
	{
		auto thiz = (wCheckboxPtr)p.thiz();

		p.canvas()->add_rect(thiz->pos$ * p.scl() + p.off(), thiz->size$ * p.scl(), thiz->background_col$, 2.f * p.scl());
		if (thiz->checked())
			p.canvas()->add_rect_filled((thiz->pos$ + 3.f) * p.scl() + p.off(), (thiz->size$ - 6.f) * p.scl(), thiz->background_col$);
	}

	void wCheckbox::init(void* _target)
	{
		init_data_types();

		size$ = Vec2(16.f);
		background_col$ = Bvec4(255);

		checked() = 0;
		target() = _target;

		if (target())
			checked() = *(bool*)target();

		draw_default$ = false;
		extra_draws$.push_back(Function<ExtraDrawParm>(checkbox_extra_draw$, {}));
		styles$.push_back({ 0, 0, Style::background_color(ui->default_frame_col, ui->default_frame_col_hovering, ui->default_frame_col_active)});
		mouse_listeners$.push_back(Function<MouseListenerParm>(checkbox_mouse_event$, {}));
	}

	void text_extra_draw$(Element::ExtraDrawParm& p)
	{
		auto thiz = (wTextPtr)p.thiz();

		if (thiz->alpha$ > 0.f && thiz->text_col().w > 0.f)
		{
			auto _pos = (thiz->pos$ + Vec2(thiz->inner_padding$[0], thiz->inner_padding$[2])) * p.scl() + p.off();
			p.canvas()->add_text(thiz->font_idx(), _pos, Bvec4(thiz->text_col(), thiz->alpha$), thiz->text$.v, thiz->sdf_scale() * p.scl());
		}
	}

	void wText::init(int _font_idx)
	{
		init_data_types();

		event_attitude$ = EventIgnore;

		font_idx() = _font_idx;
		text_col() = ui->default_text_col;
		sdf_scale() = ui->default_sdf_scale;
		text$ = L"";

		extra_draws$.push_back(Function<ExtraDrawParm>(text_extra_draw$, {}));
	}

	void wText::set_size_auto()
	{
		Vec2 v(0.f);

		if (font_idx() >= 0)
		{
			auto font = ui->canvas()->get_font(font_idx());
			v = Vec2(font->get_text_width(text$.v), font->pixel_height()) * sdf_scale();
		}
		v.x += inner_padding$[0] + inner_padding$[1];
		v.y += inner_padding$[2] + inner_padding$[3];
		set_size(v);
	}

	void wButton::init(int font_idx, const wchar_t* title)
	{
		wText::init(font_idx);
		init_data_types();

		inner_padding$ = Vec4(4.f, 4.f, 2.f, 2.f);
		event_attitude$ = EventAccept;

		styles$.push_back({ 0, 0, Style::background_color(ui->default_button_col, ui->default_button_col_hovering, ui->default_button_col_active) });

		if (title)
		{
			text$ = title;
			set_size_auto();
		}
	}

	void toggle_mouse_event$(Element::MouseListenerParm& p)
	{
		if (!p.is_clicked())
			return;

		auto thiz = (wTogglePtr)p.thiz();
		thiz->set_toggle(!thiz->toggled());
	}

	void wToggle::init(int font_idx)
	{
		wText::init(font_idx);
		init_data_types();

		background_col$ = Bvec4(255, 255, 255, 255 * 0.7f);
		background_round_radius$ = font_idx >= 0 ? ui->canvas()->get_font(font_idx)->pixel_height() * 0.5f : 0.f;
		background_offset$ = Vec4(background_round_radius$, 0.f, background_round_radius$, 0.f);
		background_round_flags$ = Rect::SideNW | Rect::SideNE | Rect::SideSW | Rect::SideSE;

		event_attitude$ = EventAccept;

		toggled() = 0;

		mouse_listeners$.push_back(Function<MouseListenerParm>(toggle_mouse_event$, {}));
	}

	void wToggle::set_toggle(bool v)
	{
		toggled() = v;
		if (!v)
			closet_id$ = 0;
		else
			closet_id$ = 1;

		on_changed();
	}

	static void menu_add_rarrow(wMenu * w)
	{
		if (w->w_rarrow())
			return;

		if (w->parent && w->parent->class$.hash == cH("menubar"))
			return;
		if (!w->sub() && w->class$.hash != cH("combo"))
			return;

		auto title = w->w_title();

		title->inner_padding$[1] += title->size$.y * 0.6f;
		title->set_size_auto();

		auto rarrow = Element::createT<wText>(w->ui, title->font_idx());

		w->w_rarrow() = rarrow;
		rarrow->align$ = AlignRightNoPadding;
		rarrow->sdf_scale() = w->w_title()->sdf_scale();
		rarrow->text$ = w->sub() ? Icon_CARET_RIGHT : Icon_ANGLE_DOWN;
		rarrow->set_size_auto();
		w->add_child(rarrow, 1);
	}

	void menuitem_mouse_event$(Element::MouseListenerParm& p)
	{
		if (!p.is_clicked())
			return;

		auto thiz = (wMenuItemPtr)p.thiz();
		thiz->ui->close_popup();
	}

	void wMenuItem::init(int font_idx, const wchar_t* title)
	{
		wText::init(font_idx);
		init_data_types();

		inner_padding$ = Vec4(4.f, 4.f, 2.f, 2.f);
		size_policy_hori$ = SizeFitLayout;
		align$ = AlignLittleEnd;
		event_attitude$ = EventAccept;

		styles$.push_back({ 0, 0, Style::background_color(Bvec4(0), ui->default_header_col_hovering, ui->default_header_col_active) });
		mouse_listeners$.push_back(Function<MouseListenerParm>(menuitem_mouse_event$, {}));

		text$ = title;
		set_size_auto();
	}

	void menu_title_mouse_event$(Element::MouseListenerParm& p)
	{
		if (!p.is_move())
			return;

		auto menu = (wMenuPtr)p.thiz()->parent;
		if (menu->ui->popup_element())
			menu->open();
	}

	void menu_items_child_event$(Element::ChildListenerParm& p)
	{
		if (p.op() != Element::ChildAdd)
			return;

		auto menu = (wMenuPtr)p.thiz()->parent;
		switch (p.src()->class$.hash)
		{
		case cH("menuitem"):
			menu_add_rarrow(menu);
			break;
		case cH("menu"):
		{
			auto src = (wMenu*)p.src();
			src->sub() = 1;
			src->size_policy_hori$ = SizeGreedy;
			src->w_items()->align$ = AlignRightOutside;

			menu_add_rarrow(menu);
		}
			break;
		}
	}

	void wMenu::init(int font_idx, const wchar_t* title, bool only_for_context_menu)
	{
		wLayout::init();
		init_data_types();

		align$ = AlignLittleEnd;
		layout_type$ = LayoutVertical;

		sub() = 0;
		opened() = 0;

		w_title() = createT<wText>(ui, font_idx);
		w_title()->inner_padding$ = Vec4(4.f, 4.f, 2.f, 2.f);
		w_title()->size_policy_hori$ = SizeGreedy;
		w_title()->align$ = AlignLittleEnd;
		w_title()->event_attitude$ = EventAccept;
		w_title()->text$ = title;
		w_title()->set_size_auto();
		add_child(w_title());

		w_title()->styles$.push_back({ 0, 0, Style::background_color(Bvec4(0), ui->default_header_col_hovering, ui->default_header_col_active) });
		w_title()->mouse_listeners$.push_back(Function<MouseListenerParm>(menu_title_mouse_event$, { this }));

		w_rarrow() = nullptr;

		w_items() = createT<wLayout>(ui, LayoutVertical);
		w_items()->class$ = "menu items";
		w_items()->background_col$ = ui->default_window_col;
		w_items()->align$ = AlignBottomOutside;
		w_items()->visible$ = false;
		add_child(w_items(), 1);

		w_items()->child_listeners$.push_back(Function<ChildListenerParm>(menu_items_child_event$, { this }));

		if (only_for_context_menu)
		{
			pos$ = hidden_pos;
			sub() = 1;
			w_items()->align$ = AlignFree;
			ui->root()->add_child(this, 1);
		}
	}

	void wMenu::open()
	{
		if (opened())
			return;

		if (parent && (parent->class$.hash == cH("menubar") || parent->class$.hash == cH("menu items")))
		{
			for (auto i = 0; i < parent->children_1$.size; i++)
			{
				auto c = parent->children_1$[i];
				if (c->class$.hash == cH("menu"))
					((wMenu*)c)->close();
			}
		}

		w_items()->set_visibility(true);
		for (auto i = 0; i < w_items()->children_1$.size; i++)
		{
			auto w = w_items()->children_1$[i];
			w->animations$.push_back(Animation(0.2f, false, Animation::fade(0.f, w->alpha$)));
		}

		opened() = 1;
	}

	void wMenu::popup(const Vec2 & pos)
	{
		if (opened() || ui->popup_element())
			return;

		open();

		ui->set_popup_element(w_items());
		w_items()->pos$ = pos - hidden_pos;
	}

	void wMenu::close()
	{
		if (!opened())
			return;

		for (auto i = 0; i < w_items()->children_1$.size; i++)
		{
			auto c = w_items()->children_1$[i];
			if (c->class$.hash == cH("menu"))
				((wMenu*)c)->close();
		}

		w_items()->set_visibility(false);

		opened() = 0;
	}

	FLAME_PACKAGE_BEGIN_2(MenuTextMouseInMenubarEventData, ElementPtr, thiz, p, wMenuPtr, menu, p)
	FLAME_PACKAGE_END_2

	void menu_text_mouse_event_in_menubar$(Element::MouseListenerParm &p)
	{
		if (!p.is_clicked())
			return;

		auto& c = p.get_capture<MenuTextMouseInMenubarEventData>();
		auto thiz = c.thiz();
		auto menu = c.menu();
		if (!menu->opened())
		{
			if (!thiz->ui->popup_element())
			{
				menu->open();

				thiz->ui->set_popup_element(thiz);
			}
		}
		else
			thiz->ui->close_popup();
	}

	void menubar_child_event$(Element::ChildListenerParm &p)
	{
		if (p.op() != Element::ChildAdd)
			return;

		if (p.src()->class$.hash == cH("menu"))
			((wMenu*)p.src())->w_title()->mouse_listeners$.push_back(Function<Element::MouseListenerParm>(menu_text_mouse_event_in_menubar$, { p.thiz(), p.src() }));
	}

	void wMenuBar::init()
	{
		wLayout::init();
		init_data_types();

		layout_type$ = LayoutHorizontal;

		child_listeners$.push_back(Function<ChildListenerParm>(menubar_child_event$, { this }));
	}

	void combo_title_mouse_event$(Element::MouseListenerParm &p)
	{
		if (!p.is_clicked())
			return;

		auto combo = (wComboPtr)(p.thiz()->parent);
		if (!combo->opened())
		{
			if (!combo->ui->popup_element())
			{
				combo->open();

				combo->ui->set_popup_element(combo);
			}
		}
		else
			combo->ui->close_popup();
	}

	FLAME_PACKAGE_BEGIN_1(ComboItemStyleData, wComboPtr, combo, p)
	FLAME_PACKAGE_END_1

	void combo_item_style$(StyleParm& p)
	{
		auto combo = p.get_capture<ComboItemStyleData>().combo();
		auto sel = combo->sel();
		auto e = p.e();
		if (sel != -1 && combo->w_items()->children_1$[sel] == e)
		{
			if (e->state == StateNormal)
				e->background_col$ = e->ui->default_header_col;
		}
	}

	FLAME_PACKAGE_BEGIN_2(ComboItemsMouseEventData, wComboPtr, combo, p, int, idx, i1)
	FLAME_PACKAGE_END_2

	void combo_item_mouse_event$(Element::MouseListenerParm &p)
	{
		if (!p.is_clicked())
			return;

		auto c = p.get_capture<ComboItemsMouseEventData>();
		c.combo()->set_sel(c.idx());
	}

	void combo_items_child_event$(Element::ChildListenerParm& p)
	{
		if (p.op() != Element::ChildAdd)
			return;

		auto combo = (wComboPtr)(p.thiz()->parent);
		if (p.src()->class$.hash == cH("menuitem"))
		{
			combo->set_width(combo->inner_padding$[0] + combo->inner_padding$[1] + combo->w_title()->inner_padding$[0] + combo->w_title()->inner_padding$[1] + combo->w_items()->size$.x);
			auto idx = combo->w_items()->children_1$.size - 1;
			p.src()->styles$.push_back(Style(0, 1, Function<StyleParm>(combo_item_style$, { combo })));
			p.src()->mouse_listeners$.push_back(Function<Element::MouseListenerParm>(combo_item_mouse_event$, { combo, idx }));
		}
	}

	void wCombo::init(void* _enum_info, void* _target, int font_idx)
	{
		((wMenu*)this)->init(-1, L"");
		init_data_types();

		sel() = -1;
		enum_info() = _enum_info;
		target() = _target;

		background_frame_thickness$ = 1.f;

		size_policy_hori$ = SizeFixed;

		w_title()->size_policy_hori$ = SizeFitLayout;
		w_title()->mouse_listeners$.push_back(Function<MouseListenerParm>(combo_title_mouse_event$, {}));

		w_items()->child_listeners$.push_back(Function<ChildListenerParm>(combo_items_child_event$, {}));

		if (enum_info())
		{
			auto e = (EnumInfo*)enum_info();

			for (auto i = 0; i < e->item_count(); i++)
				w_items()->add_child(createT<wMenuItem>(ui, font_idx, s2w(e->item(i)->name()).c_str()));
		}

		if (target())
		{
			auto p = (int*)target();
			if (enum_info())
			{
				auto e = (EnumInfo*)enum_info();
				set_sel(e->find_item(*p), true);
			}
			else
				set_sel(*p, true);
		}
	}

	void wCombo::set_sel(int idx, bool from_inner)
	{
		sel() = idx;
		auto i = (wMenuItem*)w_items()->children_1$[idx];
		w_title()->text$ = i->text$;

		if (from_inner)
			return;

		if (target())
		{
			auto p = (int*)target();
			if (enum_info())
			{
				auto e = (EnumInfo*)enum_info();
				*p = e->item(sel())->value();
			}
			else
				*p = sel();
		}

		on_changed();
	}

	void edit_extra_draw$(Element::ExtraDrawParm &p)
	{
		auto thiz = (wEditPtr)p.thiz();
		auto font_idx = thiz->font_idx();
		if (font_idx < 0)
			return;
		auto ui = thiz->ui;
		if (ui->key_focus_element() == thiz && int(ui->total_time() * 2) % 2 == 0)
		{
			auto font = ui->canvas()->get_font(font_idx);
			auto len = font->get_text_width(thiz->text$.v, thiz->text$.v + thiz->cursor());
			auto pos = (thiz->pos$ + Vec2(thiz->inner_padding$[0], thiz->inner_padding$[2])) * p.scl() + p.off();
			auto scl = p.scl() * thiz->sdf_scale();
			p.canvas()->add_text(font_idx, pos + Vec2(len - 1.f, 0.f) * scl, thiz->text_col(), L"|", scl);
		}
	}

	void edit_key_event$(Element::KeyListenerParm &p)
	{
		auto thiz = (wEditPtr)p.thiz();
		if (p.action() == KeyStateNull)
		{
			if (thiz->type() != wEdit::TypeNull && p.value() != '\b' && p.value() != 22 && p.value() != 27)
			{
				switch (thiz->type())
				{
				case wEdit::TypeInt:
					if (p.value() == L'-')
					{
						if (thiz->cursor() != 0 || thiz->text$.v[0] == L'-')
							return;
					}
					if (p.value() < '0' || p.value() > '9')
						return;
					break;
				case wEdit::TypeUint: case wEdit::TypeUchar:
					if (p.value() < '0' || p.value() > '9')
						return;
					break;
				case wEdit::TypeFloat:
					if (p.value() == L'.')
					{
						if (thiz->text$.find(L'.') != -1)
							return;
					}
					if (p.value() < '0' || p.value() > '9')
						return;
					break;
				}
			}

			switch (p.value())
			{
			case L'\b':
				if (thiz->cursor() > 0)
				{
					thiz->cursor()--;
					thiz->text$.remove(thiz->cursor());
					thiz->on_changed();
				}
				break;
			case 22:
			{
				auto str = get_clipboard();

				thiz->cursor() = 0;
				thiz->text$ = str.v;
				thiz->on_changed();
			}
				break;
			case 27:
				break;
			default:
				thiz->text$.insert(thiz->cursor(), p.value());
				thiz->cursor()++;
				thiz->on_changed();
			}
		}
		else if (p.action() == KeyStateDown)
		{
			switch (p.value())
			{
			case Key_Left:
				if (thiz->cursor() > 0)
					thiz->cursor()--;
				break;
			case Key_Right:
				if (thiz->cursor() < thiz->text$.size)
					thiz->cursor()++;
				break;
			case Key_Home:
				thiz->cursor() = 0;
				break;
			case Key_End:
				thiz->cursor() = thiz->text$.size;
				break;
			case Key_Del:
				if (thiz->cursor() < thiz->text$.size)
				{
					thiz->text$.remove(thiz->cursor());
					thiz->on_changed();
				}
				break;
			}
		}
	}

	void edit_focus_event$(Element::FoucusListenerParm& p)
	{
		if (p.is_keyfocus() != 1)
			return;

		auto thiz = (wEditPtr)p.thiz();
		switch (p.type())
		{
		case Focus_Gain:
			if (thiz->target())
				thiz->cursor() = thiz->text$.size;
			break;
		case Focus_Lost:
			if (((wEdit*)p.thiz())->target())
			{
				switch (((wEdit*)p.thiz())->type())
				{
				case wEdit::TypeInt:
				{
					auto v = (int*)(thiz->target());
					*v = stoi1(thiz->text$.v);
					thiz->text$ = to_wstring(*v);
					thiz->cursor() = 0;
				}
					break;
				case wEdit::TypeUint:
				{
					auto v = (uint*)(thiz->target());
					*v = stou1(thiz->text$.v);
					thiz->text$ = to_wstring(*v);
					thiz->cursor() = 0;
				}
					break;
				case wEdit::TypeFloat:
				{
					auto v = (float*)(thiz->target());
					*v = stof1(thiz->text$.v);
					thiz->text$ = to_wstring(*v);
					thiz->cursor() = 0;
				}
					break;
				case wEdit::TypeUchar:
				{
					auto v = (uchar*)(thiz->target());
					*v = stob1(thiz->text$.v);
					thiz->text$ = to_wstring(*v);
					thiz->cursor() = 0;
				}
					break;
				}
			}
			break;
		}
	}

	void wEdit::init(int font_idx, Type _type, void* _target)
	{
		wText::init(font_idx);
		init_data_types();

		type() = _type;
		target() = _target;

		inner_padding$ = Vec4(4.f, 4.f, 2.f, 2.f);
		background_col$ = ui->default_frame_col;
		event_attitude$ = EventAccept;
		want_key_focus$ = true;

		extra_draws$.push_back(Function<ExtraDrawParm>(edit_extra_draw$, {}));
		key_listeners$.push_back(Function<KeyListenerParm>(edit_key_event$, {}));
		focus_listeners$.push_back(Function<FoucusListenerParm>(edit_focus_event$, {}));

		if (type() != TypeNull && target())
		{
			switch (type())
			{
			case wEdit::TypeInt:
			{
				auto p = (int*)target();
				text$ = to_wstring(*p);
			}
				break;
			case wEdit::TypeUint:
			{
				auto p = (uint*)target();
				text$ = to_wstring(*p);
			}
				break;
			case wEdit::TypeFloat:
			{
				auto p = (float*)target();
				text$ = to_wstring(*p);
			}
				break;
			case wEdit::TypeUchar:
			{
				auto p = (uchar*)target();
				text$ = to_wstring(*p);
			}
				break;
			}
		}
	}

	void wEdit::set_size_by_width(float width)
	{
		set_size(Vec2(width + inner_padding$[0] + inner_padding$[1], 
			(font_idx() >= 0 ? ui->canvas()->get_font(font_idx())->pixel_height() * sdf_scale() : 0.f) + inner_padding$[2] + inner_padding$[3]));
	}

	void image_extra_draw$(Element::ExtraDrawParm& p)
	{
		auto thiz = (wImagePtr)p.thiz();
		auto pos = (thiz->pos$ + Vec2(thiz->inner_padding$[0], thiz->inner_padding$[2])) * p.scl() + p.off();
		auto size = (thiz->size$ - Vec2(thiz->inner_padding$[0] + thiz->inner_padding$[1], thiz->inner_padding$[2] + thiz->inner_padding$[3])) * p.scl() * thiz->scale$;
		if (!thiz->stretch())
			p.canvas()->add_image(pos, size, thiz->id(), thiz->uv0(), thiz->uv1());
		else
			p.canvas()->add_image_stretch(pos, size, thiz->id(), thiz->border());
	}

	void wImage::init()
	{
		init_data_types();

		id() = 0;
		uv0() = Vec2(0.f);
		uv1() = Vec2(1.f);
		stretch() = 0;
		border() = Vec4(0.f);

		extra_draws$.push_back(Function<ExtraDrawParm>(image_extra_draw$, {}));
	}

	void sizedrag_extra_draw$(Element::ExtraDrawParm& p)
	{
		auto thiz = (wSizeDragPtr)p.thiz();
		p.canvas()->add_triangle_filled(
			(thiz->pos$ + Vec2(thiz->size$.x, 0.f)) * p.scl() + p.off(),
			(thiz->pos$ + Vec2(0.f, thiz->size$.y)) * p.scl() + p.off(),
			(thiz->pos$ + Vec2(thiz->size$)) * p.scl() + p.off(),
			thiz->background_col$);
	}

	void sizedrag_mouse_event$(Element::MouseListenerParm& p)
	{
		if (!p.is_move())
			return;

		auto thiz = (wSizeDragPtr)p.thiz();
		if (thiz == thiz->ui->dragging_element())
		{
			auto target = thiz->w_target();
			auto changed = false;
			auto d = p.value() / thiz->parent->scale$;
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
	}

	void wSizeDrag::init(Element * target)
	{
		init_data_types();

		w_target() = target;
		min_size() = Vec2(0.f);

		size$ = Vec2(10.f);
		background_col$ = Bvec4(140, 225, 15, 255 * 0.5f);
		align$ = AlignRightBottomNoPadding;

		draw_default$ = false;
		extra_draws$.push_back(Function<ExtraDrawParm>(sizedrag_extra_draw$, {}));
		styles$.push_back(Style(0, 0, Style::background_color(ui->default_button_col, ui->default_button_col_hovering, ui->default_button_col_active)));
		mouse_listeners$.push_back(Function<MouseListenerParm>(sizedrag_mouse_event$, {}));
	}

	void scrollbar_btn_mouse_event$(Element::MouseListenerParm& p)
	{
		if (p.action() != KeyStateNull)
			return;

		auto scrollbar = (wScrollbarPtr)(p.thiz()->parent);
		if (p.key() == Mouse_Middle)
			scrollbar->on_mouse(KeyStateNull, Mouse_Middle, Vec2(p.value().x, 0.f));
		else
		{
			if (scrollbar->w_btn() == scrollbar->ui->dragging_element())
			{
				scrollbar->w_target()->scroll_offset$ -= (p.value().y / scrollbar->size$.y) * scrollbar->w_target()->get_content_size();
				scrollbar->w_target()->need_arrange = true;
			}
		}
	}

	FLAME_PACKAGE_BEGIN_1(ScrollbarTargetMouseEventData, wScrollbarPtr, scrollbar, p)
	FLAME_PACKAGE_END_1

	void scrollbar_target_mouse_event$(Element::MouseListenerParm& p)
	{
		if (!p.is_scroll())
			return;

		auto scrollbar = p.get_capture<ScrollbarTargetMouseEventData>().scrollbar();
		scrollbar->scroll(p.value().x);
	}

	void scrollbar_style$(StyleParm& p)
	{
		auto thiz = (wScrollbarPtr)p.thiz();
		auto s = thiz->w_target()->size$.y - thiz->w_target()->inner_padding$[2] - thiz->w_target()->inner_padding$[3];
		auto content_size = thiz->w_target()->get_content_size();
		if (content_size > s)
		{
			thiz->w_btn()->set_visibility(true);
			thiz->w_btn()->pos$.y = thiz->size$.y * (-thiz->w_target()->scroll_offset$ / content_size);
			thiz->w_btn()->size$.y = thiz->size$.y * (s / content_size);
		}
		else
			thiz->w_btn()->set_visibility(false);
	}

	void scrollbar_mouse_event$(Element::MouseListenerParm& p)
	{
		if (!p.is_scroll())
			return;

		auto thiz = (wScrollbarPtr)p.thiz();
		thiz->scroll(p.value().x);
	}

	void wScrollbar::init(Element * target)
	{
		wLayout::init();
		init_data_types();

		size$ = Vec2(10.f);
		size_policy_vert$ = SizeFitLayout;
		align$ = AlignRight;
		event_attitude$ = EventAccept;

		w_btn() = createT<wButton>(ui, -1, nullptr);
		w_btn()->size$ = size$;
		w_btn()->background_round_radius$ = 5.f;
		w_btn()->background_round_flags$ = Rect::SideNW | Rect::SideNE | Rect::SideSW | Rect::SideSE;
		w_btn()->mouse_listeners$.push_back(Function<MouseListenerParm>(scrollbar_btn_mouse_event$, {}));
		add_child(w_btn());

		w_target() = target;
		w_target()->mouse_listeners$.push_back(Function<MouseListenerParm>(scrollbar_target_mouse_event$, { this }));

		styles$.push_back(Style(0, 0, Function<StyleParm>(scrollbar_style$, {})));
		mouse_listeners$.push_back(Function<MouseListenerParm>(scrollbar_mouse_event$, {}));
	}

	void wScrollbar::scroll(int v)
	{
		w_target()->scroll_offset$ += v * 20.f;
		w_target()->need_arrange = true;
	}

	void listitem_title_mouse_event$(Element::MouseListenerParm &p)
	{
		if (!p.is_scroll())
			return;

		auto listitem = (wListItemPtr)(p.thiz()->parent);
		if (listitem->parent)
			listitem->parent->on_mouse(KeyStateNull, Mouse_Middle, Vec2(p.value().x, 0.f));
	}

	void wListItem::init(int font_idx, const wchar_t* title)
	{
		wLayout::init();
		init_data_types();

		size_policy_hori$ = SizeFitLayout;
		align$ = AlignLittleEnd;
		layout_type$ = LayoutHorizontal;

		w_title() = createT<wText>(ui, font_idx);
		w_title()->inner_padding$ = Vec4(4.f, 4.f, 2.f, 2.f);
		w_title()->size_policy_hori$ = SizeFitLayout;
		w_title()->align$ = AlignLittleEnd;
		w_title()->event_attitude$ = EventAccept;
		w_title()->text$ = title;
		w_title()->set_size_auto();
		w_title()->mouse_listeners$.push_back(Function<MouseListenerParm>(listitem_title_mouse_event$, {}));
		add_child(w_title());

		styles$.push_back(Style(0, 0, Style::background_color(Bvec4(0), ui->default_header_col_hovering, ui->default_header_col_active)));
	}

	void listitem_title_style$(StyleParm& p)
	{
		auto e = p.e();
		auto listitem = (wListItemPtr)e->parent;
		auto list = (wListPtr)listitem->parent;
		if (list->w_sel() && list->w_sel()->w_title() == e && e->state == StateNormal)
			e->background_col$ = e->ui->default_header_col;
	}

	void listitem_title_mouse_event_in_list$(Element::MouseListenerParm &p)
	{
		if (!(p.is_down() && p.key() == Mouse_Left))
			return;

		auto thiz = (wListItemPtr)(p.thiz()->parent);
		auto list = (wListPtr)thiz->parent;
		list->w_sel() = thiz;
		list->on_changed();
	}

	void list_mouse_event$(Element::MouseListenerParm& p)
	{
		if (!(p.is_down() && p.key() == Mouse_Left))
			return;

		auto thiz = (wListPtr)p.thiz();
		thiz->w_sel() = nullptr;
		thiz->on_changed();
	}

	void list_child_event$(Element::ChildListenerParm &p)
	{
		if (p.op() != Element::ChildAdd)
			return;

		if (p.src()->class$.hash == cH("listitem"))
		{
			auto listitem = (wListItem*)p.src();
			auto title = listitem->w_title();
			title->styles$.push_back(Style(0, 1, Function<StyleParm>(listitem_title_style$, {})));
			title->mouse_listeners$.push_back(Function<Element::MouseListenerParm>(listitem_title_mouse_event_in_list$, {}));
		}
	}

	void wList::init()
	{
		wLayout::init();
		init_data_types();

		inner_padding$ = Vec4(4.f);
		background_col$ = ui->default_frame_col;
		size_policy_hori$ = SizeFitLayout;
		size_policy_vert$ = SizeFitLayout;
		event_attitude$ = EventAccept;
		layout_type$ = LayoutVertical;
		clip$ = true;

		w_scrollbar() = createT<wScrollbar>(ui, this);
		add_child(w_scrollbar(), 1);

		mouse_listeners$.push_back(Function<MouseListenerParm>(list_mouse_event$, {}));
		child_listeners$.push_back(Function<ChildListenerParm>(list_child_event$, {}));
	}

	FLAME_PACKAGE_BEGIN_1(TreenodeTitleStyleData, wTreePtr, tree, p)
	FLAME_PACKAGE_END_1

	void treenode_title_style$(StyleParm& p)
	{
		auto e = p.e();
		auto tree = p.get_capture<TreenodeTitleStyleData>().tree();
		if (tree->w_sel() && tree->w_sel()->w_title() == e)
			e->background_col$ = e->ui->default_header_col;
		else
			e->background_col$ = Bvec4(0);
	}

	FLAME_PACKAGE_BEGIN_1(TreenodeTitleMouseEventData, wTreePtr, tree, p)
	FLAME_PACKAGE_END_1

	void treenode_title_mouse_event$(Element::MouseListenerParm& p)
	{
		auto treenode = (wTreeNodePtr)(p.thiz()->parent);
		auto tree = p.get_capture<TreenodeTitleMouseEventData>().tree();
		if (p.is_down() && p.key() == Mouse_Left)
		{
			tree->w_sel() = treenode;
			tree->on_changed();
		}
		else if (p.is_double_clicked())
			treenode->w_larrow()->on_mouse(KeyState(KeyStateDown | KeyStateUp), Mouse_Null, Vec2(0.f));
	}

	void treenode_larrow_mouse_event$(Element::MouseListenerParm& p)
	{
		if (!p.is_clicked())
			return;

		auto treenode = (wTreeNodePtr)(p.thiz()->parent);
		auto v = !treenode->w_items()->visible$;
		treenode->w_items()->set_visibility(v);
		treenode->w_larrow()->text$ = v ? Icon_CARET_DOWN : Icon_CARET_RIGHT;
	}

	void wTreeNode::init(int font_idx, const wchar_t* title, wTree * tree)
	{
		wLayout::init();
		init_data_types();

		layout_type$ = LayoutVertical;
		align$ = AlignLittleEnd;

		w_title() = createT<wText>(ui, font_idx);
		w_title()->inner_padding$[0] = font_idx >= 0 ? ui->canvas()->get_font(font_idx)->pixel_height() * 0.8f : 0.f;
		w_title()->inner_padding$ += Vec4(4.f, 4.f, 2.f, 2.f);
		w_title()->align$ = AlignLittleEnd;
		w_title()->event_attitude$ = EventAccept;
		w_title()->text$ = title;
		w_title()->set_size_auto();
		w_title()->styles$.push_back(Style(0, 0, Style::text_color(ui->default_text_col, ui->default_text_col_hovering_or_active)));
		if (tree)
			w_title()->styles$.push_back(Style(0, 1, Function<StyleParm>(treenode_title_style$, { tree })));
		w_title()->mouse_listeners$.push_back(Function<MouseListenerParm>(treenode_title_mouse_event$, { tree }));
		add_child(w_title());

		w_items() = createT<wLayout>(ui, LayoutVertical);
		w_items()->layout_padding$ = w_title()->inner_padding$[0];
		w_items()->align$ = AlignLittleEnd;
		w_items()->visible$ = false;
		add_child(w_items());

		w_larrow() = createT<wText>(ui, font_idx);
		w_larrow()->inner_padding$ = Vec4(4.f, 0.f, 4.f, 0.f);
		w_larrow()->background_col$ = Bvec4(255, 255, 255, 0);
		w_larrow()->align$ = AlignLeftTopNoPadding;
		w_larrow()->event_attitude$ = EventAccept;
		w_larrow()->set_size(Vec2(w_title()->inner_padding$[0], w_title()->size$.y));
		w_larrow()->text$ = Icon_CARET_RIGHT;
		w_larrow()->styles$.push_back(Style(0, 0, Style::text_color(ui->default_text_col, ui->default_text_col_hovering_or_active)));
		w_larrow()->mouse_listeners$.push_back(Function<MouseListenerParm>(treenode_larrow_mouse_event$, {}));
		add_child(w_larrow(), 1);
	}

	void wTree::init()
	{
		wLayout::init();
		init_data_types();

		w_sel() = nullptr;

		layout_type$ = LayoutVertical;
	}

	void dialog_mouse_event$(Element::MouseListenerParm& p)
	{
		auto thiz = (wDialogPtr)p.thiz();
		if (p.is_down() && p.key() == Mouse_Left)
			thiz->set_to_foreground();
		else if (p.action() == KeyStateNull && p.key() == Mouse_Null)
		{
			if (thiz == thiz->ui->dragging_element())
				thiz->pos$ += p.value() / thiz->parent->scale$;
		}
	}

	void wDialog::init(bool resize, bool modual)
	{
		wLayout::init();
		init_data_types();

		auto radius = 8.f;

		inner_padding$ = Vec4(radius);
		background_col$ = Colorf(0.5f, 0.5f, 0.5f, 0.9f);
		event_attitude$ = EventAccept;
		item_padding$ = radius;
		if (resize)
		{
			size_policy_hori$ = SizeFitLayout;
			size_policy_vert$ = SizeFitLayout;
			clip$ = true;
		}

		background_offset$[1] = 0.f;
		background_round_radius$ = radius;
		background_round_flags$ = Rect::SideNW | Rect::SideNE | Rect::SideSW | Rect::SideSE;
		background_shaow_thickness$ = 8.f;

		mouse_listeners$.push_back(Function<MouseListenerParm>(dialog_mouse_event$, {}));

		if (resize)
		{
			w_scrollbar() = createT<wScrollbar>(ui, this);
			add_child(w_scrollbar(), 1);

			w_sizedrag() = createT<wSizeDrag>(ui, this);
			w_sizedrag()->min_size() = Vec2(10.f);
			set_size(Vec2(100.f));
			add_child(w_sizedrag(), 1);
		}
		else
			w_sizedrag() = nullptr;

		if (modual)
		{
			pos$ = (Vec2(ui->root()->size$) - size$) * 0.5f;

			ui->root()->add_child(this, 0, -1, true);
		}
	}

	void message_dialog_ok_mouse_event$(Element::MouseListenerParm& p)
	{
		if (!p.is_clicked())
			return;

		p.thiz()->remove_from_parent(true);
	}

	void wMessageDialog::init(int font_idx, const wchar_t* text)
	{
		((wDialog*)this)->init(false, true);
		init_data_types();

		want_key_focus$ = true;

		layout_type$ = LayoutVertical;
		item_padding$ = 8.f;

		w_text() = createT<wText>(ui, font_idx);
		w_text()->align$ = AlignLittleEnd;
		w_text()->text$ = text;
		w_text()->set_size_auto();
		add_child(w_text());

		w_ok() = createT<wButton>(ui, font_idx, L"OK");
		w_ok()->align$ = AlignMiddle;
		w_ok()->mouse_listeners$.push_back(Function<MouseListenerParm>(message_dialog_ok_mouse_event$, {}));
		add_child(w_ok());
	}

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
	//		auto thiz = (Element*)d[0].p;

	//		thiz->instance()->set_focus_Element(thiz);
	//		thiz->instance()->set_dragging_Element(nullptr);
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
	//		auto thiz = (Element*)d[0].p;

	//		thiz->instance()->set_focus_Element(thiz);
	//		thiz->instance()->set_dragging_Element(nullptr);
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
	//		auto thiz = (Element*)d[0].p;

	//		thiz->instance()->set_focus_Element(thiz);
	//		thiz->instance()->set_dragging_Element(nullptr);
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
}

