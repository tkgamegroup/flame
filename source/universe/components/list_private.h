#pragma once

#include <flame/universe/components/list.h>

namespace flame
{
	struct cEventReceiverPrivate;

	struct cListPrivate : cList // R ~ on_*
	{
		cEventReceiverPrivate* event_receiver; // R ref

		void* mouse_listener = nullptr;

		Entity* selected = nullptr;
		bool select_air_when_clicked = true;

		void on_gain_event_receiver();
		void on_lost_event_receiver();

		void set_selected(Entity* e) override;
	};

	struct cListItemPrivate : cListItem // R ~ on_*
	{
		cListPrivate* list; // R ref place=parent
		cEventReceiverPrivate* event_receiver; // R ref

		void* mouse_listener = nullptr;

		void on_gain_event_receiver();
		void on_lost_event_receiver();
	};
}
