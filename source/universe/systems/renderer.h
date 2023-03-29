#pragma once

#include "../system.h"

namespace flame
{
	struct DrawData;

	// Reflect ctor
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

		struct MatVar
		{
			std::string name;
			uint ref = 0;
		};

		struct TexRes
		{
			graphics::ImageViewPtr iv;
			graphics::SamplerPtr sp = nullptr;
			std::string name;
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
			uint ref = 0;
		};

		Mode mode = Shaded;
		graphics::WindowPtr window;
		cCameraPtr camera = nullptr;
		std::vector<graphics::ImageViewPtr> iv_tars;
		bool use_window_targets = false;
		bool dirty = false;

		// Reflect
		virtual void set_targets(std::span<graphics::ImageViewPtr> targets, graphics::ImageLayout final_layout = graphics::ImageLayoutShaderReadOnly) = 0;
		// Reflect
		virtual void bind_window_targets() = 0;
		// Reflect
		virtual vec2 target_extent() = 0;

		// Reflect
		graphics::ImageViewPtr sky_map = nullptr;
		// Reflect
		graphics::ImageViewPtr sky_irr_map = nullptr;
		// Reflect
		graphics::ImageViewPtr sky_rad_map = nullptr;
		// Reflect
		virtual void set_sky_maps(graphics::ImageViewPtr sky_map, graphics::ImageViewPtr sky_irr_map, graphics::ImageViewPtr sky_rad_map) = 0;
		// Reflect
		float sky_intensity = 0.f;
		// Reflect
		virtual void set_sky_intensity(float v) = 0;
		// Reflect
		vec3 fog_color = vec3(0.f);
		// Reflect
		virtual void set_fog_color(const vec3& color) = 0;
		// Reflect
		float shadow_distance = 0.f;
		// Reflect
		virtual void set_shadow_distance(float d) = 0;
		// Reflect
		uint csm_levels = 0;
		// Reflect
		virtual void set_csm_levels(uint lv) = 0;
		// Reflect
		float esm_factor = 0.f;
		// Reflect
		virtual void set_esm_factor(float f) = 0;
		// Reflect
		bool post_processing_enable = false;
		// Reflect
		virtual void set_post_processing_enable(bool v) = 0;
		// Reflect
		bool ssao_enable = false;
		// Reflect
		virtual void set_ssao_enable(bool v) = 0;
		// Reflect
		float ssao_radius = 0.f;
		// Reflect
		virtual void set_ssao_radius(float v) = 0;
		// Reflect
		float ssao_bias = 0.f;
		// Reflect
		virtual void set_ssao_bias(float v) = 0;
		// Reflect
		float white_point = 0.f;
		// Reflect
		virtual void set_white_point(float v) = 0;
		// Reflect
		bool bloom_enable = false;
		// Reflect
		virtual void set_bloom_enable(bool v) = 0;
		// Reflect
		bool ssr_enable = false;
		// Reflect
		virtual void set_ssr_enable(bool v) = 0;
		// Reflect
		float ssr_thickness = 0.f;
		// Reflect
		virtual void set_ssr_thickness(float v) = 0;
		// Reflect
		float ssr_max_distance = 0.f;
		// Reflect
		virtual void set_ssr_max_distance(float v) = 0;
		// Reflect
		uint ssr_max_steps = 0;
		// Reflect
		virtual void set_ssr_max_steps(uint v) = 0;
		// Reflect
		uint ssr_binary_search_steps = 0;
		// Reflect
		virtual void set_ssr_binary_search_steps(uint v) = 0;
		// Reflect
		bool tone_mapping_enable = false;
		// Reflect
		virtual void set_tone_mapping_enable(bool v) = 0;
		// Reflect
		float gamma = 0.f;
		// Reflect
		virtual void set_gamma(float v) = 0;

		// Reflect
		virtual int get_mat_var(int id, const std::string& name) = 0;
		// Reflect
		virtual void release_mat_var(uint id) = 0;
		// Reflect
		virtual const MatVar& get_mat_var_info(uint id) = 0;
		// Reflect
		virtual void set_mat_var(uint id, const vec4& v) = 0;

