#pragma once

#include "scene.h"
#include "../components/node_private.h"

namespace flame
{
	struct sScenePrivate : sScene
	{
		OctNode* octree = nullptr;

		sScenePrivate();

		void update_transform(EntityPtr e);
		void update() override;
	};
}
