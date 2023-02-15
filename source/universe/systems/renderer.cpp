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
#include "../../graphics/device.h"
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
	const auto dep_fmt = graphics::Format::Format_Depth32;
	const auto esm_fmt = graphics::Format::Format_Depth32;
	const auto sample_count = graphics::SampleCount_4;
	const auto ShadowMapSize = uvec2(2048);
	const auto DirShadowMaxCount = 4U;
	const auto DirShadowMaxLevels = 4U;
	const auto PtShadowMaxCount = 4U;

	namespace graphics
	{
		template<typename T>
		struct PrivateResourcePipelineManager
		{
			std::filesystem::path path;
			PipelineResourceManager prm;
			std::unique_ptr<graphics::DescriptorSet> self_ds;
			std::unordered_map<uint, T*> pls;

			void init(const std::filesystem::path& _path, bool create_default_pl = true)
			{
				path = _path;
				if (create_default_pl)
				{
					auto pl = T::get(path, {});
					pls.emplace(0U, pl);
					prm.init(pl->layout, std::is_same_v<T, GraphicsPipeline> ? PipelineGraphics : PipelineCompute);
				}
			}

			T* add_pl(uint hash, const std::vector<std::string>& defines)
			{
				auto pl = T::get(path, defines);
				pls.emplace(hash, pl);
				if (!prm.pll)
					prm.init(pl->layout, std::is_same_v<T, GraphicsPipeline> ? PipelineGraphics : PipelineCompute);
				return pl;
			}

			void create_self_ds()
			{
				self_ds.reset(graphics::DescriptorSet::create(nullptr, prm.pll->dsls.back()));
				prm.set_ds(""_h, self_ds.get());
			}
		};
	}

	std::vector<sRenderer::MatVar> mat_vars;
	std::vector<sRenderer::MeshRes> mesh_reses;
	std::vector<sRenderer::TexRes> tex_reses;
	std::vector<sRenderer::MatRes> mat_reses;

	std::unique_ptr<graphics::Image> img_black;
	std::unique_ptr<graphics::Image> img_white;
	std::unique_ptr<graphics::Image> img_cube_black;
	std::unique_ptr<graphics::Image> img_cube_white;
	std::unique_ptr<graphics::Image> img_black3D;
	std::unique_ptr<graphics::Image> img_white3D;
	std::unique_ptr<graphics::Image> img_back0;
	std::unique_ptr<graphics::Image> img_back1;
	std::unique_ptr<graphics::Image> img_dst;
	std::unique_ptr<graphics::Image> img_dep;
	std::unique_ptr<graphics::Image> img_dst_ms;
	std::unique_ptr<graphics::Image> img_dep_ms;
	std::unique_ptr<graphics::Image> img_last_dst;
	std::unique_ptr<graphics::Image> img_last_dep;
	std::unique_ptr<graphics::Image> img_gbufferA;	// color
	std::unique_ptr<graphics::Image> img_gbufferB;	// normal
	std::unique_ptr<graphics::Image> img_gbufferC;	// metallic, roughness, ao, flags
	std::unique_ptr<graphics::Image> img_gbufferD;	// emissive
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
	graphics::PipelineResourceManager prm_plain;
	graphics::PrivateResourcePipelineManager<graphics::GraphicsPipeline> pl_deferred;
	graphics::PrivateResourcePipelineManager<graphics::GraphicsPipeline> pl_bloom;
	graphics::PrivateResourcePipelineManager<graphics::GraphicsPipeline> pl_blur;
	graphics::PrivateResourcePipelineManager<graphics::ComputePipeline> pl_luma;
	graphics::PrivateResourcePipelineManager<graphics::GraphicsPipeline> pl_tone;
	graphics::PrivateResourcePipelineManager<graphics::GraphicsPipeline> pl_fxaa;

	graphics::VertexBuffer buf_vtx;
	graphics::IndexBuffer buf_idx;
	graphics::VertexBuffer buf_vtx_arm;
	graphics::IndexBuffer buf_idx_arm;

	graphics::StorageBuffer buf_camera;
	graphics::SparseArray mesh_instances;
	graphics::SparseArray armature_instances;
	graphics::SparseArray terrain_instances;
	graphics::SparseArray sdf_instances;
	graphics::SparseArray volume_instances;
	graphics::StorageBuffer buf_instance;
	graphics::StorageBuffer buf_marching_cubes_loopup;
	graphics::StorageBuffer buf_transform_feedback;
	graphics::StorageBuffer buf_material;
	graphics::SparseArray dir_lights;
	graphics::SparseArray pt_lights;
	graphics::StorageBuffer buf_lighting;
	graphics::VertexBuffer buf_particles;
	graphics::VertexBuffer buf_primitives;
	graphics::StorageBuffer buf_luma;

	std::unique_ptr<graphics::DescriptorSet> ds_camera;
	std::unique_ptr<graphics::DescriptorSet> ds_instance;
	std::unique_ptr<graphics::DescriptorSet> ds_material;
	std::unique_ptr<graphics::DescriptorSet> ds_lighting;

	graphics::GraphicsPipelinePtr pl_blit = nullptr;
	graphics::GraphicsPipelinePtr pl_blit_dep = nullptr;
	graphics::GraphicsPipelinePtr pl_add = nullptr;
	graphics::GraphicsPipelinePtr pl_blend = nullptr;

	graphics::GraphicsPipelinePtr pl_mesh_plain = nullptr;
	graphics::GraphicsPipelinePtr pl_mesh_arm_plain = nullptr;
	graphics::GraphicsPipelinePtr pl_terrain_plain = nullptr;
	graphics::GraphicsPipelinePtr pl_sdf_plain = nullptr;
	graphics::GraphicsPipelinePtr pl_MC_plain = nullptr;
	graphics::GraphicsPipelinePtr pl_mesh_pickup = nullptr;
	graphics::GraphicsPipelinePtr pl_mesh_arm_pickup = nullptr;
	graphics::GraphicsPipelinePtr pl_terrain_pickup = nullptr;
	graphics::GraphicsPipelinePtr pl_sdf_pickup = nullptr;
	graphics::GraphicsPipelinePtr pl_MC_pickup = nullptr;
	graphics::GraphicsPipelinePtr pl_terrain_transform_feedback = nullptr;
	graphics::GraphicsPipelinePtr pl_MC_transform_feedback = nullptr;
	graphics::GraphicsPipelinePtr pl_line3d = nullptr;
	graphics::GraphicsPipelinePtr pl_line_strip3d = nullptr;
	graphics::GraphicsPipelinePtr pl_triangle3d = nullptr;

	std::unique_ptr<graphics::Fence> fence_pickup;

	bool mark_clear_pipelines = false;

	std::filesystem::path post_shading_code_file = L"";

	int camera_light_id = -1;
	int white_tex_id = -1;
	int black_tex_id = -1;
	int rand_tex_id = -1;

	struct MeshBatcher
	{
		graphics::IndirectBuffer buf_idr;
		std::unordered_map<graphics::GraphicsPipelinePtr, std::pair<bool, std::vector<uint>>> batches;

		void collect_idrs(const DrawData& draw_data, graphics::CommandBufferPtr cb, uint mod2 = 0);
		void draw(graphics::CommandBufferPtr cb);
	};

	struct DirShadow
	{
		mat3 rot;
		Frustum frustum;
		std::vector<cNodePtr> culled_nodes;
		MeshBatcher batcher[DirShadowMaxLevels];
		std::vector<TerrainDraw> draw_terrains[DirShadowMaxLevels];
		std::vector<VolumeDraw> draw_MCs[DirShadowMaxLevels];
	};

	struct PointShadow
	{

	};

	std::vector<cNodePtr> camera_culled_nodes;
	DrawData draw_data;
	MeshBatcher opa_batcher;
	MeshBatcher trs_batcher;
	DirShadow dir_shadows[DirShadowMaxCount];
	PointShadow pt_shadows[PtShadowMaxCount];

	std::vector<PrimitiveDraw> debug_primitives;
	bool csm_debug_sig = false;

	void combine_global_defines(std::vector<std::string>& defines)
	{
		defines.push_back(std::format("frag:WHITE_TEX_ID={}", white_tex_id));
		defines.push_back(std::format("frag:BLACK_TEX_ID={}", black_tex_id));
		defines.push_back(std::format("frag:RAND_TEX_ID={}", rand_tex_id));
		for (auto i = 0; i < tex_reses.size(); i++)
		{
			if (!tex_reses[i].name.empty())
				defines.push_back(std::format("frag:{}_MAP_ID={}", tex_reses[i].name, i));
		}
		if (!post_shading_code_file.empty())
		{
			if (auto path = Path::get(post_shading_code_file); !path.empty())
				defines.push_back(std::format("frag:POST_SHADING_CODE={}", path.string()));
		}
	}

	static graphics::GraphicsPipelinePtr get_material_pipeline(sRenderer::MatRes& res, uint type, uint modifier1 = 0, uint modifier2 = 0)
	{
		auto key = type + modifier1 + modifier2;
		auto it = res.pls.find(key);
		if (it != res.pls.end())
			return it->second;

		std::vector<std::string> defines;
		switch (type)
		{
		case "mesh"_h:
		case "terrain"_h:
		case "sdf"_h:
		case "marching_cubes"_h:
			if (modifier2 == "DEPTH_ONLY"_h)
			{
				defines.push_back("rp=" + str(rp_dep));
				defines.push_back("pll=" + str(pll_fwd));
				defines.push_back("cull_mode=" + TypeInfo::serialize_t(graphics::CullModeFront));
			}
			else if (res.mat->opaque)
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
				if (res.mat->receive_ssr)
					defines.push_back("frag:RECEIVE_SSR");
			}
			break;
		case "grass_field"_h:
		case "particle"_h:
			defines.push_back("rp=" + str(rp_fwd));
			defines.push_back("pll=" + str(pll_fwd));
			defines.push_back("frag:UNLIT");
			break;
		}

		combine_global_defines(defines);
		if (!res.mat->code_file.empty())
			defines.push_back(std::format("frag:MAT_CODE={}", Path::get(res.mat->code_file).string()));
		for (auto& d : res.mat->code_defines)
			defines.push_back(d);

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
			pipeline_name = L"flame\\shaders\\sdf.pipeline";
			break;
		case "marching_cubes"_h:
			pipeline_name = L"flame\\shaders\\volume\\marching_cubes.pipeline";
			break;
		case "particle"_h:
			pipeline_name = L"flame\\shaders\\particle.pipeline";
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
		case "DEPTH_ONLY"_h:
			defines.push_back("all_shader:DEPTH_ONLY");
			break;
		}
		std::sort(defines.begin(), defines.end());

		graphics::GraphicsPipelinePtr ret = nullptr;
		if (!pipeline_name.empty())
			ret = graphics::GraphicsPipeline::get(pipeline_name, defines);
		if (ret)
		{
			res.pls[key] = ret;
			ret->frag()->dependencies.emplace_back("flame::Graphics::Material"_h, res.mat);
			ret->dependencies.emplace_back("flame::Graphics::Material"_h, res.mat);
		}
		return ret;
	}

	static graphics::GraphicsPipelinePtr get_deferred_pipeline(uint modifier = 0)
	{
		auto it = pl_deferred.pls.find(modifier);
		if (it != pl_deferred.pls.end())
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
		combine_global_defines(defines);
		std::sort(defines.begin(), defines.end());

		return pl_deferred.add_pl(modifier, defines);
	}

	void MeshBatcher::collect_idrs(const DrawData& draw_data, graphics::CommandBufferPtr cb, uint mod2)
	{
		for (auto i = 0; i < draw_data.meshes.size(); i++)
		{
			auto& m = draw_data.meshes[i];
			auto& mesh_r = mesh_reses[m.mesh_id];
			auto& mat_r = mat_reses[m.mat_id];
			auto pl = get_material_pipeline(mat_r, "mesh"_h, mesh_r.arm ? "ARMATURE"_h : 0, mod2);
			auto it = batches.find(pl);
			if (it == batches.end())
				it = batches.emplace(pl, std::make_pair(mesh_r.arm, std::vector<uint>())).first;
			it->second.second.push_back(i);
		}

		for (auto& b : batches)
		{
			for (auto i : b.second.second)
			{
				auto& m = draw_data.meshes[i];
				auto& mesh_r = mesh_reses[m.mesh_id];
				buf_idr.add(mesh_r.idx_cnt, mesh_r.idx_off, mesh_r.vtx_off, 1, (m.ins_id << 8) + m.mat_id);
			}
		}
		buf_idr.upload(cb);
	}

	void MeshBatcher::draw(graphics::CommandBufferPtr cb)
	{
		auto off = 0;
		for (auto& b : batches)
		{
			if (b.second.second.empty())
				continue;
			if (!b.second.first)
			{
				cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
				cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
			}
			else
			{
				cb->bind_vertex_buffer(buf_vtx_arm.buf.get(), 0);
				cb->bind_index_buffer(buf_idx_arm.buf.get(), graphics::IndiceTypeUint);
			}
			cb->bind_pipeline(b.first);
			cb->draw_indexed_indirect(buf_idr.buf.get(), off, b.second.second.size());
			off += b.second.second.size();
		}
	}

	sRendererPrivate::sRendererPrivate() 
	{
	}

