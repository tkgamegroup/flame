#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cNavAgent : Component
	{
		// Reflect requires
		cNodePtr node = nullptr;

		// Reflect
		float radius = 0.2f;
		// Reflect
		float height = 2.f;
		// Reflect
		float speed = 5.f;
		// Reflect
		float speed_scale = 1.f;
		// Reflect
		virtual void set_speed_scale(float v) = 0;
		// Reflect
		float turn_speed = 900.f;
		// Reflect
		float turn_speed_scale = 1.f;
		// Reflect
		virtual void set_turn_speed_scale(float v) = 0;
		// Reflect
		float stop_distance = 0.3f;
		// Reflect
		uint separation_group = 1;
		// Reflect
		bool flying = false;

		vec3 target_pos;
		float dist = -1; // the distance between agent and target. <0 means no target
		float ang_diff = 0.f; // the angle different between agent and target
		bool reached = false;

		virtual void set_target(const vec3& pos) = 0;
		virtual void stop() = 0;
		virtual void update_pos() = 0;

		struct Create
		{
			virtual cNavAgentPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
