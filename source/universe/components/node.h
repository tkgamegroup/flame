#pragma once

#include "../component.h"

namespace flame
{
	struct cNode : Component
	{
		inline static auto type_name = "flame::cNode";
		inline static auto type_hash = ch(type_name);

		vec3 pos = vec3(0.f);
		// yaw, pitch, roll, in angle
		vec3 eul = vec3(0.f);
		quat qut = quat(1.f, 0.f, 0.f, 0.f);
		mat3 rot = mat3(1.f);
		vec3 scl = vec3(1.f);
		vec3 g_pos;
		mat3 g_rot;
		vec3 g_scl;
		AABB bounds;

		bool merge_bounds = false;

		Listeners<uint(sRendererPtr, bool, bool)> drawers;
		Listeners<uint(AABB*)> measurers;

		cNode() :
			Component(type_name, type_hash)
		{
		}

		virtual void set_pos(const vec3& pos) = 0;
		virtual void set_eul(const vec3& eul) = 0;
		virtual void set_qut(const quat& qut) = 0;
		virtual void set_scl(const vec3& scl) = 0;

		virtual void look_at(const vec3& t) = 0;

		FLAME_UNIVERSE_EXPORTS static cNode* create();
	};
}
