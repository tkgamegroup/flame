#pragma once

#include "scene.h"

namespace flame
{
	struct sScenePrivate : sScene
	{
		void update() override;
	};
}
