#pragma once

#include <flame/graphics/font.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/image.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/edit.h>
#include <flame/universe/components/checkbox.h>
#include <flame/universe/components/toggle.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/scrollbar.h>
#include <flame/universe/components/splitter.h>
#include <flame/universe/components/list.h>
#include <flame/universe/components/tree.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/combobox.h>
#include <flame/universe/components/window.h>
#include <flame/universe/ui/layer.h>
#include <flame/universe/ui/style_stack.h>
#include <flame/universe/ui/make_window.h>

namespace flame
{
	namespace ui
	{
		FLAME_UNIVERSE_EXPORTS graphics::FontAtlas* current_font_atlas();
		FLAME_UNIVERSE_EXPORTS void push_font_atlas(graphics::FontAtlas* font_atlas);
		FLAME_UNIVERSE_EXPORTS void pop_font_atlas();
		FLAME_UNIVERSE_EXPORTS Entity* current_entity();
		FLAME_UNIVERSE_EXPORTS void set_current_entity(Entity* e);
		FLAME_UNIVERSE_EXPORTS Entity* current_parent();
		FLAME_UNIVERSE_EXPORTS void push_parent(Entity* parent);
		FLAME_UNIVERSE_EXPORTS void pop_parent();
		FLAME_UNIVERSE_EXPORTS Entity* current_root();
		FLAME_UNIVERSE_EXPORTS void set_current_root(Entity* e);

