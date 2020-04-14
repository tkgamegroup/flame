#pragma once

#include <flame/graphics/font.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/image.h>
#include <flame/universe/components/tile_map.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/edit.h>
#include <flame/universe/components/checkbox.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/scrollbar.h>
#include <flame/universe/components/splitter.h>
#include <flame/universe/components/list.h>
#include <flame/universe/components/tree.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/combobox.h>
#include <flame/universe/components/window.h>
#include <flame/universe/components/extra_element_drawing.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/utils/entity.h>
#include <flame/universe/utils/event.h>
#include <flame/universe/utils/layer.h>
#include <flame/universe/utils/style.h>
#include <flame/universe/utils/menu.h>
#include <flame/universe/utils/splitter.h>
#include <flame/universe/utils/window.h>

namespace flame
{
	namespace utils
	{
		FLAME_UNIVERSE_EXPORTS graphics::FontAtlas* current_font_atlas();
		FLAME_UNIVERSE_EXPORTS void push_font_atlas(graphics::FontAtlas* font_atlas);
		FLAME_UNIVERSE_EXPORTS void pop_font_atlas();

		FLAME_UNIVERSE_EXPORTS extern Vec2f next_element_pos;
		FLAME_UNIVERSE_EXPORTS extern Vec2f next_element_size;

		inline cElement* c_element()
		{
			auto c = cElement::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			c->pos = next_element_pos;
			next_element_pos = Vec2f(0.f);
			c->size = next_element_size;
			next_element_size = Vec2f(0.f);
			current_entity()->add_component(c);
			return c;
		}

