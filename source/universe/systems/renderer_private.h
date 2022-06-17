#pragma once

#include "renderer.h"

#include "../../graphics/image.h"
#include "../../graphics/renderpass.h"
#include "../../graphics/shader.h"
#include "../../graphics/extension.h"

namespace flame
{
	struct IndexedDraws
	{
		uint mat_id;
		std::vector<uint> draw_ids;
	};

	struct sRendererPrivate : sRenderer
	{
		graphics::WindowPtr window;

		std::vector<MeshRes> mesh_reses;
		std::vector<TexRes>	tex_reses;
		std::vector<MatRes> mat_reses;

		int sky_map_res_id = -1;
		int sky_irr_map_res_id = -1;
		int sky_rad_map_res_id = -1;
		graphics::CommandBufferPtr									current_cb = nullptr;
		std::unordered_map<graphics::GraphicsPipelinePtr, uint>		mesh_draws;
		std::unordered_map<graphics::GraphicsPipelinePtr, uint>		arm_mesh_draws;
		std::unordered_map<graphics::GraphicsPipelinePtr, uint>		mesh_occulder_draws;
		std::unordered_map<graphics::GraphicsPipelinePtr, uint>		arm_mesh_occulder_draws;

		std::unique_ptr<graphics::Image>												img_black;
		std::unique_ptr<graphics::Image>												img_white;
		std::unique_ptr<graphics::Image>												img_cube_black;
		std::unique_ptr<graphics::Image>												img_cube_white;

		graphics::RenderpassPtr															rp_fwd = nullptr;
		graphics::RenderpassPtr															rp_gbuf = nullptr;
		graphics::PipelineLayoutPtr														pll_fwd = nullptr;
		graphics::PipelineLayoutPtr														pll_gbuf = nullptr;
		graphics::GraphicsPipelinePtr													pl_blit = nullptr;
		graphics::GraphicsPipelinePtr													pl_add = nullptr;
		graphics::GraphicsPipelinePtr													pl_blend = nullptr;

		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageVertex>					buf_lines;
		graphics::PipelineResourceManager<FLAME_UID>									prm_plain;
		graphics::GraphicsPipelinePtr													pl_line3d = nullptr;

		std::unique_ptr<graphics::Image>												img_dst;
		std::unique_ptr<graphics::Image>												img_dep;
		std::unique_ptr<graphics::Image>												img_col_met;	// color, metallic
		std::unique_ptr<graphics::Image>												img_nor_rou;	// normal, roughness
		std::unique_ptr<graphics::Image>												img_ao;			// ambient occlusion
		std::unique_ptr<graphics::Framebuffer>											fb_fwd;
		std::unique_ptr<graphics::Framebuffer>											fb_gbuf;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_mesh_ins;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_armature_ins;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_terrain_ins;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageUniform, false>			buf_material_misc;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_material;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageUniform, false>			buf_scene;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage>				buf_light_index;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage>				buf_light_grid;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_light_info;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage>				buf_dir_shadow;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage>				buf_pt_shadow;
		std::vector<std::unique_ptr<graphics::Image>>									imgs_dir_shadow;
		std::vector<std::unique_ptr<graphics::Image>>									imgs_pt_shadow;
		std::unique_ptr<graphics::DescriptorSet>										ds_scene;
		std::unique_ptr<graphics::DescriptorSet>										ds_instance;
		std::unique_ptr<graphics::DescriptorSet>										ds_material;
		std::unique_ptr<graphics::DescriptorSet>										ds_light;
		float																			sky_rad_levels = 1.f;
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
		std::unordered_map<uint, graphics::GraphicsPipelinePtr>							pls_deferred;
		graphics::PipelineResourceManager<FLAME_UID>									prm_deferred;
		std::unique_ptr<graphics::DescriptorSet>										ds_deferred;

		std::unique_ptr<graphics::Image>												img_back0;
		std::unique_ptr<graphics::Image>												img_back1;

