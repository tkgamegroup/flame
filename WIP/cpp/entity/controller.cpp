namespace flame
{
	ControllerComponent::ControllerComponent() :
		Component(ComponentTypeController)
	{
	}

	bool ControllerComponent::set_state(ControllerState _s, bool enable)
	{
		if (_s == ControllerStateStand)
		{
			if (state != ControllerStateStand)
			{
				state = ControllerStateStand;
				return true;
			}
			return false;
		}

		if (enable)
		{
			if (!(state & _s))
			{
				state = ControllerState(state | _s);
				return true;
			}
			return false;
		}
		else
		{
			if (state & _s)
			{
				state = ControllerState(state & ~_s);
				return true;
			}
			return false;
		}
	}

	void ControllerComponent::reset()
	{
		state = ControllerStateStand;
	}

	void ControllerComponent::on_update()
	{
		auto coord = get_parent()->get_coord();
		auto yaw = get_parent()->get_euler().x;

		if (state == ControllerStateStand)
			return;

		if (turn_speed > 0.f)
		{
			if ((state & ControllerStateTurnLeft) && !(state & ControllerStateTurnRight))
				yaw += turn_speed * elapsed_time;
			if ((state & ControllerStateTurnRight) && !(state & ControllerStateTurnLeft))
				yaw -= turn_speed * elapsed_time;
		}

		auto rad = glm::radians(yaw + ang_offset);

		if (speed > 0.f)
		{
			if ((state & ControllerStateForward) && !(state & ControllerStateBackward))
			{
				coord.x -= sin(rad) * speed * elapsed_time;
				coord.z -= cos(rad) * speed * elapsed_time;
			}
			if ((state & ControllerStateBackward) && !(state & ControllerStateForward))
			{
				coord.x += sin(rad) * speed * elapsed_time;
				coord.z += cos(rad) * speed * elapsed_time;
			}
			if ((state & ControllerStateLeft) && !(state & ControllerStateRight))
			{
				coord.x -= cos(rad) * speed * elapsed_time;
				coord.z += sin(rad) * speed * elapsed_time;
			}
			if ((state & ControllerStateRight) && !(state & ControllerStateLeft))
			{
				coord.x += cos(rad) * speed * elapsed_time;
				coord.z -= sin(rad) * speed * elapsed_time;
			}
			if ((state & ControllerStateUp) && !(state & ControllerStateDown))
				coord.y += speed * elapsed_time;
			if ((state & ControllerStateDown) && !(state & ControllerStateUp))
				coord.y -= speed * elapsed_time;
		}

		get_parent()->set_coord(coord);
		get_parent()->set_euler_x(yaw);
	}
}