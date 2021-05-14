#pragma once

#include "../component.h"

namespace flame
{
	struct cRigid : Component
	{
		inline static auto type_name = "flame::cRigid";
		inline static auto type_hash = ch(type_name);

		cRigid() :
			Component(type_name, type_hash)
		{
		}

		virtual bool get_dynamic() const = 0;
		virtual void set_dynamic(bool v) = 0;

		virtual void add_impulse(const vec3& v) = 0;

		virtual void* add_trigger_listener(void (*callback)(Capture& c, physics::TouchType type, cShapePtr trigger_shape, cShapePtr other_shape), const Capture& capture) = 0;
		virtual void remove_trigger_listener(void* lis) = 0;
		
		FLAME_UNIVERSE_EXPORTS static cRigid* create(void* parms = nullptr);
	};
}