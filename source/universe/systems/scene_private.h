#pragma once

#include "scene.h"
#include "../components/node_private.h"

namespace flame
{
	struct sScenePrivate : sScene
	{
		sScenePrivate();

		void update_transform(EntityPtr e, bool mark_dirty);

		void generate_navmesh(const std::filesystem::path& output) override;
		void update() override;
	};
}
