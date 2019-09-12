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

			mouse_listener = nullptr;
		}

		~cComboboxItemPrivate()
		{
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			if (event_receiver)
				event_receiver->remove_mouse_listener(mouse_listener);
		}

		void start()
		{
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
			style = (cStyleBackgroundColor*)(entity->find_component(cH("StyleBackgroundColor")));

			if (style)
			{
				unselected_color_normal = style->color_normal;
				unselected_color_hovering = style->color_hovering;
				unselected_color_active = style->color_active;
			}

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					auto thiz = *(cComboboxItemPrivate**)c;
					thiz->combobox->set_index(thiz->idx);
					destroy_topmost(thiz->combobox->menu_button->root);
				}
			}, new_mail_p(this));
		}

		void update()
		{
			if (style)
			{
				if (combobox->selected == entity)
				{
					style->color_normal = selected_color_normal;
					style->color_hovering = selected_color_hovering;
					style->color_active = selected_color_active;
				}
				else
				{
					style->color_normal = unselected_color_normal;
					style->color_hovering = unselected_color_hovering;
					style->color_active = unselected_color_active;
				}
			}
		}
	};

	void cComboboxItem::start()
	{
		((cComboboxItemPrivate*)this)->start();
	}

	void cComboboxItem::update()
	{
		((cComboboxItemPrivate*)this)->update();
	}

	cComboboxItem* cComboboxItem::create()
	{
		return new cComboboxItemPrivate();
	}

	struct cComboboxPrivate : cCombobox
	{
		std::vector<std::unique_ptr<Closure<void(void* c, uint idx)>>> changed_listeners;

		void on_changed(int idx)
		{
			for (auto& l : changed_listeners)
				l->function(l->capture.p, idx);
		}

		void start()
		{
			text = (cText*)(entity->find_component(cH("Text")));
			assert(text && !text->auto_size);
			menu_button = (cMenuButton*)(entity->find_component(cH("MenuButton")));
			assert(menu_button);

			auto menu = menu_button->menu;
			for (auto i = 0; i < menu->child_count(); i++)
				((cComboboxItem*)(menu->child(i)->find_component(cH("ComboboxItem"))))->combobox = this;
		}
	};

	void* cCombobox::add_changed_listener(void (*listener)(void* c, uint idx), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c, uint idx)>;
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

	void cCombobox::set_index(int idx)
	{
		if (idx < 0)
		{
			selected = nullptr;
			text->set_text(L"");
			((cComboboxPrivate*)this)->on_changed(-1);
			return;
		}

		auto menu = menu_button->menu;
		selected = menu->child(idx);
		text->set_text(((cText*)(selected->find_component(cH("Text"))))->text());
		((cComboboxPrivate*)this)->on_changed(idx);
	}

	void cCombobox::start()
	{
		((cComboboxPrivate*)this)->start();
	}

	void cCombobox::update()
	{
	}

	cCombobox* cCombobox::create()
	{
		return new cComboboxPrivate;
	}

	Entity* create_standard_combobox(float width, graphics::FontAtlas* font_atlas, float sdf_scale, Entity* root, const std::vector<std::wstring>& items, int init_item)
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
			c_element->width = width + 8.f;
			c_element->height = font_atlas->pixel_height * sdf_scale + 4.f;
			c_element->background_frame_color = default_style.text_color_normal;
			c_element->background_frame_thickness = 2.f;

			auto c_text = (cText*)e_combobox->find_component(cH("Text"));
			c_text->auto_size = false;
			if (init_item >= 0)
				c_text->set_text(items[init_item]);
			
			auto c_combobox = cCombobox::create();
			if (init_item >= 0)
				c_combobox->selected = e_menu->child(init_item);
			e_combobox->add_component(c_combobox);
		}

		return e_combobox;
	}
}
