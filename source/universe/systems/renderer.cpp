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

	int sky_map_res_id = -1;
	int sky_irr_map_res_id = -1;
	int sky_rad_map_res_id = -1;
	float sky_rad_levels = 1.f;
	float sky_intensity = 1.f;
	vec3 fog_color = vec3(1.f);
	float white_point = 4.f;
	float gamma = 1.5f;
	uint csm_levels = 3;
	float shadow_distance = 0.1f; // (0-1) of camera's far
	float ssao_radius = 0.5f;
	float ssao_bias = 0.025f;

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
	std::vector<std::unique_ptr<graphics::Image>> imgs_dir_shadow;
	std::vector<std::unique_ptr<graphics::Image>> imgs_pt_shadow;

	graphics::RenderpassPtr rp_fwd = nullptr;
	graphics::RenderpassPtr rp_gbuf = nullptr;
	graphics::RenderpassPtr rp_dep = nullptr;
	graphics::RenderpassPtr rp_col_dep = nullptr;
	std::unique_ptr<graphics::Framebuffer> fb_fwd;
	std::unique_ptr<graphics::Framebuffer> fb_gbuf;
	std::unique_ptr<graphics::Framebuffer> fb_pickup;
	graphics::PipelineLayoutPtr pll_fwd = nullptr;
	graphics::PipelineLayoutPtr pll_gbuf = nullptr;
	graphics::PipelineResourceManager<FLAME_UID> prm_fwd;
	graphics::PipelineResourceManager<FLAME_UID> prm_gbuf;
	graphics::PipelineResourceManager<FLAME_UID> prm_deferred;
	graphics::PipelineResourceManager<FLAME_UID> prm_plain;
	graphics::PipelineResourceManager<FLAME_UID> prm_post;
	graphics::PipelineResourceManager<FLAME_UID> prm_luma;
	graphics::PipelineResourceManager<FLAME_UID> prm_tone;

	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageVertex, false> buf_vtx;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageIndex, false> buf_idx;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageVertex, false> buf_vtx_arm;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageIndex, false> buf_idx_arm;

	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageUniform, false>			buf_scene;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_mesh_ins;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_armature_ins;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_terrain_ins;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_grassfield_ins;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_sdf_ins;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageUniform, false>			buf_material_system;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_material_info;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageUniform, false>			buf_lighting;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage>				buf_light_index;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage>				buf_light_grid;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_light_info;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage>				buf_dir_shadow;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage>				buf_pt_shadow;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageVertex>					buf_primitives;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_luma_avg;
	graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage, false, true>	buf_luma_hist;

	std::unique_ptr<graphics::DescriptorSet> ds_scene;
	std::unique_ptr<graphics::DescriptorSet> ds_instance;
	std::unique_ptr<graphics::DescriptorSet> ds_material;
	std::unique_ptr<graphics::DescriptorSet> ds_light;
	std::unique_ptr<graphics::DescriptorSet> ds_deferred;
	std::unique_ptr<graphics::DescriptorSet> ds_luma_avg;
	std::unique_ptr<graphics::DescriptorSet> ds_luma;

	graphics::GraphicsPipelinePtr pl_blit = nullptr;
	graphics::GraphicsPipelinePtr pl_blit_ms = nullptr;
	graphics::GraphicsPipelinePtr pl_blit_dep_ms = nullptr;
	graphics::GraphicsPipelinePtr pl_add = nullptr;
	graphics::GraphicsPipelinePtr pl_blend = nullptr;
	graphics::GraphicsPipelinePtr pl_blur_h = nullptr;
	graphics::GraphicsPipelinePtr pl_blur_v = nullptr;
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

		void collect_idrs(graphics::CommandBufferPtr cb, uint mod2 = 0);
		void draw(graphics::CommandBufferPtr cb);
	};

	struct DirShadow
	{
		mat3 rot;
		std::vector<cNodePtr> culled_nodes;
		MeshBuckets mesh_buckets[DirShadowMaxLevels];
	};

	std::vector<cNodePtr> camera_culled_nodes;
	DrawData draw_data;
	MeshBuckets mesh_buckets;
	DirShadow dir_shadows[DirShadowMaxCount];

	std::vector<PrimitiveDraw> debug_primitives;
	bool csm_debug_sig = false;

	static graphics::GraphicsPipelinePtr get_material_pipeline(sRenderer::MatRes& mr, uint type, uint modifier1 = 0, uint modifier2 = 0)
	{
		auto key = type + modifier1 + modifier2;
		auto it = mr.pls.find(key);
		if (it != mr.pls.end())
			return it->second;

		rotate(mat4(1.f), 90.f, vec3(1.f));

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
			else
			{
				defines.push_back("rp=" + str(rp_gbuf));
				defines.push_back("pll=" + str(pll_gbuf));
				defines.push_back("all_shader:GBUFFER_PASS");
			}
			break;
		case "grass_field"_h:
			defines.push_back("rp=" + str(rp_fwd));
			defines.push_back("pll=" + str(pll_fwd));
			break;
		}

		auto mat_file = Path::get(mr.mat->shader_file).string();
		defines.push_back(std::format("frag:MAT_FILE={}", mat_file));
		if (mr.mat->color_map != -1)
		{
			auto found = false;
			for (auto& d : mr.mat->shader_defines)
			{
				if (d.find(":COLOR_MAP") != std::string::npos)
				{
					found = true;
					break;
				}
			}
			if (!found)
				defines.push_back(std::format("frag:COLOR_MAP={}", mr.mat->color_map));
		}
		defines.insert(defines.end(), mr.mat->shader_defines.begin(), mr.mat->shader_defines.end());

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

	void MeshBuckets::collect_idrs(graphics::CommandBufferPtr cb, uint mod2)
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

		auto dsl_scene = graphics::DescriptorSetLayout::get(L"flame\\shaders\\scene.dsl");
		buf_scene.create(dsl_scene->get_buf_ui("Scene"_h));
		ds_scene.reset(graphics::DescriptorSet::create(nullptr, dsl_scene));
		ds_scene->set_buffer("Scene"_h, 0, buf_scene.buf.get());
		ds_scene->update();
		auto dsl_instance = graphics::DescriptorSetLayout::get(L"flame\\shaders\\instance.dsl");
		buf_mesh_ins.create_with_array_type(dsl_instance->get_buf_ui("MeshInstances"_h));
		buf_armature_ins.create_with_array_type(dsl_instance->get_buf_ui("ArmatureInstances"_h));
		buf_terrain_ins.create_with_array_type(dsl_instance->get_buf_ui("TerrainInstances"_h));
		buf_grassfield_ins.create_with_array_type(dsl_instance->get_buf_ui("GrassFieldInstances"_h));
		buf_sdf_ins.create_with_array_type(dsl_instance->get_buf_ui("SdfInstances"_h));
		ds_instance.reset(graphics::DescriptorSet::create(nullptr, dsl_instance));
		ds_instance->set_buffer("MeshInstances"_h, 0, buf_mesh_ins.buf.get());
		ds_instance->set_buffer("ArmatureInstances"_h, 0, buf_armature_ins.buf.get());
		ds_instance->set_buffer("TerrainInstances"_h, 0, buf_terrain_ins.buf.get());
		ds_instance->set_buffer("GrassFieldInstances"_h, 0, buf_grassfield_ins.buf.get());
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
		buf_material_system.create(dsl_material->get_buf_ui("MaterialSystem"_h));
		buf_material_info.create_with_array_type(dsl_material->get_buf_ui("MaterialInfos"_h));
		mat_reses.resize(buf_material_info.array_capacity);
		get_material_res(graphics::Material::get(L"default"), -1);
		tex_reses.resize(dsl_material->get_binding("material_maps"_h).count);
		ds_material.reset(graphics::DescriptorSet::create(nullptr, dsl_material));
		ds_material->set_buffer("MaterialSystem"_h, 0, buf_material_system.buf.get());
		ds_material->set_buffer("MaterialInfos"_h, 0, buf_material_info.buf.get());
		for (auto i = 0; i < tex_reses.size(); i++)
			ds_material->set_image("material_maps"_h, i, img_black->get_view(), nullptr);
		buf_material_system.set_var<"black_map_id"_h>(get_texture_res(img_black->get_view(), nullptr, -1));
		buf_material_system.set_var<"white_map_id"_h>(get_texture_res(img_white->get_view(), nullptr, -1));
		{
			auto img = graphics::Image::get(L"flame\\random.png");
			buf_material_system.set_var<"random_map_id"_h>(img ? get_texture_res(img->get_view(),
				graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressRepeat), -1) : -1);
		}
		{
			graphics::InstanceCommandBuffer cb;
			buf_material_system.upload(cb.get());
			cb.excute();
		}
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
		imgs_pt_shadow.resize(dsl_light->get_binding("pt_shadow_maps"_h).count);
		for (auto& i : imgs_pt_shadow)
		{
			i.reset(graphics::Image::create(dep_fmt, ShadowMapSize, graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 1, 6, graphics::SampleCount_1, true));
			i->change_layout(graphics::ImageLayoutShaderReadOnly);
		}
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
		buf_primitives.create(pl_line3d->vi_ui(), 1024 * 32);

		pll_fwd = graphics::PipelineLayout::get(L"flame\\shaders\\forward.pll");
		pll_gbuf = graphics::PipelineLayout::get(L"flame\\shaders\\gbuffer.pll");

		auto rp_col_ms = graphics::Renderpass::get(L"flame\\shaders\\color.rp", { "sample_count=" + TypeInfo::serialize_t(sample_count) });
		auto rp_dep_ms = graphics::Renderpass::get(L"flame\\shaders\\depth.rp", { "sample_count=" + TypeInfo::serialize_t(sample_count) });

		pl_blit = graphics::GraphicsPipeline::get(L"flame\\shaders\\blit.pipeline", {});
		pl_blit->dynamic_renderpass = true;
		pl_blit_ms = graphics::GraphicsPipeline::get(L"flame\\shaders\\blit.pipeline", { "rp=" + str(rp_col_ms) });
		pl_blit_ms->dynamic_renderpass = true;
		pl_blit_dep_ms = graphics::GraphicsPipeline::get(L"flame\\shaders\\blit.pipeline", { "rp=" + str(rp_dep_ms), "frag:DEPTH"});
		pl_blit_dep_ms->dynamic_renderpass = true;
		pl_add = graphics::GraphicsPipeline::get(L"flame\\shaders\\add.pipeline", {});
		pl_add->dynamic_renderpass = true;
		pl_blend = graphics::GraphicsPipeline::get(L"flame\\shaders\\blend.pipeline", {});
		pl_blend->dynamic_renderpass = true;

		prm_fwd.init(pll_fwd);
		prm_fwd.set_ds("scene"_h, ds_scene.get());
		prm_fwd.set_ds("instance"_h, ds_instance.get());
		prm_fwd.set_ds("material"_h, ds_material.get());
		prm_fwd.set_ds("light"_h, ds_light.get());

		prm_gbuf.init(pll_gbuf);
		prm_gbuf.set_ds("scene"_h, ds_scene.get());
		prm_gbuf.set_ds("instance"_h, ds_instance.get());
		prm_gbuf.set_ds("material"_h, ds_material.get());

		pl_mesh_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline", { "rp=" + str(rp_col_dep) });
		pl_mesh_plain->dynamic_renderpass = true;
		pl_mesh_arm_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline", { "rp=" + str(rp_col_dep), "vert:ARMATURE" });
		pl_mesh_arm_plain->dynamic_renderpass = true;
		pl_terrain_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\terrain\\terrain.pipeline", { "rp=" + str(rp_col_dep) });
		pl_terrain_plain->dynamic_renderpass = true;

		buf_vtx.create(pl_mesh_plain->vi_ui(), 1024 * 256 * 4);
		buf_idx.create(sizeof(uint), 1024 * 256 * 6);
		buf_vtx_arm.create(pl_mesh_arm_plain->vi_ui(), 1024 * 128 * 4);
		buf_idx_arm.create(sizeof(uint), 1024 * 128 * 6);
		mesh_buckets.buf_idr.create(0U, buf_mesh_ins.array_capacity);
		for (auto& s : dir_shadows)
		{
			for (auto i = 0; i < DirShadowMaxLevels; i++)
				s.mesh_buckets[i].buf_idr.create(0U, min(1024U, buf_mesh_ins.array_capacity));
		}

		prm_deferred.init(get_deferred_pipeline()->layout);
		prm_deferred.set_ds("scene"_h, ds_scene.get());
		prm_deferred.set_ds("light"_h, ds_light.get());
		ds_deferred.reset(graphics::DescriptorSet::create(nullptr, prm_deferred.pll->dsls.back()));
		prm_deferred.set_ds(""_h, ds_deferred.get());

		prm_post.init(graphics::PipelineLayout::get(L"flame\\shaders\\post\\post.pll"));
		pl_blur_h = graphics::GraphicsPipeline::get(L"flame\\shaders\\post\\blur.pipeline", { "frag:HORIZONTAL" });
		pl_blur_h->dynamic_renderpass = true;
		pl_blur_v = graphics::GraphicsPipeline::get(L"flame\\shaders\\post\\blur.pipeline", { "frag:VERTICAL" });
		pl_blur_v->dynamic_renderpass = true;
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
		if (_targets.empty())
			return;

		graphics::Queue::get()->wait_idle();

		auto img0 = _targets.front()->image;
		auto tar_size = img0->size;

		iv_tars.assign(_targets.begin(), _targets.end());

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

	void sRendererPrivate::set_sky(graphics::ImageViewPtr sky_map, graphics::ImageViewPtr sky_irr_map, graphics::ImageViewPtr sky_rad_map)
	{
		ds_light->set_image("sky_map"_h, 0, sky_map ? sky_map : img_cube_black->get_view({ 0, 1, 0, 6 }), nullptr);
		ds_light->set_image("sky_irr_map"_h, 0, sky_irr_map ? sky_irr_map : img_cube_black->get_view({ 0, 1, 0, 6 }), nullptr);
		ds_light->set_image("sky_rad_map"_h, 0, sky_rad_map ? sky_rad_map : img_cube_black->get_view({ 0, 1, 0, 6 }), nullptr);
		ds_light->update();

		sky_rad_levels = sky_rad_map ? sky_rad_map->sub.layer_count : 1.f;
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

		graphics::InstanceCommandBuffer cb;

		res.vtx_cnt = mesh->positions.size();
		res.idx_cnt = mesh->indices.size();
		res.arm = !mesh->bone_ids.empty();
		if (!res.arm)
		{
			res.vtx_off = buf_vtx.item_offset();
			for (auto i = 0; i < res.vtx_cnt; i++)
			{
				buf_vtx.set_var<"i_pos"_h>(mesh->positions[i]);
				if (!mesh->normals.empty())
					buf_vtx.set_var<"i_nor"_h>(mesh->normals[i]);
				if (!mesh->uvs.empty())
					buf_vtx.set_var<"i_uv"_h>(mesh->uvs[i]);
				buf_vtx.next_item();
			}

			res.idx_off = buf_idx.item_offset();
			buf_idx.push(res.idx_cnt, mesh->indices.data());

			buf_vtx.upload(cb.get());
			buf_idx.upload(cb.get());
		}
		else
		{
			res.vtx_off = buf_vtx_arm.item_offset();
			for (auto i = 0; i < res.vtx_cnt; i++)
			{
				buf_vtx_arm.set_var<"i_pos"_h>(mesh->positions[i]);
				if (!mesh->normals.empty())
					buf_vtx_arm.set_var<"i_nor"_h>(mesh->normals[i]);
				if (!mesh->uvs.empty())
					buf_vtx_arm.set_var<"i_uv"_h>(mesh->uvs[i]);
				if (!mesh->bone_ids.empty())
					buf_vtx_arm.set_var<"i_bids"_h>(mesh->bone_ids[i]);
				if (!mesh->bone_weights.empty())
					buf_vtx_arm.set_var<"i_bwgts"_h>(mesh->bone_weights[i]);
				buf_vtx_arm.next_item();
			}

			res.idx_off = buf_idx_arm.item_offset();
			buf_idx_arm.push(res.idx_cnt, mesh->indices.data());

			buf_vtx_arm.upload(cb.get());
			buf_idx_arm.upload(cb.get());
		}

		cb.excute();

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
			graphics::InstanceCommandBuffer cb;
			buf_material_info.select_item(id);
			buf_material_info.set_var<"opaque"_h>((int)res.mat->opaque);
			buf_material_info.set_var<"color"_h>(res.mat->color);
			buf_material_info.set_var<"metallic"_h>(res.mat->metallic);
			buf_material_info.set_var<"roughness"_h>(res.mat->roughness);
			buf_material_info.set_var<"alpha_test"_h>(res.mat->alpha_test);
			buf_material_info.set_var<"f"_h>(res.mat->float_values);
			buf_material_info.set_var<"i"_h>(res.mat->int_values);
			auto ids = (int*)buf_material_info.var_addr<"map_indices"_h>();
			for (auto i = 0; i < res.texs.size(); i++)
				ids[i] = res.texs[i].first;
			buf_material_info.upload(cb.get());
			cb.excute();
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
			ds_instance->update();
		}
		return id;
	}

	void sRendererPrivate::set_terrain_instance(uint id, const mat4& mat, const vec3& extent, const uvec2& blocks, uint tess_level, 
		graphics::ImageViewPtr height_map, graphics::ImageViewPtr normal_map, graphics::ImageViewPtr tangent_map, graphics::ImageViewPtr splash_map)
	{
		buf_terrain_ins.select_item(id);
		buf_terrain_ins.set_var<"mat"_h>(mat);
		buf_terrain_ins.set_var<"extent"_h>(extent);
		buf_terrain_ins.set_var<"blocks"_h>(blocks);
		buf_terrain_ins.set_var<"tess_level"_h>(tess_level);
		ds_instance->set_image("terrain_height_maps"_h, id, height_map, nullptr);
		ds_instance->set_image("terrain_normal_maps"_h, id, normal_map, nullptr);
		ds_instance->set_image("terrain_tangent_maps"_h, id, tangent_map, nullptr);
		ds_instance->set_image("terrain_splash_maps"_h, id, splash_map, nullptr);
		ds_instance->update();
	}

	int sRendererPrivate::register_grass_field_instance(int id)
	{
		if (id == -1)
		{
			id = buf_grassfield_ins.get_free_item();
			if (id != -1)
			{

			}
		}
		else
		{
			buf_grassfield_ins.release_item(id);
		}
		return id;
	}

	void sRendererPrivate::set_grass_field_instance(uint id, uint tess_level)
	{
		buf_grassfield_ins.select_item(id);
		buf_grassfield_ins.set_var<"tess_level"_h>(tess_level);
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
	static std::vector<float>& get_gauss_blur_weights(int radius)
	{
		radius = (radius - 1) / 2;
		if (gauss_blur_weights.empty())
		{
			gauss_blur_weights.push_back({                     0.047790, 0.904419, 0.047790 });
			gauss_blur_weights.push_back({           0.015885, 0.221463, 0.524950, 0.221463, 0.015885 });
			gauss_blur_weights.push_back({ 0.005977, 0.060598, 0.241730, 0.382925, 0.241730, 0.060598, 0.005977 });
		}

		assert(radius < gauss_blur_weights.size());
		return gauss_blur_weights[radius];
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
			n->draw(draw_data);

		buf_scene.set_var<"zNear"_h>(camera->zNear);
		buf_scene.set_var<"zFar"_h>(camera->zFar);
		buf_scene.set_var<"fovy"_h>(camera->fovy);
		buf_scene.set_var<"tan_hf_fovy"_h>((float)tan(radians(camera->fovy * 0.5f)));
		buf_scene.set_var<"camera_coord"_h>(camera->node->g_pos);
		buf_scene.set_var<"camera_dir"_h>(-camera->node->g_rot[2]);
		buf_scene.set_var<"view"_h>(camera->view_mat);
		buf_scene.set_var<"view_inv"_h>(camera->view_mat_inv);
		buf_scene.set_var<"proj"_h>(camera->proj_mat);
		buf_scene.set_var<"proj_inv"_h>(camera->proj_mat_inv);
		buf_scene.set_var<"proj_view"_h>(camera->proj_view_mat);
		buf_scene.set_var<"proj_view_inv"_h>(camera->proj_view_mat_inv);
		memcpy(buf_scene.var_addr<"frustum_planes"_h>(), camera->frustum.planes, sizeof(vec4) * 6);
		buf_scene.upload(cb);

		buf_lighting.set_var<"sky_intensity"_h>(1.f);
		buf_lighting.set_var<"sky_rad_levels"_h>(sky_rad_levels);
		buf_lighting.set_var<"fog_color"_h>(vec3(1.f));
		buf_lighting.upload(cb);

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
			buf_light_info.set_var<"color"_h>(vec3(3.14f));
			buf_light_info.set_var<"shadow_index"_h>(-1);

			buf_light_index.push(1, &camera_light_id);

			buf_light_grid.set_var<"offset"_h>(0);
			buf_light_grid.set_var<"count"_h>(1);
			buf_light_grid.next_item();
		}

		buf_mesh_ins.upload(cb);
		buf_armature_ins.upload(cb);
		buf_terrain_ins.upload(cb);
		buf_grassfield_ins.upload(cb);
		buf_sdf_ins.upload(cb);
		buf_light_index.upload(cb);
		buf_light_grid.upload(cb);
		buf_light_info.upload(cb);

		for (auto& d : mesh_buckets.draw_idxs)
			d.second.second.clear();
		draw_data.reset("draw"_h, "mesh"_h);
		for (auto n : camera_culled_nodes)
			n->draw(draw_data);
		mesh_buckets.collect_idrs(cb);

		cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutAttachment);
		cb->set_viewport_and_scissor(Rect(vec2(0), sz));

		// deferred shading pass

		cb->begin_renderpass(nullptr, fb_gbuf.get(),
			{ vec4(0.f, 0.f, 0.f, 1.f),
			vec4(0.f, 0.f, 0.f, 1.f),
			vec4(1.f, 0.f, 0.f, 0.f) });

		prm_gbuf.bind_dss(cb);

		mesh_buckets.draw(cb);

		draw_data.reset("draw"_h, "terrain"_h);
		for (auto n : camera_culled_nodes)
			n->draw(draw_data);
		for (auto& t : draw_data.terrains)
		{
			cb->bind_pipeline(get_material_pipeline(mat_reses[t.mat_id], "terrain"_h, 0, 0));
			cb->draw(4, t.blocks, 0, (t.ins_id << 24) + (t.mat_id << 16));
		}

		draw_data.reset("draw"_h, "sdf"_h);
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
				s.culled_nodes.clear();
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
						n = mix(zn, zf, n * n * shadow_distance);
						f = mix(zn, zf, f * f * shadow_distance);
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
						auto z_min = -hf_zlen;
						auto z_max = +hf_zlen;
						draw_data.reset("occulder"_h, "mesh"_h);
						auto n_draws = 0;
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

					s.mesh_buckets[lv].collect_idrs(cb, "OCCLUDER_PASS"_h);
				}

				buf_dir_shadow.set_var<"splits"_h>(splits);
				buf_dir_shadow.set_var<"far"_h>(shadow_distance * zf);
				buf_dir_shadow.next_item();
			}

			csm_debug_sig = false;

			buf_dir_shadow.upload(cb);
			buf_pt_shadow.upload(cb);

			cb->set_viewport_and_scissor(Rect(vec2(0), ShadowMapSize));
			for (auto i = 0; i < n_dir_shadows; i++)
			{
				auto& s = dir_shadows[i];
				for (auto lv = 0; lv < csm_levels; lv++)
				{
					cb->begin_renderpass(nullptr, imgs_dir_shadow[i]->get_shader_write_dst(0, lv, graphics::AttachmentLoadClear), { vec4(1.f, 0.f, 0.f, 0.f) });
					prm_fwd.bind_dss(cb);
					prm_fwd.set_pc_var<"i"_h>(ivec4(0, i, lv, 0));
					prm_fwd.push_constant(cb);
					s.mesh_buckets[lv].draw(cb);
					cb->end_renderpass();
				}
				cb->image_barrier(imgs_dir_shadow[i].get(), { 0U, 1U, 0U, csm_levels }, graphics::ImageLayoutShaderReadOnly);
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
		cb->bind_pipeline(pl_blit_ms);
		cb->bind_descriptor_set(0, img_dst->get_shader_read_src());
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();

		cb->image_barrier(img_dep.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->begin_renderpass(nullptr, img_dep_ms->get_shader_write_dst());
		cb->bind_pipeline(pl_blit_dep_ms);
		cb->bind_descriptor_set(0, img_dep->get_shader_read_src());
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();

		cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutAttachment);
		cb->image_barrier(img_dep.get(), {}, graphics::ImageLayoutAttachment);
		cb->begin_renderpass(nullptr, fb_fwd.get());
		draw_data.reset("draw"_h, "grass_field"_h);
		for (auto n : camera_culled_nodes)
			n->draw(draw_data);
		prm_fwd.bind_dss(cb);
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
		prm_luma.set_pc_var<"min_log_luma"_h>(min_log_luma);
		prm_luma.set_pc_var<"log_luma_range"_h>(max_log_luma - min_log_luma);
		prm_luma.set_pc_var<"time_coeff"_h>(1.0f);
		prm_luma.set_pc_var<"num_pixels"_h>(int(sz.x * sz.y));
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
		prm_tone.set_pc_var<"white_point"_h>(white_point);
		prm_tone.set_pc_var<"one_over_gamma"_h>(1.f / gamma);
		prm_tone.push_constant(cb);
		cb->bind_descriptor_set(1, img_back0->get_shader_read_src());
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();

		cb->image_barrier(img_back1.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->begin_renderpass(nullptr, img_dst->get_shader_write_dst());
		cb->bind_pipeline(pl_fxaa);
		prm_post.set_pc_var<"pxsz"_h>(1.f / (vec2)img_dst->size);
		prm_post.push_constant(cb);
		cb->bind_descriptor_set(0, img_back1->get_shader_read_src());
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();

		auto blur_pass = [&]() {
			cb->bind_pipeline_layout(prm_post.pll);
			prm_post.set_pc_var<"off"_h>(-3);
			prm_post.set_pc_var<"len"_h>(7);
			prm_post.set_pc_var<"pxsz"_h>(1.f / (vec2)img_back0->size);
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
				prm_fwd.set_pc_var<"f"_h>(vec4(m.color) / 255.f);
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
				prm_fwd.set_pc_var<"f"_h>(vec4(0.f));
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
			prm_fwd.set_pc_var<"f"_h>(vec4(t.color) / 255.f);
			prm_fwd.push_constant(cb);
			cb->bind_pipeline(pl_terrain_plain);
			cb->draw(4, t.blocks, 0, t.ins_id << 24);
			cb->end_renderpass();

			blur_pass();

			cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
			prm_fwd.bind_dss(cb);
			prm_fwd.set_pc_var<"f"_h>(vec4(0.f));
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
				buf_primitives.set_var<"i_pos"_h>(p);
				buf_primitives.next_item();
			}
		}
		buf_primitives.upload(cb);
		cb->begin_renderpass(nullptr, img_dst->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
		cb->bind_vertex_buffer(buf_primitives.buf.get(), 0);
		cb->bind_pipeline_layout(prm_plain.pll);
		prm_plain.set_pc_var<"mvp"_h>(camera->proj_view_mat);
		prm_plain.push_constant(cb);
		auto primitive_vtx_off = 0;
		for (auto& d : draw_data.primitives)
		{
			prm_plain.set_pc_var<"col"_h>(vec4(d.color) / 255.f);
			prm_plain.push_constant(cb, prm_plain.vu_pc.var_off<"col"_h>());
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
		draw_data.reset("draw"_h, "mesh"_h);
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
					prm_fwd.set_pc_var<"i"_h>(ivec4(i + 1, 0, 0, 0));
					prm_fwd.push_constant(cb.get());
					cb->draw_indexed(mesh_r.idx_cnt, mesh_r.idx_off, mesh_r.vtx_off, 1, m.ins_id << 8);
				}
				else
				{
					cb->bind_vertex_buffer(buf_vtx_arm.buf.get(), 0);
					cb->bind_index_buffer(buf_idx_arm.buf.get(), graphics::IndiceTypeUint);
					cb->bind_pipeline(pl_mesh_arm_pickup);
					prm_fwd.set_pc_var<"i"_h>(ivec4(i + 1, 0, 0, 0));
					prm_fwd.push_constant(cb.get());
					cb->draw_indexed(mesh_r.idx_cnt, mesh_r.idx_off, mesh_r.vtx_off, 1, m.ins_id << 8);
				}
			}
			n_draws = draw_data.meshes.size();
		}

		off = nodes.size();
		n_draws = 0;
		draw_data.reset("draw"_h, "terrain"_h);
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
				prm_fwd.set_pc_var<"i"_h>(ivec4(i + 1 + off, 0, 0, 0));
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
