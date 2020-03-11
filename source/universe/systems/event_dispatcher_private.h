#pragma once

#include <flame/universe/systems/event_dispatcher.h>
#include "../entity_private.h"

namespace flame
{
	struct sEventDispatcherPrivate : sEventDispatcher
	{
		SysWindow* window;
		void* key_listener;
		void* mouse_listener;

		std::vector<Key> keydown_inputs;
		std::vector<Key> keyup_inputs;
		std::vector<wchar_t> char_inputs;
		bool char_input_compelete;

		bool pass;
		float dbclick_timer;
		Vec2i active_pos;

		sEventDispatcherPrivate();
		~sEventDispatcherPrivate();
		void on_receiver_removed(cEventReceiver* er);
		void on_added() override;
		void dispatch_mouse(EntityPrivate* e);
		void update(Entity* root) override;
	};
}