		inline cText* c_text()
		{
			auto c = cText::create(current_font_atlas());
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cImage* c_image()
		{
			auto c = cImage::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cEventReceiver* c_event_receiver()
		{
			auto c = cEventReceiver::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cStyleColor* c_style_color()
		{
			auto c = cStyleColor::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cStyleColor2* c_style_color2()
		{
			auto c = cStyleColor2::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cStyleTextColor* c_style_text_color()
		{
			auto c = cStyleTextColor::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cStyleTextColor2* c_style_text_color2()
		{
			auto c = cStyleTextColor2::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cEdit* c_edit()
		{
			auto c = cEdit::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cCheckbox* c_checkbox()
		{
			auto c = cCheckbox::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cAligner* c_aligner(Alignx x, Aligny y)
		{
			auto c = cAligner::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			c->x_align_ = x;
			c->y_align_ = y;
			current_entity()->add_component(c);
			return c;
		}

		inline cAligner* c_aligner(SizePolicy width_policy, SizePolicy height_policy)
		{
			auto c = cAligner::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			c->width_policy_ = width_policy;
			c->height_policy_ = height_policy;
			current_entity()->add_component(c);
			return c;
		}

		inline cLayout* c_layout(LayoutType type = LayoutFree, bool width_fit_children = true, bool height_fit_children = true)
		{
			auto c = cLayout::create(type);
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			c->width_fit_children = width_fit_children;
			c->height_fit_children = height_fit_children;
			current_entity()->add_component(c);
			return c;
		}

		inline cScrollbar* c_scrollbar()
		{
			auto c = cScrollbar::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cSplitter* c_splitter(SplitterType type)
		{
			auto c = cSplitter::create(type);
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cScrollbarThumb* c_scrollbar_thumb(ScrollbarType type)
		{
			auto c = cScrollbarThumb::create(type);
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cList* c_list()
		{
			auto c = cList::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cListItem* c_list_item()
		{
			auto c = cListItem::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cTree* c_tree()
		{
			auto c = cTree::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cTreeNode* c_tree_node()
		{
			auto c = cTreeNode::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cTreeNodeTitle* c_tree_node_title()
		{
			auto c = cTreeNodeTitle::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cTreeNodeArrow* c_tree_node_arrow()
		{
			auto c = cTreeNodeArrow::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cTreeLeaf* c_tree_leaf()
		{
			auto c = cTreeLeaf::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cMenu* c_menu(cMenu::Mode mode)
		{
			auto c = cMenu::create(mode);
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cMenuItem* c_menu_item()
		{
			auto c = cMenuItem::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cCombobox* c_combobox()
		{
			auto c = cCombobox::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cComboboxItem* c_combobox_item()
		{
			auto c = cComboboxItem::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cMoveable* c_moveable()
		{
			auto c = cMoveable::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cBringToFront* c_bring_to_front()
		{
			auto c = cBringToFront::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cDockerTab* c_docker_tab()
		{
			auto c = cDockerTab::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cDockerStaticContainer* c_docker_static_container()
		{
			auto c = cDockerStaticContainer::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline cExtraElementDrawing* c_extra_element_drawing()
		{
			auto c = cExtraElementDrawing::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity()->add_component(c);
			return c;
		}

		inline Entity* e_empty()
		{
			Entity* e;
			if (next_entity)
			{
				e = next_entity;
				next_entity = nullptr;
			}
			else
				e = Entity::create();
			auto p = current_parent();
			if (p)
			{
				auto pos = (int)p->child_count() - 1;
				while (pos >= 0)
				{
					auto c = p->child(pos);
					if (c->get_component(cBringToFront) || c->get_component(cSizeDragger))
						pos--;
					else if (c->get_component(cSplitter))
					{
						if (pos == 0)
							pos--;
						else
							break;
					}
					else
						break;
				}
				pos++;
				p->add_child(e, pos);
			}
			set_current_entity(e);
			return e;
		}

		inline Entity* e_element()
		{
			auto e = e_empty();
			c_element();
			return e;
		}

		inline Entity* e_begin_layout(LayoutType type = LayoutFree, float item_padding = 0.f, bool width_fit_children = true, bool height_fit_children = true)
		{
			auto e = e_empty();
			c_element();
			auto cl = c_layout(type, width_fit_children, height_fit_children);
			cl->item_padding = item_padding;
			push_parent(e);
			return e;
		}

		inline void e_end_layout()
		{
			pop_parent();
		}

		inline Entity* e_text(const wchar_t* text)
		{
			auto e = e_empty();
			c_element();
			auto t = c_text();
			if (text)
				t->set_text(text);
			return e;
		}

		inline Entity* e_button(const wchar_t* text, void(*on_clicked)(void* c) = nullptr, const Mail& _capture = Mail(), bool use_style = true)
		{
			auto e = e_empty();
			c_element()->padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			auto cer = c_event_receiver();
			if (on_clicked)
			{
				struct Capture
				{
					void(*f)(void*);
				}capture;
				capture.f = on_clicked;
				cer->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_clicked(action, key))
						((Capture*)c)->f((char*)c + sizeof(Capture));
					return true;
				}, Mail::expand_original(&capture, _capture));
				f_free(_capture.p);
			}
			if (use_style)
			{
				auto cs = c_style_color();
				cs->color_normal = style_4c(ButtonColorNormal);
				cs->color_hovering = style_4c(ButtonColorHovering);
				cs->color_active = style_4c(ButtonColorActive);
				cs->style();
			}
			return e;
		}

		inline Entity* e_checkbox(const wchar_t* text, bool checked = false)
		{
			if (text[0])
				e_begin_layout(LayoutHorizontal, 4.f);
			auto e = e_empty();
			auto ce = c_element();
			ce->size = 16.f;
			ce->frame_thickness = 3.f;
			ce->frame_color = style_4c(TextColorNormal);
			c_event_receiver();
			auto cs = c_style_color2();
			cs->color_normal[0] = style_4c(FrameColorNormal);
			cs->color_hovering[0] = style_4c(FrameColorHovering);
			cs->color_active[0] = style_4c(FrameColorActive);
			cs->color_normal[1] = style_4c(ButtonColorNormal);
			cs->color_hovering[1] = style_4c(ButtonColorHovering);
			cs->color_active[1] = style_4c(ButtonColorActive);
			cs->style();
			c_checkbox()->set_checked(checked);
			if (text[0])
			{
				e_text(text);
				e_end_layout();
			}
			return e;
		}

		inline Entity* e_image(uint id, float padding = 0.f)
		{
			auto e = e_empty();
			auto ce = c_element();
			ce->size += Vec2f(padding) * 2.f;
			ce->padding = Vec4f(padding);
			c_image()->id = id;
			return e;
		}

		inline Entity* e_edit(float width, const wchar_t* text = nullptr)
		{
			auto e = e_empty();
			next_component_id = FLAME_CHASH("edit");
			c_timer()->interval = 0.5f;
			auto ce = c_element();
			ce->size.x() = width + 8.f;
			ce->padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			ce->color = style_4c(FrameColorNormal);
			ce->frame_color = style_4c(ForegroundColor);
			ce->frame_thickness = 2.f;
			ce->clip_flags = ClipSelf;
			auto ct = c_text();
			ct->auto_width_ = false;
			if (text)
				ct->set_text(text);
			c_event_receiver();
			if (width == 0.f)
				c_aligner(SizeFitParent, SizeFixed);
			c_edit();
			return e;
		}

		inline Entity* e_colorpicker(const Vec4c& init_col)
		{
			auto e = e_empty();
			auto ce = c_element();
			ce->size = 16.f;
			ce->color = init_col;
			c_event_receiver()->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
				push_parent(current_root());
				e_begin_layout(LayoutVertical, 4.f);

				e_end_layout();
				pop_parent();
				return true;
			}, Mail::from_p(ce));
			return e;
		}

		inline Entity* e_drag_edit()
		{
			auto e = e_begin_layout(LayoutVertical);
			e->get_component(cLayout)->fence = 1;

			auto ee = e_edit(50.f);
			ee->set_visible(false);
			push_style_4c(ButtonColorNormal, style_4c(FrameColorNormal));
			push_style_4c(ButtonColorHovering, style_4c(FrameColorHovering));
			push_style_4c(ButtonColorActive, style_4c(FrameColorActive));
			auto ed = e_button(L"");
			pop_style(ButtonColorNormal);
			pop_style(ButtonColorHovering);
			pop_style(ButtonColorActive);
			ed->get_component(cElement)->size.x() = 58.f;
			ed->get_component(cText)->auto_width_ = false;

			e_end_layout();

			struct Capture
			{
				Entity* ee;
				Entity* ed;
			}capture;
			capture.ee = ee;
			capture.ed = ed;

			ee->get_component(cEventReceiver)->focus_listeners.add([](void* c, bool focusing) {
				auto& capture = *(Capture*)c;
				if (!focusing)
				{
					capture.ee->set_visible(false);
					capture.ed->set_visible(true);
				}
				return true;
			}, Mail::from_t(&capture));

			ed->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
				auto& capture = *(Capture*)c;
				if (is_mouse_clicked(action, key) && pos == 0)
				{
					capture.ee->set_visible(true);
					capture.ed->set_visible(false);
					sEventDispatcher::current()->next_focusing = capture.ee->get_component(cEventReceiver);
				}
				return true;
			}, Mail::from_t(&capture));

			return e;
		}

		inline Entity* e_begin_scroll_view1(ScrollbarType type, const Vec2f size, float padding = 0.f)
		{
			auto e = e_empty();
			auto ce = c_element();
			ce->size = size;
			ce->padding = Vec4f(padding);
			ce->clip_flags = ClipChildren;
			if (size == 0.f)
				c_aligner(SizeFitParent, SizeFitParent);
			auto cl = c_layout(type == ScrollbarVertical ? LayoutHorizontal : LayoutVertical);
			cl->item_padding = 4.f;
			cl->width_fit_children = false;
			cl->height_fit_children = false;
			cl->fence = 2;
			push_parent(e);
			return e;
		}

		inline void e_end_scroll_view1(float step = 1.f)
		{
			auto type = current_parent()->get_component(cLayout)->type == LayoutHorizontal ? ScrollbarVertical : ScrollbarHorizontal;
			{
				e_empty();
				auto ce = c_element();
				ce->size = 10.f;
				ce->color = style_4c(ScrollbarColor);
				auto ca = c_aligner(SizeFixed, SizeFixed);
				if (type == ScrollbarVertical)
					ca->height_policy_ = SizeFitParent;
				else
					ca->width_policy_ = SizeFitParent;
				c_event_receiver();
				c_scrollbar();
				push_parent(current_entity());
			}
			cScrollbarThumb* ct;
			{
				e_empty();
				c_element()->size = 10.f;
				c_event_receiver();
				auto cs = c_style_color();
				cs->color_normal = style_4c(ScrollbarThumbColorNormal);
				cs->color_hovering = style_4c(ScrollbarThumbColorHovering);
				cs->color_active = style_4c(ScrollbarThumbColorActive);
				cs->style();
				ct = c_scrollbar_thumb(type);
				ct->step = step;
				pop_parent();
			}
			{
				e_empty();
				c_element();
				auto ce = c_event_receiver();
				ce->pass_checkers.add([](void* c, cEventReceiver* er, bool* pass) {
					*pass = true;
					return true;
				}, Mail());
				ce->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					auto thumb = (*(cScrollbarThumb**)c);
					if (is_mouse_scroll(action, key))
						thumb->update(-pos.x() * 20.f);
					return true;
				}, Mail::from_p(ct));
				c_aligner(SizeFitParent, SizeFitParent);
			}
			pop_parent();
		}

		inline Entity* e_begin_splitter(SplitterType type)
		{
			auto e = e_empty();
			c_element();
			c_aligner(SizeFitParent, SizeFitParent);
			c_layout(type == SplitterHorizontal ? LayoutHorizontal : LayoutVertical, false, false);
			push_parent(e);
			e_empty();
			make_splitter(current_entity(), type);
			return e;
		}

		inline void e_end_splitter()
		{
			pop_parent();
		}

		inline Entity* e_begin_list(bool size_fit_parent, float padding = 4.f)
		{
			auto e = e_empty();
			c_element();
			c_event_receiver();
			if (size_fit_parent)
				c_aligner(SizeFitParent, SizeFitParent);
			auto cl = c_layout(LayoutVertical);
			cl->item_padding = padding;
			cl->width_fit_children = false;
			cl->height_fit_children = false;
			c_list();
			push_parent(e);
			return e;
		}

		inline void e_end_list()
		{
			pop_parent();
		}

		inline Entity* e_list_item(const wchar_t* text, bool align = true)
		{
			auto e = e_empty();
			c_element();
			if (text[0])
				c_text()->set_text(text);
			c_event_receiver();
			auto cs = c_style_color2();
			cs->color_normal[0] = style_4c(FrameColorNormal);
			cs->color_hovering[0] = style_4c(FrameColorHovering);
			cs->color_active[0] = style_4c(FrameColorActive);
			cs->color_normal[1] = style_4c(SelectedColorNormal);
			cs->color_hovering[1] = style_4c(SelectedColorHovering);
			cs->color_active[1] = style_4c(SelectedColorActive);
			cs->style();
			if (align)
				c_aligner(SizeFitParent, SizeFixed);
			c_list_item();
			return e;
		}

		inline Entity* e_begin_tree(bool fit_parent, float padding = 0.f)
		{
			auto e = e_empty();
			auto ce = c_element();
			ce->padding = Vec4f(padding);
			c_event_receiver();
			if (fit_parent)
				c_aligner(SizeFitParent, SizeFitParent);
			auto cl = c_layout(LayoutVertical);
			cl->item_padding = 4.f;
			cl->width_fit_children = !fit_parent;
			cl->height_fit_children = !fit_parent;
			c_tree();
			push_parent(e);
			return e;
		}

		inline void e_end_tree()
		{
			pop_parent();
		}

		inline Entity* e_tree_leaf(const wchar_t* text)
		{
			auto e = e_empty();
			c_element()->padding = Vec4f(style_1u(FontSize) + 4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			c_event_receiver();
			auto cs = c_style_color2();
			cs->color_normal[0] = Vec4c(0);
			cs->color_hovering[0] = style_4c(FrameColorHovering);
			cs->color_active[0] = style_4c(FrameColorActive);
			cs->color_normal[1] = style_4c(SelectedColorNormal);
			cs->color_hovering[1] = style_4c(SelectedColorHovering);
			cs->color_active[1] = style_4c(SelectedColorActive);
			cs->style();
			c_tree_leaf();
			return e;
		}

		inline Entity* e_begin_tree_node(const wchar_t* text, bool collapsed = false)
		{
			auto e = e_empty();
			c_element();
			c_layout(LayoutVertical)->item_padding = 4.f;
			c_tree_node();
			push_parent(e);
			{
				auto e = e_empty();
				c_element()->padding = Vec4f(style_1u(FontSize) + 4.f, 2.f, 4.f, 2.f);
				c_text()->set_text(text);
				c_event_receiver();
				auto cs = c_style_color2();
				cs->color_normal[0] = Vec4c(0);
				cs->color_hovering[0] = style_4c(FrameColorHovering);
				cs->color_active[0] = style_4c(FrameColorActive);
				cs->color_normal[1] = style_4c(SelectedColorNormal);
				cs->color_hovering[1] = style_4c(SelectedColorHovering);
				cs->color_active[1] = style_4c(SelectedColorActive);
				cs->style();
				c_layout();
				c_tree_node_title();
				push_parent(e);
				{
					e_empty();
					c_element()->padding = Vec4f(0.f, 2.f, 4.f, 2.f);
					c_text()->set_text(collapsed ? Icon_CARET_RIGHT : Icon_ANGLE_DOWN);
					c_event_receiver();
					auto cs = c_style_text_color();
					cs->color_normal = style_4c(TextColorNormal);
					cs->color_else = style_4c(TextColorElse);
					cs->style();
					c_tree_node_arrow();
				}
				pop_parent();
			}
			auto es = e_empty();
			es->set_visible(!collapsed);
			c_element()->padding = Vec4f(style_1u(FontSize) * 0.5f, 0.f, 0.f, 0.f);
			c_layout(LayoutVertical)->item_padding = 4.f;
			pop_parent();
			push_parent(es);
			return e;
		}

		inline void e_end_tree_node()
		{
			pop_parent();
		}

		inline Entity* e_begin_combobox()
		{
			auto e = e_empty();
			e->gene = e;
			auto ce = c_element();
			ce->size = Vec2f(8.f, style_1u(FontSize) + 4.f);
			ce->padding = Vec4f(4.f, 2.f, 4.f + style_1u(FontSize), 2.f);
			ce->frame_color = style_4c(TextColorNormal);
			ce->frame_thickness = 2.f;
			c_text()->auto_width_ = false;
			c_event_receiver();
			auto cs = c_style_color();
			cs->color_normal = style_4c(FrameColorNormal);
			cs->color_hovering = style_4c(FrameColorHovering);
			cs->color_active = style_4c(FrameColorActive);
			cs->style();
			c_layout();
			auto cm = c_menu(cMenu::ModeMain);
			cm->root = current_root();
			auto ccb = c_combobox();
			push_parent(e);
			e_empty();
			c_element()->padding = Vec4f(0.f, 2.f, 4.f, 2.f);
			c_text()->set_text(Icon_ANGLE_DOWN);
			c_aligner(AlignxRight, AlignyFree);
			pop_parent();
			push_parent(cm->items);
			return e;
		}

		inline void e_end_combobox(int idx = -1)
		{
			auto eis = current_parent();
			auto ecb = eis->get_component(cMenuItems)->menu->entity;
			auto max_width = 0U;
			for (auto i = 0; i < eis->child_count(); i++)
			{
				auto ct = eis->child(i)->get_component(cText);
				max_width = max(max_width, ct->font_atlas->text_size(ct->font_size_, ct->text()).x());
			}
			ecb->get_component(cElement)->set_width(max_width + style_1u(FontSize), true);
			if (idx != -1)
				ecb->get_component(cCombobox)->set_index(idx, false);
			pop_parent();
		}

		inline Entity* e_combobox_item(const wchar_t* text)
		{
			auto e = e_empty();
			c_element()->padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			c_event_receiver();
			auto cs = c_style_color2();
			cs->color_normal[0] = style_4c(FrameColorNormal);
			cs->color_hovering[0] = style_4c(FrameColorHovering);
			cs->color_active[0] = style_4c(FrameColorActive);
			cs->color_normal[1] = style_4c(SelectedColorNormal);
			cs->color_hovering[1] = style_4c(SelectedColorHovering);
			cs->color_active[1] = style_4c(SelectedColorActive);
			cs->style();
			c_aligner(SizeGreedy, SizeFixed);
			c_combobox_item();
			return e;
		}

		inline Entity* e_begin_menu_bar()
		{
			auto e = e_empty();
			e->gene = e;
			c_element()->color = style_4c(FrameColorNormal);
			c_aligner(SizeFitParent, SizeFixed);
			c_layout(LayoutHorizontal)->item_padding = 4.f;
			push_parent(e);
			return e;
		}

		inline void e_end_menu_bar()
		{
			pop_parent();
		}

		inline Entity* e_begin_menubar_menu(const wchar_t* text, bool transparent = true)
		{
			auto e = e_empty();
			c_element()->padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			c_event_receiver();
			auto cm = c_menu(cMenu::ModeMenubar);
			cm->root = current_root();
			auto cs = c_style_color();
			cs->color_normal = transparent ? Vec4c(0) : style_4c(FrameColorHovering);
			cs->color_hovering = style_4c(FrameColorHovering);
			cs->color_active = style_4c(FrameColorActive);
			cs->style();
			push_parent(cm->items);
			return e;
		}

		inline void e_end_menubar_menu()
		{
			pop_parent();
		}

		inline Entity* e_begin_button_menu(const wchar_t* text)
		{
			auto e = e_empty();
			e->gene = e;
			c_element()->padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			c_event_receiver();
			auto cm = c_menu(cMenu::ModeMain);
			cm->root = current_root();
			auto cs = c_style_color();
			cs->color_normal = style_4c(ButtonColorNormal);
			cs->color_hovering = style_4c(ButtonColorHovering);
			cs->color_active = style_4c(ButtonColorActive);
			cs->style();
			push_parent(cm->items);
			return e;
		}

		inline void e_end_button_menu()
		{
			pop_parent();
		}

		inline Entity* e_begin_sub_menu(const wchar_t* text)
		{
			auto e = e_empty();
			c_element()->padding = Vec4f(4.f, 2.f, 4.f + style_1u(FontSize), 2.f);
			c_text()->set_text(text);
			c_event_receiver();
			auto cm = c_menu(cMenu::ModeSub);
			cm->root = current_root();
			auto cs = c_style_color();
			cs->color_normal = style_4c(FrameColorNormal);
			cs->color_hovering = style_4c(FrameColorHovering);
			cs->color_active = style_4c(FrameColorActive);
			cs->style();
			c_aligner(SizeGreedy, SizeFixed);
			c_layout();
			push_parent(e);
			e_empty();
			c_element()->padding = Vec4f(0.f, 2.f, 4.f, 2.f);
			c_text()->set_text(Icon_CARET_RIGHT);
			c_aligner(AlignxRight, AlignyFree);
			pop_parent();
			push_parent(cm->items);
			return e;
		}

		inline void e_end_sub_menu()
		{
			pop_parent();
		}

		inline Entity* e_menu_item(const wchar_t* text, void(*func)(void* c), const Mail& _capture, bool close_menu = true)
		{
			auto e = e_empty();
			c_element()->padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			struct Capture
			{
				Entity* e;
				bool close;
				void(*f)(void*);
			}capture;
			capture.e = e;
			capture.close = close_menu;
			capture.f = func;
			c_event_receiver()->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					auto& capture = *(Capture*)c;
					if (capture.close)
						remove_layer((Entity*)capture.e->gene);
					capture.f((char*)c + sizeof(Capture));
				}
				return true;
			}, Mail::expand_original(&capture, _capture));
			f_free(_capture.p);
			c_menu_item();
			auto cs = c_style_color();
			cs->color_normal = style_4c(FrameColorNormal);
			cs->color_hovering = style_4c(FrameColorHovering);
			cs->color_active = style_4c(FrameColorActive);
			cs->style();
			c_aligner(SizeGreedy, SizeFixed);
			return e;
		}

		inline Entity* e_separator()
		{
			auto e = e_empty();
			auto ce = c_element();
			ce->size.y() = style_1u(FontSize) * 0.5f;
			ce->padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			ce->color = style_4c(FrameColorNormal);
			auto ceed = c_extra_element_drawing();
			ceed->draw_flags = ExtraDrawHorizontalLine;
			ceed->thickness = 1.f;
			ceed->color = style_4c(TextColorNormal).new_replacely<3>(128);
			c_aligner(SizeGreedy, SizeFixed);
			return e;
		}

		inline void e_begin_popup_menu(bool attach_to_parent = true)
		{
			if (attach_to_parent)
				set_current_entity(current_parent());
			auto cm = c_menu(cMenu::ModeContext);
			cm->root = current_root();
			push_parent(cm->items);
		}

		inline void e_end_popup_menu()
		{
			pop_parent();
		}

		inline void e_color_selector()
		{

		}

		inline Entity* e_begin_window(const wchar_t* title, bool close_button = true)
		{
			Entity* el;
			auto e = e_empty();
			auto ce = c_element();
			ce->size += Vec2f(0.f, 4.f + style_1u(FontSize));
			ce->frame_thickness = 2.f;
			ce->color = style_4c(BackgroundColor);
			ce->frame_color = style_4c(ForegroundColor);
			c_event_receiver();
			c_layout(LayoutVertical)->fence = 2;
			c_moveable();
			push_parent(e);
			{
				e_empty();
				auto ce = c_element();
				ce->padding = Vec4f(4.f, 2.f, 4.f + style_1u(FontSize), 2.f);
				ce->color = style_4c(SelectedTabColorNormal);
				c_text()->set_text(title);
				c_aligner(SizeGreedy, SizeFixed);
				if (close_button)
				{
					c_layout();
					push_parent(current_entity());
					e_button(Icon_TIMES, [](void* c) {
						auto e = *(void**)c;
						looper().add_event([](void* c, bool*) {
							auto e = *(Entity**)c;
							e->parent()->remove_child(e);
						}, Mail::from_p(e));
					}, Mail::from_p(e), false);
					c_aligner(AlignxRight, AlignyFree);
					pop_parent();
				}

				el = e_empty();
				c_element()->padding = Vec4f(8.f, 4.f, 8.f, 4.f);
				c_layout(LayoutVertical)->item_padding = 4.f;

				e_empty();
				c_element();
				c_event_receiver()->pass_checkers.add([](void* c, cEventReceiver* er, bool* pass) {
					*pass = true;
					return true;
				}, Mail());
				c_aligner(SizeFitParent, SizeFitParent);
				c_bring_to_front();
			}
			pop_parent();
			push_parent(el);
			return e;
		}

		inline void e_end_window()
		{
			pop_parent();
		}

		inline Entity* e_begin_docker_floating_container()
		{
			auto e = e_empty();
			make_docker_floating_container(e, next_element_pos, next_element_size);
			next_element_pos = next_element_size = 0.f;
			push_parent(e);
			return e;
		}

		inline void e_end_docker_floating_container()
		{
			pop_parent();
		}

		inline Entity* e_begin_docker_static_container()
		{
			auto e = e_empty();
			e->set_name("docker_static_container");
			auto ce = c_element();
			ce->padding = Vec4f(8.f, 16.f, 8.f, 8.f);
			ce->color = style_4c(BackgroundColor);
			c_event_receiver();
			c_aligner(SizeFitParent, SizeFitParent);
			c_layout(LayoutFree);
			c_docker_static_container();
			push_parent(e);
			return e;
		}

		inline void e_end_docker_static_container()
		{
			pop_parent();
		}

		inline Entity* e_begin_docker_layout(LayoutType type)
		{
			auto e = e_empty();
			make_docker_layout(e, type);
			if (!is_one_of(current_parent()->name_hash(), { FLAME_CHASH("docker_floating_container"), FLAME_CHASH("docker_staitc_container") }))
			{
				auto ca = e->get_component(cAligner);
				ca->x_align_ = AlignxFree;
				ca->y_align_ = AlignyFree;
				ca->using_padding_ = false;
			}
			push_parent(e);
			return e;
		}

		inline void e_end_docker_layout()
		{
			pop_parent();
		}

		inline Entity* e_begin_docker()
		{
			auto e = e_empty();
			make_docker(e);
			if (!is_one_of(current_parent()->name_hash(), { FLAME_CHASH("docker_floating_container"), FLAME_CHASH("docker_staitc_container") }))
			{
				auto ca = e->get_component(cAligner);
				ca->x_align_ = AlignxFree;
				ca->y_align_ = AlignyFree;
				ca->using_padding_ = false;
			}
			push_parent(e);
			return e;
		}

		inline void e_end_docker()
		{
			pop_parent();
		}

		inline std::pair<Entity*, Entity*> e_begin_docker_page(const wchar_t* title)
		{
			push_parent(current_parent()->child(0));
			auto et = e_empty();
			et->set_name("docker_tab");
			c_element()->padding = Vec4f(4.f, 2.f, style_1u(FontSize) + 6.f, 2.f);
			c_text()->set_text(title);
			c_event_receiver();
			auto csb = c_style_color2();
			csb->color_normal[0] = style_4c(TabColorNormal);
			csb->color_hovering[0] = style_4c(TabColorElse);
			csb->color_active[0] = style_4c(TabColorElse);
			csb->color_normal[1] = style_4c(SelectedTabColorNormal);
			csb->color_hovering[1] = style_4c(SelectedTabColorElse);
			csb->color_active[1] = style_4c(SelectedTabColorElse);
			csb->style();
			auto cst = c_style_text_color2();
			cst->color_normal[0] = style_4c(TabTextColorNormal);
			cst->color_else[0] = style_4c(TabTextColorElse);
			cst->color_normal[1] = style_4c(SelectedTabTextColorNormal);
			cst->color_else[1] = style_4c(SelectedTabTextColorElse);
			cst->style();
			c_list_item();
			c_layout();
			auto cdt = c_docker_tab();
			cdt->root = current_root();
			push_parent(et);
			e_button(Icon_TIMES, [](void* c) {
				auto thiz = (*(cDockerTab**)c);
				looper().add_event([](void* c, bool*) {
					auto thiz = (*(cDockerTab**)c);
					thiz->take_away(true);
				}, Mail::from_p(thiz));
			}, Mail::from_p(cdt), false)->get_component(cText)->color_ = style_4c(TabTextColorElse);
			c_aligner(AlignxRight, AlignyFree);
			pop_parent();
			pop_parent();
			push_parent(current_parent()->child(1));
			auto ep = e_empty();
			{
				auto ce = c_element();
				ce->color = style_4c(BackgroundColor);
				ce->clip_flags = ClipChildren;
				auto ca = c_aligner(SizeFitParent, SizeFitParent);
				ca->x_align_ = AlignxLeft;
				ca->y_align_ = AlignyTop;
				ca->using_padding_ = true;
			}
			pop_parent();
			push_parent(ep);
			return std::make_pair(et, ep);
		}

		inline void e_end_docker_page()
		{
			pop_parent();
		}

		inline Entity* e_begin_dialog()
		{
			auto l = add_layer(current_root(), nullptr, true, Vec4c(0, 0, 0, 127));
			set_current_entity(l);
			c_layout();
			push_parent(l);
			auto e = e_empty();
			auto ce = c_element();
			ce->padding = Vec4f(8.f);
			ce->frame_thickness = 2.f;
			ce->color = style_4c(BackgroundColor);
			ce->frame_color  = col3_inv(style_4c(BackgroundColor));
			c_aligner(AlignxMiddle, AlignyMiddle);
			c_layout(LayoutVertical)->item_padding = 4.f;
			pop_parent();
			push_parent(e);
			return e;
		}

		inline void e_end_dialog()
		{
			pop_parent();
		}

		inline Entity* e_message_dialog(const wchar_t* message)
		{
			auto e = e_begin_dialog();
			auto l = e->parent();
			e_text(message);
			e_button(L"OK", [](void* c) {
				remove_layer(*(Entity**)c);
			}, Mail::from_p(l));
			c_aligner(AlignxMiddle, AlignyFree);
			e_end_dialog();
			return e;
		}

		inline Entity* e_confirm_dialog(const wchar_t* title, void (*callback)(void* c, bool yes), const Mail& _capture)
		{
			auto e = e_begin_dialog();
			auto l = e->parent();
			e_text(title);
			e_begin_layout(LayoutHorizontal, 4.f);
			c_aligner(AlignxMiddle, AlignyFree);
			struct Capture
			{
				Entity* l;
				void(*f)(void*, bool);
			}capture;
			capture.l = l;
			capture.f = callback;
			e_button(L"OK", [](void* c) {
				auto& m = *(Capture*)c;
				remove_layer(m.l);
				m.f((char*)c + sizeof(Capture), true);
			}, Mail::expand_original(&capture, _capture));
			e_button(L"Cancel", [](void* c) {
				auto& m = *(Capture*)c;
				remove_layer(m.l);
				m.f((char*)c + sizeof(Capture), false);
			}, Mail::expand_original(&capture, _capture));
			f_free(_capture.p);
			e_end_layout();
			e_end_dialog();
			return e;
		}

		inline Entity* e_input_dialog(const wchar_t* title, void (*callback)(void* c, bool ok, const wchar_t* text), const Mail& _capture, const wchar_t* default_text = nullptr)
		{
			auto e = e_begin_dialog();
			auto l = e->parent();
			e_text(title);
			auto ct = e_edit(100.f)->get_component(cText);
			if (default_text)
				ct->set_text(default_text);
			e_begin_layout(LayoutHorizontal, 4.f);
			c_aligner(AlignxMiddle, AlignyFree);
			struct Capture
			{
				Entity* l;
				cText* t;
				void(*f)(void*, bool, const wchar_t*);
			}capture;
			capture.l = l;
			capture.t = ct;
			capture.f = callback;
			e_button(L"OK", [](void* c) {
				auto& m = *(Capture*)c;
				remove_layer(m.l);
				m.f((char*)c + sizeof(Capture), true, m.t->text());
			}, Mail::expand_original(&capture, _capture));
			e_button(L"Cancel", [](void* c) {
				auto& m = *(Capture*)c;
				remove_layer(m.l);
				m.f((char*)c + sizeof(Capture), false, nullptr);
			}, Mail::expand_original(&capture, _capture));
			f_free(_capture.p);
			e_end_layout();
			e_end_dialog();
			return e;
		}

		inline Entity* e_reflector_window(sEventDispatcher* event_dispatcher)
		{
			push_parent(current_root());
			auto e = e_begin_window(L"Reflector", false);
			struct Capture
			{
				sEventDispatcher* event_dispatcher;
				cText* txt_mouse;
				bool recording;
				Entity* e_window;
				cText* txt_record;
				cText* txt_hovering;
				cText* txt_focusing;
				cText* txt_drag_overing;
			}capture;
			capture.event_dispatcher = event_dispatcher;
			capture.txt_mouse = e_text(nullptr)->get_component(cText);
			capture.recording = true;
			capture.e_window = e;
			capture.txt_record = e_text(L"Recording (Esc)")->get_component(cText);
			capture.txt_hovering = e_text(nullptr)->get_component(cText);
			capture.txt_focusing = e_text(nullptr)->get_component(cText);
			capture.txt_drag_overing = e_text(nullptr)->get_component(cText);

			auto refresh = [](Entity* et) {
				looper().add_event([](void* c, bool*) {
					auto et = *(Entity**)c;
					et->remove_children(0, -1);
					std::function<void(Entity*, Entity*)> add_node = [&](Entity* src, Entity* et) {
						if (src == et)
							return;
						auto cs = src->child_count();
						auto e = src->get_component(cElement);
						auto str = wfmt(L"%I64X (%.2f, %.2f)-(%.2f, %.2f)", (ulonglong)src,
							e->global_pos.x(), e->global_pos.y(),
							e->global_size.x(), e->global_size.y());
						if (cs == 0)
							e_tree_leaf(str.c_str());
						else
						{
							e_begin_tree_node(str.c_str(), true);
							for (auto i = 0; i < cs; i++)
								add_node(src->child(i), et);
							e_end_tree_node();
						}
					};
					push_parent(et);
					add_node(current_root(), et);
					pop_parent();
				}, Mail::from_p(et));
			};

			auto et = Entity::create();

			{
				struct Capture
				{
					decltype(refresh) refresh;
					Entity* et;
				}capture;
				capture.refresh = refresh;
				capture.et = et;
				e_button(Icon_REFRESH, [](void* c) {
					auto& capture = *(Capture*)c;
					capture.refresh(capture.et);
				}, Mail::from_t(&capture));
			}

			next_entity = et;
			e_begin_tree(false, 4.f);
			{
				auto ce = et->get_component(cElement);
				ce->frame_thickness = 2.f;
				ce->frame_color = style_4c(ForegroundColor);
			}
			e_end_tree();

			refresh(et);
			

			e_button(L"Close", [](void* c) {
				auto e = *(void**)c;
				looper().add_event([](void* c, bool*) {
					auto e = *(Entity**)c;
					e->parent()->remove_child(e);
				}, Mail::from_p(e));
			}, Mail::from_p(e));
			e_end_window();
			e->on_destroyed_listeners.add([](void* c) {
				looper().remove_event(*(void**)c);
				return true;
			}, Mail::from_p(looper().add_event([](void* c, bool* go_on) {
				auto& capture = *(Capture*)c;

				if (capture.event_dispatcher->key_states[Key_Esc] == (KeyStateDown | KeyStateJust))
				{
					capture.recording = !capture.recording;
					capture.txt_record->set_text(capture.recording ? L"Recording (Esc)" : L"Start Record (Esc)");
				}

				{
					std::wstring str = L"Mouse: ";
					str += to_wstring(capture.event_dispatcher->mouse_pos);
					capture.txt_mouse->set_text(str.c_str());
				}

				if (capture.recording)
				{
					{
						std::wstring str = L"Hovering: ";
						auto hovering = capture.event_dispatcher->hovering;
						auto e = hovering ? hovering->entity : nullptr;
						if (e->is_child_of(capture.e_window))
							e = nullptr;
						str += wfmt(L"0x%016I64X", (ulonglong)e);
						capture.txt_hovering->set_text(str.c_str());
					}
					{
						auto color = style_4c(TextColorNormal);
						std::wstring str = L"Focusing: ";
						auto focusing = capture.event_dispatcher->focusing;
						auto e = focusing ? focusing->entity : nullptr;
						if (e->is_child_of(capture.e_window))
							e = nullptr;
						str += wfmt(L"0x%016I64X", (ulonglong)e);
						if (e)
						{
							if (focusing == capture.event_dispatcher->hovering)
								color = Vec4c(0, 255, 0, 255);
							switch (capture.event_dispatcher->focusing_state)
							{
							case FocusingAndActive:
								str += L" Active";
								break;
							case FocusingAndDragging:
								str += L" Dragging";
								break;
							}
						}
						capture.txt_focusing->set_color(color);
						capture.txt_focusing->set_text(str.c_str());
					}
					{
						std::wstring str = L"Drag Overing: ";
						auto drag_overing = capture.event_dispatcher->drag_overing;
						auto e = drag_overing ? drag_overing->entity : nullptr;
						if (e->is_child_of(capture.e_window))
							e = nullptr;
						str += wfmt(L"0x%016I64X", (ulonglong)e);
						capture.txt_drag_overing->set_text(str.c_str());
					}
				}
				*go_on = true;
			}, Mail::from_t(&capture))));
			pop_parent();
			return e;
		}
	}
}
