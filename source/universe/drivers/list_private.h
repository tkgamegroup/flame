#pragma once

#include "../entity_private.h"
#include "list.h"

namespace flame
{
	struct dListPrivate : dList
	{
		cReceiverPrivate* receiver;

		EntityPrivate* selected = nullptr;
		bool enable_deselect = true;

		void on_load_finished() override;
		bool on_child_added(EntityPtr e) override;

		EntityPtr get_selected() const override { return selected; }
		void set_selected(EntityPtr e) override;
	};
}
