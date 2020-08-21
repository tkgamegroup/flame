#include <flame/serialize.h>
#include "device_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "font_private.h"
#include "renderpass_private.h"
#include "shader_private.h"
#include "command_private.h"
#include "swapchain_private.h"
#include "canvas_private.h"

namespace flame
{
	namespace graphics
	{
		/*
		inline void path_arc(std::vector<Vec2f>& points, const Vec2f& center, float radius, float a1, float a2, uint lod = 0)
		{
			const uint pieces_num = 36;
			static const Vec2f pieces[] = {
				Vec2f(+1.000000f, +0.000000f), Vec2f(+0.984808f, +0.173648f), Vec2f(+0.939693f, +0.342020f), Vec2f(+0.866025f, +0.500000f), Vec2f(+0.766044f, +0.642788f), Vec2f(+0.642788f, +0.766044f),
				Vec2f(+0.500000f, +0.866025f), Vec2f(+0.342020f, +0.939693f), Vec2f(+0.173648f, +0.984808f), Vec2f(+0.000000f, +1.000000f), Vec2f(-0.173648f, +0.984808f), Vec2f(-0.342020f, +0.939693f),
				Vec2f(-0.500000f, +0.866025f), Vec2f(-0.642788f, +0.766044f), Vec2f(-0.766044f, +0.642788f), Vec2f(-0.866025f, +0.500000f), Vec2f(-0.939693f, +0.342020f), Vec2f(-0.984808f, +0.173648f),
				Vec2f(-1.000000f, +0.000000f), Vec2f(-0.984808f, -0.173648f), Vec2f(-0.939693f, -0.342020f), Vec2f(-0.866025f, -0.500000f), Vec2f(-0.766044f, -0.642788f), Vec2f(-0.642788f, -0.766044f),
				Vec2f(-0.500000f, -0.866025f), Vec2f(-0.342020f, -0.939693f), Vec2f(-0.173648f, -0.984808f), Vec2f(-0.000000f, -1.000000f), Vec2f(+0.173648f, -0.984808f), Vec2f(+0.342020f, -0.939693f),
				Vec2f(+0.500000f, -0.866025f), Vec2f(+0.642788f, -0.766044f), Vec2f(+0.766044f, -0.642788f), Vec2f(+0.866025f, -0.500000f), Vec2f(+0.939693f, -0.342020f), Vec2f(+0.984808f, -0.173648f)
			};
			static_assert(size(pieces) == pieces_num);

			int a = pieces_num * a1;
			int b = pieces_num * a2;
			lod += 1;
			for (; a <= b; a += lod)
				points.push_back(center + pieces[a % pieces_num] * radius);
		}

		// roundness: LT RT RB LB
		inline void path_rect(std::vector<Vec2f>& points, const Vec2f& pos, const Vec2f& size, const Vec4f& roundness = Vec4f(0.f), uint lod = 0)
		{
			if (roundness[0] > 0.f)
				path_arc(points, pos + Vec2f(roundness[0]), roundness[0], 0.5f, 0.75f, lod);
			else
				points.push_back(pos);
			if (roundness[1] > 0.f)
				path_arc(points, pos + Vec2f(size.x() - roundness[1], roundness[1]), roundness[1], 0.75f, 1.f, lod);
			else
				points.push_back(pos + Vec2f(size.x(), 0.f));
			if (roundness[2] > 0.f)
				path_arc(points, pos + size - Vec2f(roundness[2]), roundness[2], 0.f, 0.25f, lod);
			else
				points.push_back(pos + size);
			if (roundness[3] > 0.f)
				path_arc(points, pos + Vec2f(roundness[3], size.y() - roundness[3]), roundness[3], 0.25f, 0.5f, lod);
			else
				points.push_back(pos + Vec2f(0.f, size.y()));
		}

		inline void path_circle(std::vector<Vec2f>& points, const Vec2f& center, float radius, uint lod = 0)
		{
			path_arc(points, center, radius, 0.f, 1.f, lod);
		}

		inline void path_bezier(std::vector<Vec2f>& points, const Vec2f& p1, const Vec2f& p2, const Vec2f& p3, const Vec2f& p4, uint level = 0)
		{
			auto dx = p4.x() - p1.x();
			auto dy = p4.y() - p1.y();
			auto d2 = ((p2.x() - p4.x()) * dy - (p2.y() - p4.y()) * dx);
			auto d3 = ((p3.x() - p4.x()) * dy - (p3.y() - p4.y()) * dx);
			d2 = (d2 >= 0) ? d2 : -d2;
			d3 = (d3 >= 0) ? d3 : -d3;
			if ((d2 + d3) * (d2 + d3) < 1.25f * (dx * dx + dy * dy))
			{
				if (points.empty())
					points.push_back(p1);
				points.push_back(p4);
			}
			else if (level < 10)
			{
				auto p12 = (p1 + p2) * 0.5f;
				auto p23 = (p2 + p3) * 0.5f;
				auto p34 = (p3 + p4) * 0.5f;
				auto p123 = (p12 + p23) * 0.5f;
				auto p234 = (p23 + p34) * 0.5f;
				auto p1234 = (p123 + p234) * 0.5f;

				path_bezier(points, p1, p12, p123, p1234, level + 1);
				path_bezier(points, p1234, p234, p34, p4, level + 1);
			}
		}
		*/

