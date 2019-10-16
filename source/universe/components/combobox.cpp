#include <flame/graphics/font.h>
#include <flame/universe/default_style.h>
#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/combobox.h>

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

			unselected_color_normal = default_style.frame_color_normal;
			unselected_color_hovering = default_style.frame_color_hovering;
			unselected_color_active = default_style.frame_color_active;
			selected_color_normal = default_style.selected_color_normal;
			selected_color_hovering = default_style.selected_color_hovering;
			selected_color_active = default_style.selected_color_active;

			idx = -1;

			mouse_listener = nullptr;
		}

		~cComboboxItemPrivate()
		{
			if (!entity->dying)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void do_style(bool selected)
		{
			if (!selected)
			{
				if (style)
				{
					style->color_normal = unselected_color_normal;
					style->color_hovering = unselected_color_hovering;
					style->color_active = unselected_color_active;
					style->style();
				}
			}
			else
			{
				if (style)
				{
					style->color_normal = selected_color_normal;
					style->color_hovering = selected_color_hovering;
					style->color_active = selected_color_active;
					style->style();
				}
			}
		}
	};

	void cComboboxItem::on_enter_hierarchy(Component* c)
	{
		if (c)
		{
			const auto add_listener = [](cComboboxItemPrivate* thiz) {
				thiz->mouse_listener = thiz->event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						auto thiz = *(cComboboxItemPrivate**)c;
						thiz->combobox->set_index(thiz->idx);
						destroy_topmost(thiz->combobox->menu_button->root);
					}
				}, new_mail_p(thiz));
			};
			if (c == this)
			{
				event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
				if (event_receiver)
					add_listener((cComboboxItemPrivate*)this);
				style = (cStyleColor*)(entity->find_component(cH("StyleColor")));
				((cComboboxItemPrivate*)this)->do_style(false);
			}
			else if (c->type_hash == cH("EventReceiver"))
			{
				event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
				add_listener((cComboboxItemPrivate*)this);
			}
			else if (c->type_hash == cH("StyleColor"))
			{
				style = (cStyleColor*)(entity->find_component(cH("StyleColor")));
				((cComboboxItemPrivate*)this)->do_style(false);
			}
		}
	}

	cComboboxItem* cComboboxItem::create()
	{
		return new cComboboxItemPrivate();
	}

	struct cComboboxPrivate : cCombobox
	{
		std::vector<std::unique_ptr<Closure<void(void* c, int idx)>>> changed_listeners;

		cComboboxPrivate()
		{
			text = nullptr;
			menu_button = nullptr;

			selected = nullptr;
		}

		void on_changed(int idx)
		{
			for (auto& l : changed_listeners)
				l->function(l->capture.p, idx);
		}
	};

	void* cCombobox::add_changed_listener(void (*listener)(void* c, int idx), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c, int idx)>;
		c->function = listener;
		c->capture = capture;
		((cComboboxPrivate*)this)->changed_listeners.emplace_back(c);
		return c;
	}

	void cCombobox::remove_changed_listener(void* ret_by_add)
	{
		auto& listeners = ((cComboboxPrivate*)this)->changed_listeners;
		for (auto it = listeners.begin(); it != listeners.end(); it++)
		{
			if (it->get() == ret_by_add)
			{
				listeners.erase(it);
				return;
			}
		}
	}

	void cCombobox::set_index(int idx, bool trigger_changed)
	{
		if (selected)
		{
			auto comboboxitem = (cComboboxItemPrivate*)selected->find_component(cH("ComboboxItem"));
			if (comboboxitem)
				comboboxitem->do_style(false);
		}
		if (idx < 0)
		{
			selected = nullptr;
			text->set_text(L"");
		}
		else
		{
			selected = menu_button->menu->child(idx);
			text->set_text(((cText*)(selected->find_component(cH("Text"))))->text());
			{
				auto comboboxitem = (cComboboxItemPrivate*)selected->find_component(cH("ComboboxItem"));
				if (comboboxitem)
					comboboxitem->do_style(true);
			}
		}
		if (trigger_changed)
			((cComboboxPrivate*)this)->on_changed(idx);
	}

	void cCombobox::on_enter_hierarchy(Component* c)
	{
		if (c)
		{
			const auto set_ptr = [](cComboboxPrivate* thiz) {
				auto menu = thiz->menu_button->menu;
				for (auto i = 0; i < menu->child_count(); i++)
					((cComboboxItem*)(menu->child(i)->find_component(cH("ComboboxItem"))))->combobox = thiz;
			};
			if (c == this)
			{
				text = (cText*)(entity->find_component(cH("Text")));
				menu_button = (cMenuButton*)(entity->find_component(cH("MenuButton")));
				if (menu_button)
					set_ptr((cComboboxPrivate*)this);
			}
			else if (c->type_hash == cH("Text"))
				text = (cText*)(entity->find_component(cH("Text")));
			else if (c->type_hash == cH("MenuButton"))
			{
				menu_button = (cMenuButton*)(entity->find_component(cH("MenuButton")));
				set_ptr((cComboboxPrivate*)this);
			}
		}
	}

	void cCombobox::update()
	{
	}

	cCombobox* cCombobox::create()
	{
		return new cComboboxPrivate;
	}

	Entity* create_standard_combobox(float width, graphics::FontAtlas* font_atlas, float sdf_scale, Entity* root, const std::vector<std::wstring>& items)
	{
		auto e_menu = create_standard_menu();
		for (auto i = 0; i < items.size(); i++)
		{
			auto e_item = create_standard_menu_item(font_atlas, sdf_scale, items[i]);
			e_menu->add_child(e_item);

			auto c_combobox_item = cComboboxItem::create();
			c_combobox_item->idx = i;
			e_item->add_component(c_combobox_item);
		}

		auto e_combobox = create_standard_menu_button(font_atlas, sdf_scale, L"", root, e_menu, false, SideS, true, false, false, Icon_ANGLE_DOWN);
		{
			auto c_element = (cElement*)e_combobox->find_component(cH("Element"));
			c_element->size.x() = width + 8.f;
			c_element->size.y() = font_atlas->pixel_height * sdf_scale + 4.f;
			c_element->frame_color = default_style.text_color_normal;
			c_element->frame_thickness = 2.f;

			auto c_text = (cText*)e_combobox->find_component(cH("Text"));
			c_text->auto_width = false;
			
			e_combobox->add_component(cCombobox::create());
		}

		return e_combobox;
	}
}
