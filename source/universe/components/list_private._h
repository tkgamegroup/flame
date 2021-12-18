#pragma once

#include "list.h"

namespace flame
{
	struct cListPrivate : cList
	{
		EntityPrivate* selected = nullptr;
		bool enable_deselect = true;

		cReceiverPrivate* receiver;

		EntityPtr get_selected() const override { return selected; }
		void set_selected(EntityPtr v) override;

		void on_load_finished() override;
		bool on_before_adding_child(EntityPtr e);
	};
}
