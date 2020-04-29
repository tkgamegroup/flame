#pragma once

#include <flame/graphics/font.h>
#include <flame/universe/components/timer.h>
#include <flame/universe/components/data_keeper.h>
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
#include <flame/universe/utils/layer.h>

namespace flame
{
	struct UI
	{
		Entity* current_root;
		Entity* current_entity;
		std::stack<Entity*> parents;
		std::stack<CommonValue> styles[StyleCount];
		std::stack<graphics::FontAtlas*> font_atlases;

		Entity* next_entity;
		uint next_component_id;
		Vec2f next_element_pos;
		Vec2f next_element_size;
		Vec4f next_element_padding;
		Vec4f next_element_roundness;
		uint next_element_roundness_lod;
		float next_element_frame_thickness;
		Vec4c next_element_color;
		Vec4c next_element_frame_color;

		UI()
		{
			current_root = current_entity = nullptr;

			next_entity = nullptr;
			next_component_id = 0;
			next_element_pos = 0.f;
			next_element_padding = 0.f;
			next_element_roundness = 0.f;
			next_element_roundness_lod = 0;
			next_element_frame_thickness = 0.f;
			next_element_color = 0;
			next_element_frame_color = Vec4c(255);
		}

		inline cTimer* c_timer()
		{
			auto c = cTimer::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity->add_component(c);
			return c;
		}

		inline cDataKeeper* c_data_keeper()
		{
			auto c = cDataKeeper::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity->add_component(c);
			return c;
		}

		const CommonValue& style(Style s)
		{
			return styles[s].top();
		}

		void push_style(Style s, const CommonValue& cv)
		{
			styles[s].push(cv);
		}

		void pop_style(Style s)
		{
			styles[s].pop();
		}

		void change_style(Style s, const CommonValue& cv)
		{
			auto& style = styles[s];
			if (style.empty())
				style.push(cv);
			else
				style.top() = cv;
		}

		void style_set_to_light()
		{
			change_style(FontSize, common(Vec1u(16U)));
			change_style(BackgroundColor, common(Vec4c(240, 240, 240, 255)));
			change_style(ForegroundColor, common(Vec4c(0, 0, 0, 255)));
			change_style(TextColorNormal, common(Vec4c(0, 0, 0, 255)));
			change_style(TextColorElse, common(Vec4c(0, 120, 205, 255)));
			change_style(FrameColorNormal, common(Vec4c(220, 220, 220, 255)));
			change_style(FrameColorHovering, common(Vec4c(230, 230, 230, 255)));
			change_style(FrameColorActive, common(Vec4c(225, 225, 225, 255)));
			change_style(ButtonColorNormal, common(Vec4c(86, 119, 252, 255)));
			change_style(ButtonColorHovering, common(Vec4c(78, 108, 239, 255)));
			change_style(ButtonColorActive, common(Vec4c(69, 94, 222, 255)));
			change_style(SelectedColorNormal, common(Vec4c(200, 220, 245, 255)));
			change_style(SelectedColorHovering, common(Vec4c(205, 225, 250, 255)));
			change_style(SelectedColorActive, common(Vec4c(195, 215, 240, 255)));
			change_style(ScrollbarColor, common(Vec4c(245, 245, 245, 255)));
			change_style(ScrollbarThumbColorNormal, common(Vec4c(194, 195, 201, 255)));
			change_style(ScrollbarThumbColorHovering, common(Vec4c(104, 104, 104, 255)));
			change_style(ScrollbarThumbColorActive, common(Vec4c(91, 91, 91, 255)));
			change_style(TabColorNormal, common(Vec4c(0, 0, 0, 0)));
			change_style(TabColorElse, common(Vec4c(28, 151, 234, 255)));
			change_style(TabTextColorNormal, common(Vec4c(0, 0, 0, 255)));
			change_style(TabTextColorElse, common(Vec4c(255, 255, 255, 255)));
			change_style(SelectedTabColorNormal, common(Vec4c(0, 122, 204, 255)));
			change_style(SelectedTabColorElse, common(Vec4c(0, 122, 204, 255)));
			change_style(SelectedTabTextColorNormal, common(Vec4c(255, 255, 255, 255)));
			change_style(SelectedTabTextColorElse, common(Vec4c(255, 255, 255, 255)));
		}

