#include "renderer_private.h"
#include "scene_private.h"
#include "../octree.h"
#include "../draw_data.h"
#include "../world_private.h"
#include "../components/node_private.h"
#include "../components/camera_private.h"

#include "../../foundation/typeinfo.h"
#include "../../foundation/typeinfo_serialize.h"
#include "../../foundation/window.h"
#include "../../graphics/renderpass.h"
#include "../../graphics/shader.h"
#include "../../graphics/window.h"
#include "../../graphics/material.h"
#include "../../graphics/model.h"
#include "../../graphics/extension.h"
#include "../../graphics/debug.h"

namespace flame
{
	const auto col_fmt = graphics::Format::Format_R16G16B16A16_SFLOAT;
	const auto dep_fmt = graphics::Format::Format_Depth16;
	const auto sample_count = graphics::SampleCount_4;
	const auto ShadowMapSize = uvec2(2048);
	const auto DirShadowMaxCount = 4U;
	const auto DirShadowMaxLevels = 4U;
	const auto PtShadowMaxCount = 4U;

	std::vector<sRenderer::MeshRes> mesh_reses;
	std::vector<sRenderer::TexRes> tex_reses;
	std::vector<sRenderer::MatRes> mat_reses;

	std::unique_ptr<graphics::Image> img_black;
	std::unique_ptr<graphics::Image> img_white;
	std::unique_ptr<graphics::Image> img_cube_black;
	std::unique_ptr<graphics::Image> img_cube_white;
	std::unique_ptr<graphics::Image> img_back0;
	std::unique_ptr<graphics::Image> img_back1;
	std::unique_ptr<graphics::Image> img_dst;
	std::unique_ptr<graphics::Image> img_dep;
	std::unique_ptr<graphics::Image> img_dst_ms;
	std::unique_ptr<graphics::Image> img_dep_ms;
	std::unique_ptr<graphics::Image> img_col_met;	// color, metallic
	std::unique_ptr<graphics::Image> img_nor_rou;	// normal, roughness
	std::unique_ptr<graphics::Image> img_ao;		// ambient occlusion
	std::unique_ptr<graphics::Image> img_pickup;
	std::unique_ptr<graphics::Image> img_dep_pickup;
	std::vector<std::unique_ptr<graphics::Image>>	imgs_dir_shadow;
	std::vector<std::unique_ptr<graphics::Image>>	imgs_pt_shadow;
	std::unique_ptr<graphics::Image>				img_dir_shadow_back;
	std::unique_ptr<graphics::Image>				img_pt_shadow_back;

	graphics::RenderpassPtr rp_fwd = nullptr;
	graphics::RenderpassPtr rp_gbuf = nullptr;
	graphics::RenderpassPtr rp_dep = nullptr;
	graphics::RenderpassPtr rp_col_dep = nullptr;
	std::unique_ptr<graphics::Framebuffer> fb_fwd;
	std::unique_ptr<graphics::Framebuffer> fb_gbuf;
	std::unique_ptr<graphics::Framebuffer> fb_pickup;
	graphics::PipelineLayoutPtr pll_fwd = nullptr;
	graphics::PipelineLayoutPtr pll_gbuf = nullptr;
	graphics::PipelineResourceManager prm_fwd;
	graphics::PipelineResourceManager prm_gbuf;
	graphics::PipelineResourceManager prm_deferred;
	graphics::PipelineResourceManager prm_plain;
	graphics::PipelineResourceManager prm_post;
	graphics::PipelineResourceManager prm_luma;
	graphics::PipelineResourceManager prm_tone;

	graphics::VertexBuffer													buf_vtx;
	graphics::IndexBuffer													buf_idx;
	graphics::VertexBuffer													buf_vtx_arm;
	graphics::IndexBuffer													buf_idx_arm;

	graphics::StorageBuffer2														buf_camera;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_mesh_ins;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_armature_ins;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_terrain_ins;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_sdf_ins;
	graphics::StorageBuffer2														buf_material;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageUniform, false>			buf_lighting;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage>				buf_light_index;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage>				buf_light_grid;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_light_info;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage>				buf_dir_shadow;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage>				buf_pt_shadow;
	graphics::VertexBuffer													buf_primitives;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_luma_avg;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_luma_hist;

	std::unique_ptr<graphics::DescriptorSet> ds_camera;
	std::unique_ptr<graphics::DescriptorSet> ds_instance;
	std::unique_ptr<graphics::DescriptorSet> ds_material;
	std::unique_ptr<graphics::DescriptorSet> ds_light;
	std::unique_ptr<graphics::DescriptorSet> ds_deferred;
	std::unique_ptr<graphics::DescriptorSet> ds_luma_avg;
	std::unique_ptr<graphics::DescriptorSet> ds_luma;

	graphics::GraphicsPipelinePtr pl_blit = nullptr;
	graphics::GraphicsPipelinePtr pl_blit_dep = nullptr;
	graphics::GraphicsPipelinePtr pl_add = nullptr;
	graphics::GraphicsPipelinePtr pl_blend = nullptr;
	graphics::GraphicsPipelinePtr pl_blur_h = nullptr;
	graphics::GraphicsPipelinePtr pl_blur_v = nullptr;
	graphics::GraphicsPipelinePtr pl_blur_dep_h = nullptr;
	graphics::GraphicsPipelinePtr pl_blur_dep_v = nullptr;
	graphics::GraphicsPipelinePtr pl_localmax_h = nullptr;
	graphics::GraphicsPipelinePtr pl_localmax_v = nullptr;
	graphics::ComputePipelinePtr pl_luma_hist = nullptr;
	graphics::ComputePipelinePtr pl_luma_avg = nullptr;
	graphics::GraphicsPipelinePtr pl_tone = nullptr;
	graphics::GraphicsPipelinePtr pl_fxaa = nullptr;

	graphics::GraphicsPipelinePtr pl_mesh_plain = nullptr;
	graphics::GraphicsPipelinePtr pl_mesh_arm_plain = nullptr;
	graphics::GraphicsPipelinePtr pl_terrain_plain = nullptr;
	graphics::GraphicsPipelinePtr pl_mesh_pickup = nullptr;
	graphics::GraphicsPipelinePtr pl_mesh_arm_pickup = nullptr;
	graphics::GraphicsPipelinePtr pl_terrain_pickup = nullptr;
	graphics::GraphicsPipelinePtr pl_line3d = nullptr;
	graphics::GraphicsPipelinePtr pl_triangle3d = nullptr;

	std::unique_ptr<graphics::Fence> fence_pickup;

	int camera_light_id = -1;

	struct MeshBuckets
	{
		graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageIndirect> buf_idr;
		std::unordered_map<graphics::GraphicsPipelinePtr, std::pair<bool, std::vector<uint>>> draw_idxs;

		void collect_idrs(const DrawData& draw_data, graphics::CommandBufferPtr cb, uint mod2 = 0);
		void draw(graphics::CommandBufferPtr cb);
	};

	struct DirShadow
	{
		mat3 rot;
		std::vector<cNodePtr> culled_nodes;
		MeshBuckets mesh_buckets[DirShadowMaxLevels];
		std::vector<TerrainDraw> draw_terrains[DirShadowMaxLevels];
	};

	std::vector<cNodePtr> camera_culled_nodes;
	DrawData draw_data;
	MeshBuckets opa_mesh_buckets;
	MeshBuckets trs_mesh_buckets;
	DirShadow dir_shadows[DirShadowMaxCount];

	std::vector<PrimitiveDraw> debug_primitives;
	bool csm_debug_sig = false;

	static graphics::GraphicsPipelinePtr get_material_pipeline(sRenderer::MatRes& mr, uint type, uint modifier1 = 0, uint modifier2 = 0)
	{
		auto key = type + modifier1 + modifier2;
		auto it = mr.pls.find(key);
		if (it != mr.pls.end())
			return it->second;

		std::vector<std::string> defines;
		switch (type)
		{
		case "mesh"_h:
		case "terrain"_h:
		case "sdf"_h:
			if (modifier2 == "OCCLUDER_PASS"_h)
			{
				defines.push_back("rp=" + str(rp_dep));
				defines.push_back("pll=" + str(pll_fwd));
				defines.push_back("cull_mode=" + TypeInfo::serialize_t(graphics::CullModeFront));
			}
			else if (mr.opa)
			{
				defines.push_back("rp=" + str(rp_gbuf));
				defines.push_back("pll=" + str(pll_gbuf));
				defines.push_back("all_shader:GBUFFER_PASS");
			}
			else
			{
				defines.push_back("rp=" + str(rp_fwd));
				defines.push_back("pll=" + str(pll_fwd));
				defines.push_back("a2c=true");
			}
			break;
		case "grass_field"_h:
			defines.push_back("rp=" + str(rp_fwd));
			defines.push_back("pll=" + str(pll_fwd));
			break;
		}

		auto mat_file = Path::get(mr.mat->shader_file).string();
		defines.push_back(std::format("frag:MAT_FILE={}", mat_file));
		for (auto& d : mr.mat->shader_defines)
			defines.push_back("all_shader:" + d);

		std::filesystem::path pipeline_name;
		switch (type)
		{
		case "mesh"_h:
			pipeline_name = L"flame\\shaders\\mesh\\mesh.pipeline";
			break;
		case "terrain"_h:
			pipeline_name = L"flame\\shaders\\terrain\\terrain.pipeline";
			break;
		case "grass_field"_h:
			pipeline_name = L"flame\\shaders\\terrain\\grass_field.pipeline";
			defines.push_back("tese:HAS_GEOM");
			defines.push_back("all_shader:GRASS_FIELD");
			break;
		case "sdf"_h:
			pipeline_name = L"flame\\shaders\\sdf\\sdf.pipeline";
			break;
		}
		switch (modifier1)
		{
		case "ARMATURE"_h:
			defines.push_back("vert:ARMATURE");
			break;
		}
		switch (modifier2)
		{
		case "OCCLUDER_PASS"_h:
			defines.push_back("all_shader:OCCLUDER_PASS");
			break;
		}
		std::sort(defines.begin(), defines.end());

		graphics::GraphicsPipelinePtr ret = nullptr;
		if (!pipeline_name.empty())
			ret = graphics::GraphicsPipeline::get(pipeline_name, defines);
		if (ret)
		{
			ret->dynamic_renderpass = true;
			mr.pls[key] = ret;
		}
		return ret;
	}

