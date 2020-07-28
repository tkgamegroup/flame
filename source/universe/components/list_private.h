#pragma once

#include <flame/universe/components/list.h>

namespace flame
{
	struct cListPrivate : cList // R ~ on_*
	{

		//		cEventReceiver* event_receiver;
		//
		//		Entity* selected;
	};

	struct cListItemPrivate : cListItem // R ~ on_*
	{

		//		cEventReceiver* event_receiver;
		//		cList* list;
//		void* mouse_listener;
	};
}
