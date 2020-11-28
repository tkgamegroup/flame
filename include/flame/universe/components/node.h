#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cNode : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cNode";
		inline static auto type_hash = ch(type_name);

		cNode() :
			Component(type_name, type_hash)
		{
		}

		virtual vec3 get_position() const = 0;
		virtual void set_position(const vec3& pos) = 0;
		virtual vec4 get_quat() const = 0;
		virtual void set_quat(const vec4& quat) = 0;
		virtual vec3 get_scale() const = 0;
		virtual void set_scale(const vec3& scale) = 0;

		// yaw, pitch, roll, in angle
		virtual void set_euler(const vec3& e) = 0;

		virtual vec3 get_local_dir(uint idx) = 0;

		virtual vec3 get_global_pos() = 0;
		virtual vec3 get_global_dir(uint idx) = 0;

		virtual mat4 get_transform() = 0;

		FLAME_UNIVERSE_EXPORTS static cNode* create();
	};
}
