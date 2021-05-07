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

		MaterialUsageCount
	};

	struct ElemnetRenderData;
	struct NodeRenderData;

	struct sRendererPrivate : sRenderer
	{
		bool wireframe = false;
		bool always_update = false;

		graphics::Device* device;
		graphics::DescriptorPool* dsp;
		graphics::Sampler* sp_nearest;
		graphics::Sampler* sp_linear;

		graphics::Renderpass* rp_rgba8c;
		graphics::Renderpass* rp_rgba8;
		std::vector<UniPtr<graphics::Framebuffer>> fb_tars;
		vec2 tar_sz;

		UniPtr<graphics::Image> img_wht;
		UniPtr<graphics::Image> img_back;
		UniPtr<graphics::DescriptorSet>	ds_back;

		cCameraPrivate* camera = nullptr;

		std::unique_ptr<ElemnetRenderData>	_ed;
		std::unique_ptr<NodeRenderData>		_nd;

		bool dirty = true;

		sRendererPrivate(sRendererParms* parms);

		void set_shade_wireframe(bool v) override { wireframe = v; }
		void set_always_update(bool a) override { always_update = a; }

		void* get_element_res(uint idx, char* type) const override;
		int set_element_res(int idx, const char* type, void* v) override;
		int find_element_res(void* v) const override;

		void fill_rect(uint layer, cElementPtr element, const vec2& pos, const vec2& size, const cvec4& color) override;
		void stroke_rect(uint layer, cElementPtr element, const vec2& pos, const vec2& size, float thickness, const cvec4& color) override;
		void draw_text(uint layer, cElementPtr element, const vec2& pos, uint font_size, uint font_id, const wchar_t* text_beg, const wchar_t* text_end, const cvec4& color) override;

		int set_texture_res(int idx, graphics::ImageView* tex, graphics::Sampler* sp) override;
		int find_texture_res(graphics::ImageView* tex) const override;

		int set_material_res(int idx, graphics::Material* mat) override;
		int find_material_res(graphics::Material* mat) const override;

		int set_mesh_res(int idx, graphics::model::Mesh* mesh) override;
		int find_mesh_res(graphics::model::Mesh* mesh) const override;

		graphics::Pipeline* get_material_pipeline(MaterialUsage usage, const std::filesystem::path& mat, const std::string& defines);
		void release_material_pipeline(MaterialUsage usage, graphics::Pipeline* pl);

		cCameraPtr get_camera() const override { return camera; }
		void set_camera(cCameraPtr c) override { camera = c; }

		void add_light(cNodePtr node, LightType type, const vec3& color, bool cast_shadow) override;
		void draw_mesh(cNodePtr node, uint mesh_id, bool cast_shadow) override;
		void draw_terrain(cNodePtr node, const uvec2& blocks, uint tess_levels, uint height_map_id, uint normal_map_id, uint material_id) override;

		bool is_dirty() const override { return always_update || dirty; }
		void mark_dirty() override { dirty = true; }

		uint element_render(uint layer, cElementPrivate* element);
		void node_render(cNodePrivate* node);

		void set_targets(uint tar_cnt, graphics::ImageView* const* ivs) override;
		void record(uint tar_idx, graphics::CommandBuffer* cb) override;

		void on_added() override;

		void update() override;
	};
}
