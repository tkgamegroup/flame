#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cNode : Component
	{
		/// Reflect
		bool is_static = false;

		/// Reflect
		vec3 pos = vec3(0.f);
		/// Reflect
		virtual void set_pos(const vec3& pos) = 0;
		inline void add_pos(const vec3& p)
		{
			set_pos(pos + p);
		}

		// yaw, pitch, roll, in angle
		vec3 eul = vec3(0.f);
		/// Reflect
		virtual vec3 get_eul() = 0;
		/// Reflect
		virtual void set_eul(const vec3& eul) = 0;
		inline void add_eul(const vec3& e)
		{
			set_eul(get_eul() + e);
		}

		quat qut = quat(1.f, 0.f, 0.f, 0.f);
		virtual quat get_qut() = 0;
		virtual void set_qut(const quat& qut) = 0;

		mat3 rot = mat3(1.f);

		/// Reflect
		vec3 scl = vec3(1.f);
		/// Reflect
		virtual void set_scl(const vec3& scl) = 0;

		vec3 g_pos;
		mat3 g_rot;
		vec3 g_scl;
		AABB bounds;

		OctNode* octnode = nullptr;

		Listeners<void(sNodeRendererPtr, bool)> drawers;
		Listeners<bool(AABB*)> measurers;

		virtual void look_at(const vec3& t) = 0;
		
		struct Create
		{
			virtual cNodePtr operator()(EntityPtr) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_EXPORTS static Create& create;
	};
}
