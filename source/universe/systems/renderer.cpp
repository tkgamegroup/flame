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
#include "../../graphics/model.h"
#include "../../graphics/extension.h"

namespace flame
{
	const graphics::Format dep_fmt = graphics::Format::Format_Depth16;
	const graphics::Format col_fmt = graphics::Format::Format_R8G8B8A8_UNORM;

	sRendererPrivate::sRendererPrivate(graphics::WindowPtr w) :
		window(w)
	{
		img_black.reset(graphics::Image::create(nullptr, graphics::Format_R8G8B8A8_UNORM, uvec2(4), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, 1, 8));
		img_white.reset(graphics::Image::create(nullptr, graphics::Format_R8G8B8A8_UNORM, uvec2(4), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, 1, 8));
		img_black->clear(vec4(0.f), graphics::ImageLayoutShaderReadOnly);
		img_white->clear(vec4(1.f), graphics::ImageLayoutShaderReadOnly);

		auto dsl_scene = graphics::DescriptorSetLayout::get(nullptr, L"flame\\shaders\\scene.dsl");
		buf_scene.create(dsl_scene->get_buf_ui("Scene"));
		ds_scene.reset(graphics::DescriptorSet::create(nullptr, dsl_scene));
		ds_scene->set_buffer("Scene", 0, buf_scene.buf.get());
		ds_scene->update();
		auto dsl_instance = graphics::DescriptorSetLayout::get(nullptr, L"flame\\shaders\\instance.dsl");
		buf_mesh_ins.create_with_array_type(dsl_instance->get_buf_ui("MeshInstances"));
		buf_armature_ins.create_with_array_type(dsl_instance->get_buf_ui("ArmatureInstances"));
		buf_terrain_ins.create_with_array_type(dsl_instance->get_buf_ui("TerrainInstances"));
		ds_instance.reset(graphics::DescriptorSet::create(nullptr, dsl_instance));
		ds_instance->set_buffer("MeshInstances", 0, buf_mesh_ins.buf.get());
		ds_instance->set_buffer("ArmatureInstances", 0, buf_armature_ins.buf.get());
		ds_instance->set_buffer("TerrainInstances", 0, buf_terrain_ins.buf.get());
		for (auto i = 0; i < buf_terrain_ins.array_capacity; i++)
			ds_instance->set_image("terrain_textures", i, img_black->get_view({ 0, 1, 0, 3 }), nullptr);
		ds_instance->update();

		mesh_reses.resize(1024);

		rp_col = graphics::Renderpass::get(nullptr, L"flame\\shaders\\color.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(&col_fmt) });
		rp_col_dep = graphics::Renderpass::get(nullptr, L"flame\\shaders\\color_depth.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(&col_fmt),
			  "dep_fmt=" + TypeInfo::serialize_t(&dep_fmt) });
		rp_fwd = graphics::Renderpass::get(nullptr, L"flame\\shaders\\forward.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(&col_fmt),
			  "dep_fmt=" + TypeInfo::serialize_t(&dep_fmt) });

