#pragma once

#include <flame/math.h>
#include <flame/engine/entity/component.h>

namespace flame
{
	enum ControllerState
	{
		ControllerStateStand = 0,
		ControllerStateForward = 1 << 0,
		ControllerStateBackward = 1 << 1,
		ControllerStateLeft = 1 << 2,
		ControllerStateRight = 1 << 3,
		ControllerStateUp = 1 << 4,
		ControllerStateDown = 1 << 5,
		ControllerStateTurnLeft = 1 << 6,
		ControllerStateTurnRight = 1 << 7
	};

	class ControllerComponent : public Component
	{
	private:
		ControllerState state = ControllerStateStand;
		float ang_offset = 0.f;
		float speed = 1.f;
		float turn_speed = 75.f;
	};
}