		static RenderpassPrivate* rp_tar = nullptr;
		static DescriptorSetLayoutPrivate* dsl_el = nullptr;
		static PipelineLayoutPrivate* pll_el = nullptr;
		static PipelinePrivate* pl_el = nullptr;

		static RenderpassPrivate* rp_bk = nullptr;
		static DescriptorSetLayoutPrivate* dsl_1_img = nullptr;
		static PipelineLayoutPrivate* pll_blur = nullptr;
		static PipelinePrivate* pl_blurh = nullptr;
		static PipelinePrivate* pl_blurv = nullptr;
		static PipelineLayoutPrivate* pll_blt = nullptr;
		static PipelinePrivate* pl_blt = nullptr;

		CanvasPrivate::CanvasPrivate(DevicePrivate* d) :
			device(d)
		{
			auto shader_path = std::filesystem::path(L"shaders");
			{
				auto engin_path = getenv("FLAME_PATH");
				if (engin_path)
					shader_path = engin_path / shader_path;
			}

			if (!rp_tar)
			{
				RenderpassAttachmentInfo att;
				att.format = Swapchain::get_format();
				att.clear = false;
				RenderpassSubpassInfo sp;
				uint col_refs[] = {
					0
				};
				sp.color_attachments_count = 1;
				sp.color_attachments = col_refs;
				rp_tar = new RenderpassPrivate(d, { &att, 1 }, { &sp,1 });
			}
			if (!dsl_el)
			{
				DescriptorBindingInfo db;
				db.type = DescriptorSampledImage;
				db.count = resources_count;
				db.name = "images";
				dsl_el = new DescriptorSetLayoutPrivate(d, { &db, 1});
			}
			if (!pll_el)
				pll_el = new PipelineLayoutPrivate(d, { &dsl_el, 1 }, 16);
			if (!pl_el)
			{
				VertexAttributeInfo vias[3];
				auto& via1 = vias[0];
				via1.format = Format_R32G32_SFLOAT;
				via1.name = "pos";
				auto& via2 = vias[1];
				via2.format = Format_R32G32_SFLOAT;
				via2.name = "uv";
				auto& via3 = vias[2];
				via3.format = Format_R8G8B8A8_UNORM;
				via3.name = "color";
				VertexBufferInfo vib;
				vib.attributes_count = size(vias);
				vib.attributes = vias;
				VertexInfo vi;
				vi.buffers_count = 1;
				vi.buffers = &vib;
				ShaderPrivate* shaders[] = {
					new ShaderPrivate(L"element.vert"),
					new ShaderPrivate(L"element.frag")
				};
				pl_el = PipelinePrivate::create(d, shader_path, shaders, pll_el, rp_tar, 0, &vi);
			}

			if (!rp_bk)
			{
				auto fmt = Swapchain::get_format();
				RenderpassAttachmentInfo att;
				att.format = Format_R8G8B8A8_UNORM;
				att.clear = false;
				RenderpassSubpassInfo sp;
				uint col_refs[] = {
					0
				};
				sp.color_attachments_count = 1;
				sp.color_attachments = col_refs;
				rp_bk = new RenderpassPrivate(d, { &att, 1 }, { &sp,1 });
			}
			if (!dsl_1_img)
			{
				DescriptorBindingInfo db;
				db.type = DescriptorSampledImage;
				db.count = 1;
				db.name = "image";
				dsl_1_img = new DescriptorSetLayoutPrivate(d, { &db, 1 });
			}
			if (!pll_blur)
				pll_blur = new PipelineLayoutPrivate(d, { &dsl_1_img, 1 }, sizeof(Vec4f));
			if (!pl_blurh)
			{
				ShaderPrivate* shaders[] = {
					new ShaderPrivate(L"fullscreen.vert", "#define NO_COORD"),
					new ShaderPrivate(L"blur.frag", "#define H")
				};
				pl_blurh = PipelinePrivate::create(d, shader_path, shaders, pll_blur, rp_bk, 0);
			}
			if (!pl_blurv)
			{
				ShaderPrivate* shaders[] = {
					new ShaderPrivate(L"fullscreen.vert", "#define NO_COORD"),
					new ShaderPrivate(L"blur.frag", "#define V")
				};
				pl_blurv = PipelinePrivate::create(d, shader_path, shaders, pll_blur, rp_bk, 0);
			}
			if (!pll_blt)
				pll_blt = new PipelineLayoutPrivate(d, { &dsl_1_img, 1 }, sizeof(Vec4f));
			if (!pl_blt)
			{
				ShaderPrivate* shaders[] = {
					new ShaderPrivate(L"fullscreen.vert"),
					new ShaderPrivate(L"blt.frag")
				};
				pl_blt = PipelinePrivate::create(d, shader_path, shaders, pll_blt, rp_bk, 0);
			}

			buf_vtx.reset(new BufferPrivate(d, 3495200, BufferUsageVertex, MemoryPropertyHost | MemoryPropertyCoherent));
			buf_vtx->map();
			buf_idx.reset(new BufferPrivate(d, 1048576, BufferUsageIndex, MemoryPropertyHost | MemoryPropertyCoherent));
			buf_idx->map();

			img_white.reset(new ImagePrivate(d, Format_R8G8B8A8_UNORM, Vec2u(1), 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled));
			img_white->clear(ImageLayoutUndefined, ImageLayoutShaderReadOnly, Vec4c(255));
			resources.resize(resources_count);
			auto iv_white = img_white->default_view.get();
			for (auto i = 0; i < resources_count; i++)
			{
				auto r = new CanvasResourcePrivate;
				r->view = iv_white;
				resources[i].reset(r);
			}

			ds_el.reset(new DescriptorSetPrivate(d->descriptor_pool.get(), dsl_el));
			auto sp = d->sampler_linear.get();
			for (auto i = 0; i < resources_count; i++)
				ds_el->set_image(0, i, iv_white, sp);
		}

