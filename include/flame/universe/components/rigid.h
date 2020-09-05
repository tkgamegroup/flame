#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cRigid : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cRigid";
		inline static auto type_hash = ch(type_name);

		cRigid() :
			Component(type_name, type_hash)
		{
		}

		virtual bool get_dynamic() const = 0;
		virtual void set_dynamic(bool v) = 0;

		virtual void add_impulse(const Vec3f& v) = 0;

		FLAME_UNIVERSE_EXPORTS static cRigid* create();
	};
}
