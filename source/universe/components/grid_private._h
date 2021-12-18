#pragma once

#include "grid.h"

namespace flame
{
	struct cGridPrivate : cGrid
	{
		cElementPrivate* element;
		cReceiverPrivate* receiver;
		EntityPrivate* anchor;
		cElementPrivate* anchor_element;

		void on_load_finished() override;
		bool on_before_adding_child(EntityPtr e) override;
	};
}
