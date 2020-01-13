#pragma once

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
#include <flame/universe/systems/layout_management.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/ui/layer.h>
#include <flame/universe/ui/style_stack.h>

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

		inline cLayout* c_layout(LayoutType t = LayoutFree)
		{
			auto c = cLayout::create(t);
			current_entity()->add_component(c);
			return c;
		}

		inline Entity* e_empty()
		{
			auto e = Entity::create();
			current_parent()->add_child(e);
			set_current_entity(e);
			return e;
		}

		inline Entity* e_element()
		{
			auto e = e_empty();
			c_element();
			return e;
		}

		inline Entity* e_begin_layout(float x = 0.f, float y = 0.f, LayoutType t = LayoutFree, float item_padding = 0.f)
		{
			auto e = e_empty();
			c_element()->pos_ = Vec2f(x, y);
			c_layout(t)->item_padding = item_padding;
			push_parent(e);
			return e;
		}

		inline void e_end_layout()
		{
			pop_parent();
		}

		inline Entity* e_text(const wchar_t* t)
		{
			auto e = e_empty();
			c_element();
			c_text()->set_text(t);
			return e;
		}

		inline Entity* e_button(const wchar_t* t, void(*f)(void* c, Entity* e), const Mail<>& m)
		{
			auto e = e_empty();
			c_element()->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
			c_text()->set_text(t);
			struct WrapedMail
			{
				void(*f)(void*, Entity*);
				Mail<> m;
				Entity* e;

				~WrapedMail()
				{
					delete_mail(m);
				}
			};
			auto new_m = new_mail<WrapedMail>();
			new_m.p->f = f;
			new_m.p->m = m;
			new_m.p->e = e;
			c_event_receiver()->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				if (is_mouse_clicked(action, key))
				{
					auto& m = *(WrapedMail*)c;
					m.f(m.m.p, m.e);
				}
			}, new_m);
			auto cs = c_style_color();
			cs->color_normal = ui::style(ui::ButtonColorNormal).c();
			cs->color_hovering = ui::style(ui::ButtonColorHovering).c();
			cs->color_active = ui::style(ui::ButtonColorActive).c();
			cs->style();
			return e;
		}

		inline Entity* e_checkbox(const wchar_t* t)
		{
			e_begin_layout(0.f, 0.f, LayoutHorizontal, 4.f);
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
			c_checkbox();
			e_text(t);
			e_end_layout();
			return e;
		}

		inline Entity* e_toggle(const wchar_t* t)
		{
			auto e = e_empty();
			auto ce = c_element();
			auto r = ui::style(ui::FontSize).u()[0] * 0.5f;
			ce->roundness_ = r;
			ce->inner_padding_ = Vec4f(r, 2.f, r, 2.f);
			c_text()->set_text(t);
			c_event_receiver();
			auto cs = c_style_color2();
			cs->color_normal[0] = Vec4c(color(Vec3f(52.f, 0.23f, 0.97f)), 0.40f * 255.f);
			cs->color_hovering[0] = Vec4c(color(Vec3f(52.f, 0.23f, 0.97f)), 1.00f * 255.f);
			cs->color_active[0] = Vec4c(color(Vec3f(49.f, 0.43f, 0.97f)), 1.00f * 255.f);
			cs->color_normal[1] = ui::style(ui::ButtonColorNormal).c();
			cs->color_hovering[1] = ui::style(ui::ButtonColorHovering).c();
			cs->color_active[1] = ui::style(ui::ButtonColorActive).c();
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

		inline Entity* e_edit(float width)
		{
			auto e = e_empty();
			auto ce = c_element();
			ce->size_.x() = width + 8.f;
			ce->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
			ce->color_ = ui::style(ui::FrameColorNormal).c();
			ce->frame_color_ = ui::style(ui::TextColorNormal).c();
			ce->frame_thickness_ = 2.f;
			c_text()->auto_width_ = false;
			c_event_receiver();
			if (width == 0.f)
				c_aligner(SizeFitParent, SizeFixed);
			c_edit();
			return e;
		}

		inline Entity* e_begin_list()
		{
			auto e = e_empty();
			c_element();
			return e;
		}

		inline void e_end_list()
		{

		}
	}
}
