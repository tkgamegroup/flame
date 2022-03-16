#pragma once

#include "renderer.h"

#include "../../graphics/image.h"
#include "../../graphics/renderpass.h"
#include "../../graphics/shader.h"
#include "../../graphics/extension.h"

namespace flame
{
	struct MeshRes
	{
		graphics::Mesh* mesh = nullptr;
		bool arm;
		uint vtx_off;
		uint vtx_cnt;
		uint idx_off;
		uint idx_cnt;
		uint ref = 0;
	};

	struct TexRes
	{
		std::filesystem::path filename;
		uint hash = 0;
		std::unique_ptr<graphics::Image> img;
		graphics::SamplerPtr sp = nullptr;
		uint ref = 0;
	};

	struct MatRes
	{
		graphics::Material* mat = nullptr;
		std::vector<int> texs;
		std::unordered_map<uint, graphics::GraphicsPipelinePtr> pls;
		std::vector<uint> draw_ids;
		uint ref = 0;
	};

	struct DrawLine
	{
		cNodePtr node;
		uint offset;
		uint count;
		cvec4 color;
	};

	struct DrawMesh
	{
		cNodePtr node;
		uint ins_id;
		uint mesh_id;
		uint mat_id;
		cvec4 color;
	};

	struct DrawTerrain
	{
		cNodePtr node;
		uint ins_id;
		uint blocks;
		uint mat_id;
		cvec4 color;
	};

	struct sRendererPrivate : sRenderer
	{
		graphics::WindowPtr window;

		std::vector<MeshRes> mesh_reses;
		std::vector<TexRes>	tex_reses;
		std::vector<MatRes> mat_reses;

		cNodePtr current_node = nullptr;
		std::vector<DrawLine>		draw_lines;
		std::vector<DrawMesh>		draw_meshes;
		std::vector<DrawMesh>		draw_arm_meshes;
		std::vector<DrawMesh>		draw_occluder_meshes;
		std::vector<DrawMesh>		draw_occluder_arm_meshes;
		std::vector<DrawMesh>		draw_outline_meshes;
		std::vector<DrawMesh>		draw_outline_arm_meshes;
		std::vector<DrawMesh>		draw_wireframe_meshes;
		std::vector<DrawMesh>		draw_wireframe_arm_meshes;
		std::vector<DrawTerrain>	draw_terrains;
		std::vector<DrawTerrain>	draw_outline_terrains;
		std::vector<DrawTerrain>	draw_wireframe_terrains;
		std::vector<uint> opaque_draw_meshes;
		std::vector<uint> opaque_draw_arm_meshes;
		std::vector<uint> transparent_draw_meshes;
		std::vector<uint> transparent_draw_arm_meshes;

		std::vector<graphics::ImageViewPtr> iv_tars;

		std::unique_ptr<graphics::Image>												img_black;
		std::unique_ptr<graphics::Image>												img_white;

		graphics::RenderpassPtr															rp_col = nullptr;
		graphics::RenderpassPtr															rp_col_dep = nullptr;
		graphics::RenderpassPtr															rp_fwd = nullptr;
		graphics::RenderpassPtr															rp_gbuf = nullptr;
		graphics::PipelineLayoutPtr														pll_fwd = nullptr;
		graphics::PipelineLayoutPtr														pll_gbuf = nullptr;
		graphics::GraphicsPipelinePtr													pl_blit = nullptr;
		graphics::GraphicsPipelinePtr													pl_blit_tar = nullptr;
		graphics::GraphicsPipelinePtr													pl_add = nullptr;
		graphics::GraphicsPipelinePtr													pl_blend = nullptr;

		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageVertex>					buf_lines;
		graphics::PipelineResourceManager<FLAME_UID>									prm_plain3d;
		graphics::GraphicsPipelinePtr													pl_line3d = nullptr;

