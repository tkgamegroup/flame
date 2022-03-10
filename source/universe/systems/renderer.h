#pragma once

#include "../system.h"

namespace flame
{
	/// Reflect
	struct sRenderer : System
	{
		cCameraPtr camera = nullptr;

		bool dirty = false;

		virtual void set_targets(std::span<graphics::ImageViewPtr> targets, graphics::ImageLayout final_layout = graphics::ImageLayoutShaderReadOnly) = 0;
		virtual void bind_window_targets() = 0;

		virtual int get_texture_res(const std::filesystem::path& filename, bool srgb = false, graphics::SamplerPtr sp = nullptr, const graphics::Image::MipmapOption& mipmap_option = {}) = 0;
		virtual void release_texture_res(uint id) = 0;

		virtual int set_mesh_res(int idx, graphics::Mesh* mesh) = 0;
		virtual int find_mesh_res(graphics::Mesh* mesh) const = 0;

		virtual int set_material_res(int idx, graphics::Material* mat) = 0;
		virtual int find_material_res(graphics::Material* mat) const = 0;

		// id == -1 to register or to unregister id
		virtual int register_mesh_instance(int id) = 0;
		virtual void set_mesh_instance(uint id, const mat4& mat, const mat3& nor) = 0;

		// id == -1 to register or to unregister id
		virtual int register_armature_instance(int id) = 0;
		virtual mat4* set_armature_instance(uint id) = 0;

		// id == -1 to register or to unregister id
		virtual int register_terrain_instance(int id) = 0;
		virtual void set_terrain_instance(uint id, const mat4& mat, const vec3& extent, const uvec2& blocks, uint tess_level, graphics::ImageViewPtr textures) = 0;

		// you must call draw in a node's drawer

		virtual void draw_line(const vec3* points, uint count, const cvec4& color) = 0;

		virtual void draw_mesh(uint instance_id, uint mesh_id, uint skin) = 0;
		virtual void draw_mesh_occluder(uint instance_id, uint mesh_id, uint skin) = 0;
		virtual void draw_mesh_outline(uint instance_id, uint mesh_id, const cvec4& color) = 0;
		virtual void draw_mesh_wireframe(uint instance_id, uint mesh_id, const cvec4& color) = 0;
		virtual void draw_terrain(uint instance_id, uint blocks, uint material_id) = 0;
		virtual void draw_terrain_outline(uint instance_id, uint blocks, const cvec4& color) = 0;
		virtual void draw_terrain_wireframe(uint instance_id, uint blocks, const cvec4& color) = 0;
		virtual void render(uint tar_idx, graphics::CommandBufferPtr cb) = 0;

		virtual cNodePtr pick_up(const uvec2& screen_pos, vec3* out_pos = nullptr) = 0;

		struct Instance
		{
			virtual sRendererPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;

		struct Create
		{
			virtual sRendererPtr operator()(WorldPtr) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