		void CanvasPrivate::set_target(std::span<ImageViewPrivate*> views)
		{
			fbs_tar.clear();
			dss_tar.clear();

			img_bk.reset();
			for (auto i = 0; i < size(fbs_bk); i++)
				fbs_bk[i].reset();

			if (views.empty())
				target_size = 0.f;
			else
			{
				auto sp = device->sampler_nearest.get();

				target_size = views[0]->image->size;
				views_tar.resize(views.size());
				fbs_tar.resize(views.size());
				dss_tar.resize(views.size());
				auto cb = std::make_unique<CommandBufferPrivate>(device->graphics_command_pool.get());
				cb->begin(true);
				for (auto i = 0; i < views.size(); i++)
				{
					cb->change_image_layout(views[i]->image, ImageLayoutUndefined, ImageLayoutPresent);
					views_tar[i] = views[i];
					fbs_tar[i].reset(new FramebufferPrivate(device, rp_tar, { &views[i], 1 }));
					dss_tar[i].reset(new DescriptorSetPrivate(device->descriptor_pool.get(), dsl_1_img));
					dss_tar[i]->set_image(0, 0, views[i], sp);
				}
				cb->end();
				auto q = device->graphics_queue.get();
				q->submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
				q->wait_idle();

				img_bk.reset(new ImagePrivate(device, Format_R8G8B8A8_UNORM, target_size, downsample_level, 1, SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
				for (auto i = 0; i < size(fbs_bk); i++)
				{
					ivs_bk[i].reset(new ImageViewPrivate(img_bk.get(), ImageView2D, i, 1));
					auto iv = ivs_bk[i].get();
					fbs_bk[i].reset(new FramebufferPrivate(device, rp_bk, { &iv, 1 }));
					dss_bk[i].reset(new DescriptorSetPrivate(device->descriptor_pool.get(), dsl_1_img));
					dss_bk[i]->set_image(0, 0, iv, sp);
				}
			}
		}

		uint CanvasPrivate::set_resource(int slot, ImageViewPrivate* v, SamplerPrivate* sp, const std::string& name, ImageAtlasPrivate* image_atlas, FontAtlasPrivate* font_atlas)
		{
			if (resources.empty())
				return -1;

			auto iv_white = img_white->default_view.get();

			if (slot == -1)
			{
				assert(v);
				slot = white_slot;
				white_slot = -1;
			}

			auto r = new CanvasResourcePrivate;
			ds_el->set_image(0, slot, v, sp ? sp : device->sampler_linear.get());
			r->name = name;
			r->view = v ? v : iv_white;
			r->image_atlas = image_atlas;
			r->font_atlas = font_atlas;
			resources[slot].reset(r);

			if (white_slot == -1)
			{
				for (auto i = 0; i < resources.size(); i++)
				{
					if (resources[i]->view == iv_white)
					{
						white_slot = i;
						break;
					}
				}
				assert(white_slot != -1);
			}

			return slot;
		}

		void CanvasPrivate::add_draw_cmd(uint id)
		{
			auto equal = [&]() {
				if (cmds.empty())
					return false;
				auto& back = cmds.back();
				if (back.type == CmdDrawElement && back.v.d1.id == id)
					return true;
				return false;
			};
			if (equal())
				return;

			Cmd cmd;
			cmd.type = CmdDrawElement;
			cmd.v.d1.id = id;
			cmd.v.d1.vtx_cnt = 0;
			cmd.v.d1.idx_cnt = 0;
			cmds.push_back(cmd);

			auto& d = cmds.back().v.d1;
			p_vtx_cnt = &d.vtx_cnt;
			p_idx_cnt = &d.idx_cnt;
		}

		void CanvasPrivate::add_vtx(const Vec2f& pos, const Vec2f& uv, const Vec4c& col)
		{
			auto& v = *vtx_end;
			v.pos = pos; 
			v.uv = uv;
			v.col = col; 
			vtx_end++;

			(*p_vtx_cnt)++;
		}

		void CanvasPrivate::add_idx(uint idx)
		{
			*idx_end = idx;
			idx_end++;

			(*p_idx_cnt)++;
		}

		void CanvasPrivate::begin_path()
		{
			paths.clear();
		}

		void CanvasPrivate::move_to(const Vec2f& pos)
		{
			if (paths.empty())
				paths.emplace_back();
			if (paths.back().empty())
			{
				paths.back().push_back(pos);
				return;
			}
			paths.emplace_back();
			paths.back().push_back(pos);
		}

		void CanvasPrivate::line_to(const Vec2f& pos)
		{
			paths.back().push_back(pos);
		}

		void CanvasPrivate::close_path()
		{
			paths.back().push_back(paths.back().front());
		}

		static const auto feather = 1.f;

		static std::vector<Vec2f> calculate_normals(const std::vector<Vec2f>& points, bool closed)
		{
			std::vector<Vec2f> normals(points.size());
			for (auto i = 0; i < points.size() - 1; i++)
			{
				auto d = normalize(points[i + 1] - points[i]);
				auto normal = Vec2f(d.y(), -d.x());

				if (i > 0)
				{
					auto n = normalize((normal + normals[i]) * 0.5f);
					normals[i] = n / dot(n, normal);
				}
				else
					normals[i] = normal;

				if (closed && i + 1 == points.size() - 1)
				{
					auto n = normalize((normal + normals[0]) * 0.5f);
					normals.front() = normals.back() = n / dot(n, normal);
				}
				else
					normals[i + 1] = normal;
			}
			return normals;
		}

		void CanvasPrivate::stroke(const Vec4c& col, float thickness, bool aa)
		{
			thickness *= 0.5f;

			add_draw_cmd(white_slot);
			auto uv = Vec2f(0.5f);

			for (auto& path : paths)
			{
				auto points = path;
				if (points.size() < 2)
					return;

				auto vtx_cnt0 = *p_vtx_cnt;

				auto closed = points[0] == points[points.size() - 1];
				auto normals = calculate_normals(points, closed);

				if (aa)
				{
					auto col_c = col;
					col_c.a() *= min(thickness / feather, 1.f);
					auto col_t = col;
					col_t.a() = 0;

					if (thickness > feather)
					{
						auto edge = thickness - feather;

						for (auto i = 0; i < points.size() - 1; i++)
						{
							if (i == 0)
							{
								auto p0 = points[0];
								auto p1 = points[1];

								auto n0 = normals[0];
								auto n1 = normals[1];

								auto vtx_cnt = *p_vtx_cnt;

								add_vtx(p0 + n0 * thickness, uv, col_t);
								add_vtx(p0 + n0 * edge, uv, col_c);
								add_vtx(p0 - n0 * edge, uv, col_c);
								add_vtx(p0 - n0 * thickness, uv, col_t);
								add_vtx(p1 + n1 * thickness, uv, col_t);
								add_vtx(p1 + n1 * edge, uv, col_c);
								add_vtx(p1 - n1 * edge, uv, col_c);
								add_vtx(p1 - n1 * thickness, uv, col_t);
								add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 6); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 5); add_idx(vtx_cnt + 6);
								add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 5); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 4); add_idx(vtx_cnt + 5);
								add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 7); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 6); add_idx(vtx_cnt + 7);
							}
							else if (closed && i == points.size() - 2)
							{
								auto vtx_cnt = *p_vtx_cnt;

								add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 2); add_idx(vtx_cnt - 2); add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt0 + 2);
								add_idx(vtx_cnt - 4); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 3); add_idx(vtx_cnt - 4); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
								add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 3); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 2); add_idx(vtx_cnt0 + 3);
							}
							else
							{
								auto p1 = points[i + 1];

								auto n1 = normals[i + 1];

								auto vtx_cnt = *p_vtx_cnt;

								add_vtx(p1 + n1 * thickness, uv, col_t);
								add_vtx(p1 + n1 * edge, uv, col_c);
								add_vtx(p1 - n1 * edge, uv, col_c);
								add_vtx(p1 - n1 * thickness, uv, col_t);
								add_idx(vtx_cnt - 3); add_idx(vtx_cnt + 2); add_idx(vtx_cnt - 2); add_idx(vtx_cnt - 3); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 2);
								add_idx(vtx_cnt - 4); add_idx(vtx_cnt + 1); add_idx(vtx_cnt - 3); add_idx(vtx_cnt - 4); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 1);
								add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 3); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 3);
							}
						}
					}
					else
					{
						for (auto i = 0; i < points.size() - 1; i++)
						{
							if (i == 0)
							{
								auto p0 = points[0];
								auto p1 = points[1];

								auto n0 = normals[0];
								auto n1 = normals[1];

								auto vtx_cnt = *p_vtx_cnt;

								vtx_cnt = *p_vtx_cnt;

								add_vtx(p0 + n0 * feather, uv, col_t);
								add_vtx(p0, uv, col_c);
								add_vtx(p0 - n0 * feather, uv, col_t);
								add_vtx(p1 + n1 * feather, uv, col_t);
								add_vtx(p1, uv, col_c);
								add_vtx(p1 - n1 * feather, uv, col_t);
								add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 4); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 4);
								add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 5); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 4); add_idx(vtx_cnt + 5);
							}
							else if (closed && i == points.size() - 2)
							{
								auto vtx_cnt = *p_vtx_cnt;

								add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
								add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 2); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt0 + 2);
							}
							else
							{
								auto p1 = points[i + 1];

								auto n1 = normals[i + 1];

								auto vtx_cnt = *p_vtx_cnt;

								add_vtx(p1 + n1 * feather, uv, col_t);
								add_vtx(p1, uv, col_c);
								add_vtx(p1 - n1 * feather, uv, col_t);
								add_idx(vtx_cnt - 3); add_idx(vtx_cnt + 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt - 3); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 1);
								add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 2); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 2);
							}
						}
					}

					if (!closed)
					{
						auto ext = max(feather, thickness);

						{
							auto vtx_cnt = *p_vtx_cnt;

							auto p0 = points[0];
							auto p1 = points[1];

							auto n0 = normals[0];

							auto p = p0 - normalize(p1 - p0);
							add_vtx(p + n0 * ext, uv, col_t);
							add_vtx(p - n0 * ext, uv, col_t);
							add_vtx(p0 + n0 * ext, uv, col_t);
							add_vtx(p0, uv, col_c);
							add_vtx(p0 - n0 * ext, uv, col_t);
							add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 4); add_idx(vtx_cnt + 2);
							add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 4); 
							add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 0);
						}

						{
							auto vtx_cnt = *p_vtx_cnt;

							auto p0 = points[points.size() - 2];
							auto p1 = points[points.size() - 1];

							auto n1 = normals[points.size() - 1];

							auto p = p1 + normalize(p1 - p0);
							add_vtx(p1 + n1 * ext, uv, col_t);
							add_vtx(p1, uv, col_c);
							add_vtx(p1 - n1 * ext, uv, col_t);
							add_vtx(p + n1 * ext, uv, col_t);
							add_vtx(p - n1 * ext, uv, col_t);
							add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 4); add_idx(vtx_cnt + 2);
							add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 4);
							add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3);
						}
					}
				}
				else
				{
					for (auto i = 0; i < points.size() - 1; i++)
					{
						if (i == 0)
						{
							auto p0 = points[0];
							auto p1 = points[1];

							auto n0 = normals[0];
							auto n1 = normals[1];

							auto vtx_cnt = *p_vtx_cnt;

							add_vtx(p0 + n0 * thickness, uv, col);
							add_vtx(p0 - n0 * thickness, uv, col);
							add_vtx(p1 + n1 * thickness, uv, col);
							add_vtx(p1 - n1 * thickness, uv, col);
							add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 3);
						}
						else if (closed && i == points.size() - 2)
						{
							auto vtx_cnt = *p_vtx_cnt;

							add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
						}
						else
						{
							auto p1 = points[i + 1];

							auto n1 = normals[i + 1];

							auto vtx_cnt = *p_vtx_cnt;

							add_vtx(p1 + n1 * thickness, uv, col);
							add_vtx(p1 - n1 * thickness, uv, col);
							add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 1);
						}
					}
				}
			}
		}

		void CanvasPrivate::fill(const Vec4c& col, bool aa)
		{
			add_draw_cmd(white_slot);
			auto uv = Vec2f(0.5f);

			for (auto& path : paths)
			{
				auto points = path;
				if (points.size() < 3)
					return;

				for (auto i = 0; i < points.size() - 2; i++)
				{
					auto vtx_cnt = *p_vtx_cnt;

					add_vtx(points[0], uv, col);
					add_vtx(points[i + 1], uv, col);
					add_vtx(points[i + 2], uv, col);
					add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1);
				}

				if (aa)
				{
					auto vtx_cnt0 = *p_vtx_cnt;
					auto _feather = feather * 2.f;

					points.push_back(points.front());
					auto normals = calculate_normals(points, true);

					auto col_t = col;
					col_t.a() = 0;

					for (auto i = 0; i < points.size() - 1; i++)
					{
						if (i == 0)
						{
							auto p0 = points[0];
							auto p1 = points[1];

							auto n0 = normals[0];
							auto n1 = normals[1];

							auto vtx_cnt = *p_vtx_cnt;

							add_vtx(p0, uv, col);
							add_vtx(p0 - n0 * _feather, uv, col_t);
							add_vtx(p1, uv, col);
							add_vtx(p1 - n1 * _feather, uv, col_t);
							add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 3);
						}
						else if (i == points.size() - 2)
						{
							auto vtx_cnt = *p_vtx_cnt;

							add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
						}
						else
						{
							auto p1 = points[i + 1];

							auto n1 = normals[i + 1];

							auto vtx_cnt = *p_vtx_cnt;

							add_vtx(p1, uv, col);
							add_vtx(p1 - n1 * _feather, uv, col_t);
							add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 1);
						}
					}
				}
			}
		}

		void CanvasPrivate::add_text(uint res_id, const wchar_t* text_beg, const wchar_t* text_end, uint font_size, const Vec4c& col, const Vec2f& pos, const Mat2f& axes)
		{
			auto atlas = resources[res_id]->font_atlas;

			add_draw_cmd(res_id);

			auto p = Vec2f(0.f);

			auto ptext = text_beg;
			while ((!text_end || ptext != text_end) && *ptext)
			{
				auto ch = *ptext;
				if (!ch)
					break;
				if (ch == '\n')
				{
					p.y() += font_size;
					p.x() = 0.f;
				}
				else if (ch != '\r')
				{
					if (ch == '\t')
						ch = ' ';

					auto g = atlas->get_glyph(ch, font_size);
					auto o = p + Vec2f(g->off);
					auto s = Vec2f(g->size);
					auto uv = g->uv;
					auto uv0 = Vec2f(uv.x(), uv.y());
					auto uv1 = Vec2f(uv.z(), uv.w());

					auto vtx_cnt = *p_vtx_cnt;

					add_vtx(pos + axes * o, uv0, col);
					add_vtx(pos + axes * (o + Vec2f(0.f, -s.y())), Vec2f(uv0.x(), uv1.y()), col);
					add_vtx(pos + axes * (o + Vec2f(s.x(), -s.y())), uv1, col);
					add_vtx(pos + axes * (o + Vec2f(s.x(), 0.f)), Vec2f(uv1.x(), uv0.y()), col);
					add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 2);

					p.x() += g->advance;
				}

				ptext++;
			}
		}

		void CanvasPrivate::add_image(uint res_id, uint tile_id, const Vec2f& LT, const Vec2f& RT, const Vec2f& RB, const Vec2f& LB, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col)
		{
			res_id = min(res_id, resources_count - 1);
			auto atlas = resources[res_id]->image_atlas;
			auto _uv0 = uv0;
			auto _uv1 = uv1;
			if (atlas)
			{
				auto tile = atlas->tiles[tile_id].get();
				auto tuv = tile->uv;
				auto tuv0 = Vec2f(tuv.x(), tuv.y());
				auto tuv1 = Vec2f(tuv.z(), tuv.w());
				_uv0 = mix(tuv0, tuv1, uv0);
				_uv1 = mix(tuv0, tuv1, uv1);
			}

			add_draw_cmd(res_id);

			auto vtx_cnt = *p_vtx_cnt;

			add_vtx(LT, _uv0, tint_col);
			add_vtx(RT, Vec2f(_uv1.x(), _uv0.y()), tint_col);
			add_vtx(RB, _uv1, tint_col);
			add_vtx(LB, Vec2f(_uv0.x(), _uv1.y()), tint_col);
			add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 2);
		}

		void CanvasPrivate::add_blur(const Vec4f& _range, float sigma)
		{
			auto range = Vec4f(
				max(_range.x(), 0.f),
				max(_range.y(), 0.f),
				min(_range.z(), (float)target_size.x()),
				min(_range.w(), (float)target_size.y()));
			Cmd cmd;
			cmd.type = CmdBlur;
			cmd.v.d2.scissor = range;
			cmd.v.d2.sigma = sigma;
			cmds.push_back(cmd);
		}

		void CanvasPrivate::set_scissor(const Vec4f& _scissor)
		{
			auto scissor = Vec4f(
				max(_scissor.x(), 0.f),
				max(_scissor.y(), 0.f),
				min(_scissor.z(), (float)target_size.x()),
				min(_scissor.w(), (float)target_size.y()));
			if (scissor == curr_scissor)
				return;
			curr_scissor = scissor;
			Cmd cmd;
			cmd.type = CmdSetScissor;
			cmd.v.d2.scissor = scissor;
			cmds.push_back(cmd);
		}

		void CanvasPrivate::prepare()
		{
			vtx_end = (Vertex*)buf_vtx->get_mapped();
			idx_end = (uint*)buf_idx->get_mapped();

			curr_scissor = Vec4f(Vec2f(0.f), Vec2f(target_size));

			cmds.clear();
		}

		void CanvasPrivate::record(CommandBufferPrivate* cb, uint image_index)
		{
			Cmd cmd;
			cmd.type = CmdBlur;
			cmd.v.d2.scissor = Vec4f(100.f, 100.f, 200.f, 200.f);
			cmd.v.d2.sigma = 1.f;
			cmds.push_back(cmd);

			struct Pass
			{
				int type;
				std::vector<int> cmd_ids;
			};
			std::vector<Pass> passes;
			for (auto i = 0; i < cmds.size(); i++)
			{
				switch (cmds[i].type)
				{
				case CmdDrawElement:
				case CmdSetScissor:
				{
					Pass* p;
					if (passes.empty() || passes.back().type != 0)
						passes.emplace_back();
					p = &passes.back();
					p->cmd_ids.push_back(i);

				}
					break;
				case CmdBlur:
				{
					Pass p;
					p.type = 1;
					p.cmd_ids.push_back(i);
					passes.push_back(p);
				}
					break;
				}
			}

			cb->begin();

			auto tar = views_tar[image_index];

			cb->change_image_layout(tar->image, ImageLayoutPresent, ImageLayoutTransferDst,
				tar->base_level, tar->level_count, tar->base_layer, tar->layer_count);
			cb->clear_image(tar->image, clear_color);

			cb->set_viewport(curr_scissor);
			cb->set_scissor(curr_scissor);
			cb->bind_vertex_buffer(buf_vtx.get(), 0);
			cb->bind_index_buffer(buf_idx.get(), IndiceTypeUint);
			auto vtx_off = 0;
			auto idx_off = 0;
			for (auto& p : passes)
			{
				switch (p.type)
				{
				case 0:
				{
					cb->begin_renderpass(fbs_tar[image_index].get());
					cb->bind_pipeline(pl_el);
					auto scale = Vec2f(2.f / target_size.x(), 2.f / target_size.y());
					cb->push_constant(0, sizeof(Vec2f), &scale, pll_el);
					cb->bind_descriptor_set(ds_el.get(), 0, pll_el);
					for (auto& i : p.cmd_ids)
					{
						auto& cmd = cmds[i];
						switch (cmd.type)
						{
						case CmdDrawElement:
							if (cmd.v.d1.idx_cnt > 0)
							{
								cb->draw_indexed(cmd.v.d1.idx_cnt, idx_off, vtx_off, 1, cmd.v.d1.id);
								vtx_off += cmd.v.d1.vtx_cnt;
								idx_off += cmd.v.d1.idx_cnt;
							}
							break;
						case CmdSetScissor:
							cb->set_scissor(cmd.v.d2.scissor);
							break;
						}
					}
					cb->end_renderpass();
				}
					break;
				case 1:
				{
					auto range = cmd.v.d2.scissor;
					cb->push_constant(0, sizeof(Vec4f), &range, pll_blur);

					//Vec4f kernel;
					//auto a = 1.f;
					//auto a2 = a * a;
					//for (auto i = 0; i < 4; i++)
					//	kernel[i] = exp(-(i * i) / (2.f * a2)) / sqrt(2.f * M_PI * a2);
					//auto s = kernel[0];
					//for (auto i = 1; i < 4; i++)
					//	s += kernel[i] * 2.f;
					//for (auto i = 0; i < 4; i++)
					//	kernel[i] /= s;
					//cb->push_constant(0, sizeof(Vec4f), &kernel, pll_blur);

					//cb->change_image_layout(tar->image, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
					//cb->begin_renderpass(fb_bk.get());
					//cb->bind_pipeline(pl_blurh);
					//cb->bind_descriptor_set(ds_tar[image_index].get(), 0, pll_blur);
					//cb->draw(3, 1, 0, 0);
					//cb->end_renderpass();

					//cb->change_image_layout(img_bk.get(), ImageLayoutUndefined, ImageLayoutShaderReadOnly);
					//cb->begin_renderpass(fbs_tar[image_index].get());
					//cb->bind_pipeline(pl_blurv);
					//cb->bind_descriptor_set(ds_bk.get(), 0, pll_blur);
					//cb->draw(3, 1, 0, 0);
					//cb->end_renderpass();
				}
					break;
				}
			}

			cb->change_image_layout(tar->image, ImageLayoutShaderReadOnly, ImageLayoutPresent,
				tar->base_level, tar->level_count, tar->base_layer, tar->layer_count);

			cb->end();

			cmds.clear();
		}

		Canvas* Canvas::create(Device* d)
		{
			return new CanvasPrivate((DevicePrivate*)d);
		}
	}
}
