#include "renderer_private.h"
#include "scene_private.h"
#include "input_private.h"
#include "../octree.h"
#include "../draw_data.h"
#include "../world_private.h"
#include "../components/node_private.h"
#include "../components/element_private.h"
#include "../components/camera_private.h"

#include "../../foundation/typeinfo_serialize.h"
#include "../../foundation/window.h"
#include "../../graphics/device.h"
#include "../../graphics/renderpass.h"
#include "../../graphics/shader.h"
#include "../../graphics/window.h"
#include "../../graphics/material.h"
#include "../../graphics/model.h"
#include "../../graphics/font.h"
#include "../../graphics/extension.h"
#include "../../graphics/debug.h"

namespace flame
{
	const auto col_fmt = graphics::Format::Format_R16G16B16A16_SFLOAT;
	const auto dep_fmt = graphics::Format::Format_Depth32;
	const auto esm_fmt = graphics::Format::Format_R32_SFLOAT;
	const auto sample_count = graphics::SampleCount_4;
	const auto ShadowMapSize = uvec2(2048);
	const auto DirShadowMaxCount = 4U;
	const auto DirShadowMaxLevels = 4U;
	const auto PtShadowMaxCount = 4U; 
	bool use_mesh_shader = false;

	/* SHARED RESOURCES */
	// Reses
	std::vector<sRenderer::MatVar>	mat_vars;
	std::vector<sRenderer::MeshRes>	mesh_reses;
	std::vector<sRenderer::TexRes>	tex_reses;
	std::vector<sRenderer::MatRes>	mat_reses;
	// Instances
	graphics::SparseSlots	mesh_instances;
	graphics::SparseSlots	armature_instances;
	graphics::SparseSlots	terrain_instances;
	graphics::SparseSlots	sdf_instances;
	graphics::SparseSlots	volume_instances;
	graphics::StorageBuffer buf_marching_cubes_loopup;
	graphics::StorageBuffer buf_transform_feedback;
	graphics::SparseSlots	dir_lights;
	graphics::SparseSlots	pt_lights;
	// Buffers
	graphics::StorageBuffer						buf_instance;
	graphics::StorageBuffer						buf_material;
	graphics::StorageBuffer						buf_lighting;
	graphics::StorageBuffer						buf_luma;
	std::unique_ptr<graphics::DescriptorSet>	ds_instance;
	std::unique_ptr<graphics::DescriptorSet>	ds_material;
	std::unique_ptr<graphics::DescriptorSet>	ds_lighting;
	// Vertex Buffers
	graphics::VertexBuffer	buf_vtx;
	graphics::IndexBuffer	buf_idx;
	graphics::VertexBuffer	buf_vtx_arm;
	graphics::IndexBuffer	buf_idx_arm;
	graphics::VertexBuffer	buf_particles;
	graphics::VertexBuffer	buf_primitives;
	// Render Passes
	graphics::RenderpassPtr rp_fwd = nullptr;
	graphics::RenderpassPtr rp_fwd_clear = nullptr;
	graphics::RenderpassPtr rp_gbuf = nullptr;
	graphics::RenderpassPtr rp_dst = nullptr;
	graphics::RenderpassPtr rp_dep = nullptr;
	graphics::RenderpassPtr rp_col_dep = nullptr;
	graphics::RenderpassPtr rp_esm = nullptr;
	graphics::RenderpassPtr rp_primitive = nullptr;
	// Images
	std::unique_ptr<graphics::Image> img_black;
	std::unique_ptr<graphics::Image> img_white;
	std::unique_ptr<graphics::Image> img_dummy_depth;
	std::unique_ptr<graphics::Image> img_cube_black;
	std::unique_ptr<graphics::Image> img_cube_white;
	std::unique_ptr<graphics::Image> img_black3D;
	std::unique_ptr<graphics::Image> img_white3D;
	// Shadow Maps
	std::unique_ptr<graphics::Image>					img_shadow_depth;
	std::vector<std::unique_ptr<graphics::Image>>		imgs_dir_shadow;
	std::vector<std::unique_ptr<graphics::Image>>		imgs_pt_shadow;
	std::vector<std::unique_ptr<graphics::Framebuffer>> fbs_dir_shadow;
	std::vector<std::unique_ptr<graphics::Framebuffer>> fbs_pt_shadow;
	std::unique_ptr<graphics::Image>					img_dir_shadow_back;
	std::unique_ptr<graphics::Image>					img_pt_shadow_back;
	// Generic Pipeline Layouts
	graphics::PipelineLayoutPtr pll_fwd = nullptr;
	graphics::PipelineLayoutPtr pll_gbuf = nullptr;
	graphics::PipelineLayoutPtr pll_deferred = nullptr;
	// Generic Pipelines
	graphics::GraphicsPipelinePtr pl_blit = nullptr;
	graphics::GraphicsPipelinePtr pl_blit_blend = nullptr;
	graphics::GraphicsPipelinePtr pl_blit_dep = nullptr;
	graphics::GraphicsPipelinePtr pl_add = nullptr;
	graphics::GraphicsPipelinePtr pl_blend = nullptr;
	// Special Geometry Pipelines
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
	graphics::GraphicsPipelinePtr pl_point3d = nullptr;
	graphics::GraphicsPipelinePtr pl_point3d_dt = nullptr;
	graphics::GraphicsPipelinePtr pl_line3d = nullptr;
	graphics::GraphicsPipelinePtr pl_line3d_dt = nullptr;
	graphics::GraphicsPipelinePtr pl_line_strip3d = nullptr;
	graphics::GraphicsPipelinePtr pl_line_strip3d_dt = nullptr;
	graphics::GraphicsPipelinePtr pl_triangle3d = nullptr;
	graphics::GraphicsPipelinePtr pl_triangle3d_dt = nullptr;
	// Deferred Shading Pipelines With Different Modifiers
	std::unordered_map<uint, graphics::GraphicsPipelinePtr> pls_deferred;
	// Post-Processing Pipelines
	graphics::GraphicsPipelinePtr				pl_down_sample;
	graphics::GraphicsPipelinePtr				pl_up_sample;
	graphics::GraphicsPipelinePtr				pl_down_sample_depth;
	graphics::GraphicsPipelinePtr				pl_up_sample_depth;
	graphics::GraphicsPipelinePtr				pl_outline_pp;
	graphics::ComputePipelinePtr				pl_luma_hist;
	graphics::ComputePipelinePtr				pl_luma_avg;
	graphics::GraphicsPipelinePtr				pl_bloom_bright;
	graphics::GraphicsPipelinePtr				pl_blur_h;
	graphics::GraphicsPipelinePtr				pl_blur_v;
	graphics::GraphicsPipelinePtr				pl_blur_depth_h;
	graphics::GraphicsPipelinePtr				pl_blur_depth_v;
	graphics::GraphicsPipelinePtr				pl_blur_max_h;
	graphics::GraphicsPipelinePtr				pl_blur_max_v;
	graphics::GraphicsPipelinePtr				pl_dof;
	graphics::GraphicsPipelinePtr				pl_tone;
	graphics::GraphicsPipelinePtr				pl_fxaa;
	/****************************************/

	int camera_light_id = -1;
	int white_tex_id = -1;
	int black_tex_id = -1;
	int rand_tex_id = -1;

	uint cel_shading_levels = 0;

	std::filesystem::path post_shading_code_file = L"";

	struct MeshBatcher
	{
		struct Batch
		{
			bool				armature;
			std::vector<uint>	draw_indices;
			uint				sub_cmd_offset;
			uint				sub_cmd_count;
		};

		graphics::IndirectBuffer buf_idr;
		std::unordered_map<graphics::GraphicsPipelinePtr, Batch> batches;

		void collect(RenderMode render_mode, const DrawData& draw_data, graphics::CommandBufferPtr cb, uint mod2 = 0);
		void draw(graphics::CommandBufferPtr cb);
	};

	struct DirShadow
	{
		mat3 rot;
		Frustum frustum;
		std::vector<std::pair<EntityPtr, cNodePtr>> culled_nodes;
		MeshBatcher batcher[DirShadowMaxLevels];
		std::vector<TerrainDrawData> draw_terrains[DirShadowMaxLevels];
		std::vector<VolumeDrawData> draw_MCs[DirShadowMaxLevels];
	};

	struct PointShadow
	{

	};

	std::vector<std::pair<EntityPtr, cNodePtr>> camera_culled_nodes;
	DrawData draw_data;
	MeshBatcher gbuffer_batcher;
	MeshBatcher transparent_batcher;
	DirShadow dir_shadows[DirShadowMaxCount];
	PointShadow pt_shadows[PtShadowMaxCount];

