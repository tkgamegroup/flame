#pragma once

#include "renderer.h"

#include "../../graphics/image.h"
#include "../../graphics/renderpass.h"
#include "../../graphics/shader.h"
#include "../../graphics/extension.h"
#include "../../graphics/canvas.h"

namespace flame
{
	struct RenderTaskPrivate : RenderTask
	{
		graphics::WindowPtr bound_window = nullptr;

		std::unique_ptr<graphics::Image>			img_back0;
		std::unique_ptr<graphics::Image>			img_back1;
		std::unique_ptr<graphics::Image>			img_dst;
		std::unique_ptr<graphics::Image>			img_dep;
		std::unique_ptr<graphics::Image>			img_dst_ms;
		std::unique_ptr<graphics::Image>			img_dep_ms;
		std::unique_ptr<graphics::Image>			img_last_dst;
		std::unique_ptr<graphics::Image>			img_last_dep;
		std::unique_ptr<graphics::Image>			img_gbufferA;	// color
		std::unique_ptr<graphics::Image>			img_gbufferB;	// normal
		std::unique_ptr<graphics::Image>			img_gbufferC;	// metallic, roughness, ao, flags
		std::unique_ptr<graphics::Image>			img_gbufferD;	// emissive

		std::unique_ptr<graphics::Framebuffer>		fb_fwd;
		std::unique_ptr<graphics::Framebuffer>		fb_fwd_clear;
		std::unique_ptr<graphics::Framebuffer>		fb_gbuf;
		std::unique_ptr<graphics::Framebuffer>		fb_primitive;

		graphics::PipelineResourceManager			prm_plain;
		graphics::StorageBuffer						buf_camera;
		std::unique_ptr<graphics::DescriptorSet>	ds_camera;
		std::unique_ptr<graphics::DescriptorSet>	ds_target;
		graphics::PipelineResourceManager			prm_fwd;
		graphics::PipelineResourceManager			prm_gbuf;
		graphics::PipelineResourceManager			prm_deferred;
		std::unique_ptr<graphics::DescriptorSet>	ds_deferred;
		graphics::PipelineResourceManager			prm_outline_pp;
		std::vector<std::unique_ptr<graphics::DescriptorSet>>	
													dss_outline_pp;
		graphics::PipelineResourceManager			prm_box;
		graphics::PipelineResourceManager			prm_bloom;
		graphics::PipelineResourceManager			prm_blur;
		std::unique_ptr<graphics::DescriptorSet>	ds_dof;
		graphics::PipelineResourceManager			prm_dof;
		std::unique_ptr<graphics::DescriptorSet>	ds_luma;
		graphics::PipelineResourceManager			prm_luma;
		graphics::PipelineResourceManager			prm_tone;
		graphics::PipelineResourceManager			prm_fxaa;

		std::unique_ptr<graphics::Image>			img_pickup;
		std::unique_ptr<graphics::Image>			img_dep_pickup;
		std::unique_ptr<graphics::Framebuffer>		fb_pickup;
		std::unique_ptr<graphics::Fence>			fence_pickup;

		void init();
		vec2 target_extent() const override;
		void set_targets(const std::vector<graphics::ImageViewPtr>& targets) override;
	};

	struct OutlineDrawGroup
	{
		std::vector<ObjectDrawData> draws;
		cvec4 color;
		uint width;
		OutlineMode mode;
	};

	struct PrimitivesDraw
	{
		PrimitiveType type;
		uint vtx_cnt;
		cvec4 color;
		bool depth_test;
	};

	struct ParticlesDraw
	{
		uint mat_id;
		uint vtx_off;
		uint vtx_cnt;
	};

	struct HudLayout
	{
		HudLayoutType type;
		Rect rect;
		vec2 cursor;
		vec2 pivot;
		vec2 item_spacing;
		vec4 border;
		vec2 item_max;
		bool auto_size;
	};

	struct Hud
	{
		vec2 pos;
		vec2 size;
		cvec4 color;
		vec2 pivot;
		vec4 border;
		bool stencil;
		graphics::Canvas::DrawVert* bg_verts;
		uint bg_vert_count;
		int translate_cmd_idx;

		std::vector<HudLayout> layouts;
	};

	struct sRendererPrivate : sRenderer
	{
		bool mark_clear_pipelines = false;

		std::vector<OutlineDrawGroup> outline_groups;
		std::vector<PrimitivesDraw> primitives_draws;
		std::vector<ParticlesDraw> particles_draws;

		std::vector<std::stack<vec2>> hud_style_vars;
		std::vector<std::stack<cvec4>> hud_style_colors;
		std::vector<Hud> huds;
		Rect hud_last_rect;

		sRendererPrivate(int); // dummy for reflection
		sRendererPrivate();
		~sRendererPrivate();

		RenderTaskPtr add_render_task(RenderMode mode, cCameraPtr camera, graphics::WindowPtr window = nullptr,
			const std::vector<graphics::ImageViewPtr>& targets = {}, graphics::ImageLayout final_layout =
			graphics::ImageLayoutShaderReadOnly, bool need_canvas = true, bool need_pickup = true) override;
		void remove_render_task(RenderTaskPtr task) override;

		RenderMode get_render_mode() override;
		void set_render_mode(RenderMode mode) override;

