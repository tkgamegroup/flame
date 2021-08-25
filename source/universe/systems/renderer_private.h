#pragma once

#include "../../graphics/command.h"
#include "renderer.h"

namespace flame
{
	enum MaterialUsage
	{
		MaterialForMesh,
		MaterialForMeshArmature,
		MaterialForMeshShadow,
		MaterialForMeshShadowArmature,
		MaterialMeshUsageCount,
		MaterialForTerrain = MaterialMeshUsageCount,
		MaterialForWater,

		MaterialUsageCount
	};

	inline uint const ArmatureMaxBones = 128;

	struct ElemenetRenderData;
	struct NodeRenderData;

	struct sRendererPrivate : sRenderer
	{
		bool always_update = false;

		graphics::Device* device;
		graphics::DescriptorPool* dsp;
		graphics::Sampler* sp_nearest;
		graphics::Sampler* sp_linear;
		graphics::Sampler* sp_tri_linear;

		graphics::Renderpass* rp_rgba8;
		graphics::Renderpass* rp_rgba8c;
		graphics::Renderpass* rp_bgra8;
		graphics::Renderpass* rp_bgra8c;
		std::vector<graphics::ImageView*> iv_tars;
		std::vector<UniPtr<graphics::Framebuffer>> fb_tars;
		uvec2 tar_sz;

		UniPtr<graphics::Image> img_white;
		UniPtr<graphics::Image> img_black;
		UniPtr<graphics::Image> img_black_cube;
		UniPtr<graphics::Image> img_dst;

		cCameraPrivate* camera = nullptr;

		RenderType render_type = RenderShaded;

		void* sky_id = nullptr;

		uint frame;
		std::unique_ptr<ElemenetRenderData>	_ed;
		std::unique_ptr<NodeRenderData>		_nd;

		bool dirty = true;

		sRendererPrivate(sRendererParms* parms);
		~sRendererPrivate();

		void set_always_update(bool a) override { always_update = a; }
		void set_render_type(RenderType type) override { render_type = type; }
		void get_shadow_props(uint* dir_levels, float* dir_dist, float* pt_dist) override;
		void set_shadow_props(uint dir_levels, float dir_dist, float pt_dist) override;

		graphics::ImageView* get_element_res(uint idx) const override;
		int set_element_res(int idx, graphics::ImageView* iv) override;
		int find_element_res(graphics::ImageView* iv) const override;

		void fill(uint layer, uint pt_cnt, const vec2* pts, const cvec4& color) override;
		void stroke(uint layer, uint pt_cnt, const vec2* pts, float thickness, const cvec4& color, bool closed) override;
		void draw_glyphs(uint layer, uint cnt, const graphics::GlyphDraw* glyphs, uint res_id, const cvec4& color) override;
		void draw_image(uint layer, const vec2* pts, uint res_id, const vec4& uvs, const cvec4& tint_color) override;

		int set_texture_res(int idx, graphics::ImageView* tex, graphics::Sampler* sp) override;
		int find_texture_res(graphics::ImageView* tex) const override;

		int set_material_res(int idx, graphics::Material* mat) override;
		int find_material_res(graphics::Material* mat) const override;

		int set_mesh_res(int idx, graphics::Mesh* mesh) override;
		int find_mesh_res(graphics::Mesh* mesh) const override;

		graphics::Pipeline* get_material_pipeline(MaterialUsage usage, const std::filesystem::path& mat, std::vector<std::string> defines);
		void release_material_pipeline(MaterialUsage usage, graphics::Pipeline* pl);

		cCameraPtr get_camera() const override { return camera; }
		void set_camera(cCameraPtr c) override { camera = c; }

		void* get_sky_id() override { return sky_id; }
		void set_sky(graphics::ImageView* box, graphics::ImageView* irr,
			graphics::ImageView* rad, graphics::ImageView* lut, const vec3& fog_color, float intensity, void* id) override;

		uint add_light(const mat4& mat, LightType type, const vec3& color, bool cast_shadow) override;
		mat4 get_shaodw_mat(uint id, uint idx) const;
		uint add_mesh_transform(const mat4& mat, const mat3& nor) override;
		uint add_mesh_armature(uint bones_count, const mat4* bones) override;
		void draw_mesh(uint idx, uint mesh_id, uint skin, ShadingFlags flags) override;
		void draw_mesh_occluder(uint idx, uint mesh_id, uint skin) override;
		void draw_terrain(const vec3& coord, const vec3& extent, const uvec2& blocks, uint tess_levels, uint height_map_id, 
			uint normal_map_id, uint tangent_map_id, uint material_id, ShadingFlags flags) override;
		void draw_water(const vec3& coord, const vec2& extent,
			uint material_id, ShadingFlags flags) override;
		void draw_particles(uint count, graphics::Particle* partcles, uint res_id) override;

		void draw_lines(uint count, graphics::Line* lines) override;

		bool is_dirty() const override { return always_update || dirty; }
		void mark_dirty() override { dirty = true; }

		uint element_render(uint layer, cElementPrivate* element);
		void node_render(cNodePrivate* node, Frustum* lod_frustums);

		void set_targets(uint tar_cnt, graphics::ImageView* const* ivs) override;
		void record(uint tar_idx, graphics::CommandBuffer* cb) override;

		void on_added() override;

		void update() override;
	};
}
