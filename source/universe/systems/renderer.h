#pragma once

#include "../system.h"
#include "../draw_data.h"

namespace flame
{
	enum RenderMode
	{
		RenderModeSimple, // forward shading, no shadows, no post-processing
		RenderModeShaded,
		RenderModeCameraLight,
		RenderModeWireframe // forward shading, wireframe, no shadows, no post-processing
	};

	enum FogType
	{
		FogNone,
		FogLinear,
		FogExp,
		FogExp2,
		FogHeightLinear,
		FogHeightExp,
		FogHeightExp2
	};

	enum PrimitiveType
	{
		PrimitivePointList,
		PrimitiveLineList,
		PrimitiveLineStrip,
		PrimitiveTriangleList,
		PrimitiveTriangleStrip,
		PrimitiveQuadList
	};

	enum OutlineMode
	{
		OutlineMax,
		OutlineBox
	};

	enum HudLayoutType
	{
		HudVertical,
		HudHorizontal
	};

	enum HudStyleVar
	{
		HudStyleVarWindowPadding,
		HudStyleVarItemSpacing,
		HudStyleVarScaling,
		HudStyleVarCount
	};

	struct RenderTask
	{
		RenderMode mode = RenderModeShaded;
		cCameraPtr camera = nullptr;
		std::vector<graphics::ImageViewPtr> targets;
		graphics::ImageLayout final_layout = graphics::ImageLayoutShaderReadOnly;
		graphics::CanvasPtr canvas = nullptr;

		virtual ~RenderTask() {}
		virtual vec2 target_extent() const = 0;
		virtual void set_targets(const std::vector<graphics::ImageViewPtr>& targets) = 0;
	};

	// Reflect ctor
	struct sRenderer : System
	{
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
			std::vector<std::string> defines;
			std::vector<float> float_values;
			std::vector<int> int_values;
			std::unordered_map<uint, graphics::GraphicsPipelinePtr> pls;
			uint ref = 0;
		};

		graphics::WindowPtr window;
		std::vector<std::unique_ptr<RenderTaskT>> render_tasks;
		bool dirty = false;

		virtual RenderTaskPtr add_render_task(RenderMode mode, cCameraPtr camera, 
			const std::vector<graphics::ImageViewPtr>& targets, graphics::ImageLayout final_layout = 
			graphics::ImageLayoutShaderReadOnly, bool need_canvas = true, bool need_pickup = true) = 0;
		virtual RenderTaskPtr add_render_task_with_window_targets(RenderMode mode, cCameraPtr camera, 
			bool need_canvas = true, bool need_pickup = true) = 0;
		virtual void remove_render_task(RenderTaskPtr task) = 0;

