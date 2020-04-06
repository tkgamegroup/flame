#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/list.h>
#include <flame/universe/utils/style.h>

namespace flame
{
	struct cListItemPrivate : cListItem
	{
		void* mouse_listener;

		cListItemPrivate()
		{
			event_receiver = nullptr;
			background_style = nullptr;
			text_style = nullptr;
			list = nullptr;

			mouse_listener = nullptr;
		}

		~cListItemPrivate()
		{
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void do_style(bool selected)
		{
			if (background_style)
			{
				background_style->level = selected ? 1 : 0;
				background_style->style();
			}
			if (text_style)
			{
				text_style->level = selected ? 1 : 0;
				text_style->style();
			}
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && (key == Mouse_Left || key == Mouse_Right))
					{
						auto thiz = *(cListItemPrivate**)c;
						if (thiz->list)
							thiz->list->set_selected(thiz->entity);
					}
					return true;
				}, Mail::from_p(this));
			}
			else if (c->name_hash == FLAME_CHASH("cStyleColor2"))
			{
				background_style = (cStyleColor2*)c;
				background_style->level = 0;
				do_style(false);
			}
			else if (c->name_hash == FLAME_CHASH("cStyleTextColor2"))
			{
				text_style = (cStyleTextColor2*)c;
				text_style->level = 0;
				do_style(false);
			}
		}
	};

	cListItem* cListItem::create()
	{
		return new cListItemPrivate();
	}

	struct cListPrivate : cList
	{
		bool select_air_when_clicked;
		void* mouse_listener;

		cListPrivate(bool _select_air_when_clicked)
		{
			event_receiver = nullptr;

			selected = nullptr;

			select_air_when_clicked = _select_air_when_clicked;
			mouse_listener = nullptr;
		}

		~cListPrivate()
		{
			if (!entity->dying_ && mouse_listener)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				if (select_air_when_clicked)
				{
					mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
						if (is_mouse_down(action, key, true) && key == Mouse_Left)
							(*(cListPrivate**)c)->set_selected(nullptr);
						return true;
					}, Mail::from_p(this));
				}
			}
		}

		void on_child_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cListItem"))
				((cListItem*)c)->list = this;
		}
	};

	void cList::set_selected(Entity* e, void* sender)
	{
		if (selected == e)
			return;
		if (selected)
		{
			auto listitem = (cListItemPrivate*)selected->get_component(cListItem);
			if (listitem)
				listitem->do_style(false);
		}
		if (e)
		{
			auto listitem = (cListItemPrivate*)e->get_component(cListItem);
			if (listitem)
				listitem->do_style(true);
		}
		selected = e;
		data_changed(FLAME_CHASH("selected"), sender);
	}

	cList* cList::create(bool select_air_when_clicked)
	{
		return new cListPrivate(select_air_when_clicked);
	}
}
