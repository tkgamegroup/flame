#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cController : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cController";
		inline static auto type_hash = ch(type_name);

		cController() :
			Component(type_name, type_hash)
		{
		}

		virtual void move(const Vec3f& disp) = 0;

		FLAME_UNIVERSE_EXPORTS static cController* create();
	};
}