		pl_blit = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\blit.pipeline",
			{ "rp=" + str(rp_col) });
		pl_add = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\add.pipeline",
			{ "rp=" + str(rp_col) });
		pl_blend = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\blend.pipeline",
			{ "rp=" + str(rp_col) });

		prm_plain3d.init(graphics::PipelineLayout::get(nullptr, L"flame\\shaders\\plain\\plain3d.pll"));
		pl_line3d = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\plain\\line3d.pipeline",
			{ "rp=" + str(rp_col) });

		buf_lines.create(pl_line3d->vi_ui(), 1024 * 32);

		prm_fwd.init(graphics::PipelineLayout::get(nullptr, L"flame\\shaders\\forward.pll"));
		prm_fwd.set_ds("scene"_h, ds_scene.get());
		prm_fwd.set_ds("instance"_h, ds_instance.get());
		pl_mesh_fwd = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\mesh\\mesh.pipeline",
			{ "rp=" + str(rp_fwd),
			  "frag:CAMERA_LIGHT" });
		pl_mesh_arm_fwd = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\mesh\\mesh.pipeline",
			{ "rp=" + str(rp_fwd),
			  "vert:ARMATURE",
			  "frag:CAMERA_LIGHT" });
		pl_terrain_fwd = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\terrain\\terrain.pipeline",
			{ "rp=" + str(rp_fwd),
			  "frag:CAMERA_LIGHT" });

		buf_vtx.create(pl_mesh_fwd->vi_ui(), 1024 * 256 * 4);
		buf_idx.create(sizeof(uint), 1024 * 256 * 6);
		buf_vtx_arm.create(pl_mesh_arm_fwd->vi_ui(), 1024 * 128 * 4);
		buf_idx_arm.create(sizeof(uint), 1024 * 128 * 6);
		buf_idr_mesh.create(0U, buf_mesh_ins.array_capacity);
		buf_idr_mesh_arm.create(0U, buf_armature_ins.array_capacity);

		set_mesh_res(-1, &graphics::Model::get(L"standard:cube")->meshes[0]);
		set_mesh_res(-1, &graphics::Model::get(L"standard:sphere")->meshes[0]);

		pl_mesh_plain = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\mesh\\mesh.pipeline",
			{ "rp=" + str(rp_col) });
		pl_mesh_arm_plain = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\mesh\\mesh.pipeline",
			{ "rp=" + str(rp_col),
			  "vert:ARMATURE" });
		pl_terrain_plain = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\terrain\\terrain.pipeline",
			{ "rp=" + str(rp_col) });

		prm_post.init(graphics::PipelineLayout::get(nullptr, L"flame\\shaders\\post\\post.pll"));
		pl_blur_h = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\post\\blur.pipeline",
			{ "rp=" + str(rp_col),
			  "frag:HORIZONTAL" });
		pl_blur_v = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\post\\blur.pipeline",
			{ "rp=" + str(rp_col),
			  "frag:VERTICAL" });
		pl_localmax_h = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\post\\blur.pipeline",
			{ "rp=" + str(rp_col),
			  "frag:LOCAL_MAX",
			  "frag:HORIZONTAL" });
		pl_localmax_v = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\post\\blur.pipeline",
			{ "rp=" + str(rp_col),
			  "frag:LOCAL_MAX",
			  "frag:VERTICAL" });

		pl_mesh_pickup = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\mesh\\mesh.pipeline",
			{ "rp=" + str(rp_col_dep),
			  "frag:PICKUP" });
		pl_mesh_arm_pickup = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\mesh\\mesh.pipeline",
			{ "rp=" + str(rp_col_dep),
			  "vert:ARMATURE",
			  "frag:PICKUP" });
		pl_terrain_pickup = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\terrain\\terrain.pipeline",
			{ "rp=" + str(rp_col_dep),
			  "frag:PICKUP" });
		fence_pickup.reset(graphics::Fence::create(nullptr, false));
		
		w->renderers.add([this](uint img_idx, graphics::CommandBufferPtr cb) {
			render(img_idx, cb);
		});
	}

	void sRendererPrivate::set_targets(std::span<graphics::ImageViewPtr> _targets, graphics::ImageLayout _final_layout)
	{
		auto img0 = _targets.front()->image;
		auto tar_size = img0->size;
		
		auto rp_tar = graphics::Renderpass::get(nullptr, L"flame\\shaders\\color.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(&img0->format) });

		if (!pl_blit_tar)
		{
			pl_blit_tar = graphics::GraphicsPipeline::get(nullptr, L"flame\\shaders\\blit.pipeline",
				{ "rp=" + str(rp_tar) });
		}

		iv_tars.assign(_targets.begin(), _targets.end());

		img_dst.reset(graphics::Image::create(nullptr, col_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_dep.reset(graphics::Image::create(nullptr, dep_fmt, tar_size, graphics::ImageUsageAttachment));
		fb_fwd.reset(graphics::Framebuffer::create(rp_fwd, { img_dst->get_view(), img_dep->get_view() }));

		final_layout = _final_layout;

		img_back0.reset(graphics::Image::create(nullptr, col_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		img_back1.reset(graphics::Image::create(nullptr, col_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));

		img_pickup.reset(graphics::Image::create(nullptr, col_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageTransferSrc));
		img_dep_pickup.reset(graphics::Image::create(nullptr, dep_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageTransferSrc));
		fb_pickup.reset(graphics::Framebuffer::create(rp_col_dep, { img_pickup->get_view(), img_dep_pickup->get_view() }));
	}

	void sRendererPrivate::bind_window_targets()
	{
		window->native->resize_listeners.add([this](const uvec2& sz) {
			graphics::Queue::get(nullptr)->wait_idle();
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

	int sRendererPrivate::set_material_res(int idx, graphics::Material* mat)
	{
		//auto opaque = mat->get_opaque();

		//if (idx == -1)
		//{
		//	auto beg = opaque ? (uint)MaterialCustom : TrnMatBase;
		//	auto end = opaque ? TrnMatBase : MaxMatCount;
		//	for (auto i = beg; i < end; i++)
		//	{
		//		if (!nd.mat_reses[i].mat)
		//		{
		//			idx = i;
		//			break;
		//		}
		//	}
		//}
		//if (idx == -1)
		//	return -1;

		//auto& dst = nd.mat_reses[idx];
		//if (dst.mat)
		//{
		//	for (auto i = 0; i < MaxMaterialTexturesCount; i++)
		//	{
		//		if (dst.texs[i] != -1)
		//			set_texture_res(dst.texs[i], nullptr, nullptr);
		//	}
		//	for (auto i = 0; i < countof(dst.pls); i++)
		//	{
		//		if (dst.pls[i])
		//			release_material_pipeline((MaterialUsage)i, dst.pls[i]);
		//	}
		//}
		//dst.mat = mat;
		//dst.opaque = mat->get_opaque();
		//dst.sort = mat->get_sort();
		//{
		//	wchar_t buf[260];
		//	mat->get_pipeline_file(buf);
		//	dst.pipeline_file = buf;
		//}
		//dst.pipeline_defines = Shader::format_defines(mat->get_pipeline_defines());
		//if (!opaque)
		//	dst.pipeline_defines.push_back("TRANSPARENT");
		//if (mat)
		//{
		//	InstanceCB cb(nullptr);

		//	auto& data = nd.buf_materials.item(idx);
		//	data.color = mat->get_color();
		//	data.metallic = mat->get_metallic();
		//	data.roughness = mat->get_roughness();
		//	auto alpha_map_id = -1;
		//	auto alpha_test = 0.f;
		//	std::string str;
		//	if (parse_define(dst.pipeline_defines, "ALPHA_TEST", str))
		//		alpha_test = std::stof(str);
		//	if (parse_define(dst.pipeline_defines, "ALPHA_MAP", str))
		//		alpha_map_id = { std::stoi(str) };
		//	else if (parse_define(dst.pipeline_defines, "COLOR_MAP", str))
		//		alpha_map_id = { std::stoi(str) };
		//	for (auto i = 0; i < MaxMaterialTexturesCount; i++)
		//	{
		//		wchar_t buf[260]; buf[0] = 0;
		//		mat->get_texture_file(i, buf);
		//		auto fn = std::filesystem::path(buf);
		//		if (fn.empty() || !std::filesystem::exists(fn))
		//		{
		//			dst.texs[i] = -1;
		//			data.map_indices[i] = -1;
		//		}
		//		else
		//		{
		//			auto srgb = mat->get_texture_srgb(i);
		//			if (mat->get_texture_auto_mipmap(i))
		//			{
		//				auto ext = fn.extension();
		//				if (ext != L".dds" && ext != L".ktx")
		//				{
		//					auto dds_fn = fn;
		//					dds_fn += L".dds";
		//					if (!std::filesystem::exists(dds_fn))
		//					{
		//						auto img = Image::create(nullptr, fn.c_str(), srgb, true, alpha_map_id == i ? alpha_test : 0.f);
		//						img->save(dds_fn.c_str());
		//						delete img;
		//					}
		//					fn = dds_fn;
		//					srgb = false;
		//				}
		//			}

		//			auto img = Image::get(nullptr, fn.c_str(), srgb);
		//			auto iv = img->get_view({ 0, img->get_levels(), 0, 1 });
		//			auto id = find_texture_res(iv);
		//			if (id == -1)
		//				id = set_texture_res(-1, iv, mat->get_texture_sampler(nullptr, i));
		//			dst.texs[i] = id;
		//			data.map_indices[i] = id;
		//		}
		//	}

		//	nd.buf_materials.upload(cb.get());
		//}

		return idx;
	}

	int sRendererPrivate::find_material_res(graphics::Material* mat) const
	{
		//for (auto i = 0; i < mat_reses.size(); i++)
		//{
		//	if (mat_reses[i].mat == mat)
		//		return i;
		//}
		return -1;
	}

	int sRendererPrivate::set_mesh_res(int idx, graphics::Mesh* mesh)
	{
		if (idx == -1)
		{
			for (auto i = 0; i < mesh_reses.size(); i++)
			{
				if (!mesh_reses[i].mesh)
				{
					idx = i;
					break;
				}
			}
		}
		if (idx == -1)
			return -1;

		auto& dst = mesh_reses[idx];
		if (dst.mesh)
		{
			// TODO: mark uploaded vertexs invalid
		}
		dst.mesh = mesh;

		if (mesh)
		{
			graphics::InstanceCB cb(nullptr);

			dst.vtx_cnt = mesh->positions.size();
			dst.idx_cnt = mesh->indices.size();
			dst.arm = !mesh->bone_ids.empty();
			if (!dst.arm)
			{
				dst.vtx_off = buf_vtx.item_offset();
				for (auto i = 0; i < dst.vtx_cnt; i++)
				{
					buf_vtx.set_var<"i_pos"_h>(mesh->positions[i]);
					if (!mesh->normals.empty())
						buf_vtx.set_var<"i_nor"_h>(mesh->normals[i]);
					if (!mesh->uvs.empty())
						buf_vtx.set_var<"i_uv"_h>(mesh->uvs[i]);
					buf_vtx.next_item();
				}

				dst.idx_off = buf_idx.item_offset();
				buf_idx.push(dst.idx_cnt, mesh->indices.data());

				buf_vtx.upload(cb.get());
				buf_idx.upload(cb.get());
			}
			else
			{
				dst.vtx_off = buf_vtx_arm.item_offset();
				for (auto i = 0; i < dst.vtx_cnt; i++)
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

				dst.idx_off = buf_idx_arm.item_offset();
				buf_idx_arm.push(dst.idx_cnt, mesh->indices.data());

				buf_vtx_arm.upload(cb.get());
				buf_idx_arm.upload(cb.get());
			}

			for (auto m : mesh->materials)
			{
				auto id = find_material_res(m);
				if (id == -1)
					id = set_material_res(id, m);
				dst.mat_ids.push_back(id);
			}
		}

		return idx;
	}

	int sRendererPrivate::find_mesh_res(graphics::Mesh* mesh) const
	{
		for (auto i = 0; i < mesh_reses.size(); i++)
		{
			if (mesh_reses[i].mesh == mesh)
				return i;
		}
		return -1;
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

	void sRendererPrivate::draw_mesh(uint instance_id, uint mesh_id, uint skin)
	{
		DrawMesh d;
		d.node = current_node;
		d.instance_id = instance_id;
		d.mesh_id = mesh_id;
		d.skin = skin;
		if (!mesh_reses[mesh_id].arm)
			draw_meshes.push_back(d);
		else
			draw_arm_meshes.push_back(d);
	}

	void sRendererPrivate::draw_mesh_occluder(uint instance_id, uint mesh_id, uint skin)
	{
		DrawMesh d;
		d.node = current_node;
		d.instance_id = instance_id;
		d.mesh_id = mesh_id;
		d.skin = skin;
		if (!mesh_reses[mesh_id].arm)
			draw_occluder_meshes.push_back(d);
		else
			draw_occluder_arm_meshes.push_back(d);
	}

	void sRendererPrivate::draw_mesh_outline(uint instance_id, uint mesh_id, const cvec4& color)
	{
		DrawMesh d;
		d.node = current_node;
		d.instance_id = instance_id;
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
		d.instance_id = instance_id;
		d.mesh_id = mesh_id;
		d.color = color;
		if (!mesh_reses[mesh_id].arm)
			draw_wireframe_meshes.push_back(d);
		else
			draw_wireframe_arm_meshes.push_back(d);
	}

	void sRendererPrivate::draw_terrain(uint instance_id, uint blocks, uint material_id)
	{
		DrawTerrain d;
		d.node = current_node;
		d.instance_id = instance_id;
		d.material_id = material_id;
		d.blocks = blocks;
		draw_terrains.push_back(d);
	}

	void sRendererPrivate::draw_terrain_outline(uint instance_id, uint blocks, const cvec4& color)
	{
		DrawTerrain d;
		d.node = current_node;
		d.instance_id = instance_id;
		d.blocks = blocks;
		d.color = color;
		draw_outline_terrains.push_back(d);
	}

	void sRendererPrivate::draw_terrain_wireframe(uint instance_id, uint blocks, const cvec4& color)
	{
		DrawTerrain d;
		d.node = current_node;
		d.instance_id = instance_id;
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

		std::vector<cNodePtr> nodes;
		sScene::instance()->octree->get_within_frustum(camera->frustum, nodes);
		current_node = nullptr;
		draw_lines.clear();
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
		for (auto& n : nodes)
		{
			current_node = n;
			n->draw(this, false);
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

		for (auto& d : draw_meshes)
		{
			auto& mr = mesh_reses[d.mesh_id];
			buf_idr_mesh.add_draw_indexed_indirect(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (d.instance_id << 8) + 0/* mat id */);
		}
		for (auto& d : draw_arm_meshes)
		{
			auto& mr = mesh_reses[d.mesh_id];
			buf_idr_mesh_arm.add_draw_indexed_indirect(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (d.instance_id << 8) + 0/* mat id */);
		}

		buf_lines.upload(cb);
		buf_mesh_ins.upload(cb);
		buf_armature_ins.upload(cb);
		buf_terrain_ins.upload(cb);
		buf_idr_mesh.upload(cb);
		buf_idr_mesh_arm.upload(cb);

		cb->set_viewport(Rect(vec2(0), sz));
		cb->set_scissor(Rect(vec2(0), sz));

		cb->begin_renderpass(nullptr, fb_fwd.get(), 
			{ vec4(0.f, 0.f, 0.f, 1.f),
			vec4(1.f, 0.f, 0.f, 0.f) });

		prm_fwd.bind_dss(cb);
		prm_fwd.set_pc_var<"f"_h>(vec4(1.f));
		prm_fwd.push_constant(cb);

		if (!draw_meshes.empty())
		{
			cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
			cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
			cb->bind_pipeline(pl_mesh_fwd);
			cb->draw_indexed_indirect(buf_idr_mesh.buf.get(), 0, draw_meshes.size());
		}
		if (!draw_arm_meshes.empty())
		{
			cb->bind_vertex_buffer(buf_vtx_arm.buf.get(), 0);
			cb->bind_index_buffer(buf_idx_arm.buf.get(), graphics::IndiceTypeUint);
			cb->bind_pipeline(pl_mesh_arm_fwd);
			cb->draw_indexed_indirect(buf_idr_mesh_arm.buf.get(), 0, draw_arm_meshes.size());
		}
		if (!draw_terrains.empty())
		{
			cb->bind_pipeline(pl_terrain_fwd);
			for (auto& d : draw_terrains)
				cb->draw(4, d.blocks, 0, d.instance_id << 24);
		}

		cb->end_renderpass();

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
					cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, d.instance_id << 8);
					cb->end_renderpass();

					blur_pass();

					cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
					prm_fwd.bind_dss(cb);
					prm_fwd.set_pc_var<"f"_h>(vec4(0.f));
					prm_fwd.push_constant(cb);
					cb->bind_pipeline(pl_mesh_plain);
					cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, d.instance_id << 8);
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
					cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, d.instance_id << 8);
					cb->end_renderpass();

					blur_pass();

					cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
					prm_fwd.bind_dss(cb);
					prm_fwd.set_pc_var<"f"_h>(vec4(0.f));
					prm_fwd.push_constant(cb);
					cb->bind_pipeline(pl_mesh_arm_plain);
					cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, d.instance_id << 8);
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
					cb->draw(4, d.blocks, 0, d.instance_id << 24);
					cb->end_renderpass();

					blur_pass();

					cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
					prm_fwd.bind_dss(cb);
					prm_fwd.set_pc_var<"f"_h>(vec4(0.f));
					prm_fwd.push_constant(cb);
					cb->bind_pipeline(pl_terrain_plain);
					cb->draw(4, d.blocks, 0, d.instance_id << 24);
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
			graphics::InstanceCB cb(nullptr, fence_pickup.get());

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
					cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, d.instance_id << 8);
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
					cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, d.instance_id << 8);
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
					cb->draw(4, d.blocks, 0, d.instance_id << 24);
				}
				off += draw_terrains.size();
			}

			cb->end_renderpass();
		}

		{
			int index; ushort depth;
			graphics::StagingBuffer sb(nullptr, sizeof(index) + sizeof(depth), nullptr, graphics::BufferUsageTransferDst);
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
