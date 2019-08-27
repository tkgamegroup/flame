#include <flame/universe/default_style.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/list.h>

namespace flame
{
	struct cListItemPrivate : cListItem
	{
		void* mouse_listener;

		cListItemPrivate()
		{
			event_receiver = nullptr;
			style = nullptr;
			list = nullptr;

			unselected_color_normal = default_style.frame_color_normal;
			unselected_color_hovering = default_style.frame_color_hovering;
			unselected_color_active = default_style.frame_color_active;
			selected_color_normal = default_style.header_color_normal;
			selected_color_hovering = default_style.header_color_hovering;
			selected_color_active = default_style.header_color_active;

			mouse_listener = nullptr;
		}

		~cListItemPrivate()
		{
			event_receiver->remove_mouse_listener(mouse_listener);
		}

		void start()
		{
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
			style = (cStyleBackgroundColor*)(entity->find_component(cH("StyleBackgroundColor")));
			list = (cList*)(entity->parent()->find_component(cH("List")));
			assert(list);

			if (style)
			{
				unselected_color_normal = style->color_normal;
				unselected_color_hovering = style->color_hovering;
				unselected_color_active = style->color_active;
			}

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					auto thiz = *(cListItemPrivate**)c;
					thiz->list->select = thiz->entity;
				}
			}, new_mail_p(this));
		}

		void update()
		{
			if (style)
			{
				if (list->select == entity)
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

	cListItem::~cListItem()
	{
		((cListItemPrivate*)this)->~cListItemPrivate();
	}

	void cListItem::start()
	{
		((cListItemPrivate*)this)->start();
	}

	void cListItem::update()
	{
		((cListItemPrivate*)this)->update();
	}

	cListItem* cListItem::create()
	{
		return new cListItemPrivate();
	}

	struct cListPrivate : cList
	{
	};

	cList::~cList()
	{
		((cListPrivate*)this)->~cListPrivate();
	}

	void cList::update()
	{
	}

	cList* cList::create()
	{
		return new cListPrivate();
	}
}
