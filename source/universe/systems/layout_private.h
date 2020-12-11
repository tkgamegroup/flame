#pragma once

#include <flame/universe/systems/layout.h>

namespace flame
{
	struct Window;

	struct cElementPrivate;

	struct sLayoutPrivate : sLayout
	{
		Window* window = nullptr;

		std::deque<cElementPrivate*> sizing_list;
		std::deque<cElementPrivate*> layout_list;

		void on_added() override;
		void update() override;
	};
}