		// Reflect
		virtual std::filesystem::path get_post_shading_code_file() = 0;
		// Reflect
		virtual void set_post_shading_code_file(const std::filesystem::path& path) = 0;

		// id: >=0: specify an id, -1: get an empty slot, -2: only find the res id (no need to release)
		// Reflect
		virtual int get_texture_res(graphics::ImageViewPtr iv, graphics::SamplerPtr sp = nullptr, int id = -1) = 0;
		// Reflect
		virtual void release_texture_res(uint id) = 0;
		// Reflect
		virtual const TexRes& get_texture_res_info(uint id) = 0;
		// set name to texture will let it appear in shaders
		// Reflect
		virtual void set_texture_res_name(uint id, const std::string& name) = 0;

		// id: >=0: specify an id, -1: get an empty slot, -2: only find the res id (no need to release)
		// Reflect
		virtual int get_mesh_res(graphics::MeshPtr mesh, int id = -1) = 0;
		// Reflect
		virtual void release_mesh_res(uint id) = 0;
		// Reflect
		virtual const MeshRes& get_mesh_res_info(uint id) = 0;

		// id: >=0: specify an id, -1: get an empty slot, -2: only find the res id (no need to release)
		// Reflect
		virtual int get_material_res(graphics::Material* mat, int id = -1) = 0;
		// Reflect
		virtual void release_material_res(uint id) = 0;
		// Reflect
		virtual const MatRes& get_material_res_info(uint id) = 0;

		// id == -1 to register or to unregister id
		// Reflect
		virtual int register_light_instance(LightType type, int id) = 0;
		// Reflect
		virtual void set_dir_light_instance(uint id, const vec3& dir, const vec3& color) = 0;
		// Reflect
		virtual void set_pt_light_instance(uint id, const vec3& pos, const vec3& color, float range) = 0;

		// id == -1 to register or to unregister id
		// Reflect
		virtual int register_mesh_instance(int id) = 0;
		// Reflect
		virtual void set_mesh_instance(uint id, const mat4& mat, const mat3& nor) = 0;

		// id == -1 to register or to unregister id
		// Reflect
		virtual int register_armature_instance(int id) = 0;
		// Reflect
		virtual void set_armature_instance(uint id, const mat4* mats, uint size) = 0;

		// id == -1 to register or to unregister id
		// Reflect
		virtual int register_terrain_instance(int id) = 0;
		// Reflect
		virtual void set_terrain_instance(uint id, const mat4& mat, const vec3& extent, const uvec2& blocks, uint tess_level, uint grass_field_tess_level, uint grass_channel, int grass_texture_id,
			graphics::ImageViewPtr height_map, graphics::ImageViewPtr normal_map, graphics::ImageViewPtr tangent_map) = 0;

		// id == -1 to register or to unregister id
		// Reflect
		virtual int register_sdf_instance(int id) = 0;
		// Reflect
		virtual void set_sdf_instance(uint id, uint boxes_count, std::pair<vec3, vec3>* boxes, uint spheres_count, std::pair<vec3, float>* spheres) = 0;

		// id == -1 to register or to unregister id
		// Reflect
		virtual int register_volume_instance(int id) = 0;
		// Reflect
		virtual void set_volume_instance(uint id, const mat4& mat, const vec3& extent, const uvec3& blocks, graphics::ImageViewPtr data_map) = 0;

		// Reflect
		virtual void render(uint tar_idx, graphics::CommandBufferPtr cb) = 0;

		// Reflect
		virtual cNodePtr pick_up(const uvec2& screen_pos, vec3* out_pos = nullptr, const std::function<void(cNodePtr, DrawData&)>& draw_callback = {}) = 0;
		// Reflect
		virtual cElementPtr pick_up_2d(const uvec2& screen_pos) = 0;
		// Reflect
		virtual std::vector<vec3> transform_feedback(cNodePtr node) = 0;

		// Reflect
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
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