	static graphics::GraphicsPipelinePtr get_deferred_pipeline(uint modifier = 0)
	{
		static std::unordered_map<uint, graphics::GraphicsPipelinePtr> pls;
		auto it = pls.find(modifier);
		if (it != pls.end())
			return it->second;

		std::vector<std::string> defines;
		switch (modifier)
		{
		case "ALBEDO_DATA"_h:
			defines.push_back("frag:ALBEDO_DATA");
			break;
		case "NORMAL_DATA"_h:
			defines.push_back("frag:NORMAL_DATA");
			break;
		case "METALLIC_DATA"_h:
			defines.push_back("frag:METALLIC_DATA");
			break;
		case "ROUGHNESS_DATA"_h:
			defines.push_back("frag:ROUGHNESS_DATA");
			break;
		case "IBL_VALUE"_h:
			defines.push_back("frag:IBL_VALUE");
			break;
		case "FOG_VALUE"_h:
			defines.push_back("frag:FOG_VALUE");
			break;
		}
		std::sort(defines.begin(), defines.end());

		auto ret = graphics::GraphicsPipeline::get(L"flame\\shaders\\deferred.pipeline", defines);
		if (ret)
		{
			ret->dynamic_renderpass = true;
			pls[modifier] = ret;
		}
		return ret;
	}

	void MeshBuckets::collect_idrs(const DrawData& draw_data, graphics::CommandBufferPtr cb, uint mod2)
	{
		for (auto i = 0; i < draw_data.meshes.size(); i++)
		{
			auto& m = draw_data.meshes[i];
			auto& mesh_r = mesh_reses[m.mesh_id];
			auto& mat_r = mat_reses[m.mat_id];
			auto pl = get_material_pipeline(mat_r, "mesh"_h, mesh_r.arm ? "ARMATURE"_h : 0, mod2);
			auto it = draw_idxs.find(pl);
			if (it == draw_idxs.end())
				it = draw_idxs.emplace(pl, std::make_pair(mesh_r.arm, std::vector<uint>())).first;
			it->second.second.push_back(i);
		}

		for (auto& d : draw_idxs)
		{
			for (auto i : d.second.second)
			{
				auto& m = draw_data.meshes[i];
				auto& mesh_r = mesh_reses[m.mesh_id];
				buf_idr.add_draw_indexed_indirect(mesh_r.idx_cnt, mesh_r.idx_off, mesh_r.vtx_off, 1, (m.ins_id << 8) + m.mat_id);
			}
		}
		buf_idr.upload(cb);
	}

	void MeshBuckets::draw(graphics::CommandBufferPtr cb)
	{
		auto off = 0;
		for (auto& d : draw_idxs)
		{
			if (!d.second.first)
			{
				cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
				cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
			}
			else
			{
				cb->bind_vertex_buffer(buf_vtx_arm.buf.get(), 0);
				cb->bind_index_buffer(buf_idx_arm.buf.get(), graphics::IndiceTypeUint);
			}
			cb->bind_pipeline(d.first);
			cb->draw_indexed_indirect(buf_idr.buf.get(), off, d.second.second.size());
			off += d.second.second.size();
		}
	}

	sRendererPrivate::sRendererPrivate() 
	{
	}

