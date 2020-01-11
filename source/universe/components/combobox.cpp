#include "../universe_private.h"
#include <flame/graphics/font.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/combobox.h>
#include <flame/universe/ui/layer.h>
#include <flame/universe/ui/style_stack.h>

namespace flame
{
	struct cComboboxItemPrivate : cComboboxItem
	{
		void* mouse_listener;

		cComboboxItemPrivate()
		{
			event_receiver = nullptr;
			style = nullptr;
			combobox = nullptr;

			idx = -1;

			mouse_listener = nullptr;
		}

		~cComboboxItemPrivate()
		{
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void do_style(bool selected)
		{
			if (style)
			{
				style->level = selected ? 1 : 0;
				style->style();
			}
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					auto thiz = *(cComboboxItemPrivate**)c;
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						ui::remove_top_layer(thiz->combobox->menu_button->root);

						thiz->combobox->set_index(thiz->idx);
					}
				}, new_mail_p(this));
			}
			else if (c->name_hash == FLAME_CHASH("cStyleColor"))
			{
				style = (cStyleColor2*)c;
				style->level = 0;
				do_style(false);
			}
		}
	};

	cComboboxItem* cComboboxItem::create()
	{
		return new cComboboxItemPrivate();
	}

	struct cComboboxPrivate : cCombobox
	{
		cComboboxPrivate()
		{
			text = nullptr;
			menu_button = nullptr;

			idx = -1;
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cText"))
				text = (cText*)c;
			else if (c->name_hash == FLAME_CHASH("cMenuButton"))
			{
				menu_button = (cMenuButton*)c;
				auto menu = menu_button->menu;
				for (auto i = 0; i < menu->child_count(); i++)
					menu->child(i)->get_component(cComboboxItem)->combobox = this;
			}
		}
	};

	void cCombobox::set_index(int _idx, bool trigger_changed)
	{
		auto menu = menu_button->menu;
		if (idx != -1)
		{
			auto comboboxitem = (cComboboxItemPrivate*)menu->child(idx)->get_component(cComboboxItem);
			if (comboboxitem)
				comboboxitem->do_style(false);
		}
		idx = _idx;
		if (idx < 0)
			text->set_text(L"");
		else
		{
			auto selected = menu->child(idx);
			text->set_text(selected->get_component(cText)->text());
			{
				auto comboboxitem = (cComboboxItemPrivate*)selected->get_component(cComboboxItem);
				if (comboboxitem)
					comboboxitem->do_style(true);
			}
		}
		if (trigger_changed)
			data_changed(FLAME_CHASH("index"), nullptr);
	}

	cCombobox* cCombobox::create()
	{
		return new cComboboxPrivate;
	}

	Entity* create_standard_combobox(float width, graphics::FontAtlas* font_atlas, float font_size_scale, Entity* root, uint item_count, const wchar_t* const* items)
	{
		auto e_menu = create_standard_menu();
		for (auto i = 0; i < item_count; i++)
		{
			auto e_item = Entity::create();
			{
				auto c_element = cElement::create();
				c_element->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
				e_item->add_component(c_element);

				auto c_text = cText::create(font_atlas);
				c_text->font_size_ = ui::style(ui::FontSize).u()[0] * font_size_scale;
				c_text->set_text(items[i]);
				e_item->add_component(c_text);

				e_item->add_component(cEventReceiver::create());

				auto c_style = cStyleColor2::create();
				c_style->color_normal[0] = ui::style(ui::FrameColorNormal).c();
				c_style->color_hovering[0] = ui::style(ui::FrameColorHovering).c();
				c_style->color_active[0] = ui::style(ui::FrameColorActive).c();
				c_style->color_normal[1] = ui::style(ui::SelectedColorNormal).c();
				c_style->color_hovering[1] = ui::style(ui::SelectedColorHovering).c();
				c_style->color_active[1] = ui::style(ui::SelectedColorActive).c();
				e_item->add_component(c_style);

				auto c_aligner = cAligner::create();
				c_aligner->width_policy_ = SizeGreedy;
				e_item->add_component(c_aligner);
			}
			e_menu->add_child(e_item);

			auto c_combobox_item = cComboboxItem::create();
			c_combobox_item->idx = i;
			e_item->add_component(c_combobox_item);
		}

		auto e_combobox = create_standard_menu_button(font_atlas, font_size_scale, L"", root, e_menu, false, SideS, true, false, false, Icon_ANGLE_DOWN);
		{
			auto c_element = e_combobox->get_component(cElement);
			c_element->size_.x() = width + 8.f;
			c_element->size_.y() = ui::style(ui::FontSize).u()[0] + 4.f;
			c_element->frame_color_ = ui::style(ui::TextColorNormal).c();
			c_element->frame_thickness_ = 2.f;

			e_combobox->get_component(cText)->auto_width_ = false;
			
			e_combobox->add_component(cCombobox::create());
		}

		return e_combobox;
	}
}
