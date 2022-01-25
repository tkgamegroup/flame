#include "node_renderer_private.h"
#include "scene_private.h"
#include "../octree.h"
#include "../world_private.h"
#include "../components/node_private.h"
#include "../components/camera_private.h"

#include "../../foundation/typeinfo.h"
#include "../../foundation/typeinfo_serialize.h"
#include "../../graphics/renderpass.h"
#include "../../graphics/shader.h"
#include "../../graphics/window.h"
#include "../../graphics/model.h"
#include "../../graphics/extension.h"

namespace flame
{
	const graphics::Format dep_fmt = graphics::Format::Format_Depth16;
	const graphics::Format pickup_fmt = graphics::Format::Format_R8G8B8A8_UNORM;

	sNodeRendererPrivate::sNodeRendererPrivate(graphics::WindowPtr w)
	{
		auto dsl_scene = graphics::DescriptorSetLayout::get(nullptr, L"default_assets\\shaders\\scene.dsl");
		buf_scene.create(dsl_scene->get_buf_ui("Scene"));
		ds_scene.reset(graphics::DescriptorSet::create(nullptr, dsl_scene));
		ds_scene->set_buffer("Scene", 0, buf_scene.buf.get());
		ds_scene->update();
		auto dsl_object = graphics::DescriptorSetLayout::get(nullptr, L"default_assets\\shaders\\object.dsl");
		buf_objects.create_with_array_type(dsl_object->get_buf_ui("Objects"));
		ds_object.reset(graphics::DescriptorSet::create(nullptr, dsl_object));
		ds_object->set_buffer("Objects", 0, buf_objects.buf.get());
		ds_object->update();

		mesh_reses.resize(1024);
		
		w->renderers.add([this](uint img_idx, graphics::CommandBufferPtr cb) {
			render(img_idx, cb);
		});
	}

