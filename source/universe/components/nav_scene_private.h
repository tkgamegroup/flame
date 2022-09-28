#pragma once

#include "nav_scene.h"

namespace flame
{
	struct cNavScenePrivate : cNavScene
	{
		void start() override;
	};
}
