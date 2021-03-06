#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cNode : Component
	{
		inline static auto type_name = "flame::cNode";
		inline static auto type_hash = ch(type_name);

		cNode() :
			Component(type_name, type_hash)
		{
		}

		virtual vec3 get_pos() const = 0;
		virtual void set_pos(const vec3& pos) = 0;
		virtual quat get_quat() const = 0;
		virtual void set_quat(const quat& quat) = 0;
		virtual vec3 get_scale() const = 0;
		virtual void set_scale(const vec3& scale) = 0;

		// yaw, pitch, roll, in angle
		virtual vec3 get_euler() const = 0;
		virtual void set_euler(const vec3& e) = 0;

		virtual vec3 get_local_dir(uint idx) = 0;

		virtual vec3 get_global_pos() = 0;
		virtual vec3 get_global_dir(uint idx) = 0;

		FLAME_UNIVERSE_EXPORTS static cNode* create(void* parms = nullptr);
	};
}
