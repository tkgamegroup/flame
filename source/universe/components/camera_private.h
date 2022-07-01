#pragma once

#include "camera.h"

namespace flame
{
	struct cCameraPrivate : cCamera
	{
		void on_active() override;
		void on_inactive() override;
		void update() override;
	};
}