		graphics::ImageViewPtr sky_map = nullptr;
		graphics::ImageViewPtr sky_irr_map = nullptr;
		graphics::ImageViewPtr sky_rad_map = nullptr;
		float sky_rad_levels = 1.f;
		void set_sky_maps(graphics::ImageViewPtr sky_map, graphics::ImageViewPtr sky_irr_map, graphics::ImageViewPtr sky_rad_map) override;
		void set_sky_intensity(float v) override;
		uint get_cel_shading_levels() const override;
		void set_cel_shading_levels(uint v) override;
		void set_fog_type(FogType type) override;
		void set_fog_density(float v) override;
		void set_fog_start(float v) override;
		void set_fog_end(float v) override;
		void set_fog_base_height(float v) override;
		void set_fog_max_height(float v) override;
		void set_fog_color(const vec3& color) override;
		void set_shadow_distance(float d) override;
		void set_csm_levels(uint lv) override;
		void set_esm_factor(float f) override;
		void set_shadow_bleeding_reduction(float f) override;
		void set_shadow_darkening(float f) override;
		void set_post_processing_enable(bool v) override;
		void set_outline_pp_enable(bool v) override;
		void set_outline_pp_width(uint v) override;
		void set_outline_pp_depth_scale(float v) override;
		void set_outline_pp_normal_scale(float v) override;
		void set_outline_pp_color(const vec3& col) override;
		void set_ssao_enable(bool v) override;
		void set_ssao_radius(float v) override;
		void set_ssao_bias(float v) override;
		void set_white_point(float v) override;
		void set_bloom_enable(bool v) override;
		void set_dof_enable(bool v) override;
		void set_dof_focus_point(float v) override;
		void set_dof_focus_scale(float v) override;
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

		void update_mat_res(uint id, bool update_parameters = true, bool update_textures = true, bool update_pipeline = true);
		int get_material_res(graphics::Material* mat, int id) override;
		void release_material_res(uint id) override;
		const MatRes& get_material_res_info(uint id) override;

		int register_light_instance(LightType type, int id) override;
		void set_dir_light_instance(uint id, const vec3& dir, const vec3& color) override;
		void set_pt_light_instance(uint id, const vec3& pos, const vec3& color, float range) override;

		int register_mesh_instance(int id) override;
		void set_mesh_instance(uint id, const mat4& mat, const mat3& nor, const cvec4& col) override;

		int register_armature_instance(int id) override;
		void set_armature_instance(uint id, const mat4* mats, uint size) override;

		int register_terrain_instance(int id) override;
		void set_terrain_instance(uint id, const mat4& mat, const vec3& extent, const uvec2& blocks, uint tess_level, uint grass_field_tess_level, uint grass_channel, int grass_texture_id,
			graphics::ImageViewPtr height_map, graphics::ImageViewPtr normal_map, graphics::ImageViewPtr tangent_map) override;

		int register_sdf_instance(int id) override;
		void set_sdf_instance(uint id, uint boxes_count, std::pair<vec3, vec3>* boxes, uint spheres_count, std::pair<vec3, float>* spheres) override;

		int register_volume_instance(int id) override;
		void set_volume_instance(uint id, const mat4& mat, const vec3& extent, const uvec3& blocks, graphics::ImageViewPtr data_map) override;

		void draw_outlines(const std::vector<ObjectDrawData>& draw_datas, const cvec4& color, uint width, OutlineMode mode) override;
		void draw_primitives(PrimitiveType type, const vec3* points, uint count, const cvec4& color, bool depth_test) override;
		void draw_particles(uint mat_id, const std::vector<ParticleDrawData::Ptc>& ptcs) override;

		void render(graphics::CommandBufferPtr cb) override;

		void update() override;

		cNodePtr pick_up(const uvec2& screen_pos, vec3* out_pos, const std::function<void(cNodePtr, DrawData&)>& draw_callback) override;
		cElementPtr pick_up_2d(const uvec2& screen_pos) override;
		std::vector<vec3> transform_feedback(cNodePtr node) override;
		graphics::ImagePtr get_image(uint name) override;

		HudLayout& hud_add_layout(HudLayoutType type);
		void hud_finish_layout(HudLayout& layout);

		void hud_begin(const vec2& pos, const vec2& size, const cvec4& col, const vec2& pivot, const graphics::ImageDesc& image, const vec4& border) override;
		void hud_end() override;
		void hud_set_cursor(const vec2& pos) override;
		Rect hud_hud_rect() const override;
		Rect hud_item_rect() const override;
		vec2 hud_screen_size() const override;
		void hud_push_style_var(HudStyleVar idx, const vec2& value) override;
		void hud_pop_style_var(HudStyleVar idx) override;
		void hud_push_style_color(HudStyleColor idx, const cvec4& color) override;
		void hud_pop_style_color(HudStyleColor idx) override;
		void hud_begin_layout(HudLayoutType type, const vec2& size, const vec2& item_spacing, const vec4& border) override;
		void hud_end_layout() override;
		void hud_new_line() override;
		void hud_begin_stencil_write() override;
		void hud_end_stencil_write() override;
		void hud_begin_stencil_compare() override;
		void hud_end_stencil_compare() override;
		Rect hud_add_rect(const vec2& sz);
		void hud_rect(const vec2& size, const cvec4& col) override;
		void hud_text(std::wstring_view text, uint font_size, const cvec4& col) override;
		void hud_image(const vec2& size, const graphics::ImageDesc& image, const cvec4& col) override;
		void hud_image_stretched(const vec2& size, const graphics::ImageDesc& image, const vec4& border, const cvec4& col) override;
		void hud_image_rotated(const vec2& size, const graphics::ImageDesc& image, const cvec4& col, float angle) override;
		bool hud_button(std::wstring_view label, uint font_size) override;
		bool hud_image_button(const vec2& size, const graphics::ImageDesc& image, const vec4& border) override;
		void hud_stroke_item(float thickness, const cvec4& col) override;
		bool hud_item_hovered() override;
		bool hud_item_clicked() override;

		void send_debug_string(const std::string& str) override;
	};
}