		std::unique_ptr<graphics::Image>												img_dst;
		std::unique_ptr<graphics::Image>												img_dep;
		std::unique_ptr<graphics::Image>												img_col_met;	// color, metallic
		std::unique_ptr<graphics::Image>												img_nor_rou;	// normal, roughness
		std::unique_ptr<graphics::Image>												img_ao;		// ambient occlusion
		std::unique_ptr<graphics::Framebuffer>											fb_fwd;
		std::unique_ptr<graphics::Framebuffer>											fb_gbuf;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_mesh_ins;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_armature_ins;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_terrain_ins;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_material;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageUniform, false>			buf_scene;
		std::unique_ptr<graphics::DescriptorSet>										ds_scene;
		std::unique_ptr<graphics::DescriptorSet>										ds_instance;
		std::unique_ptr<graphics::DescriptorSet>										ds_material;
		std::unique_ptr<graphics::DescriptorSet>										ds_light;
		graphics::PipelineResourceManager<FLAME_UID>									prm_fwd;
		graphics::PipelineResourceManager<FLAME_UID>									prm_gbuf;
		graphics::GraphicsPipelinePtr													pl_mesh_plain = nullptr;
		graphics::GraphicsPipelinePtr													pl_mesh_arm_plain = nullptr;
		graphics::GraphicsPipelinePtr													pl_terrain_plain = nullptr;
		graphics::GraphicsPipelinePtr													pl_mesh_camlit = nullptr;
		graphics::GraphicsPipelinePtr													pl_mesh_arm_camlit = nullptr;
		graphics::GraphicsPipelinePtr													pl_terrain_camlit = nullptr;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageVertex, false>			buf_vtx;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageIndex, false>			buf_idx;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageVertex, false>			buf_vtx_arm;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageIndex, false>			buf_idx_arm;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageIndirect>				buf_idr_mesh;
		graphics::PipelineResourceManager<FLAME_UID>									prm_deferred;
		std::unique_ptr<graphics::DescriptorSet>										ds_deferred;
		graphics::GraphicsPipelinePtr													pl_deferred = nullptr;

		std::unique_ptr<graphics::Image>												img_back0;
		std::unique_ptr<graphics::Image>												img_back1;

		graphics::PipelineResourceManager<FLAME_UID>									prm_post;
		graphics::GraphicsPipelinePtr													pl_blur_h = nullptr;
		graphics::GraphicsPipelinePtr													pl_blur_v = nullptr;
		graphics::GraphicsPipelinePtr													pl_localmax_h = nullptr;
		graphics::GraphicsPipelinePtr													pl_localmax_v = nullptr;

		std::unique_ptr<graphics::Image>												img_pickup;
		std::unique_ptr<graphics::Image>												img_dep_pickup;
		std::unique_ptr<graphics::Framebuffer>											fb_pickup;
		graphics::GraphicsPipelinePtr													pl_mesh_pickup = nullptr;
		graphics::GraphicsPipelinePtr													pl_mesh_arm_pickup = nullptr;
		graphics::GraphicsPipelinePtr													pl_terrain_pickup = nullptr;
		std::unique_ptr<graphics::Fence>												fence_pickup;

		graphics::ImageLayout final_layout;

		sRendererPrivate() {}
		sRendererPrivate(graphics::WindowPtr w);

		void set_targets(std::span<graphics::ImageViewPtr> targets, graphics::ImageLayout final_layout) override;
		void bind_window_targets() override;

		int get_texture_res(const std::filesystem::path& filename, bool srgb, 
			graphics::SamplerPtr sp, const graphics::MipmapOption& mipmap_option) override;
		void release_texture_res(uint id) override;

		int get_mesh_res(graphics::Mesh* mesh) override;
		void release_mesh_res(uint id) override;

		int get_material_res(graphics::Material* mat) override;
		void release_material_res(uint id) override;
		graphics::GraphicsPipelinePtr get_material_pipeline(MatRes& mr, uint hash);

		int register_mesh_instance(int id) override;
		void set_mesh_instance(uint id, const mat4& mat, const mat3& nor) override;

		int register_armature_instance(int id) override;
		mat4* set_armature_instance(uint id) override;

		int register_terrain_instance(int id) override;
		void set_terrain_instance(uint id, const mat4& mat, const vec3& extent, const uvec2& blocks, uint tess_level, graphics::ImageViewPtr textures) override;

		void draw_line(const vec3* points, uint count, const cvec4& color) override;

		void draw_mesh(uint instance_id, uint mesh_id, uint mat_id) override;
		void draw_mesh_occluder(uint instance_id, uint mesh_id, uint mat_id) override;
		void draw_mesh_outline(uint instance_id, uint mesh_id, const cvec4& color) override;
		void draw_mesh_wireframe(uint instance_id, uint mesh_id, const cvec4& color) override;
		void draw_terrain(uint instance_id, uint blocks, uint mat_id) override;
		void draw_terrain_outline(uint instance_id, uint blocks, const cvec4& color) override;
		void draw_terrain_wireframe(uint instance_id, uint blocks, const cvec4& color) override;
		void render(uint tar_idx, graphics::CommandBufferPtr cb) override;

		void update() override;

		cNodePtr pick_up(const uvec2& screen_pos, vec3* out_pos) override;
	};
}
