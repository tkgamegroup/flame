#pragma once

#ifdef FLAME_UNIVERSE_MODULE
#define FLAME_UNIVERSE_EXPORTS __declspec(dllexport)
#else
#define FLAME_UNIVERSE_EXPORTS __declspec(dllimport)
#endif

#include <flame/foundation/foundation.h>

namespace flame
{
	struct Entity;

	FLAME_UNIVERSE_EXPORTS int get_world_frame();
	FLAME_UNIVERSE_EXPORTS void reset_world_frame();
	FLAME_UNIVERSE_EXPORTS void update_world(Entity* root_node, float delta_time);
	FLAME_UNIVERSE_EXPORTS void traverse_forward(Entity* node, const Function<void(void* c, Entity* e)>& callback);
	FLAME_UNIVERSE_EXPORTS void traverse_backward(Entity* node, const Function<void(void* c, Entity* e)>& callback);

	struct ATTRIBUTE_BOOL
	{
		bool val;
		int frame;

		ATTRIBUTE_BOOL() :
			frame(-1)
		{
		}

		operator bool() const
		{
			return val;
		}

		void operator=(bool rhs)
		{
			val = rhs;
			frame = get_world_frame();
		}

		void operator&=(bool rhs)
		{
			val &= rhs;
			frame = get_world_frame();
		}

		void operator|=(bool rhs)
		{
			val |= rhs;
			frame = get_world_frame();
		}
	};

	template <class T>
	struct ATTRIBUTE_NUMBER
	{
		T val;
		int frame;

		ATTRIBUTE_NUMBER() :
			frame(-1)
		{
		}

		operator T() const
		{
			return val;
		}

		void operator=(const T& rhs)
		{
			val = rhs;
			frame = get_world_frame();
		}

		void operator+=(const T& rhs)
		{
			val += rhs;
			frame = get_world_frame();
		}

		void operator-=(const T& rhs)
		{
			val -= rhs;
			frame = get_world_frame();
		}

		void operator*=(const T& rhs)
		{
			val *= rhs;
			frame = get_world_frame();
		}

		void operator/=(const T& rhs)
		{
			val /= rhs;
			frame = get_world_frame();
		}
	};
}

