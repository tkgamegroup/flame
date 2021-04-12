#pragma once

#include "../entity_private.h"
#include "grid.h"

namespace flame
{
	struct dGridPrivate : dGrid
	{
		cElementPrivate* element;
		cReceiverPrivate* receiver;
		EntityPrivate* anchor;
		cElementPrivate* anchor_element;

		void on_load_finished() override;
		bool on_child_added(EntityPtr e) override;
	};
}
