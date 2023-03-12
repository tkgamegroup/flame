#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cNode : Component
	{
		// Reflect
		bool is_static = false;

		// Reflect
		vec3 pos = vec3(0.f);
		// Reflect
		virtual void set_pos(const vec3& pos) = 0;
		inline void add_pos(const vec3& p)
		{
			set_pos(pos + p);
		}

		// yaw, pitch, roll, in angle
		vec3 eul = vec3(0.f);
		// Reflect
		virtual vec3 get_eul() = 0;
		// Reflect
		virtual void set_eul(const vec3& eul) = 0;
		inline void add_eul(const vec3& e)
		{
			set_eul(get_eul() + e);
		}

		quat qut = quat(1.f, 0.f, 0.f, 0.f);
		// Reflect
		virtual quat get_qut() = 0;
		// Reflect
		virtual void set_qut(const quat& qut) = 0;
		inline void mul_qut(const quat& qut)
		{
			set_qut(get_qut() * qut);
		}

		mat3 rot = mat3(1.f);

		// Reflect
		vec3 scl = vec3(1.f);
		// Reflect
		virtual void set_scl(const vec3& scl) = 0;

		inline void set_transform(const vec3& pos, const vec3& scl)
		{
			set_pos(pos);
			set_scl(scl);
		}

		inline void set_transform(const vec3& pos, const quat& qut)
		{
			set_pos(pos);
			set_qut(qut);
		}

		inline void set_transform(const vec3& pos, const quat& qut, const vec3& scl)
		{
			set_pos(pos);
			set_qut(qut);
			set_scl(scl);
		}

		vec3 g_pos;
		mat3 g_rot;
		vec3 g_scl;
		mat4 transform;
		AABB bounds;

		OctNode* octnode = nullptr;

		virtual void mark_transform_dirty() = 0;
		virtual void mark_drawing_dirty() = 0;

		Listeners<void(DrawData&)> drawers;
		Listeners<void(AABB&)> measurers;

		virtual void look_at(const vec3& t) = 0;

		virtual void update_eul() = 0;
		virtual void update_qut() = 0;
		virtual void update_rot() = 0;
		virtual bool update_transform() = 0;

		struct Create
		{
			virtual cNodePtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
