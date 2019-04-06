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
	public:
		ControllerComponent();
		
		ControllerState get_state() const;
		float get_ang_offset() const;
		float get_speed() const;
		float get_turn_speed() const;

		bool set_state(ControllerState _s, bool enable);
		void set_ang_offset(float v);
		void set_speed(float v);
		void set_turn_speed(float v);

		void reset();
	protected:
		virtual void on_update() override;
	};
}
