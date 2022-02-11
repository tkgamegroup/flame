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

		// id == -1 to register or to unregister id
		virtual int register_mesh_instance(int id) = 0;
		virtual void set_mesh_instance(uint id, const mat4& mat, const mat3& nor) = 0;

		// id == -1 to register or to unregister id
		virtual int register_armature_instance(int id) = 0;
		virtual mat4* set_armature_instance(uint id) = 0;

		// id == -1 to register or to unregister id
		virtual int register_terrain_instance(int id) = 0;
		virtual void set_terrain_instance(uint id, const mat4& mat) = 0;

		// you must call draw in a node's drawer

		virtual void draw_mesh(uint object_id, uint mesh_id, uint skin) = 0;
		virtual void draw_mesh_occluder(uint object_id, uint mesh_id, uint skin) = 0;
		virtual void draw_mesh_outline(uint object_id, uint mesh_id, const cvec4& color) = 0;
		virtual void draw_mesh_wireframe(uint object_id, uint mesh_id, const cvec4& color) = 0;
		virtual void draw_terrain() = 0;
		virtual void draw_terrain_outline() = 0;
		virtual void draw_terrain_wireframe() = 0;

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
