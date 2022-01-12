#include "node_renderer_private.h"
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

	sNodeRendererPrivate::sNodeRendererPrivate(graphics::WindowPtr w)
	{
		mesh_reses.resize(1024);
		
		w->renderers.add([this](uint img_idx, graphics::CommandBufferPtr cb) {
			if (!initialized)
				return;

			img_idx = min((int)fb_tars.size() - 1, (int)img_idx);
			auto iv = iv_tars[img_idx];
			auto sz = vec2(iv->image->size);

			auto camera = cCamera::main();
			if (!camera)
				return;

			camera->aspect = sz.x / sz.y;
			camera->update();

			buf_scene.set_var<"proj"_h>(camera->proj_mat);
			buf_scene.set_var<"view"_h>(camera->view_mat);
			buf_scene.set_var<"proj_view"_h>(camera->proj_mat * camera->view_mat);
			buf_scene.upload(cb);

			buf_mesh_transforms.upload(cb);

			collect_draws(world->root.get());
			auto n_mesh_idr = buf_idr_mesh.n_offset();
			buf_idr_mesh.upload(cb);

			vec4 cvs[] = { vec4(0.9f, 0.8f, 0.1f, 1.f), 
				vec4(1.f, 0.f, 0.f, 0.f) };
			cb->begin_renderpass(nullptr, fb_tars[img_idx].get(), cvs);
			{
				cb->set_viewport(Rect(vec2(0), sz));
				cb->set_scissor(Rect(vec2(0), sz));

				cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
				cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
				cb->bind_pipeline(pl_mesh_fwd);
				prm_mesh_fwd.set_pc_var<"f"_h>(vec4(1.f));
				prm_mesh_fwd.push_constant(cb);
				prm_mesh_fwd.set_ds("scene"_h, ds_scene.get());
				prm_mesh_fwd.set_ds("mesh"_h, ds_mesh.get());
				prm_mesh_fwd.bind_dss(cb);
				
				cb->draw_indexed_indirect(buf_idr_mesh.buf.get(), 0, n_mesh_idr);
			}
			cb->end_renderpass();

			cb->image_barrier(iv->image, iv->sub, dst_layout);
		});
	}

	void sNodeRendererPrivate::set_targets(std::span<graphics::ImageViewPtr> targets, graphics::ImageLayout _dst_layout)
	{
		auto img0 = targets.front()->image;
		auto size = img0->size;

		if (!initialized)
		{
			rp_fwd = graphics::Renderpass::get(nullptr, L"default_assets\\shaders\\forward.rp",
				{ "col_fmt=" + TypeInfo::serialize_t(&img0->format),
				  "dep_fmt=" + TypeInfo::serialize_t(&dep_fmt) });
			pl_mesh_fwd = graphics::GraphicsPipeline::get(nullptr, L"default_assets\\shaders\\mesh\\mesh.pipeline",
				{ "rp=0x" + to_string((uint64)rp_fwd) });

			buf_vtx.create(pl_mesh_fwd->vi_ui(), 1024 * 128 * 4);
			buf_idx.create(sizeof(uint), 1024 * 128 * 6);
			auto scene_dsl = graphics::DescriptorSetLayout::get(nullptr, L"default_assets\\shaders\\scene.dsl");
			buf_scene.create(scene_dsl->get_buf_ui("Scene"));
			ds_scene.reset(graphics::DescriptorSet::create(nullptr, scene_dsl));
			ds_scene->set_buffer("Scene", 0, buf_scene.buf.get());
			ds_scene->update();
			auto mesh_dsl = graphics::DescriptorSetLayout::get(nullptr, L"default_assets\\shaders\\mesh\\mesh.dsl");
			buf_mesh_transforms.create_with_array_type(mesh_dsl->get_buf_ui("Transforms"));
			ds_mesh.reset(graphics::DescriptorSet::create(nullptr, mesh_dsl));
			ds_mesh->set_buffer("Transforms", 0, buf_mesh_transforms.buf.get());
			ds_mesh->update();
			prm_mesh_fwd.init(pl_mesh_fwd->layout);
			buf_idr_mesh.create(0U, buf_mesh_transforms.array_capacity);

			set_mesh_res(-1, &graphics::Model::get(L"standard:cube")->meshes[0]);
			set_mesh_res(-1, &graphics::Model::get(L"standard:sphere")->meshes[0]);

			initialized = true;
		}

		img_dep.reset(graphics::Image::create(nullptr, dep_fmt, size, 1, 1, graphics::SampleCount_1, graphics::ImageUsageAttachment));
		auto iv_dep = img_dep->get_view();

		iv_tars.assign(targets.begin(), targets.end());
		fb_tars.clear();
		for (auto iv : targets)
		{
			graphics::ImageViewPtr ivs[] = { iv, iv_dep };
			fb_tars.emplace_back(graphics::Framebuffer::create(rp_fwd, ivs));
		}

		dst_layout = _dst_layout;
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
				dst.vtx_off = buf_vtx.n_offset();
				for (auto i = 0; i < dst.vtx_cnt; i++)
				{
					buf_vtx.set_var<"i_pos"_h>(mesh->positions[i]);
					buf_vtx.next_item();
				}
				//	vtx.uv = auv ? auv[i] : vec2(0.f);
				//	vtx.normal = anormal ? anormal[i] : vec3(1.f, 0.f, 0.f);

				dst.idx_off = buf_idx.n_offset();
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
		return -1;
	}

	uint sNodeRendererPrivate::add_mesh_transform(const mat4& mat, const mat3& nor)
	{
		return 0;
	}

	uint sNodeRendererPrivate::add_mesh_armature(const mat4* bones, uint count)
	{
		return 0;
	}

	void sNodeRendererPrivate::draw_mesh(uint id, uint mesh_id, uint skin, ShadingFlags flags)
	{

	}

	void sNodeRendererPrivate::collect_draws(Entity* e)
	{
		if (!e->global_enable)
			return;

		if (auto node = e->get_component_i<cNodeT>(0); node)
		{
			auto id = buf_mesh_transforms.n_offset();
			buf_mesh_transforms.set_var<"mat"_h>(node->transform);
			buf_mesh_transforms.set_var<"nor"_h>(mat4(node->g_rot));
			buf_mesh_transforms.next_item();

			auto& mr = mesh_reses[0];
			buf_idr_mesh.add_draw_indexed_indirect(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, (id << 16) + 0/* mat id */);
		}

		for (auto& c : e->children)
			collect_draws(c.get());
	}

	void sNodeRendererPrivate::update()
	{
		if (!initialized)
			return;
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
		sNodeRendererPtr operator()(WorldPtr) override
		{
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
