#pragma once

#include "../system.h"

namespace flame
{
	struct DrawData;

	/// Reflect
	struct sRenderer : System
	{
		enum Mode
		{
			Shaded,
			CameraLight,
			AlbedoData,
			NormalData,
			MetallicData,
			RoughnessData,
			IBLValue,
			FogValue
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
			bool opa;
			std::vector<std::pair<int, graphics::ImagePtr>> texs;
			std::unordered_map<uint, graphics::GraphicsPipelinePtr> pls;
			uint ref = 0;
		};

		Mode mode = Shaded;
		cCameraPtr camera = nullptr;

		std::vector<graphics::ImageViewPtr> iv_tars;

		bool dirty = false;

		virtual void set_targets(std::span<graphics::ImageViewPtr> targets, graphics::ImageLayout final_layout = graphics::ImageLayoutShaderReadOnly) = 0;
		virtual void bind_window_targets() = 0;
		virtual vec2 target_size() = 0;

		graphics::ImageViewPtr sky_map = nullptr;
		graphics::ImageViewPtr sky_irr_map = nullptr;
		graphics::ImageViewPtr sky_rad_map = nullptr;
		virtual void set_sky_maps(graphics::ImageViewPtr sky_map, graphics::ImageViewPtr sky_irr_map, graphics::ImageViewPtr sky_rad_map) = 0;
		float sky_intensity = 1.f;
		virtual void set_sky_intensity(float v) = 0;
		vec3 fog_color = vec3(1.f);
		virtual void set_fog_color(const vec3& color) = 0;
		float shadow_distance = 100.f;
		virtual void set_shadow_distance(float d) = 0;
		uint csm_levels = 2;
		virtual void set_csm_levels(uint lv) = 0;
		float esm_factor = 7.f;
		virtual void set_esm_factor(float f) = 0;
		float ssao_radius = 0.5f;
		float ssao_bias = 0.025f;
		float white_point = 4.f;
		float gamma = 1.5f;

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
		virtual int register_light_instance(int id) = 0;
		virtual void set_light_instance(uint id, LightType type, const vec3& pos, const vec3& color, float range) = 0;

		// id == -1 to register or to unregister id
		virtual int register_mesh_instance(int id) = 0;
		virtual void set_mesh_instance(uint id, const mat4& mat, const mat3& nor) = 0;

		// id == -1 to register or to unregister id
		virtual int register_armature_instance(int id) = 0;
		virtual void set_armature_instance(uint id, const mat4* mats, uint size) = 0;

		// id == -1 to register or to unregister id
		virtual int register_terrain_instance(int id) = 0;
		virtual void set_terrain_instance(uint id, const mat4& mat, const vec3& extent, const uvec2& blocks, uint tess_level, uint grass_field_tess_level, uint grass_channel, int grass_texture_id,
			graphics::ImageViewPtr height_map, graphics::ImageViewPtr normal_map, graphics::ImageViewPtr tangent_map, graphics::ImageViewPtr splash_map) = 0;

		// id == -1 to register or to unregister id
		virtual int register_sdf_instance(int id) = 0;
		virtual void set_sdf_instance(uint id, uint boxes_count, std::pair<vec3, vec3>* boxes, uint spheres_count, std::pair<vec3, float>* spheres) = 0;

		virtual void render(uint tar_idx, graphics::CommandBufferPtr cb) = 0;

		virtual cNodePtr pick_up(const uvec2& screen_pos, vec3* out_pos = nullptr, const std::function<void(cNodePtr, DrawData&)>& draw_callback = {}) = 0;

		virtual void send_debug_string(const std::string& str) = 0;

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