		void style_set_to_dark()
		{
			change_style(FontSize, common(Vec1u(16U)));
			change_style(BackgroundColor, common(Vec4c(40, 40, 40, 255)));
			change_style(ForegroundColor, common(Vec4c(255, 255, 255, 255)));
			change_style(TextColorNormal, common(Vec4c(255, 255, 255, 255)));
			change_style(TextColorElse, common(Vec4c(180, 180, 180, 255)));
			change_style(FrameColorNormal, common(Vec4c(55, 55, 55, 255)));
			change_style(FrameColorHovering, common(Vec4c(65, 65, 65, 255)));
			change_style(FrameColorActive, common(Vec4c(60, 60, 60, 255)));
			change_style(ButtonColorNormal, common(Vec4c(86, 119, 252, 255)));
			change_style(ButtonColorHovering, common(Vec4c(78, 108, 239, 255)));
			change_style(ButtonColorActive, common(Vec4c(69, 94, 222, 255)));
			change_style(SelectedColorNormal, common(Vec4c(100, 110, 125, 255)));
			change_style(SelectedColorHovering, common(Vec4c(105, 115, 130, 255)));
			change_style(SelectedColorActive, common(Vec4c(90, 105, 120, 255)));
			change_style(ScrollbarColor, common(Vec4c(62, 62, 66, 255)));
			change_style(ScrollbarThumbColorNormal, common(Vec4c(104, 104, 104, 255)));
			change_style(ScrollbarThumbColorHovering, common(Vec4c(158, 158, 158, 255)));
			change_style(ScrollbarThumbColorActive, common(Vec4c(239, 235, 239, 255)));
			change_style(TabColorNormal, common(Vec4c(0, 0, 0, 0)));
			change_style(TabColorElse, common(Vec4c(28, 151, 234, 255)));
			change_style(TabTextColorNormal, common(Vec4c(255, 255, 255, 255)));
			change_style(TabTextColorElse, common(Vec4c(255, 255, 255, 255)));
			change_style(SelectedTabColorNormal, common(Vec4c(0, 122, 204, 255)));
			change_style(SelectedTabColorElse, common(Vec4c(0, 122, 204, 255)));
			change_style(SelectedTabTextColorNormal, common(Vec4c(255, 255, 255, 255)));
			change_style(SelectedTabTextColorElse, common(Vec4c(255, 255, 255, 255)));
		}

		void init(World* w)
		{
			auto root = w->root();

			auto sg = new StyleGetter;
			sg->pf = [](Capture& c, Style s) -> const CommonValue& {
				return c.thiz<UI>()->style(s);
			};
			sg->c = Capture().set_thiz(this);
			w->add_object(sg);
			root->on_destroyed_listeners.add([](Capture& c) {
				delete c.thiz<StyleGetter>();
				return true;
			}, Capture().set_thiz(sg));

			style_set_to_dark();

			font_atlases.push((graphics::FontAtlas*)w->find_object(FLAME_CHASH("FontAtlas"), 0));

			current_root = current_entity = root;
		}

		inline cElement* c_element()
		{
			auto c = cElement::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			c->pos = next_element_pos;
			next_element_pos = 0.f;
			c->size = next_element_size;
			next_element_size = 0.f;
			c->padding = next_element_padding;
			next_element_padding = 0.f;
			c->roundness = next_element_roundness;
			next_element_roundness = 0.f;
			c->roundness_lod = next_element_roundness_lod;
			next_element_roundness_lod = 0;
			c->frame_thickness = next_element_frame_thickness;
			next_element_frame_thickness = 0.f;
			c->color = next_element_color;
			next_element_color = 0;
			c->frame_color = next_element_frame_color;
			next_element_frame_color = 255;
			current_entity->add_component(c);
			return c;
		}

		inline cText* c_text(bool auto_size = true)
		{
			auto c = cText::create();
			c->font_atlas = font_atlases.top();
			c->font_size = style(FontSize).u.x();
			c->color = style(TextColorNormal).c;
			c->auto_size = auto_size;
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
			return c;
		}

		inline cAligner* c_aligner(uint x_flags, uint y_flags)
		{
			auto c = cAligner::create();
			if (next_component_id)
			{
				c->id = next_component_id;
				next_component_id = 0;
			}
			c->x_align_flags = (AlignFlag)x_flags;
			c->y_align_flags = (AlignFlag)y_flags;
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			auto ce = c->items->get_component(cElement);
			ce->padding = 1.f;
			ce->frame_thickness = 1.f;
			ce->frame_color = style(ForegroundColor).c;
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			current_entity->add_component(c);
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
			auto p = parents.top();
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
			current_entity = e;
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
			parents.push(e);
			return e;
		}

