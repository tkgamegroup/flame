#include <flame/graphics/font.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/combobox.h>
#include <flame/universe/utils/layer.h>

namespace flame
{
	struct cComboboxItemPrivate : cComboboxItem
	{
		void* mouse_listener;

		cComboboxItemPrivate()
		{
			event_receiver = nullptr;
			style = nullptr;

			index = -1;

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

		void on_added() override
		{
			if (index == -1)
			{
				auto items = entity->parent();
				if (items)
					index = items->child_count() - 1;
			}
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						auto thiz = *(cComboboxItemPrivate**)c;
						auto menu = thiz->entity->parent()->get_component(cMenuItems)->menu;
						auto combobox = menu->entity->get_component(cCombobox);
						utils::remove_layer((Entity*)thiz->entity->gene);
						combobox->set_index(thiz->index);
					}
					return true;
				}, Mail::from_p(this));
			}
			else if (c->name_hash == FLAME_CHASH("cStyleColor2"))
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
		void* mouse_listener;

		cComboboxPrivate()
		{
			text = nullptr;
			event_receiver = nullptr;

			index = -1;
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cText"))
				text = (cText*)c;
		}
	};

	void cCombobox::set_index(int _index, void* sender)
	{
		if (index == _index)
			return;
		auto items = entity->get_component(cMenu)->items;
		if (index != -1)
		{
			auto comboboxitem = (cComboboxItemPrivate*)items->child(index)->get_component(cComboboxItem);
			if (comboboxitem)
				comboboxitem->do_style(false);
		}
		index = _index;
		if (index < 0)
			text->set_text(L"");
		else
		{
			auto selected = items->child(index);
			text->set_text(selected->get_component(cText)->text.v);
			{
				auto comboboxitem = (cComboboxItemPrivate*)selected->get_component(cComboboxItem);
				if (comboboxitem)
					comboboxitem->do_style(true);
			}
		}
		data_changed(FLAME_CHASH("index"), sender);
	}

	cCombobox* cCombobox::create()
	{
		return new cComboboxPrivate;
	}
}
