#pragma once

#include "../system.h"

namespace flame
{
	/// Reflect
	struct sScene : System
	{
		//		virtual bool is_any_within_circle(const vec2& c, float r, uint filter_tag = 0xffffffff) = 0;
		//		virtual uint get_within_circle(const vec2& c, float r, EntityPtr* dst, uint max_count, uint filter_tag = 0xffffffff) = 0;

		struct Instance
		{
			virtual sScenePtr operator()() = 0;
		};
		FLAME_UNIVERSE_EXPORTS static Instance& instance;

		struct Create
		{
			virtual sScenePtr operator()(WorldPtr) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_EXPORTS static Create& create;
	};
}
