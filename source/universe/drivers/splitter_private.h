#pragma once

#include "../entity_private.h"
#include <flame/universe/drivers/splitter.h>

namespace flame
{
	struct cElementPrivate;
	struct cReceiverPrivate;

	struct dSplitterPrivate : dSplitter
	{
		cElementPrivate* element;

		EntityPrivate* bar;
		cElementPrivate* bar_element;
		cReceiverPrivate* bar_receiver;

		SplitterType type = SplitterHorizontal;
		std::vector<cElementPrivate*> targets;

		SplitterType get_type() const override { return type; }
		void set_type(SplitterType type) override;

		void on_load_finished() override;
		bool on_child_added(Entity* e) override;
	};
}
