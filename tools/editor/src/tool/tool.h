#pragma once

#include <flame/engine/entity/camera.h>

struct Tool
{
	Tool();
	virtual void show(glm::vec2 _window_pos, glm::vec2 _window_size, flame::CameraComponent *camera) = 0;
	virtual ~Tool() {};
};
