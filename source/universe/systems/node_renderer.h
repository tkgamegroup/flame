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

		// return: transform id
		virtual uint add_mesh_transform(const mat4& mat, const mat3& nor) = 0;
		// return: armature id
		virtual uint add_mesh_armature(const mat4* bones, uint count) = 0;
		// id: transform or armature id
		virtual void draw_mesh(uint id, uint mesh_id, uint skin, ShadingFlags flags = ShadingMaterial) = 0;

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
