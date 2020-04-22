#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/checkbox.h>
#include <flame/universe/components/layout.h>

namespace flame
{
	struct cCheckboxPrivate : cCheckbox
	{
		void* mouse_listener;

		cCheckboxPrivate()
		{
			event_receiver = nullptr;
			style = nullptr;

			checked = false;

			mouse_listener = nullptr;
		}

		~cCheckboxPrivate()
		{
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cElement"))
				element = (cElement*)c;
			else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_clicked(action, key))
					{
						auto thiz = *(cCheckboxPrivate**)c;
						thiz->set_checked(!thiz->checked);
					}
					return true;
				}, Mail::from_p(this));
			}
			else if (c->name_hash == FLAME_CHASH("cStyleColor2"))
			{
				style = (cStyleColor2*)c;
				style->level = checked ? 1 : 0;
				do_style();
			}
		}

		void do_style()
		{
			if (style)
			{
				style->level = checked ? 1 : 0;
				style->style();
			}
		}
	};

	void cCheckbox::set_checked(bool _checked, void* sender)
	{
		auto thiz = (cCheckboxPrivate*)this;
		checked = _checked;
		thiz->do_style();
		data_changed(FLAME_CHASH("checked"), sender);
	}

	cCheckbox* cCheckbox::create()
	{
		return new cCheckboxPrivate();
	}
}
