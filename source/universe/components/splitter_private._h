#pragma once

#include "splitter.h"

namespace flame
{
	struct cSplitterPrivate : cSplitter
	{
		SplitterType type = SplitterHorizontal;

		cElementPrivate* element;

		EntityPrivate* bar;
		cElementPrivate* bar_element;
		cReceiverPrivate* bar_receiver;

		std::vector<cElementPrivate*> targets;

		SplitterType get_type() const override { return type; }
		void set_type(SplitterType v) override;

		void on_load_finished() override;
		bool on_before_adding_child(EntityPtr e);
	};
}
