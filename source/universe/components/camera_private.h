#pragma once

#include "camera.h"

namespace flame
{
	struct cCameraPrivate : cCamera
	{
		vec2 world_to_screen(const vec3& pos) override;

		void on_active() override;
		void on_inactive() override;
		void update_matrices();
	};
}
