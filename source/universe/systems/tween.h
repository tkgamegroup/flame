#pragma once

#include "../universe.h"

namespace flame
{
	struct Tween
	{
		virtual uint begin(EntityPtr e) = 0;
		virtual void end(uint id) = 0;
		virtual void newline(uint id) = 0;
		virtual void wait(uint id, float time) = 0;
		virtual void move_to(uint id, const vec3& pos, float duration) = 0;
		virtual void rotate_to(uint id, const quat& rot, float duration) = 0;
		virtual void rotate_to(uint id, const vec3& eul, float duration) = 0;
		virtual void scale_to(uint id, float scale, float duration) = 0;
		virtual void scale_to(uint id, const vec3& scale, float duration) = 0;
		virtual void play_animation(uint id, uint name) = 0;
		virtual void kill(uint id) = 0;

		struct Instance
		{
			virtual TweenPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;
	};
}