		graphics::PipelineResourceManager<FLAME_UID>									prm_post;
		graphics::GraphicsPipelinePtr													pl_blur_h = nullptr;
		graphics::GraphicsPipelinePtr													pl_blur_v = nullptr;
		graphics::GraphicsPipelinePtr													pl_localmax_h = nullptr;
		graphics::GraphicsPipelinePtr													pl_localmax_v = nullptr;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_luma_avg;
		std::unique_ptr<graphics::DescriptorSet>										ds_luma_avg;
		graphics::PipelineResourceManager<FLAME_UID>									prm_luma;
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_luma_hist;
		std::unique_ptr<graphics::DescriptorSet>										ds_luma;
		graphics::ComputePipelinePtr													pl_luma_hist = nullptr;
		graphics::ComputePipelinePtr													pl_luma_avg = nullptr;
		graphics::PipelineResourceManager<FLAME_UID>									prm_tone;
		graphics::GraphicsPipelinePtr													pl_tone = nullptr;

		std::unique_ptr<graphics::Image>												img_pickup;
		std::unique_ptr<graphics::Image>												img_dep_pickup;
		std::unique_ptr<graphics::Framebuffer>											fb_pickup;
		graphics::GraphicsPipelinePtr													pl_mesh_pickup = nullptr;
		graphics::GraphicsPipelinePtr													pl_mesh_arm_pickup = nullptr;
		graphics::GraphicsPipelinePtr													pl_terrain_pickup = nullptr;
		std::unique_ptr<graphics::Fence>												fence_pickup;

		graphics::ImageLayout final_layout;

		sRendererPrivate();
		sRendererPrivate(graphics::WindowPtr w);

		void set_targets(std::span<graphics::ImageViewPtr> targets, graphics::ImageLayout final_layout) override;
		void bind_window_targets() override;

		void set_sky(graphics::ImageViewPtr sky_map, graphics::ImageViewPtr sky_irr_map, graphics::ImageViewPtr sky_rad_map) override;

		int get_texture_res(graphics::ImageViewPtr iv, graphics::SamplerPtr sp, int id) override;
		void release_texture_res(uint id) override;
		const TexRes& get_texture_res_info(uint id) override { return tex_reses[id]; }

		int get_mesh_res(graphics::MeshPtr mesh, int id) override;
		void release_mesh_res(uint id) override;
		const MeshRes& get_mesh_res_info(uint id) override { return mesh_reses[id]; }

		void update_mat_res(uint id, bool dying, bool update_parameters = true, bool update_textures = true, bool update_pipelines = true);
		int get_material_res(graphics::Material* mat, int id) override;
		void release_material_res(uint id) override;
		const MatRes& get_material_res_info(uint id) override { return mat_reses[id]; }
		graphics::GraphicsPipelinePtr get_material_pipeline(MatRes& mr, uint type, uint modifier1 = 0, uint modifier2 = 0);
		graphics::GraphicsPipelinePtr get_deferred_pipeline(uint modifier = 0);

		void update_res(uint id, uint type_hash, uint name_hash) override;

		int register_mesh_instance(int id) override;
		void set_mesh_instance(uint id, const mat4& mat, const mat3& nor) override;

		int register_armature_instance(int id) override;
		mat4* set_armature_instance(uint id) override;

		int register_terrain_instance(int id) override;
		void set_terrain_instance(uint id, const mat4& mat, const vec3& extent, const uvec2& blocks, uint tess_level, 
			graphics::ImageViewPtr height_map, graphics::ImageViewPtr normal_map, graphics::ImageViewPtr tangent_map, graphics::ImageViewPtr splash_map) override;

		int register_light_instance(int id) override;

		void render(uint tar_idx, graphics::CommandBufferPtr cb) override;

		void update() override;

		cNodePtr pick_up(const uvec2& screen_pos, vec3* out_pos) override;
	};
}
