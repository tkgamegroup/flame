#include "node_renderer_private.h"
#include "../world_private.h"
#include "../components/node_private.h"

#include "../../foundation/typeinfo.h"
#include "../../foundation/typeinfo_serialize.h"
#include "../../graphics/command_ext.h"
#include "../../graphics/renderpass.h"
#include "../../graphics/shader.h"
#include "../../graphics/window.h"
#include "../../graphics/model.h"

namespace flame
{
	const graphics::Format dep_fmt = graphics::Format::Format_Depth16;

	sNodeRendererPrivate::sNodeRendererPrivate(graphics::WindowPtr w)
	{
		mesh_reses.resize(1024);
		
		w->renderers.add([this](uint img_idx, graphics::CommandBufferPtr cb) {
			img_idx = min((int)fb_tars.size() - 1, (int)img_idx);
			auto iv = iv_tars[img_idx];
			auto sz = vec2(iv->image->size);

			vec4 cvs[] = { vec4(0.9f, 0.8f, 0.1f, 1.f), 
				vec4(1.f, 0.f, 0.f, 0.f) };
			cb->begin_renderpass(nullptr, fb_tars[img_idx].get(), cvs);
			{
				cb->set_viewport(Rect(vec2(0), sz));
				cb->set_scissor(Rect(vec2(0), sz));

				auto mat = perspective(45.f, sz.x / sz.y, 1.f, 1000.f) * lookAt(vec3(5.f), vec3(0.f), vec3(0.f, 1.f, 0.f));
				mat[1][1] *= -1.f;

				cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
				cb->bind_index_buffer(buf_idx.buf.get(), graphics::IndiceTypeUint);
				cb->bind_pipeline(pl_fwd);
				cb->push_constant_t(mat, vu_pc.var_off<"mvp"_h>());
				cb->push_constant_t(vec4(1.f), vu_pc.var_off<"col"_h>());
				auto& mr = mesh_reses[0];
				cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, 0);
			}
			cb->end_renderpass();

			cb->image_barrier(iv->image, { iv->sub.base_level, 1, iv->sub.base_layer, 1 }, dst_layout);
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
			pl_fwd = graphics::GraphicsPipeline::get(nullptr, L"default_assets\\shaders\\plain\\mesh3d.pipeline",
				{ "rp=0x" + to_string((uint64)rp_fwd) });

			buf_vtx.create(pl_fwd->info.shaders[0]->in_ui, 1024 * 128 * 4);
			buf_idx.create(sizeof(uint), 1024 * 128 * 6);
			vu_pc.ui = pl_fwd->info.layout->pc_ui;

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

	void sNodeRendererPrivate::update()
	{
		auto n = world->root->get_component_i<cNodeT>(0);
		n->pos;
	}

	struct sNodeRendererCreatePrivate : sNodeRenderer::Create
	{
		sNodeRendererPtr operator()(WorldPtr) override
		{
			auto& windows = graphics::Window::get_list();
			if (windows.empty())
			{
				printf("node renderer system need graphics window\n");
				return nullptr;
			}

			return new sNodeRendererPrivate(windows[0]);
		}
	}sNodeRenderer_create_private;
	sNodeRenderer::Create& sNodeRenderer::create = sNodeRenderer_create_private;
}
