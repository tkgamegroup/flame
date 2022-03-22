#include "renderer_private.h"
#include "scene_private.h"
#include "../octree.h"
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

namespace flame
{
	const graphics::Format col_fmt = graphics::Format::Format_R8G8B8A8_UNORM;
	const graphics::Format dep_fmt = graphics::Format::Format_Depth16;
	const uvec2 shadow_map_size = uvec2(2048);

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

		auto sp_bilinear = graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, true, graphics::AddressRepeat);
		auto sp_shadow = graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressClampToBorder);

		auto dsl_scene = graphics::DescriptorSetLayout::get(L"flame\\shaders\\scene.dsl");
		buf_scene.create(dsl_scene->get_buf_ui("Scene"));
		ds_scene.reset(graphics::DescriptorSet::create(nullptr, dsl_scene));
		ds_scene->set_buffer("Scene", 0, buf_scene.buf.get());
		ds_scene->update();
		auto dsl_instance = graphics::DescriptorSetLayout::get(L"flame\\shaders\\instance.dsl");
		buf_mesh_ins.create_with_array_type(dsl_instance->get_buf_ui("MeshInstances"));
		buf_armature_ins.create_with_array_type(dsl_instance->get_buf_ui("ArmatureInstances"));
		buf_terrain_ins.create_with_array_type(dsl_instance->get_buf_ui("TerrainInstances"));
		ds_instance.reset(graphics::DescriptorSet::create(nullptr, dsl_instance));
		ds_instance->set_buffer("MeshInstances", 0, buf_mesh_ins.buf.get());
		ds_instance->set_buffer("ArmatureInstances", 0, buf_armature_ins.buf.get());
		ds_instance->set_buffer("TerrainInstances", 0, buf_terrain_ins.buf.get());
		for (auto i = 0; i < buf_terrain_ins.array_capacity; i++)
			ds_instance->set_image("terrain_textures", i, img_black->get_view({ 0, 1, 0, 3 }), sp_bilinear);
		ds_instance->update();
		auto dsl_material = graphics::DescriptorSetLayout::get(L"flame\\shaders\\material.dsl");
		buf_material.create_with_array_type(dsl_material->get_buf_ui("MaterialInfos"));
		mat_reses.resize(buf_material.array_capacity);
		tex_reses.resize(dsl_material->find_binding("material_maps")->count);
		ds_material.reset(graphics::DescriptorSet::create(nullptr, dsl_material));
		ds_material->set_buffer("MaterialInfos", 0, buf_material.buf.get());
		for (auto i = 0; i < tex_reses.size(); i++)
			ds_material->set_image("material_maps", i, img_black->get_view(), nullptr);
		ds_material->update();
		auto dsl_light = graphics::DescriptorSetLayout::get(L"flame\\shaders\\light.dsl");
		ds_light.reset(graphics::DescriptorSet::create(nullptr, dsl_light));
		buf_light_index.create_with_array_type(dsl_light->get_buf_ui("LightIndexs"));
		buf_light_grid.create_with_array_type(dsl_light->get_buf_ui("LightGrids"));
		buf_light_info.create_with_array_type(dsl_light->get_buf_ui("LightInfos"));
		buf_dir_shadow.create_with_array_type(dsl_light->get_buf_ui("DirShadows"));
		buf_pt_shadow.create_with_array_type(dsl_light->get_buf_ui("PtShadows"));
		imgs_dir_shadow.resize(dsl_light->find_binding("dir_shadow_maps")->count);
		for (auto& i : imgs_dir_shadow)
		{
			i.reset(graphics::Image::create(dep_fmt, shadow_map_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 1, 4));
			i->change_layout(graphics::ImageLayoutShaderReadOnly);
		}
		imgs_pt_shadow.resize(dsl_light->find_binding("pt_shadow_maps")->count);
		for (auto& i : imgs_pt_shadow)
		{
			i.reset(graphics::Image::create(dep_fmt, shadow_map_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled, 1, 6, graphics::SampleCount_1, true));
			i->change_layout(graphics::ImageLayoutShaderReadOnly);
		}
		ds_light->set_buffer("LightIndexs", 0, buf_light_index.buf.get());
		ds_light->set_buffer("LightGrids", 0, buf_light_grid.buf.get());
		ds_light->set_buffer("LightInfos", 0, buf_light_info.buf.get());
		ds_light->set_buffer("DirShadows", 0, buf_dir_shadow.buf.get());
		ds_light->set_buffer("PtShadows", 0, buf_pt_shadow.buf.get());
		for (auto i = 0; i < imgs_dir_shadow.size(); i++)
			ds_light->set_image("dir_shadow_maps", i, imgs_dir_shadow[i]->get_view({ 0, 1, 0, 4 }), sp_shadow);
		for (auto i = 0; i < imgs_pt_shadow.size(); i++)
			ds_light->set_image("pt_shadow_maps", i, imgs_pt_shadow[i]->get_view({ 0, 1, 0, 6 }), sp_shadow);
		ds_light->set_image("sky_box", 0, img_cube_black->get_view({ 0, 1, 0, 6 }), sp_bilinear);
		ds_light->set_image("sky_irr", 0, img_cube_black->get_view({ 0, 1, 0, 6 }), sp_bilinear);
		ds_light->set_image("sky_rad", 0, img_cube_black->get_view({ 0, 1, 0, 6 }), sp_bilinear);
		ds_light->set_image("sky_lut", 0, img_black->get_view(), nullptr);
		ds_light->update();

		mesh_reses.resize(1024);

		rp_col = graphics::Renderpass::get(L"flame\\shaders\\color.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(&col_fmt) });
		rp_col_dep = graphics::Renderpass::get(L"flame\\shaders\\color_depth.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(&col_fmt),
			  "dep_fmt=" + TypeInfo::serialize_t(&dep_fmt) });
		rp_fwd = graphics::Renderpass::get(L"flame\\shaders\\forward.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(&col_fmt),
			  "dep_fmt=" + TypeInfo::serialize_t(&dep_fmt) });
		rp_gbuf = graphics::Renderpass::get(L"flame\\shaders\\gbuffer.rp",
			{ "dep_fmt=" + TypeInfo::serialize_t(&dep_fmt) });

		prm_plain3d.init(graphics::PipelineLayout::get(L"flame\\shaders\\plain\\plain3d.pll"));
		pl_line3d = graphics::GraphicsPipeline::get(L"flame\\shaders\\plain\\line3d.pipeline",
			{ "rp=" + str(rp_col) });
		buf_lines.create(pl_line3d->vi_ui(), 1024 * 32);

		pll_fwd = graphics::PipelineLayout::get(L"flame\\shaders\\forward.pll");
		pll_gbuf = graphics::PipelineLayout::get(L"flame\\shaders\\gbuffer.pll");

		pl_blit = graphics::GraphicsPipeline::get(L"flame\\shaders\\blit.pipeline",
			{ "rp=" + str(rp_col) });
		pl_add = graphics::GraphicsPipeline::get(L"flame\\shaders\\add.pipeline",
			{ "rp=" + str(rp_col) });
		pl_blend = graphics::GraphicsPipeline::get(L"flame\\shaders\\blend.pipeline",
			{ "rp=" + str(rp_col) });

		prm_fwd.init(graphics::PipelineLayout::get(L"flame\\shaders\\forward.pll"));
		prm_fwd.set_ds("scene"_h, ds_scene.get());
		prm_fwd.set_ds("instance"_h, ds_instance.get());
		prm_fwd.set_ds("material"_h, ds_material.get());

		prm_gbuf.init(graphics::PipelineLayout::get(L"flame\\shaders\\gbuffer.pll"));
		prm_gbuf.set_ds("scene"_h, ds_scene.get());
		prm_gbuf.set_ds("instance"_h, ds_instance.get());
		prm_gbuf.set_ds("material"_h, ds_material.get());

		pl_mesh_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline",
			{ "rp=" + str(rp_col) });
		pl_mesh_arm_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline",
			{ "rp=" + str(rp_col),
			  "vert:ARMATURE" });
		pl_terrain_plain = graphics::GraphicsPipeline::get(L"flame\\shaders\\terrain\\terrain.pipeline",
			{ "rp=" + str(rp_col) });
		pl_mesh_camlit = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline",
			{ "rp=" + str(rp_fwd),
			  "frag:CAMERA_LIGHT" });
		pl_mesh_arm_camlit = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline",
			{ "rp=" + str(rp_fwd),
			  "vert:ARMATURE",
			  "frag:CAMERA_LIGHT" });
		pl_terrain_camlit = graphics::GraphicsPipeline::get(L"flame\\shaders\\terrain\\terrain.pipeline",
			{ "rp=" + str(rp_fwd),
			  "frag:CAMERA_LIGHT" });

		buf_vtx.create(pl_mesh_plain->vi_ui(), 1024 * 256 * 4);
		buf_idx.create(sizeof(uint), 1024 * 256 * 6);
		buf_vtx_arm.create(pl_mesh_arm_plain->vi_ui(), 1024 * 128 * 4);
		buf_idx_arm.create(sizeof(uint), 1024 * 128 * 6);
		buf_idr_mesh.create(0U, buf_mesh_ins.array_capacity);

		pl_deferred = graphics::GraphicsPipeline::get(L"flame\\shaders\\deferred.pipeline",
			{ "rp=" + str(rp_col) });
		prm_deferred.init(pl_deferred->layout);
		prm_deferred.set_ds("scene"_h, ds_scene.get());
		prm_deferred.set_ds("light"_h, ds_light.get());
		ds_deferred.reset(graphics::DescriptorSet::create(nullptr, prm_deferred.pll->dsls.back()));
		ds_deferred->update();
		prm_deferred.set_ds(""_h, ds_deferred.get());

		prm_post.init(graphics::PipelineLayout::get(L"flame\\shaders\\post\\post.pll"));
		pl_blur_h = graphics::GraphicsPipeline::get(L"flame\\shaders\\post\\blur.pipeline",
			{ "rp=" + str(rp_col),
			  "frag:HORIZONTAL" });
		pl_blur_v = graphics::GraphicsPipeline::get(L"flame\\shaders\\post\\blur.pipeline",
			{ "rp=" + str(rp_col),
			  "frag:VERTICAL" });
		pl_localmax_h = graphics::GraphicsPipeline::get(L"flame\\shaders\\post\\blur.pipeline",
			{ "rp=" + str(rp_col),
			  "frag:LOCAL_MAX",
			  "frag:HORIZONTAL" });
		pl_localmax_v = graphics::GraphicsPipeline::get(L"flame\\shaders\\post\\blur.pipeline",
			{ "rp=" + str(rp_col),
			  "frag:LOCAL_MAX",
			  "frag:VERTICAL" });

		pl_mesh_pickup = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline",
			{ "rp=" + str(rp_col_dep),
			  "frag:PICKUP" });
		pl_mesh_arm_pickup = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline",
			{ "rp=" + str(rp_col_dep),
			  "vert:ARMATURE",
			  "frag:PICKUP" });
		pl_terrain_pickup = graphics::GraphicsPipeline::get(L"flame\\shaders\\terrain\\terrain.pipeline",
			{ "rp=" + str(rp_col_dep),
			  "frag:PICKUP" });
		fence_pickup.reset(graphics::Fence::create(false));
		
		w->renderers.add([this](uint img_idx, graphics::CommandBufferPtr cb) {
			render(img_idx, cb);
		});
	}

	void sRendererPrivate::set_targets(std::span<graphics::ImageViewPtr> _targets, graphics::ImageLayout _final_layout)
	{
		graphics::Queue::get()->wait_idle();

		auto img0 = _targets.front()->image;
		auto tar_size = img0->size;
		
		auto rp_tar = graphics::Renderpass::get(L"flame\\shaders\\color.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(&img0->format) });

		if (!pl_blit_tar)
		{
			pl_blit_tar = graphics::GraphicsPipeline::get(L"flame\\shaders\\blit.pipeline",
				{ "rp=" + str(rp_tar) });
		}

		iv_tars.assign(_targets.begin(), _targets.end());

		img_dst.reset(graphics::Image::create(col_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_dep.reset(graphics::Image::create(dep_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_col_met.reset(graphics::Image::create(col_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_nor_rou.reset(graphics::Image::create(col_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_ao.reset(graphics::Image::create(graphics::Format_R16_UNORM, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		fb_fwd.reset(graphics::Framebuffer::create(rp_fwd, { img_dst->get_view(), img_dep->get_view() }));
		fb_gbuf.reset(graphics::Framebuffer::create(rp_gbuf, { img_col_met->get_view(), img_nor_rou->get_view(), img_dep->get_view()}));
		ds_deferred->set_image("img_col_met", 0, img_col_met->get_view(), nullptr);
		ds_deferred->set_image("img_nor_rou", 0, img_nor_rou->get_view(), nullptr);
		ds_deferred->set_image("img_ao", 0, img_ao->get_view(), nullptr);
		ds_deferred->set_image("img_dep", 0, img_dep->get_view(), nullptr);
		ds_deferred->update();

		final_layout = _final_layout;

		img_back0.reset(graphics::Image::create(col_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_back1.reset(graphics::Image::create(col_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));

		img_pickup.reset(graphics::Image::create(col_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageTransferSrc));
		img_dep_pickup.reset(graphics::Image::create(dep_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageTransferSrc));
		fb_pickup.reset(graphics::Framebuffer::create(rp_col_dep, { img_pickup->get_view(), img_dep_pickup->get_view() }));
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

	int sRendererPrivate::get_texture_res(graphics::ImageViewPtr iv, graphics::SamplerPtr sp)
	{
		auto id = -1;
		for (auto i = 0; i < tex_reses.size(); i++)
		{
			auto& res = tex_reses[i];
			if (res.iv == iv && res.sp == sp)
			{
				res.ref++;
				id = i;
				break;
			}
		}
		if (id == -1)
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
		if (id == -1)
			return -1;

		auto& res = tex_reses[id];
		res.iv = iv;
		res.sp = sp;
		res.ref = 1;

		ds_material->set_image("material_maps", id, iv, sp);
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
			ds_material->set_image("material_maps", id, img_black->get_view(), nullptr);
			ds_material->update();
			
			res.iv = nullptr;
			res.ref = 0;
		}
		else
			res.ref--;
	}

	int sRendererPrivate::get_mesh_res(graphics::Mesh* mesh)
	{
		auto id = -1;
		for (auto i = 0; i < mesh_reses.size(); i++)
		{
			auto& res = mesh_reses[i];
			if (res.mesh == mesh)
			{
				res.ref++;
				id = i;
				break;
			}
		}
		if (id == -1)
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
		if (id == -1)
			return -1;

		auto& res = mesh_reses[id];
		res.mesh = mesh;
		res.ref = 1;

		graphics::InstanceCB cb(nullptr);

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

	int sRendererPrivate::get_material_res(graphics::Material* mat)
	{
		auto id = -1;
		for (auto i = 0; i < mat_reses.size(); i++)
		{
			auto& res = mat_reses[i];
			if (res.mat == mat)
			{
				res.ref++;
				id = i;
				break;
			}
		}
		if (id == -1)
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
		if (id == -1)
			return -1;

		auto& res = mat_reses[id];
		res.mat = mat;
		res.ref = 1;

		graphics::InstanceCB cb(nullptr);

		buf_material.select_item(id);
		buf_material.set_var<"opaque"_h>((int)mat->opaque);
		buf_material.set_var<"color"_h>(mat->color);
		buf_material.set_var<"metallic"_h>(mat->metallic);
		buf_material.set_var<"roughness"_h>(mat->roughness);
		buf_material.set_var<"alpha_test"_h>(mat->alpha_test);
		buf_material.set_var<"f"_h>(mat->float_values);
		buf_material.set_var<"i"_h>(mat->int_values);

		res.texs.resize(countof(mat->textures));
		for (auto i = 0; i < res.texs.size(); i++)
		{
			res.texs[i].first = -1;
			res.texs[i].second = nullptr;
			auto& src = mat->textures[i];
			if (!src.filename.empty())
			{
				auto image = graphics::Image::get(src.filename, src.srgb, { src.auto_mipmap, i == mat->alpha_map ? mat->alpha_test : 0.f });
				if (image)
				{
					res.texs[i].second = image;
					res.texs[i].first = get_texture_res(image->get_view({ 0, image->n_levels, 0, image->n_layers }),
						graphics::Sampler::get(src.mag_filter, src.min_filter, src.linear_mipmap, src.address_mode));
				}
			}
		}
		auto ids = (int*)buf_material.var_addr<"map_indices"_h>();
		for (auto i = 0; i < res.texs.size(); i++)
			ids[i] = res.texs[i].first;

		buf_material.upload(cb.get());

		return id;
	}

	void sRendererPrivate::release_material_res(uint id)
	{
		auto& res = mat_reses[id];
		if (res.ref == 1)
		{
			for (auto& tex : res.texs)
			{
				if (tex.first != -1)
				{
					release_texture_res(tex.first);
					graphics::Image::release(tex.second);
					tex.second = nullptr;
				}
			}
			for (auto& pl : res.pls)
				graphics::GraphicsPipeline::release(pl.second);
		}
		else
			res.ref--;
	}

	graphics::GraphicsPipelinePtr sRendererPrivate::get_material_pipeline(MatRes& mr, uint hash)
	{
		auto it = mr.pls.find(hash);
		if (it != mr.pls.end())
			return it->second;

		std::vector<std::string> defines;
		defines.push_back("rp=" + str(rp_gbuf));
		defines.push_back("pll=" + str(pll_gbuf));
		defines.push_back("all_shader:DEFERRED");
		auto mat_file = Path::get(mr.mat->shader_file).string();
		defines.push_back(std::format("frag:MAT_FILE={}", mat_file));
		if (mr.mat->color_map != -1)
			defines.push_back(std::format("frag:COLOR_MAP={}", mr.mat->color_map));
		defines.insert(defines.end(), mr.mat->shader_defines.begin(), mr.mat->shader_defines.end());

		graphics::GraphicsPipelinePtr ret = nullptr;
		switch (hash)
		{
		case "Mesh"_h:
			ret = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline", defines);
			break;
		case "Terrain"_h:
			ret = graphics::GraphicsPipeline::get(L"flame\\shaders\\terrain\\terrain.pipeline", defines);
			break;
		}
		if (ret)
			mr.pls[hash] = ret;
		return ret;
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
			ds_instance->set_image("terrain_textures", id, img_black->get_view({ 0, 1, 0, 3 }), nullptr);
			ds_instance->update();
		}
		return id;
	}

	void sRendererPrivate::set_terrain_instance(uint id, const mat4& mat, const vec3& extent, const uvec2& blocks, uint tess_level, graphics::ImageViewPtr textures)
	{
		buf_terrain_ins.select_item(id);
		buf_terrain_ins.set_var<"mat"_h>(mat);
		buf_terrain_ins.set_var<"extent"_h>(extent);
		buf_terrain_ins.set_var<"blocks"_h>(blocks);
		buf_terrain_ins.set_var<"tess_level"_h>(tess_level);
		ds_instance->set_image("terrain_textures", id, textures, nullptr);
		ds_instance->update();
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

	void sRendererPrivate::add_light(uint instance_id, LightType type, const vec3& pos, const vec3& color, float range, bool cast_shadow)
	{
		buf_light_info.select_item(instance_id);
		buf_light_info.set_var<"type"_h>((int)type);
		buf_light_info.set_var<"pos"_h>(pos);
		buf_light_info.set_var<"color"_h>(color);
		buf_light_info.set_var<"shadow_index"_h>(-1);

		switch (type)
		{
		case LightDirectional:
		{
			DirectionalLight l;
			l.node = current_node;
			l.ins_id = instance_id;
			l.dir = pos;
			l.color = color;
			l.range = range;
			dir_lights.push_back(l);
		}
			break;
		}
	}

	void sRendererPrivate::draw_line(const vec3* points, uint count, const cvec4& color)
	{
		DrawLine d;
		d.node = current_node;
		d.offset = buf_lines.item_offset();
		d.count = count;
		d.color = color;
		draw_lines.push_back(d);

		for (auto i = 0; i < count; i++)
		{
			buf_lines.set_var<"i_pos"_h>(points[i]);
			buf_lines.next_item();
		}
	}

	void sRendererPrivate::draw_mesh(uint instance_id, uint mesh_id, uint mat_id)
	{
		DrawMesh d;
		d.node = current_node;
		d.ins_id = instance_id;
		d.mesh_id = mesh_id;
		d.mat_id = mat_id;
		if (!mesh_reses[mesh_id].arm)
			draw_meshes.push_back(d);
		else
			draw_arm_meshes.push_back(d);
	}

	void sRendererPrivate::draw_mesh_occluder(uint instance_id, uint mesh_id, uint mat_id)
	{
		DrawMesh d;
		d.node = current_node;
		d.ins_id = instance_id;
		d.mesh_id = mesh_id;
		d.mat_id = mat_id;
		if (!mesh_reses[mesh_id].arm)
			draw_occluder_meshes.push_back(d);
		else
			draw_occluder_arm_meshes.push_back(d);
	}

	void sRendererPrivate::draw_mesh_outline(uint instance_id, uint mesh_id, const cvec4& color)
	{
		DrawMesh d;
		d.node = current_node;
		d.ins_id = instance_id;
		d.mesh_id = mesh_id;
		d.color = color;
		if (!mesh_reses[mesh_id].arm)
			draw_outline_meshes.push_back(d);
		else
			draw_outline_arm_meshes.push_back(d);
	}

	void sRendererPrivate::draw_mesh_wireframe(uint instance_id, uint mesh_id, const cvec4& color)
	{
		DrawMesh d;
		d.node = current_node;
		d.ins_id = instance_id;
		d.mesh_id = mesh_id;
		d.color = color;
		if (!mesh_reses[mesh_id].arm)
			draw_wireframe_meshes.push_back(d);
		else
			draw_wireframe_arm_meshes.push_back(d);
	}

	void sRendererPrivate::draw_terrain(uint instance_id, uint blocks, uint mat_id)
	{
		DrawTerrain d;
		d.node = current_node;
		d.ins_id = instance_id;
		d.mat_id = mat_id;
		d.blocks = blocks;
		draw_terrains.push_back(d);
	}

	void sRendererPrivate::draw_terrain_outline(uint instance_id, uint blocks, const cvec4& color)
	{
		DrawTerrain d;
		d.node = current_node;
		d.ins_id = instance_id;
		d.blocks = blocks;
		d.color = color;
		draw_outline_terrains.push_back(d);
	}

	void sRendererPrivate::draw_terrain_wireframe(uint instance_id, uint blocks, const cvec4& color)
	{
		DrawTerrain d;
		d.node = current_node;
		d.ins_id = instance_id;
		d.blocks = blocks;
		d.color = color;
		draw_wireframe_terrains.push_back(d);
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
		if (camera == INVALID_POINTER)
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

		draw_meshes.clear();
		draw_arm_meshes.clear();
		draw_occluder_meshes.clear();
		draw_occluder_arm_meshes.clear();
		draw_outline_meshes.clear();
		draw_outline_arm_meshes.clear();
		draw_wireframe_meshes.clear();
		draw_wireframe_arm_meshes.clear();
		draw_terrains.clear();
		draw_outline_terrains.clear();
		draw_wireframe_terrains.clear();
		draw_lines.clear();
		dir_lights.clear();
		std::vector<cNodePtr> nodes;
		sScene::instance()->octree->get_within_frustum(camera->frustum, nodes);
		current_node = nullptr;
		for (auto& n : nodes)
		{
			current_node = n;
			n->draw(this, "Mesh"_h);
		}
		current_node = nullptr;
		for (auto& n : nodes)
		{
			current_node = n;
			n->draw(this, "Light"_h);
		}

		buf_scene.set_var<"zNear"_h>(camera->zNear);
		buf_scene.set_var<"zFar"_h>(camera->zFar);

		buf_scene.set_var<"camera_coord"_h>(camera->node->g_pos);
		buf_scene.set_var<"camera_dir"_h>(-camera->node->g_rot[2]);

		buf_scene.set_var<"view"_h>(camera->view_mat);
		buf_scene.set_var<"view_inv"_h>(camera->view_mat_inv);
		buf_scene.set_var<"proj"_h>(camera->proj_mat);
		buf_scene.set_var<"proj_inv"_h>(camera->proj_mat_inv);
		buf_scene.set_var<"proj_view"_h>(camera->proj_view_mat);
		memcpy(buf_scene.var_addr<"frustum_planes"_h>(), camera->frustum.planes, sizeof(vec4) * 6);
		
		buf_scene.upload(cb);

		switch (type)
		{
		case Shaded:
		{
			for (auto i = 0; i < draw_meshes.size(); i++)
			{
				auto& d = draw_meshes[i];
				auto& mesh_r = mesh_reses[d.mesh_id];
				auto& mat_r = mat_reses[d.mat_id];
				if (mat_r.draw_ids.empty())
				{
					if (mat_r.mat->opaque)
						opaque_draw_meshes.push_back(d.mat_id);
					else
						transparent_draw_meshes.push_back(d.mat_id);
				}
				mat_r.draw_ids.push_back(i);
			}
			for (auto& d : draw_arm_meshes)
			{

			}
			for (auto mid : opaque_draw_meshes)
			{
				auto& mat_r = mat_reses[mid];
				for (auto i : mat_r.draw_ids)
				{
					auto& d = draw_meshes[i];
					auto& mesh_r = mesh_reses[d.mesh_id];
					buf_idr_mesh.add_draw_indexed_indirect(mesh_r.idx_cnt, mesh_r.idx_off, mesh_r.vtx_off, 1, (d.ins_id << 8) + mid);
				}
			}

			auto lit_idx_off = 0;
			for (auto& l : dir_lights)
			{
				buf_light_index.push(1, &l.ins_id);
				lit_idx_off++;
			}
			buf_light_grid.set_var<"offset"_h>(0);
			buf_light_grid.set_var<"count"_h>(dir_lights.size());
			buf_light_grid.next_item();
			auto cx = max(1U, uint(sz.x / 16.f));
			auto cy = max(1U, uint(sz.y / 16.f));
			for (auto y = 0; y < cy; y++)
			{
				for (auto x = 0; x < cx; x++)
				{

				}
			}
		}
			break;
		case CameraLight:
			for (auto& d : draw_meshes)
			{
				auto& mr = mesh_reses[d.mesh_id];
				buf_idr_mesh.add_draw_indexed_indirect(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, d.ins_id << 8);
			}
			for (auto& d : draw_arm_meshes)
			{
				auto& mr = mesh_reses[d.mesh_id];
				buf_idr_mesh.add_draw_indexed_indirect(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, d.ins_id << 8);
			}
			break;
		}

		buf_mesh_ins.upload(cb);
		buf_armature_ins.upload(cb);
		buf_terrain_ins.upload(cb);
		buf_idr_mesh.upload(cb);
		buf_lines.upload(cb);
		buf_light_index.upload(cb);
		buf_light_grid.upload(cb);
		buf_light_info.upload(cb);

		cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutAttachment);

		switch (type)
		{
		case Shaded:
		{
			cb->set_viewport(Rect(vec2(0), sz));
			cb->set_scissor(Rect(vec2(0), sz));

			cb->begin_renderpass(nullptr, fb_gbuf.get(),
				{ vec4(0.f, 0.f, 0.f, 1.f),
				vec4(0.f, 0.f, 0.f, 1.f),
				vec4(1.f, 0.f, 0.f, 0.f) });

			prm_gbuf.bind_dss(cb);

			if (!draw_meshes.empty())
			{
				cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
				cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
				auto idr_off = 0;
				for (auto mid : opaque_draw_meshes)
				{
					auto& mr = mat_reses[mid];
					auto num = mr.draw_ids.size();
					mr.draw_ids.clear();
					cb->bind_pipeline(get_material_pipeline(mr, "Mesh"_h));
					cb->draw_indexed_indirect(buf_idr_mesh.buf.get(), idr_off, num);
					idr_off += num;
				}
				opaque_draw_meshes.clear();
			}
			if (!draw_arm_meshes.empty())
			{
				cb->bind_vertex_buffer(buf_vtx_arm.buf.get(), 0);
				cb->bind_index_buffer(buf_idx_arm.buf.get(), graphics::IndiceTypeUint);
			}
			if (!draw_terrains.empty())
			{
				for (auto& d : draw_terrains)
				{
					auto& mr = mat_reses[d.mat_id];
					cb->bind_pipeline(get_material_pipeline(mr, "Terrain"_h));
					cb->draw(4, d.blocks, 0, (d.ins_id << 24) + (d.mat_id << 16));
				}
			}

			cb->end_renderpass();

			cb->begin_renderpass(nullptr, img_ao->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), { vec4(1.f)});
			cb->end_renderpass();

			cb->image_barrier(img_col_met.get(), {}, graphics::ImageLayoutShaderReadOnly);
			cb->image_barrier(img_nor_rou.get(), {}, graphics::ImageLayoutShaderReadOnly);
			cb->image_barrier(img_ao.get(), {}, graphics::ImageLayoutShaderReadOnly);
			cb->image_barrier(img_dep.get(), {}, graphics::ImageLayoutShaderReadOnly);
			cb->begin_renderpass(nullptr, img_dst->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
			prm_deferred.bind_dss(cb);
			cb->bind_pipeline(pl_deferred);
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();
		}
			break;
		case CameraLight:
		{
			cb->set_viewport(Rect(vec2(0), sz));
			cb->set_scissor(Rect(vec2(0), sz));

			cb->begin_renderpass(nullptr, fb_fwd.get(),
				{ vec4(0.f, 0.f, 0.f, 1.f),
				vec4(1.f, 0.f, 0.f, 0.f) });

			prm_fwd.bind_dss(cb);
			prm_fwd.set_pc_var<"f"_h>(vec4(1.f));
			prm_fwd.push_constant(cb);

			auto idr_off = 0;
			if (!draw_meshes.empty())
			{
				cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
				cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
				cb->bind_pipeline(pl_mesh_camlit);
				cb->draw_indexed_indirect(buf_idr_mesh.buf.get(), idr_off, draw_meshes.size());
				idr_off += draw_meshes.size();
			}
			if (!draw_arm_meshes.empty())
			{
				cb->bind_vertex_buffer(buf_vtx_arm.buf.get(), 0);
				cb->bind_index_buffer(buf_idx_arm.buf.get(), graphics::IndiceTypeUint);
				cb->bind_pipeline(pl_mesh_arm_camlit);
				cb->draw_indexed_indirect(buf_idr_mesh.buf.get(), idr_off, draw_arm_meshes.size());
				idr_off += draw_arm_meshes.size();
			}
			if (!draw_terrains.empty())
			{
				cb->bind_pipeline(pl_terrain_camlit);
				for (auto& d : draw_terrains)
					cb->draw(4, d.blocks, 0, d.ins_id << 24);
			}

			cb->end_renderpass();
		}
			break;
		}

		if (!draw_outline_meshes.empty() || !draw_outline_arm_meshes.empty() || !draw_terrains.empty())
		{
			cb->set_viewport(Rect(vec2(0), sz));
			cb->set_scissor(Rect(vec2(0), sz));

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

			if (!draw_outline_meshes.empty())
			{
				cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
				cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
				for (auto& d : draw_outline_meshes)
				{
					auto& mr = mesh_reses[d.mesh_id];

					cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), { vec4(0.f) });
					prm_fwd.bind_dss(cb);
					prm_fwd.set_pc_var<"f"_h>(vec4(d.color) / 255.f);
					prm_fwd.push_constant(cb);
					cb->bind_pipeline(pl_mesh_plain);
					cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, d.ins_id << 8);
					cb->end_renderpass();

					blur_pass();

					cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
					prm_fwd.bind_dss(cb);
					prm_fwd.set_pc_var<"f"_h>(vec4(0.f));
					prm_fwd.push_constant(cb);
					cb->bind_pipeline(pl_mesh_plain);
					cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, d.ins_id << 8);
					cb->end_renderpass();

					blend_pass();
				}
			}
			if (!draw_outline_arm_meshes.empty())
			{
				cb->bind_vertex_buffer(buf_vtx_arm.buf.get(), 0);
				cb->bind_index_buffer(buf_idx_arm.buf.get(), graphics::IndiceTypeUint);
				for (auto& d : draw_outline_arm_meshes)
				{
					auto& mr = mesh_reses[d.mesh_id];

					cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), { vec4(0.f) });
					prm_fwd.bind_dss(cb);
					prm_fwd.set_pc_var<"f"_h>(vec4(d.color) / 255.f);
					prm_fwd.push_constant(cb);
					cb->bind_pipeline(pl_mesh_arm_plain);
					cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, d.ins_id << 8);
					cb->end_renderpass();

					blur_pass();

					cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
					prm_fwd.bind_dss(cb);
					prm_fwd.set_pc_var<"f"_h>(vec4(0.f));
					prm_fwd.push_constant(cb);
					cb->bind_pipeline(pl_mesh_arm_plain);
					cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, d.ins_id << 8);
					cb->end_renderpass();

					blend_pass();
				}
			}
			if (!draw_outline_terrains.empty())
			{
				for (auto& d : draw_outline_terrains)
				{
					cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), { vec4(0.f) });
					prm_fwd.bind_dss(cb);
					prm_fwd.set_pc_var<"f"_h>(vec4(d.color) / 255.f);
					prm_fwd.push_constant(cb);
					cb->bind_pipeline(pl_terrain_plain);
					cb->draw(4, d.blocks, 0, d.ins_id << 24);
					cb->end_renderpass();

					blur_pass();

					cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
					prm_fwd.bind_dss(cb);
					prm_fwd.set_pc_var<"f"_h>(vec4(0.f));
					prm_fwd.push_constant(cb);
					cb->bind_pipeline(pl_terrain_plain);
					cb->draw(4, d.blocks, 0, d.ins_id << 24);
					cb->end_renderpass();

					blend_pass();
				}
			}
		}

		if (!draw_lines.empty())
		{
			cb->set_viewport(Rect(vec2(0), sz));
			cb->set_scissor(Rect(vec2(0), sz));

			cb->begin_renderpass(nullptr, img_dst->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
			cb->bind_vertex_buffer(buf_lines.buf.get(), 0);
			cb->bind_pipeline(pl_line3d);
			prm_plain3d.set_pc_var<"mvp"_h>(camera->proj_view_mat);
			for (auto& d : draw_lines)
			{
				prm_plain3d.set_pc_var<"col"_h>(vec4(d.color) / 255.f);
				prm_plain3d.push_constant(cb);
				cb->draw(d.count, 1, d.offset, 0);
			}
			cb->end_renderpass();
		}

		cb->image_barrier(img, iv->sub, graphics::ImageLayoutAttachment);
		cb->image_barrier(img_dst.get(), {}, graphics::ImageLayoutShaderReadOnly);
		cb->begin_renderpass(nullptr, img->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
		cb->bind_pipeline(pl_blit_tar);
		cb->bind_descriptor_set(0, img_dst->get_shader_read_src());
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();
		cb->image_barrier(img, iv->sub, final_layout);
	}

	void sRendererPrivate::update()
	{
	}

	cNodePtr sRendererPrivate::pick_up(const uvec2& screen_pos, vec3* out_pos)
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

		{
			graphics::InstanceCB cb(fence_pickup.get());

			cb->set_viewport(Rect(vec2(0), sz));
			cb->set_scissor(Rect(vec2(screen_pos), vec2(screen_pos + 1U)));

			cb->begin_renderpass(nullptr, fb_pickup.get(), { vec4(0.f),
				vec4(1.f, 0.f, 0.f, 0.f) });

			auto off = 1;
			prm_fwd.bind_dss(cb.get());
			if (!draw_meshes.empty())
			{
				cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
				cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
				cb->bind_pipeline(pl_mesh_pickup);
				for (auto i = 0; i < draw_meshes.size(); i++)
				{
					auto& d = draw_meshes[i];
					prm_fwd.set_pc_var<"i"_h>(ivec4(i + off, 0, 0, 0));
					prm_fwd.push_constant(cb.get());
					auto& mr = mesh_reses[d.mesh_id];
					cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, d.ins_id << 8);
				}
				off += draw_meshes.size();
			}
			if (!draw_arm_meshes.empty())
			{
				cb->bind_vertex_buffer(buf_vtx_arm.buf.get(), 0);
				cb->bind_index_buffer(buf_idx_arm.buf.get(), graphics::IndiceTypeUint);
				cb->bind_pipeline(pl_mesh_arm_pickup);
				for (auto i = 0; i < draw_arm_meshes.size(); i++)
				{
					auto& d = draw_arm_meshes[i];
					prm_fwd.set_pc_var<"i"_h>(ivec4(i + off, 0, 0, 0));
					prm_fwd.push_constant(cb.get());
					auto& mr = mesh_reses[d.mesh_id];
					cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, d.ins_id << 8);
				}
				off += draw_arm_meshes.size();
			}
			if (!draw_terrains.empty())
			{
				cb->bind_pipeline(pl_terrain_pickup);
				for (auto i = 0; i < draw_terrains.size(); i++)
				{
					auto& d = draw_terrains[i];
					prm_fwd.set_pc_var<"i"_h>(ivec4(i + off, 0, 0, 0));
					prm_fwd.push_constant(cb.get());
					cb->draw(4, d.blocks, 0, d.ins_id << 24);
				}
				off += draw_terrains.size();
			}

			cb->end_renderpass();
		}

		{
			int index; ushort depth;
			graphics::StagingBuffer sb(sizeof(index) + sizeof(depth), nullptr, graphics::BufferUsageTransferDst);
			{
				graphics::InstanceCB cb(nullptr);
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
			if (index < draw_meshes.size())
				return draw_meshes[index].node;
			index -= draw_meshes.size();
			if (index < draw_arm_meshes.size())
				return draw_arm_meshes[index].node;
			index -= draw_arm_meshes.size();
			if (index < draw_terrains.size())
				return draw_terrains[index].node;
			return nullptr;
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
				printf("node renderer system needs graphics window\n");
				return nullptr;
			}

			_instance = new sRendererPrivate(windows[0]);
			return _instance;
		}
	}sRenderer_create;
	sRenderer::Create& sRenderer::create = sRenderer_create;
}