		inline cElement* c_element()
		{
			auto c = cElement::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cText* c_text()
		{
			auto c = cText::create(current_font_atlas());
			current_entity()->add_component(c);
			return c;
		}

		inline cImage* c_image()
		{
			auto c = cImage::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cEventReceiver* c_event_receiver()
		{
			auto c = cEventReceiver::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cStyleColor* c_style_color()
		{
			auto c = cStyleColor::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cStyleColor2* c_style_color2()
		{
			auto c = cStyleColor2::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cStyleTextColor* c_style_text_color()
		{
			auto c = cStyleTextColor::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cStyleTextColor2* c_style_text_color2()
		{
			auto c = cStyleTextColor2::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cEdit* c_edit()
		{
			auto c = cEdit::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cCheckbox* c_checkbox()
		{
			auto c = cCheckbox::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cToggle* c_toggle()
		{
			auto c = cToggle::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cAligner* c_aligner(Alignx x, Aligny y)
		{
			auto c = cAligner::create();
			c->x_align_ = x;
			c->y_align_ = y;
			current_entity()->add_component(c);
			return c;
		}

		inline cAligner* c_aligner(SizePolicy width_policy, SizePolicy height_policy)
		{
			auto c = cAligner::create();
			c->width_policy_ = width_policy;
			c->height_policy_ = height_policy;
			current_entity()->add_component(c);
			return c;
		}

		inline cLayout* c_layout(LayoutType type = LayoutFree)
		{
			auto c = cLayout::create(type);
			current_entity()->add_component(c);
			return c;
		}

		inline cScrollbar* c_scrollbar()
		{
			auto c = cScrollbar::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cScrollbarThumb* c_scrollbar_thumb(ScrollbarType type)
		{
			auto c = cScrollbarThumb::create(type);
			current_entity()->add_component(c);
			return c;
		}

		inline cList* c_list()
		{
			auto c = cList::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cListItem* c_list_item()
		{
			auto c = cListItem::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cTree* c_tree()
		{
			auto c = cTree::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cTreeNode* c_tree_node()
		{
			auto c = cTreeNode::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cTreeNodeTitle* c_tree_node_title()
		{
			auto c = cTreeNodeTitle::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cTreeNodeArrow* c_tree_node_arrow()
		{
			auto c = cTreeNodeArrow::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cTreeLeaf* c_tree_leaf()
		{
			auto c = cTreeLeaf::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cMenu* c_menu(cMenu::Mode mode)
		{
			auto c = cMenu::create(mode);
			current_entity()->add_component(c);
			return c;
		}

		inline cCombobox* c_combobox()
		{
			auto c = cCombobox::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cComboboxItem* c_combobox_item()
		{
			auto c = cComboboxItem::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cMoveable* c_moveable()
		{
			auto c = cMoveable::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cBringToFront* c_bring_to_front()
		{
			auto c = cBringToFront::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cSizeDragger* c_size_dragger()
		{
			auto c = cSizeDragger::create();
			current_entity()->add_component(c);
			return c;
		}

		inline cDockerTab* c_docker_tab()
		{
			auto c = cDockerTab::create();
			current_entity()->add_component(c);
			return c;
		}

		inline Entity* e_empty(int pos = -1)
		{
			auto e = Entity::create();
			auto p = current_parent();
			if (p)
				p->add_child(e, pos);
			set_current_entity(e);
			return e;
		}

		inline Entity* e_element()
		{
			auto e = e_empty();
			c_element();
			return e;
		}

		inline Entity* e_begin_layout(const Vec2f pos = Vec2f(0.f), LayoutType type = LayoutFree, float item_padding = 0.f)
		{
			auto e = e_empty();
			c_element()->pos_ = pos;
			c_layout(type)->item_padding = item_padding;
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
			c_text()->set_text(text);
			return e;
		}

		inline Entity* e_button(const wchar_t* text, void(*func)(void* c, Entity* e) = nullptr, const Mail<>& mail = Mail<>(), bool use_style = true)
		{
			auto e = e_empty();
			c_element()->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			auto cer = c_event_receiver();
			if (func)
			{
				struct WrapedMail
				{
					void(*f)(void*, Entity*);
					Mail<> m;
					Entity* e;

					~WrapedMail()
					{
						delete_mail(m);
						m.p = nullptr;
					}
				};
				auto new_m = new_mail<WrapedMail>();
				new_m.p->f = func;
				new_m.p->m = mail;
				new_m.p->e = e;
				cer->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_clicked(action, key))
					{
						auto& m = *(WrapedMail*)c;
						m.f(m.m.p, m.e);
					}
				}, new_m);
			}
			if (use_style)
			{
				auto cs = c_style_color();
				cs->color_normal = ui::style(ui::ButtonColorNormal).c();
				cs->color_hovering = ui::style(ui::ButtonColorHovering).c();
				cs->color_active = ui::style(ui::ButtonColorActive).c();
				cs->style();
			}
			return e;
		}

		inline Entity* e_checkbox(const wchar_t* text, bool checked = false)
		{
			if (text[0])
				e_begin_layout(Vec2f(0.f), LayoutHorizontal, 4.f);
			auto e = e_empty();
			auto ce = c_element();
			ce->size_ = 16.f;
			ce->frame_thickness_ = 3.f;
			ce->frame_color_ = ui::style(ui::TextColorNormal).c();
			c_event_receiver();
			auto cs = c_style_color2();
			cs->color_normal[0] = ui::style(ui::UncheckedColorNormal).c();
			cs->color_hovering[0] = ui::style(ui::UncheckedColorHovering).c();
			cs->color_active[0] = ui::style(ui::UncheckedColorActive).c();
			cs->color_normal[1] = ui::style(ui::CheckedColorNormal).c();
			cs->color_hovering[1] = ui::style(ui::CheckedColorHovering).c();
			cs->color_active[1] = ui::style(ui::CheckedColorActive).c();
			cs->style();
			c_checkbox()->checked = checked;
			if (text[0])
			{
				e_text(text);
				e_end_layout();
			}
			return e;
		}

		inline Entity* e_toggle(const wchar_t* text)
		{
			auto e = e_empty();
			auto ce = c_element();
			auto r = ui::style(ui::FontSize).u()[0] * 0.5f;
			ce->roundness_ = r;
			ce->inner_padding_ = Vec4f(r, 2.f, r, 2.f);
			c_text()->set_text(text);
			c_event_receiver();
			auto cs = c_style_color2();
			cs->color_normal[0] = Vec4c(color(Vec3f(52.f, 0.23f, 0.97f)), 0.40f * 255.f);
			cs->color_hovering[0] = Vec4c(color(Vec3f(52.f, 0.23f, 0.97f)), 1.00f * 255.f);
			cs->color_active[0] = Vec4c(color(Vec3f(49.f, 0.43f, 0.97f)), 1.00f * 255.f);
			cs->color_normal[1] = ui::style(ui::ButtonColorNormal).c();
			cs->color_hovering[1] = ui::style(ui::ButtonColorHovering).c();
			cs->color_active[1] = ui::style(ui::ButtonColorActive).c();
			cs->style();
			c_toggle();
			return e;
		}

		inline Entity* e_image(uint id, const Vec2f& size, float padding = 0.f, float frame_thickness = 0.f, const Vec4c& frame_color = Vec4c(0))
		{
			auto e = e_empty();
			auto ce = c_element();
			ce->size_ = size + Vec2f(padding) * 2.f;
			ce->inner_padding_ = Vec4f(padding);
			ce->frame_thickness_ = frame_thickness;
			ce->frame_color_ = frame_color;
			c_image()->id = id;
			return e;
		}

		inline Entity* e_edit(float width, const wchar_t* text = nullptr)
		{
			auto e = e_empty();
			auto ce = c_element();
			ce->size_.x() = width + 8.f;
			ce->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
			ce->color_ = ui::style(ui::FrameColorNormal).c();
			ce->frame_color_ = ui::style(ui::TextColorNormal).c();
			ce->frame_thickness_ = 2.f;
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

		inline Entity* e_begin_scroll_view1(ScrollbarType type, const Vec2f size, float padding = 0.f, float frame_thickness = 0.f)
		{
			auto e = e_empty();
			auto ce = c_element();
			ce->size_ = size;
			ce->inner_padding_ = Vec4f(padding);
			ce->frame_thickness_ = frame_thickness;
			ce->clip_children = true;
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
				ce->size_ = 10.f;
				ce->color_ = ui::style(ui::ScrollbarColor).c();
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
				c_element()->size_ = 10.f;
				c_event_receiver();
				auto cs = c_style_color();
				cs->color_normal = ui::style(ui::ScrollbarThumbColorNormal).c();
				cs->color_hovering = ui::style(ui::ScrollbarThumbColorHovering).c();
				cs->color_active = ui::style(ui::ScrollbarThumbColorActive).c();
				cs->style();
				ct = c_scrollbar_thumb(type);
				ct->step = step;
				pop_parent();
			}
			{
				e_empty();
				c_element();
				auto ce = c_event_receiver();
				ce->pass = (Entity*)FLAME_INVALID_POINTER;
				ce->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					auto thumb = (*(cScrollbarThumb**)c);
					if (is_mouse_scroll(action, key))
						thumb->update(-pos.x() * 20.f);
				}, new_mail_p(ct));
				c_aligner(SizeFitParent, SizeFitParent);
			}
			pop_parent();
		}

		inline Entity* e_begin_list(bool size_fit_parent)
		{
			auto e = e_empty();
			c_element();
			c_event_receiver();
			if (size_fit_parent)
				c_aligner(SizeFitParent, SizeFitParent);
			auto cl = c_layout(LayoutVertical);
			cl->item_padding = 4.f;
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
			cs->color_normal[0] = ui::style(ui::FrameColorNormal).c();
			cs->color_hovering[0] = ui::style(ui::FrameColorHovering).c();
			cs->color_active[0] = ui::style(ui::FrameColorActive).c();
			cs->color_normal[1] = ui::style(ui::SelectedColorNormal).c();
			cs->color_hovering[1] = ui::style(ui::SelectedColorHovering).c();
			cs->color_active[1] = ui::style(ui::SelectedColorActive).c();
			cs->style();
			if (align)
				c_aligner(SizeFitParent, SizeFixed);
			c_list_item();
			return e;
		}

		inline Entity* e_begin_tree(bool fit_parent, float padding = 0.f, float frame_thickness = 0.f)
		{
			auto e = e_empty();
			auto ce = c_element();
			ce->inner_padding_ = Vec4f(padding);
			ce->frame_thickness_ = frame_thickness;
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
			c_element()->inner_padding_ = Vec4f(ui::style(ui::FontSize).u()[0] + 4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			c_event_receiver();
			auto cs = c_style_color2();
			cs->color_normal[0] = Vec4c(0);
			cs->color_hovering[0] = ui::style(ui::FrameColorHovering).c();
			cs->color_active[0] = ui::style(ui::FrameColorActive).c();
			cs->color_normal[1] = ui::style(ui::SelectedColorNormal).c();
			cs->color_hovering[1] = ui::style(ui::SelectedColorHovering).c();
			cs->color_active[1] = ui::style(ui::SelectedColorActive).c();
			cs->style();
			c_tree_leaf();
			return e;
		}

		inline Entity* e_begin_tree_node(const wchar_t* text)
		{
			auto e = e_empty();
			c_element();
			c_layout(LayoutVertical)->item_padding = 4.f;
			c_tree_node();
			push_parent(e);
			{
				auto e = e_empty();
				c_element()->inner_padding_ = Vec4f(ui::style(ui::FontSize).u()[0] + 4.f, 2.f, 4.f, 2.f);
				c_text()->set_text(text);
				c_event_receiver();
				auto cs = c_style_color2();
				cs->color_normal[0] = Vec4c(0);
				cs->color_hovering[0] = ui::style(ui::FrameColorHovering).c();
				cs->color_active[0] = ui::style(ui::FrameColorActive).c();
				cs->color_normal[1] = ui::style(ui::SelectedColorNormal).c();
				cs->color_hovering[1] = ui::style(ui::SelectedColorHovering).c();
				cs->color_active[1] = ui::style(ui::SelectedColorActive).c();
				cs->style();
				c_layout();
				c_tree_node_title();
				push_parent(e);
				{
					e_empty();
					c_element()->inner_padding_ = Vec4f(0.f, 2.f, 4.f, 2.f);
					c_text()->set_text(Icon_ANGLE_DOWN);
					c_event_receiver();
					auto cs = c_style_text_color();
					cs->color_normal = ui::style(ui::TextColorNormal).c();
					cs->color_else = ui::style(ui::TextColorElse).c();
					cs->style();
					c_tree_node_arrow();
				}
				pop_parent();
			}
			auto es = e_empty();
			c_element()->inner_padding_ = Vec4f(ui::style(ui::FontSize).u()[0] * 0.5f, 0.f, 0.f, 0.f);
			c_layout(LayoutVertical)->item_padding = 4.f;
			pop_parent();
			push_parent(es);
			return e;
		}

		inline void e_end_tree_node()
		{
			pop_parent();
		}

		inline Entity* e_begin_combobox(float width, int idx = -1)
		{
			auto e = e_empty();
			auto ce = c_element();
			ce->size_ = Vec2f(width + 8.f, ui::style(ui::FontSize).u()[0] + 4.f);
			ce->inner_padding_ = Vec4f(4.f, 2.f, 4.f + ui::style(ui::FontSize).u()[0], 2.f);
			ce->frame_color_ = ui::style(ui::TextColorNormal).c();
			ce->frame_thickness_ = 2.f;
			c_text()->auto_width_ = false;
			c_event_receiver();
			auto cs = c_style_color();
			cs->color_normal = ui::style(ui::FrameColorNormal).c();
			cs->color_hovering = ui::style(ui::FrameColorHovering).c();
			cs->color_active = ui::style(ui::FrameColorActive).c();
			cs->style();
			c_layout();
			auto cm = c_menu(cMenu::ModeMain);
			cm->root = current_root();
			auto ccb = c_combobox();
			if (idx != -1)
				ccb->set_index(idx, false);
			push_parent(e);
			e_empty();
			c_element()->inner_padding_ = Vec4f(0.f, 2.f, 4.f, 2.f);
			c_text()->set_text(Icon_ANGLE_DOWN);
			c_aligner(AlignxRight, AlignyFree);
			pop_parent();
			push_parent(cm->items);
			return e;
		}

		inline void e_end_combobox()
		{
			pop_parent();
		}

		inline Entity* e_combobox_item(const wchar_t* text)
		{
			auto e = e_empty();
			c_element()->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			c_event_receiver();
			auto cs = c_style_color2();
			cs->color_normal[0] = ui::style(ui::FrameColorNormal).c();
			cs->color_hovering[0] = ui::style(ui::FrameColorHovering).c();
			cs->color_active[0] = ui::style(ui::FrameColorActive).c();
			cs->color_normal[1] = ui::style(ui::SelectedColorNormal).c();
			cs->color_hovering[1] = ui::style(ui::SelectedColorHovering).c();
			cs->color_active[1] = ui::style(ui::SelectedColorActive).c();
			cs->style();
			c_aligner(SizeGreedy, SizeFixed);
			c_combobox_item();
			return e;
		}

		inline Entity* e_begin_menu_bar()
		{
			auto e = e_empty();
			c_element()->color_ = ui::style(ui::FrameColorNormal).c();
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
			c_element()->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			c_event_receiver();
			auto cm = c_menu(cMenu::ModeMenubar);
			cm->root = current_root();
			auto cs = c_style_color();
			cs->color_normal = transparent ? Vec4c(0) : ui::style(ui::FrameColorHovering).c();
			cs->color_hovering = ui::style(ui::FrameColorHovering).c();
			cs->color_active = ui::style(ui::FrameColorActive).c();
			cs->style();
			push_parent(cm->items);
			return e;
		}

		inline void e_end_menubar_menu()
		{
			pop_parent();
		}

		inline Entity* e_begin_sub_menu(const wchar_t* text)
		{
			auto e = e_empty();
			c_element()->inner_padding_ = Vec4f(4.f, 2.f, 4.f + ui::style(ui::FontSize).u()[0], 2.f);
			c_text()->set_text(text);
			c_event_receiver();
			auto cm = c_menu(cMenu::ModeSub);
			cm->root = current_root();
			auto cs = c_style_color();
			cs->color_normal = ui::style(ui::FrameColorNormal).c();
			cs->color_hovering = ui::style(ui::FrameColorHovering).c();
			cs->color_active = ui::style(ui::FrameColorActive).c();
			cs->style();
			c_aligner(SizeGreedy, SizeFixed);
			c_layout();
			push_parent(e);
			e_empty();
			c_element()->inner_padding_ = Vec4f(0.f, 2.f, 4.f, 2.f);
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

		inline Entity* e_menu_item(const wchar_t* text, void(*func)(void* c, Entity* e), const Mail<>& mail)
		{
			auto e = e_empty();
			c_element()->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			struct WrapedMail
			{
				Entity* root;
				void(*f)(void*, Entity*);
				Mail<> m;
				Entity* e;

				~WrapedMail()
				{
					delete_mail(m);
					m.p = nullptr;
				}
			};
			auto new_m = new_mail<WrapedMail>();
			new_m.p->root = current_root();
			new_m.p->f = func;
			new_m.p->m = mail;
			new_m.p->e = e;
			c_event_receiver()->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					auto& m = *(WrapedMail*)c;
					remove_top_layer(m.root);
					m.f(m.m.p, m.e);
				}
			}, new_m);
			auto cs = c_style_color();
			cs->color_normal = ui::style(ui::FrameColorNormal).c();
			cs->color_hovering = ui::style(ui::FrameColorHovering).c();
			cs->color_active = ui::style(ui::FrameColorActive).c();
			cs->style();
			c_aligner(SizeGreedy, SizeFixed);
			return e;
		}

		inline void e_begin_popup_menu()
		{
			set_current_entity(current_parent());
			auto cm = c_menu(cMenu::ModeContext);
			cm->root = current_root();
			push_parent(cm->items);
		}

		inline void e_end_popup_menu()
		{
			pop_parent();
		}

		inline Entity* e_begin_docker_container(const Vec2f& pos, const Vec2f& size, bool floating = true)
		{
			auto e = e_empty();
			make_docker_container(e, pos, size, floating);
			push_parent(e);
			return e;
		}

		inline void e_end_docker_container()
		{
			pop_parent();
		}

		inline Entity* e_begin_docker_layout(LayoutType type)
		{
			auto is_parent_container = current_parent()->name_hash() == FLAME_CHASH("docker_container");
			auto pos = -1;
			if (is_parent_container)
				pos = 0;
			else
			{
				if (current_parent()->child_count() == 1)
					pos = 0;
				else
					pos = 2;
			}
			auto e = e_empty(pos);
			make_docker_layout(e, type);
			if (!is_parent_container)
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
			auto is_parent_container = current_parent()->name_hash() == FLAME_CHASH("docker_container");
			auto pos = -1;
			if (is_parent_container)
				pos = 0;
			else
			{
				if (current_parent()->child_count() == 1)
					pos = 0;
				else
					pos = 2;
			}
			auto e = e_empty(pos);
			make_docker(e);
			if (!is_parent_container)
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

		struct sDockerPage
		{
			Entity* tab;
			Entity* page;
		};

		inline sDockerPage e_begin_docker_page(const wchar_t* title)
		{
			push_parent(current_parent()->child(0));
			auto et = e_empty();
			et->set_name("docker_tab");
			c_element()->inner_padding_ = Vec4f(4.f, 2.f, ui::style(ui::FontSize).u()[0] + 6.f, 2.f);
			c_text()->set_text(title);
			c_event_receiver();
			auto csb = c_style_color2();
			csb->color_normal[0] = ui::style(ui::TabColorNormal).c();
			csb->color_hovering[0] = ui::style(ui::TabColorElse).c();
			csb->color_active[0] = ui::style(ui::TabColorElse).c();
			csb->color_normal[1] = ui::style(ui::SelectedTabColorNormal).c();
			csb->color_hovering[1] = ui::style(ui::SelectedTabColorElse).c();
			csb->color_active[1] = ui::style(ui::SelectedTabColorElse).c();
			csb->style();
			auto cst = c_style_text_color2();
			cst->color_normal[0] = ui::style(ui::TabTextColorNormal).c();
			cst->color_else[0] = ui::style(ui::TabTextColorElse).c();
			cst->color_normal[1] = ui::style(ui::SelectedTabTextColorNormal).c();
			cst->color_else[1] = ui::style(ui::SelectedTabTextColorElse).c();
			cst->style();
			c_list_item();
			c_layout();
			auto cdt = c_docker_tab();
			cdt->root = current_root();
			push_parent(et);
			e_button(Icon_WINDOW_CLOSE, [](void* c, Entity*) {
				auto thiz = (*(cDockerTab**)c);
				looper().add_event([](void* c) {
					auto thiz = (*(cDockerTab**)c);
					thiz->take_away(true);
				}, new_mail_p(thiz));
			}, new_mail_p(cdt), false);
			c_aligner(AlignxRight, AlignyFree);
			pop_parent();
			pop_parent();
			push_parent(current_parent()->child(1));
			auto ep = e_empty();
			{
				auto ce = c_element();
				ce->color_ = ui::style(ui::WindowColor).c();
				ce->clip_children = true;
				c_aligner(SizeFitParent, SizeFitParent);
			}
			pop_parent();
			push_parent(ep);
			sDockerPage ret;
			ret.tab = et;
			ret.page = ep;
			return ret;
		}

		inline void e_end_docker_page()
		{
			pop_parent();
		}

		inline Entity* e_begin_dialog()
		{
			auto r = current_root();
			auto l = get_top_layer(r);
			if (!l)
			{
				l = ui::add_layer(r, "dialog", nullptr, true, Vec4c(0, 0, 0, 127));
				set_current_entity(l);
				c_layout();
			}
			push_parent(l);
			auto e = e_empty();
			auto ce = c_element();
			ce->inner_padding_ = Vec4f(8.f);
			ce->color_ = Vec4c(255);
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
			e_text(message);
			e_button(L"OK", [](void* c, Entity*) {
				remove_top_layer(*(Entity**)c);
			}, new_mail_p(current_root()));
			e_end_dialog();
			return e;
		}

		inline Entity* e_confirm_dialog(const wchar_t* title, void (*callback)(void* c, bool yes), const Mail<>& m)
		{
			auto e = e_begin_dialog();
			e_text(title);
			e_begin_layout(Vec2f(0.f), LayoutHorizontal, 4.f);
			struct WrapedMail
			{
				void(*f)(void*, bool);
				Mail<> m;
				Entity* r;

				~WrapedMail()
				{
					delete_mail(m);
					m.p = nullptr;
				}
			};
			auto new_m = new_mail<WrapedMail>();
			new_m.p->f = callback;
			new_m.p->m = m;
			new_m.p->r = current_root();
			e_button(L"OK", [](void* c, Entity*) {
				auto& m = *(WrapedMail*)c;
				remove_top_layer(m.r);
				m.f(m.m.p, true);
			}, new_m);
			e_button(L"Cancel", [](void* c, Entity*) {
				auto& m = *(WrapedMail*)c;
				remove_top_layer(m.r);
				m.f(m.m.p, false);
			}, new_m);
			e_end_layout();
			e_end_dialog();
			return e;
		}

		inline Entity* e_input_dialog(const wchar_t* title, void (*callback)(void* c, bool ok, const wchar_t* text), const Mail<>& m)
		{
			auto e = e_begin_dialog();
			e_begin_layout(Vec2f(0.f), LayoutHorizontal, 4.f);
			e_text(title);
			auto ct = e_edit(100.f)->get_component(cText);
			e_end_layout();
			e_begin_layout(Vec2f(0.f), LayoutHorizontal, 4.f);
			struct WrapedMail
			{
				void(*f)(void*, bool, const wchar_t*);
				Mail<> m;
				Entity* r;
				cText* t;

				~WrapedMail()
				{
					delete_mail(m);
					m.p = nullptr;
				}
			};
			auto new_m = new_mail<WrapedMail>();
			new_m.p->f = callback;
			new_m.p->m = m;
			new_m.p->r = current_root();
			new_m.p->t = ct;
			e_button(L"OK", [](void* c, Entity*) {
				auto& m = *(WrapedMail*)c;
				remove_top_layer(m.r);
				m.f(m.m.p, true, m.t->text());
			}, new_m);
			e_button(L"Cancel", [](void* c, Entity*) {
				auto& m = *(WrapedMail*)c;
				remove_top_layer(m.r);
				m.f(m.m.p, false, nullptr);
			}, new_m);
			e_end_layout();
			e_end_dialog();
			return e;
		}
	}
}
