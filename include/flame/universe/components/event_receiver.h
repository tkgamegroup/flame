#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cEventDispatcher;

	struct cEventReceiver : Component
	{
		cElement* element;
		cEventDispatcher* event_dispatcher;

		bool hovering;
		bool dragging;
		bool focusing;

		cEventReceiver() :
			Component("EventReceiver")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cEventReceiver() override;

		FLAME_UNIVERSE_EXPORTS virtual void on_add_to_parent() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS void* add_key_listener(void (*listener)(void* c, KeyState action, uint value), const Mail<>& capture);
		FLAME_UNIVERSE_EXPORTS void* add_mouse_listener(void (*listener)(void* c, KeyState action, MouseKey key, const Vec2f& pos), const Mail<>& capture);

		FLAME_UNIVERSE_EXPORTS void remove_key_listener(void* ret_by_add);
		FLAME_UNIVERSE_EXPORTS void remove_mouse_listener(void* ret_by_add);

		FLAME_UNIVERSE_EXPORTS void on_key(KeyState action, uint value);
		FLAME_UNIVERSE_EXPORTS void on_mouse(KeyState action, MouseKey key, const Vec2f& value);
		FLAME_UNIVERSE_EXPORTS void on_drop(cEventReceiver* src);

		FLAME_UNIVERSE_EXPORTS static cEventReceiver* create();
	};
}
