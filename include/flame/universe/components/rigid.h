#pragma once

#include <flame/physics/physics.h>
#include <flame/universe/component.h>

namespace flame
{
	struct cShape;

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

		virtual void add_impulse(const vec3& v) = 0;

		virtual void* add_trigger_listener(void (*callback)(Capture& c, physics::TouchType type, cShape* trigger_shape, cShape* other_shape), const Capture& capture) = 0;
		virtual void remove_trigger_listener(void* lis) = 0;

		virtual void add_trigger_listener_s(uint slot) = 0;
		virtual void remove_trigger_listener_s(uint slot) = 0;

		virtual void on_trigger_event(physics::TouchType type, cShape* trigger_shape, cShape* other_shape) = 0;

		FLAME_UNIVERSE_EXPORTS static cRigid* create();
	};
}
