#pragma once

#include "../system.h"

namespace flame
{
	/// Reflect
	struct sRenderer : System
	{
		enum Type
		{
			Shaded,
			CameraLight
		};

		struct TexRes
		{
			graphics::ImageViewPtr iv;
			graphics::SamplerPtr sp = nullptr;
			uint ref = 0;
		};

		struct MeshRes
		{
			graphics::MeshPtr mesh = nullptr;
			bool arm;
			uint vtx_off;
			uint vtx_cnt;
			uint idx_off;
			uint idx_cnt;
			uint ref = 0;
		};

		struct MatRes
		{
			graphics::MaterialPtr mat = nullptr;
			std::vector<std::pair<int, graphics::ImagePtr>> texs;
			std::unordered_map<uint, graphics::GraphicsPipelinePtr> pls;
			std::vector<uint> draw_ids;
			uint ref = 0;
		};

		Type type = Shaded;
		cCameraPtr camera = nullptr;

		bool dirty = false;

		virtual void set_targets(std::span<graphics::ImageViewPtr> targets, graphics::ImageLayout final_layout = graphics::ImageLayoutShaderReadOnly) = 0;
		virtual void bind_window_targets() = 0;

		// id: >=0: specify an id, -1: get an empty slot, -2: only find the res id (no need to release)
		virtual int get_texture_res(graphics::ImageViewPtr iv, graphics::SamplerPtr sp = nullptr, int id = -1) = 0;
		virtual void release_texture_res(uint id) = 0;
		virtual const TexRes& get_texture_res_info(uint id) = 0;

		// id: >=0: specify an id, -1: get an empty slot, -2: only find the res id (no need to release)
		virtual int get_mesh_res(graphics::MeshPtr mesh, int id = -1) = 0;
		virtual void release_mesh_res(uint id) = 0;
		virtual const MeshRes& get_mesh_res_info(uint id) = 0;

		// id: >=0: specify an id, -1: get an empty slot, -2: only find the res id (no need to release)
		virtual int get_material_res(graphics::Material* mat, int id = -1) = 0;
		virtual void release_material_res(uint id) = 0;
		virtual const MatRes& get_material_res_info(uint id) = 0;

		// id == -1 to register or to unregister id
		virtual int register_mesh_instance(int id) = 0;
		virtual void set_mesh_instance(uint id, const mat4& mat, const mat3& nor) = 0;

		// id: res id
		// type_hash: texture, mesh or material
		// name_hash: name or genre, to decide which part to update, set to 0 means ALL, (see below)
		// Type      | Name                     | Description
		//  texture  |  *currently not support  |  -
		//  mesh     |  *currently not support  |  -
		//  material |  parameters              |  all data except textures and pipelines
		//  material |  textures                |  the textures
		//  material |  pipelines               |  the pipelines
		virtual void update_res(uint id, uint type_hash, uint name_hash) = 0;

		// id == -1 to register or to unregister id
		virtual int register_armature_instance(int id) = 0;
		virtual mat4* set_armature_instance(uint id) = 0;

		// id == -1 to register or to unregister id
		virtual int register_terrain_instance(int id) = 0;
		virtual void set_terrain_instance(uint id, const mat4& mat, const vec3& extent, const uvec2& blocks, uint tess_level, 
			graphics::ImageViewPtr height_map, graphics::ImageViewPtr normal_map, graphics::ImageViewPtr tangent_map) = 0;

		// id == -1 to register or to unregister id
		virtual int register_light_instance(int id) = 0;
		virtual void add_light(uint instance_id, LightType type, const vec3& pos, const vec3& color, float range, bool cast_shadow) = 0;

		// you must call draw in a node's drawer

		virtual void draw_line(const vec3* points, uint count, const cvec4& color) = 0;

		virtual void draw_mesh(uint instance_id, uint mesh_id, uint mat_id) = 0;
		virtual void draw_mesh_occluder(uint instance_id, uint mesh_id, uint mat_id) = 0;
		virtual void draw_mesh_outline(uint instance_id, uint mesh_id, const cvec4& color) = 0;
		virtual void draw_mesh_wireframe(uint instance_id, uint mesh_id, const cvec4& color) = 0;
		virtual void draw_terrain(uint instance_id, uint blocks, uint mat_id) = 0;
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
