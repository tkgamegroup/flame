#pragma once

#include "node_renderer.h"

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

	struct DrawMesh
	{
		cNodePtr node;
		uint object_id;
		uint mesh_id;
		uint skin;
	};

	struct DrawMeshOccluder
	{
		cNodePtr node;
		uint object_id;
		uint mesh_id;
		uint skin;
	};

	struct DrawMeshOutline
	{
		cNodePtr node;
		uint object_id;
		uint mesh_id;
		cvec4 color;
	};

	struct DrawMeshWireframe
	{
		cNodePtr node;
		uint object_id;
		uint mesh_id;
		cvec4 color;
	};

	struct sNodeRendererPrivate : sNodeRenderer
	{
		std::vector<MeshRes> mesh_reses;

		std::vector<DrawMesh>			draw_meshes;
		std::vector<DrawMesh>			draw_arm_meshes;
		std::vector<DrawMeshOccluder>	draw_occluder_meshes;
		std::vector<DrawMeshOutline>	draw_outline_meshes;
		std::vector<DrawMeshWireframe>	draw_wireframe_meshes;
		cNodePtr current_node = nullptr;

		std::vector<graphics::ImageViewPtr> iv_tars;

		graphics::GraphicsPipelinePtr													pl_blit;
		graphics::GraphicsPipelinePtr													pl_add;

		std::unique_ptr<graphics::Image>												img_dep;
		std::vector<std::unique_ptr<graphics::Framebuffer>>								fbs_fwd;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_objects;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_armatures;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageVertex, false>			buf_vtx;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageIndex, false>			buf_idx;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageVertex, false>			buf_vtx_arm;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageIndex, false>			buf_idx_arm;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageUniform, false>			buf_scene;
		std::unique_ptr<graphics::DescriptorSet>										ds_scene;
		std::unique_ptr<graphics::DescriptorSet>										ds_object;
		graphics::PipelineResourceManager<FLAME_UID>									prm_mesh_fwd;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageIndirect>				buf_idr_mesh;
		graphics::GraphicsPipelinePtr													pl_mesh_fwd;

		std::unique_ptr<graphics::Image>												img_back0;
		std::unique_ptr<graphics::Image>												img_back1;

		graphics::GraphicsPipelinePtr													pl_mesh_plain;

		graphics::PipelineResourceManager<FLAME_UID>									prm_post;
		graphics::GraphicsPipelinePtr													pl_blur_h;
		graphics::GraphicsPipelinePtr													pl_blur_v;
		graphics::GraphicsPipelinePtr													pl_localmax_h;
		graphics::GraphicsPipelinePtr													pl_localmax_v;

		std::unique_ptr<graphics::Image>												img_pickup;
		std::unique_ptr<graphics::Image>												img_dep_pickup;
		std::unique_ptr<graphics::Framebuffer>											fb_pickup;
		graphics::GraphicsPipelinePtr													pl_mesh_pickup;
		graphics::GraphicsPipelinePtr													pl_mesh_arm_pickup;
		std::unique_ptr<graphics::Fence>												fence_pickup;

		graphics::ImageLayout dst_layout;

		bool initialized = false;

		sNodeRendererPrivate() {}
		sNodeRendererPrivate(graphics::WindowPtr w);

		void set_targets(std::span<graphics::ImageViewPtr> targets, graphics::ImageLayout dst_layout) override;

		int set_material_res(int idx, graphics::Material* mat) override;
		int find_material_res(graphics::Material* mat) const override;

		int set_mesh_res(int idx, graphics::Mesh* mesh) override;
		int find_mesh_res(graphics::Mesh* mesh) const override;

		int register_object() override;
		void unregister_object(uint id) override;
		void set_object_matrix(uint id, const mat4& mat, const mat3& nor) override;

		int register_armature_object() override;
		void unregister_armature_object(uint id) override;
		void set_armature_object_matrices(uint id, const mat4* bones, uint count) override;

		void draw_mesh(uint object_id, uint mesh_id, uint skin) override;
		void draw_mesh_occluder(uint object_id, uint mesh_id, uint skin) override;
		void draw_mesh_outline(uint object_id, uint mesh_id, const cvec4& color) override;
		void draw_mesh_wireframe(uint object_id, uint mesh_id, const cvec4& color) override;
		void render(uint img_idx, graphics::CommandBufferPtr cb);

		void update() override;

		cNodePtr pick_up(const uvec2& pos) override;
	};
}
