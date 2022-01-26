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
	const graphics::Format col_fmt = graphics::Format::Format_R8G8B8A8_UNORM;

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

	void sNodeRendererPrivate::set_targets(std::span<graphics::ImageViewPtr> _targets, graphics::ImageLayout _dst_layout)
	{
		auto img0 = _targets.front()->image;
		auto tar_size = img0->size;
		
		auto rp_fwd = graphics::Renderpass::get(nullptr, L"default_assets\\shaders\\forward.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(&img0->format),
			  "dep_fmt=" + TypeInfo::serialize_t(&dep_fmt) });
		auto rp_col_dep = graphics::Renderpass::get(nullptr, L"default_assets\\shaders\\color_depth.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(&col_fmt),
			  "dep_fmt=" + TypeInfo::serialize_t(&dep_fmt) });
		auto rp_col = graphics::Renderpass::get(nullptr, L"default_assets\\shaders\\color.rp",
			{ "col_fmt=" + TypeInfo::serialize_t(&col_fmt) });

		if (!initialized)
		{
			pl_blit = graphics::GraphicsPipeline::get(nullptr, L"default_assets\\shaders\\blit.pipeline",
				{ "rp=" + str(rp_col) });
			pl_add = graphics::GraphicsPipeline::get(nullptr, L"default_assets\\shaders\\add.pipeline",
				{ "rp=" + str(rp_col) });

			pl_mesh_fwd = graphics::GraphicsPipeline::get(nullptr, L"default_assets\\shaders\\mesh\\mesh.pipeline",
				{ "rp=" + str(rp_fwd),
				  "frag:CAMERA_LIGHT" });

			buf_vtx.create(pl_mesh_fwd->vi_ui(), 1024 * 128 * 4);
			buf_idx.create(sizeof(uint), 1024 * 128 * 6);
			prm_mesh_fwd.init(pl_mesh_fwd->layout);
			buf_idr_mesh.create(0U, buf_objects.array_capacity);

			pl_mesh_plain = graphics::GraphicsPipeline::get(nullptr, L"default_assets\\shaders\\mesh\\mesh.pipeline",
				{ "rp=" + str(rp_col_dep) });

			prm_post.init(graphics::PipelineLayout::get(nullptr, L"default_assets\\shaders\\post\\post.pll"));
			pl_blur_h = graphics::GraphicsPipeline::get(nullptr, L"default_assets\\shaders\\post\\blur.pipeline",
				{ "rp=" + str(rp_col),
				"frag:HORIZONTAL" });
			pl_blur_v = graphics::GraphicsPipeline::get(nullptr, L"default_assets\\shaders\\post\\blur.pipeline",
				{ "rp=" + str(rp_col),
				"frag:VERTICAL" });

			pl_mesh_pickup = graphics::GraphicsPipeline::get(nullptr, L"default_assets\\shaders\\mesh\\mesh.pipeline",
				{ "rp=" + str(rp_col_dep),
				  "frag:PICKUP" });
			fence_pickup.reset(graphics::Fence::create(nullptr, false));

			set_mesh_res(-1, &graphics::Model::get(L"standard:cube")->meshes[0]);
			set_mesh_res(-1, &graphics::Model::get(L"standard:sphere")->meshes[0]);

			initialized = true;
		}

		iv_tars.assign(_targets.begin(), _targets.end());

		img_dep.reset(graphics::Image::create(nullptr, dep_fmt, tar_size, graphics::ImageUsageAttachment));
		auto iv_dep = img_dep->get_view();
		fbs_fwd.clear();
		for (auto iv : _targets)
			fbs_fwd.emplace_back(graphics::Framebuffer::create(rp_fwd, { iv, iv_dep }));

		dst_layout = _dst_layout;

		img_back0.reset(graphics::Image::create(nullptr, col_fmt, tar_size, graphics::ImageUsageAttachment));
		img_back1.reset(graphics::Image::create(nullptr, col_fmt, tar_size, graphics::ImageUsageAttachment));

		img_pickup.reset(graphics::Image::create(nullptr, col_fmt, tar_size, graphics::ImageUsageAttachment | graphics::ImageUsageTransferSrc));
		img_dep_pickup.reset(graphics::Image::create(nullptr, dep_fmt, tar_size, graphics::ImageUsageAttachment));
		fb_pickup.reset(graphics::Framebuffer::create(rp_col_dep, { img_pickup->get_view(), img_dep_pickup->get_view() }));
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

	void sNodeRendererPrivate::draw_mesh(uint object_id, uint mesh_id, uint skin)
	{
		DrawMesh d;
		d.node = current_node;
		d.object_id = object_id;
		d.mesh_id = mesh_id;
		d.skin = skin;
		draw_meshes.push_back(d);
	}

	void sNodeRendererPrivate::draw_mesh_occluder(uint object_id, uint mesh_id, uint skin)
	{
		DrawMeshOccluder d;
		d.node = current_node;
		d.object_id = object_id;
		d.mesh_id = mesh_id;
		d.skin = skin;
		draw_occluder_meshes.push_back(d);
	}

	void sNodeRendererPrivate::draw_mesh_outline(uint object_id, uint mesh_id, const cvec4& color)
	{
		DrawMeshOutline d;
		d.node = current_node;
		d.object_id = object_id;
		d.mesh_id = mesh_id;
		d.color = color;
		draw_outline_meshes.push_back(d);
	}

	void sNodeRendererPrivate::draw_mesh_wireframe(uint object_id, uint mesh_id, const cvec4& color)
	{
		DrawMeshWireframe d;
		d.node = current_node;
		d.object_id = object_id;
		d.mesh_id = mesh_id;
		d.color = color;
		draw_wireframe_meshes.push_back(d);
	}

	static std::vector<std::vector<float>> gauss_blur_weights;
	static std::vector<float>& get_gauss_blur_weights(int radius)
	{
		radius = radius - 1 / 2 - 1;
		if (gauss_blur_weights.empty())
		{
			gauss_blur_weights.push_back({                     0.047790, 0.904419, 0.047790 });
			gauss_blur_weights.push_back({           0.015885, 0.221463, 0.524950, 0.221463, 0.015885 });
			gauss_blur_weights.push_back({ 0.005977, 0.060598, 0.241730, 0.382925, 0.241730, 0.060598, 0.005977 });
		}

		assert(radius < gauss_blur_weights.size());
		return gauss_blur_weights[radius];
	}

	void sNodeRendererPrivate::render(uint img_idx, graphics::CommandBufferPtr cb)
	{
		if (!initialized)
			return;

		img_idx = min(max(0, (int)fbs_fwd.size() - 1), (int)img_idx);
		auto iv = iv_tars[img_idx];
		auto img = iv->image;
		auto sz = vec2(img->size);

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
		draw_occluder_meshes.clear();
		draw_outline_meshes.clear();
		draw_wireframe_meshes.clear();
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

		cb->begin_renderpass(nullptr, fbs_fwd[img_idx].get(), 
			{ vec4(0.9f, 0.8f, 0.1f, 1.f),
			vec4(1.f, 0.f, 0.f, 0.f) });

		cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
		cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);

		cb->bind_pipeline(pl_mesh_fwd);
		prm_mesh_fwd.set_ds("scene"_h, ds_scene.get());
		prm_mesh_fwd.set_ds("object"_h, ds_object.get());
		prm_mesh_fwd.bind_dss(cb);
		prm_mesh_fwd.set_pc_var<"f"_h>(vec4(1.f));
		prm_mesh_fwd.push_constant(cb);

		cb->draw_indexed_indirect(buf_idr_mesh.buf.get(), 0, draw_meshes.size());

		if (!draw_outline_meshes.empty())
		{
			cb->set_viewport(Rect(vec2(0), sz));
			cb->set_scissor(Rect(vec2(0), sz));

			cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), { vec4(0.f) });
			cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
			cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
			cb->bind_pipeline(pl_mesh_plain);
			prm_mesh_fwd.set_ds("scene"_h, ds_scene.get());
			prm_mesh_fwd.set_ds("object"_h, ds_object.get());
			prm_mesh_fwd.bind_dss(cb);
			for (auto& d : draw_outline_meshes)
			{
				prm_mesh_fwd.set_pc_var<"f"_h>(vec4(d.color) / 255.f);
				prm_mesh_fwd.push_constant(cb);
				auto& mr = mesh_reses[d.mesh_id];
				cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, d.object_id << 16);
			}
			cb->end_renderpass();

			cb->bind_pipeline_layout(prm_post.pll);
			prm_post.set_ds(""_h, img_back1->get_shader_read_src());
			prm_post.bind_dss(cb);
			auto& weights = get_gauss_blur_weights(5);
			prm_post.set_pc_var<"off"_h>(-((int)weights.size() - 1) / 2);
			prm_post.set_pc_var<"len"_h>((int)weights.size());
			prm_post.set_pc_var<"weights"_h>(weights.data(), sizeof(float) * weights.size());
			prm_post.set_pc_var<"pxsz"_h>(1.f / (vec2)img_back0->size);
			prm_post.push_constant(cb);

			cb->image_barrier(img_back0.get(), {}, graphics::ImageLayoutShaderReadOnly);
			cb->begin_renderpass(nullptr, img_back1->get_shader_write_dst());
			cb->bind_pipeline(pl_blur_h);
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();

			cb->image_barrier(img_back1.get(), {}, graphics::ImageLayoutShaderReadOnly);
			cb->begin_renderpass(nullptr, img_back0->get_shader_write_dst());
			cb->bind_pipeline(pl_blur_v);
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();

			cb->image_barrier(img_back0.get(), {}, graphics::ImageLayoutShaderReadOnly);
			cb->begin_renderpass(nullptr, img->get_shader_write_dst(0, 0, graphics::AttachmentLoadLoad));
			cb->bind_pipeline(pl_add);
			cb->bind_descriptor_set(0, img_back0->get_shader_read_src());
			cb->push_constant_t(1.f / (vec2)img_back0->size);
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();
		}

		cb->end_renderpass();

		cb->image_barrier(img, iv->sub, dst_layout);
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

			cb->begin_renderpass(nullptr, fb_pickup.get(), { vec4(0.f),
				vec4(1.f, 0.f, 0.f, 0.f) });

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