		inline void e_end_layout()
		{
			parents.pop();
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

		inline Entity* e_button(const wchar_t* text, void(*on_clicked)(Capture& c) = nullptr, const Capture& _capture = Capture(), bool use_style = true)
		{
			auto e = e_empty();
			c_element()->padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			auto cer = c_event_receiver();
			if (on_clicked)
			{
				struct Capturing
				{
					void(*f)(Capture&);
				}capture;
				capture.f = on_clicked;
				cer->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_clicked(action, key))
						c.data<Capturing>().f(c.release<Capturing>());
					return true;
				}, Capture().absorb(&capture, _capture, true));
			}
			if (use_style)
			{
				auto cs = c_style_color();
				cs->color_normal = style(ButtonColorNormal).c;
				cs->color_hovering = style(ButtonColorHovering).c;
				cs->color_active = style(ButtonColorActive).c;
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
			ce->frame_color = style(TextColorNormal).c;
			c_event_receiver();
			auto cs = c_style_color2();
			cs->color_normal[0] = style(FrameColorNormal).c;
			cs->color_hovering[0] = style(FrameColorHovering).c;
			cs->color_active[0] = style(FrameColorActive).c;
			cs->color_normal[1] = style(ButtonColorNormal).c;
			cs->color_hovering[1] = style(ButtonColorHovering).c;
			cs->color_active[1] = style(ButtonColorActive).c;
			cs->style();
			c_checkbox()->set_checked(checked);
			if (text[0])
			{
				e_text(text);
				e_end_layout();
			}
			return e;
		}

		inline Entity* e_toggle(const wchar_t* text, bool toggled = false)
		{
			auto e = e_empty();
			c_element()->padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			c_event_receiver();
			auto cs = c_style_color2();
			cs->color_normal[0] = style(FrameColorNormal).c;
			cs->color_hovering[0] = style(FrameColorHovering).c;
			cs->color_active[0] = style(FrameColorActive).c;
			cs->color_normal[1] = style(ButtonColorNormal).c;
			cs->color_hovering[1] = style(ButtonColorHovering).c;
			cs->color_active[1] = style(ButtonColorActive).c;
			cs->style();
			c_checkbox()->set_checked(toggled);
			return e;
		}

		inline Entity* e_image(uint id)
		{
			auto e = e_empty();
			c_element();
			c_image()->id = id;
			return e;
		}

		inline Entity* e_edit(float width, const wchar_t* text = nullptr, bool enter_to_throw_focus = false, bool trigger_changed_on_lost_focus = false)
		{
			auto e = e_empty();
			next_component_id = FLAME_CHASH("edit");
			c_timer()->interval = 0.5f;
			auto ce = c_element();
			ce->size = Vec2f(8.f + width, style(FontSize).u.x() + 4.f);
			ce->padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			ce->color = style(FrameColorNormal).c;
			ce->frame_color = style(ForegroundColor).c;
			ce->frame_thickness = 2.f;
			ce->clip_flags = ClipSelf;
			auto ct = c_text(false);
			if (text)
				ct->set_text(text);
			c_event_receiver();
			if (width == 0.f)
				c_aligner(AlignMinMax, 0);
			auto cedt = c_edit();
			cedt->enter_to_throw_focus = enter_to_throw_focus;
			cedt->trigger_changed_on_lost_focus = trigger_changed_on_lost_focus;
			return e;
		}

