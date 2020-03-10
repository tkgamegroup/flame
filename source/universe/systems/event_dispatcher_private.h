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

		float dbclick_timer;

		Vec2i active_pos;

		std::vector<cEventReceiver*> key_dispatch_list;

		sEventDispatcherPrivate();
		~sEventDispatcherPrivate();
		void remove_receiver(cEventReceiver* er);
		void on_added() override;
		void update(Entity* root) override;
	};

	struct HoversSearcher
	{
		sEventDispatcherPrivate* thiz;
		Vec2f pos;
		EntityPrivate* pass;
		std::vector<cEventReceiver*> mouse_dispatch_list;

		void search(sEventDispatcherPrivate* thiz, const Vec2i& pos, EntityPrivate* root);
		void search_r(EntityPrivate* e);
	};
	static HoversSearcher hovers_searcher;
}
