#pragma once

#include <flame/universe/components/list.h>

namespace flame
{
	struct EntityPrivate;
	struct cEventReceiverPrivate;

	struct cListPrivate : cList // R ~ on_*
	{
		cEventReceiverPrivate* event_receiver; // R ref

		void* mouse_listener = nullptr;

		EntityPrivate* selected = nullptr;
		bool select_air_when_clicked = true;

		void on_gain_event_receiver();
		void on_lost_event_receiver();

		void cListPrivate::set_selected(Entity* e) override;
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
