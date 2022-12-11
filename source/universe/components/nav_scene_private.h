#pragma once

#include "nav_scene.h"

namespace flame
{
	struct cNavScenePrivate : cNavScene
	{
		uint _frame = 0;

		void start() override;
		void update() override;
	};
}
