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
		std::vector<uint> mat_ids;
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
		uint instance_id;
		uint mesh_id;
		uint skin;
		cvec4 color;
	};

	struct DrawTerrain
	{
		cNodePtr node;
		uint instance_id;
		uint blocks;
		uint material_id;
		cvec4 color;
	};

	struct sRendererPrivate : sRenderer
	{
		graphics::WindowPtr window;

		std::vector<MeshRes> mesh_reses;

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
		cNodePtr current_node = nullptr;

		std::vector<graphics::ImageViewPtr> iv_tars;

		std::unique_ptr<graphics::Image>												img_black;
		std::unique_ptr<graphics::Image>												img_white;

		graphics::RenderpassPtr															rp_col = nullptr;
		graphics::RenderpassPtr															rp_col_dep = nullptr;
		graphics::RenderpassPtr															rp_fwd = nullptr;
		graphics::GraphicsPipelinePtr													pl_blit = nullptr;
		graphics::GraphicsPipelinePtr													pl_blit_tar = nullptr;
		graphics::GraphicsPipelinePtr													pl_add = nullptr;
		graphics::GraphicsPipelinePtr													pl_blend = nullptr;

		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageVertex>					buf_lines;
		graphics::PipelineResourceManager<FLAME_UID>									prm_plain3d;
		graphics::GraphicsPipelinePtr													pl_line3d = nullptr;

		std::unique_ptr<graphics::Image>												img_dst;
		std::unique_ptr<graphics::Image>												img_dep;
		std::unique_ptr<graphics::Framebuffer>											fb_fwd;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_mesh_ins;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_armature_ins;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_terrain_ins;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageVertex, false>			buf_vtx;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageIndex, false>			buf_idx;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageVertex, false>			buf_vtx_arm;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageIndex, false>			buf_idx_arm;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageUniform, false>			buf_scene;
		std::unique_ptr<graphics::DescriptorSet>										ds_scene;
		std::unique_ptr<graphics::DescriptorSet>										ds_instance;
		graphics::PipelineResourceManager<FLAME_UID>									prm_fwd;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageIndirect>				buf_idr_mesh;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageIndirect>				buf_idr_mesh_arm;
		graphics::GraphicsPipelinePtr													pl_mesh_fwd = nullptr;
		graphics::GraphicsPipelinePtr													pl_mesh_arm_fwd = nullptr;
		graphics::GraphicsPipelinePtr													pl_terrain_fwd = nullptr;

		std::unique_ptr<graphics::Image>												img_back0;
		std::unique_ptr<graphics::Image>												img_back1;

		graphics::GraphicsPipelinePtr													pl_mesh_plain = nullptr;
		graphics::GraphicsPipelinePtr													pl_mesh_arm_plain = nullptr;
		graphics::GraphicsPipelinePtr													pl_terrain_plain = nullptr;

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

		int set_material_res(int idx, graphics::Material* mat) override;
		int find_material_res(graphics::Material* mat) const override;

		int set_mesh_res(int idx, graphics::Mesh* mesh) override;
		int find_mesh_res(graphics::Mesh* mesh) const override;

		int register_mesh_instance(int id) override;
		void set_mesh_instance(uint id, const mat4& mat, const mat3& nor) override;

		int register_armature_instance(int id) override;
		mat4* set_armature_instance(uint id) override;

		int register_terrain_instance(int id) override;
		void set_terrain_instance(uint id, const mat4& mat, const vec3& extent, const uvec2& blocks, uint tess_level, graphics::ImageViewPtr textures) override;

		void draw_line(const vec3* points, uint count, const cvec4& color) override;

		void draw_mesh(uint instance_id, uint mesh_id, uint skin) override;
		void draw_mesh_occluder(uint instance_id, uint mesh_id, uint skin) override;
		void draw_mesh_outline(uint instance_id, uint mesh_id, const cvec4& color) override;
		void draw_mesh_wireframe(uint instance_id, uint mesh_id, const cvec4& color) override;
		void draw_terrain(uint instance_id, uint blocks, uint material_id) override;
		void draw_terrain_outline(uint instance_id, uint blocks, const cvec4& color) override;
		void draw_terrain_wireframe(uint instance_id, uint blocks, const cvec4& color) override;
		void render(uint tar_idx, graphics::CommandBufferPtr cb) override;

		void update() override;

		cNodePtr pick_up(const uvec2& screen_pos, vec3* out_pos) override;
	};
}