	void sNodeRendererPrivate::set_targets(std::span<graphics::ImageViewPtr> targets, graphics::ImageLayout _dst_layout)
	{
		auto img0 = targets.front()->image;
		auto size = img0->size;
		
		auto rp_fwd = graphics::Renderpass::get(nullptr, L"default_assets\\shaders\\forward.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(&img0->format),
			  "dep_fmt=" + TypeInfo::serialize_t(&dep_fmt) });
		auto rp_pickup = graphics::Renderpass::get(nullptr, L"default_assets\\shaders\\forward.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(&pickup_fmt),
			  "dep_fmt=" + TypeInfo::serialize_t(&dep_fmt) });

		if (!initialized)
		{
			std::filesystem::path mesh_pl_path = L"default_assets\\shaders\\mesh\\mesh.pipeline";
			pl_mesh_fwd = graphics::GraphicsPipeline::get(nullptr, mesh_pl_path,
				{ "rp=" + str(rp_fwd),
				  "frag:CAMERA_LIGHT" });
			pl_mesh_pickup = graphics::GraphicsPipeline::get(nullptr, mesh_pl_path,
				{ "rp=" + str(rp_pickup),
				  "frag:PICKUP" });

			buf_vtx.create(pl_mesh_fwd->vi_ui(), 1024 * 128 * 4);
			buf_idx.create(sizeof(uint), 1024 * 128 * 6);
			prm_mesh_fwd.init(pl_mesh_fwd->layout);
			buf_idr_mesh.create(0U, buf_objects.array_capacity);

			fence_pickup.reset(graphics::Fence::create(nullptr, false));

			set_mesh_res(-1, &graphics::Model::get(L"standard:cube")->meshes[0]);
			set_mesh_res(-1, &graphics::Model::get(L"standard:sphere")->meshes[0]);

			initialized = true;
		}

		iv_tars.assign(targets.begin(), targets.end());

		img_dep.reset(graphics::Image::create(nullptr, dep_fmt, size, 1, 1, graphics::SampleCount_1, graphics::ImageUsageAttachment));
		auto iv_dep = img_dep->get_view();
		fbs_fwd.clear();
		for (auto iv : targets)
		{
			graphics::ImageViewPtr ivs[] = { iv, iv_dep };
			fbs_fwd.emplace_back(graphics::Framebuffer::create(rp_fwd, ivs));
		}

		dst_layout = _dst_layout;

		img_pickup.reset(graphics::Image::create(nullptr, pickup_fmt, size, 1, 1, graphics::SampleCount_1, graphics::ImageUsageAttachment | graphics::ImageUsageTransferSrc));
		img_dep_pickup.reset(graphics::Image::create(nullptr, dep_fmt, size, 1, 1, graphics::SampleCount_1, graphics::ImageUsageAttachment));
		graphics::ImageViewPtr vs[] = { img_pickup->get_view(), img_dep_pickup->get_view() };
		fb_pickup.reset(graphics::Framebuffer::create(rp_pickup, vs));
	}

	int sNodeRendererPrivate::set_material_res(int idx, graphics::Material* mat)
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
		//	auto alpha_map_id = std::make_pair(-1, 0);
		//	auto alpha_test = 0.f;
		//	std::string str;
		//	if (parse_define(dst.pipeline_defines, "ALPHA_TEST", str))
		//		alpha_test = std::stof(str);
		//	if (parse_define(dst.pipeline_defines, "ALPHA_MAP", str))
		//		alpha_map_id = { std::stoi(str), 0 };
		//	else if (parse_define(dst.pipeline_defines, "COLOR_MAP", str))
		//		alpha_map_id = { std::stoi(str), 3 };
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
		//						auto img = Image::get(nullptr, fn.c_str(), srgb);
		//						img->generate_mipmaps();
		//						if (alpha_test > 0.f && alpha_map_id.first == i)
		//						{
		//							auto coverage = img->alpha_test_coverage(0, alpha_test, alpha_map_id.second, 1.f);
		//							auto lvs = img->get_levels();
		//							for (auto i = 1; i < lvs; i++)
		//								img->scale_alpha_to_coverage(i, coverage, alpha_test, alpha_map_id.second);
		//						}
		//						img->save(dds_fn.c_str());
		//						img->release();
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

		//if (opaque)
		//	nd.max_opq_mat_id = max((uint)idx, nd.max_opq_mat_id);
		//else
		//	nd.max_trn_mat_id = max((uint)idx, nd.max_trn_mat_id);
		return idx;
	}

	int sNodeRendererPrivate::find_material_res(graphics::Material* mat) const
	{
		//for (auto i = 0; i < mat_reses.size(); i++)
		//{
		//	if (mat_reses[i].mat == mat)
		//		return i;
		//}
		return -1;
	}

	int sNodeRendererPrivate::set_mesh_res(int idx, graphics::Mesh* mesh)
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
					buf_vtx.set_var<"i_nor"_h>(mesh->normals[i]);
					buf_vtx.next_item();
				}
				//	vtx.uv = auv ? auv[i] : vec2(0.f);
				//	vtx.normal = anormal ? anormal[i] : vec3(1.f, 0.f, 0.f);

				dst.idx_off = buf_idx.item_offset();
				buf_idx.push(dst.idx_cnt, mesh->indices.data());

				buf_vtx.upload(cb.get());
				buf_idx.upload(cb.get());
			}
			else
			{
				//dst.vtx_off = nd.buf_arm_mesh_vtx.n1;
				//auto pvtx = nd.buf_arm_mesh_vtx.alloc(dst.vtx_cnt);

				//for (auto i = 0; i < dst.vtx_cnt; i++)
				//{
				//	auto& vtx = pvtx[i];
				//	vtx.pos = apos[i];
				//	vtx.uv = auv ? auv[i] : vec2(0.f);
				//	vtx.normal = anormal ? anormal[i] : vec3(1.f, 0.f, 0.f);
				//	vtx.ids = abids ? abids[i] : ivec4(-1);
				//	vtx.weights = abwgts ? abwgts[i] : vec4(0.f);
				//}

				//dst.idx_off = nd.buf_arm_mesh_idx.n1;
				//memcpy(nd.buf_arm_mesh_idx.alloc(dst.idx_cnt), aidx, sizeof(uint) * dst.idx_cnt);

				//nd.buf_arm_mesh_vtx.upload(cb.get());
				//nd.buf_arm_mesh_idx.upload(cb.get());
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

	int sNodeRendererPrivate::find_mesh_res(graphics::Mesh* mesh) const
	{
		for (auto i = 0; i < mesh_reses.size(); i++)
		{
			if (mesh_reses[i].mesh == mesh)
				return i;
		}
		return -1;
	}

	int sNodeRendererPrivate::register_object()
	{
		return buf_objects.get_free_item();
	}

	void sNodeRendererPrivate::unregister_object(uint id)
	{
		buf_objects.release_item(id);
	}

	void sNodeRendererPrivate::set_object_matrix(uint id, const mat4& mat, const mat3& nor)
	{
		buf_objects.select_item(id);
		buf_objects.set_var<"mat"_h>(mat);
		buf_objects.set_var<"nor"_h>(mat4(nor));
	}

	int sNodeRendererPrivate::register_armature_object()
	{
		return -1;
	}

	void sNodeRendererPrivate::unregister_armature_object(uint id)
	{

	}

	void sNodeRendererPrivate::set_armature_object_matrices(uint id, const mat4* bones, uint count)
	{

	}

	void sNodeRendererPrivate::draw_mesh(uint object_id, uint mesh_id, uint skin, DrawType type)
	{
		DrawMesh d;
		d.node = current_node;
		d.object_id = object_id;
		d.mesh_id = mesh_id;
		d.skin = skin;
		d.type = type;
		if (type == DrawOutline)
			draw_outline_meshes.push_back(d);
		else
			draw_meshes.push_back(d);
	}

	void sNodeRendererPrivate::render(uint img_idx, graphics::CommandBufferPtr cb)
	{
		if (!initialized)
			return;

		img_idx = min(max(0, (int)fbs_fwd.size() - 1), (int)img_idx);
		auto iv = iv_tars[img_idx];
		auto sz = vec2(iv->image->size);

		auto camera = cCamera::main();
		if (!camera)
			return;

		camera->aspect = sz.x / sz.y;
		camera->update();
		auto view_inv = inverse(camera->view_mat);
		auto proj_inv = inverse(camera->proj_mat);

		std::vector<cNodePtr> nodes;
		sScene::instance()->octree->get_within_frustum(Frustum(proj_inv), nodes);
		draw_meshes.clear();
		current_node = nullptr;
		for (auto& n : nodes)
		{
			current_node = n;
			n->draw(this, false);
		}

		buf_scene.set_var<"camera_coord"_h>(camera->node->g_pos);
		buf_scene.set_var<"camera_dir"_h>(-camera->node->g_rot[2]);

		buf_scene.set_var<"view"_h>(camera->view_mat);
		buf_scene.set_var<"view_inv"_h>(view_inv);
		buf_scene.set_var<"proj"_h>(camera->proj_mat);
		buf_scene.set_var<"proj_inv"_h>(proj_inv);
		buf_scene.set_var<"proj_view"_h>(camera->proj_mat * camera->view_mat);
		
		buf_scene.upload(cb);

		for (auto& d : draw_meshes)
		{
			auto& mr = mesh_reses[d.mesh_id];
			buf_idr_mesh.add_draw_indexed_indirect(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (d.object_id << 16) + 0/* mat id */);
		}

		buf_objects.upload(cb);
		buf_idr_mesh.upload(cb);

		cb->set_viewport(Rect(vec2(0), sz));
		cb->set_scissor(Rect(vec2(0), sz));

		vec4 cvs[] = { vec4(0.9f, 0.8f, 0.1f, 1.f),
			vec4(1.f, 0.f, 0.f, 0.f) };
		cb->begin_renderpass(nullptr, fbs_fwd[img_idx].get(), cvs);

		cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
		cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);

		cb->bind_pipeline(pl_mesh_fwd);
		prm_mesh_fwd.set_ds("scene"_h, ds_scene.get());
		prm_mesh_fwd.set_ds("object"_h, ds_object.get());
		prm_mesh_fwd.bind_dss(cb);
		prm_mesh_fwd.set_pc_var<"f"_h>(vec4(1.f));
		prm_mesh_fwd.push_constant(cb);

		cb->draw_indexed_indirect(buf_idr_mesh.buf.get(), 0, draw_meshes.size());

		cb->end_renderpass();

		cb->image_barrier(iv->image, iv->sub, dst_layout);
	}

	void sNodeRendererPrivate::update()
	{
	}

	cNodePtr sNodeRendererPrivate::pick_up(const uvec2& pos)
	{
		{
			auto sz = vec2(img_pickup->size);

			graphics::InstanceCB cb(nullptr, fence_pickup.get());

			cb->set_viewport(Rect(vec2(0), sz));
			cb->set_scissor(Rect(vec2(pos), vec2(pos + 1U)));
			vec4 cvs[] = { vec4(0.f),
				vec4(1.f, 0.f, 0.f, 0.f) };
			cb->begin_renderpass(nullptr, fb_pickup.get(), cvs);

			cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
			cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
			cb->bind_pipeline(pl_mesh_pickup);
			prm_mesh_fwd.set_ds("scene"_h, ds_scene.get());
			prm_mesh_fwd.set_ds("object"_h, ds_object.get());
			prm_mesh_fwd.bind_dss(cb.get());
			for (auto i = 0; i < draw_meshes.size(); i++)
			{
				auto& d = draw_meshes[i];
				prm_mesh_fwd.set_pc_var<"i"_h>(ivec4(i + 1, 0, 0, 0));
				prm_mesh_fwd.push_constant(cb.get());
				auto& mr = mesh_reses[d.mesh_id];
				cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, d.object_id << 16);
			}

			cb->end_renderpass();
		}

		{
			int index;
			graphics::StagingBuffer sb(nullptr, sizeof(index), nullptr, graphics::BufferUsageTransferDst);
			{
				graphics::InstanceCB cb(nullptr);
				graphics::BufferImageCopy cpy;
				cpy.img_off = pos;
				cpy.img_ext = uvec2(1U);
				cb->image_barrier(img_pickup.get(), cpy.img_sub, graphics::ImageLayoutTransferSrc);
				cb->copy_image_to_buffer(img_pickup.get(), sb.get(), { &cpy, 1 });
				cb->image_barrier(img_pickup.get(), cpy.img_sub, graphics::ImageLayoutAttachment);
			}
			memcpy(&index, sb->mapped, sizeof(index));
			index -= 1;
			if (index == -1)
				return nullptr;
			return draw_meshes[index].node;
		}
	}

	static sNodeRendererPtr _instance = nullptr;

	struct sNodeRendererInstance : sNodeRenderer::Instance
	{
		sNodeRendererPtr operator()() override
		{
			return _instance;
		}
	}sNodeRenderer_instance_private;
	sNodeRenderer::Instance& sNodeRenderer::instance = sNodeRenderer_instance_private;

	struct sNodeRendererCreatePrivate : sNodeRenderer::Create
	{
		sNodeRendererPtr operator()(WorldPtr w) override
		{
			if (!w)
				return new sNodeRendererPrivate();

			assert(!_instance);

			auto& windows = graphics::Window::get_list();
			if (windows.empty())
			{
				printf("node renderer system needs graphics window\n");
				return nullptr;
			}

			_instance = new sNodeRendererPrivate(windows[0]);
			return _instance;
		}
	}sNodeRenderer_create_private;
	sNodeRenderer::Create& sNodeRenderer::create = sNodeRenderer_create_private;
}