		inline Entity* e_drag_edit()
		{
			auto e = e_begin_layout(LayoutVertical);
			e->get_component(cLayout)->fence = 1;

			auto ee = e_edit(50.f);
			ee->set_visible(false);
			push_style(ButtonColorNormal, common(style(FrameColorNormal).c));
			push_style(ButtonColorHovering, common(style(FrameColorHovering).c));
			push_style(ButtonColorActive, common(style(FrameColorActive).c));
			auto ed = e_button(L"");
			pop_style(ButtonColorNormal);
			pop_style(ButtonColorHovering);
			pop_style(ButtonColorActive);

			e_end_layout();

			struct Capturing
			{
				Entity* ee;
				Entity* ed;
			}capture;
			capture.ee = ee;
			capture.ed = ed;

			ee->get_component(cEventReceiver)->focus_listeners.add([](Capture& c, bool focusing) {
				auto& capture = c.data<Capturing>();
				if (!focusing)
				{
					capture.ee->set_visible(false);
					capture.ed->set_visible(true);
				}
				return true;
			}, Capture().set_data(&capture));

			ed->get_component(cEventReceiver)->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
				auto& capture = c.data<Capturing>();
				if (is_mouse_clicked(action, key) && pos == 0)
				{
					capture.ee->set_visible(true);
					capture.ed->set_visible(false);
					c.current<cEventReceiver>()->dispatcher->next_focusing = capture.ee->get_component(cEventReceiver);
				}
				return true;
			}, Capture().set_data(&capture));

			return e;
		}

		inline Entity* e_begin_scrollbar(ScrollbarType type, bool size_fit_parent)
		{
			auto e = e_empty();
			c_element()->clip_flags = ClipChildren;
			if (size_fit_parent)
				c_aligner(AlignMinMax, AlignMinMax);
			auto cl = c_layout(type == ScrollbarVertical ? LayoutHorizontal : LayoutVertical, false, false);
			cl->item_padding = 4.f;
			cl->fence = 2;
			parents.push(e);
			return e;
		}

		inline void e_end_scrollbar(float step = 1.f)
		{
			auto type = parents.top()->get_component(cLayout)->type == LayoutHorizontal ? ScrollbarVertical : ScrollbarHorizontal;
			{
				e_empty();
				auto ce = c_element();
				ce->size = 10.f;
				ce->color = style(ScrollbarColor).c;
				auto ca = c_aligner(0, 0);
				if (type == ScrollbarVertical)
					ca->y_align_flags = AlignMinMax;
				else
					ca->x_align_flags = AlignMinMax;
				c_event_receiver();
				c_scrollbar();
				parents.push(current_entity);
			}
			{
				e_empty();
				c_element()->size = 10.f;
				c_event_receiver();
				auto cs = c_style_color();
				cs->color_normal = style(ScrollbarThumbColorNormal).c;
				cs->color_hovering = style(ScrollbarThumbColorHovering).c;
				cs->color_active = style(ScrollbarThumbColorActive).c;
				cs->style();
				auto ct = c_scrollbar_thumb(type);
				ct->step = step;
				parents.pop();

				e_empty();
				c_element();
				auto ce = c_event_receiver();
				ce->pass_checkers.add([](Capture& c, cEventReceiver* er, bool* pass) {
					*pass = true;
					return true;
				}, Capture());
				ce->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_scroll(action, key))
						c.thiz<cScrollbarThumb>()->update(-pos.x() * 20.f);
					return true;
				}, Capture().set_thiz(ct));
				c_aligner(AlignMinMax, AlignMinMax);
			}
			parents.pop();
		}

		inline Entity* e_begin_splitter(SplitterType type)
		{
			auto e = e_empty();
			c_element();
			c_aligner(AlignMinMax, AlignMinMax);
			c_layout(type == SplitterHorizontal ? LayoutHorizontal : LayoutVertical, false, false);
			parents.push(e);
			e_empty();
			cSplitter::make(current_entity, type);
			return e;
		}

		inline void e_end_splitter()
		{
			parents.pop();
		}

		inline Entity* e_begin_list(bool size_fit_parent)
		{
			auto e = e_empty();
			c_element();
			c_event_receiver();
			if (size_fit_parent)
				c_aligner(AlignMinMax, AlignMinMax);
			auto cl = c_layout(LayoutVertical);
			if (size_fit_parent)
			{
				cl->width_fit_children = false;
				cl->height_fit_children = false;
			}
			c_list();
			parents.push(e);
			return e;
		}

		inline void e_end_list()
		{
			parents.pop();
		}

		inline Entity* e_list_item(const wchar_t* text, bool align = true)
		{
			auto e = e_empty();
			c_element();
			if (text[0])
				c_text()->set_text(text);
			c_event_receiver();
			auto cs = c_style_color2();
			cs->color_normal[0] = style(FrameColorNormal).c;
			cs->color_hovering[0] = style(FrameColorHovering).c;
			cs->color_active[0] = style(FrameColorActive).c;
			cs->color_normal[1] = style(SelectedColorNormal).c;
			cs->color_hovering[1] = style(SelectedColorHovering).c;
			cs->color_active[1] = style(SelectedColorActive).c;
			cs->style();
			if (align)
				c_aligner(AlignMinMax, 0);
			c_list_item();
			return e;
		}

		inline Entity* e_begin_tree(bool fit_parent)
		{
			auto e = e_empty();
			auto ce = c_element();
			c_event_receiver();
			if (fit_parent)
			{
				ce->clip_flags = ClipChildren;
				c_aligner(AlignMinMax, AlignMinMax);
			}
			auto cl = c_layout(LayoutVertical);
			cl->item_padding = 4.f;
			if (fit_parent)
			{
				cl->width_fit_children = false;
				cl->height_fit_children = false;
			}
			c_tree();
			parents.push(e);
			return e;
		}

		inline void e_end_tree()
		{
			parents.pop();
		}

		inline Entity* e_tree_leaf(const wchar_t* text)
		{
			auto e = e_empty();
			c_element()->padding = Vec4f(style(FontSize).u.x() + 4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			c_event_receiver();
			auto cs = c_style_color2();
			cs->color_normal[0] = Vec4c(0);
			cs->color_hovering[0] = style(FrameColorHovering).c;
			cs->color_active[0] = style(FrameColorActive).c;
			cs->color_normal[1] = style(SelectedColorNormal).c;
			cs->color_hovering[1] = style(SelectedColorHovering).c;
			cs->color_active[1] = style(SelectedColorActive).c;
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
			parents.push(e);
			{
				auto e = e_empty();
				c_element()->padding = Vec4f(style(FontSize).u.x() + 4.f, 2.f, 4.f, 2.f);
				c_text()->set_text(text);
				c_event_receiver();
				auto cs = c_style_color2();
				cs->color_normal[0] = Vec4c(0);
				cs->color_hovering[0] = style(FrameColorHovering).c;
				cs->color_active[0] = style(FrameColorActive).c;
				cs->color_normal[1] = style(SelectedColorNormal).c;
				cs->color_hovering[1] = style(SelectedColorHovering).c;
				cs->color_active[1] = style(SelectedColorActive).c;
				cs->style();
				c_layout();
				c_tree_node_title();
				parents.push(e);
				{
					e_empty();
					c_element()->padding = Vec4f(0.f, 2.f, 4.f, 2.f);
					c_text()->set_text(collapsed ? Icon_CARET_RIGHT : Icon_CARET_DOWN);
					c_event_receiver();
					auto cs = c_style_text_color();
					cs->color_normal = style(TextColorNormal).c;
					cs->color_else = style(TextColorElse).c;
					cs->style();
					c_tree_node_arrow();
				}
				parents.pop();
			}
			auto es = e_empty();
			es->set_visible(!collapsed);
			c_element()->padding = Vec4f(style(FontSize).u.x() * 0.5f, 0.f, 0.f, 0.f);
			c_layout(LayoutVertical)->item_padding = 4.f;
			parents.pop();
			parents.push(es);
			return e;
		}

		inline void e_end_tree_node()
		{
			parents.pop();
		}

		inline Entity* e_begin_combobox()
		{
			auto e = e_empty();
			e->gene = e;
			auto ce = c_element();
			ce->size = Vec2f(0.f, style(FontSize).u.x() + 4.f);
			ce->padding = Vec4f(4.f, 2.f, 4.f + style(FontSize).u.x(), 2.f);
			ce->frame_color = style(TextColorNormal).c;
			ce->frame_thickness = 2.f;
			c_text(false);
			c_event_receiver();
			auto cs = c_style_color();
			cs->color_normal = style(FrameColorNormal).c;
			cs->color_hovering = style(FrameColorHovering).c;
			cs->color_active = style(FrameColorActive).c;
			cs->style();
			c_layout();
			auto cm = c_menu(cMenu::ModeMain);
			cm->root = current_root;
			auto ccb = c_combobox();
			parents.push(e);
			e_empty();
			c_element()->padding = Vec4f(0.f, 2.f, 4.f, 2.f);
			c_text()->set_text(Icon_ANGLE_DOWN);
			c_aligner(AlignMax | AlignAbsolute, 0);
			parents.pop();
			parents.push(cm->items);
			return e;
		}

		inline Entity* e_end_combobox(int idx = -1)
		{
			auto eis = parents.top();
			auto ecb = eis->get_component(cMenuItems)->menu->entity;
			auto max_width = 0U;
			for (auto i = 0; i < eis->child_count(); i++)
			{
				auto ct = eis->child(i)->get_component(cText);
				max_width = max(max_width, ct->font_atlas->text_size(ct->font_size, ct->text.v).x());
			}
			ecb->get_component(cElement)->add_width(8.f + max_width + style(FontSize).u.x());
			if (idx != -1)
				ecb->get_component(cCombobox)->set_index(idx, false);
			parents.pop();
			return ecb;
		}

		inline Entity* e_combobox_item(const wchar_t* text)
		{
			auto e = e_empty();
			c_element()->padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			c_event_receiver();
			auto cs = c_style_color2();
			cs->color_normal[0] = style(FrameColorNormal).c;
			cs->color_hovering[0] = style(FrameColorHovering).c;
			cs->color_active[0] = style(FrameColorActive).c;
			cs->color_normal[1] = style(SelectedColorNormal).c;
			cs->color_hovering[1] = style(SelectedColorHovering).c;
			cs->color_active[1] = style(SelectedColorActive).c;
			cs->style();
			c_aligner(AlignMinMax | AlignGreedy, 0);
			c_combobox_item();
			return e;
		}

		inline Entity* e_begin_menu_bar()
		{
			auto e = e_empty();
			e->gene = e;
			c_element()->color = style(FrameColorNormal).c;
			c_aligner(AlignMinMax, 0);
			c_layout(LayoutHorizontal)->item_padding = 4.f;
			parents.push(e);
			return e;
		}

		inline void e_end_menu_bar()
		{
			parents.pop();
		}

		inline Entity* e_begin_menubar_menu(const wchar_t* text, bool transparent = true)
		{
			auto e = e_empty();
			c_element()->padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			c_event_receiver();
			auto cm = c_menu(cMenu::ModeMenubar);
			cm->root = current_root;
			auto cs = c_style_color();
			cs->color_normal = transparent ? Vec4c(0) : style(FrameColorHovering).c;
			cs->color_hovering = style(FrameColorHovering).c;
			cs->color_active = style(FrameColorActive).c;
			cs->style();
			parents.push(cm->items);
			return e;
		}

		inline void e_end_menubar_menu()
		{
			parents.pop();
		}

		inline Entity* e_begin_button_menu(const wchar_t* text)
		{
			auto e = e_empty();
			e->gene = e;
			c_element()->padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			c_event_receiver();
			auto cm = c_menu(cMenu::ModeMain);
			cm->root = current_root;
			auto cs = c_style_color();
			cs->color_normal = style(ButtonColorNormal).c;
			cs->color_hovering = style(ButtonColorHovering).c;
			cs->color_active = style(ButtonColorActive).c;
			cs->style();
			parents.push(cm->items);
			return e;
		}

		inline void e_end_button_menu()
		{
			parents.pop();
		}

		inline Entity* e_begin_sub_menu(const wchar_t* text)
		{
			auto e = e_empty();
			c_element()->padding = Vec4f(4.f, 2.f, 4.f + style(FontSize).u.x(), 2.f);
			c_text()->set_text(text);
			c_event_receiver();
			auto cm = c_menu(cMenu::ModeSub);
			cm->root = current_root;
			auto cs = c_style_color();
			cs->color_normal = style(FrameColorNormal).c;
			cs->color_hovering = style(FrameColorHovering).c;
			cs->color_active = style(FrameColorActive).c;
			cs->style();
			c_aligner(AlignMinMax | AlignGreedy, 0);
			c_layout();
			parents.push(e);
			e_empty();
			c_element()->padding = Vec4f(0.f, 2.f, 4.f, 2.f);
			c_text()->set_text(Icon_CARET_RIGHT);
			c_aligner(AlignMax | AlignAbsolute, 0);
			parents.pop();
			parents.push(cm->items);
			return e;
		}

		inline void e_end_sub_menu()
		{
			parents.pop();
		}

		inline Entity* e_menu_item(const wchar_t* text, void(*func)(Capture& c), const Capture& _capture, bool close_menu = true)
		{
			auto e = e_empty();
			c_element()->padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(text);
			struct Capturing
			{
				Entity* e;
				bool close;
				void(*f)(Capture&);
			}capture;
			capture.e = e;
			capture.close = close_menu;
			capture.f = func;
			c_event_receiver()->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					auto& capture = c.data<Capturing>();
					if (capture.close)
						remove_layer((Entity*)capture.e->gene);
					capture.f(c.release<Capturing>());
				}
				return true;
			}, Capture().absorb(&capture, _capture, true));
			c_menu_item();
			auto cs = c_style_color();
			cs->color_normal = style(FrameColorNormal).c;
			cs->color_hovering = style(FrameColorHovering).c;
			cs->color_active = style(FrameColorActive).c;
			cs->style();
			c_aligner(AlignMinMax | AlignGreedy, 0);
			return e;
		}

		inline Entity* e_separator()
		{
			auto e = e_empty();
			auto ce = c_element();
			ce->size.y() = style(FontSize).u.x() * 0.5f;
			ce->padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			ce->color = style(FrameColorNormal).c;
			auto ceed = c_extra_element_drawing();
			ceed->draw_flags = ExtraDrawHorizontalLine;
			ceed->thickness = 1.f;
			ceed->color = style(TextColorNormal).c.copy().set_w(128);
			c_aligner(AlignMinMax | AlignGreedy, 0);
			return e;
		}

		inline void e_begin_popup_menu(bool attach_to_parent = true)
		{
			if (attach_to_parent)
				current_entity = parents.top();
			auto cm = c_menu(cMenu::ModeContext);
			cm->root = current_root;
			parents.push(cm->items);
		}

		inline void e_end_popup_menu()
		{
			parents.pop();
		}

		inline void e_color_selector()
		{

		}

		inline Entity* e_bring_to_front()
		{
			auto e = e_empty();
			cBringToFront::make(e);
			return e;
		}

		inline Entity* e_size_dragger()
		{
			auto e = e_empty();
			cSizeDragger::make(e);
			return e;
		}

		inline Entity* e_begin_window(const wchar_t* title, bool close_button = true)
		{
			Entity* el;
			auto e = e_empty();
			auto ce = c_element();
			ce->size += Vec2f(0.f, 4.f + style(FontSize).u.x());
			ce->frame_thickness = 2.f;
			ce->color = style(BackgroundColor).c;
			ce->frame_color = style(ForegroundColor).c;
			c_event_receiver();
			c_layout(LayoutVertical)->fence = -1;
			c_moveable();
			parents.push(e);
			{
				e_empty();
				auto ce = c_element();
				ce->padding = Vec4f(4.f, 2.f, 4.f + style(FontSize).u.x(), 2.f);
				ce->color = style(SelectedTabColorNormal).c;
				c_text()->set_text(title);
				c_aligner(AlignMinMax | AlignGreedy, 0);
				if (close_button)
				{
					c_layout();
					parents.push(current_entity);
					e_button(Icon_TIMES, [](Capture& c) {
						looper().add_event([](Capture& c) {
							auto e = c.thiz<Entity>();
							e->parent()->remove_child(e);
						}, Capture().set_thiz(c.thiz<Entity>()));
					}, Capture().set_thiz(e), false);
					c_aligner(AlignMax | AlignAbsolute, 0);
					parents.pop();
				}

				el = e_empty();
				c_element()->padding = Vec4f(8.f, 4.f, 8.f, 4.f);
				c_layout(LayoutVertical)->item_padding = 4.f;

				e_empty();
				c_element();
				c_event_receiver()->pass_checkers.add([](Capture& c, cEventReceiver* er, bool* pass) {
					*pass = true;
					return true;
				}, Capture());
				c_aligner(AlignMinMax, AlignMinMax);
				c_bring_to_front();
			}
			parents.pop();
			parents.push(el);
			return e;
		}

		inline void e_end_window()
		{
			parents.pop();
		}

		inline Entity* e_begin_docker_floating_container()
		{
			auto e = e_empty();
			cDockerTab::make_floating_container(e, next_element_pos, next_element_size);
			next_element_pos = next_element_size = 0.f;
			parents.push(e);
			return e;
		}

		inline void e_end_docker_floating_container()
		{
			parents.pop();
		}

		inline Entity* e_begin_docker_static_container()
		{
			auto e = e_empty();
			cDockerTab::make_static_container(e);
			parents.push(e);
			return e;
		}

		inline void e_end_docker_static_container()
		{
			parents.pop();
		}

		inline Entity* e_begin_docker_layout(LayoutType type)
		{
			auto e = e_empty();
			cDockerTab::make_layout(e, type);
			parents.push(e);
			return e;
		}

		inline void e_end_docker_layout()
		{
			parents.pop();
		}

		inline Entity* e_begin_docker()
		{
			auto e = e_empty();
			cDockerTab::make_docker(e);
			parents.push(e);
			return e;
		}

		inline void e_end_docker()
		{
			parents.pop();
		}

		inline std::pair<Entity*, Entity*> e_begin_docker_page(const wchar_t* title, void(*on_close)(Capture& c) = nullptr, const Capture& _close_capture = Capture())
		{
			parents.push(parents.top()->child(0));
			auto et = e_empty();
			et->set_name("docker_tab");
			c_element()->padding = Vec4f(4.f, 2.f, style(FontSize).u.x() + 6.f, 2.f);
			c_text()->set_text(title);
			c_event_receiver();
			auto csb = c_style_color2();
			csb->color_normal[0] = style(TabColorNormal).c;
			csb->color_hovering[0] = style(TabColorElse).c;
			csb->color_active[0] = style(TabColorElse).c;
			csb->color_normal[1] = style(SelectedTabColorNormal).c;
			csb->color_hovering[1] = style(SelectedTabColorElse).c;
			csb->color_active[1] = style(SelectedTabColorElse).c;
			csb->style();
			auto cst = c_style_text_color2();
			cst->color_normal[0] = style(TabTextColorNormal).c;
			cst->color_else[0] = style(TabTextColorElse).c;
			cst->color_normal[1] = style(SelectedTabTextColorNormal).c;
			cst->color_else[1] = style(SelectedTabTextColorElse).c;
			cst->style();
			c_list_item();
			c_layout();
			auto cdt = c_docker_tab();
			cdt->root = current_root;
			parents.push(et);
			{
				struct Capturing
				{
					cDockerTab* t;
					void(*f)(Capture&);
				}capture;
				capture.t = cdt;
				capture.f = on_close;
				push_style(TextColorNormal, common(style(TabTextColorElse).c));
				e_button(Icon_TIMES, [](Capture& c) {
					auto& capture = c.data<Capturing>();
					if (capture.f)
						capture.f(c.release<Capturing>());
					looper().add_event([](Capture& c) {
						c.data<cDockerTab*>()->take_away(true);
					}, Capture().set_data(&capture.t));
				}, Capture().absorb(&capture, _close_capture, true), false);
				c_aligner(AlignMax | AlignAbsolute, 0);
				pop_style(TextColorNormal);
			}
			parents.pop();
			parents.pop();
			parents.push(parents.top()->child(1));
			auto ep = e_empty();
			{
				auto ce = c_element();
				ce->color = style(BackgroundColor).c;
				ce->clip_flags = ClipChildren;
				c_aligner(AlignMinMax, AlignMinMax);
			}
			parents.pop();
			parents.push(ep);
			return std::make_pair(et, ep);
		}

		inline void e_end_docker_page()
		{
			parents.pop();
		}

		inline Entity* e_begin_dialog()
		{
			auto l = add_layer(current_root, nullptr, true, Vec4c(0, 0, 0, 127));
			current_entity = l;
			c_layout();
			parents.push(l);
			auto e = e_empty();
			auto ce = c_element();
			ce->padding = Vec4f(8.f);
			ce->frame_thickness = 2.f;
			ce->color = style(BackgroundColor).c;
			ce->frame_color = col3_inv(style(BackgroundColor).c);
			c_aligner(AlignMiddle, AlignMiddle);
			c_layout(LayoutVertical)->item_padding = 4.f;
			parents.pop();
			parents.push(e);
			return e;
		}

		inline void e_end_dialog()
		{
			parents.pop();
		}

		inline Entity* e_message_dialog(const wchar_t* message)
		{
			auto e = e_begin_dialog();
			auto l = e->parent();
			e_text(message);
			e_button(L"OK", [](Capture& c) {
				remove_layer(c.thiz<Entity>());
			}, Capture().set_thiz(l));
			c_aligner(AlignMiddle, 0);
			e_end_dialog();
			return e;
		}

		inline Entity* e_confirm_dialog(const wchar_t* title, void (*callback)(Capture& c, bool yes), const Capture& _capture)
		{
			auto e = e_begin_dialog();
			auto l = e->parent();
			e_text(title);
			e_begin_layout(LayoutHorizontal, 4.f);
			c_aligner(AlignMiddle, 0);
			struct Capturing
			{
				Entity* l;
				void(*f)(Capture&, bool);
			}capture;
			capture.l = l;
			capture.f = callback;
			e_button(L"OK", [](Capture& c) {
				auto& m = c.data<Capturing>();
				remove_layer(m.l);
				m.f(c.release<Capturing>(), true);
			}, Capture().absorb(&capture, _capture));
			e_button(L"Cancel", [](Capture& c) {
				auto& m = c.data<Capturing>();
				remove_layer(m.l);
				m.f(c.release<Capturing>(), false);
			}, Capture().absorb(&capture, _capture));
			f_free(_capture._data);
			e_end_layout();
			e_end_dialog();
			return e;
		}

		inline Entity* e_input_dialog(const wchar_t* title, void (*callback)(Capture& c, bool ok, const wchar_t* text), const Capture& _capture, const wchar_t* default_text = nullptr)
		{
			auto e = e_begin_dialog();
			auto l = e->parent();
			e_text(title);
			auto ct = e_edit(100.f)->get_component(cText);
			if (default_text)
				ct->set_text(default_text);
			e_begin_layout(LayoutHorizontal, 4.f);
			c_aligner(AlignMiddle, 0);
			struct Capturing
			{
				Entity* l;
				cText* t;
				void(*f)(Capture&, bool, const wchar_t*);
			}capture;
			capture.l = l;
			capture.t = ct;
			capture.f = callback;
			e_button(L"OK", [](Capture& c) {
				auto& m = c.data<Capturing>();
				remove_layer(m.l);
				m.f(c.release<Capturing>(), true, m.t->text.v);
			}, Capture().absorb(&capture, _capture));
			e_button(L"Cancel", [](Capture& c) {
				auto& m = c.data<Capturing>();
				remove_layer(m.l);
				m.f(c.release<Capturing>(), false, nullptr);
			}, Capture().absorb(&capture, _capture));
			f_free(_capture._data);
			e_end_layout();
			e_end_dialog();
			return e;
		}
	};
}
