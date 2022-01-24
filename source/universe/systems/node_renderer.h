#pragma once

#include "../system.h"

namespace flame
{
	/// Reflect
	struct sNodeRenderer : System
	{
		bool dirty = false;

		virtual void set_targets(std::span<graphics::ImageViewPtr> targets, graphics::ImageLayout dst_layout = graphics::ImageLayoutPresent) = 0;

		virtual int set_material_res(int idx, graphics::Material* mat) = 0;
		virtual int find_material_res(graphics::Material* mat) const = 0;

		virtual int set_mesh_res(int idx, graphics::Mesh* mesh) = 0;
		virtual int find_mesh_res(graphics::Mesh* mesh) const = 0;

		virtual int register_object() = 0;
		virtual void unregister_object(uint id) = 0;
		virtual void set_object_matrix(uint id, const mat4& mat, const mat3& nor) = 0;

		virtual int register_armature_object() = 0;
		virtual void unregister_armature_object(uint id) = 0;
		virtual void set_armature_object_matrices(uint id, const mat4* bones, uint count) = 0;

		virtual void draw_mesh(uint object_id, uint mesh_id, uint skin, DrawType type) = 0;

		virtual cNodePtr pick_up(const uvec2& pos) = 0;

		struct Instance
		{
			virtual sNodeRendererPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;

		struct Create
		{
			virtual sNodeRendererPtr operator()(WorldPtr) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
