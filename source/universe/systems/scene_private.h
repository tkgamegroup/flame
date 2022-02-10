#pragma once

#include "scene.h"
#include "../components/node_private.h"

namespace flame
{
	struct sScenePrivate : sScene
	{
		sScenePrivate();

		void update_transform(EntityPtr e, bool mark_dirty);
		void update() override;
	};
}