	sRendererPrivate::sRendererPrivate(graphics::WindowPtr w) :
		window(w)
	{
		img_black.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec2(4), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, 1, 8));
		img_white.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec2(4), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, 1, 8));
		img_cube_black.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec2(4), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, 1, 6, graphics::SampleCount_1, true));
		img_cube_white.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec2(4), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, 1, 6, graphics::SampleCount_1, true));
		img_black->clear(vec4(0.f), graphics::ImageLayoutShaderReadOnly);
		img_white->clear(vec4(1.f), graphics::ImageLayoutShaderReadOnly);
		img_cube_black->clear(vec4(0.f), graphics::ImageLayoutShaderReadOnly);
		img_cube_white->clear(vec4(1.f), graphics::ImageLayoutShaderReadOnly);

		rp_fwd = graphics::Renderpass::get(L"flame\\shaders\\forward.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(col_fmt),
			  "dep_fmt=" + TypeInfo::serialize_t(dep_fmt),
			  "sample_count=" + TypeInfo::serialize_t(sample_count) });
		rp_gbuf = graphics::Renderpass::get(L"flame\\shaders\\gbuffer.rp",
			{ "dep_fmt=" + TypeInfo::serialize_t(dep_fmt) });
		rp_dep = graphics::Renderpass::get(L"flame\\shaders\\depth.rp",
			{ "dep_fmt=" + TypeInfo::serialize_t(dep_fmt) });
		rp_col_dep = graphics::Renderpass::get(L"flame\\shaders\\color_depth.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(graphics::Format_R8G8B8A8_UNORM),
			  "dep_fmt=" + TypeInfo::serialize_t(dep_fmt) });

		auto sp_trilinear = graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, true, graphics::AddressClampToEdge);
		auto sp_shadow = graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressClampToBorder);

		auto dsl_camera = graphics::DescriptorSetLayout::get(L"flame\\shaders\\camera.dsl");
		buf_camera.create(graphics::BufferUsageUniform, dsl_camera->get_buf_ui("Camera"_h));
		ds_camera.reset(graphics::DescriptorSet::create(nullptr, dsl_camera));
		ds_camera->set_buffer("Camera"_h, 0, buf_camera.buf.get());
		ds_camera->update();
		auto dsl_instance = graphics::DescriptorSetLayout::get(L"flame\\shaders\\instance.dsl");
		buf_mesh_ins.create_with_array_type(dsl_instance->get_buf_ui("MeshInstances"_h));
		buf_armature_ins.create_with_array_type(dsl_instance->get_buf_ui("ArmatureInstances"_h));
		buf_terrain_ins.create_with_array_type(dsl_instance->get_buf_ui("TerrainInstances"_h));
		buf_sdf_ins.create_with_array_type(dsl_instance->get_buf_ui("SdfInstances"_h));
		ds_instance.reset(graphics::DescriptorSet::create(nullptr, dsl_instance));
		ds_instance->set_buffer("MeshInstances"_h, 0, buf_mesh_ins.buf.get());
		ds_instance->set_buffer("ArmatureInstances"_h, 0, buf_armature_ins.buf.get());
		ds_instance->set_buffer("TerrainInstances"_h, 0, buf_terrain_ins.buf.get());
		ds_instance->set_buffer("SdfInstances"_h, 0, buf_sdf_ins.buf.get());
		for (auto i = 0; i < buf_terrain_ins.array_capacity; i++)
		{
			ds_instance->set_image("terrain_height_maps"_h, i, img_black->get_view(), sp_trilinear);
			ds_instance->set_image("terrain_normal_maps"_h, i, img_black->get_view(), sp_trilinear);
			ds_instance->set_image("terrain_tangent_maps"_h, i, img_black->get_view(), sp_trilinear);
			ds_instance->set_image("terrain_splash_maps"_h, i, img_black->get_view(), sp_trilinear);
		}
		ds_instance->update();
		auto dsl_material = graphics::DescriptorSetLayout::get(L"flame\\shaders\\material.dsl");
		buf_material.create(graphics::BufferUsageStorage, dsl_material->get_buf_ui("Material"_h));
		buf_material.item("black_map_id"_h).set(get_texture_res(img_black->get_view(), nullptr, -1));
		buf_material.item("white_map_id"_h).set(get_texture_res(img_white->get_view(), nullptr, -1));
		{
			auto img = graphics::Image::get(L"flame\\random.png");
			buf_material.item("random_map_id"_h).set(img ? get_texture_res(img->get_view(),
				graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressRepeat), -1) : -1);
		}
		mat_reses.resize(buf_material.item_info("infos"_h).array_size);
		get_material_res(graphics::Material::get(L"default"), -1);
		tex_reses.resize(dsl_material->get_binding("material_maps"_h).count);
		ds_material.reset(graphics::DescriptorSet::create(nullptr, dsl_material));
		ds_material->set_buffer("Material"_h, 0, buf_material.buf.get());
		for (auto i = 0; i < tex_reses.size(); i++)
			ds_material->set_image("material_maps"_h, i, img_black->get_view(), nullptr);
		ds_material->update();
		auto dsl_light = graphics::DescriptorSetLayout::get(L"flame\\shaders\\light.dsl");
		ds_light.reset(graphics::DescriptorSet::create(nullptr, dsl_light));
		buf_lighting.create(dsl_light->get_buf_ui("Lighting"_h));
		buf_light_index.create_with_array_type(dsl_light->get_buf_ui("LightIndexs"_h));
		buf_light_grid.create_with_array_type(dsl_light->get_buf_ui("LightGrids"_h));
		buf_light_info.create_with_array_type(dsl_light->get_buf_ui("LightInfos"_h));
		buf_dir_shadow.create_with_array_type(dsl_light->get_buf_ui("DirShadows"_h));
		buf_pt_shadow.create_with_array_type(dsl_light->get_buf_ui("PtShadows"_h));
		imgs_dir_shadow.resize(dsl_light->get_binding("dir_shadow_maps"_h).count);
		for (auto& i : imgs_dir_shadow)
		{
			i.reset(graphics::Image::create(dep_fmt, ShadowMapSize, graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 1, DirShadowMaxLevels));
			i->change_layout(graphics::ImageLayoutShaderReadOnly);
		}
		img_dir_shadow_back.reset(graphics::Image::create(dep_fmt, ShadowMapSize, graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 1, DirShadowMaxLevels));
		imgs_pt_shadow.resize(dsl_light->get_binding("pt_shadow_maps"_h).count);
		for (auto& i : imgs_pt_shadow)
		{
			i.reset(graphics::Image::create(dep_fmt, ShadowMapSize, graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 1, 6, graphics::SampleCount_1, true));
			i->change_layout(graphics::ImageLayoutShaderReadOnly);
		}
		img_pt_shadow_back.reset(graphics::Image::create(dep_fmt, ShadowMapSize, graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 1, 6, graphics::SampleCount_1, true));
		ds_light->set_buffer("Lighting"_h, 0, buf_lighting.buf.get());
		ds_light->set_buffer("LightIndexs"_h, 0, buf_light_index.buf.get());
		ds_light->set_buffer("LightGrids"_h, 0, buf_light_grid.buf.get());
		ds_light->set_buffer("LightInfos"_h, 0, buf_light_info.buf.get());
		ds_light->set_buffer("DirShadows"_h, 0, buf_dir_shadow.buf.get());
		ds_light->set_buffer("PtShadows"_h, 0, buf_pt_shadow.buf.get());
		for (auto i = 0; i < imgs_dir_shadow.size(); i++)
			ds_light->set_image("dir_shadow_maps"_h, i, imgs_dir_shadow[i]->get_view({ 0, 1, 0, DirShadowMaxLevels }), sp_shadow);
		for (auto i = 0; i < imgs_pt_shadow.size(); i++)
			ds_light->set_image("pt_shadow_maps"_h, i, imgs_pt_shadow[i]->get_view({ 0, 1, 0, 6 }), sp_shadow);
		ds_light->set_image("sky_map"_h, 0, img_cube_black->get_view({ 0, 1, 0, 6 }), nullptr);
		ds_light->set_image("sky_irr_map"_h, 0, img_cube_black->get_view({ 0, 1, 0, 6 }), nullptr);
		ds_light->set_image("sky_rad_map"_h, 0, img_cube_black->get_view({ 0, 1, 0, 6 }), nullptr);
		{
			auto img = graphics::Image::get(L"flame\\brdf.dds");
			ds_light->set_image("brdf_map"_h, 0, img ? img->get_view() : img_black->get_view(), nullptr);
		}
		ds_light->update();

		mesh_reses.resize(1024);

		prm_plain.init(graphics::PipelineLayout::get(L"flame\\shaders\\plain\\plain.pll"));
		pl_line3d = graphics::GraphicsPipeline::get(L"flame\\shaders\\plain\\line3d.pipeline", {});
		pl_line3d->dynamic_renderpass = true;
		pl_triangle3d = graphics::GraphicsPipeline::get(L"flame\\shaders\\plain\\triangle3d.pipeline", {});
		pl_triangle3d->dynamic_renderpass = true;
		buf_primitives.create(pl_line3d->vi_ui(), 1024 * 128);

		pll_fwd = graphics::PipelineLayout::get(L"flame\\shaders\\forward.pll");
		pll_gbuf = graphics::PipelineLayout::get(L"flame\\shaders\\gbuffer.pll");

		pl_blit = graphics::GraphicsPipeline::get(L"flame\\shaders\\blit.pipeline", {});
		pl_blit->dynamic_renderpass = true;
		pl_blit_dep = graphics::GraphicsPipeline::get(L"flame\\shaders\\blit.pipeline", { "rp=" + str(rp_dep), "frag:DEPTH" });
		pl_blit_dep->dynamic_renderpass = true;
		pl_add = graphics::GraphicsPipeline::get(L"flame\\shaders\\add.pipeline", {});
		pl_add->dynamic_renderpass = true;
		pl_blend = graphics::GraphicsPipeline::get(L"flame\\shaders\\blend.pipeline", {});
		pl_blend->dynamic_renderpass = true;

		prm_fwd.init(pll_fwd);
		prm_fwd.set_ds("camera"_h, ds_camera.get());
		prm_fwd.set_ds("instance"_h, ds_instance.get());
		prm_fwd.set_ds("material"_h, ds_material.get());
		prm_fwd.set_ds("light"_h, ds_light.get());

		prm_gbuf.init(pll_gbuf);
		prm_gbuf.set_ds("camera"_h, ds_camera.get());
		prm_gbuf.set_ds("instance"_h, ds_instance.get());
		prm_gbuf.set_ds("material"_h, ds_material.get());

		pl_mesh_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline", { "rp=" + str(rp_col_dep) });
		pl_mesh_plain->dynamic_renderpass = true;
		pl_mesh_arm_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline", { "rp=" + str(rp_col_dep), "vert:ARMATURE" });
		pl_mesh_arm_plain->dynamic_renderpass = true;
		pl_terrain_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\terrain\\terrain.pipeline", { "rp=" + str(rp_col_dep) });
		pl_terrain_plain->dynamic_renderpass = true;

		buf_vtx.create(pl_mesh_plain->vi_ui(), 1024 * 256 * 4);
		buf_idx.create(1024 * 256 * 6);
		buf_vtx_arm.create(pl_mesh_arm_plain->vi_ui(), 1024 * 128 * 4);
		buf_idx_arm.create(1024 * 128 * 6);
		opa_mesh_buckets.buf_idr.create(0U, buf_mesh_ins.array_capacity);
		trs_mesh_buckets.buf_idr.create(0U, buf_mesh_ins.array_capacity);
		for (auto& s : dir_shadows)
		{
			for (auto i = 0; i < DirShadowMaxLevels; i++)
				s.mesh_buckets[i].buf_idr.create(0U, min(1024U, buf_mesh_ins.array_capacity));
		}

		prm_deferred.init(get_deferred_pipeline()->layout);
		prm_deferred.set_ds("camera"_h, ds_camera.get());
		prm_deferred.set_ds("light"_h, ds_light.get());
		ds_deferred.reset(graphics::DescriptorSet::create(nullptr, prm_deferred.pll->dsls.back()));
		prm_deferred.set_ds(""_h, ds_deferred.get());

		prm_post.init(graphics::PipelineLayout::get(L"flame\\shaders\\post\\post.pll"));
		pl_blur_h = graphics::GraphicsPipeline::get(L"flame\\shaders\\post\\blur.pipeline", { "frag:HORIZONTAL" });
		pl_blur_h->dynamic_renderpass = true;
		pl_blur_v = graphics::GraphicsPipeline::get(L"flame\\shaders\\post\\blur.pipeline", { "frag:VERTICAL" });
		pl_blur_v->dynamic_renderpass = true;
		pl_blur_dep_h = graphics::GraphicsPipeline::get(L"flame\\shaders\\post\\blur.pipeline", { "rp=" + str(rp_dep), "frag:HORIZONTAL", "frag:DEPTH" });
		pl_blur_dep_h->dynamic_renderpass = true;
		pl_blur_dep_v = graphics::GraphicsPipeline::get(L"flame\\shaders\\post\\blur.pipeline", { "rp=" + str(rp_dep), "frag:VERTICAL", "frag:DEPTH" });
		pl_blur_dep_v->dynamic_renderpass = true;
		pl_localmax_h = graphics::GraphicsPipeline::get(L"flame\\shaders\\post\\blur.pipeline",
			{ "frag:LOCAL_MAX",
			  "frag:HORIZONTAL" });
		pl_localmax_h->dynamic_renderpass = true;
		pl_localmax_v = graphics::GraphicsPipeline::get(L"flame\\shaders\\post\\blur.pipeline",
			{ "frag:LOCAL_MAX",
			  "frag:VERTICAL" });
		pl_localmax_v->dynamic_renderpass = true;

		auto dsl_luma_avg = graphics::DescriptorSetLayout::get(L"flame\\shaders\\post\\luma_avg.dsl");
		buf_luma_avg.create(dsl_luma_avg->get_buf_ui("LumaAvg"_h));
		ds_luma_avg.reset(graphics::DescriptorSet::create(nullptr, dsl_luma_avg));
		ds_luma_avg->set_buffer("LumaAvg"_h, 0, buf_luma_avg.buf.get());
		ds_luma_avg->update();
		prm_luma.init(graphics::PipelineLayout::get(L"flame\\shaders\\post\\luma.pll"), graphics::PipelineCompute);
		auto dsl_luma = prm_luma.pll->dsls.back();
		buf_luma_hist.create(dsl_luma->get_buf_ui("LumaHist"_h));
		ds_luma.reset(graphics::DescriptorSet::create(nullptr, dsl_luma));
		ds_luma->set_buffer("LumaHist"_h, 0, buf_luma_hist.buf.get());
		ds_luma->update();
		prm_luma.set_ds("luma_avg"_h, ds_luma_avg.get());
		prm_luma.set_ds(""_h, ds_luma.get());
		pl_luma_hist = graphics::ComputePipeline::get(L"flame\\shaders\\post\\luma_hist.pipeline", {});
		pl_luma_avg = graphics::ComputePipeline::get(L"flame\\shaders\\post\\luma_avg.pipeline", {});

		pl_tone = graphics::GraphicsPipeline::get(L"flame\\shaders\\post\\tone.pipeline", {});
		pl_tone->dynamic_renderpass = true;
		prm_tone.init(pl_tone->layout);
		prm_tone.set_ds("luma_avg"_h, ds_luma_avg.get());

		pl_fxaa = graphics::GraphicsPipeline::get(L"flame\\shaders\\post\\fxaa.pipeline", {});
		pl_fxaa->dynamic_renderpass = true;

		pl_mesh_pickup = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline", { "rp=" + str(rp_col_dep), "frag:PICKUP" });
		pl_mesh_pickup->dynamic_renderpass = true;
		pl_mesh_arm_pickup = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline",
			{ "rp=" + str(rp_col_dep),
			  "vert:ARMATURE",
			  "frag:PICKUP" });
		pl_mesh_arm_pickup->dynamic_renderpass = true;
		pl_terrain_pickup = graphics::GraphicsPipeline::get(L"flame\\shaders\\terrain\\terrain.pipeline", { "rp=" + str(rp_col_dep), "frag:PICKUP" });
		pl_terrain_pickup->dynamic_renderpass = true;
		fence_pickup.reset(graphics::Fence::create(false));
		
		w->renderers.add([this](uint img_idx, graphics::CommandBufferPtr cb) {
			render(img_idx, cb);
		}, 0, 0);
	}

	void sRendererPrivate::set_targets(std::span<graphics::ImageViewPtr> _targets, graphics::ImageLayout _final_layout)
	{
		iv_tars.assign(_targets.begin(), _targets.end());

		if (_targets.empty())
			return;

		graphics::Queue::get()->wait_idle();

		auto img0 = _targets.front()->image;
		auto tar_size = img0->size;

		auto sp_nearest = graphics::Sampler::get(graphics::FilterNearest, graphics::FilterNearest, false, graphics::AddressClampToEdge);

		img_dst.reset(graphics::Image::create(col_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled | graphics::ImageUsageStorage));
		img_dep.reset(graphics::Image::create(dep_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_dst_ms.reset(graphics::Image::create(col_fmt, tar_size, graphics::ImageUsageAttachment, 1, 1, sample_count));
		img_dep_ms.reset(graphics::Image::create(dep_fmt, tar_size, graphics::ImageUsageAttachment, 1, 1, sample_count));
		img_col_met.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_nor_rou.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_ao.reset(graphics::Image::create(graphics::Format_R16_UNORM, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		fb_fwd.reset(graphics::Framebuffer::create(rp_fwd, { img_dst_ms->get_view(), img_dep_ms->get_view(), img_dst->get_view(), img_dep->get_view() }));
		fb_gbuf.reset(graphics::Framebuffer::create(rp_gbuf, { img_col_met->get_view(), img_nor_rou->get_view(), img_dep->get_view()}));
		ds_deferred->set_image("img_col_met"_h, 0, img_col_met->get_view(), nullptr);
		ds_deferred->set_image("img_nor_rou"_h, 0, img_nor_rou->get_view(), nullptr);
		ds_deferred->set_image("img_ao"_h, 0, img_ao->get_view(), nullptr);
		ds_deferred->set_image("img_dep"_h, 0, img_dep->get_view(), nullptr);
		ds_deferred->update();
		ds_luma->set_image("img_col"_h, 0, img_dst->get_view(), nullptr);
		ds_luma->update();

		img_back0.reset(graphics::Image::create(col_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_back1.reset(graphics::Image::create(col_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));

		img_pickup.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageTransferSrc));
		img_dep_pickup.reset(graphics::Image::create(dep_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageTransferSrc));
		fb_pickup.reset(graphics::Framebuffer::create(rp_col_dep, { img_pickup->get_view(), img_dep_pickup->get_view() }));

		final_layout = _final_layout;

		camera_light_id = register_light_instance(-1);
	}

	void sRendererPrivate::bind_window_targets()
	{
		window->native->resize_listeners.add([this](const uvec2& sz) {
			graphics::Queue::get()->wait_idle();
			std::vector<graphics::ImageViewPtr> views;
			for (auto& i : window->swapchain->images)
				views.push_back(i->get_view());
			set_targets(views, graphics::ImageLayoutAttachment);
		});
		std::vector<graphics::ImageViewPtr> views;
		for (auto& i : window->swapchain->images) 
			views.push_back(i->get_view());
		set_targets(views, graphics::ImageLayoutAttachment);
	}

	vec2 sRendererPrivate::target_size()
	{
		if (iv_tars.empty())
			return vec2(0.f);
		return iv_tars[0]->image->size;
	}

	void sRendererPrivate::set_sky_maps(graphics::ImageViewPtr sky_map, graphics::ImageViewPtr sky_irr_map, graphics::ImageViewPtr sky_rad_map)
	{
		dirty = true;

		ds_light->set_image("sky_map"_h, 0, sky_map ? sky_map : img_cube_black->get_view({ 0, 1, 0, 6 }), nullptr);
		ds_light->set_image("sky_irr_map"_h, 0, sky_irr_map ? sky_irr_map : img_cube_black->get_view({ 0, 1, 0, 6 }), nullptr);
		ds_light->set_image("sky_rad_map"_h, 0, sky_rad_map ? sky_rad_map : img_cube_black->get_view({ 0, 1, 0, 6 }), nullptr);
		ds_light->update();

		sky_rad_levels = sky_rad_map ? sky_rad_map->sub.layer_count : 1.f;
	}

	void sRendererPrivate::set_sky_intensity(float v)
	{
		dirty = true;

		sky_intensity = v;
	}

	void sRendererPrivate::set_fog_color(const vec3& color)
	{
		dirty = true;

		fog_color = color;
	}

	void sRendererPrivate::set_shadow_distance(float d)
	{
		dirty = true;

		shadow_distance = d;
	}

	void sRendererPrivate::set_csm_levels(uint lv)
	{
		dirty = true;

		csm_levels = lv;
	}

	void sRendererPrivate::set_esm_factor(float f)
	{
		dirty = true;

		esm_factor = f;
	}

	int sRendererPrivate::get_texture_res(graphics::ImageViewPtr iv, graphics::SamplerPtr sp, int id)
	{
		if (id < 0)
		{
			for (auto i = 0; i < tex_reses.size(); i++)
			{
				auto& res = tex_reses[i];
				if (res.iv == iv && res.sp == sp)
				{
					if (id != -2)
						res.ref++;
					return i;
				}
			}
			if (id == -2)
				return -1;
			else
			{
				for (auto i = 0; i < tex_reses.size(); i++)
				{
					if (!tex_reses[i].iv)
					{
						id = i;
						break;
					}
				}
			}
			if (id < 0)
				return -1;
		}

		auto& res = tex_reses[id];
		res.iv = iv;
		res.sp = sp;
		res.ref = 1;

		ds_material->set_image("material_maps"_h, id, iv, sp);
		ds_material->update();

		return id;
	}

	void sRendererPrivate::release_texture_res(uint id)
	{
		graphics::Queue::get()->wait_idle();

		auto& res = tex_reses[id];
		if (res.ref == 1)
		{
			graphics::Queue::get()->wait_idle();
			ds_material->set_image("material_maps"_h, id, img_black->get_view(), nullptr);
			ds_material->update();
			
			res.iv = nullptr;
			res.ref = 0;
		}
		else
			res.ref--;
	}

	const sRenderer::TexRes& sRendererPrivate::get_texture_res_info(uint id)
	{ 
		return tex_reses[id]; 
	}

	int sRendererPrivate::get_mesh_res(graphics::MeshPtr mesh, int id)
	{
		if (id < 0)
		{
			for (auto i = 0; i < mesh_reses.size(); i++)
			{
				auto& res = mesh_reses[i];
				if (res.mesh == mesh)
				{
					if (id != -2)
						res.ref++;
					return i;
				}
			}
			if (id == -2)
				return -1;
			else
			{
				for (auto i = 0; i < mesh_reses.size(); i++)
				{
					if (!mesh_reses[i].mesh)
					{
						id = i;
						break;
					}
				}
			}
			if (id < 0)
				return -1;
		}

		auto& res = mesh_reses[id];
		res.mesh = mesh;
		res.ref = 1;

		res.vtx_cnt = mesh->positions.size();
		res.idx_cnt = mesh->indices.size();
		res.arm = !mesh->bone_ids.empty();
		if (!res.arm)
		{
			res.vtx_off = buf_vtx.stag_top;
			for (auto i = 0; i < res.vtx_cnt; i++)
			{
				auto pv = buf_vtx.add();
				pv.item("i_pos"_h).set(mesh->positions[i]);
				if (!mesh->normals.empty())
					pv.item("i_nor"_h).set(mesh->normals[i]);
				if (!mesh->uvs.empty())
					pv.item("i_uv"_h).set(mesh->uvs[i]);
			}

			res.idx_off = buf_idx.stag_top;
			buf_idx.add(mesh->indices.data(), res.idx_cnt);
		}
		else
		{
			res.vtx_off = buf_vtx_arm.stag_top;
			for (auto i = 0; i < res.vtx_cnt; i++)
			{
				auto pv = buf_vtx_arm.add();
				pv.item("i_pos"_h).set(mesh->positions[i]);
				if (!mesh->normals.empty())
					pv.item("i_nor"_h).set(mesh->normals[i]);
				if (!mesh->uvs.empty())
					pv.item("i_uv"_h).set(mesh->uvs[i]);
				if (!mesh->bone_ids.empty())
					pv.item("i_bids"_h).set(mesh->bone_ids[i]);
				if (!mesh->bone_weights.empty())
					pv.item("i_bwgts"_h).set(mesh->bone_weights[i]);
			}

			res.idx_off = buf_idx_arm.stag_top;
			buf_idx_arm.add(mesh->indices.data(), res.idx_cnt);
		}

		return id;
	}

	void sRendererPrivate::release_mesh_res(uint id)
	{
		auto& res = mesh_reses[id];
		if (res.ref == 1)
		{
			res.mesh = nullptr;
			res.ref = 0;
		}
		else
			res.ref--;
	}

	const sRenderer::MeshRes& sRendererPrivate::get_mesh_res_info(uint id)
	{ 
		return mesh_reses[id]; 
	}

	void sRendererPrivate::update_mat_res(uint id, bool dying, bool update_parameters, bool update_textures, bool update_pipelines)
	{
		auto& res = mat_reses[id];

		if (update_textures)
		{
			if (dying)
			{
				for (auto& tex : res.texs)
				{
					if (tex.first != -1)
						release_texture_res(tex.first);
					if (tex.second)
						graphics::Image::release(tex.second);
					tex.first = -1;
					tex.second = nullptr;
				}
				res.texs.clear();
			}
			else
			{
				res.texs.resize(res.mat->textures.size());
				for (auto i = 0; i < res.texs.size(); i++)
				{
					res.texs[i].first = -1;
					res.texs[i].second = nullptr;
					auto& src = res.mat->textures[i];
					if (!src.filename.empty())
					{
						if (auto image = graphics::Image::get(src.filename, src.srgb, src.auto_mipmap, i == res.mat->alpha_map ? res.mat->alpha_test : 0.f); image)
						{
							res.texs[i].second = image;
							res.texs[i].first = get_texture_res(image->get_view({ 0, image->n_levels, 0, image->n_layers }),
								graphics::Sampler::get(src.mag_filter, src.min_filter, src.linear_mipmap, src.address_mode), -1);
						}
					}
				}
			}
		}
		if (update_pipelines)
		{
			for (auto& pl : res.pls)
				graphics::GraphicsPipeline::release(pl.second);
			res.pls.clear();
		}
		if (update_parameters || update_textures)
		{
			auto p_info = buf_material.item("infos"_h, id);
			buf_material.mark_dirty(p_info);
			p_info.item("color"_h).set(res.mat->color);
			p_info.item("metallic"_h).set(res.mat->metallic);
			p_info.item("roughness"_h).set(res.mat->roughness);
			p_info.item("opaque"_h).set(res.mat->opaque);
			p_info.item("f"_h).set(res.mat->float_values);
			p_info.item("i"_h).set(res.mat->int_values);
			auto p_ids = (int*)p_info.item("map_indices"_h).pdata;
			for (auto i = 0; i < res.texs.size(); i++)
				p_ids[i] = res.texs[i].first;
		}
	}

	int sRendererPrivate::get_material_res(graphics::Material* mat, int id)
	{
		if (id < 0)
		{
			for (auto i = 0; i < mat_reses.size(); i++)
			{
				auto& res = mat_reses[i];
				if (res.mat == mat)
				{
					if (id != -2)
						res.ref++;
					return i;
				}
			}
			if (id == -2)
				return -1;
			else
			{
				for (auto i = 0; i < mat_reses.size(); i++)
				{
					if (!mat_reses[i].mat)
					{
						id = i;
						break;
					}
				}
			}
			if (id < 0)
				return -1;
		}

		auto& res = mat_reses[id];
		if (mat->color_map != -1)
		{
			auto found = false;
			for (auto& d : mat->shader_defines)
			{
				if (d.starts_with("COLOR_MAP") != std::string::npos)
				{
					d = "COLOR_MAP=" + str(mat->color_map);
					found = true;
					break;
				}
			}
			if (!found)
				mat->shader_defines.push_back("COLOR_MAP=" + str(mat->color_map));
		}
		if (mat->alpha_test > 0.f)
		{
			auto found = false;
			for (auto& d : mat->shader_defines)
			{
				if (d.starts_with("ALPHA_TEST"))
				{
					d = "ALPHA_TEST=" + str(mat->alpha_test);
					found = true;
					break;
				}
			}
			if (!found)
				mat->shader_defines.push_back("ALPHA_TEST=" + str(mat->alpha_test));
		}
		res.mat = mat;
		res.ref = 1;
		res.opa = mat->opaque;
		update_mat_res(id, false);
		return id;
	}

	void sRendererPrivate::release_material_res(uint id)
	{
		auto& res = mat_reses[id];
		if (res.ref == 1)
		{
			update_mat_res(id, true);
			res.mat = nullptr;
		}
		else
			res.ref--;
	}

	const sRenderer::MatRes& sRendererPrivate::get_material_res_info(uint id)
	{ 
		return mat_reses[id]; 
	}

	void sRendererPrivate::update_res(uint id, uint type_hash, uint name_hash)
	{
		switch (type_hash)
		{
		case "material"_h:
			if (auto& res = mat_reses[id]; res.mat)
			{
				update_mat_res(id, false, 
					name_hash == "parameters"_h || name_hash == 0,
					name_hash == "textures"_h || name_hash == 0, 
					name_hash == "pipelines"_h || name_hash == 0);
			}
			break;
		}
	}

	int sRendererPrivate::register_light_instance(int id)
	{
		if (id == -1)
		{
			id = buf_light_info.get_free_item();
			if (id != -1)
			{

			}
		}
		else
		{
			buf_light_info.release_item(id);

		}
		return id;
	}

	void sRendererPrivate::set_light_instance(uint id, LightType type, const vec3& pos, const vec3& color, float range)
	{
		buf_light_info.select_item(id);
		buf_light_info.set_var<"type"_h>(type);
		buf_light_info.set_var<"pos"_h>(pos);
		buf_light_info.set_var<"color"_h>(color);
	}

	int sRendererPrivate::register_mesh_instance(int id)
	{
		if (id == -1)
		{
			id = buf_mesh_ins.get_free_item();
			if (id != -1)
				set_mesh_instance(id, mat4(1.f), mat3(1.f));
		}
		else
			buf_mesh_ins.release_item(id);
		return id;
	}

	void sRendererPrivate::set_mesh_instance(uint id, const mat4& mat, const mat3& nor)
	{
		buf_mesh_ins.select_item(id);
		buf_mesh_ins.set_var<"mat"_h>(mat);
		buf_mesh_ins.set_var<"nor"_h>(mat4(nor));
	}

	int sRendererPrivate::register_armature_instance(int id)
	{
		if (id == -1)
		{
			id = buf_armature_ins.get_free_item();
			if (id != -1)
			{
				auto dst = set_armature_instance(id);
				auto size = buf_armature_ins.ui->variables[0].array_size;
				for (auto i = 0; i < size; i++)
					dst[i] = mat4(1.f);
			}
		}
		else
			buf_armature_ins.release_item(id);
		return id;
	}

	mat4* sRendererPrivate::set_armature_instance(uint id)
	{
		buf_armature_ins.select_item(id);
		return (mat4*)buf_armature_ins.pend;
	}

	int sRendererPrivate::register_terrain_instance(int id)
	{
		if (id == -1)
		{
			id = buf_terrain_ins.get_free_item();
			if (id != -1)
			{

			}
		}
		else
		{
			buf_terrain_ins.release_item(id);
			ds_instance->set_image("terrain_height_maps"_h, id, img_black->get_view(), nullptr);
			ds_instance->set_image("terrain_normal_maps"_h, id, img_black->get_view(), nullptr);
			ds_instance->set_image("terrain_tangent_maps"_h, id, img_black->get_view(), nullptr);
			ds_instance->set_image("terrain_splash_maps"_h, id, img_black->get_view(), nullptr);
			ds_instance->update();
		}
		return id;
	}

	void sRendererPrivate::set_terrain_instance(uint id, const mat4& mat, const vec3& extent, const uvec2& blocks, uint tess_level, uint grass_field_tess_level, uint grass_channel, int grass_texture_id,
		graphics::ImageViewPtr height_map, graphics::ImageViewPtr normal_map, graphics::ImageViewPtr tangent_map, graphics::ImageViewPtr splash_map)
	{
		buf_terrain_ins.select_item(id);
		buf_terrain_ins.set_var<"mat"_h>(mat);
		buf_terrain_ins.set_var<"extent"_h>(extent);
		buf_terrain_ins.set_var<"blocks"_h>(blocks);
		buf_terrain_ins.set_var<"tess_level"_h>(tess_level);
		buf_terrain_ins.set_var<"grass_field_tess_level"_h>(grass_field_tess_level);
		buf_terrain_ins.set_var<"grass_channel"_h>(grass_channel);
		buf_terrain_ins.set_var<"grass_texture_id"_h>(grass_texture_id);
		ds_instance->set_image("terrain_height_maps"_h, id, height_map, nullptr);
		ds_instance->set_image("terrain_normal_maps"_h, id, normal_map, nullptr);
		ds_instance->set_image("terrain_tangent_maps"_h, id, tangent_map, nullptr);
		ds_instance->set_image("terrain_splash_maps"_h, id, splash_map, nullptr);
		ds_instance->update();
	}

	int sRendererPrivate::register_sdf_instance(int id)
	{
		if (id == -1)
		{
			id = buf_sdf_ins.get_free_item();
			if (id != -1)
			{

			}
		}
		else
		{
			buf_sdf_ins.release_item(id);
		}
		return id;
	}

	void sRendererPrivate::set_sdf_instance(uint id, uint boxes_count, std::pair<vec3, vec3>* boxes, uint spheres_count, std::pair<vec3, float>* spheres)
	{
		buf_sdf_ins.select_item(id);
		buf_sdf_ins.set_var<"boxes_count"_h>(boxes_count);
		auto boxes_dst = buf_sdf_ins.var_addr<"boxes"_h>();
		for (auto i = 0; i < boxes_count; i++)
		{
			auto p = boxes_dst + i * sizeof(vec4) * 2;
			*(vec4*)p = vec4(boxes[i].first, 0.f); p += sizeof(vec4);
			*(vec4*)p = vec4(boxes[i].second, 0.f);
		}
		buf_sdf_ins.set_var<"spheres_count"_h>(spheres_count);
		auto spheres_dst = buf_sdf_ins.var_addr<"spheres"_h>();
		for (auto i = 0; i < spheres_count; i++)
		{
			auto p = spheres_dst + i * sizeof(vec4);
			*(vec4*)p = vec4(spheres[i].first, spheres[i].second);
		}
	}

	static std::vector<std::vector<float>> gauss_blur_weights;
	static float* get_gauss_blur_weights(uint len)
	{
		if (gauss_blur_weights.empty())
		{
			gauss_blur_weights.push_back({                     0.047790, 0.904419, 0.047790 });
			gauss_blur_weights.push_back({           0.015885, 0.221463, 0.524950, 0.221463, 0.015885 });
			gauss_blur_weights.push_back({ 0.005977, 0.060598, 0.241730, 0.382925, 0.241730, 0.060598, 0.005977 });
		}

		auto radius = (len - 1) / 2 - 1;
		assert(radius < gauss_blur_weights.size());
		return gauss_blur_weights[radius].data();
	}

	void sRendererPrivate::render(uint tar_idx, graphics::CommandBufferPtr cb)
	{
		if (camera == INVALID_POINTER || iv_tars.empty())
			return;
		if (!camera)
		{
			auto& list = cCamera::list();
			if (list.empty())
				return;
			camera = list.front();
		}

		tar_idx = min(max(0, (int)iv_tars.size() - 1), (int)tar_idx);
		auto iv = iv_tars[tar_idx];
		auto img = iv->image;
		auto sz = vec2(img->size);

		camera->aspect = sz.x / sz.y;
		camera->update();

		camera_culled_nodes.clear();
		sScene::instance()->octree->get_within_frustum(camera->frustum, camera_culled_nodes);

		draw_data.reset("instance"_h, 0);
		for (auto n : camera_culled_nodes)
		{
			if (n->instance_frame < frames)
			{
				n->draw(draw_data);
				n->instance_frame = frames;
			}
		}

		buf_vtx.upload(cb);
		buf_idx.upload(cb);
		buf_vtx_arm.upload(cb);
		buf_idx_arm.upload(cb);

		buf_camera.item("zNear"_h).set(camera->zNear);
		buf_camera.item("zFar"_h).set(camera->zFar);
		buf_camera.item("fovy"_h).set(camera->fovy);
		buf_camera.item("tan_hf_fovy"_h).set((float)tan(radians(camera->fovy * 0.5f)));
		buf_camera.item("coord"_h).set(camera->node->g_pos);
		buf_camera.item("front"_h).set(-camera->node->g_rot[2]);
		buf_camera.item("right"_h).set(camera->node->g_rot[0]);
		buf_camera.item("up"_h).set(camera->node->g_rot[1]);
		buf_camera.item("view"_h).set(camera->view_mat);
		buf_camera.item("view_inv"_h).set(camera->view_mat_inv);
		buf_camera.item("proj"_h).set(camera->proj_mat);
		buf_camera.item("proj_inv"_h).set(camera->proj_mat_inv);
		buf_camera.item("proj_view"_h).set(camera->proj_view_mat);
		buf_camera.item("proj_view_inv"_h).set(camera->proj_view_mat_inv);
		memcpy(buf_camera.item("frustum_planes"_h).pdata, camera->frustum.planes, sizeof(vec4) * 6);
		buf_camera.mark_dirty(buf_camera);
		buf_camera.upload(cb);

		buf_lighting.set_var<"sky_intensity"_h>(sky_intensity);
		buf_lighting.set_var<"sky_rad_levels"_h>(sky_rad_levels);
		buf_lighting.set_var<"fog_color"_h>(fog_color);
		buf_lighting.upload(cb); 
		
		buf_material.upload(cb);

		auto n_dir_lights = 0;
		auto n_dir_shadows = 0;
		auto n_pt_lights = 0;
		auto n_pt_shadows = 0;
		if (mode == Shaded)
		{
			draw_data.reset("light"_h, 0);
			for (auto n : camera_culled_nodes)
			{
				n->draw(draw_data);
				if (n_dir_lights < draw_data.directional_lights.size())
				{
					for (auto i = n_dir_lights; i < draw_data.directional_lights.size(); i++)
					{
						auto& l = draw_data.directional_lights[i];

						if (l.cast_shadow)
						{
							if (n_dir_shadows < countof(dir_shadows))
							{
								auto idx = n_dir_shadows;
								buf_light_info.select_item(l.ins_id);
								buf_light_info.set_var<"shadow_index"_h>(idx);

								auto& rot = dir_shadows[idx].rot;
								rot = n->g_rot;
								rot[2] *= -1.f;

								n_dir_shadows++;
							}
						}

						buf_light_index.push(1, &l.ins_id); // push dir light index here
					}
					n_dir_lights = draw_data.directional_lights.size();
				}
			}

			// pack dir lights
			buf_light_grid.set_var<"offset"_h>(0);
			buf_light_grid.set_var<"count"_h>(n_dir_lights);
			buf_light_grid.next_item();
			// pack pt lights
			auto cx = max(1U, uint(sz.x / 16.f));
			auto cy = max(1U, uint(sz.y / 16.f));
			for (auto y = 0; y < cy; y++)
			{
				for (auto x = 0; x < cx; x++)
				{

				}
			}
		}
		else if (mode == CameraLight)
		{
			buf_light_info.select_item(camera_light_id);
			buf_light_info.set_var<"type"_h>(LightDirectional);
			buf_light_info.set_var<"pos"_h>(camera->node->g_rot[2]);
			buf_light_info.set_var<"color"_h>(vec3(1.f));
			buf_light_info.set_var<"shadow_index"_h>(-1);

			buf_light_index.push(1, &camera_light_id);

			buf_light_grid.set_var<"offset"_h>(0);
			buf_light_grid.set_var<"count"_h>(1);
			buf_light_grid.next_item();
		}

		buf_mesh_ins.upload(cb);
		buf_armature_ins.upload(cb);
		buf_terrain_ins.upload(cb);
		buf_sdf_ins.upload(cb);
		buf_light_index.upload(cb);
		buf_light_grid.upload(cb);
		buf_light_info.upload(cb);

		for (auto& d : opa_mesh_buckets.draw_idxs)
			d.second.second.clear();
		draw_data.reset("gbuffer"_h, "mesh"_h);
		for (auto n : camera_culled_nodes)
			n->draw(draw_data);
		opa_mesh_buckets.collect_idrs(draw_data, cb);

		cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutAttachment);
		cb->set_viewport_and_scissor(Rect(vec2(0), sz));

		// deferred shading pass

		cb->begin_renderpass(nullptr, fb_gbuf.get(),
			{ vec4(0.f, 0.f, 0.f, 1.f),
			vec4(0.f, 0.f, 0.f, 1.f),
			vec4(1.f, 0.f, 0.f, 0.f) });

		prm_gbuf.bind_dss(cb);
		opa_mesh_buckets.draw(cb);

		draw_data.reset("gbuffer"_h, "terrain"_h);
		for (auto n : camera_culled_nodes)
			n->draw(draw_data);
		for (auto& t : draw_data.terrains)
		{
			cb->bind_pipeline(get_material_pipeline(mat_reses[t.mat_id], "terrain"_h, 0, 0));
			cb->draw(4, t.blocks, 0, (t.ins_id << 24) + (t.mat_id << 16));
		}

		draw_data.reset("gbuffer"_h, "sdf"_h);
		for (auto n : camera_culled_nodes)
			n->draw(draw_data);
		for (auto& s : draw_data.sdfs)
		{
			cb->bind_pipeline(get_material_pipeline(mat_reses[s.mat_id], "sdf"_h, 0, 0));
			cb->draw(3, 1, 0, (s.ins_id << 16) + s.mat_id);
		}

		cb->end_renderpass();

		if (mode == Shaded)
		{
			for (auto i = 0; i < DirShadowMaxCount; i++)
			{
				auto& s = dir_shadows[i];
				for (auto j = 0; j < DirShadowMaxLevels; j++)
				{
					for (auto& d : s.mesh_buckets[j].draw_idxs)
						d.second.second.clear();
				}
			}

			auto zn = camera->zNear; auto zf = camera->zFar;
			for (auto i = 0; i < n_dir_shadows; i++)
			{
				auto& s = dir_shadows[i];
				auto splits = vec4(zf);
				auto mats = (mat4*)buf_dir_shadow.var_addr<"mats"_h>();
				for (auto lv = 0; lv < csm_levels; lv++)
				{
					auto n = lv / (float)csm_levels;
					auto f = (lv + 1) / (float)csm_levels;
					n = mix(zn, zf, n * n * shadow_distance / zf);
					f = mix(zn, zf, f * f * shadow_distance / zf);
					splits[lv] = f;

					{
						auto p = camera->proj_mat * vec4(0.f, 0.f, -n, 1.f);
						n = p.z / p.w;
					}
					{
						auto p = camera->proj_mat * vec4(0.f, 0.f, -f, 1.f);
						f = p.z / p.w;
					}
					auto frustum_slice = Frustum::get_points(camera->proj_view_mat_inv, n, f);
					if (csm_debug_sig)
						debug_primitives.emplace_back("LineList"_h, Frustum::points_to_lines(frustum_slice.data()), cvec4(156, 127, 0, 255));
					auto b = AABB(frustum_slice, inverse(s.rot));
					auto hf_xlen = (b.b.x - b.a.x) * 0.5f;
					auto hf_ylen = (b.b.y - b.a.y) * 0.5f;
					auto hf_zlen = (b.b.z - b.a.z) * 0.5f;
					auto c = s.rot * b.center();

					auto proj = orthoRH(-hf_xlen, +hf_xlen, -hf_ylen, +hf_ylen, 0.f, 20000.f);
					proj[1][1] *= -1.f;
					auto view = lookAt(c - s.rot[2] * 10000.f, c, s.rot[1]);
					auto proj_view = proj * view;

					s.culled_nodes.clear();
					sScene::instance()->octree->get_within_frustum(inverse(proj_view), s.culled_nodes);
					draw_data.reset("instance"_h, 0);
					for (auto n : s.culled_nodes)
					{
						if (n->instance_frame < frames)
						{
							n->draw(draw_data);
							n->instance_frame = frames;
						}
					}

					auto z_min = -hf_zlen;
					auto z_max = +hf_zlen;
					int n_draws;

					draw_data.reset("occulder"_h, "mesh"_h);
					n_draws = 0;
					for (auto n : s.culled_nodes)
					{
						n->draw(draw_data);
						if (draw_data.meshes.size() > n_draws)
						{
							auto r = n->bounds.radius();
							auto d = dot(n->g_pos - c, s.rot[2]);
							z_min = min(d - r, z_min);
							z_max = max(d + r, z_max);

							n_draws = draw_data.meshes.size();
						}
					}
					s.mesh_buckets[lv].collect_idrs(draw_data, cb, "OCCLUDER_PASS"_h);

					draw_data.reset("occulder"_h, "terrain"_h);
					n_draws = 0;
					for (auto n : s.culled_nodes)
					{
						n->draw(draw_data);
						if (draw_data.terrains.size() > n_draws)
						{
							for (auto& p : n->bounds.get_points())
							{
								auto d = dot(p - c, s.rot[2]);
								z_min = min(d, z_min);
								z_max = max(d, z_max);
							}

							n_draws = draw_data.terrains.size();
						}
					}
					s.draw_terrains[lv] = draw_data.terrains;

					proj = orthoRH(-hf_xlen, +hf_xlen, -hf_ylen, +hf_ylen, 0.f, z_max - z_min);
					proj[1][1] *= -1.f;
					view = lookAt(c + s.rot[2] * z_min, c, s.rot[1]);
					proj_view = proj * view;
					mats[lv] = proj_view;
					if (csm_debug_sig)
					{
						auto frustum_points = Frustum::get_points(inverse(proj_view));
						debug_primitives.emplace_back("LineList"_h, Frustum::points_to_lines(frustum_points.data()), cvec4(255, 127, 0, 255));
						auto c = (frustum_points[0] + frustum_points[6]) * 0.5f;
						vec3 pts[2];
						pts[0] = c; pts[1] = c + s.rot[0] * hf_xlen;
						debug_primitives.emplace_back("LineList"_h, pts, 2, cvec4(255, 0, 0, 255));
						pts[0] = c; pts[1] = c + s.rot[1] * hf_ylen;
						debug_primitives.emplace_back("LineList"_h, pts, 2, cvec4(0, 255, 0, 255));
						pts[0] = c; pts[1] = c + s.rot[2] * hf_zlen;
						debug_primitives.emplace_back("LineList"_h, pts, 2, cvec4(0, 0, 255, 255));
					}
				}

				buf_dir_shadow.set_var<"splits"_h>(splits);
				buf_dir_shadow.set_var<"far"_h>(shadow_distance);
				buf_dir_shadow.next_item();
			}

			csm_debug_sig = false;

			buf_dir_shadow.upload(cb);
			buf_pt_shadow.upload(cb);

			auto set_blur_args = [cb](const vec2 img_size) {
				cb->bind_pipeline_layout(prm_post.pll);
				prm_post.pc.item("off"_h).set(-3);
				prm_post.pc.item("len"_h).set(7);
				prm_post.pc.item("pxsz"_h).set(1.f / img_size);
				prm_post.pc.item("weights"_h).set(get_gauss_blur_weights(7), sizeof(float) * 7);
				prm_post.push_constant(cb);
			};

			cb->set_viewport_and_scissor(Rect(vec2(0), ShadowMapSize));
			for (auto i = 0; i < n_dir_shadows; i++)
			{
				auto& s = dir_shadows[i];
				for (auto lv = 0U; lv < csm_levels; lv++)
				{
					cb->begin_renderpass(nullptr, imgs_dir_shadow[i]->get_shader_write_dst(0, lv, graphics::AttachmentLoadClear), { vec4(1.f, 0.f, 0.f, 0.f) });
					prm_fwd.bind_dss(cb);
					prm_fwd.pc.item("i"_h).set(ivec4(0, i, lv, 0));
					prm_fwd.push_constant(cb);

					s.mesh_buckets[lv].draw(cb);

					for (auto& t : s.draw_terrains[lv])
					{
						cb->bind_pipeline(get_material_pipeline(mat_reses[t.mat_id], "terrain"_h, 0, "OCCLUDER_PASS"_h));
						cb->draw(4, t.blocks, 0, (t.ins_id << 24) + (t.mat_id << 16));
					}

					cb->end_renderpass();
				}
				cb->image_barrier(imgs_dir_shadow[i].get(), { 0U, 1U, 0U, csm_levels }, graphics::ImageLayoutShaderReadOnly);
			}

			set_blur_args(vec2(ShadowMapSize));
			cb->bind_pipeline_layout(prm_post.pll);
			for (auto i = 0; i < n_dir_shadows; i++)
			{
				for (auto lv = 0U; lv < csm_levels; lv++)
				{
					cb->image_barrier(img_dir_shadow_back.get(), { 0U, 1U, lv, 1 }, graphics::ImageLayoutAttachment);
					cb->begin_renderpass(nullptr, img_dir_shadow_back->get_shader_write_dst(0, lv));
					cb->bind_pipeline(pl_blur_dep_h);
					cb->bind_descriptor_set(0, imgs_dir_shadow[i]->get_shader_read_src(0, lv));
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->image_barrier(img_dir_shadow_back.get(), { 0U, 1U, lv, 1 }, graphics::ImageLayoutShaderReadOnly);
					cb->begin_renderpass(nullptr, imgs_dir_shadow[i]->get_shader_write_dst(0, lv));
					cb->bind_pipeline(pl_blur_dep_v);
					cb->bind_descriptor_set(0, img_dir_shadow_back->get_shader_read_src(0, lv));
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();
				}
				cb->image_barrier(imgs_dir_shadow[i].get(), { 0U, 1U, 0U, csm_levels }, graphics::ImageLayoutShaderReadOnly);
			}

			cb->set_viewport_and_scissor(Rect(vec2(0), ShadowMapSize / 2U));
			for (auto i = 0; i < n_pt_shadows; i++)
			{

			}
		}

		cb->set_viewport_and_scissor(Rect(vec2(0), sz));

		auto pl_mod = 0;
		switch (mode)
		{
		case AlbedoData: pl_mod = "ALBEDO_DATA"_h; break;
		case NormalData: pl_mod = "NORMAL_DATA"_h; break;
		case MetallicData: pl_mod = "METALLIC_DATA"_h; break;
		case RoughnessData: pl_mod = "ROUGHNESS_DATA"_h; break;
		case IBLValue: pl_mod = "IBL_VALUE"_h; break;
		case FogValue: pl_mod = "FOG_VALUE"_h; break;
		}

		cb->image_barrier(img_col_met.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->image_barrier(img_nor_rou.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->image_barrier(img_ao.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->image_barrier(img_dep.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->begin_renderpass(nullptr, img_dst->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
		prm_deferred.bind_dss(cb);
		cb->bind_pipeline(get_deferred_pipeline(pl_mod));
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();

		// forward pass

		cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->begin_renderpass(nullptr, img_dst_ms->get_shader_write_dst());
		cb->bind_pipeline(pl_blit);
		cb->bind_descriptor_set(0, img_dst->get_shader_read_src());
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();

		cb->image_barrier(img_dep.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->begin_renderpass(nullptr, img_dep_ms->get_shader_write_dst());
		cb->bind_pipeline(pl_blit_dep);
		cb->bind_descriptor_set(0, img_dep->get_shader_read_src());
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();

		for (auto& d : trs_mesh_buckets.draw_idxs)
			d.second.second.clear();
		draw_data.reset("forward"_h, "mesh"_h);
		for (auto n : camera_culled_nodes)
			n->draw(draw_data);
		trs_mesh_buckets.collect_idrs(draw_data, cb);

		cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutAttachment);
		cb->image_barrier(img_dep.get(), {}, graphics::ImageLayoutAttachment);
		cb->begin_renderpass(nullptr, fb_fwd.get());
		prm_fwd.bind_dss(cb);

		trs_mesh_buckets.draw(cb);

		draw_data.reset("forward"_h, "grass_field"_h);
		for (auto n : camera_culled_nodes)
			n->draw(draw_data);
		for (auto& t : draw_data.terrains)
		{
			cb->bind_pipeline(get_material_pipeline(mat_reses[t.mat_id], "grass_field"_h, 0, 0));
			cb->draw(4, t.blocks, 0, (t.ins_id << 24) + (t.mat_id << 16));
		}

		cb->end_renderpass();

		// post processing

		cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutGeneral);
		prm_luma.bind_dss(cb);
		const auto min_log_luma = -5.f;
		const auto max_log_luma = +5.f;
		prm_luma.pc.item("min_log_luma"_h).set(min_log_luma);
		prm_luma.pc.item("log_luma_range"_h).set(max_log_luma - min_log_luma);
		prm_luma.pc.item("time_coeff"_h).set(1.0f);
		prm_luma.pc.item("num_pixels"_h).set(int(sz.x * sz.y));
		prm_luma.push_constant(cb);
		cb->bind_pipeline(pl_luma_hist);
		cb->dispatch(uvec3(ceil(sz.x / 16), ceil(sz.y / 16), 1));
		cb->buffer_barrier(buf_luma_hist.buf.get(), graphics::AccessShaderRead | graphics::AccessShaderWrite,
			graphics::AccessShaderRead | graphics::AccessShaderWrite,
			graphics::PipelineStageCompShader, graphics::PipelineStageCompShader);
		cb->bind_pipeline(pl_luma_avg);
		cb->dispatch(uvec3(256, 1, 1));
		cb->buffer_barrier(buf_luma_avg.buf.get(), graphics::AccessShaderRead | graphics::AccessShaderWrite,
			graphics::AccessShaderRead,
			graphics::PipelineStageCompShader, graphics::PipelineStageAllGraphics);

		cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst());
		cb->bind_pipeline(pl_blit);
		cb->bind_descriptor_set(0, img_dst->get_shader_read_src());
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();

		cb->image_barrier(img_back0.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->begin_renderpass(nullptr, img_back1->get_shader_write_dst());
		cb->bind_pipeline(pl_tone);
		prm_tone.bind_dss(cb);
		prm_tone.pc.item("white_point"_h).set(white_point);
		prm_tone.pc.item("one_over_gamma"_h).set(1.f / gamma);
		prm_tone.push_constant(cb);
		cb->bind_descriptor_set(1, img_back0->get_shader_read_src());
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();

		cb->image_barrier(img_back1.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->begin_renderpass(nullptr, img_dst->get_shader_write_dst());
		cb->bind_pipeline(pl_fxaa);
		prm_post.pc.item("pxsz"_h).set(1.f / (vec2)img_dst->size);
		prm_post.push_constant(cb);
		cb->bind_descriptor_set(0, img_back1->get_shader_read_src());
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();

		auto blur_pass = [&]() {
			cb->bind_pipeline_layout(prm_post.pll);
			prm_post.pc.item("off"_h).set(-3);
			prm_post.pc.item("len"_h).set(7);
			prm_post.pc.item("pxsz"_h).set(1.f / (vec2)img_back0->size);
			prm_post.push_constant(cb);

			cb->image_barrier(img_back0.get(), {}, graphics::ImageLayoutShaderReadOnly);
			cb->begin_renderpass(nullptr, img_back1->get_shader_write_dst());
			cb->bind_pipeline(pl_localmax_h);
			cb->bind_descriptor_set(0, img_back0->get_shader_read_src());
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();

			cb->image_barrier(img_back1.get(), {}, graphics::ImageLayoutShaderReadOnly);
			cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst());
			cb->bind_pipeline(pl_localmax_v);
			cb->bind_descriptor_set(0, img_back1->get_shader_read_src());
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();
		};

		auto blend_pass = [&]() {
			cb->image_barrier(img_back0.get(), {}, graphics::ImageLayoutShaderReadOnly);
			cb->begin_renderpass(nullptr, img_dst->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
			cb->bind_pipeline(pl_blend);
			cb->bind_descriptor_set(0, img_back0->get_shader_read_src());
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();
		};

		draw_data.reset("outline"_h, 0);
		for (auto n : camera_culled_nodes)
			n->draw(draw_data);
		auto outline_idx = 0;
		std::vector<uint> outline_groups;
		outline_groups.push_back(0);
		for (auto i = 0; i < draw_data.meshes.size(); i++)
		{
			outline_groups.back()++;
			if (draw_data.meshes[i].mat_id == 0)
				outline_groups.push_back(0);
		}
		for (auto n : outline_groups)
		{
			cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), { vec4(0.f) });
			for (auto i = 0; i < n; i++)
			{
				auto& m = draw_data.meshes[i];
				auto& mesh_r = mesh_reses[m.mesh_id];

				prm_fwd.bind_dss(cb);
				prm_fwd.pc.item("f"_h).set(vec4(m.color) / 255.f);
				prm_fwd.push_constant(cb);

				if (!mesh_r.arm)
				{
					cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
					cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
					cb->bind_pipeline(pl_mesh_plain);
					cb->draw_indexed(mesh_r.idx_cnt, mesh_r.idx_off, mesh_r.vtx_off, 1, m.ins_id << 8);
				}
				else
				{
					cb->bind_vertex_buffer(buf_vtx_arm.buf.get(), 0);
					cb->bind_index_buffer(buf_idx_arm.buf.get(), graphics::IndiceTypeUint);
					cb->bind_pipeline(pl_mesh_arm_plain);
					cb->draw_indexed(mesh_r.idx_cnt, mesh_r.idx_off, mesh_r.vtx_off, 1, m.ins_id << 8);
				}
			}
			cb->end_renderpass();

			blur_pass();
			cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
			for (auto i = 0; i < n; i++)
			{
				auto& m = draw_data.meshes[i];
				auto& mesh_r = mesh_reses[m.mesh_id];

				prm_fwd.bind_dss(cb);
				prm_fwd.pc.item("f"_h).set(vec4(0.f));
				prm_fwd.push_constant(cb);

				if (!mesh_r.arm)
				{
					cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
					cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
					cb->bind_pipeline(pl_mesh_plain);
					cb->draw_indexed(mesh_r.idx_cnt, mesh_r.idx_off, mesh_r.vtx_off, 1, m.ins_id << 8);
				}
				else
				{
					cb->bind_vertex_buffer(buf_vtx_arm.buf.get(), 0);
					cb->bind_index_buffer(buf_idx_arm.buf.get(), graphics::IndiceTypeUint);
					cb->bind_pipeline(pl_mesh_arm_plain);
					cb->draw_indexed(mesh_r.idx_cnt, mesh_r.idx_off, mesh_r.vtx_off, 1, m.ins_id << 8);

				}
			}
			cb->end_renderpass();
			blend_pass();

			outline_idx += n;
		}
		for (auto& m : draw_data.meshes)
		{
			auto& mesh_r = mesh_reses[m.mesh_id];
		}
		for (auto& t : draw_data.terrains)
		{
			cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), { vec4(0.f) });
			prm_fwd.bind_dss(cb);
			prm_fwd.pc.item("f"_h).set(vec4(t.color) / 255.f);
			prm_fwd.push_constant(cb);
			cb->bind_pipeline(pl_terrain_plain);
			cb->draw(4, t.blocks, 0, t.ins_id << 24);
			cb->end_renderpass();

			blur_pass();

			cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
			prm_fwd.bind_dss(cb);
			prm_fwd.pc.item("f"_h).set(vec4(0.f));
			prm_fwd.push_constant(cb);
			cb->bind_pipeline(pl_terrain_plain);
			cb->draw(4, t.blocks, 0, t.ins_id << 24);
			cb->end_renderpass();

			blend_pass();
		}

		draw_data.reset("primitive"_h, 0);
		for (auto n : camera_culled_nodes)
			n->draw(draw_data);
		if (!debug_primitives.empty())
			draw_data.primitives.insert(draw_data.primitives.end(), debug_primitives.begin(), debug_primitives.end());
		for (auto& l : draw_data.primitives)
		{
			for (auto& p : l.points)
			{
				auto pv = buf_primitives.add();
				pv.item("i_pos"_h).set(p);
			}
		}
		buf_primitives.upload(cb);
		buf_primitives.buf_top = buf_primitives.stag_top = 0;
		cb->begin_renderpass(nullptr, img_dst->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
		cb->bind_vertex_buffer(buf_primitives.buf.get(), 0);
		cb->bind_pipeline_layout(prm_plain.pll);
		prm_plain.pc.item("mvp"_h).set(camera->proj_view_mat);
		prm_plain.push_constant(cb);
		auto primitive_vtx_off = 0;
		for (auto& d : draw_data.primitives)
		{
			auto col_item = prm_plain.pc.item("col"_h);
			col_item.set(vec4(d.color) / 255.f);
			prm_plain.pc.mark_dirty(col_item);
			prm_plain.push_constant(cb);
			switch (d.type)
			{
			case "LineList"_h:
				cb->bind_pipeline(pl_line3d);
				break;
			case "TriangleList"_h:
				cb->bind_pipeline(pl_triangle3d);
				break;
			}
			cb->draw(d.points.size(), 1, primitive_vtx_off, 0);
			primitive_vtx_off += d.points.size();
		}
		cb->end_renderpass();

		cb->image_barrier(img, iv->sub, graphics::ImageLayoutAttachment);
		cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->begin_renderpass(nullptr, img->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
		cb->bind_pipeline(pl_blit);
		cb->bind_descriptor_set(0, img_dst->get_shader_read_src());
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();
		cb->image_barrier(img, iv->sub, final_layout);
	}

	void sRendererPrivate::update()
	{
	}

	cNodePtr sRendererPrivate::pick_up(const uvec2& screen_pos, vec3* out_pos, const std::function<void(cNodePtr, DrawData&)>& draw_callback)
	{
		if (camera == INVALID_POINTER)
			return nullptr;
		if (!camera)
		{
			auto& list = cCamera::list();
			if (list.empty())
				return nullptr;
			camera = list.front();
		}

		auto sz = vec2(img_pickup->size);
		if (screen_pos.x >= sz.x || screen_pos.y >= sz.y)
			return nullptr;

		graphics::InstanceCommandBuffer cb(fence_pickup.get());

		cb->set_viewport(Rect(vec2(0), sz));
		cb->set_scissor(Rect(vec2(screen_pos), vec2(screen_pos + 1U)));

		cb->begin_renderpass(nullptr, fb_pickup.get(), { vec4(0.f),
			vec4(1.f, 0.f, 0.f, 0.f) });

		prm_fwd.bind_dss(cb.get());

		std::vector<cNodePtr> nodes;

		auto off = 0;
		auto n_draws = 0;
		draw_data.reset("pick_up"_h, "mesh"_h);
		std::vector<cNodePtr> camera_culled_nodes; // collect here (again), because there may have changes between render() and pick_up()
		sScene::instance()->octree->get_within_frustum(camera->frustum, camera_culled_nodes);
		for (auto n : camera_culled_nodes)
		{
			if (draw_callback)
				draw_callback(n, draw_data);
			else
				n->draw(draw_data);

			for (auto i = n_draws; i < draw_data.meshes.size(); i++)
			{
				nodes.push_back(n);

				auto& m = draw_data.meshes[i];
				auto& mesh_r = mesh_reses[m.mesh_id];
				if (!mesh_r.arm)
				{
					cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
					cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
					cb->bind_pipeline(pl_mesh_pickup);
					prm_fwd.pc.item("i"_h).set(ivec4(i + 1, 0, 0, 0));
					prm_fwd.push_constant(cb.get());
					cb->draw_indexed(mesh_r.idx_cnt, mesh_r.idx_off, mesh_r.vtx_off, 1, m.ins_id << 8);
				}
				else
				{
					cb->bind_vertex_buffer(buf_vtx_arm.buf.get(), 0);
					cb->bind_index_buffer(buf_idx_arm.buf.get(), graphics::IndiceTypeUint);
					cb->bind_pipeline(pl_mesh_arm_pickup);
					prm_fwd.pc.item("i"_h).set(ivec4(i + 1, 0, 0, 0));
					prm_fwd.push_constant(cb.get());
					cb->draw_indexed(mesh_r.idx_cnt, mesh_r.idx_off, mesh_r.vtx_off, 1, m.ins_id << 8);
				}
			}
			n_draws = draw_data.meshes.size();
		}

		off = nodes.size();
		n_draws = 0;
		draw_data.reset("pick_up"_h, "terrain"_h);
		cb->bind_pipeline(pl_terrain_pickup);
		for (auto n : camera_culled_nodes)
		{
			if (draw_callback)
				draw_callback(n, draw_data);
			else
				n->draw(draw_data);

			for (auto i = n_draws; i < draw_data.terrains.size(); i++)
			{
				nodes.push_back(n);

				auto& t = draw_data.terrains[i];
				prm_fwd.pc.item("i"_h).set(ivec4(i + 1 + off, 0, 0, 0));
				prm_fwd.push_constant(cb.get());
				cb->draw(4, t.blocks, 0, t.ins_id << 24);
			}
			n_draws = draw_data.terrains.size();
		}

		cb->end_renderpass();
		if (draw_data.graphics_debug)
			graphics::Debug::start_capture_frame();
		cb.excute();
		if (draw_data.graphics_debug)
			graphics::Debug::end_capture_frame();

		int index; ushort depth;
		graphics::StagingBuffer sb(sizeof(index) + sizeof(depth), nullptr, graphics::BufferUsageTransferDst);
		{
			graphics::InstanceCommandBuffer cb(nullptr);
			graphics::BufferImageCopy cpy;
			cpy.img_off = screen_pos;
			cpy.img_ext = uvec2(1U);
			cb->image_barrier(img_pickup.get(), cpy.img_sub, graphics::ImageLayoutTransferSrc);
			cb->copy_image_to_buffer(img_pickup.get(), sb.get(), { &cpy, 1 });
			cb->image_barrier(img_pickup.get(), cpy.img_sub, graphics::ImageLayoutAttachment);
			if (out_pos)
			{
				cpy.buf_off = sizeof(uint);
				cb->image_barrier(img_dep_pickup.get(), cpy.img_sub, graphics::ImageLayoutTransferSrc);
				cb->copy_image_to_buffer(img_dep_pickup.get(), sb.get(), { &cpy, 1 });
				cb->image_barrier(img_dep_pickup.get(), cpy.img_sub, graphics::ImageLayoutAttachment);
			}
			cb.excute();
		}
		memcpy(&index, sb->mapped, sizeof(index));
		if (out_pos)
		{
			memcpy(&depth, (char*)sb->mapped + sizeof(index), sizeof(depth));
			auto depth_f = depth / 65535.f;
			auto p = vec4(vec2(screen_pos) / sz * 2.f - 1.f, depth_f, 1.f);
			p = camera->proj_mat_inv * p;
			p /= p.w;
			p = camera->view_mat_inv * p;
			*out_pos = p;
		}
		index -= 1;
		if (index == -1)
			return nullptr;
		return nodes[index];
	}

	void sRendererPrivate::send_debug_string(const std::string& str)
	{
		if (str == "clear_debug")
			debug_primitives.clear();
		else if (str == "sig_csm_debug")
			csm_debug_sig = true;
	}

	static sRendererPtr _instance = nullptr;

	struct sRendererInstance : sRenderer::Instance
	{
		sRendererPtr operator()() override
		{
			return _instance;
		}
	}sRenderer_instance;
	sRenderer::Instance& sRenderer::instance = sRenderer_instance;

	struct sRendererCreatePrivate : sRenderer::Create
	{
		sRendererPtr operator()(WorldPtr w) override
		{
			if (!w)
				return new sRendererPrivate();

			assert(!_instance);

			auto& windows = graphics::Window::get_list();
			if (windows.empty())
			{
				printf("node renderer system needs graphics window\n");
				return nullptr;
			}

			_instance = new sRendererPrivate(windows[0]);
			return _instance;
		}
	}sRenderer_create;
	sRenderer::Create& sRenderer::create = sRenderer_create;
}
