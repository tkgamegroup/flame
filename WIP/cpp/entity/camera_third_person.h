#pragma once

#include <flame/math.h>

namespace flame
{
	class CameraComponent;

	void set_camera_third_person_position(CameraComponent *camera, const glm::vec3 target, float length);
}
