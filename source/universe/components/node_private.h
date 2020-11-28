#pragma once

#include <flame/universe/components/node.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}
	
	struct sRendererPrivate;

	struct cNodePrivate : cNode // R ~ on_*
	{
		vec3 pos = vec3(0.f);
		vec4 quat = vec4(0.f, 0.f, 0.f, 1.f);
		vec3 scaling = vec3(1.f);

		bool transform_dirty = true;
		mat3 rotation;
		vec3 global_pos;
		vec4 global_quat;
		vec3 global_scale;
		mat3 global_dirs;
		mat4 transform;

		std::vector<std::pair<Component*, void(*)(Component*, graphics::Canvas*)>> drawers;

		cNodePrivate* p_node; // R ref place=parent optional
		sRendererPrivate* renderer = nullptr; // R ref

		vec3 get_pos() const override { return pos; }
		void set_pos(const vec3& pos) override;
		vec4 get_quat() const override { return quat; }
		void set_quat(const vec4& quat) override;
		vec3 get_scale() const override { return scaling; }
		void set_scale(const vec3 & scale) override;

		void set_euler(const vec3& e) override;

		vec3 get_local_dir(uint idx) override;

		vec3 get_global_pos() override;
		vec3 get_global_dir(uint idx) override;

		mat4 get_transform() override;

		void update_transform();

		void mark_transform_dirty();
		void mark_drawing_dirty();

		void on_local_message(Message msg, void* p) override;
	};
}