		// Reflect
		graphics::ImageViewPtr sky_map = nullptr;
		// Reflect
		graphics::ImageViewPtr sky_irr_map = nullptr;
		// Reflect
		graphics::ImageViewPtr sky_rad_map = nullptr;
		// Reflect
		virtual void set_sky_maps(graphics::ImageViewPtr sky_map, graphics::ImageViewPtr sky_irr_map, graphics::ImageViewPtr sky_rad_map) = 0;
		// Reflect
		float sky_intensity = 1.f;
		// Reflect
		virtual void set_sky_intensity(float v) = 0;
		// Reflect
		virtual uint get_cel_shading_levels() const = 0;
		// Reflect
		virtual void set_cel_shading_levels(uint v) = 0;
		// Reflect
		FogType fog_type = FogLinear;
		// Reflect
		virtual void set_fog_type(FogType type) = 0;
		// Reflect
		float fog_density = 1.f;
		// Reflect
		virtual void set_fog_density(float v) = 0;
		// Reflect
		float fog_start = 10.f;
		// Reflect
		virtual void set_fog_start(float v) = 0;
		// Reflect
		float fog_end = 1000.f;
		// Reflect
		virtual void set_fog_end(float v) = 0;
		// Reflect
		float fog_base_height = 0.f;
		// Reflect
		virtual void set_fog_base_height(float v) = 0;
		// Reflect
		float fog_max_height = 10.f;
		// Reflect
		virtual void set_fog_max_height(float v) = 0;
		// Reflect
		vec3 fog_color = vec3(1.f);
		// Reflect
		virtual void set_fog_color(const vec3& color) = 0;
		// Reflect
		float shadow_distance = 100.f;
		// Reflect
		virtual void set_shadow_distance(float d) = 0;
		// Reflect
		uint csm_levels = 2;
		// Reflect
		virtual void set_csm_levels(uint lv) = 0;
		// Reflect
		float esm_factor = 30.f;
		// Reflect
		virtual void set_esm_factor(float f) = 0;
		// Reflect
		float shadow_bleeding_reduction = 0.95f;
		// Reflect
		virtual void set_shadow_bleeding_reduction(float f) = 0;
		// Reflect
		float shadow_darkening = 0.1f;
		// Reflect
		virtual void set_shadow_darkening(float f) = 0;
		// Reflect
		bool post_processing_enable = true;
		// Reflect
		virtual void set_post_processing_enable(bool v) = 0;
		// Reflect
		bool outline_pp_enable = false; // post-processing outline
		// Reflect
		virtual void set_outline_pp_enable(bool v) = 0;
		// Reflect
		uint outline_pp_width = 1;
		// Reflect
		virtual void set_outline_pp_width(uint v) = 0;
		// Reflect
		float outline_pp_depth_scale = 1.f;
		// Reflect
		virtual void set_outline_pp_depth_scale(float v) = 0;
		// Reflect
		float outline_pp_normal_scale = 1.f;
		// Reflect
		virtual void set_outline_pp_normal_scale(float v) = 0;
		// Reflect
		vec3 outline_pp_color = vec3(1.f);
		// Reflect
		virtual void set_outline_pp_color(const vec3& col) = 0;
		// Reflect
		bool ssao_enable = true;
		// Reflect
		virtual void set_ssao_enable(bool v) = 0;
		// Reflect
		float ssao_radius = 0.5f;
		// Reflect
		virtual void set_ssao_radius(float v) = 0;
		// Reflect
		float ssao_bias = 0.025f;
		// Reflect
		virtual void set_ssao_bias(float v) = 0;
		// Reflect
		float white_point = 4.f;
		// Reflect
		virtual void set_white_point(float v) = 0;
		// Reflect
		bool bloom_enable = true;
		// Reflect
		virtual void set_bloom_enable(bool v) = 0;
		// Reflect
		bool dof_enable = false;
		// Reflect
		virtual void set_dof_enable(bool v) = 0;
		// Reflect
		float dof_focus_point = 50.f;
		// Reflect
		virtual void set_dof_focus_point(float v) = 0;
		// Reflect
		float dof_focus_scale = 20.f;
		// Reflect
		virtual void set_dof_focus_scale(float v) = 0;
		// Reflect
		bool ssr_enable = true;
		// Reflect
		virtual void set_ssr_enable(bool v) = 0;
		// Reflect
		float ssr_thickness = 0.5f;
		// Reflect
		virtual void set_ssr_thickness(float v) = 0;
		// Reflect
		float ssr_max_distance = 8.f;
		// Reflect
		virtual void set_ssr_max_distance(float v) = 0;
		// Reflect
		uint ssr_max_steps = 64;
		// Reflect
		virtual void set_ssr_max_steps(uint v) = 0;
		// Reflect
		uint ssr_binary_search_steps = 5;
		// Reflect
		virtual void set_ssr_binary_search_steps(uint v) = 0;
		// Reflect
		bool tone_mapping_enable = true;
		// Reflect
		virtual void set_tone_mapping_enable(bool v) = 0;
		// Reflect
		float gamma = 1.5f;
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
		virtual void set_mesh_instance(uint id, const mat4& mat, const mat3& nor, const cvec4& col) = 0;

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
		virtual void draw_outlines(const std::vector<ObjectDrawData>& draw_datas, const cvec4& color, uint width, OutlineMode mode = OutlineMax) = 0;
		// Reflect
		virtual void draw_primitives(PrimitiveType type, const vec3* points, uint count, const cvec4& color, bool depth_test = false) = 0;
		// Reflect
		virtual void draw_particles(uint mat_id, const std::vector<ParticleDrawData::Ptc>& ptcs) = 0;

		// Reflect
		virtual void render(int tar_idx, graphics::CommandBufferPtr cb) = 0;

		// Reflect
		virtual cNodePtr pick_up(const uvec2& screen_pos, vec3* out_pos = nullptr, const std::function<void(cNodePtr, DrawData&)>& draw_callback = {}) = 0;
		// Reflect
		virtual cElementPtr pick_up_2d(const uvec2& screen_pos) = 0;
		// Reflect
		virtual std::vector<vec3> transform_feedback(cNodePtr node) = 0;
		// Reflect
		virtual graphics::ImagePtr get_image(uint name) = 0;

		Listeners<void()> hud_callbacks;

		virtual void hud_begin(uint id, const vec2& pos, const vec2& size = vec2(0.f) /* 0 size means auto layout */, const cvec4& col = cvec4(0, 0, 0, 255), const vec2& pivot = vec2(0.f),
			const graphics::ImageDesc& image = {}, float image_scale = 1.f) = 0;
		virtual void hud_end() = 0;
		virtual void hud_set_cursor(const vec2& pos) = 0;
		virtual Rect hud_get_rect() const = 0;
		virtual vec2 hud_screen_size() const = 0;
		virtual void hud_push_style(HudStyleVar var, const vec2& value) = 0;
		virtual void hud_pop_style(HudStyleVar var) = 0;
		virtual void hud_begin_layout(HudLayoutType type) = 0;
		virtual void hud_end_layout() = 0;
		virtual void hud_new_line() = 0;
		virtual void hud_begin_stencil_write() = 0;
		virtual void hud_end_stencil_write() = 0;
		virtual void hud_begin_stencil_compare() = 0;
		virtual void hud_end_stencil_compare() = 0;
		virtual void hud_rect(const vec2& size, const cvec4& col) = 0;
		virtual void hud_text(std::wstring_view text, uint font_size = 24, const cvec4& col = cvec4(255)) = 0;
		virtual void hud_image(const vec2& size, const graphics::ImageDesc& image, float image_scale = 1.f, const cvec4& col = cvec4(255)) = 0;
		virtual bool hud_button(std::wstring_view label, uint font_size = 24, const graphics::ImageDesc& image = {}, float image_scale = 1.f, bool* p_hovered = nullptr) = 0;

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
