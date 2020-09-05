#pragma once

#include <flame/universe/components/node.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}
	
	struct cCamera;
	struct sRendererPrivate;

	struct cNodePrivate : cNode // R ~ on_*
	{
		Vec3f pos = Vec3f(0.f);
		Vec4f quat = Vec4f(0.f, 0.f, 0.f, 1.f);
		Vec3f scale = Vec3f(1.f);

		bool transform_dirty = true;
		Vec3f global_pos;
		Vec4f global_quat;
		Vec3f global_scale;
		Mat3f axes;
		Mat4f transform;

		std::vector<std::pair<Component*, void(*)(Component*, graphics::Canvas*, cCamera*)>> drawers;

		cNodePrivate* p_node; // R ref place=parent optional
		sRendererPrivate* renderer = nullptr; // R ref

		Vec3f get_pos() const override { return pos; }
		void set_pos(const Vec3f& pos) override;
		Vec4f get_quat() const override { return quat; }
		void set_quat(const Vec4f& quat) override;
		Vec3f get_scale() const override { return scale; }
		void set_scale(const Vec3f & scale) override;

		void set_euler(const Vec3f& e) override;

		Vec3f get_dir(uint idx) override;

		void update_transform();

		void mark_transform_dirty();
		void mark_drawing_dirty();

		void on_local_message(Message msg, void* p) override;
	};
}
