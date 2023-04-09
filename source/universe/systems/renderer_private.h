#pragma once

#include "renderer.h"

#include "../../graphics/image.h"
#include "../../graphics/renderpass.h"
#include "../../graphics/shader.h"
#include "../../graphics/extension.h"

namespace flame
{
	struct sRendererPrivate : sRenderer
	{
		graphics::ImageLayout final_layout;

		sRendererPrivate();
		sRendererPrivate(graphics::WindowPtr w);

		void set_targets(std::span<graphics::ImageViewPtr> targets, graphics::ImageLayout final_layout) override;
		void bind_window_targets() override;
		vec2 target_extent() override;

		graphics::ImageViewPtr sky_map = nullptr;
		graphics::ImageViewPtr sky_irr_map = nullptr;
		graphics::ImageViewPtr sky_rad_map = nullptr;
		float sky_rad_levels = 1.f;
		void set_sky_maps(graphics::ImageViewPtr sky_map, graphics::ImageViewPtr sky_irr_map, graphics::ImageViewPtr sky_rad_map) override;
		void set_sky_intensity(float v) override;
		void set_fog_color(const vec3& color) override;
		void set_shadow_distance(float d) override;
		void set_csm_levels(uint lv) override;
		void set_esm_factor(float f) override;
		void set_post_processing_enable(bool v) override;
		void set_ssao_enable(bool v) override;
		void set_ssao_radius(float v) override;
		void set_ssao_bias(float v) override;
		void set_white_point(float v) override;
		void set_bloom_enable(bool v) override;
		void set_ssr_enable(bool v) override;
		void set_ssr_thickness(float v) override;
		void set_ssr_max_distance(float v) override;
		void set_ssr_max_steps(uint v) override;
		void set_ssr_binary_search_steps(uint v) override;
		void set_tone_mapping_enable(bool v) override;
		void set_gamma(float v) override;

		int get_mat_var(int id, const std::string& name) override;
		void release_mat_var(uint id) override;
		const MatVar& get_mat_var_info(uint id) override;
		void set_mat_var(uint id, const vec4& v) override;

		std::filesystem::path get_post_shading_code_file() override;
		void set_post_shading_code_file(const std::filesystem::path& path) override;

		int get_texture_res(graphics::ImageViewPtr iv, graphics::SamplerPtr sp, int id) override;
		void release_texture_res(uint id) override;
		const TexRes& get_texture_res_info(uint id) override;
		void set_texture_res_name(uint id, const std::string& name) override;

		int get_mesh_res(graphics::MeshPtr mesh, int id) override;
		void release_mesh_res(uint id) override;
		const MeshRes& get_mesh_res_info(uint id) override;

		void update_mat_res(uint id, bool update_parameters = true, bool update_textures = true, bool update_pipelines = true);
		int get_material_res(graphics::Material* mat, int id) override;
		void release_material_res(uint id) override;
		const MatRes& get_material_res_info(uint id) override;

		int register_light_instance(LightType type, int id) override;
		void set_dir_light_instance(uint id, const vec3& dir, const vec3& color) override;
		void set_pt_light_instance(uint id, const vec3& pos, const vec3& color, float range) override;

		int register_mesh_instance(int id) override;
		void set_mesh_instance(uint id, const mat4& mat, const mat3& nor) override;

		int register_armature_instance(int id) override;
		void set_armature_instance(uint id, const mat4* mats, uint size) override;

		int register_terrain_instance(int id) override;
		void set_terrain_instance(uint id, const mat4& mat, const vec3& extent, const uvec2& blocks, uint tess_level, uint grass_field_tess_level, uint grass_channel, int grass_texture_id,
			graphics::ImageViewPtr height_map, graphics::ImageViewPtr normal_map, graphics::ImageViewPtr tangent_map) override;

		int register_sdf_instance(int id) override;
		void set_sdf_instance(uint id, uint boxes_count, std::pair<vec3, vec3>* boxes, uint spheres_count, std::pair<vec3, float>* spheres) override;

		int register_volume_instance(int id) override;
		void set_volume_instance(uint id, const mat4& mat, const vec3& extent, const uvec3& blocks, graphics::ImageViewPtr data_map) override;

		void draw_outline(uint type, uint res_id, uint ins_id, const cvec4& color, uint width, uint mode, bool new_group) override;
		void draw_primitives(uint type, const vec3* points, uint count, const cvec4& color, bool depth_test) override;

		void render(uint tar_idx, graphics::CommandBufferPtr cb) override;

		void update() override;

		cNodePtr pick_up(const uvec2& screen_pos, vec3* out_pos, const std::function<void(cNodePtr, DrawData&)>& draw_callback) override;
		cElementPtr pick_up_2d(const uvec2& screen_pos) override;
		std::vector<vec3> transform_feedback(cNodePtr node) override;

		void send_debug_string(const std::string& str) override;
	};
}
