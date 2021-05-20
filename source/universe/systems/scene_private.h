#pragma once

#include "scene.h"

namespace flame
{
	struct sScenePrivate : sScene
	{
		Window* window = nullptr;

		std::deque<cElementPrivate*> sizing_list;
		std::deque<cElementPrivate*> layout_list;
		std::deque<cNodePrivate*> reindex_list;

		void on_added() override;
		void update() override;
	};
}
