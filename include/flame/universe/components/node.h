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

		virtual Vec3f get_pos() const = 0;
		virtual void set_pos(const Vec3f& pos) = 0;
		virtual Vec4f get_quat() const = 0;
		virtual void set_quat(const Vec4f& quat) = 0;
		virtual Vec3f get_scale() const = 0;
		virtual void set_scale(const Vec3f& scale) = 0;

		// yaw, pitch, roll, in angle
		virtual void set_euler(const Vec3f& e) = 0;

		virtual Vec3f get_global_pos() = 0;
		virtual Vec3f get_global_dir(uint idx) = 0;

		FLAME_UNIVERSE_EXPORTS static cNode* create();
	};
}
