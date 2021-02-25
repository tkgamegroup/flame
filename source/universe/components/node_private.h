#pragma once

#include <flame/universe/components/node.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}
	
	struct sRendererPrivate;

	struct cNodePrivate : cNode
	{
		vec3 pos = vec3(0.f);
		vec3 eul = vec3(0.f);
		quat qut = quat(1.f, 0.f, 0.f, 0.f);
		mat3 rot = mat3(1.f);
		vec3 scl = vec3(1.f);
		bool eul_dirty = false;
		bool qut_dirty = false;
		bool rot_dirty = false;
		bool auto_update_eul = false;
		bool auto_update_qut = false;

		cNodePrivate* pnode = nullptr;
		bool transform_dirty = true;
		vec3 g_pos = vec3(0.f);
		quat g_qut = quat(1.f, 0.f, 0.f, 0.f);
		mat3 g_rot = mat3(1.f);
		vec3 g_scl = vec3(1.f);
		mat4 transform = mat4(1.f);

		std::vector<std::unique_ptr<Closure<void(Capture&, graphics::Canvas*)>>> drawers;

		sRendererPrivate* renderer = nullptr;

		vec3 get_pos() const override { return pos; }
		void set_pos(const vec3& pos) override;
		vec3 get_euler() const override { return eul; }
		void set_euler(const vec3& e) override;
		quat get_quat() const override { return qut; }
		void set_quat(const quat& quat) override;
		vec3 get_scale() const override { return scl; }
		void set_scale(const vec3 & scale) override;

		vec3 get_local_dir(uint idx) override;

		vec3 get_global_pos() override;
		vec3 get_global_dir(uint idx) override;

		void* add_drawer(void (*drawer)(Capture&, graphics::Canvas*), const Capture& capture);
		void remove_drawer(void* drawer);

		void update_eul();
		void update_qut();
		void update_rot();
		void update_transform();

		void set_auto_update_eul();
		void set_auto_update_qut();

		void mark_transform_dirty();
		void mark_drawing_dirty();

		void on_self_added() override;
		void on_self_removed() override;
		void on_entered_world() override;
		void on_left_world() override;

		bool on_save_attribute(uint64 h) override;
	};
}
