#pragma once

#include "../entity_private.h"
#include <flame/universe/drivers/grid.h>

namespace flame
{
	struct cElementPrivate;
	struct cReceiverPrivate;

	struct dGridPrivate : dGrid
	{
		cElementPrivate* element;
		cReceiverPrivate* receiver;
		EntityPrivate* anchor;
		cElementPrivate* anchor_element;

		void on_load_finished() override;
		bool on_child_added(Entity* e) override;
	};
}