	bool csm_debug_capture_flag = false;
	struct Primitives
	{
		PrimitiveType type;
		std::vector<vec3> points;
		cvec4 color;
		float depth_test;
	};
	std::vector<Primitives> csm_debug_draws;

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
		if (cel_shading_levels > 0)
			defines.push_back("frag:CEL_SHADING");
		if (!post_shading_code_file.empty())
		{
			if (auto path = Path::get(post_shading_code_file); !path.empty())
				defines.push_back(std::format("frag:POST_SHADING_CODE={}", path.string()));
		}
	}

	static graphics::GraphicsPipelinePtr get_material_pipeline(RenderMode render_mode, sRenderer::MatRes& res, uint type, uint modifier1 = 0, uint modifier2 = 0)
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
				defines.push_back("rp=" + str(rp_esm));
				defines.push_back("pll=" + str(pll_fwd));
				//defines.push_back("cull_mode=" + TypeInfo::serialize_t(graphics::CullModeFront));
			}
			else if (modifier2 == "FORCE_FORWARD"_h)
			{
				defines.push_back("rp=" + str(rp_fwd));
				defines.push_back("pll=" + str(pll_fwd));
			}
			else if (res.mat->render_queue == graphics::RenderQueue::Opaque || 
				res.mat->render_queue == graphics::RenderQueue::AlphaTest)
			{
				defines.push_back("rp=" + str(rp_gbuf));
				defines.push_back("pll=" + str(pll_gbuf));
				defines.push_back("all_shader:GBUFFER_PASS");
			}
			else if (res.mat->render_queue == graphics::RenderQueue::Transparent)
			{
				defines.push_back("rp=" + str(rp_fwd));
				defines.push_back("pll=" + str(pll_fwd));
				defines.push_back("blend=true");
			}
			break;
		case "grass_field"_h:
		case "particle"_h:
			defines.push_back("rp=" + str(rp_fwd));
			defines.push_back("pll=" + str(pll_fwd));
			defines.push_back("frag:UNLIT");
			break;
		}

		if (render_mode == RenderModeWireframe)
			defines.push_back("pm=" + TypeInfo::serialize_t(graphics::PolygonModeLine));

		combine_global_defines(defines);
		if (!res.mat->code_file.empty())
		{
			auto path = res.mat->code_file;
			if (path.extension() == L".bp")
				path += L".glsl";
			defines.push_back(std::format("frag:MAT_CODE={}", Path::get(path).string()));
		}
		for (auto& d : res.defines)
		{
			if (!d.empty())
				defines.push_back(d);
		}

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
			auto path = res.mat->code_file;
			if (path.extension() == L".bp")
				path += L".glsl";
			auto dependency = Path::get(path);
			ret->frag()->dependencies.emplace_back(dependency);
			ret->dependencies.emplace_back(dependency);
		}
		return ret;
	}


	static graphics::GraphicsPipelinePtr get_deferred_pipeline(uint modifier = 0)
	{
		auto it = pls_deferred.find(modifier);
		if (it != pls_deferred.end())
			return it->second;

		std::vector<std::string> defines;
		switch (modifier)
		{
		case "NO_SKY"_h:
			defines.push_back("frag:NO_SKY");
			break;
		}
		combine_global_defines(defines);
		std::sort(defines.begin(), defines.end());

		auto pl = graphics::GraphicsPipeline::get(L"flame\\shaders\\deferred.pipeline", defines);
		pls_deferred.emplace(modifier, pl);
		if (!pll_deferred)
			pll_deferred = pl->layout;
		return pl;
	}

	void MeshBatcher::collect(RenderMode render_mode, const DrawData& draw_data, graphics::CommandBufferPtr cb, uint mod2)
	{
		for (auto i = 0; i < draw_data.meshes.size(); i++)
		{
			auto& m = draw_data.meshes[i];
			auto& mesh_r = mesh_reses[m.mesh_id];
			auto& mat_r = mat_reses[m.mat_id];
			auto pl = get_material_pipeline(render_mode, mat_r, "mesh"_h, mesh_r.arm ? "ARMATURE"_h : 0, mod2);
			auto it = batches.find(pl);
			if (it == batches.end())
			{
				it = batches.emplace(pl, Batch()).first;
				it->second.armature = mesh_r.arm;
			}
			it->second.draw_indices.push_back(i);
		}

		for (auto& b : batches)
		{
			b.second.sub_cmd_offset = buf_idr.top;
			for (auto i : b.second.draw_indices)
			{
				auto& m = draw_data.meshes[i];
				auto& mesh_r = mesh_reses[m.mesh_id];
				buf_idr.add(mesh_r.idx_cnt, mesh_r.idx_off, mesh_r.vtx_off, 1, (m.mat_id << 16) + m.ins_id);
			}
			b.second.sub_cmd_count = buf_idr.top - b.second.sub_cmd_offset;
		}
		buf_idr.upload(cb);
	}

	void MeshBatcher::draw(graphics::CommandBufferPtr cb)
	{
		for (auto& b : batches)
		{
			if (b.second.sub_cmd_count == 0)
				continue;
			if (!b.second.armature)
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
			cb->draw_indexed_indirect(buf_idr.buf.get(), b.second.sub_cmd_offset, b.second.sub_cmd_count);
		}
	}

	void RenderTaskPrivate::init()
	{
		prm_plain.init(graphics::PipelineLayout::get(L"flame\\shaders\\plain\\plain.pll"), graphics::PipelineGraphics);

		auto dsl_camera = graphics::DescriptorSetLayout::get(L"flame\\shaders\\camera.dsl");
		buf_camera.create(graphics::BufferUsageUniform, dsl_camera->get_buf_ui("Camera"_h));
		ds_camera.reset(graphics::DescriptorSet::create(nullptr, dsl_camera));
		ds_camera->set_buffer("Camera"_h, 0, buf_camera.buf.get());
		ds_camera->update();

		auto dsl_target = graphics::DescriptorSetLayout::get(L"flame\\shaders\\target.dsl");
		ds_target.reset(graphics::DescriptorSet::create(nullptr, dsl_target));

		prm_fwd.init(pll_fwd, graphics::PipelineGraphics);
		prm_fwd.set_ds("camera"_h, ds_camera.get());
		prm_fwd.set_ds("instance"_h, ds_instance.get());
		prm_fwd.set_ds("material"_h, ds_material.get());
		prm_fwd.set_ds("lighting"_h, ds_lighting.get());
		prm_fwd.set_ds("target"_h, ds_target.get());

		if (mode != RenderModeSimple)
		{
			prm_gbuf.init(pll_gbuf, graphics::PipelineGraphics);
			prm_gbuf.set_ds("camera"_h, ds_camera.get());
			prm_gbuf.set_ds("instance"_h, ds_instance.get());
			prm_gbuf.set_ds("material"_h, ds_material.get());

			prm_deferred.init(pll_deferred, graphics::PipelineGraphics);
			auto dsl_deferred = pll_deferred->dsls.back();
			ds_deferred.reset(graphics::DescriptorSet::create(nullptr, dsl_deferred));
			prm_deferred.set_ds("camera"_h, ds_camera.get());
			prm_deferred.set_ds("material"_h, ds_material.get());
			prm_deferred.set_ds("lighting"_h, ds_lighting.get());
			prm_deferred.set_ds("target"_h, ds_target.get());
			prm_deferred.set_ds(""_h, ds_deferred.get());

			prm_outline_pp.init(pl_outline_pp->layout, graphics::PipelineGraphics);

			prm_box.init(pl_down_sample->layout, graphics::PipelineGraphics);
			prm_bloom.init(pl_bloom_bright->layout, graphics::PipelineGraphics);
			prm_blur.init(pl_blur_h->layout, graphics::PipelineGraphics);

			prm_dof.init(pl_dof->layout, graphics::PipelineGraphics);
			ds_dof.reset(graphics::DescriptorSet::create(nullptr, prm_dof.pll->dsls.back()));
			prm_dof.set_ds(""_h, ds_dof.get());

			prm_luma.init(pl_luma_hist->layout, graphics::PipelineCompute);
			auto dsl_luma = prm_luma.pll->dsls.back();
			if (!buf_luma.buf)
			{
				buf_luma.create(graphics::BufferUsageStorage | graphics::BufferUsageTransferSrc,
					dsl_luma->get_buf_ui("Luma"_h), graphics::BufferUsageTransferDst);
			}
			ds_luma.reset(graphics::DescriptorSet::create(nullptr, dsl_luma));
			ds_luma->set_buffer("Luma"_h, 0, buf_luma.buf.get());
			ds_luma->update();
			prm_luma.set_ds(""_h, ds_luma.get());

			prm_tone.init(pl_tone->layout, graphics::PipelineGraphics);
			prm_fxaa.init(pl_fxaa->layout, graphics::PipelineGraphics);
		}
	}

	void RenderTaskPrivate::set_targets(const std::vector<graphics::ImageViewPtr>& _targets)
	{
		targets = _targets;

		if (canvas)
		{
			canvas->set_targets(targets);
			canvas->clear_framebuffer = false;
		}

		if (targets.empty())
			return;

		graphics::Queue::get()->wait_idle();

		auto img0 = targets.front()->image;
		auto tar_ext = img0->extent;

		auto graphics_device = graphics::Device::current();
		static auto sp_nearest = graphics::Sampler::get(graphics::FilterNearest, graphics::FilterNearest, false, graphics::AddressClampToEdge);
		static auto sp_nearest_dep = graphics::Sampler::get(graphics::FilterNearest, graphics::FilterNearest, false, graphics::AddressClampToBorder);

		img_dst.reset(graphics::Image::create(col_fmt, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled | graphics::ImageUsageStorage));
		img_dst->filename = L"#img_dst";
		graphics_device->set_object_debug_name(img_dst.get(), "Dst");
		img_dep.reset(graphics::Image::create(dep_fmt, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 0));
		img_dep->filename = L"#img_dep";
		graphics_device->set_object_debug_name(img_dep.get(), "Dep");

		if (mode != RenderModeSimple)
		{
			img_back0.reset(graphics::Image::create(col_fmt, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 0));
			img_back0->filename = L"#img_back0";
			graphics_device->set_object_debug_name(img_back0.get(), "Back0");
			img_back1.reset(graphics::Image::create(col_fmt, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
			img_back1->filename = L"#img_back1";
			graphics_device->set_object_debug_name(img_back1.get(), "Back1");
		}

		img_dst_ms.reset(graphics::Image::create(col_fmt, tar_ext, graphics::ImageUsageAttachment, 1, 1, sample_count));
		img_dep_ms.reset(graphics::Image::create(dep_fmt, tar_ext, graphics::ImageUsageAttachment, 1, 1, sample_count));
		if (mode != RenderModeSimple)
		{
			img_last_dst.reset(graphics::Image::create(col_fmt, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled | graphics::ImageUsageStorage));
			img_last_dep.reset(graphics::Image::create(dep_fmt, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
			img_gbufferA.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
			img_gbufferA->filename = L"#img_gbufferA";
			graphics_device->set_object_debug_name(img_gbufferA.get(), "GBufferA");
			img_gbufferB.reset(graphics::Image::create(graphics::Format_A2R10G10B10_UNORM, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 0));
			img_gbufferB->filename = L"#img_gbufferB";
			graphics_device->set_object_debug_name(img_gbufferB.get(), "GBufferB");
			img_gbufferC.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
			img_gbufferC->filename = L"#img_gbufferC";
			graphics_device->set_object_debug_name(img_gbufferC.get(), "GBufferC");
			img_gbufferD.reset(graphics::Image::create(graphics::Format_B10G11R11_UFLOAT, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
			img_gbufferD->filename = L"#img_gbufferD";
			graphics_device->set_object_debug_name(img_gbufferD.get(), "GBufferD");
		}

		if (mode != RenderModeSimple)
			fb_fwd.reset(graphics::Framebuffer::create(rp_fwd, { img_dst_ms->get_view(), img_dep_ms->get_view(), img_dst->get_view(), img_dep->get_view() }));
		fb_fwd_clear.reset(graphics::Framebuffer::create(rp_fwd_clear, { img_dst_ms->get_view(), img_dep_ms->get_view(), img_dst->get_view(), img_dep->get_view() }));

		if (mode != RenderModeSimple)
			fb_gbuf.reset(graphics::Framebuffer::create(rp_gbuf, { img_gbufferA->get_view(), img_gbufferB->get_view(), img_gbufferC->get_view(), img_gbufferD->get_view(), img_dep->get_view() }));
		fb_primitive.reset(graphics::Framebuffer::create(rp_primitive, { img_dst->get_view(), img_dep->get_view() }));

		ds_target->set_image("img_dep"_h, 0, img_dep->get_view(), sp_nearest_dep);
		if (mode != RenderModeSimple)
		{
			ds_target->set_image("img_last_dst"_h, 0, img_last_dst->get_view(), sp_nearest);
			ds_target->set_image("img_last_dep"_h, 0, img_last_dep->get_view(), sp_nearest_dep);
		}
		else
		{
			ds_target->set_image("img_last_dst"_h, 0, img_black->get_view(), sp_nearest);
			ds_target->set_image("img_last_dep"_h, 0, img_dummy_depth->get_view(), sp_nearest_dep);
		}
		ds_target->update();

		if (mode != RenderModeSimple)
		{
			ds_deferred->set_image("img_gbufferA"_h, 0, img_gbufferA->get_view(), nullptr);
			ds_deferred->set_image("img_gbufferB"_h, 0, img_gbufferB->get_view(), nullptr);
			ds_deferred->set_image("img_gbufferC"_h, 0, img_gbufferC->get_view(), nullptr);
			ds_deferred->set_image("img_gbufferD"_h, 0, img_gbufferD->get_view(), nullptr);
			ds_deferred->update();

			dss_outline_pp.resize(img_dep->n_levels);
			for (auto i = 0; i < img_dep->n_levels; i++)
			{
				dss_outline_pp[i].reset(graphics::DescriptorSet::create(nullptr, prm_outline_pp.pll->dsls.back()));
				dss_outline_pp[i]->set_image("img_depth"_h, 0, img_dep->get_view({ (uint)i }), nullptr);
				dss_outline_pp[i]->set_image("img_normal"_h, 0, img_gbufferB->get_view({ (uint)i }), nullptr);
				dss_outline_pp[i]->update();
			}

			ds_dof->set_image("img_col"_h, 0, img_dst->get_view(), nullptr);
			ds_dof->set_image("img_dep"_h, 0, img_dep->get_view(), nullptr);
			ds_dof->update();

			ds_luma->set_image("img_col"_h, 0, img_dst->get_view(), nullptr);
			ds_luma->update();
		}

		if (img_pickup)
		{
			img_pickup.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageTransferSrc));
			img_dep_pickup.reset(graphics::Image::create(dep_fmt, tar_ext, graphics::ImageUsageAttachment | graphics::ImageUsageTransferSrc));
			fb_pickup.reset(graphics::Framebuffer::create(rp_col_dep, { img_pickup->get_view(), img_dep_pickup->get_view() }));
		}
	}

	vec2 RenderTaskPrivate::target_extent() const
	{
		if (targets.empty())
			return vec2(0.f);
		return (vec2)targets.front()->image->extent;
	}

	sRendererPrivate::sRendererPrivate() 
	{
	}

#include "marching_cubes_lookup.h"

	sRendererPrivate::sRendererPrivate(graphics::WindowPtr w)
	{
		window = w;

		auto graphics_device = graphics::Device::current();
		static auto sp_trilinear = graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, true, graphics::AddressClampToEdge);
		static auto sp_shadow = graphics::Sampler::create(graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressClampToBorder, std::numeric_limits<float>::max());

		graphics::InstanceCommandBuffer cb;

		img_black.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(4, 4, 1), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, 1, 8));
		graphics_device->set_object_debug_name(img_black.get(), "Black");
		img_white.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(4, 4, 1), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, 1, 8));
		graphics_device->set_object_debug_name(img_white.get(), "White");
		img_dummy_depth.reset(graphics::Image::create(graphics::Format_Depth16, uvec3(4, 4, 1), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled));
		img_cube_black.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(4, 4, 1), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, 1, -6, graphics::SampleCount_1));
		img_cube_white.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(4, 4, 1), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, 1, -6, graphics::SampleCount_1));
		img_black3D.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(4, 4, 4), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled));
		img_white3D.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(4, 4, 4), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled));
		img_black->clear(vec4(0.f), graphics::ImageLayoutShaderReadOnly);
		img_white->clear(vec4(1.f), graphics::ImageLayoutShaderReadOnly);
		img_dummy_depth->clear(vec4(1.f, 0.f, 0.f, 0.f), graphics::ImageLayoutShaderReadOnly);
		img_cube_black->clear(vec4(0.f), graphics::ImageLayoutShaderReadOnly);
		img_cube_white->clear(vec4(1.f), graphics::ImageLayoutShaderReadOnly);
		img_black3D->clear(vec4(0.f), graphics::ImageLayoutShaderReadOnly);
		img_white3D->clear(vec4(1.f), graphics::ImageLayoutShaderReadOnly);

		rp_fwd = graphics::Renderpass::get(L"flame\\shaders\\forward.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(col_fmt),
			  "dep_fmt=" + TypeInfo::serialize_t(dep_fmt),
			  "sample_count=" + TypeInfo::serialize_t(sample_count) });
		rp_fwd_clear = graphics::Renderpass::get(L"flame\\shaders\\forward.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(col_fmt),
			  "dep_fmt=" + TypeInfo::serialize_t(dep_fmt),
			  "sample_count=" + TypeInfo::serialize_t(sample_count),
			  "load_op=" + TypeInfo::serialize_t(graphics::AttachmentLoadClear) });
		rp_gbuf = graphics::Renderpass::get(L"flame\\shaders\\gbuffer.rp",
			{ "dep_fmt=" + TypeInfo::serialize_t(dep_fmt) });
		rp_dst = graphics::Renderpass::get(L"flame\\shaders\\color.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(col_fmt) });
		rp_dep = graphics::Renderpass::get(L"flame\\shaders\\depth.rp",
			{ "dep_fmt=" + TypeInfo::serialize_t(dep_fmt) });
		rp_col_dep = graphics::Renderpass::get(L"flame\\shaders\\color_depth.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(graphics::Format_R8G8B8A8_UNORM),
			  "dep_fmt=" + TypeInfo::serialize_t(dep_fmt) });
		rp_esm = graphics::Renderpass::get(L"flame\\shaders\\color_depth.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(esm_fmt),
			  "dep_fmt=" + TypeInfo::serialize_t(dep_fmt) });
		rp_primitive = graphics::Renderpass::get(L"flame\\shaders\\color_depth.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(col_fmt),
			  "dep_fmt=" + TypeInfo::serialize_t(dep_fmt),
			  "col_load_op=" + TypeInfo::serialize_t(graphics::AttachmentLoadLoad),
			  "dep_load_op=" + TypeInfo::serialize_t(graphics::AttachmentLoadLoad),
			  "col_initia_layout=" + TypeInfo::serialize_t(graphics::ImageLayoutAttachment), 
			  "dep_initia_layout=" + TypeInfo::serialize_t(graphics::ImageLayoutAttachment) });

		uint u;
		use_mesh_shader = graphics_device->get_config("mesh_shader"_h, u) ? u == 1 : true;

		auto dsl_instance = graphics::DescriptorSetLayout::get(L"flame\\shaders\\instance.dsl");
		buf_instance.create(graphics::BufferUsageStorage, dsl_instance->get_buf_ui("Instance"_h));
		mesh_instances.init(buf_instance.child_type<TI_A>("meshes"_h)->extent);
		register_mesh_instance(-1);
		armature_instances.init(buf_instance.child_type<TI_A>("armatures"_h)->extent);
		register_armature_instance(-1);
		terrain_instances.init(buf_instance.child_type<TI_A>("terrains"_h)->extent);
		sdf_instances.init(buf_instance.child_type<TI_A>("sdfs"_h)->extent);
		volume_instances.init(buf_instance.child_type<TI_A>("volumes"_h)->extent);
		buf_marching_cubes_loopup.create(graphics::BufferUsageStorage, dsl_instance->get_buf_ui("MarchingCubesLookup"_h));
		{
			auto items = buf_marching_cubes_loopup.mark_dirty_c(0);
			assert(sizeof(MarchingCubesLookup) == items.type->size);
			auto p = items.data;
			for (auto i = 0; i < 256; i++)
			{
				memcpy(p, &MarchingCubesLookup[i], sizeof(MarchingCubesLookupItem));
				p += sizeof(MarchingCubesLookupItem);
			}
			buf_marching_cubes_loopup.upload(cb.get());
		}
		buf_transform_feedback.create(graphics::BufferUsageStorage | graphics::BufferUsageTransferSrc, dsl_instance->get_buf_ui("TransformFeedback"_h), graphics::BufferUsageTransferDst);
		buf_transform_feedback.mark_dirty_c("vertex_count"_h).as<uint>() = 0;
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
		mat_vars.resize(buf_material.child_type<TI_A>("vars"_h)->extent);
		mat_reses.resize(buf_material.child_type<TI_A>("infos"_h)->extent);
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
		buf_lighting.child("sky_intensity"_h).as<float>() = sky_intensity;
		buf_lighting.child("fog_type"_h).as<int>() = (int)fog_type;
		buf_lighting.child("fog_density"_h).as<float>() = fog_density;
		buf_lighting.child("fog_start"_h).as<float>() = fog_start;
		buf_lighting.child("fog_end"_h).as<float>() = fog_end;
		buf_lighting.child("fog_base_height"_h).as<float>() = fog_base_height;
		buf_lighting.child("fog_max_height"_h).as<float>() = fog_max_height;
		buf_lighting.child("fog_color"_h).as<vec3>() = fog_color;
		buf_lighting.child("csm_levels"_h).as<uint>() = csm_levels;
		buf_lighting.child("esm_factor"_h).as<float>() = esm_factor;
		buf_lighting.child("shadow_bleeding_reduction"_h).as<float>() = shadow_bleeding_reduction;
		buf_lighting.child("shadow_darkening"_h).as<float>() = shadow_darkening;
		buf_lighting.child("ssr_enable"_h).as<int>() = ssr_enable ? 1U : 0U;
		buf_lighting.child("ssr_thickness"_h).as<float>() = ssr_thickness;
		buf_lighting.child("ssr_max_distance"_h).as<float>() = ssr_max_distance;
		buf_lighting.child("ssr_max_steps"_h).as<uint>() = ssr_max_steps;
		buf_lighting.child("ssr_binary_search_steps"_h).as<uint>() = ssr_binary_search_steps;
		buf_lighting.mark_dirty();
		buf_lighting.upload(cb.get());
		dir_lights.init(buf_lighting.child_type<TI_A>("dir_lights"_h)->extent);
		pt_lights.init(buf_lighting.child_type<TI_A>("pt_lights"_h)->extent);
		img_shadow_depth.reset(graphics::Image::create(dep_fmt, uvec3(ShadowMapSize, 1), graphics::ImageUsageAttachment));
		img_shadow_depth->filename = L"#img_shadow_depth";
		graphics_device->set_object_debug_name(img_shadow_depth.get(), "Shadow Depth");
		imgs_dir_shadow.resize(dsl_lighting->get_binding("dir_shadow_maps"_h).count);
		for (auto i = 0; i < imgs_dir_shadow.size(); i++)
		{
			auto img = graphics::Image::create(esm_fmt, uvec3(ShadowMapSize, 1), graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 1, DirShadowMaxLevels);
			imgs_dir_shadow[i].reset(img);
			img->filename = L"#img_dir_shadow" + wstr(i);
			graphics_device->set_object_debug_name(img, "Directional Shadow " + str(i));
			img->change_layout(graphics::ImageLayoutShaderReadOnly);
		}
		for (auto i = 0; i < imgs_dir_shadow.size(); i++)
		{
			auto img = imgs_dir_shadow[i].get();
			for (auto ly = 0; ly < img->n_layers; ly++)
				fbs_dir_shadow.emplace_back(graphics::Framebuffer::create(rp_esm, { img->get_view({ 0, 1, (uint)ly }), img_shadow_depth->get_view() }));
		}
		img_dir_shadow_back.reset(graphics::Image::create(esm_fmt, uvec3(ShadowMapSize, 1), graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 1, DirShadowMaxLevels));
		imgs_pt_shadow.resize(dsl_lighting->get_binding("pt_shadow_maps"_h).count);
		for (auto i = 0; i < imgs_pt_shadow.size(); i++)
		{
			auto img = graphics::Image::create(esm_fmt, uvec3(ShadowMapSize / 2U, 1), graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 1, -6, graphics::SampleCount_1);
			imgs_pt_shadow[i].reset(img);
			img->filename = L"#img_pt_shadow" + wstr(i);
			graphics_device->set_object_debug_name(img, "Point Shadow " + str(i));
			img->change_layout(graphics::ImageLayoutShaderReadOnly);
		}
		for (auto i = 0; i < imgs_pt_shadow.size(); i++)
		{
			auto img = imgs_pt_shadow[i].get();
			for (auto ly = 0; ly < img->n_layers; ly++)
				fbs_pt_shadow.emplace_back(graphics::Framebuffer::create(rp_esm, { img->get_view({ 0, 1, (uint)ly }), img_shadow_depth->get_view() }));
		}
		img_pt_shadow_back.reset(graphics::Image::create(esm_fmt, uvec3(ShadowMapSize / 2U, 1), graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 1, -6, graphics::SampleCount_1));
		ds_lighting->set_buffer("Lighting"_h, 0, buf_lighting.buf.get());
		for (auto i = 0; i < imgs_dir_shadow.size(); i++)
			ds_lighting->set_image("dir_shadow_maps"_h, i, imgs_dir_shadow[i]->get_view({ 0, 1, 0, DirShadowMaxLevels }), sp_shadow);
		for (auto i = 0; i < imgs_pt_shadow.size(); i++)
			ds_lighting->set_image("pt_shadow_maps"_h, i, imgs_pt_shadow[i]->get_view({ 0, 1, 0, 6 }, {}, true), sp_shadow);
		ds_lighting->set_image("sky_map"_h, 0, img_cube_black->get_view({ 0, 1, 0, 6 }, {}, true), nullptr);
		ds_lighting->set_image("sky_irr_map"_h, 0, img_cube_black->get_view({ 0, 1, 0, 6 }, {}, true), nullptr);
		ds_lighting->set_image("sky_rad_map"_h, 0, img_cube_black->get_view({ 0, 1, 0, 6 }, {}, true), nullptr);
		{
			auto img = graphics::Image::get(L"flame\\brdf.dds");
			ds_lighting->set_image("brdf_map"_h, 0, img ? img->get_view() : img_black->get_view(), nullptr);
		}
		ds_lighting->update();

		buf_vtx.create(L"flame\\shaders\\mesh\\mesh.vi", {}, 1024 * 256 * 4);
		buf_idx.create(1024 * 256 * 6);
		buf_vtx_arm.create(L"flame\\shaders\\mesh\\mesh.vi", { "ARMATURE" }, 1024 * 128 * 4);
		buf_idx_arm.create(1024 * 128 * 6);
		buf_particles.create(L"flame\\shaders\\particle.vi", {}, 1024 * 128);
		buf_primitives.create(L"flame\\shaders\\plain\\plain3d.vi", {}, 1024 * 256);

		mesh_reses.resize(1024 * 8);

		pl_point3d = graphics::GraphicsPipeline::get(L"flame\\shaders\\plain\\point3d.pipeline", { "dc=true", "dt=false", "dw=false" });
		pl_point3d_dt = graphics::GraphicsPipeline::get(L"flame\\shaders\\plain\\point3d.pipeline", { "dt=true", "dw=false" });
		pl_line3d = graphics::GraphicsPipeline::get(L"flame\\shaders\\plain\\line3d.pipeline", { "dc=true", "dt=false", "dw=false" });
		pl_line3d_dt = graphics::GraphicsPipeline::get(L"flame\\shaders\\plain\\line3d.pipeline", { "dt=true", "dw=false" });
		pl_line_strip3d = graphics::GraphicsPipeline::get(L"flame\\shaders\\plain\\line3d.pipeline", { "dc=true", "pt=LineStrip", "dt=false", "dw=false" });
		pl_line_strip3d_dt = graphics::GraphicsPipeline::get(L"flame\\shaders\\plain\\line3d.pipeline", { "pt=LineStrip", "dt=true", "dw=false" });
		pl_triangle3d = graphics::GraphicsPipeline::get(L"flame\\shaders\\plain\\triangle3d.pipeline", { "dc=true", "dt=false", "dw=false" });
		pl_triangle3d_dt = graphics::GraphicsPipeline::get(L"flame\\shaders\\plain\\triangle3d.pipeline", { "dt=true", "dw=false" });

		pll_fwd = graphics::PipelineLayout::get(L"flame\\shaders\\forward.pll");
		pll_gbuf = graphics::PipelineLayout::get(L"flame\\shaders\\gbuffer.pll");

		pl_blit = graphics::GraphicsPipeline::get(L"flame\\shaders\\blit.pipeline", {});
		pl_blit_blend = graphics::GraphicsPipeline::get(L"flame\\shaders\\blit.pipeline", { "be=true", "sbc=SrcAlpha", "dbc=OneMinusSrcAlpha" });
		pl_blit_dep = graphics::GraphicsPipeline::get(L"flame\\shaders\\blit.pipeline", { "rp=" + str(rp_dep), "frag:DEPTH" });
		pl_add = graphics::GraphicsPipeline::get(L"flame\\shaders\\add.pipeline", {});
		pl_blend = graphics::GraphicsPipeline::get(L"flame\\shaders\\blend.pipeline", {});

		pl_mesh_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline", { "rp=" + str(rp_col_dep) });
		pl_mesh_arm_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline", { "rp=" + str(rp_col_dep), "vert:ARMATURE" });
		pl_terrain_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\terrain\\terrain.pipeline", { "rp=" + str(rp_col_dep) });

		if (use_mesh_shader)
			pl_MC_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\volume\\marching_cubes.pipeline", { "rp=" + str(rp_col_dep) });

		gbuffer_batcher.buf_idr.create(mesh_instances.capacity);
		transparent_batcher.buf_idr.create(mesh_instances.capacity);
		for (auto& s : dir_shadows)
		{
			for (auto i = 0; i < DirShadowMaxLevels; i++)
				s.batcher[i].buf_idr.create(min(8192U, mesh_instances.capacity));
		}

		get_deferred_pipeline();

		pl_down_sample = graphics::GraphicsPipeline::get(L"flame\\shaders\\box.pipeline", {});
		pl_up_sample = graphics::GraphicsPipeline::get(L"flame\\shaders\\box.pipeline", { "be=true", "sbc=One", "dbc=One" });
		pl_down_sample_depth = graphics::GraphicsPipeline::get(L"flame\\shaders\\box.pipeline", { "rp=" + str(rp_dep), "frag:DEPTH" });
		pl_up_sample_depth = graphics::GraphicsPipeline::get(L"flame\\shaders\\box.pipeline", { "rp=" + str(rp_dep), "be=true", "sbc=One", "dbc=One", "frag:DEPTH" });

		pl_outline_pp = graphics::GraphicsPipeline::get(L"flame\\shaders\\outline_pp.pipeline", { "rp=" + str(rp_dst) });

		pl_bloom_bright = graphics::GraphicsPipeline::get(L"flame\\shaders\\bloom.pipeline", { "frag:BRIGHT_PASS" });

		pl_blur_h = graphics::GraphicsPipeline::get(L"flame\\shaders\\blur.pipeline", { "frag:HORIZONTAL" });
		pl_blur_v = graphics::GraphicsPipeline::get(L"flame\\shaders\\blur.pipeline", { "frag:VERTICAL" });
		pl_blur_depth_h = graphics::GraphicsPipeline::get(L"flame\\shaders\\blur.pipeline", { "rp=" + str(rp_dep), "frag:DEPTH", "frag:HORIZONTAL" });
		pl_blur_depth_v = graphics::GraphicsPipeline::get(L"flame\\shaders\\blur.pipeline", { "rp=" + str(rp_dep), "frag:DEPTH", "frag:VERTICAL" });
		pl_blur_max_h = graphics::GraphicsPipeline::get(L"flame\\shaders\\blur.pipeline", { "frag:MAX", "frag:HORIZONTAL" });
		pl_blur_max_v = graphics::GraphicsPipeline::get(L"flame\\shaders\\blur.pipeline", { "frag:MAX", "frag:VERTICAL" });

		pl_luma_hist = graphics::ComputePipeline::get(L"flame\\shaders\\luma.pipeline", { "comp:HISTOGRAM_PASS" });
		pl_luma_avg = graphics::ComputePipeline::get(L"flame\\shaders\\luma.pipeline", { "comp:AVERAGE_PASS" });

		pl_dof = graphics::GraphicsPipeline::get(L"flame\\shaders\\dof.pipeline", { "rp=" + str(rp_dst) });
		pl_tone = graphics::GraphicsPipeline::get(L"flame\\shaders\\tone.pipeline", {});
		pl_fxaa = graphics::GraphicsPipeline::get(L"flame\\shaders\\fxaa.pipeline", {});

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

		camera_light_id = register_light_instance(LightDirectional, -1);
		white_tex_id = get_texture_res(img_white->get_view(), nullptr, -1);
		black_tex_id = get_texture_res(img_black->get_view(), nullptr, -1);
		if (auto img = graphics::Image::get(L"flame\\random.png"); img)
		{
			rand_tex_id = get_texture_res(img->get_view(),
				graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressRepeat), -1);
		}

		cb.excute();
		
		w->renderers.add([this](int img_idx, graphics::CommandBufferPtr cb) {
			render(img_idx, cb);
		}, 0, 0);

		hud_style_vars.resize(HudStyleVarCount);
		hud_style_vars[HudStyleVarScaling].push(vec2(1.f));
	}

	sRendererPrivate::~sRendererPrivate()
	{
	}

	RenderTaskPtr sRendererPrivate::add_render_task(RenderMode mode, cCameraPtr camera,
		const std::vector<graphics::ImageViewPtr>& targets, graphics::ImageLayout final_layout, bool need_canvas, bool need_pickup)
	{
		auto ret = new RenderTaskPrivate;
		ret->mode = mode;
		ret->camera = camera;
		ret->final_layout = final_layout;
		if (need_canvas)
			ret->canvas = graphics::Canvas::create(window);
		if (need_pickup)
		{
			ret->img_pickup.reset(graphics::Image::create(graphics::Format_R8_UNORM, uvec3(4, 4, 1), graphics::ImageUsageTransferSrc));
			ret->img_dep_pickup.reset(graphics::Image::create(graphics::Format_R8_UNORM, uvec3(4, 4, 1), graphics::ImageUsageTransferSrc));
			ret->fence_pickup.reset(graphics::Fence::create(false));
		}
		ret->init();
		ret->set_targets(targets);
		render_tasks.emplace_back(ret);
		return ret;
	}

	RenderTaskPtr sRendererPrivate::add_render_task_with_window_targets(RenderMode mode, cCameraPtr camera, bool need_canvas, bool need_pickup)
	{
		auto ret = add_render_task(mode, camera, {}, graphics::ImageLayoutAttachment, need_canvas, need_pickup);
		window->native->resize_listeners.add([this, ret](const uvec2& sz) {
			graphics::Queue::get()->wait_idle();
			std::vector<graphics::ImageViewPtr> targets;
			for (auto& i : window->swapchain->images)
				targets.push_back(i->get_view());
			ret->set_targets(targets);
		});
		std::vector<graphics::ImageViewPtr> targets;
		for (auto& i : window->swapchain->images)
			targets.push_back(i->get_view());
		ret->set_targets(targets);
		return ret;
	}

	void sRendererPrivate::remove_render_task(RenderTaskPtr task)
	{
		for (auto it = render_tasks.begin(); it != render_tasks.end(); it++)
		{
			if (it->get() == task)
			{
				render_tasks.erase(it);
				return;
			}
		}
	}

	void sRendererPrivate::set_sky_maps(graphics::ImageViewPtr _sky_map, graphics::ImageViewPtr _sky_irr_map, graphics::ImageViewPtr _sky_rad_map)
	{
		if (sky_map == _sky_map && sky_irr_map == _sky_irr_map && sky_rad_map == _sky_rad_map)
			return;

		sky_map = _sky_map;
		sky_irr_map = _sky_irr_map;
		sky_rad_map = _sky_rad_map;
		graphics::Queue::get()->wait_idle();
		ds_lighting->set_image("sky_map"_h, 0, sky_map ? sky_map : img_cube_black->get_view({ 0, 1, 0, 6 }, {}, true), nullptr);
		ds_lighting->set_image("sky_irr_map"_h, 0, sky_irr_map ? sky_irr_map : img_cube_black->get_view({ 0, 1, 0, 6 }, {}, true), nullptr);
		ds_lighting->set_image("sky_rad_map"_h, 0, sky_rad_map ? sky_rad_map : img_cube_black->get_view({ 0, 1, 0, 6 }, {}, true), nullptr);
		ds_lighting->update();

		sky_rad_levels = sky_rad_map ? sky_rad_map->sub.level_count : 1.f;
		buf_lighting.mark_dirty_c("sky_rad_levels"_h).as<float>() = sky_rad_levels;

		dirty = true;
	}

	void sRendererPrivate::set_sky_intensity(float v)
	{
		if (sky_intensity == v)
			return;
		sky_intensity = v;
		buf_lighting.mark_dirty_c("sky_intensity"_h).as<float>() = sky_intensity;

		dirty = true;
	}

	uint sRendererPrivate::get_cel_shading_levels() const
	{
		return cel_shading_levels;
	}

	void sRendererPrivate::set_cel_shading_levels(uint v)
	{
		if (cel_shading_levels == v)
			return;
		if (cel_shading_levels == 0 || v == 0)
			mark_clear_pipelines = true;
		cel_shading_levels = v;
		buf_lighting.mark_dirty_c("cel_shading_levels"_h).as<uint>() = cel_shading_levels;

		dirty = true;
	}

	void sRendererPrivate::set_fog_type(FogType type)
	{
		if (fog_type == type)
			return;
		fog_type = type;
		buf_lighting.mark_dirty_c("fog_type"_h).as<int>() = (int)fog_type;

		dirty = true;
	
	}

	void sRendererPrivate::set_fog_density(float v)
	{
		if (fog_density == v)
			return;
		fog_density = v;
		buf_lighting.mark_dirty_c("fog_density"_h).as<float>() = fog_density;

		dirty = true;
	}

	void sRendererPrivate::set_fog_start(float v)
	{
		if (fog_start == v)
			return;
		fog_start = v;
		buf_lighting.mark_dirty_c("fog_start"_h).as<float>() = fog_start;

		dirty = true;
	}

	void sRendererPrivate::set_fog_end(float v)
	{
		if (fog_end == v)
			return;
		fog_end = v;
		buf_lighting.mark_dirty_c("fog_end"_h).as<float>() = fog_end;

		dirty = true;
	}

	void sRendererPrivate::set_fog_base_height(float v)
	{
		if (fog_base_height == v)
			return;
		fog_base_height = v;
		buf_lighting.mark_dirty_c("fog_base_height"_h).as<float>() = fog_base_height;

		dirty = true;
	}

	void sRendererPrivate::set_fog_max_height(float v)
	{
		if (fog_max_height == v)
			return;
		fog_max_height = v;
		buf_lighting.mark_dirty_c("fog_max_height"_h).as<float>() = fog_max_height;

		dirty = true;
	}

	void sRendererPrivate::set_fog_color(const vec3& color)
	{
		if (fog_color == color)
			return;
		fog_color = color;
		buf_lighting.mark_dirty_c("fog_color"_h).as<vec3>() = fog_color;

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
		csm_levels = min(lv, 4U);
		buf_lighting.mark_dirty_c("csm_levels"_h).as<uint>() = csm_levels;

		dirty = true;
	}

	void sRendererPrivate::set_esm_factor(float f)
	{
		if (esm_factor == f)
			return;
		esm_factor = f;
		buf_lighting.mark_dirty_c("esm_factor"_h).as<float>() = esm_factor;

		dirty = true;
	}

	void sRendererPrivate::set_shadow_bleeding_reduction(float f)
	{
		if (shadow_bleeding_reduction == f)
			return;
		shadow_bleeding_reduction = f;
		buf_lighting.mark_dirty_c("shadow_bleeding_reduction"_h).as<float>() = shadow_bleeding_reduction;

		dirty = true;
	}

	void sRendererPrivate::set_shadow_darkening(float f)
	{
		if (shadow_darkening == f)
			return;
		shadow_darkening = f;
		buf_lighting.mark_dirty_c("shadow_darkening"_h).as<float>() = shadow_darkening;

		dirty = true;
	}

	void sRendererPrivate::set_post_processing_enable(bool v)
	{
		if (post_processing_enable == v)
			return;
		post_processing_enable = v;

		dirty = true;
	}

	void sRendererPrivate::set_outline_pp_enable(bool v)
	{
		if (outline_pp_enable == v)
			return;
		outline_pp_enable = v;

		dirty = true;
	}

	void sRendererPrivate::set_outline_pp_width(uint v)
	{
		if (outline_pp_width == v)
			return;
		outline_pp_width = v;

		dirty = true;
	}

	void sRendererPrivate::set_outline_pp_depth_scale(float v)
	{
		if (outline_pp_depth_scale == v)
			return;
		outline_pp_depth_scale = v;

		dirty = true;
	}

	void sRendererPrivate::set_outline_pp_normal_scale(float v)
	{
		if (outline_pp_normal_scale == v)
			return;
		outline_pp_normal_scale = v;

		dirty = true;
	}

	void sRendererPrivate::set_outline_pp_color(const vec3& col)
	{
		if (outline_pp_color == col)
			return;
		outline_pp_color = col;

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

	void sRendererPrivate::set_dof_enable(bool v)
	{
		if (dof_enable == v)
			return;
		dof_enable = v;

		dirty = true;
	}

	void sRendererPrivate::set_dof_focus_point(float v)
	{
		if (dof_focus_point == v)
			return;
		dof_focus_point = v;

		dirty = true;
	}

	void sRendererPrivate::set_dof_focus_scale(float v)
	{
		if (dof_focus_scale == v)
			return;
		dof_focus_scale = v;

		dirty = true;
	}

	void sRendererPrivate::set_ssr_enable(bool v)
	{
		if (ssr_enable == v)
			return;
		ssr_enable = v;
		buf_lighting.mark_dirty_c("ssr_enable"_h).as<int>() = v ? 1U : 0U;

		dirty = true;
	}

	void sRendererPrivate::set_ssr_thickness(float v)
	{
		if (ssr_thickness == v)
			return;
		ssr_thickness = v;
		buf_lighting.mark_dirty_c("ssr_thickness"_h).as<float>() = v;

		dirty = true;
	}

	void sRendererPrivate::set_ssr_max_distance(float v)
	{
		if (ssr_max_distance == v)
			return;
		ssr_max_distance = v;
		buf_lighting.mark_dirty_c("ssr_max_distance"_h).as<float>() = v;
		 
		dirty = true;
	}

	void sRendererPrivate::set_ssr_max_steps(uint v)
	{
		if (ssr_max_steps == v)
			return;
		ssr_max_steps = v;
		buf_lighting.mark_dirty_c("ssr_max_steps"_h).as<uint>() = v;

		dirty = true;
	}

	void sRendererPrivate::set_ssr_binary_search_steps(uint v)
	{
		if (ssr_binary_search_steps == v)
			return;
		ssr_binary_search_steps = v;
		buf_lighting.mark_dirty_c("ssr_binary_search_steps"_h).as<uint>() = v;

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
		buf_material.mark_dirty_ci("vars"_h, id).as<vec4>() = v;
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

		graphics::Queue::get()->wait_idle();
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
			res.vtx_off = buf_vtx.add(nullptr, res.vtx_cnt);
			for (auto i = 0; i < res.vtx_cnt; i++)
			{
				auto vtx = buf_vtx.item(res.vtx_off + i);
				vtx.child("i_pos"_h).as<vec3>() = mesh->positions[i];
				if (!mesh->uvs.empty())
					vtx.child("i_uv"_h).as<vec2>() = mesh->uvs[i];
				if (!mesh->normals.empty())
					vtx.child("i_nor"_h).as<vec3>() = mesh->normals[i];
				if (!mesh->tangents.empty())
					vtx.child("i_tan"_h).as<vec3>() = mesh->tangents[i];
			}

			res.idx_off = buf_idx.add(mesh->indices.data(), res.idx_cnt);
		}
		else
		{
			res.vtx_off = buf_vtx_arm.add(nullptr, res.vtx_cnt);
			for (auto i = 0; i < res.vtx_cnt; i++)
			{
				auto vtx = buf_vtx_arm.item(res.vtx_off + i);
				vtx.child("i_pos"_h).as<vec3>() = mesh->positions[i];
				if (!mesh->uvs.empty())
					vtx.child("i_uv"_h).as<vec2>() = mesh->uvs[i];
				if (!mesh->normals.empty())
					vtx.child("i_nor"_h).as<vec3>() = mesh->normals[i];
				if (!mesh->tangents.empty())
					vtx.child("i_tan"_h).as<vec3>() = mesh->tangents[i];
				if (!mesh->bone_ids.empty())
					vtx.child("i_bids"_h).as<ivec4>() = mesh->bone_ids[i];
				if (!mesh->bone_weights.empty())
					vtx.child("i_bwgts"_h).as<vec4>() = mesh->bone_weights[i];
			}

			res.idx_off = buf_idx_arm.add(mesh->indices.data(), res.idx_cnt);
		}

		return id;
	}

	void sRendererPrivate::release_mesh_res(uint id)
	{
		auto& res = mesh_reses[id];
		if (res.ref == 1)
		{
			if (!res.arm)
			{
				buf_vtx.release(res.vtx_off, res.vtx_cnt);
				buf_idx.release(res.idx_off, res.idx_cnt);
			}
			else
			{
				buf_vtx_arm.release(res.vtx_off, res.vtx_cnt);
				buf_idx_arm.release(res.idx_off, res.idx_cnt);
			}

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

	void sRendererPrivate::update_mat_res(uint id, bool update_parameters, bool update_textures, bool update_pipeline)
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
				auto& tex = res.mat->textures[i];
				if (!tex.filename.empty())
				{
					if (auto image = graphics::Image::get(tex.filename); image)
					{
						res.texs[i].second = image;
						res.texs[i].first = get_texture_res(image->get_view({ 0, image->n_levels, 0, image->n_layers }),
							graphics::Sampler::get(tex.mag_filter, tex.min_filter, tex.linear_mipmap, tex.address_mode), -1);
						image->dependencies.emplace_back("flame::Graphics::Material"_h, res.mat);
					}
				}
			}
		}
		if (update_pipeline)
		{
			res.defines = res.mat->defines;
			res.float_values.clear();
			res.int_values.clear();
			auto add_float = [&](const std::string& name, float v) {
				res.defines.push_back("frag:" + name + "_V=" + str((int)res.float_values.size()));
				res.float_values.push_back(v);
			};
			auto add_int = [&](const std::string& name, int v) {
				res.defines.push_back("frag:" + name + "_V=" + str((int)res.int_values.size()));
				res.int_values.push_back(v);
			};

			if (res.mat->color_map != -1)
				res.defines.push_back("frag:COLOR_MAP=" + str(res.mat->color_map));
			if (res.mat->normal_map != -1)
				res.mat->defines.push_back("frag:NORMAL_MAP=" + str(res.mat->normal_map));
			if (res.mat->metallic_map != -1)
				res.mat->defines.push_back("frag:METALLIC_MAP=" + str(res.mat->metallic_map));
			if (res.mat->roughness_map != -1)
				res.mat->defines.push_back("frag:ROUGHNESS_MAP=" + str(res.mat->roughness_map));
			if (res.mat->emissive_map != -1)
				res.mat->defines.push_back("frag:EMISSIVE_MAP=" + str(res.mat->emissive_map));
			if (res.mat->alpha_map != -1)
				res.mat->defines.push_back("frag:ALPHA_MAP=" + str(res.mat->alpha_map));
			if (res.mat->splash_map != -1)
				res.mat->defines.push_back("frag:SPLASH_MAP=" + str(res.mat->splash_map));
			if (auto alpha_map = res.mat->alpha_map != -1 ? res.mat->alpha_map : res.mat->color_map; alpha_map != -1)
			{
				auto& tex = res.mat->textures[alpha_map];
				float alpha_test = 0.f;
				{
					auto s = tex.filename.filename().stem().string();
					for (auto t : SUS::to_string_vector(SUS::split(s, '%')))
					{
						if (SUS::strip_head_if(t, "at"))
							alpha_test = s2t<int>(t) / 10.f;
					}
				}
				if (alpha_test > 0.f)
					add_float("ALPHA_TEST", alpha_test);
			}

			for (auto& v : res.mat->float_variables)
				add_float(v.first, v.second);
			for (auto& v : res.mat->int_variables)
				add_int(v.first, v.second);

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
			for (auto it = gbuffer_batcher.batches.begin(); it != gbuffer_batcher.batches.end();)
			{
				if (has_pl(it->first))
					it = gbuffer_batcher.batches.erase(it);
				else
					it++;
			}
			for (auto it = transparent_batcher.batches.begin(); it != transparent_batcher.batches.end();)
			{
				if (has_pl(it->first))
					it = transparent_batcher.batches.erase(it);
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
			auto info = buf_material.mark_dirty_ci("infos"_h, id);
			info.child("color"_h).as<vec4>() = res.mat->color;
			info.child("metallic"_h).as<float>() = res.mat->metallic;
			info.child("roughness"_h).as<float>() = res.mat->roughness;
			info.child("emissive"_h).as<vec4>() = res.mat->emissive;
			info.child("tiling"_h).as<float>() = res.mat->tiling;
			info.child("normal_map_strength"_h).as<float>() = res.mat->normal_map_strength;
			info.child("emissive_map_strength"_h).as<float>() = res.mat->emissive_map_strength;
			info.child("flags"_h).as<int>() = res.mat->get_flags();
			memcpy(info.child("f"_h).data, res.float_values.data(), sizeof(float) * res.float_values.size());
			memcpy(info.child("i"_h).data, res.int_values.data(), sizeof(int) * res.int_values.size());
			auto ids = (int*)info.child("map_indices"_h).data;
			for (auto i = 0; i < res.texs.size(); i++)
				ids[i] = res.texs[i].first;
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
			case "defines"_h:
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
		auto ins = buf_lighting.mark_dirty_ci("dir_lights"_h, id);
		ins.child("dir"_h).as<vec3>() = dir;
		ins.child("color"_h).as<vec3>() = color;
	}

	void sRendererPrivate::set_pt_light_instance(uint id, const vec3& pos, const vec3& color, float range)
	{
		auto ins = buf_lighting.mark_dirty_ci("pt_lights"_h, id);
		ins.child("pos"_h).as<vec3>() = pos;
		ins.child("color"_h).as<vec3>() = color;
	}

	int sRendererPrivate::register_mesh_instance(int id)
	{
		if (id == -1)
		{
			id = mesh_instances.get_free_item();
			if (id != -1)
				set_mesh_instance(id, mat4(1.f), mat3(1.f), cvec4(255));
		}
		else
			mesh_instances.release_item(id);
		return id;
	}

	void sRendererPrivate::set_mesh_instance(uint id, const mat4& mat, const mat3& nor, const cvec4& col)
	{
		auto ins = buf_instance.mark_dirty_ci("meshes"_h, id);
		ins.child("mat"_h).as<mat4>() = mat;
		ins.child("nor"_h).as<mat3x4>() = nor;
		ins.child("col"_h).as<cvec4>() = col;
	}

	int sRendererPrivate::register_armature_instance(int id)
	{
		if (id == -1)
		{
			id = armature_instances.get_free_item();
			if (id != -1)
			{
				std::vector<mat4> mats(buf_instance.child_type<TI_A>("armatures"_h)->extent);
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
		auto ins = buf_instance.mark_dirty_ci("armatures"_h, id);
		memcpy(ins.data, mats, size * sizeof(mat4));
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
			graphics::Queue::get()->wait_idle();
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
		auto ins = buf_instance.mark_dirty_ci("terrains"_h, id);
		ins.child("mat"_h).as<mat4>() = mat;
		ins.child("extent"_h).as<vec3>() = extent;
		ins.child("blocks"_h).as<uvec2>() = blocks;
		ins.child("tess_level"_h).as<uint>() = tess_level;
		ins.child("grass_field_tess_level"_h).as<uint>() = grass_field_tess_level;
		ins.child("grass_channel"_h).as<uint>() = grass_channel;
		ins.child("grass_texture_id"_h).as<int>() = grass_texture_id;

		graphics::Queue::get()->wait_idle();
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

	void sRendererPrivate::set_sdf_instance(uint id, uint _boxes_count, std::pair<vec3, vec3>* _boxes, uint _spheres_count, std::pair<vec3, float>* _spheres)
	{
		auto ins = buf_instance.mark_dirty_ci("sdfs"_h, id);
		ins.child("boxes_count"_h).as<uint>() = _boxes_count;
		auto boxes = ins.child("boxes"_h);
		for (auto i = 0; i < _boxes_count; i++)
		{
			auto box = boxes.item(i);
			box.child("coord"_h).as<vec3>() = _boxes[i].first;
			box.child("extent"_h).as<vec3>() = _boxes[i].second;
		}
		ins.child("spheres_count"_h).as<uint>() = _spheres_count;
		auto spheres = ins.child("spheres"_h);
		for (auto i = 0; i < _spheres_count; i++)
		{
			auto sphere = spheres.item(i);
			sphere.child("coord"_h).as<vec3>() = _spheres[i].first;
			sphere.child("radius"_h).as<float>() = _spheres[i].second;
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
			graphics::Queue::get()->wait_idle();
			ds_instance->set_image("volume_data_maps"_h, id, img_black3D->get_view(), nullptr);
			ds_instance->update();
		}
		return id;
	}

	void sRendererPrivate::set_volume_instance(uint id, const mat4& mat, const vec3& extent, const uvec3& blocks, graphics::ImageViewPtr data_map)
	{
		auto ins = buf_instance.mark_dirty_ci("volumes"_h, id);
		ins.child("mat"_h).as<mat4>() = mat;
		ins.child("extent"_h).as<vec3>() = extent;
		ins.child("blocks"_h).as<uvec3>() = blocks;

		graphics::Queue::get()->wait_idle();
		ds_instance->set_image("volume_data_maps"_h, id, data_map, graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressClampToEdge, graphics::BorderColorBlack));
		ds_instance->update();
	}

	void sRendererPrivate::draw_outlines(const std::vector<ObjectDrawData>& draw_datas, const cvec4& color, uint width, OutlineMode mode)
	{
		auto& od = outline_groups.emplace_back();
		od.draws = draw_datas;
		od.color = color;
		od.width = width;
		od.mode = mode;
	}

	void sRendererPrivate::draw_primitives(PrimitiveType type, const vec3* points, uint count, const cvec4& color, bool depth_test)
	{
		auto& pd = primitives_draws.emplace_back();
		pd.type = type;
		pd.color = color;
		pd.depth_test = depth_test;
		if (type == PrimitiveQuadList)
		{
			pd.type = PrimitiveTriangleList;
			auto quad_num = count / 4;
			pd.vtx_cnt = quad_num * 6;
			auto off = buf_primitives.add(nullptr, pd.vtx_cnt);
			for (auto i = 0; i < quad_num; i++)
			{
				buf_primitives.item(off + i * 6 + 0).child("i_pos"_h).as<vec3>() = points[i * 4 + 0];
				buf_primitives.item(off + i * 6 + 1).child("i_pos"_h).as<vec3>() = points[i * 4 + 1];
				buf_primitives.item(off + i * 6 + 2).child("i_pos"_h).as<vec3>() = points[i * 4 + 2];
				buf_primitives.item(off + i * 6 + 3).child("i_pos"_h).as<vec3>() = points[i * 4 + 0];
				buf_primitives.item(off + i * 6 + 4).child("i_pos"_h).as<vec3>() = points[i * 4 + 2];
				buf_primitives.item(off + i * 6 + 5).child("i_pos"_h).as<vec3>() = points[i * 4 + 3];
			}
		}
		else
		{
			pd.vtx_cnt = count;
			auto off = buf_primitives.add(nullptr, pd.vtx_cnt);
			for (auto i = 0; i < pd.vtx_cnt; i++)
				buf_primitives.item(off + i).child("i_pos"_h).as<vec3>() = points[i];
		}
	}

	void sRendererPrivate::draw_particles(uint mat_id, const std::vector<ParticleDrawData::Ptc>& ptcs)
	{
		auto off = buf_particles.add(nullptr, ptcs.size());
		for (auto i = 0; i < ptcs.size(); i++)
		{
			auto& src = ptcs[i];
			auto dst = buf_particles.item(off + i);
			dst.child("i_pos0"_h).as<vec3>() = src.pos0;
			dst.child("i_pos1"_h).as<vec3>() = src.pos1;
			dst.child("i_pos2"_h).as<vec3>() = src.pos2;
			dst.child("i_pos3"_h).as<vec3>() = src.pos3;
			dst.child("i_uv"_h).as<vec4>() = src.uv;
			dst.child("i_col"_h).as<cvec4>() = src.col;
			dst.child("i_time"_h).as<float>() = src.time;
		}
		auto& ptd = particles_draws.emplace_back();
		ptd.mat_id = mat_id;
		ptd.vtx_off = off;
		ptd.vtx_cnt = ptcs.size();
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

	static void render_element(graphics::CanvasPtr canvas, EntityPtr e)
	{
		if (!e->global_enable)
			return;

		if (auto element = e->get_component<cElement>(); element)
		{
			if (element->scissor)
				canvas->push_scissor(Rect(element->global_pos0(), element->global_pos1()));

			element->drawers.call(canvas);
			for (auto& c : e->children)
				render_element(canvas, c.get());

			if (element->scissor)
				canvas->pop_scissor();
		}
	}

	void sRendererPrivate::render(int tar_idx, graphics::CommandBufferPtr cb)
	{
		if (mark_clear_pipelines)
		{
			gbuffer_batcher.batches.clear();
			transparent_batcher.batches.clear();
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
			for (auto& pl : pls_deferred)
				graphics::GraphicsPipeline::release(pl.second);
			pls_deferred.clear();

			mark_clear_pipelines = false;
		}

		// clear staging draws and reset buffers
		if (tar_idx < 0)
		{
			outline_groups.clear();
			buf_primitives.reset();
			primitives_draws.clear();
			return;
		}

		hud_callbacks.call();

		auto first = true;
		for (auto& t : render_tasks)
		{
			if (t->camera == INVALID_POINTER || t->targets.empty())
				continue;
			if (!t->camera)
			{
				auto& list = cCamera::list();
				if (list.empty())
					continue;
				t->camera = list.front();
			}

			auto mode = t->mode;
			tar_idx = clamp(tar_idx, 0, (int)t->targets.size() - 1);
			auto iv = t->targets[tar_idx];
			auto img = iv->image;
			auto ext = vec2(img->extent);

			auto camera = t->camera;
			camera->aspect = ext.x / ext.y;
			camera->update_matrices();

			camera_culled_nodes.clear();
			sScene::instance()->octree->get_within_frustum(camera->frustum, camera_culled_nodes);

			draw_data.reset(PassInstance, 0);
			for (auto& n : camera_culled_nodes)
				n.second->drawers.call<DrawData&, cCameraPtr>(draw_data, camera);

			static auto sp_nearest = graphics::Sampler::get(graphics::FilterNearest, graphics::FilterNearest, false, graphics::AddressClampToEdge);

			auto n_dir_lights = 0;
			auto n_dir_shadows = 0;
			auto n_pt_lights = 0;
			auto n_pt_shadows = 0;

			cb->begin_debug_label("Upload Buffers");
			{
				buf_vtx.upload(cb);
				buf_idx.upload(cb);
				buf_vtx_arm.upload(cb);
				buf_idx_arm.upload(cb);

				auto& buf_camera = t->buf_camera;
				buf_camera.child("zNear"_h).as<float>() = camera->zNear;
				buf_camera.child("zFar"_h).as<float>() = camera->zFar;
				buf_camera.child("fovy"_h).as<float>() = camera->fovy;
				buf_camera.child("tan_hf_fovy"_h).as<float>() = tan(radians(camera->fovy * 0.5f));
				buf_camera.child("viewport"_h).as<vec2>() = ext;
				buf_camera.child("coord"_h).as<vec3>() = camera->node->global_pos();
				buf_camera.child("front"_h).as<vec3>() = -camera->view_mat_inv[2];
				buf_camera.child("right"_h).as<vec3>() = camera->view_mat_inv[0];
				buf_camera.child("up"_h).as<vec3>() = camera->view_mat_inv[1];
				buf_camera.child("last_view"_h).as<mat4>() = buf_camera.child("view"_h).as<mat4>();
				buf_camera.child("view"_h).as<mat4>() = camera->view_mat;
				buf_camera.child("view_inv"_h).as<mat4>() = camera->view_mat_inv;
				buf_camera.child("proj"_h).as<mat4>() = camera->proj_mat;
				buf_camera.child("proj_inv"_h).as<mat4>() = camera->proj_mat_inv;
				buf_camera.child("proj_view"_h).as<mat4>() = camera->proj_view_mat;
				buf_camera.child("proj_view_inv"_h).as<mat4>() = camera->proj_view_mat_inv;
				memcpy(buf_camera.child("frustum_planes"_h).data, camera->frustum.planes, sizeof(vec4) * 6);
				buf_camera.child("time"_h).as<float>() = total_time;
				buf_camera.mark_dirty();
				buf_camera.upload(cb);

				buf_material.upload(cb);

				if (mode == RenderModeShaded)
				{
					draw_data.reset(PassLight, 0);
					for (auto& n : camera_culled_nodes)
					{
						n.second->drawers.call<DrawData&, cCameraPtr>(draw_data, camera);
						auto n_lights = n_dir_lights + n_pt_lights;
						if (n_lights < draw_data.lights.size())
						{
							for (auto i = n_lights; i < draw_data.lights.size(); i++)
							{
								auto& l = draw_data.lights[i];
								switch (l.type)
								{
								case LightDirectional:
									buf_lighting.mark_dirty_ci("dir_lights_list"_h, n_dir_lights).as<uint>() = l.ins_id;
									if (l.cast_shadow)
									{
										if (n_dir_shadows < countof(dir_shadows))
										{
											auto idx = n_dir_shadows;
											auto shadow_index = buf_lighting.child("dir_lights"_h).item(l.ins_id).child("shadow_index"_h);
											shadow_index.as<int>() = idx;
											buf_lighting.mark_dirty(shadow_index);

											auto& rot = dir_shadows[idx].rot;
											rot = mat3(n.second->g_qut);
											rot[2] *= -1.f;

											n_dir_shadows++;
										}
									}
									n_dir_lights++;
									break;
								case LightPoint:
									buf_lighting.mark_dirty_ci("pt_lights_list"_h, n_dir_lights).as<uint>() = l.ins_id;
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

					buf_lighting.mark_dirty_c("dir_lights_count"_h).as<uint>() = n_dir_lights;
					buf_lighting.mark_dirty_c("pt_lights_count"_h).as<uint>() = n_pt_lights;
				}
				else if (first && (mode == RenderModeCameraLight || mode == RenderModeSimple))
				{
					auto ins = buf_lighting.mark_dirty_ci("dir_lights"_h, camera_light_id);
					ins.child("dir"_h).as<vec3>() = camera->view_mat_inv[2];
					ins.child("color"_h).as<vec3>() = vec3(1.f);
					ins.child("shadow_index"_h).as<int>() = -1;

					buf_lighting.mark_dirty_ci("dir_lights_list"_h, 0).as<uint>() = camera_light_id;
					buf_lighting.mark_dirty_c("dir_lights_count"_h).as<uint>() = 1;
					buf_lighting.mark_dirty_c("pt_lights_count"_h).as<uint>() = 0;
				}

				buf_lighting.upload(cb);
			}
			cb->end_debug_label();

			// occulder pass
			if (mode == RenderModeShaded)
			{
				cb->begin_debug_label("Occulder Pass");

				for (auto i = 0; i < DirShadowMaxCount; i++)
				{
					auto& s = dir_shadows[i];
					for (auto j = 0; j < DirShadowMaxLevels; j++)
					{
						for (auto& b : s.batcher[j].batches)
							b.second.draw_indices.clear();
					}
				}

				if (shadow_distance > 0.f)
				{
					auto zn = camera->zNear; auto zf = camera->zFar;
					for (auto i = 0; i < n_dir_shadows; i++)
					{
						auto& s = dir_shadows[i];
						auto splits = vec4(zf);
						auto shadow = buf_lighting.mark_dirty_ci("dir_shadows"_h, i);
						auto mats = (mat4*)shadow.child("mats"_h).data;
						for (auto lv = 0; lv < csm_levels; lv++)
						{
							auto n = lv / (float)csm_levels;
							auto f = (lv + 1) / (float)csm_levels;
							n = mix(zn, shadow_distance, n * n);
							f = mix(zn, shadow_distance, f * f);
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
							if (csm_debug_capture_flag)
							{
								{
									auto& prims = csm_debug_draws.emplace_back();
									prims.type = PrimitiveQuadList;
									prims.points = Frustum::points_to_quads(frustum_slice.data());
									prims.color = cvec4(0, 127, 255, 190);
									prims.depth_test = false;
								}
								{
									auto& prims = csm_debug_draws.emplace_back();
									prims.type = PrimitiveLineList;
									prims.points = Frustum::points_to_lines(frustum_slice.data());
									prims.color = cvec4(255, 255, 255, 255);
									prims.depth_test = false;
								}
							}
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
							for (auto& n : s.culled_nodes)
								n.second->drawers.call<DrawData&, cCameraPtr>(draw_data, camera);

							auto z_min = 0.f;
							auto z_max = 0.f;
							auto n_mesh_draws = 0;
							auto n_terrain_draws = 0;
							auto n_MC_draws = 0;

							draw_data.reset(PassOcculder, CateMesh | CateTerrain | CateMarchingCubes);
							for (auto& n : s.culled_nodes)
							{
								n.second->drawers.call<DrawData&, cCameraPtr>(draw_data, camera);
								if (draw_data.meshes.size() > n_mesh_draws)
								{
									for (auto& p : n.second->bounds.get_points())
									{
										auto d = dot(p - c, s.rot[2]);
										z_min = min(d, z_min);
										z_max = max(d, z_max);
									}

									n_mesh_draws = draw_data.meshes.size();
								}
								if (draw_data.terrains.size() > n_terrain_draws)
								{
									for (auto& p : n.second->bounds.get_points())
									{
										auto d = dot(p - c, s.rot[2]);
										z_min = min(d, z_min);
										z_max = max(d, z_max);
									}

									n_terrain_draws = draw_data.terrains.size();
								}
								if (draw_data.volumes.size() > n_MC_draws)
								{
									for (auto& p : n.second->bounds.get_points())
									{
										auto d = dot(p - c, s.rot[2]);
										z_min = min(d, z_min);
										z_max = max(d, z_max);
									}

									n_MC_draws = draw_data.volumes.size();
								}
							}
							s.batcher[lv].collect(mode, draw_data, cb, "DEPTH_ONLY"_h);
							s.draw_terrains[lv] = draw_data.terrains;
							s.draw_MCs[lv] = draw_data.volumes;

							proj = orthoRH(-hf_xlen, +hf_xlen, -hf_ylen, +hf_ylen, 0.f, z_max - z_min);
							proj[1][1] *= -1.f;
							view = lookAt(c + s.rot[2] * z_min, c, s.rot[1]);
							proj_view = proj * view;
							mats[lv] = proj_view;
							s.frustum = Frustum(inverse(proj_view));
							if (csm_debug_capture_flag)
							{
								auto frustum_points = Frustum::get_points(inverse(proj_view));
								{
									Primitives prims;
									prims.type = PrimitiveQuadList;
									prims.points = Frustum::points_to_quads(frustum_points.data());
									prims.color = cvec4(255, 127, 0, 190);
									prims.depth_test = false;
									csm_debug_draws.emplace(csm_debug_draws.begin(), prims);
								}
								{
									auto& prims = csm_debug_draws.emplace_back();
									prims.type = PrimitiveLineList;
									prims.points = Frustum::points_to_lines(frustum_points.data());
									prims.color = cvec4(255, 255, 255, 255);
									prims.depth_test = false;
								}
								auto c = (frustum_points[0] + frustum_points[6]) * 0.5f;
								vec3 pts[2];
								pts[0] = c; pts[1] = c + s.rot[0] * hf_xlen;
								{
									auto& prims = csm_debug_draws.emplace_back();
									prims.type = PrimitiveLineList;
									prims.points = { pts[0], pts[1] };
									prims.color = cvec4(255, 0, 0, 255);
									prims.depth_test = false;
								}
								pts[0] = c; pts[1] = c + s.rot[1] * hf_ylen;
								{
									auto& prims = csm_debug_draws.emplace_back();
									prims.type = PrimitiveLineList;
									prims.points = { pts[0], pts[1] };
									prims.color = cvec4(0, 255, 0, 255);
									prims.depth_test = false;
								}
								pts[0] = c; pts[1] = c + s.rot[2] * (z_max - z_min) * 0.5f;
								{
									auto& prims = csm_debug_draws.emplace_back();
									prims.type = PrimitiveLineList;
									prims.points = { pts[0], pts[1] };
									prims.color = cvec4(0, 0, 255, 255);
									prims.depth_test = false;
								}
							}
						}

						shadow.child("splits"_h).as<vec4>() = splits;
						shadow.child("far"_h).as<float>() = shadow_distance;
					}

					csm_debug_capture_flag = false;

					for (auto i = 0; i < n_pt_shadows; i++)
					{

					}
				}

				buf_instance.upload(cb);

				auto set_blur_args = [&](const vec2 img_size) {
					t->prm_blur.pc.child("off"_h).as<int>() = -3;
					t->prm_blur.pc.child("len"_h).as<int>() = 7;
					t->prm_blur.pc.child("pxsz"_h).as<vec2>() = 1.f / img_size;
					const auto len = 7;
					memcpy(t->prm_blur.pc.child("weights"_h).data, get_gauss_blur_weights(len), sizeof(float) * len);
					t->prm_blur.pc.mark_dirty();
					t->prm_blur.push_constant(cb);
				};

				cb->set_viewport_and_scissor(Rect(vec2(0), ShadowMapSize));
				for (auto i = 0; i < n_dir_shadows; i++)
				{
					auto& s = dir_shadows[i];
					for (auto lv = 0U; lv < csm_levels; lv++)
					{
						cb->begin_renderpass(nullptr, fbs_dir_shadow[i * DirShadowMaxLevels + lv].get(), { vec4(std::numeric_limits<float>::max(), 0.f, 0.f, 0.f), vec4(1.f, 0.f, 0.f, 0.f) });
						t->prm_fwd.bind_dss(cb);
						t->prm_fwd.pc.mark_dirty_c("i"_h).as<ivec4>() = ivec4(0, i, lv, 0);
						t->prm_fwd.push_constant(cb);

						s.batcher[lv].draw(cb);

						for (auto& dt : s.draw_terrains[lv])
						{
							cb->bind_pipeline(get_material_pipeline(mode, mat_reses[dt.mat_id], "terrain"_h, 0, "DEPTH_ONLY"_h));
							t->prm_fwd.pc.mark_dirty_c("index"_h).as<uint>() = (dt.mat_id << 16) + dt.ins_id;
							t->prm_fwd.push_constant(cb);
							cb->draw(4, dt.blocks.x * dt.blocks.y, 0, 0);
						}
						for (auto& dv : s.draw_MCs[lv])
						{
							cb->bind_pipeline(get_material_pipeline(mode, mat_reses[dv.mat_id], "marching_cubes"_h, 0, "DEPTH_ONLY"_h));
							t->prm_fwd.pc.mark_dirty_c("index"_h).as<uint>() = (dv.mat_id << 16) + dv.ins_id;
							for (auto z = 0; z < dv.blocks.z; z++)
							{
								for (auto y = 0; y < dv.blocks.y; y++)
								{
									for (auto x = 0; x < dv.blocks.x; x++)
									{
										t->prm_fwd.pc.mark_dirty_c("offset"_h).as<vec3>() = vec3(x, y, z);
										t->prm_fwd.push_constant(cb);
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
						cb->bind_pipeline(pl_blur_h);
						cb->bind_descriptor_set(0, imgs_dir_shadow[i]->get_shader_read_src(0, lv));
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();

						cb->image_barrier(img_dir_shadow_back.get(), { 0U, 1U, lv, 1 }, graphics::ImageLayoutShaderReadOnly);
						cb->begin_renderpass(nullptr, imgs_dir_shadow[i]->get_shader_write_dst(0, lv));
						cb->bind_pipeline(pl_blur_v);
						cb->bind_descriptor_set(0, img_dir_shadow_back->get_shader_read_src(0, lv));
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();
					}
					cb->image_barrier(imgs_dir_shadow[i].get(), { 0U, 1U, 0U, csm_levels }, graphics::ImageLayoutShaderReadOnly);
				}

				cb->set_viewport_and_scissor(Rect(vec2(0), ShadowMapSize / 2U));

				cb->end_debug_label();
			}
			else
				buf_instance.upload(cb);

			cb->set_viewport_and_scissor(Rect(vec2(0), ext));

			if (mode == RenderModeShaded || mode == RenderModeCameraLight)
			{
				// deferred shading pass
				cb->begin_debug_label("Deferred Shading");
				{
					for (auto& b : gbuffer_batcher.batches)
						b.second.draw_indices.clear();
					draw_data.reset(PassGBuffer, CateMesh | CateTerrain | CateSDF | CateMarchingCubes);
					for (auto& n : camera_culled_nodes)
					{
						if (n.second->entity->layer & camera->layer)
							n.second->drawers.call<DrawData&, cCameraPtr>(draw_data, camera);
					}
					gbuffer_batcher.collect(mode, draw_data, cb);

					cb->image_barrier(t->img_dst.get(), {}, graphics::ImageLayoutAttachment);
					cb->set_viewport_and_scissor(Rect(vec2(0), ext));

					cb->begin_debug_label("GBuffer Pass");
					{
						auto& prm_gbuf = t->prm_gbuf;

						cb->begin_renderpass(nullptr, t->fb_gbuf.get(),
							{ vec4(0.f, 0.f, 0.f, 0.f),
							vec4(0.f, 0.f, 0.f, 0.f),
							vec4(0.f, 0.f, 0.f, 0.f),
							vec4(0.f, 0.f, 0.f, 0.f),
							vec4(1.f, 0.f, 0.f, 0.f) });

						t->prm_gbuf.bind_dss(cb);
						gbuffer_batcher.draw(cb);

						for (auto& t : draw_data.terrains)
						{
							cb->bind_pipeline(get_material_pipeline(mode, mat_reses[t.mat_id], "terrain"_h, 0, 0));
							prm_gbuf.pc.mark_dirty_c("index"_h).as<uint>() = (t.mat_id << 16) + t.ins_id;
							prm_gbuf.push_constant(cb);
							cb->draw(4, t.blocks.x * t.blocks.y, 0, 0);
						}
						for (auto& s : draw_data.sdfs)
						{
							cb->bind_pipeline(get_material_pipeline(mode, mat_reses[s.mat_id], "sdf"_h, 0, 0));
							prm_gbuf.pc.mark_dirty_c("index"_h).as<uint>() = (s.mat_id << 16) + s.ins_id;
							prm_gbuf.push_constant(cb);
							cb->draw(3, 1, 0, 0);
						}
						for (auto& v : draw_data.volumes)
						{
							cb->bind_pipeline(get_material_pipeline(mode, mat_reses[v.mat_id], "marching_cubes"_h, 0, 0));
							prm_gbuf.pc.mark_dirty_c("index"_h).as<uint>() = (v.mat_id << 16) + v.ins_id;
							for (auto z = 0; z < v.blocks.z; z++)
							{
								for (auto y = 0; y < v.blocks.y; y++)
								{
									for (auto x = 0; x < v.blocks.x; x++)
									{
										prm_gbuf.pc.mark_dirty_c("offset"_h).as<vec3>() = (vec3(x, y, z));
										prm_gbuf.push_constant(cb);
										// 128 / 4 = 32
										cb->draw_mesh_tasks(uvec3(32 * 32 * 32, 1, 1));
									}
								}
							}
						}

						cb->end_renderpass();
					}
					cb->end_debug_label();

					cb->image_barrier(t->img_last_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
					cb->image_barrier(t->img_last_dep.get(), {}, graphics::ImageLayoutShaderReadOnly);

					cb->image_barrier(t->img_gbufferA.get(), {}, graphics::ImageLayoutShaderReadOnly);
					cb->image_barrier(t->img_gbufferB.get(), {}, graphics::ImageLayoutShaderReadOnly);
					cb->image_barrier(t->img_gbufferC.get(), {}, graphics::ImageLayoutShaderReadOnly);
					cb->image_barrier(t->img_gbufferD.get(), {}, graphics::ImageLayoutShaderReadOnly);
					cb->image_barrier(t->img_dep.get(), {}, graphics::ImageLayoutShaderReadOnly);
					cb->begin_renderpass(nullptr, t->img_dst->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
					t->prm_deferred.bind_dss(cb);
					cb->bind_pipeline(get_deferred_pipeline());
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();
				}
				cb->end_debug_label();

			}

			// forward pass
			cb->begin_debug_label("Forward Shading");
			{
				if (mode == RenderModeShaded || mode == RenderModeCameraLight)
				{
					cb->image_barrier(t->img_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
					cb->begin_renderpass(nullptr, t->img_dst_ms->get_shader_write_dst());
					cb->bind_pipeline(pl_blit);
					cb->bind_descriptor_set(0, t->img_dst->get_shader_read_src(0, 0, sp_nearest));
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->image_barrier(t->img_dep.get(), {}, graphics::ImageLayoutShaderReadOnly);
					cb->begin_renderpass(nullptr, t->img_dep_ms->get_shader_write_dst());
					cb->bind_pipeline(pl_blit_dep);
					cb->bind_descriptor_set(0, t->img_dep->get_shader_read_src(0, 0, sp_nearest));
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					for (auto& b : transparent_batcher.batches)
						b.second.draw_indices.clear();
					draw_data.reset(PassForward, CateMesh | CateGrassField | CateParticle);
					for (auto& n : camera_culled_nodes)
					{
						if (n.second->entity->layer & camera->layer)
							n.second->drawers.call<DrawData&, cCameraPtr>(draw_data, camera);
					}
					transparent_batcher.collect(mode, draw_data, cb);

					for (auto& p : draw_data.particles)
					{
						auto off = buf_particles.add(nullptr, p.ptcs.size());
						for (auto i = 0; i < p.ptcs.size(); i++)
						{
							auto& src = p.ptcs[i];
							auto dst = buf_particles.item(off + i);
							dst.child("i_pos0"_h).as<vec3>() = src.pos0;
							dst.child("i_pos1"_h).as<vec3>() = src.pos1;
							dst.child("i_pos2"_h).as<vec3>() = src.pos2;
							dst.child("i_pos3"_h).as<vec3>() = src.pos3;
							dst.child("i_uv"_h).as<vec4>() = src.uv;
							dst.child("i_col"_h).as<cvec4>() = src.col;
							dst.child("i_time"_h).as<float>() = src.time;
						}
						auto& ptd = particles_draws.emplace_back();
						ptd.mat_id = p.mat_id;
						ptd.vtx_off = off;
						ptd.vtx_cnt = p.ptcs.size();
					}
					buf_particles.upload(cb);
					buf_particles.reset();

					cb->image_barrier(t->img_dst.get(), {}, graphics::ImageLayoutAttachment);
					cb->image_barrier(t->img_dep.get(), {}, graphics::ImageLayoutAttachment);
					cb->begin_renderpass(nullptr, t->fb_fwd.get());
					t->prm_fwd.bind_dss(cb);

					transparent_batcher.draw(cb);

					for (auto& dt : draw_data.terrains)
					{
						cb->bind_pipeline(get_material_pipeline(mode, mat_reses[dt.mat_id], "grass_field"_h, 0, 0));
						t->prm_fwd.pc.mark_dirty_c("index"_h).as<uint>() = (dt.mat_id << 16) + dt.ins_id;
						t->prm_fwd.push_constant(cb);
						cb->draw(4, dt.blocks.x * dt.blocks.y, 0, 0);
					}

					if (post_processing_enable && outline_pp_enable)
					{
						cb->end_renderpass();

						cb->begin_debug_label("Outline PP");
						auto n_levels = min((uint)std::bit_width(outline_pp_width), t->img_dep->n_levels);
						if (n_levels > 0)
						{
							auto img_normal = t->img_gbufferB.get();

							for (auto i = 1; i < n_levels; i++)
							{
								cb->image_barrier(t->img_dep.get(), {(uint)i - 1, 1, 0, 1}, graphics::ImageLayoutShaderReadOnly);
								cb->set_viewport(Rect(vec2(0.f), vec2(t->img_dep->levels[i].extent)));
								cb->begin_renderpass(nullptr, t->img_dep->get_shader_write_dst(i));
								cb->bind_pipeline(pl_down_sample_depth);
								cb->bind_descriptor_set(0, t->img_dep->get_shader_read_src(i - 1));
								t->prm_box.pc.mark_dirty_c("pxsz"_h).as<vec2>() = 1.f / vec2(t->img_dep->levels[i - 1].extent);
								t->prm_box.push_constant(cb);
								cb->draw(3, 1, 0, 0);
								cb->end_renderpass();

								cb->image_barrier(img_normal, { (uint)i - 1, 1, 0, 1 }, graphics::ImageLayoutShaderReadOnly);
								cb->set_viewport(Rect(vec2(0.f), vec2(img_normal->levels[i].extent)));
								cb->begin_renderpass(nullptr, img_normal->get_shader_write_dst(i));
								cb->bind_pipeline(pl_down_sample);
								cb->bind_descriptor_set(0, img_normal->get_shader_read_src(i - 1));
								t->prm_box.pc.mark_dirty_c("pxsz"_h).as<vec2>() = 1.f / vec2(img_normal->levels[i - 1].extent);
								t->prm_box.push_constant(cb);
								cb->draw(3, 1, 0, 0);
								cb->end_renderpass();
							}

							cb->image_barrier(t->img_dep.get(), {n_levels - 1, 1, 0, 1}, graphics::ImageLayoutShaderReadOnly);
							cb->image_barrier(img_normal, {n_levels - 1, 1, 0, 1}, graphics::ImageLayoutShaderReadOnly);

							cb->begin_renderpass(nullptr, t->img_back0->get_shader_write_dst(n_levels - 1));
							cb->bind_pipeline(pl_outline_pp);
							t->prm_outline_pp.set_ds(""_h, t->dss_outline_pp[n_levels - 1].get());
							t->prm_outline_pp.bind_dss(cb);
							t->prm_outline_pp.pc.mark_dirty_c("pxsz"_h).as<vec2>() = 1.f / vec2(t->img_dep->levels[n_levels - 1].extent);
							t->prm_outline_pp.pc.mark_dirty_c("near"_h).as<float>() = camera->zNear;
							t->prm_outline_pp.pc.mark_dirty_c("far"_h).as<float>() = camera->zFar;
							t->prm_outline_pp.pc.mark_dirty_c("depth_scale"_h).as<float>() = outline_pp_depth_scale;
							t->prm_outline_pp.pc.mark_dirty_c("normal_scale"_h).as<float>() = outline_pp_normal_scale;
							t->prm_outline_pp.pc.mark_dirty_c("color"_h).as<vec3>() = outline_pp_color;
							t->prm_outline_pp.push_constant(cb);
							cb->draw(3, 1, 0, 0);
							cb->end_renderpass();

							cb->set_viewport(Rect(vec2(0), ext));
							cb->image_barrier(t->img_back0.get(), {n_levels - 1}, graphics::ImageLayoutShaderReadOnly);
							cb->begin_renderpass(nullptr, t->img_dst->get_shader_write_dst());
							cb->bind_pipeline(pl_blit_blend);
							cb->bind_descriptor_set(0, t->img_back0->get_shader_read_src(n_levels - 1));
							cb->draw(3, 1, 0, 0);
							cb->end_renderpass();
						}
						cb->end_debug_label();

						cb->image_barrier(t->img_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
						cb->begin_renderpass(nullptr, t->img_dst_ms->get_shader_write_dst());
						cb->bind_pipeline(pl_blit);
						cb->bind_descriptor_set(0, t->img_dst->get_shader_read_src(0, 0, sp_nearest));
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();

						cb->image_barrier(t->img_dst.get(), {}, graphics::ImageLayoutAttachment);
						cb->image_barrier(t->img_dep.get(), {}, graphics::ImageLayoutAttachment);
						cb->begin_renderpass(nullptr, t->fb_fwd.get());
						t->prm_fwd.bind_dss(cb);
					}

					cb->bind_vertex_buffer(buf_particles.buf.get(), 0);
					for (auto& ptd : particles_draws)
					{
						cb->bind_pipeline(get_material_pipeline(mode, mat_reses[ptd.mat_id], "particle"_h, 0, 0));
						cb->draw(ptd.vtx_cnt, 1, ptd.vtx_off, ptd.mat_id << 16);
					}
					particles_draws.clear();

					cb->end_renderpass();
				}
				else
				{
					draw_data.reset(PassPickUp, CateMesh); // use pick up to grap all meshes
					for (auto& n : camera_culled_nodes)
					{
						if (n.second->entity->layer & camera->layer)
							n.second->drawers.call<DrawData&, cCameraPtr>(draw_data, camera);
					}

					cb->image_barrier(t->img_dst_ms.get(), {}, graphics::ImageLayoutAttachment);
					cb->image_barrier(t->img_dep_ms.get(), {}, graphics::ImageLayoutAttachment);
					cb->image_barrier(t->img_dst.get(), {}, graphics::ImageLayoutAttachment);
					cb->image_barrier(t->img_dep.get(), {}, graphics::ImageLayoutAttachment);
					cb->begin_renderpass(nullptr, t->fb_fwd_clear.get(), { vec4(0.5f, 0.55f, 0.7f, 1.f), vec4(1.f, 0.f, 0.f, 0.f) });
					t->prm_fwd.bind_dss(cb);

					for (auto& m : draw_data.meshes)
					{
						auto& mesh_res = mesh_reses[m.mesh_id];
						auto& mat_res = mat_reses[m.mat_id];
						if (!mesh_res.arm)
						{
							cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
							cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
							cb->bind_pipeline(get_material_pipeline(mode, mat_res, "mesh"_h, 0, "FORCE_FORWARD"_h));
							cb->draw_indexed(mesh_res.idx_cnt, mesh_res.idx_off, mesh_res.vtx_off, 1, (m.mat_id << 16) + m.ins_id);
						}
						else
						{
							cb->bind_vertex_buffer(buf_vtx_arm.buf.get(), 0);
							cb->bind_index_buffer(buf_idx_arm.buf.get(), graphics::IndiceTypeUint);
							cb->bind_pipeline(get_material_pipeline(mode, mat_res, "mesh"_h, "ARMATURE"_h, "FORCE_FORWARD"_h));
							cb->draw_indexed(mesh_res.idx_cnt, mesh_res.idx_off, mesh_res.vtx_off, 1, (m.mat_id << 16) + m.ins_id);
						}
					}

					cb->end_renderpass();
				}
			}
			cb->end_debug_label();

			if (mode == RenderModeShaded)
			{
				cb->begin_debug_label("Store Last Frame");

				cb->image_barrier(t->img_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
				cb->image_barrier(t->img_dep.get(), {}, graphics::ImageLayoutShaderReadOnly);

				cb->begin_renderpass(nullptr, t->img_last_dst->get_shader_write_dst());
				cb->bind_pipeline(pl_blit);
				cb->bind_descriptor_set(0, t->img_dst->get_shader_read_src(0, 0, sp_nearest));
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();

				cb->begin_renderpass(nullptr, t->img_last_dep->get_shader_write_dst());
				cb->bind_pipeline(pl_blit_dep);
				cb->bind_descriptor_set(0, t->img_dep->get_shader_read_src(0, 0, sp_nearest));
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();

				cb->end_debug_label();
			}

			auto draw_object = [&](const ObjectDrawData& d, const cvec4& color) {
				switch (d.type)
				{
				case "mesh"_h:
				{
					auto& mesh_r = mesh_reses[d.res_id];

					t->prm_fwd.bind_dss(cb);
					t->prm_fwd.pc.mark_dirty_c("f"_h).as<vec4>() = vec4(color) / 255.f;
					t->prm_fwd.push_constant(cb);

					if (!mesh_r.arm)
					{
						cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
						cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
						cb->bind_pipeline(pl_mesh_plain);
						cb->draw_indexed(mesh_r.idx_cnt, mesh_r.idx_off, mesh_r.vtx_off, 1, d.ins_id);
					}
					else
					{
						cb->bind_vertex_buffer(buf_vtx_arm.buf.get(), 0);
						cb->bind_index_buffer(buf_idx_arm.buf.get(), graphics::IndiceTypeUint);
						cb->bind_pipeline(pl_mesh_arm_plain);
						cb->draw_indexed(mesh_r.idx_cnt, mesh_r.idx_off, mesh_r.vtx_off, 1, d.ins_id);
					}
				}
					break;
				case "terrain"_h:
				{
					auto blocks = buf_instance.child("terrains"_h).item(d.ins_id).child("blocks"_h).as<uvec2>();

					t->prm_fwd.bind_dss(cb);
					t->prm_fwd.pc.mark_dirty_c("index"_h).as<uint>() = d.ins_id;
					t->prm_fwd.pc.mark_dirty_c("f"_h).as<vec4>() = vec4(color) / 255.f;
					t->prm_fwd.push_constant(cb);
					cb->bind_pipeline(pl_terrain_plain);
					cb->draw(4, blocks.x * blocks.y, 0, 0);
					cb->end_renderpass();
				}
					break;
				}
			};

			// post processing
			if ((mode == RenderModeShaded || mode == RenderModeCameraLight) && post_processing_enable)
			{
				cb->begin_debug_label("Post Processing");

				if (ssao_enable)
				{
					cb->begin_debug_label("SSAO");
					// TODO
					cb->end_debug_label();
				}

				if (bloom_enable)
				{
					cb->begin_debug_label("Bloom");

					cb->image_barrier(t->img_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
					cb->begin_renderpass(nullptr, t->img_back0->get_shader_write_dst());
					cb->bind_pipeline(pl_bloom_bright);
					cb->bind_descriptor_set(0, t->img_dst->get_shader_read_src(0, 0, sp_nearest));
					t->prm_bloom.pc.mark_dirty_c("white_point"_h).as<float>() = white_point;
					t->prm_bloom.push_constant(cb);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					for (auto i = 1; i < t->img_back0->n_levels; i++)
					{
						cb->image_barrier(t->img_back0.get(), { (uint)i - 1, 1, 0, 1 }, graphics::ImageLayoutShaderReadOnly);
						cb->set_viewport(Rect(vec2(0.f), vec2(t->img_back0->levels[i].extent)));
						cb->begin_renderpass(nullptr, t->img_back0->get_shader_write_dst(i));
						cb->bind_pipeline(pl_down_sample);
						cb->bind_descriptor_set(0, t->img_back0->get_shader_read_src(i - 1));
						t->prm_box.pc.mark_dirty_c("pxsz"_h).as<vec2>() = 1.f / vec2(t->img_back0->levels[i - 1].extent);
						t->prm_box.push_constant(cb);
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();
					}
					for (auto i = (int)t->img_back0->n_levels - 1; i > 1; i--)
					{
						cb->image_barrier(t->img_back0.get(), { (uint)i, 1, 0, 1 }, graphics::ImageLayoutShaderReadOnly);
						cb->set_viewport(Rect(vec2(0.f), vec2(t->img_back0->levels[i - 1].extent)));
						cb->begin_renderpass(nullptr, t->img_back0->get_shader_write_dst(i - 1));
						cb->bind_pipeline(pl_up_sample);
						cb->bind_descriptor_set(0, t->img_back0->get_shader_read_src(i));
						t->prm_box.pc.mark_dirty_c("pxsz"_h).as<vec2>() = 1.f / vec2(t->img_back0->levels[i].extent);
						t->prm_box.push_constant(cb);
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();
					}

					cb->set_viewport(Rect(vec2(0), ext));
					cb->image_barrier(t->img_back0.get(), { 1U }, graphics::ImageLayoutShaderReadOnly);
					cb->begin_renderpass(nullptr, t->img_dst->get_shader_write_dst());
					cb->bind_pipeline(pl_up_sample);
					cb->bind_descriptor_set(0, t->img_back0->get_shader_read_src(1));
					t->prm_box.pc.mark_dirty_c("pxsz"_h).as<vec2>() = 1.f / vec2(t->img_back0->levels[1].extent);
					t->prm_box.push_constant(cb);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->end_debug_label();
				}

				if (dof_enable)
				{
					cb->begin_debug_label("DOF");

					cb->image_barrier(t->img_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
					cb->image_barrier(t->img_dep.get(), {}, graphics::ImageLayoutShaderReadOnly);
					cb->begin_renderpass(nullptr, t->img_back0->get_shader_write_dst());
					cb->bind_pipeline(pl_dof);
					t->prm_dof.bind_dss(cb);
					t->prm_dof.pc.mark_dirty_c("pxsz"_h).as<vec2>() = 1.f / vec2(t->img_dst->extent);
					t->prm_dof.pc.mark_dirty_c("zFar"_h).as<float>() = camera->zFar;
					t->prm_dof.pc.mark_dirty_c("focus_point"_h).as<float>() = dof_focus_point;
					t->prm_dof.pc.mark_dirty_c("focus_scale"_h).as<float>() = dof_focus_scale;
					t->prm_dof.push_constant(cb);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->image_barrier(t->img_back0.get(), {}, graphics::ImageLayoutShaderReadOnly);
					cb->begin_renderpass(nullptr, t->img_dst->get_shader_write_dst());
					cb->bind_pipeline(pl_blit);
					cb->bind_descriptor_set(0, t->img_back0->get_shader_read_src(0, 0, sp_nearest));
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->end_debug_label();
				}

				cb->begin_debug_label("Outline (box)");
				{
					for (auto& od : outline_groups)
					{
						if (od.mode != OutlineBox)
							continue;
						auto n_levels = min((uint)std::bit_width(od.width), t->img_back0->n_levels);
						if (n_levels > 0)
						{
							cb->begin_renderpass(nullptr, t->img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), { vec4(0.f) });
							for (auto& d : od.draws)
								draw_object(d, od.color);
							cb->end_renderpass();

							for (auto i = 1; i < n_levels; i++)
							{
								cb->image_barrier(t->img_back0.get(), { (uint)i - 1, 1, 0, 1 }, graphics::ImageLayoutShaderReadOnly);
								cb->set_viewport(Rect(vec2(0.f), vec2(t->img_back0->levels[i].extent)));
								cb->begin_renderpass(nullptr, t->img_back0->get_shader_write_dst(i, 0, graphics::AttachmentLoadClear), { vec4(0.f) });
								cb->bind_pipeline(pl_down_sample);
								cb->bind_descriptor_set(0, t->img_back0->get_shader_read_src(i - 1));
								t->prm_box.pc.mark_dirty_c("pxsz"_h).as<vec2>() = 1.f / vec2(t->img_back0->levels[i - 1].extent);
								t->prm_box.push_constant(cb);
								cb->draw(3, 1, 0, 0);
								cb->end_renderpass();
							}
							for (auto i = (int)n_levels - 1; i > 0; i--)
							{
								cb->image_barrier(t->img_back0.get(), { (uint)i, 1, 0, 1 }, graphics::ImageLayoutShaderReadOnly);
								cb->set_viewport(Rect(vec2(0.f), vec2(t->img_back0->levels[i - 1].extent)));
								cb->begin_renderpass(nullptr, t->img_back0->get_shader_write_dst(i - 1, 0, graphics::AttachmentLoadClear), { vec4(0.f) });
								cb->bind_pipeline(pl_up_sample);
								cb->bind_descriptor_set(0, t->img_back0->get_shader_read_src(i));
								t->prm_box.pc.mark_dirty_c("pxsz"_h).as<vec2>() = 1.f / vec2(t->img_back0->levels[i].extent);
								t->prm_box.push_constant(cb);
								cb->draw(3, 1, 0, 0);
								cb->end_renderpass();
							}

							cb->begin_renderpass(nullptr, t->img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
							for (auto& d : od.draws)
								draw_object(d, cvec4(0));
							cb->end_renderpass();

							cb->image_barrier(t->img_dst.get(), {}, graphics::ImageLayoutAttachment);
							cb->image_barrier(t->img_back0.get(), {}, graphics::ImageLayoutShaderReadOnly);
							cb->begin_renderpass(nullptr, t->img_dst->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
							cb->bind_pipeline(pl_add);
							cb->bind_descriptor_set(0, t->img_back0->get_shader_read_src(0));
							cb->draw(3, 1, 0, 0);
							cb->end_renderpass();
						}
					}
				}
				cb->end_debug_label();

				if (tone_mapping_enable)
				{
					cb->begin_debug_label("Tone Mapping");

					auto& prm_luma = t->prm_luma;
					cb->image_barrier(t->img_dst.get(), {}, graphics::ImageLayoutGeneral);
					prm_luma.bind_dss(cb);
					const auto min_log_luma = -5.f;
					const auto max_log_luma = +5.f;
					prm_luma.pc.child("min_log_luma"_h).as<float>() = min_log_luma;
					prm_luma.pc.child("log_luma_range"_h).as<float>() = max_log_luma - min_log_luma;
					prm_luma.pc.child("time_coeff"_h).as<float>() = 1.0f;
					prm_luma.pc.child("num_pixels"_h).as<int>() = ext.x * ext.y;
					prm_luma.pc.mark_dirty();
					prm_luma.push_constant(cb);
					cb->bind_pipeline(pl_luma_hist);
					cb->dispatch(uvec3(ceil(ext.x / 16), ceil(ext.y / 16), 1));
					cb->buffer_barrier(buf_luma.buf.get(), graphics::AccessShaderRead | graphics::AccessShaderWrite,
						graphics::AccessShaderRead | graphics::AccessShaderWrite,
						graphics::PipelineStageCompShader, graphics::PipelineStageCompShader);
					cb->bind_pipeline(pl_luma_avg);
					cb->dispatch(uvec3(256, 1, 1));
					cb->buffer_barrier(buf_luma.buf.get(), graphics::AccessShaderRead | graphics::AccessShaderWrite,
						graphics::AccessHostRead,
						graphics::PipelineStageCompShader, graphics::PipelineStageHost);

					auto avg_luma = buf_luma.child("avg"_h);
					cb->copy_buffer(buf_luma.buf.get(), buf_luma.stag.get(), graphics::BufferCopy(buf_luma.offset(avg_luma), avg_luma.type->size));
					cb->buffer_barrier(buf_luma.buf.get(), graphics::AccessHostRead,
						graphics::AccessShaderRead | graphics::AccessShaderWrite,
						graphics::PipelineStageHost, graphics::PipelineStageCompShader);

					cb->image_barrier(t->img_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
					cb->begin_renderpass(nullptr, t->img_back0->get_shader_write_dst());
					cb->bind_pipeline(pl_blit);
					cb->bind_descriptor_set(0, t->img_dst->get_shader_read_src(0, 0, sp_nearest));
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->image_barrier(t->img_back0.get(), {}, graphics::ImageLayoutShaderReadOnly);
					cb->begin_renderpass(nullptr, t->img_back1->get_shader_write_dst());
					cb->bind_pipeline(pl_tone);
					t->prm_tone.bind_dss(cb);
					t->prm_tone.pc.child("average_luminance"_h).as<float>() = *(float*)avg_luma.data;
					t->prm_tone.pc.child("white_point"_h).as<float>() = white_point;
					t->prm_tone.pc.child("one_over_gamma"_h).as<float>() = 1.f / gamma;
					t->prm_tone.pc.mark_dirty();
					t->prm_tone.push_constant(cb);
					cb->bind_descriptor_set(0, t->img_back0->get_shader_read_src(0, 0, sp_nearest));
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->image_barrier(t->img_back1.get(), {}, graphics::ImageLayoutShaderReadOnly);
					cb->begin_renderpass(nullptr, t->img_dst->get_shader_write_dst());
					cb->bind_pipeline(pl_fxaa);
					t->prm_fxaa.pc.child("pxsz"_h).as<vec2>() = 1.f / (vec2)t->img_dst->extent;
					t->prm_fxaa.pc.mark_dirty();
					t->prm_fxaa.push_constant(cb);
					cb->bind_descriptor_set(0, t->img_back1->get_shader_read_src());
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->end_debug_label();
				}

				cb->end_debug_label();
			}

			if (mode == RenderModeShaded || mode == RenderModeCameraLight)
			{
				cb->begin_debug_label("Outline (max)");
				{
					for (auto& od : outline_groups)
					{
						if (od.mode != OutlineMax)
							continue;
						cb->begin_renderpass(nullptr, t->img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), { vec4(0.f) });
						for (auto& d : od.draws)
							draw_object(d, od.color);
						cb->end_renderpass();

						const auto w = od.width;
						auto len = w * 2 + 1;
						t->prm_blur.pc.child("off"_h).as<int>() = -w;
						t->prm_blur.pc.child("len"_h).as<int>() = len;
						t->prm_blur.pc.child("pxsz"_h).as<vec2>() = 1.f / (vec2)t->img_back0->extent;
						memcpy(t->prm_blur.pc.child("weights"_h).data, get_gauss_blur_weights(len), sizeof(float) * len);
						t->prm_blur.pc.mark_dirty();
						t->prm_blur.push_constant(cb);

						cb->image_barrier(t->img_back0.get(), {}, graphics::ImageLayoutShaderReadOnly);
						cb->begin_renderpass(nullptr, t->img_back1->get_shader_write_dst());
						cb->bind_pipeline(pl_blur_max_h);
						cb->bind_descriptor_set(0, t->img_back0->get_shader_read_src());
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();

						cb->image_barrier(t->img_back1.get(), {}, graphics::ImageLayoutShaderReadOnly);
						cb->begin_renderpass(nullptr, t->img_back0->get_shader_write_dst());
						cb->bind_pipeline(pl_blur_max_v);
						cb->bind_descriptor_set(0, t->img_back1->get_shader_read_src());
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();

						cb->begin_renderpass(nullptr, t->img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
						for (auto& d : od.draws)
							draw_object(d, cvec4(0));
						cb->end_renderpass();

						cb->image_barrier(t->img_back0.get(), {}, graphics::ImageLayoutShaderReadOnly);
						cb->begin_renderpass(nullptr, t->img_dst->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
						cb->bind_pipeline(pl_blend);
						cb->bind_descriptor_set(0, t->img_back0->get_shader_read_src());
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();
					}
				}
				cb->end_debug_label();
			}

			if (mode != RenderModeSimple)
			{
				cb->begin_debug_label("Primitives");
				{
					if (!csm_debug_draws.empty())
					{
						for (auto& d : csm_debug_draws)
							draw_primitives(d.type, d.points.data(), d.points.size(), d.color, d.depth_test);
					}

					buf_primitives.upload(cb);
					buf_primitives.reset();
					cb->image_barrier(t->img_dep.get(), {}, graphics::ImageLayoutAttachment);

					auto& prm_plain = t->prm_plain;
					cb->begin_renderpass(nullptr, t->fb_primitive.get());
					cb->bind_vertex_buffer(buf_primitives.buf.get(), 0);
					cb->bind_pipeline_layout(prm_plain.pll);
					prm_plain.pc.mark_dirty_c("mvp"_h).as<mat4>() = camera->proj_view_mat;
					prm_plain.push_constant(cb);
					{
						auto vtx_off = 0;
						for (auto& d : primitives_draws)
						{
							prm_plain.pc.mark_dirty_c("col"_h).as<vec4>() = vec4(d.color) / 255.f;
							prm_plain.push_constant(cb);
							switch (d.type)
							{
							case PrimitivePointList:
								cb->bind_pipeline(d.depth_test ? pl_point3d_dt : pl_point3d);
								break;
							case PrimitiveLineList:
								cb->bind_pipeline(d.depth_test ? pl_line3d_dt : pl_line3d);
								break;
							case PrimitiveLineStrip:
								cb->bind_pipeline(d.depth_test ? pl_line_strip3d_dt : pl_line_strip3d);
								break;
							case PrimitiveTriangleList:
								cb->bind_pipeline(d.depth_test ? pl_triangle3d_dt : pl_triangle3d);
								break;
							}
							cb->draw(d.vtx_cnt, 1, vtx_off, 0);
							vtx_off += d.vtx_cnt;
						}
					}
					cb->end_renderpass();
					primitives_draws.clear();
				}
				cb->end_debug_label();
			}

			if (t->canvas)
			{
				cb->begin_debug_label("Elements");
				{
					if (auto first_element = sScene::instance()->first_element; first_element)
						render_element(t->canvas, first_element);
				}
				cb->end_debug_label();
			}

			cb->image_barrier(img, iv->sub, graphics::ImageLayoutAttachment);
			cb->image_barrier(t->img_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
			cb->begin_renderpass(nullptr, img->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
			cb->bind_pipeline(pl_blit);
			cb->bind_descriptor_set(0, t->img_dst->get_shader_read_src());
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();
			cb->image_barrier(img, iv->sub, t->final_layout);

			first = false;
		}
		// clear draws
		outline_groups.clear();
	}

	void sRendererPrivate::update()
	{
	}

	cNodePtr sRendererPrivate::pick_up(const uvec2& screen_pos, vec3* out_pos, const std::function<void(cNodePtr, DrawData&)>& draw_callback)
	{
		if (render_tasks.empty())
			return nullptr;
		auto render_task = render_tasks.front().get();
		if (!render_task->fb_pickup)
			return nullptr;
		auto camera = render_task->camera;
		if (camera == INVALID_POINTER)
			return nullptr;
		if (!camera)
		{
			auto& list = cCamera::list();
			if (list.empty())
				return nullptr;
			camera = list.front();
		}
		auto& prm_fwd = render_task->prm_fwd;
		auto img_pickup = render_task->img_pickup.get();
		auto img_dep_pickup = render_task->img_dep_pickup.get();
		auto fb_pickup = render_task->fb_pickup.get();
		auto fence_pickup = render_task->fence_pickup.get();

		auto sz = vec2(img_pickup->extent);
		if (screen_pos.x >= sz.x || screen_pos.y >= sz.y)
			return nullptr;

		std::vector<cNodePtr> nodes;

		graphics::InstanceCommandBuffer cb(fence_pickup);
		cb->begin_debug_label("Pick Up");
		{
			cb->set_viewport(Rect(vec2(0), sz));
			cb->set_scissor(Rect(vec2(screen_pos), vec2(screen_pos + 1U)));
			cb->begin_renderpass(nullptr, fb_pickup, { vec4(0.f), vec4(1.f, 0.f, 0.f, 0.f) });
			prm_fwd.bind_dss(cb.get());

			auto n_mesh_draws = 0;
			auto n_terrain_draws = 0;
			auto n_MC_draws = 0;
			draw_data.reset(PassPickUp, CateMesh | CateTerrain | CateMarchingCubes);
			std::vector<std::pair<EntityPtr, cNodePtr>> camera_culled_nodes; // collect here (again), because there may have changes between render() and pick_up()
			sScene::instance()->octree->get_within_frustum(camera->frustum, camera_culled_nodes);
			for (auto& n : camera_culled_nodes)
			{
				if (n.first->tag & TagNotPickable)
					continue;

				if (draw_callback)
					draw_callback(n.second, draw_data);
				else
					n.second->drawers.call<DrawData&, cCameraPtr>(draw_data, camera);

				for (auto i = n_mesh_draws; i < draw_data.meshes.size(); i++)
				{
					auto& m = draw_data.meshes[i];
					auto& mesh_res = mesh_reses[m.mesh_id];
					if (!mesh_res.arm)
					{
						cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
						cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
						cb->bind_pipeline(pl_mesh_pickup);
						prm_fwd.pc.mark_dirty_c("i"_h).as<ivec4>() = ivec4((int)nodes.size() + 1, 0, 0, 0);
						prm_fwd.push_constant(cb.get());
						cb->draw_indexed(mesh_res.idx_cnt, mesh_res.idx_off, mesh_res.vtx_off, 1, m.ins_id);
					}
					else
					{
						cb->bind_vertex_buffer(buf_vtx_arm.buf.get(), 0);
						cb->bind_index_buffer(buf_idx_arm.buf.get(), graphics::IndiceTypeUint);
						cb->bind_pipeline(pl_mesh_arm_pickup);
						prm_fwd.pc.mark_dirty_c("i"_h).as<ivec4>() = ivec4((int)nodes.size() + 1, 0, 0, 0);
						prm_fwd.push_constant(cb.get());
						cb->draw_indexed(mesh_res.idx_cnt, mesh_res.idx_off, mesh_res.vtx_off, 1, m.ins_id);
					}

					nodes.push_back(n.second);
				}
				n_mesh_draws = draw_data.meshes.size();

				for (auto i = n_terrain_draws; i < draw_data.terrains.size(); i++)
				{
					cb->bind_pipeline(pl_terrain_pickup);
					auto& t = draw_data.terrains[i];
					prm_fwd.pc.mark_dirty_c("index"_h).as<uint>() = (t.mat_id << 16) + t.ins_id;
					prm_fwd.pc.mark_dirty_c("i"_h).as<ivec4>() = ivec4((int)nodes.size() + 1, 0, 0, 0);
					prm_fwd.push_constant(cb.get());
					cb->draw(4, t.blocks.x * t.blocks.y, 0, 0);

					nodes.push_back(n.second);
				}
				n_terrain_draws = draw_data.terrains.size();

				for (auto i = n_MC_draws; i < draw_data.volumes.size(); i++)
				{
					cb->bind_pipeline(pl_MC_pickup);
					auto& v = draw_data.volumes[i];
					prm_fwd.pc.mark_dirty_c("index"_h).as<uint>() = (v.mat_id << 16) + v.ins_id;
					prm_fwd.pc.mark_dirty_c("i"_h).as<ivec4>() = ivec4((int)nodes.size() + 1, 0, 0, 0);
					for (auto z = 0; z < v.blocks.z; z++)
					{
						for (auto y = 0; y < v.blocks.y; y++)
						{
							for (auto x = 0; x < v.blocks.x; x++)
							{
								prm_fwd.pc.mark_dirty_c("offset"_h).as<vec3>() = vec3(x, y, z);
								prm_fwd.push_constant(cb.get());
								// 128 / 4 = 32
								cb->draw_mesh_tasks(uvec3(32 * 32 * 32, 1, 1));
							}
						}
					}

					nodes.push_back(n.second);
				}
				n_MC_draws = draw_data.volumes.size();
			}

			cb->end_renderpass();
		}
		cb->end_debug_label();
		if (draw_data.graphics_debug)
			graphics::Debug::start_capture_frame();
		cb.excute();
		if (draw_data.graphics_debug)
			graphics::Debug::end_capture_frame();

		int index; uint depth_data;
		graphics::StagingBuffer sb(sizeof(index) + sizeof(depth_data), nullptr, graphics::BufferUsageTransferDst);
		{
			graphics::InstanceCommandBuffer cb(nullptr);
			cb->begin_debug_label("Get Pick Up Result");
			graphics::BufferImageCopy cpy;
			cpy.img_off = uvec3(screen_pos, 0);
			cpy.img_ext = uvec3(1U);
			cb->image_barrier(img_pickup, cpy.img_sub, graphics::ImageLayoutTransferSrc);
			cb->copy_image_to_buffer(img_pickup, sb.get(), cpy);
			cb->image_barrier(img_pickup, cpy.img_sub, graphics::ImageLayoutAttachment); 
			if (out_pos)
			{
				cpy.buf_off = sizeof(uint);
				cb->image_barrier(img_dep_pickup, cpy.img_sub, graphics::ImageLayoutTransferSrc);
				cb->copy_image_to_buffer(img_dep_pickup, sb.get(), cpy);
				cb->image_barrier(img_dep_pickup, cpy.img_sub, graphics::ImageLayoutAttachment);
			}

			cb->end_debug_label();
			cb.excute();
		}
		memcpy(&index, sb->mapped, sizeof(index));
		if (out_pos)
		{
			memcpy(&depth_data, (char*)sb->mapped + sizeof(index), sizeof(depth_data));
			float depth;
			if (dep_fmt == graphics::Format::Format_Depth16)
				depth = depth_data / 65535.f;
			else
				depth = *(float*)&depth_data;
			auto p = vec4(vec2(screen_pos) / sz * 2.f - 1.f, depth, 1.f);
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

	cElementPtr sRendererPrivate::pick_up_2d(const uvec2& screen_pos)
	{
		auto first_element = sScene::instance()->first_element; 
		if (!first_element)
			return nullptr;

		cElementPtr ret = nullptr;

		first_element->traversal_bfs([&](EntityPtr e, int depth) {
			if (!e->global_enable)
				return false;

			if (auto element = e->get_component<cElementT>(); element)
			{
				if (Rect(element->global_pos0(), element->global_pos1()).contains(screen_pos))
					ret = element;
			}

			return true;
		});

		return ret;
	}

	std::vector<vec3> sRendererPrivate::transform_feedback(cNodePtr node)
	{
		std::vector<vec3> ret;
		if (render_tasks.empty())
			return ret;
		auto render_task = render_tasks.front().get();
		if (!render_task->fb_pickup)
			return ret;

		auto fb_pickup = render_task->fb_pickup.get();
		auto& prm_fwd = render_task->prm_fwd;

		graphics::InstanceCommandBuffer cb;

		buf_transform_feedback.mark_dirty_c("vertex_count"_h).as<uint>() = 0;
		buf_transform_feedback.upload(cb.get());

		cb->set_viewport_and_scissor(Rect(vec2(0), vec2(1)));
		cb->begin_renderpass(nullptr, fb_pickup, { vec4(0.f), vec4(1.f, 0.f, 0.f, 0.f) });
		prm_fwd.bind_dss(cb.get());

		draw_data.reset(PassTransformFeedback, CateMesh | CateTerrain | CateMarchingCubes);
		node->drawers.call<DrawData&, cCameraPtr>(draw_data, nullptr);
		cb->bind_pipeline(pl_MC_transform_feedback);
		for (auto& v : draw_data.volumes)
		{
			prm_fwd.pc.mark_dirty_c("index"_h).as<uint>() = (v.mat_id << 16) + v.ins_id;
			for (auto z = 0; z < v.blocks.z; z++)
			{
				for (auto y = 0; y < v.blocks.y; y++)
				{
					for (auto x = 0; x < v.blocks.x; x++)
					{
						prm_fwd.pc.mark_dirty_c("offset"_h).as<vec3>() = vec3(x, y, z);
						prm_fwd.push_constant(cb.get());
						// 128 / 4 = 32
						cb->draw_mesh_tasks(uvec3(32 * 32 * 32, 1, 1));
					}
				}
			}
		}

		cb->end_renderpass();

		buf_transform_feedback.mark_dirty();
		buf_transform_feedback.download(cb.get());

		cb.excute();

		auto num = buf_transform_feedback.child("vertex_count"_h).as<uint>();
		ret.resize(num);
		auto vertex_x = buf_transform_feedback.child("vertex_x"_h);
		auto vertex_y = buf_transform_feedback.child("vertex_y"_h);
		auto vertex_z = buf_transform_feedback.child("vertex_z"_h);
		for (auto i = 0; i < num; i++)
		{
			ret[i].x = vertex_x.item(i).as<float>();
			ret[i].y = vertex_y.item(i).as<float>();
			ret[i].z = vertex_z.item(i).as<float>();
		}

		return ret;
	}

	graphics::ImagePtr sRendererPrivate::get_image(uint name)
	{
		switch (name)
		{
		case "GBufferA"_h:
			return render_tasks.front()->img_gbufferA.get();
		case "GBufferB"_h:
			return render_tasks.front()->img_gbufferB.get();
		case "GBufferC"_h:
			return render_tasks.front()->img_gbufferC.get();
		case "GBufferD"_h:
			return render_tasks.front()->img_gbufferD.get();
		}
		return nullptr;
	}

	HudLayout& sRendererPrivate::hud_add_layout(HudLayoutType type)
	{
		auto& hud = huds.back();
		auto pos = hud.layouts.back().cursor;
		auto& layout = hud.layouts.emplace_back();
		layout.type = type;
		layout.rect.a = layout.rect.b = pos;
		layout.cursor = pos;
		layout.auto_size = true;
		return layout;
	}

	void sRendererPrivate::hud_finish_layout(HudLayout& layout)
	{
		auto scaling = hud_style_vars[HudStyleVarScaling].top();

		if (layout.auto_size)
		{
			if (layout.rect.b.x > layout.rect.a.x && layout.rect.b.y > layout.rect.a.y)
				layout.rect.b -= layout.item_spacing * scaling;
		}
	}

	void sRendererPrivate::hud_begin(const vec2& pos, const vec2& size, const cvec4& col, const vec2& pivot, const graphics::ImageDesc& image, const vec4& border)
	{
		auto& hud = huds.emplace_back();

		auto canvas = render_tasks.front()->canvas;
		auto scaling = hud_style_vars[HudStyleVarScaling].top();
		auto auto_sizing = size.x == 0.f && size.y == 0.f;

		hud.pos = pos;
		if (pos.x < 0.f)
			hud.pos.x += canvas->size.x;
		if (pos.y < 0.f)
			hud.pos.y += canvas->size.y;
		hud.size = size * scaling;
		if (!auto_sizing)
			hud.pos -= hud.size * pivot;
		hud.color = col;
		hud.pivot = pivot;
		hud.border = border * vec4(scaling, scaling);

		hud.layouts.clear();
		auto& layout = hud.layouts.emplace_back();
		layout.type = HudVertical;
		layout.rect.a = layout.rect.b = hud.pos + hud.border.xy();
		layout.cursor = layout.rect.a;
		layout.auto_size = auto_sizing;

		if (!image.view)
		{
			hud.bg_verts = canvas->add_rect_filled(vec2(0.f), vec2(100.f), col); // 100 for temporary size
			hud.bg_vert_count = 4;
		}
		else
		{
			auto size = (vec2)image.view->image->extent.xy() * (image.uvs.zw() - image.uvs.xy());
			hud.bg_verts = canvas->add_image_stretched(image.view, vec2(0.f), vec2(border.xy() + border.zw()) + vec2(1.f), image.uvs, border, image.border_uvs, col);
			hud.bg_vert_count = 9 * 4;
		}
		if ((pivot.x != 0.f || pivot.y != 0.f) && auto_sizing)
			hud.translate_cmd_idx = canvas->set_translate(vec2(0.f));
		else
			hud.translate_cmd_idx = -1;
	}

	void sRendererPrivate::hud_end()
	{
		if (huds.empty())
			return;
		auto& hud = huds.back();
		if (hud.layouts.empty())
			return;
		auto& layout = hud.layouts[0];

		auto canvas = render_tasks.front()->canvas;
		auto input = sInput::instance();

		hud_finish_layout(layout);
		if (layout.auto_size)
			hud.size = layout.rect.size() + hud.border.xy() + hud.border.zw();

		if (hud.translate_cmd_idx != -1)
		{
			auto translate = -hud.size * hud.pivot;
			hud.pos += translate;
			canvas->draw_cmds[hud.translate_cmd_idx].data.translate = translate;
			canvas->set_translate(vec2(0.f));
		}

		if (hud.bg_verts)
		{
			if (hud.bg_vert_count <= 4)
			{
				auto scl = hud.size / 100.f; // 100 is temporary size
				for (auto i = 0; i < hud.bg_vert_count; i++)
					hud.bg_verts[i].pos = hud.bg_verts[i].pos * scl + hud.pos;
			}
			else // is a stretched image
			{
				auto sz = hud.size - hud.bg_verts[1].pos;
				// hud.bg_verts[0] stays the same
				// hud.bg_verts[1] stays the same
				hud.bg_verts[2].pos.x += sz.x - 1.f;
				hud.bg_verts[3].pos.x += sz.x - 1.f;

				hud.bg_verts[4].pos.y += sz.y - 1.f;
				hud.bg_verts[5].pos.y += sz.y - 1.f;
				hud.bg_verts[6].pos += sz - 1.f;
				hud.bg_verts[7].pos += sz - 1.f;

				// hud.bg_verts[8] stays the same
				hud.bg_verts[9].pos.y += sz.y - 1.f;
				hud.bg_verts[10].pos.y += sz.y - 1.f;
				// hud.bg_verts[11] stays the same

				hud.bg_verts[12].pos.x += sz.x - 1.f;
				hud.bg_verts[13].pos += sz - 1.f;
				hud.bg_verts[14].pos += sz - 1.f;
				hud.bg_verts[15].pos.x += sz.x - 1.f;

				// hud.bg_verts[16] stays the same
				// hud.bg_verts[17] stays the same
				// hud.bg_verts[18] stays the same
				// hud.bg_verts[19] stays the same

				hud.bg_verts[20].pos.x += sz.x - 1.f;
				hud.bg_verts[21].pos.x += sz.x - 1.f;
				hud.bg_verts[22].pos.x += sz.x - 1.f;
				hud.bg_verts[23].pos.x += sz.x - 1.f;

				hud.bg_verts[24].pos.y += sz.y - 1.f;
				hud.bg_verts[25].pos.y += sz.y - 1.f;
				hud.bg_verts[26].pos.y += sz.y - 1.f;
				hud.bg_verts[27].pos.y += sz.y - 1.f;

				hud.bg_verts[28].pos += sz - 1.f;
				hud.bg_verts[29].pos += sz - 1.f;
				hud.bg_verts[30].pos += sz - 1.f;
				hud.bg_verts[31].pos += sz - 1.f;

				// hud.bg_verts[32] stays the same
				hud.bg_verts[33].pos.y += sz.y - 1.f;
				hud.bg_verts[34].pos += sz - 1.f;
				hud.bg_verts[35].pos.x += sz.x - 1.f;

				for (auto i = 0; i < hud.bg_vert_count; i++)
					hud.bg_verts[i].pos += hud.pos;
			}
		}
		Rect rect(hud.pos, hud.pos + hud.size);
		if (hud.color.a > 0 && rect.contains(input->mpos))
			input->mouse_used = true;

		huds.pop_back();
	}

	void sRendererPrivate::hud_set_cursor(const vec2& pos)
	{
		if (huds.empty())
			return;
		auto& hud = huds.back();
		if (hud.layouts.empty())
			return;
		auto& layout = hud.layouts.back();

		layout.cursor = pos;
		if (layout.auto_size)
			layout.rect.b = max(layout.rect.b, pos);
	}

	Rect sRendererPrivate::hud_get_rect() const
	{
		return hud_last_rect;
	}

	vec2 sRendererPrivate::hud_screen_size() const
	{
		auto canvas = render_tasks.front()->canvas;

		return canvas->size;
	}

	void sRendererPrivate::hud_push_style(HudStyleVar var, const vec2& value)
	{
		hud_style_vars[var].push(value);
	}

	void sRendererPrivate::hud_pop_style(HudStyleVar var)
	{
		hud_style_vars[var].pop();
	}

	void sRendererPrivate::hud_begin_layout(HudLayoutType type, const vec2& item_spacing)
	{
		auto& layout = hud_add_layout(type);
		layout.item_spacing = item_spacing;
	}

	void sRendererPrivate::hud_end_layout()
	{
		if (huds.empty())
			return;
		auto& hud = huds.back();
		auto& layout = hud.layouts.back();
		hud_finish_layout(layout);
		auto size = layout.rect.size();
		hud.layouts.pop_back();
		hud_add_rect(size);
	}

	void sRendererPrivate::hud_new_line()
	{
		if (huds.empty())
			return;
		auto& hud = huds.back();
		if (hud.layouts.empty())
			return;
		auto& layout = hud.layouts.back();

		auto scaling = hud_style_vars[HudStyleVarScaling].top();

		if (layout.type == HudHorizontal)
		{
			layout.cursor.x = layout.rect.a.x;
			layout.cursor.y += layout.item_max.y + layout.item_spacing.y * scaling.y;
			if (layout.auto_size)
				layout.rect.b = max(layout.rect.b, layout.cursor);
		}
	}

	void sRendererPrivate::hud_begin_stencil_write()
	{
		auto canvas = render_tasks.front()->canvas;
		canvas->begin_stencil_write();
	}

	void sRendererPrivate::hud_end_stencil_write()
	{
		auto canvas = render_tasks.front()->canvas;
		canvas->end_stencil_write();
	}

	void sRendererPrivate::hud_begin_stencil_compare()
	{
		auto canvas = render_tasks.front()->canvas;
		canvas->begin_stencil_compare();
	}

	void sRendererPrivate::hud_end_stencil_compare()
	{
		auto canvas = render_tasks.front()->canvas;
		canvas->end_stencil_compare();
	}

	Rect sRendererPrivate::hud_add_rect(const vec2& _sz)
	{
		if (huds.empty())
			return Rect();
		auto& hud = huds.back();
		if (hud.layouts.empty())
			return Rect();
		auto& layout = hud.layouts.back();

		auto item_spacing = layout.item_spacing;
		auto scaling = hud_style_vars[HudStyleVarScaling].top();
		auto sz = _sz * scaling;
		item_spacing *= scaling;

		Rect rect(layout.cursor, layout.cursor + sz);
		layout.item_max = max(layout.item_max, sz);
		if (layout.type == HudHorizontal)
		{
			layout.cursor.x += sz.x + item_spacing.x;
			if (layout.auto_size)
				layout.rect.b = max(layout.rect.b, layout.cursor + vec2(0.f, layout.item_max.y));
		}
		else
		{
			if (layout.auto_size)
				layout.rect.b.x = max(layout.rect.b.x, layout.cursor.x + sz.x + item_spacing.x);
			layout.cursor.x = layout.rect.a.x;
			layout.cursor.y += sz.y + item_spacing.y;
			if (layout.auto_size)
				layout.rect.b.y = layout.cursor.y;
		}
		hud_last_rect = rect;
		return rect;
	}

	static vec2 calc_text_size(graphics::FontAtlasPtr font_atlas, uint font_size, std::wstring_view str)
	{
		auto scale = font_atlas->get_scale(font_size);
		auto p = vec2(0.f);
		auto max_x = 0.f;
		for (auto ch : str)
		{
			if (ch == L'\n')
			{
				p.y += font_size;
				p.x = 0.f;
				continue;
			}

			auto& g = font_atlas->get_glyph(ch, font_size);
			p.x += g.advance * scale;
			max_x = max(max_x, p.x);
		}
		return vec2(max_x, p.y + font_size);
	}

	void sRendererPrivate::hud_text(std::wstring_view text, uint font_size, const cvec4& col)
	{
		auto canvas = render_tasks.front()->canvas;

		auto sz = calc_text_size(canvas->default_font_atlas, font_size, text);
		auto rect = hud_add_rect(sz);
		canvas->add_text(canvas->default_font_atlas, font_size, rect.a, text, col, 0.5f, 0.2f);
	}

	void sRendererPrivate::hud_rect(const vec2& size, const cvec4& col)
	{
		auto canvas = render_tasks.front()->canvas;
		auto input = sInput::instance();

		auto rect = hud_add_rect(size);
		canvas->add_rect_filled(rect.a, rect.b, col);
	}

	void sRendererPrivate::hud_image(const vec2& size, const graphics::ImageDesc& image, const cvec4& col)
	{
		auto canvas = render_tasks.front()->canvas;
		auto input = sInput::instance();

		auto rect = hud_add_rect(size);
		canvas->add_image(image.view, rect.a, rect.b, image.uvs, col);
	}

	void sRendererPrivate::hud_image_stretched(const vec2& size, const graphics::ImageDesc& image, const vec4& border, const cvec4& col)
	{
		auto canvas = render_tasks.front()->canvas;
		auto input = sInput::instance();

		auto rect = hud_add_rect(size);
		canvas->add_image_stretched(image.view, rect.a, rect.b, image.uvs, border, image.border_uvs, col);
	}

	bool sRendererPrivate::hud_button(std::wstring_view label, uint font_size)
	{
		auto canvas = render_tasks.front()->canvas;
		auto input = sInput::instance();

		vec4 border(2.f);
		auto sz = calc_text_size(canvas->default_font_atlas, font_size, label);
		sz += border.xy() + border.zw();

		auto rect = hud_add_rect(sz);
		auto state = 0;
		if (rect.contains(input->mpos))
		{
			state = 1;
			if (input->mpressed(Mouse_Left))
				state = 2;

			input->mouse_used = true;
		}
		canvas->add_rect_filled(rect.a, rect.b, state == 0 ? cvec4(35, 69, 109, 255) : cvec4(66, 150, 250, 255));
		canvas->add_text(canvas->default_font_atlas, font_size, rect.a + border.xy(), label, cvec4(255), 0.5f, 0.2f);
		return state == 2;
	}

	bool sRendererPrivate::hud_image_button(const vec2& size, const graphics::ImageDesc& image, const vec4& border)
	{
		auto canvas = render_tasks.front()->canvas;
		auto input = sInput::instance();

		auto sz = size;
		sz += border.xy() + border.zw();

		auto rect = hud_add_rect(sz);
		auto state = 0;
		if (rect.contains(input->mpos))
		{
			state = 1;
			if (input->mpressed(Mouse_Left))
				state = 2;

			input->mouse_used = true;
		}
		if (image.view)
		{
			if (border.x > 0.f || border.y > 0.f || border.z > 0.f || border.w > 0.f)
				canvas->add_image_stretched(image.view, rect.a, rect.b, image.uvs, border, image.border_uvs, state == 0 ? cvec4(255) : cvec4(200));
			else
				canvas->add_image(image.view, rect.a, rect.b, image.uvs, state == 0 ? cvec4(255) : cvec4(200));
		}
		else
			canvas->add_rect_filled(rect.a, rect.b, state == 0 ? cvec4(35, 69, 109, 255) : cvec4(66, 150, 250, 255));
		return state == 2;
	}

	bool sRendererPrivate::hud_item_hovered()
	{
		return hud_last_rect.contains(sInput::instance()->mpos);
	}

	void sRendererPrivate::send_debug_string(const std::string& str)
	{
		if (str == "csm_debug_capture")
			csm_debug_capture_flag = !csm_debug_capture_flag;
		else if (str == "clear_csm_debug")
		{
			csm_debug_capture_flag = false;
			csm_debug_draws.clear();
		}
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
				printf("renderer system needs graphics window\n");
				return nullptr;
			}

			_instance = new sRendererPrivate(windows[0]);
			return _instance;
		}
	}sRenderer_create;
	sRenderer::Create& sRenderer::create = sRenderer_create;
}
