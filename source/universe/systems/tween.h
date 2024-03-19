#pragma once

#include "../system.h"

namespace flame
{
	struct sTween : System
	{
		enum RendererType
		{
			RendererGui
		};

		virtual uint begin(EntityPtr e) = 0;
		virtual uint begin(RendererType render_type, BlueprintInstanceGroupPtr render) = 0;
		virtual void end(uint id) = 0;
		virtual void newline(uint id) = 0;
		virtual void wait(uint id, float time) = 0;
		virtual void move_to(uint id, const vec3& pos, float duration) = 0;
		virtual void rotate_to(uint id, const quat& qut, float duration) = 0;
		virtual void scale_to(uint id, const vec3& scale, float duration) = 0;
		virtual void play_animation(uint id, uint name) = 0;
		virtual void kill(uint id) = 0;

		struct Instance
		{
			virtual sTweenPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;

		struct Create
		{
			virtual sTweenPtr operator()(WorldPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
