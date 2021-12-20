#pragma once

#include "scene.h"

namespace flame
{
	struct sScenePrivate : sScene
	{
		NativeWindow* window = nullptr;

		std::deque<std::pair<uint, std::deque<cElementPrivate*>>> sizing_list;
		std::deque<std::pair<uint, std::deque<cElementPrivate*>>> layout_list;
		std::deque<std::pair<uint, std::deque<cNodePrivate*>>> update_bounds_list;

		void setup(NativeWindow* window) override;

		void add_to_sizing(cElementPrivate* e);
		void remove_from_sizing(cElementPrivate* e);
		void add_to_layout(cElementPrivate* e);
		void remove_from_layout(cElementPrivate* e);
		void add_to_update_bounds(cNodePrivate* n);
		void remove_from_update_bounds(cNodePrivate* n);

		void update() override;
	};
}
