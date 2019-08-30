#include <flame/universe/default_style.h>
#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/combobox.h>

namespace flame
{
	struct cComboboxItemPrivate : cComboboxItem
	{
		void* mouse_listener;

		cComboboxItemPrivate()
		{
			text = nullptr;
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
			event_receiver->remove_mouse_listener(mouse_listener);
		}

		void start()
		{
			text = (cText*)(entity->find_component(cH("Text")));
			assert(text);
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
					auto combobox = thiz->combobox;
					combobox->selected = thiz->entity;
					combobox->text->set_text(thiz->text->text());
					destroy_topmost();
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

	cComboboxItem::~cComboboxItem()
	{
		((cComboboxItemPrivate*)this)->~cComboboxItemPrivate();
	}

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
		cComboboxPrivate()
		{
		}

		~cComboboxPrivate()
		{
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

	cCombobox::~cCombobox()
	{
		((cComboboxPrivate*)this)->~cComboboxPrivate();
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
}
