#pragma once

#include "scene.h"

namespace flame
{
	struct sScenePrivate : sScene
	{
		Window* window = nullptr;

		std::deque<std::pair<uint, std::deque<cElementPrivate*>>> sizing_list;
		std::deque<std::pair<uint, std::deque<cElementPrivate*>>> layout_list;
		std::deque<std::pair<uint, std::deque<cNodePrivate*>>> bounds_list;

		void add_to_sizing(cElementPrivate* e);
		void remove_from_sizing(cElementPrivate* e);
		void add_to_layout(cElementPrivate* e);
		void remove_from_layout(cElementPrivate* e);
		void add_to_bounds(cNodePrivate* n);
		void remove_from_bounds(cNodePrivate* n);

		void on_added() override;
		void update() override;
	};
}