#include "marching_cubes_lookup.h"

	sRendererPrivate::sRendererPrivate(graphics::WindowPtr w) :
		window(w)
	{
		graphics::InstanceCommandBuffer cb;

		img_black.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(4, 4, 1), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, 1, 8));
		img_white.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(4, 4, 1), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, 1, 8));
		img_cube_black.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(4, 4, 1), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, 1, 6, graphics::SampleCount_1, true));
		img_cube_white.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(4, 4, 1), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, 1, 6, graphics::SampleCount_1, true));
		img_black3D.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(4, 4, 4), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled));
		img_white3D.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(4, 4, 4), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled));
		img_black->clear(vec4(0.f), graphics::ImageLayoutShaderReadOnly);
		img_white->clear(vec4(1.f), graphics::ImageLayoutShaderReadOnly);
		img_cube_black->clear(vec4(0.f), graphics::ImageLayoutShaderReadOnly);
		img_cube_white->clear(vec4(1.f), graphics::ImageLayoutShaderReadOnly);
		img_black3D->clear(vec4(0.f), graphics::ImageLayoutShaderReadOnly);
		img_white3D->clear(vec4(1.f), graphics::ImageLayoutShaderReadOnly);

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

		auto graphics_device = graphics::Device::current();
		static auto sp_trilinear = graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, true, graphics::AddressClampToEdge);
		static auto sp_shadow = graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressClampToBorder);

		auto use_mesh_shader = graphics_device->get_config("mesh_shader"_h) != 0;

		auto dsl_camera = graphics::DescriptorSetLayout::get(L"flame\\shaders\\camera.dsl");
		buf_camera.create(graphics::BufferUsageUniform, dsl_camera->get_buf_ui("Camera"_h));
		ds_camera.reset(graphics::DescriptorSet::create(nullptr, dsl_camera));
		ds_camera->set_buffer("Camera"_h, 0, buf_camera.buf.get());
		ds_camera->update();
		auto dsl_instance = graphics::DescriptorSetLayout::get(L"flame\\shaders\\instance.dsl");
		buf_instance.create(graphics::BufferUsageStorage, dsl_instance->get_buf_ui("Instance"_h));
		mesh_instances.init(buf_instance.item_info("meshes"_h).array_size);
		armature_instances.init(buf_instance.item_info("armatures"_h).array_size);
		terrain_instances.init(buf_instance.item_info("terrains"_h).array_size);
		sdf_instances.init(buf_instance.item_info("sdfs"_h).array_size);
		volume_instances.init(buf_instance.item_info("volumes"_h).array_size);
		buf_marching_cubes_loopup.create(graphics::BufferUsageStorage, dsl_instance->get_buf_ui("MarchingCubesLookup"_h));
		{
			auto pi = buf_marching_cubes_loopup.itemv_d("items"_h, 256);
			auto pdata = pi.pdata;
			assert(sizeof(MarchingCubesLookup) == pi.size);
			for (auto i = 0; i < 256; i++)
			{
				memcpy(pdata, &MarchingCubesLookup[i], sizeof(MarchingCubesLookupItem));
				pdata += sizeof(MarchingCubesLookupItem);
			}
			buf_marching_cubes_loopup.upload(cb.get());
		}
		buf_transform_feedback.create(graphics::BufferUsageStorage | graphics::BufferUsageTransferSrc, dsl_instance->get_buf_ui("TransformFeedback"_h), graphics::BufferUsageTransferDst);
		buf_transform_feedback.item_d("vertex_count"_h).set(0);
		buf_transform_feedback.upload(cb.get());
		ds_instance.reset(graphics::DescriptorSet::create(nullptr, dsl_instance));
		ds_instance->set_buffer("Instance"_h, 0, buf_instance.buf.get());
		for (auto i = 0; i < terrain_instances.capacity; i++)
		{
			ds_instance->set_image("terrain_height_maps"_h, i, img_black->get_view(), sp_trilinear);
			ds_instance->set_image("terrain_normal_maps"_h, i, img_black->get_view(), sp_trilinear);
			ds_instance->set_image("terrain_tangent_maps"_h, i, img_black->get_view(), sp_trilinear);
		}
		ds_instance->set_buffer("MarchingCubesLookup"_h, 0, buf_marching_cubes_loopup.buf.get());
		ds_instance->set_buffer("TransformFeedback"_h, 0, buf_transform_feedback.buf.get());
		ds_instance->update();
		auto dsl_material = graphics::DescriptorSetLayout::get(L"flame\\shaders\\material.dsl");
		buf_material.create(graphics::BufferUsageStorage, dsl_material->get_buf_ui("Material"_h));
		mat_vars.resize(buf_material.item_info("vars"_h).array_size);
		mat_reses.resize(buf_material.item_info("infos"_h).array_size);
		get_material_res(graphics::Material::get(L"default"), -1);
		tex_reses.resize(dsl_material->get_binding("material_maps"_h).count);
		ds_material.reset(graphics::DescriptorSet::create(nullptr, dsl_material));
		ds_material->set_buffer("Material"_h, 0, buf_material.buf.get());
		for (auto i = 0; i < tex_reses.size(); i++)
			ds_material->set_image("material_maps"_h, i, img_black->get_view(), nullptr);
		ds_material->update();
		auto dsl_lighting = graphics::DescriptorSetLayout::get(L"flame\\shaders\\lighting.dsl");
		ds_lighting.reset(graphics::DescriptorSet::create(nullptr, dsl_lighting));
		buf_lighting.create(graphics::BufferUsageStorage, dsl_lighting->get_buf_ui("Lighting"_h));
		dir_lights.init(buf_lighting.item_info("dir_lights"_h).array_size);
		pt_lights.init(buf_lighting.item_info("pt_lights"_h).array_size);
		imgs_dir_shadow.resize(dsl_lighting->get_binding("dir_shadow_maps"_h).count);
		for (auto& i : imgs_dir_shadow)
		{
			i.reset(graphics::Image::create(esm_fmt, uvec3(ShadowMapSize, 1), graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 1, DirShadowMaxLevels));
			i->change_layout(graphics::ImageLayoutShaderReadOnly);
		}
		img_dir_shadow_back.reset(graphics::Image::create(esm_fmt, uvec3(ShadowMapSize, 1), graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 1, DirShadowMaxLevels));
		imgs_pt_shadow.resize(dsl_lighting->get_binding("pt_shadow_maps"_h).count);
		for (auto& i : imgs_pt_shadow)
		{
			i.reset(graphics::Image::create(esm_fmt, uvec3(ShadowMapSize / 2U, 1), graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 1, 6, graphics::SampleCount_1, true));
			i->change_layout(graphics::ImageLayoutShaderReadOnly);
		}
		img_pt_shadow_back.reset(graphics::Image::create(esm_fmt, uvec3(ShadowMapSize / 2U, 1), graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 1, 6, graphics::SampleCount_1, true));
		ds_lighting->set_buffer("Lighting"_h, 0, buf_lighting.buf.get());
		for (auto i = 0; i < imgs_dir_shadow.size(); i++)
			ds_lighting->set_image("dir_shadow_maps"_h, i, imgs_dir_shadow[i]->get_view({ 0, 1, 0, DirShadowMaxLevels }), sp_shadow);
		for (auto i = 0; i < imgs_pt_shadow.size(); i++)
			ds_lighting->set_image("pt_shadow_maps"_h, i, imgs_pt_shadow[i]->get_view({ 0, 1, 0, 6 }), sp_shadow);
		ds_lighting->set_image("sky_map"_h, 0, img_cube_black->get_view({ 0, 1, 0, 6 }), nullptr);
		ds_lighting->set_image("sky_irr_map"_h, 0, img_cube_black->get_view({ 0, 1, 0, 6 }), nullptr);
		ds_lighting->set_image("sky_rad_map"_h, 0, img_cube_black->get_view({ 0, 1, 0, 6 }), nullptr);
		{
			auto img = graphics::Image::get(L"flame\\brdf.dds");
			ds_lighting->set_image("brdf_map"_h, 0, img ? img->get_view() : img_black->get_view(), nullptr);
		}
		ds_lighting->update();

		buf_vtx.create(L"flame\\shaders\\mesh\\mesh.vi", {}, 1024 * 256 * 4);
		buf_idx.create(1024 * 256 * 6);
		buf_vtx_arm.create(L"flame\\shaders\\mesh\\mesh.vi", { "ARMATURE" }, 1024 * 128 * 4);
		buf_idx_arm.create(1024 * 128 * 6);
		buf_particles.create(L"flame\\shaders\\particle.vi", {}, 1024 * 64);
		buf_primitives.create(L"flame\\shaders\\plain\\plain3d.vi", {}, 1024 * 128);

		mesh_reses.resize(1024);

		prm_plain.init(graphics::PipelineLayout::get(L"flame\\shaders\\plain\\plain.pll"), graphics::PipelineGraphics);
		pl_line3d = graphics::GraphicsPipeline::get(L"flame\\shaders\\plain\\line3d.pipeline", {});
		pl_line_strip3d = graphics::GraphicsPipeline::get(L"flame\\shaders\\plain\\line3d.pipeline", { "pt=LineStrip" });
		pl_triangle3d = graphics::GraphicsPipeline::get(L"flame\\shaders\\plain\\triangle3d.pipeline", {});

		pll_fwd = graphics::PipelineLayout::get(L"flame\\shaders\\forward.pll");
		pll_gbuf = graphics::PipelineLayout::get(L"flame\\shaders\\gbuffer.pll");

		pl_blit = graphics::GraphicsPipeline::get(L"flame\\shaders\\blit.pipeline", {});
		pl_blit_dep = graphics::GraphicsPipeline::get(L"flame\\shaders\\blit.pipeline", { "rp=" + str(rp_dep), "frag:DEPTH" });
		pl_add = graphics::GraphicsPipeline::get(L"flame\\shaders\\add.pipeline", {});
		pl_blend = graphics::GraphicsPipeline::get(L"flame\\shaders\\blend.pipeline", {});

		prm_fwd.init(pll_fwd, graphics::PipelineGraphics);
		prm_fwd.set_ds("camera"_h, ds_camera.get());
		prm_fwd.set_ds("instance"_h, ds_instance.get());
		prm_fwd.set_ds("material"_h, ds_material.get());
		prm_fwd.set_ds("lighting"_h, ds_lighting.get());

		prm_gbuf.init(pll_gbuf, graphics::PipelineGraphics);
		prm_gbuf.set_ds("camera"_h, ds_camera.get());
		prm_gbuf.set_ds("instance"_h, ds_instance.get());
		prm_gbuf.set_ds("material"_h, ds_material.get());

		pl_mesh_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline", { "rp=" + str(rp_col_dep) });
		pl_mesh_arm_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline", { "rp=" + str(rp_col_dep), "vert:ARMATURE" });
		pl_terrain_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\terrain\\terrain.pipeline", { "rp=" + str(rp_col_dep) });

		if (use_mesh_shader)
			pl_MC_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\volume\\marching_cubes.pipeline", { "rp=" + str(rp_col_dep) });

		opa_batcher.buf_idr.create(mesh_instances.capacity);
		trs_batcher.buf_idr.create(mesh_instances.capacity);
		for (auto& s : dir_shadows)
		{
			for (auto i = 0; i < DirShadowMaxLevels; i++)
				s.batcher[i].buf_idr.create(min(1024U, mesh_instances.capacity));
		}

		pl_deferred.init(L"flame\\shaders\\deferred.pipeline");
		pl_deferred.create_self_ds();
		pl_deferred.prm.set_ds("camera"_h, ds_camera.get());
		pl_deferred.prm.set_ds("lighting"_h, ds_lighting.get());
		pl_deferred.prm.set_ds("material"_h, ds_material.get());

		pl_bloom.init(L"flame\\shaders\\bloom.pipeline", false);
		pl_bloom.add_pl("BRIGHT"_h, { "frag:BRIGHT_PASS" });
		pl_bloom.add_pl("DOWNSAMPLE"_h, { "frag:BOX_PASS" });
		pl_bloom.add_pl("UPSAMPLE"_h, { "be=true", "bc=One", "frag:BOX_PASS" });

		pl_blur.init(L"flame\\shaders\\blur.pipeline", false);
		pl_blur.add_pl("H"_h, { "frag:HORIZONTAL" });
		pl_blur.add_pl("V"_h, { "frag:VERTICAL" });
		pl_blur.add_pl("DEPTH_H"_h, { "rp=" + str(rp_dep), "frag:HORIZONTAL", "frag:DEPTH" });
		pl_blur.add_pl("DEPTH_V"_h, { "rp=" + str(rp_dep), "frag:VERTICAL", "frag:DEPTH" });
		pl_blur.add_pl("LOCAL_MAX_H"_h, { "frag:HORIZONTAL", "frag:LOCAL_MAX" });
		pl_blur.add_pl("LOCAL_MAX_V"_h, { "frag:VERTICAL", "frag:LOCAL_MAX" });

		pl_luma.init(L"flame\\shaders\\luma.pipeline", false);
		pl_luma.add_pl("hist"_h, { "comp:HISTOGRAM_PASS" });
		pl_luma.add_pl("avg"_h, { "comp:AVERAGE_PASS" });
		pl_luma.create_self_ds();
		buf_luma.create(graphics::BufferUsageStorage | graphics::BufferUsageTransferSrc, pl_luma.prm.pll->dsls.back()->get_buf_ui("Luma"_h), graphics::BufferUsageTransferDst);
		pl_luma.self_ds->set_buffer("Luma"_h, 0, buf_luma.buf.get());
		pl_luma.self_ds->update();

		pl_tone.init(L"flame\\shaders\\tone.pipeline");
		pl_fxaa.init(L"flame\\shaders\\fxaa.pipeline");

		pl_mesh_pickup = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline", { "rp=" + str(rp_col_dep), "frag:PICKUP" });
		pl_mesh_arm_pickup = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline",
			{ "rp=" + str(rp_col_dep),
			  "vert:ARMATURE",
			  "frag:PICKUP" });
		pl_terrain_pickup = graphics::GraphicsPipeline::get(L"flame\\shaders\\terrain\\terrain.pipeline", { "rp=" + str(rp_col_dep), "frag:PICKUP" });
		if (use_mesh_shader)
		{
			pl_MC_pickup = graphics::GraphicsPipeline::get(L"flame\\shaders\\volume\\marching_cubes.pipeline", { "rp=" + str(rp_col_dep), "frag:PICKUP" });
			pl_MC_transform_feedback = graphics::GraphicsPipeline::get(L"flame\\shaders\\volume\\marching_cubes.pipeline", { "rp=" + str(rp_col_dep), "rasterizer_discard=true", "mesh:TRANSFORM_FEEDBACK" });
		}

		fence_pickup.reset(graphics::Fence::create(false));

		camera_light_id = register_light_instance(LightDirectional, -1);
		white_tex_id = get_texture_res(img_white->get_view(), nullptr, -1);
		black_tex_id = get_texture_res(img_black->get_view(), nullptr, -1);
		if (auto img = graphics::Image::get(L"flame\\random.png"); img)
		{
			rand_tex_id = get_texture_res(img->get_view(),
				graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressRepeat), -1);
		}

		set_sky_intensity(1.f);
		set_fog_color(vec3(1.f));
		set_shadow_distance(100.f);
		set_csm_levels(2);
		set_esm_factor(100.f);
		set_post_processing_enable(true);
		set_ssao_enable(true);
		set_ssao_radius(0.5f);
		set_ssao_bias(0.025f);
		set_white_point(4.f);
		set_bloom_enable(true);
		set_ssr_enable(true);
		set_ssr_thickness(0.4f);
		set_ssr_max_distance(8.f);
		set_ssr_max_steps(64);
		set_ssr_binary_search_steps(5);
		set_tone_mapping_enable(true);
		set_gamma(1.5f);

		cb.excute();
		
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
		auto tar_ext = img0->extent;

		static auto sp_nearest = graphics::Sampler::get(graphics::FilterNearest, graphics::FilterNearest, false, graphics::AddressClampToEdge);
		static auto sp_nearest_dep = graphics::Sampler::get(graphics::FilterNearest, graphics::FilterNearest, false, graphics::AddressClampToBorder);

		img_dst.reset(graphics::Image::create(col_fmt, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled | graphics::ImageUsageStorage));
		img_dst->filename = L"##img_dst";
		img_back0.reset(graphics::Image::create(col_fmt, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 0));
		img_back0->filename = L"##img_back0";
		img_back1.reset(graphics::Image::create(col_fmt, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_back1->filename = L"##img_back1";
		img_dep.reset(graphics::Image::create(dep_fmt, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_dep->filename = L"##img_dep";
		img_dst_ms.reset(graphics::Image::create(col_fmt, tar_ext, graphics::ImageUsageAttachment, 1, 1, sample_count));
		img_dep_ms.reset(graphics::Image::create(dep_fmt, tar_ext, graphics::ImageUsageAttachment, 1, 1, sample_count));
		img_last_dst.reset(graphics::Image::create(col_fmt, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled | graphics::ImageUsageStorage));
		img_last_dep.reset(graphics::Image::create(dep_fmt, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_gbufferA.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_gbufferB.reset(graphics::Image::create(graphics::Format_A2R10G10B10_UNORM, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_gbufferC.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_gbufferD.reset(graphics::Image::create(graphics::Format_B10G11R11_UFLOAT, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		fb_fwd.reset(graphics::Framebuffer::create(rp_fwd, { img_dst_ms->get_view(), img_dep_ms->get_view(), img_dst->get_view(), img_dep->get_view() }));
		fb_gbuf.reset(graphics::Framebuffer::create(rp_gbuf, { img_gbufferA->get_view(), img_gbufferB->get_view(), img_gbufferC->get_view(), img_gbufferD->get_view(), img_dep->get_view()}));
		ds_lighting->set_image("img_dep"_h, 0, img_dep->get_view(), sp_nearest_dep);
		ds_lighting->set_image("img_last_dst"_h, 0, img_last_dst->get_view(), sp_nearest_dep);
		ds_lighting->set_image("img_last_dep"_h, 0, img_last_dep->get_view(), sp_nearest);
		ds_lighting->update();
		pl_deferred.self_ds->set_image("img_gbufferA"_h, 0, img_gbufferA->get_view(), nullptr);
		pl_deferred.self_ds->set_image("img_gbufferB"_h, 0, img_gbufferB->get_view(), nullptr);
		pl_deferred.self_ds->set_image("img_gbufferC"_h, 0, img_gbufferC->get_view(), nullptr);
		pl_deferred.self_ds->set_image("img_gbufferD"_h, 0, img_gbufferD->get_view(), nullptr);
		pl_deferred.self_ds->update();
		pl_luma.self_ds->set_image("img_col"_h, 0, img_dst->get_view(), nullptr);
		pl_luma.self_ds->update();

		img_pickup.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageTransferSrc));
		img_dep_pickup.reset(graphics::Image::create(dep_fmt, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageTransferSrc));
		fb_pickup.reset(graphics::Framebuffer::create(rp_col_dep, { img_pickup->get_view(), img_dep_pickup->get_view() }));

		final_layout = _final_layout;
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

	vec2 sRendererPrivate::target_extent()
	{
		if (iv_tars.empty())
			return vec2(0.f);
		return iv_tars[0]->image->extent;
	}

	void sRendererPrivate::set_sky_maps(graphics::ImageViewPtr _sky_map, graphics::ImageViewPtr _sky_irr_map, graphics::ImageViewPtr _sky_rad_map)
	{
		if (sky_map == _sky_map && sky_irr_map == _sky_irr_map && sky_rad_map == _sky_rad_map)
			return;
		sky_map = _sky_map;
		sky_irr_map = _sky_irr_map;
		sky_rad_map = _sky_rad_map;
		ds_lighting->set_image("sky_map"_h, 0, sky_map ? sky_map : img_cube_black->get_view({ 0, 1, 0, 6 }), nullptr);
		ds_lighting->set_image("sky_irr_map"_h, 0, sky_irr_map ? sky_irr_map : img_cube_black->get_view({ 0, 1, 0, 6 }), nullptr);
		ds_lighting->set_image("sky_rad_map"_h, 0, sky_rad_map ? sky_rad_map : img_cube_black->get_view({ 0, 1, 0, 6 }), nullptr);
		ds_lighting->update();

		sky_rad_levels = sky_rad_map ? sky_rad_map->sub.level_count : 1.f;
		buf_lighting.item_d("sky_rad_levels"_h).set(sky_rad_levels);

		dirty = true;
	}

	void sRendererPrivate::set_sky_intensity(float v)
	{
		if (sky_intensity == v)
			return;
		sky_intensity = v;
		buf_lighting.item_d("sky_intensity"_h).set(sky_intensity);

		dirty = true;
	}

	void sRendererPrivate::set_fog_color(const vec3& color)
	{
		if (fog_color == color)
			return;
		fog_color = color;
		buf_lighting.item_d("fog_color"_h).set(fog_color);

		dirty = true;
	}

	void sRendererPrivate::set_shadow_distance(float d)
	{
		if (shadow_distance == d)
			return;
		shadow_distance = d;

		dirty = true;
	}

	void sRendererPrivate::set_csm_levels(uint lv)
	{
		if (csm_levels == lv)
			return;
		csm_levels = lv;

		dirty = true;
	}

	void sRendererPrivate::set_esm_factor(float f)
	{
		if (esm_factor == f)
			return;
		esm_factor = f;
		buf_lighting.item_d("esm_factor"_h).set(esm_factor);

		dirty = true;
	}

	void sRendererPrivate::set_post_processing_enable(bool v)
	{
		if (post_processing_enable == v)
			return;
		post_processing_enable = v;

		dirty = true;
	}

	void sRendererPrivate::set_ssao_enable(bool v)
	{
		if (ssao_enable == v)
			return;
		ssao_enable = v;

		dirty = true;
	}

	void sRendererPrivate::set_ssao_radius(float v)
	{
		if (ssao_radius == v)
			return;
		ssao_radius = v;

		dirty = true;
	}

	void sRendererPrivate::set_ssao_bias(float v)
	{
		if (ssao_bias == v)
			return;
		ssao_bias = v;

		dirty = true;
	}

	void sRendererPrivate::set_white_point(float v)
	{
		if (white_point == v)
			return;
		white_point = v;

		dirty = true;
	}

	void sRendererPrivate::set_bloom_enable(bool v)
	{
		if (bloom_enable == v)
			return;
		bloom_enable = v;

		dirty = true;
	}

	void sRendererPrivate::set_ssr_enable(bool v)
	{
		if (ssr_enable == v)
			return;
		ssr_enable = v;
		buf_lighting.item_d("ssr_enable"_h).set(v ? 1U : 0U);

		dirty = true;
	}

	void sRendererPrivate::set_ssr_thickness(float v)
	{
		if (ssr_thickness == v)
			return;
		ssr_thickness = v;
		buf_lighting.item_d("ssr_thickness"_h).set(v);

		dirty = true;
	}

	void sRendererPrivate::set_ssr_max_distance(float v)
	{
		if (ssr_max_distance == v)
			return;
		ssr_max_distance = v;
		buf_lighting.item_d("ssr_max_distance"_h).set(v);
		 
		dirty = true;
	}

	void sRendererPrivate::set_ssr_max_steps(uint v)
	{
		if (ssr_max_steps == v)
			return;
		ssr_max_steps = v;
		buf_lighting.item_d("ssr_max_steps"_h).set(v);

		dirty = true;
	}

	void sRendererPrivate::set_ssr_binary_search_steps(uint v)
	{
		if (ssr_binary_search_steps == v)
			return;
		ssr_binary_search_steps = v;
		buf_lighting.item_d("ssr_binary_search_steps"_h).set(v);

		dirty = true;
	}

	void sRendererPrivate::set_tone_mapping_enable(bool v)
	{
		if (tone_mapping_enable == v)
			return;
		tone_mapping_enable = v;

		dirty = true;
	}

	void sRendererPrivate::set_gamma(float v)
	{
		if (gamma == v)
			return;
		gamma = v;

		dirty = true;
	}

	int sRendererPrivate::get_mat_var(int id, const std::string& name)
	{
		if (id < 0)
		{
			for (auto i = 0; i < mat_vars.size(); i++)
			{
				auto& res = mat_vars[i];
				if (res.name == name)
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
				for (auto i = 0; i < mat_vars.size(); i++)
				{
					if (mat_vars[i].name.empty())
					{
						id = i;
						break;
					}
				}
			}
			if (id < 0)
				return -1;
		}

		auto& res = mat_vars[id];
		res.name = name;
		res.ref = 1;

		mark_clear_pipelines = true;
	}

	void sRendererPrivate::release_mat_var(uint id)
	{
		auto& res = mat_vars[id];
		if (res.ref == 1)
		{
			res.name.clear();
			res.ref = 0;
		}
		else
			res.ref--;
	}

	const sRenderer::MatVar& sRendererPrivate::get_mat_var_info(uint id)
	{
		return mat_vars[id];
	}

	void sRendererPrivate::set_mat_var(uint id, const vec4& v)
	{
		auto p_info = buf_material.item_d("vars"_h, id);
		p_info.set(v);
	}

	std::filesystem::path sRendererPrivate::get_post_shading_code_file()
	{
		return post_shading_code_file;
	}

	void sRendererPrivate::set_post_shading_code_file(const std::filesystem::path& path)
	{
		if (post_shading_code_file == path)
			return;
		post_shading_code_file = path;

		mark_clear_pipelines = true;
		dirty = true;
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

			if (!res.name.empty())
			{
				res.name.clear();
				mark_clear_pipelines = true;
			}
			
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

	void sRendererPrivate::set_texture_res_name(uint id, const std::string& name)
	{
		auto& res = tex_reses[id];
		if (!res.iv || res.name == name)
			return;
		res.name = name;

		mark_clear_pipelines = true;

		dirty = true;
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
				if (!mesh->uvs.empty())
					pv.item("i_uv"_h).set(mesh->uvs[i]);
				if (!mesh->normals.empty())
					pv.item("i_nor"_h).set(mesh->normals[i]);
				if (!mesh->tangents.empty())
					pv.item("i_tan"_h).set(mesh->tangents[i]);
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
				if (!mesh->uvs.empty())
					pv.item("i_uv"_h).set(mesh->uvs[i]);
				if (!mesh->normals.empty())
					pv.item("i_nor"_h).set(mesh->normals[i]);
				if (!mesh->tangents.empty())
					pv.item("i_tan"_h).set(mesh->tangents[i]);
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

	void sRendererPrivate::update_mat_res(uint id, bool update_parameters, bool update_textures, bool update_pipelines)
	{
		auto& res = mat_reses[id];

		if (update_textures)
		{
			for (auto& tex : res.texs)
			{
				if (tex.first != -1)
				{
					release_texture_res(tex.first);
					tex.first = -1;
				}
				if (tex.second)
				{
					graphics::Image::release(tex.second);
					tex.second = nullptr;
				}
			}

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
						image->dependencies.emplace_back("flame::Graphics::Material"_h, res.mat);
					}
				}
			}
		}
		if (update_pipelines)
		{
			if (res.mat->color_map != -1)
			{
				auto found = false;
				for (auto& d : res.mat->code_defines)
				{
					if (d.starts_with("frag:COLOR_MAP"))
					{
						d = "frag:COLOR_MAP=" + str(res.mat->color_map);
						found = true;
						break;
					}
				}
				if (!found)
					res.mat->code_defines.push_back("frag:COLOR_MAP=" + str(res.mat->color_map));
			}
			else
			{
				auto& defines = res.mat->code_defines;
				for (auto it = defines.begin(); it != defines.end(); )
				{
					if (it->starts_with("frag:COLOR_MAP"))
						it = defines.erase(it);
					else
						it++;
				}
			}
			if (res.mat->normal_map != -1)
			{
				auto found = false;
				for (auto& d : res.mat->code_defines)
				{
					if (d.starts_with("frag:NORMAL_MAP"))
					{
						d = "frag:NORMAL_MAP=" + str(res.mat->normal_map);
						found = true;
						break;
					}
				}
				if (!found)
					res.mat->code_defines.push_back("frag:NORMAL_MAP=" + str(res.mat->normal_map));
			}
			else
			{
				auto& defines = res.mat->code_defines;
				for (auto it = defines.begin(); it != defines.end(); )
				{
					if (it->starts_with("frag:NORMAL_MAP"))
						it = defines.erase(it);
					else
						it++;
				}
			}
			if (res.mat->metallic_map != -1)
			{
				auto found = false;
				for (auto& d : res.mat->code_defines)
				{
					if (d.starts_with("frag:METALLIC_MAP"))
					{
						d = "frag:METALLIC_MAP=" + str(res.mat->metallic_map);
						found = true;
						break;
					}
				}
				if (!found)
					res.mat->code_defines.push_back("frag:METALLIC_MAP=" + str(res.mat->metallic_map));
			}
			else
			{
				auto& defines = res.mat->code_defines;
				for (auto it = defines.begin(); it != defines.end(); )
				{
					if (it->starts_with("frag:METALLIC_MAP"))
						it = defines.erase(it);
					else
						it++;
				}
			}
			if (res.mat->roughness_map != -1)
			{
				auto found = false;
				for (auto& d : res.mat->code_defines)
				{
					if (d.starts_with("frag:ROUGHNESS_MAP"))
					{
						d = "frag:ROUGHNESS_MAP=" + str(res.mat->roughness_map);
						found = true;
						break;
					}
				}
				if (!found)
					res.mat->code_defines.push_back("frag:ROUGHNESS_MAP=" + str(res.mat->roughness_map));
			}
			else
			{
				auto& defines = res.mat->code_defines;
				for (auto it = defines.begin(); it != defines.end(); )
				{
					if (it->starts_with("frag:ROUGHNESS_MAP"))
						it = defines.erase(it);
					else
						it++;
				}
			}
			if (res.mat->emissive_map != -1)
			{
				auto found = false;
				for (auto& d : res.mat->code_defines)
				{
					if (d.starts_with("frag:EMISSIVE_MAP"))
					{
						d = "frag:EMISSIVE_MAP=" + str(res.mat->emissive_map);
						found = true;
						break;
					}
				}
				if (!found)
					res.mat->code_defines.push_back("frag:EMISSIVE_MAP=" + str(res.mat->emissive_map));
			}
			else
			{
				auto& defines = res.mat->code_defines;
				for (auto it = defines.begin(); it != defines.end(); )
				{
					if (it->starts_with("frag:EMISSIVE_MAP"))
						it = defines.erase(it);
					else
						it++;
				}
			}
			if (res.mat->alpha_test > 0.f)
			{
				auto found = false;
				for (auto& d : res.mat->code_defines)
				{
					if (d.starts_with("frag:ALPHA_TEST"))
					{
						d = "frag:ALPHA_TEST=" + str(res.mat->alpha_test);
						found = true;
						break;
					}
				}
				if (!found)
					res.mat->code_defines.push_back("frag:ALPHA_TEST=" + str(res.mat->alpha_test));
			}
			else
			{
				auto& defines = res.mat->code_defines;
				for (auto it = defines.begin(); it != defines.end(); )
				{
					if (it->starts_with("frag:ALPHA_TEST"))
						it = defines.erase(it);
					else
						it++;
				}
			}
			if (res.mat->splash_map != -1)
			{
				auto found = false;
				for (auto& d : res.mat->code_defines)
				{
					if (d.starts_with("frag:SPLASH_MAP"))
					{
						d = "frag:SPLASH_MAP=" + str(res.mat->splash_map);
						found = true;
						break;
					}
				}
				if (!found)
					res.mat->code_defines.push_back("frag:SPLASH_MAP=" + str(res.mat->splash_map));
			}
			else
			{
				auto& defines = res.mat->code_defines;
				for (auto it = defines.begin(); it != defines.end(); )
				{
					if (it->starts_with("frag:SPLASH_MAP"))
						it = defines.erase(it);
					else
						it++;
				}
			}

			graphics::Queue::get()->wait_idle();

			for (auto& pl : res.pls)
				graphics::GraphicsPipeline::release(pl.second);

			auto has_pl = [&](graphics::GraphicsPipelinePtr pl) {
				for (auto& pair : res.pls)
				{
					if (pair.second == pl)
						return true;
				}
				return false;
			};
			for (auto it = opa_batcher.batches.begin(); it != opa_batcher.batches.end();)
			{
				if (has_pl(it->first))
					it = opa_batcher.batches.erase(it);
				else
					it++;
			}
			for (auto it = trs_batcher.batches.begin(); it != trs_batcher.batches.end();)
			{
				if (has_pl(it->first))
					it = trs_batcher.batches.erase(it);
				else
					it++;
			}

			for (auto& s : dir_shadows)
			{
				for (auto& mb : s.batcher)
				{
					for (auto it = mb.batches.begin(); it != mb.batches.end();)
					{
						if (has_pl(it->first))
							it = mb.batches.erase(it);
						else
							it++;
					}
				}
			}

			res.pls.clear();
		}
		if (update_parameters || update_textures)
		{
			auto p_info = buf_material.item_d("infos"_h, id);
			p_info.item("color"_h).set(res.mat->color);
			p_info.item("metallic"_h).set(res.mat->metallic);
			p_info.item("roughness"_h).set(res.mat->roughness);
			p_info.item("emissive"_h).set(res.mat->emissive);
			p_info.item("tiling"_h).set(res.mat->tiling);
			p_info.item("normal_map_strength"_h).set(res.mat->normal_map_strength);
			p_info.item("emissive_map_strength"_h).set(res.mat->emissive_map_strength);
			p_info.item("flags"_h).set(res.mat->get_flags());
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

		mat->data_listeners.add([this, id](uint hash) {
			switch (hash)
			{
			case "color_map"_h:
			case "normal_map"_h:
			case "metallic_map"_h:
			case "roughness_map"_h:
			case "emissive_map"_h:
			case "alpha_map"_h:
			case "splash_map"_h:
			case "code_file"_h:
			case "code_defines"_h:
				update_mat_res(id, false, false, true);
				break;
			case "textures"_h:
				update_mat_res(id, false, true, false);
				break;
			default:
				update_mat_res(id, true, false, false);
			}
		}, "renderer"_h);

		auto& res = mat_reses[id];
		res.mat = mat;
		res.ref = 1;
		update_mat_res(id);
		return id;
	}

	void sRendererPrivate::release_material_res(uint id)
	{
		auto& res = mat_reses[id];
		if (res.ref == 1)
		{
			res.mat->data_listeners.remove("renderer"_h);

			update_mat_res(id);
			res.mat = nullptr;
		}
		else
			res.ref--;
	}

	const sRenderer::MatRes& sRendererPrivate::get_material_res_info(uint id)
	{ 
		return mat_reses[id]; 
	}

	int sRendererPrivate::register_light_instance(LightType type, int id)
	{
		auto& arr = type == LightDirectional ? dir_lights : pt_lights;
		if (id == -1)
		{
			id = arr.get_free_item();
			if (id != -1)
			{

			}
		}
		else
		{
			arr.release_item(id);

		}
		return id;
	}

	void sRendererPrivate::set_dir_light_instance(uint id, const vec3& dir, const vec3& color)
	{
		auto pi = buf_lighting.item_d("dir_lights"_h, id);
		pi.item("dir"_h).set(dir);
		pi.item("color"_h).set(color);
	}

	void sRendererPrivate::set_pt_light_instance(uint id, const vec3& pos, const vec3& color, float range)
	{
		auto pi = buf_lighting.item_d("pt_lights"_h, id);
		pi.item("pos"_h).set(pos);
		pi.item("color"_h).set(color);
	}

	int sRendererPrivate::register_mesh_instance(int id)
	{
		if (id == -1)
		{
			id = mesh_instances.get_free_item();
			if (id != -1)
				set_mesh_instance(id, mat4(1.f), mat3(1.f));
		}
		else
			mesh_instances.release_item(id);
		return id;
	}

	void sRendererPrivate::set_mesh_instance(uint id, const mat4& mat, const mat3& nor)
	{
		auto pi = buf_instance.item_d("meshes"_h, id);
		pi.item("mat"_h).set(mat);
		pi.item("nor"_h).set(mat4(nor));
	}

	int sRendererPrivate::register_armature_instance(int id)
	{
		if (id == -1)
		{
			id = armature_instances.get_free_item();
			if (id != -1)
			{
				std::vector<mat4> mats(buf_instance.item_info("armatures"_h).array_size);
				for (auto i = 0; i < mats.size(); i++)
					mats[i] = mat4(1.f);
				set_armature_instance(id, mats.data(), mats.size());
			}
		}
		else
			armature_instances.release_item(id);
		return id;
	}

	void sRendererPrivate::set_armature_instance(uint id, const mat4* mats, uint size)
	{
		auto pi = buf_instance.item_d("armatures"_h, id);
		memcpy(pi.pdata, mats, size * sizeof(mat4));
	}

	int sRendererPrivate::register_terrain_instance(int id)
	{
		if (id == -1)
		{
			id = terrain_instances.get_free_item();
			if (id != -1)
			{

			}
		}
		else
		{
			terrain_instances.release_item(id);
			ds_instance->set_image("terrain_height_maps"_h, id, img_black->get_view(), nullptr);
			ds_instance->set_image("terrain_normal_maps"_h, id, img_black->get_view(), nullptr);
			ds_instance->set_image("terrain_tangent_maps"_h, id, img_black->get_view(), nullptr);
			ds_instance->update();
		}
		return id;
	}

	void sRendererPrivate::set_terrain_instance(uint id, const mat4& mat, const vec3& extent, const uvec2& blocks, uint tess_level, uint grass_field_tess_level, uint grass_channel, int grass_texture_id,
		graphics::ImageViewPtr height_map, graphics::ImageViewPtr normal_map, graphics::ImageViewPtr tangent_map)
	{
		auto pi = buf_instance.item_d("terrains"_h, id);
		pi.item("mat"_h).set(mat);
		pi.item("extent"_h).set(extent);
		pi.item("blocks"_h).set(blocks);
		pi.item("tess_level"_h).set(tess_level);
		pi.item("grass_field_tess_level"_h).set(grass_field_tess_level);
		pi.item("grass_channel"_h).set(grass_channel);
		pi.item("grass_channel"_h).set(grass_channel);
		pi.item("grass_texture_id"_h).set(grass_texture_id);

		ds_instance->set_image("terrain_height_maps"_h, id, height_map, nullptr);
		ds_instance->set_image("terrain_normal_maps"_h, id, normal_map, nullptr);
		ds_instance->set_image("terrain_tangent_maps"_h, id, tangent_map, nullptr);
		ds_instance->update();
	}

	int sRendererPrivate::register_sdf_instance(int id)
	{
		if (id == -1)
		{
			id = sdf_instances.get_free_item();
			if (id != -1)
			{

			}
		}
		else
		{
			sdf_instances.release_item(id);
		}
		return id;
	}

	void sRendererPrivate::set_sdf_instance(uint id, uint boxes_count, std::pair<vec3, vec3>* boxes, uint spheres_count, std::pair<vec3, float>* spheres)
	{
		auto pi = buf_instance.item_d("sdfs"_h, id);
		pi.item("boxes_count"_h).set(boxes_count);
		auto pbs = pi.itemv("boxes"_h, boxes_count);
		for (auto i = 0; i < boxes_count; i++)
		{
			auto pb = pbs.at(i);
			pb.item("coord"_h).set(boxes[i].first);
			pb.item("extent"_h).set(boxes[i].second);
		}
		pi.item("spheres_count"_h).set(spheres_count);
		auto pss = pi.itemv("spheres"_h, spheres_count);
		for (auto i = 0; i < spheres_count; i++)
		{
			auto ps = pss.at(i);
			ps.item("coord"_h).set(spheres[i].first);
			ps.item("radius"_h).set(spheres[i].second);
		}
	}

	int sRendererPrivate::register_volume_instance(int id)
	{
		if (id == -1)
		{
			id = volume_instances.get_free_item();
			if (id != -1)
			{

			}
		}
		else
		{
			volume_instances.release_item(id);
			ds_instance->set_image("volume_data_maps"_h, id, img_black3D->get_view(), nullptr);
			ds_instance->update();
		}
		return id;
	}

	void sRendererPrivate::set_volume_instance(uint id, const mat4& mat, const vec3& extent, const uvec3& blocks, graphics::ImageViewPtr data_map)
	{
		auto pi = buf_instance.item_d("volumes"_h, id);
		pi.item("mat"_h).set(mat);
		pi.item("extent"_h).set(extent);
		pi.item("blocks"_h).set(blocks);

		ds_instance->set_image("volume_data_maps"_h, id, data_map, graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressClampToEdge, graphics::BorderColorBlack));
		ds_instance->update();
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
		if (mark_clear_pipelines)
		{
			opa_batcher.batches.clear();
			trs_batcher.batches.clear();
			for (auto& s : dir_shadows)
			{
				for (auto& mb : s.batcher)
					mb.batches.clear();
			}
			for (auto& res : mat_reses)
			{
				if (res.mat)
				{
					for (auto& pl : res.pls)
						graphics::GraphicsPipeline::release(pl.second);
					res.pls.clear();
				}
			}
			for (auto& pl : pl_deferred.pls)
				graphics::GraphicsPipeline::release(pl.second);
			pl_deferred.pls.clear();

			mark_clear_pipelines = false;
		}

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
		auto ext = vec2(img->extent);

		camera->aspect = ext.x / ext.y;
		camera->update();

		camera_culled_nodes.clear();
		sScene::instance()->octree->get_within_frustum(camera->frustum, camera_culled_nodes);

		draw_data.reset(PassInstance, 0);
		for (auto n : camera_culled_nodes)
		{
			if (n->instance_frame < frames)
			{
				n->draw(draw_data);
				n->instance_frame = frames;
			}
		}

		static auto sp_nearest = graphics::Sampler::get(graphics::FilterNearest, graphics::FilterNearest, false, graphics::AddressClampToEdge);

		buf_vtx.upload(cb);
		buf_idx.upload(cb);
		buf_vtx_arm.upload(cb);
		buf_idx_arm.upload(cb);

		buf_camera.item("zNear"_h).set(camera->zNear);
		buf_camera.item("zFar"_h).set(camera->zFar);
		buf_camera.item("fovy"_h).set(camera->fovy);
		buf_camera.item("tan_hf_fovy"_h).set((float)tan(radians(camera->fovy * 0.5f)));
		buf_camera.item("viewport"_h).set(ext);
		buf_camera.item("coord"_h).set(camera->node->g_pos);
		buf_camera.item("front"_h).set(-camera->node->g_rot[2]);
		buf_camera.item("right"_h).set(camera->node->g_rot[0]);
		buf_camera.item("up"_h).set(camera->node->g_rot[1]);
		buf_camera.item("last_view"_h).set(buf_camera.item("view"_h).get<mat4>());
		buf_camera.item("view"_h).set(camera->view_mat);
		buf_camera.item("view_inv"_h).set(camera->view_mat_inv);
		buf_camera.item("proj"_h).set(camera->proj_mat);
		buf_camera.item("proj_inv"_h).set(camera->proj_mat_inv);
		buf_camera.item("proj_view"_h).set(camera->proj_view_mat);
		buf_camera.item("proj_view_inv"_h).set(camera->proj_view_mat_inv);
		memcpy(buf_camera.item("frustum_planes"_h).pdata, camera->frustum.planes, sizeof(vec4) * 6);
		buf_camera.item("time"_h).set(total_time);
		buf_camera.mark_dirty(buf_camera);
		buf_camera.upload(cb);

		buf_material.upload(cb);
		buf_instance.upload(cb);

		auto n_dir_lights = 0;
		auto n_dir_shadows = 0;
		auto n_pt_lights = 0;
		auto n_pt_shadows = 0;
		if (mode == Shaded)
		{
			draw_data.reset(PassLight, 0);
			for (auto n : camera_culled_nodes)
			{
				n->draw(draw_data);
				auto n_lights = n_dir_lights + n_pt_lights;
				if (n_lights < draw_data.lights.size())
				{
					for (auto i = n_lights; i < draw_data.lights.size(); i++)
					{
						auto& l = draw_data.lights[i];
						switch (l.type)
						{
						case LightDirectional:
							buf_lighting.item_d("dir_lights_list"_h, n_dir_lights).set(l.ins_id);
							if (l.cast_shadow)
							{
								if (n_dir_shadows < countof(dir_shadows))
								{
									auto idx = n_dir_shadows;
									auto pi = buf_lighting.item("dir_lights"_h, l.ins_id);
									pi = pi.item("shadow_index"_h);
									pi.set(idx);
									buf_lighting.mark_dirty(pi);

									auto& rot = dir_shadows[idx].rot;
									rot = n->g_rot;
									rot[2] *= -1.f;

									n_dir_shadows++;
								}
							}
							n_dir_lights++;
							break;
						case LightPoint:
							buf_lighting.item_d("pt_lights_list"_h, n_dir_lights).set(l.ins_id);
							if (l.cast_shadow)
							{
								if (n_pt_shadows < countof(pt_shadows))
								{

								}
							}
							n_pt_lights++;
							break;
						}
					}
				}
			}

			buf_lighting.item_d("dir_lights_count"_h).set(n_dir_lights);
			buf_lighting.item_d("pt_lights_count"_h).set(n_pt_lights);
		}
		else if (mode == CameraLight)
		{
			auto pl = buf_lighting.item_d("dir_lights"_h, camera_light_id);
			pl.item("dir"_h).set(camera->node->g_rot[2]);
			pl.item("color"_h).set(vec3(1.f));
			pl.item("shadow_index"_h).set(-1);

			buf_lighting.item_d("dir_lights_list"_h, 0).set(camera_light_id);
			buf_lighting.item_d("dir_lights_count"_h).set(1);
			buf_lighting.item_d("pt_lights_count"_h).set(0);
		}

		buf_lighting.upload(cb);

		// occulder pass

		if (mode == Shaded)
		{
			for (auto i = 0; i < DirShadowMaxCount; i++)
			{
				auto& s = dir_shadows[i];
				for (auto j = 0; j < DirShadowMaxLevels; j++)
				{
					for (auto& b : s.batcher[j].batches)
						b.second.second.clear();
				}
			}

			if (shadow_distance > 0.f)
			{
				auto zn = camera->zNear; auto zf = camera->zFar;
				for (auto i = 0; i < n_dir_shadows; i++)
				{
					auto& s = dir_shadows[i];
					auto splits = vec4(zf);
					auto p_shadow = buf_lighting.item_d("dir_shadows"_h, i);
					auto mats = (mat4*)p_shadow.item("mats"_h).pdata;
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
						draw_data.reset(PassInstance, 0);
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
						auto n_mesh_draws = 0;
						auto n_terrain_draws = 0;
						auto n_MC_draws = 0;

						draw_data.reset(PassOcculder, CateMesh | CateTerrain | CateMarchingCubes);
						for (auto n : s.culled_nodes)
						{
							n->draw(draw_data);
							if (draw_data.meshes.size() > n_mesh_draws)
							{
								auto r = n->bounds.radius();
								auto d = dot(n->g_pos - c, s.rot[2]);
								z_min = min(d - r, z_min);
								z_max = max(d + r, z_max);

								n_mesh_draws = draw_data.meshes.size();
							}
							if (draw_data.terrains.size() > n_terrain_draws)
							{
								for (auto& p : n->bounds.get_points())
								{
									auto d = dot(p - c, s.rot[2]);
									z_min = min(d, z_min);
									z_max = max(d, z_max);
								}

								n_terrain_draws = draw_data.terrains.size();
							}
							if (draw_data.volumes.size() > n_MC_draws)
							{
								for (auto& p : n->bounds.get_points())
								{
									auto d = dot(p - c, s.rot[2]);
									z_min = min(d, z_min);
									z_max = max(d, z_max);
								}

								n_MC_draws = draw_data.volumes.size();
							}
						}
						s.batcher[lv].collect_idrs(draw_data, cb, "DEPTH_ONLY"_h);
						s.draw_terrains[lv] = draw_data.terrains;
						s.draw_MCs[lv] = draw_data.volumes;

						proj = orthoRH(-hf_xlen, +hf_xlen, -hf_ylen, +hf_ylen, 0.f, z_max - z_min);
						proj[1][1] *= -1.f;
						view = lookAt(c + s.rot[2] * z_min, c, s.rot[1]);
						proj_view = proj * view;
						mats[lv] = proj_view;
						s.frustum = Frustum(inverse(proj_view));
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

					p_shadow.item("splits"_h).set(splits);
					p_shadow.item("far"_h).set(shadow_distance);
				}

				for (auto i = 0; i < n_pt_shadows; i++)
				{

				}
			}

			csm_debug_sig = false;

			auto set_blur_args = [cb](const vec2 img_size) {
				pl_blur.prm.pc.item_d("off"_h).set(-3);
				pl_blur.prm.pc.item_d("len"_h).set(7);
				pl_blur.prm.pc.item_d("pxsz"_h).set(1.f / img_size);
				const auto len = 7;
				pl_blur.prm.pc.itemv_d("weights"_h, len).set(get_gauss_blur_weights(len));
				pl_blur.prm.push_constant(cb);
			};

			cb->set_viewport_and_scissor(Rect(vec2(0), ShadowMapSize));
			for (auto i = 0; i < n_dir_shadows; i++)
			{
				auto& s = dir_shadows[i];
				for (auto lv = 0U; lv < csm_levels; lv++)
				{
					cb->begin_renderpass(nullptr, imgs_dir_shadow[i]->get_shader_write_dst(0, lv, graphics::AttachmentLoadClear), { vec4(1.f, 0.f, 0.f, 0.f) });
					prm_fwd.bind_dss(cb);
					prm_fwd.pc.item_d("i"_h).set(ivec4(0, i, lv, 0));
					prm_fwd.push_constant(cb);

					s.batcher[lv].draw(cb);

					for (auto& t : s.draw_terrains[lv])
					{
						cb->bind_pipeline(get_material_pipeline(mat_reses[t.mat_id], "terrain"_h, 0, "DEPTH_ONLY"_h));
						prm_fwd.pc.item_d("index"_h).set((t.ins_id << 16) + t.mat_id);
						prm_fwd.push_constant(cb);
						cb->draw(4, t.blocks.x * t.blocks.y, 0, (t.ins_id << 24) + (t.mat_id << 16));
					}
					for (auto& v : s.draw_MCs[lv])
					{
						cb->bind_pipeline(get_material_pipeline(mat_reses[v.mat_id], "marching_cubes"_h, 0, "DEPTH_ONLY"_h));
						prm_fwd.pc.item_d("index"_h).set((v.ins_id << 16) + v.mat_id);
						for (auto z = 0; z < v.blocks.z; z++)
						{
							for (auto y = 0; y < v.blocks.y; y++)
							{
								for (auto x = 0; x < v.blocks.x; x++)
								{
									prm_fwd.pc.item_d("offset"_h).set(vec3(x, y, z));
									prm_fwd.push_constant(cb);
									// 128 / 4 = 32
									cb->draw_mesh_tasks(uvec3(32 * 32 * 32, 1, 1));
								}
							}
						}
					}

					cb->end_renderpass();
				}
				cb->image_barrier(imgs_dir_shadow[i].get(), { 0U, 1U, 0U, csm_levels }, graphics::ImageLayoutShaderReadOnly);
			}

			set_blur_args(vec2(ShadowMapSize));
			for (auto i = 0; i < n_dir_shadows; i++)
			{
				for (auto lv = 0U; lv < csm_levels; lv++)
				{
					cb->image_barrier(img_dir_shadow_back.get(), { 0U, 1U, lv, 1 }, graphics::ImageLayoutAttachment);
					cb->begin_renderpass(nullptr, img_dir_shadow_back->get_shader_write_dst(0, lv));
					cb->bind_pipeline(pl_blur.pls["DEPTH_H"_h]);
					cb->bind_descriptor_set(0, imgs_dir_shadow[i]->get_shader_read_src(0, lv));
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->image_barrier(img_dir_shadow_back.get(), { 0U, 1U, lv, 1 }, graphics::ImageLayoutShaderReadOnly);
					cb->begin_renderpass(nullptr, imgs_dir_shadow[i]->get_shader_write_dst(0, lv));
					cb->bind_pipeline(pl_blur.pls["DEPTH_H"_h]);
					cb->bind_descriptor_set(0, img_dir_shadow_back->get_shader_read_src(0, lv));
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();
				}
				cb->image_barrier(imgs_dir_shadow[i].get(), { 0U, 1U, 0U, csm_levels }, graphics::ImageLayoutShaderReadOnly);
			}

			cb->set_viewport_and_scissor(Rect(vec2(0), ShadowMapSize / 2U));
		}

		cb->set_viewport_and_scissor(Rect(vec2(0), ext));

		// deferred shading pass

		for (auto& b : opa_batcher.batches)
			b.second.second.clear();
		draw_data.reset(PassGBuffer, CateMesh | CateTerrain | CateSDF | CateMarchingCubes);
		for (auto n : camera_culled_nodes)
			n->draw(draw_data);
		opa_batcher.collect_idrs(draw_data, cb);

		cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutAttachment);
		cb->set_viewport_and_scissor(Rect(vec2(0), ext));

		cb->begin_renderpass(nullptr, fb_gbuf.get(),
			{ vec4(0.f, 0.f, 0.f, 0.f),
			vec4(0.f, 0.f, 0.f, 0.f),
			vec4(0.f, 0.f, 0.f, 0.f),
			vec4(0.f, 0.f, 0.f, 0.f),
			vec4(1.f, 0.f, 0.f, 0.f) });

		prm_gbuf.bind_dss(cb);
		opa_batcher.draw(cb);

		for (auto& t : draw_data.terrains)
		{
			cb->bind_pipeline(get_material_pipeline(mat_reses[t.mat_id], "terrain"_h, 0, 0));
			prm_gbuf.pc.item_d("index"_h).set((t.ins_id << 16) + t.mat_id);
			prm_fwd.push_constant(cb);
			cb->draw(4, t.blocks.x * t.blocks.y, 0, 0);
		}
		for (auto& s : draw_data.sdfs)
		{
			cb->bind_pipeline(get_material_pipeline(mat_reses[s.mat_id], "sdf"_h, 0, 0));
			prm_gbuf.pc.item_d("index"_h).set((s.ins_id << 16) + s.mat_id);
			prm_fwd.push_constant(cb);
			cb->draw(3, 1, 0, 0);
		}
		for (auto& v : draw_data.volumes)
		{
			cb->bind_pipeline(get_material_pipeline(mat_reses[v.mat_id], "marching_cubes"_h, 0, 0));
			prm_gbuf.pc.item_d("index"_h).set((v.ins_id << 16) + v.mat_id);
			for (auto z = 0; z < v.blocks.z; z++)
			{
				for (auto y = 0; y < v.blocks.y; y++)
				{
					for (auto x = 0; x < v.blocks.x; x++)
					{
						prm_gbuf.pc.item_d("offset"_h).set(vec3(x, y, z));
						prm_gbuf.push_constant(cb);
						// 128 / 4 = 32
						cb->draw_mesh_tasks(uvec3(32 * 32 * 32, 1, 1));
					}
				}
			}
		}

		cb->end_renderpass();

		cb->image_barrier(img_last_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->image_barrier(img_last_dep.get(), {}, graphics::ImageLayoutShaderReadOnly);

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

		cb->image_barrier(img_gbufferA.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->image_barrier(img_gbufferB.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->image_barrier(img_gbufferC.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->image_barrier(img_gbufferD.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->image_barrier(img_dep.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->begin_renderpass(nullptr, img_dst->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
		pl_deferred.prm.bind_dss(cb);
		cb->bind_pipeline(get_deferred_pipeline(pl_mod));
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();

		// forward pass

		cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->begin_renderpass(nullptr, img_dst_ms->get_shader_write_dst());
		cb->bind_pipeline(pl_blit);
		cb->bind_descriptor_set(0, img_dst->get_shader_read_src(0, 0, sp_nearest));
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();

		cb->image_barrier(img_dep.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->begin_renderpass(nullptr, img_dep_ms->get_shader_write_dst());
		cb->bind_pipeline(pl_blit_dep);
		cb->bind_descriptor_set(0, img_dep->get_shader_read_src(0, 0, sp_nearest));
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();

		for (auto& b : trs_batcher.batches)
			b.second.second.clear();
		draw_data.reset(PassForward, CateMesh | CateGrassField | CateParticle);
		for (auto n : camera_culled_nodes)
			n->draw(draw_data);
		trs_batcher.collect_idrs(draw_data, cb);

		for (auto& p : draw_data.particles)
		{
			for (auto& ptc : p.ptcs)
			{
				auto pv = buf_particles.add();
				pv.item("i_pos"_h).set(ptc.pos);
				pv.item("i_xext"_h).set(ptc.x_ext);
				pv.item("i_yext"_h).set(ptc.y_ext);
				pv.item("i_uv"_h).set(ptc.uv);
				pv.item("i_col"_h).set(ptc.col);
				pv.item("i_time"_h).set(ptc.time);
			}
		}
		buf_particles.upload(cb);
		buf_particles.buf_top = buf_particles.stag_top = 0;

		cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutAttachment);
		cb->image_barrier(img_dep.get(), {}, graphics::ImageLayoutAttachment);
		cb->begin_renderpass(nullptr, fb_fwd.get());
		prm_fwd.bind_dss(cb);

		trs_batcher.draw(cb);

		for (auto& t : draw_data.terrains)
		{
			cb->bind_pipeline(get_material_pipeline(mat_reses[t.mat_id], "grass_field"_h, 0, 0));
			prm_fwd.pc.item_d("index"_h).set((t.ins_id << 16) + t.mat_id);
			prm_fwd.push_constant(cb);
			cb->draw(4, t.blocks.x * t.blocks.y, 0, (t.ins_id << 24) + (t.mat_id << 16));
		}

		{
			cb->bind_vertex_buffer(buf_particles.buf.get(), 0);
			auto vtx_off = 0;
			for (auto& p : draw_data.particles)
			{
				cb->bind_pipeline(get_material_pipeline(mat_reses[p.mat_id], "particle"_h, 0, 0));
				cb->draw(p.ptcs.size(), 1, vtx_off, p.mat_id << 16);
				vtx_off += p.ptcs.size();
			}
		}

		cb->end_renderpass();

		cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->image_barrier(img_dep.get(), {}, graphics::ImageLayoutShaderReadOnly);

		cb->begin_renderpass(nullptr, img_last_dst->get_shader_write_dst());
		cb->bind_pipeline(pl_blit);
		cb->bind_descriptor_set(0, img_dst->get_shader_read_src(0, 0, sp_nearest));
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();

		cb->begin_renderpass(nullptr, img_last_dep->get_shader_write_dst());
		cb->bind_pipeline(pl_blit_dep);
		cb->bind_descriptor_set(0, img_dep->get_shader_read_src(0, 0, sp_nearest));
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();

		// post processing
		if (mode == Shaded || mode == CameraLight && post_processing_enable)
		{
			if (ssao_enable)
			{
			}

			if (bloom_enable)
			{
				cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
				cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst());
				cb->bind_pipeline(pl_bloom.pls["BRIGHT"_h]);
				cb->bind_descriptor_set(0, img_dst->get_shader_read_src(0, 0, sp_nearest));
				pl_bloom.prm.pc.item_d("white_point"_h).set(white_point);
				pl_bloom.prm.push_constant(cb);
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();

				for (auto i = 1; i < img_back0->n_levels; i++)
				{
					cb->image_barrier(img_back0.get(), { (uint)i - 1, 1, 0, 1 }, graphics::ImageLayoutShaderReadOnly);
					cb->set_viewport(Rect(vec2(0.f), vec2(img_back0->levels[i].extent)));
					cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(i));
					cb->bind_pipeline(pl_bloom.pls["DOWNSAMPLE"_h]);
					cb->bind_descriptor_set(0, img_back0->get_shader_read_src(i - 1));
					pl_bloom.prm.pc.item_d("pxsz"_h).set(1.f / vec2(img_back0->levels[i - 1].extent));
					pl_bloom.prm.push_constant(cb);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();
				}
				for (auto i = (int)img_back0->n_levels - 1; i > 1; i--)
				{
					cb->image_barrier(img_back0.get(), { (uint)i, 1, 0, 1 }, graphics::ImageLayoutShaderReadOnly);
					cb->set_viewport(Rect(vec2(0.f), vec2(img_back0->levels[i - 1].extent)));
					cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(i - 1));
					cb->bind_pipeline(pl_bloom.pls["UPSAMPLE"_h]);
					cb->bind_descriptor_set(0, img_back0->get_shader_read_src(i));
					pl_bloom.prm.pc.item_d("pxsz"_h).set(1.f / vec2(img_back0->levels[i].extent));
					pl_bloom.prm.push_constant(cb);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();
				}

				cb->set_viewport(Rect(vec2(0), ext));
				cb->image_barrier(img_back0.get(), { 1U }, graphics::ImageLayoutShaderReadOnly);
				cb->begin_renderpass(nullptr, img_dst->get_shader_write_dst());
				cb->bind_pipeline(pl_bloom.pls["UPSAMPLE"_h]);
				cb->bind_descriptor_set(0, img_back0->get_shader_read_src(1));
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();
			}

			if (tone_mapping_enable)
			{
				cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutGeneral);
				pl_luma.prm.bind_dss(cb);
				const auto min_log_luma = -5.f;
				const auto max_log_luma = +5.f;
				pl_luma.prm.pc.item_d("min_log_luma"_h).set(min_log_luma);
				pl_luma.prm.pc.item_d("log_luma_range"_h).set(max_log_luma - min_log_luma);
				pl_luma.prm.pc.item_d("time_coeff"_h).set(1.0f);
				pl_luma.prm.pc.item_d("num_pixels"_h).set(int(ext.x * ext.y));
				pl_luma.prm.push_constant(cb);
				cb->bind_pipeline(pl_luma.pls["hist"_h]);
				cb->dispatch(uvec3(ceil(ext.x / 16), ceil(ext.y / 16), 1));
				cb->buffer_barrier(buf_luma.buf.get(), graphics::AccessShaderRead | graphics::AccessShaderWrite,
					graphics::AccessShaderRead | graphics::AccessShaderWrite,
					graphics::PipelineStageCompShader, graphics::PipelineStageCompShader);
				cb->bind_pipeline(pl_luma.pls["avg"_h]);
				cb->dispatch(uvec3(256, 1, 1));
				cb->buffer_barrier(buf_luma.buf.get(), graphics::AccessShaderRead | graphics::AccessShaderWrite,
					graphics::AccessHostRead,
					graphics::PipelineStageCompShader, graphics::PipelineStageHost);

				auto p_avg_luma = buf_luma.item("avg"_h);
				cb->copy_buffer(buf_luma.buf.get(), buf_luma.stag.get(), graphics::BufferCopy(buf_luma.offset(p_avg_luma), p_avg_luma.size));
				cb->buffer_barrier(buf_luma.buf.get(), graphics::AccessHostRead,
					graphics::AccessShaderRead | graphics::AccessShaderWrite,
					graphics::PipelineStageHost, graphics::PipelineStageCompShader);

				cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
				cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst());
				cb->bind_pipeline(pl_blit);
				cb->bind_descriptor_set(0, img_dst->get_shader_read_src(0, 0, sp_nearest));
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();

				cb->image_barrier(img_back0.get(), {}, graphics::ImageLayoutShaderReadOnly);
				cb->begin_renderpass(nullptr, img_back1->get_shader_write_dst());
				cb->bind_pipeline(pl_tone.pls[0]);
				pl_tone.prm.bind_dss(cb);
				pl_tone.prm.pc.item_d("average_luminance"_h).set(*(float*)p_avg_luma.pdata);
				pl_tone.prm.pc.item_d("white_point"_h).set(white_point);
				pl_tone.prm.pc.item_d("one_over_gamma"_h).set(1.f / gamma);
				pl_tone.prm.push_constant(cb);
				cb->bind_descriptor_set(0, img_back0->get_shader_read_src(0, 0, sp_nearest));
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();

				cb->image_barrier(img_back1.get(), {}, graphics::ImageLayoutShaderReadOnly);
				cb->begin_renderpass(nullptr, img_dst->get_shader_write_dst());
				cb->bind_pipeline(pl_fxaa.pls[0]);
				pl_fxaa.prm.pc.item_d("pxsz"_h).set(1.f / (vec2)img_dst->extent);
				pl_fxaa.prm.push_constant(cb);
				cb->bind_descriptor_set(0, img_back1->get_shader_read_src());
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();
			}
		}

		auto blur_pass = [&](int w = 3) {
			pl_blur.prm.pc.item_d("off"_h).set(-w);
			pl_blur.prm.pc.item_d("len"_h).set(w * 2 + 1);
			pl_blur.prm.pc.item_d("pxsz"_h).set(1.f / (vec2)img_back0->extent);
			pl_blur.prm.push_constant(cb);

			cb->image_barrier(img_back0.get(), {}, graphics::ImageLayoutShaderReadOnly);
			cb->begin_renderpass(nullptr, img_back1->get_shader_write_dst());
			cb->bind_pipeline(pl_blur.pls["LOCAL_MAX_H"_h]);
			cb->bind_descriptor_set(0, img_back0->get_shader_read_src());
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();

			cb->image_barrier(img_back1.get(), {}, graphics::ImageLayoutShaderReadOnly);
			cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst());
			cb->bind_pipeline(pl_blur.pls["LOCAL_MAX_V"_h]);
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

		draw_data.reset(PassOutline, 0);
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
				prm_fwd.pc.item_d("f"_h).set(vec4(m.color) / 255.f);
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

			blur_pass(draw_data.line_width);

			cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
			for (auto i = 0; i < n; i++)
			{
				auto& m = draw_data.meshes[i];
				auto& mesh_r = mesh_reses[m.mesh_id];

				prm_fwd.bind_dss(cb);
				prm_fwd.pc.item_d("f"_h).set(vec4(0.f));
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
		for (auto& t : draw_data.terrains)
		{
			cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), { vec4(0.f) });
			prm_fwd.bind_dss(cb);
			prm_fwd.pc.item_d("index"_h).set((t.ins_id << 16) + t.mat_id);
			prm_fwd.pc.item_d("f"_h).set(vec4(t.color) / 255.f);
			prm_fwd.push_constant(cb);
			cb->bind_pipeline(pl_terrain_plain);
			cb->draw(4, t.blocks.x* t.blocks.y, 0, t.ins_id << 24);
			cb->end_renderpass();

			blur_pass(draw_data.line_width);

			cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
			prm_fwd.bind_dss(cb);
			prm_fwd.pc.item_d("f"_h).set(vec4(0.f));
			prm_fwd.push_constant(cb);
			cb->bind_pipeline(pl_terrain_plain);
			cb->draw(4, t.blocks.x* t.blocks.y, 0, t.ins_id << 24);
			cb->end_renderpass();

			blend_pass();
		}

		draw_data.reset(PassPrimitive, 0);
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
		prm_plain.pc.item_d("mvp"_h).set(camera->proj_view_mat);
		prm_plain.push_constant(cb);
		{
			auto vtx_off = 0;
			for (auto& d : draw_data.primitives)
			{
				auto col_item = prm_plain.pc.item_d("col"_h);
				col_item.set(vec4(d.color) / 255.f);
				prm_plain.pc.mark_dirty(col_item);
				prm_plain.push_constant(cb);
				switch (d.type)
				{
				case "LineList"_h:
					cb->bind_pipeline(pl_line3d);
					break;
				case "LineStrip"_h:
					cb->bind_pipeline(pl_line_strip3d);
					break;
				case "TriangleList"_h:
					cb->bind_pipeline(pl_triangle3d);
					break;
				}
				cb->draw(d.points.size(), 1, vtx_off, 0);
				vtx_off += d.points.size();
			}
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

		auto sz = vec2(img_pickup->extent);
		if (screen_pos.x >= sz.x || screen_pos.y >= sz.y)
			return nullptr;

		graphics::InstanceCommandBuffer cb(fence_pickup.get());

		cb->set_viewport(Rect(vec2(0), sz));
		cb->set_scissor(Rect(vec2(screen_pos), vec2(screen_pos + 1U)));
		cb->begin_renderpass(nullptr, fb_pickup.get(), { vec4(0.f), vec4(1.f, 0.f, 0.f, 0.f) });
		prm_fwd.bind_dss(cb.get());

		std::vector<cNodePtr> nodes;

		auto n_mesh_draws = 0;
		auto n_terrain_draws = 0;
		auto n_MC_draws = 0;
		draw_data.reset(PassPickUp, CateMesh | CateTerrain | CateMarchingCubes);
		std::vector<cNodePtr> camera_culled_nodes; // collect here (again), because there may have changes between render() and pick_up()
		sScene::instance()->octree->get_within_frustum(camera->frustum, camera_culled_nodes);
		for (auto n : camera_culled_nodes)
		{
			if (draw_callback)
				draw_callback(n, draw_data);
			else
				n->draw(draw_data);

			for (auto i = n_mesh_draws; i < draw_data.meshes.size(); i++)
			{
				auto& m = draw_data.meshes[i];
				auto& mesh_r = mesh_reses[m.mesh_id];
				if (!mesh_r.arm)
				{
					cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
					cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
					cb->bind_pipeline(pl_mesh_pickup);
					prm_fwd.pc.item_d("i"_h).set(ivec4((int)nodes.size() + 1, 0, 0, 0));
					prm_fwd.push_constant(cb.get());
					cb->draw_indexed(mesh_r.idx_cnt, mesh_r.idx_off, mesh_r.vtx_off, 1, m.ins_id << 8);
				}
				else
				{
					cb->bind_vertex_buffer(buf_vtx_arm.buf.get(), 0);
					cb->bind_index_buffer(buf_idx_arm.buf.get(), graphics::IndiceTypeUint);
					cb->bind_pipeline(pl_mesh_arm_pickup);
					prm_fwd.pc.item_d("i"_h).set(ivec4((int)nodes.size() + 1, 0, 0, 0));
					prm_fwd.push_constant(cb.get());
					cb->draw_indexed(mesh_r.idx_cnt, mesh_r.idx_off, mesh_r.vtx_off, 1, m.ins_id << 8);
				}

				nodes.push_back(n);
			}
			n_mesh_draws = draw_data.meshes.size();

			for (auto i = n_terrain_draws; i < draw_data.terrains.size(); i++)
			{
				cb->bind_pipeline(pl_terrain_pickup);
				auto& t = draw_data.terrains[i];
				prm_fwd.pc.item_d("index"_h).set((t.ins_id << 16) + t.mat_id);
				prm_fwd.pc.item_d("i"_h).set(ivec4((int)nodes.size() + 1, 0, 0, 0));
				prm_fwd.push_constant(cb.get());
				cb->draw(4, t.blocks.x * t.blocks.y, 0, t.ins_id << 24);

				nodes.push_back(n);
			}
			n_terrain_draws = draw_data.terrains.size();

			for (auto i = n_MC_draws; i < draw_data.volumes.size(); i++)
			{
				cb->bind_pipeline(pl_MC_pickup);
				auto& v = draw_data.volumes[i];
				prm_fwd.pc.item_d("index"_h).set((v.ins_id << 16) + v.mat_id);
				prm_fwd.pc.item_d("i"_h).set(ivec4((int)nodes.size() + 1, 0, 0, 0));
				for (auto z = 0; z < v.blocks.z; z++)
				{
					for (auto y = 0; y < v.blocks.y; y++)
					{
						for (auto x = 0; x < v.blocks.x; x++)
						{
							prm_fwd.pc.item_d("offset"_h).set(vec3(x, y, z));
							prm_fwd.push_constant(cb.get());
							// 128 / 4 = 32
							cb->draw_mesh_tasks(uvec3(32 * 32 * 32, 1, 1));
						}
					}
				}

				nodes.push_back(n);
			}
			n_MC_draws = draw_data.volumes.size();
		}

		cb->end_renderpass();
		if (draw_data.graphics_debug)
			graphics::Debug::start_capture_frame();
		cb.excute();
		if (draw_data.graphics_debug)
			graphics::Debug::end_capture_frame();

		int index; uint depth;
		graphics::StagingBuffer sb(sizeof(index) + sizeof(depth), nullptr, graphics::BufferUsageTransferDst);
		{
			graphics::InstanceCommandBuffer cb(nullptr);
			graphics::BufferImageCopy cpy;
			cpy.img_off = uvec3(screen_pos, 0);
			cpy.img_ext = uvec3(1U);
			cb->image_barrier(img_pickup.get(), cpy.img_sub, graphics::ImageLayoutTransferSrc);
			cb->copy_image_to_buffer(img_pickup.get(), sb.get(), cpy);
			cb->image_barrier(img_pickup.get(), cpy.img_sub, graphics::ImageLayoutAttachment);
			if (out_pos)
			{
				cpy.buf_off = sizeof(uint);
				cb->image_barrier(img_dep_pickup.get(), cpy.img_sub, graphics::ImageLayoutTransferSrc);
				cb->copy_image_to_buffer(img_dep_pickup.get(), sb.get(), cpy);
				cb->image_barrier(img_dep_pickup.get(), cpy.img_sub, graphics::ImageLayoutAttachment);
			}

			cb.excute();
		}
		memcpy(&index, sb->mapped, sizeof(index));
		if (out_pos)
		{
			memcpy(&depth, (char*)sb->mapped + sizeof(index), sizeof(depth));
			float depth_f;
			if (dep_fmt == graphics::Format::Format_Depth16)
				depth_f = depth / 65535.f;
			else
				depth_f = *(float*)&depth;
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

	std::vector<vec3> sRendererPrivate::transform_feedback(cNodePtr node)
	{
		std::vector<vec3> ret;

		graphics::InstanceCommandBuffer cb;

		buf_transform_feedback.item_d("vertex_count"_h).set(0U);
		buf_transform_feedback.upload(cb.get());

		cb->set_viewport_and_scissor(Rect(vec2(0), vec2(1)));
		cb->begin_renderpass(nullptr, fb_pickup.get(), { vec4(0.f), vec4(1.f, 0.f, 0.f, 0.f) });
		prm_fwd.bind_dss(cb.get());

		draw_data.reset(PassTransformFeedback, CateMesh | CateTerrain | CateMarchingCubes);
		node->draw(draw_data);
		cb->bind_pipeline(pl_MC_transform_feedback);
		for (auto& v : draw_data.volumes)
		{
			prm_fwd.pc.item_d("index"_h).set((v.ins_id << 16) + v.mat_id);
			for (auto z = 0; z < v.blocks.z; z++)
			{
				for (auto y = 0; y < v.blocks.y; y++)
				{
					for (auto x = 0; x < v.blocks.x; x++)
					{
						prm_fwd.pc.item_d("offset"_h).set(vec3(x, y, z));
						prm_fwd.push_constant(cb.get());
						// 128 / 4 = 32
						cb->draw_mesh_tasks(uvec3(32 * 32 * 32, 1, 1));
					}
				}
			}
		}

		cb->end_renderpass();

		buf_transform_feedback.mark_dirty(buf_transform_feedback);
		buf_transform_feedback.download(cb.get());

		cb.excute();

		auto num = buf_transform_feedback.item("vertex_count"_h).get<uint>();
		ret.resize(num);
		auto vertex_x = buf_transform_feedback.itemv("vertex_x"_h, num);
		auto vertex_y = buf_transform_feedback.itemv("vertex_y"_h, num);
		auto vertex_z = buf_transform_feedback.itemv("vertex_z"_h, num);
		for (auto i = 0; i < num; i++)
		{
			ret[i].x = vertex_x.at(i).get<float>();
			ret[i].y = vertex_y.at(i).get<float>();
			ret[i].z = vertex_z.at(i).get<float>();
		}

		return ret;
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
