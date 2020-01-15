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
			auto menu = entity->parent();
			if (menu)
			{
				idx = menu->child_count() - 1;
				combobox = menu->get_component(cMenu)->button->entity->get_component(cCombobox);
			}
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
				menu_button = (cMenuButton*)c;
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
}
