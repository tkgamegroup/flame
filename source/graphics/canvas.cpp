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

		static auto initialized = false;

		static RenderpassPrivate* one_image_renderpass = nullptr;
		static RenderpassPrivate* forward_renderpass = nullptr;
		static DescriptorSetLayoutPrivate* element_descriptorsetlayout = nullptr;
		static DescriptorSetLayoutPrivate* forward_descriptorsetlayout = nullptr;
		static DescriptorSetLayoutPrivate* one_image_descriptorsetlayout = nullptr;
		static PipelineLayoutPrivate* element_pipelinelayout = nullptr;
		static PipelineLayoutPrivate* forward_pipelinelayout = nullptr;
		static PipelineLayoutPrivate* blt_pipelinelayout = nullptr;
		static PipelineLayoutPrivate* blur_pipelinelayout = nullptr;
		static PipelineLayoutPrivate* downsample_pipelinelayout = nullptr;
		static PipelinePrivate* element_pipeline = nullptr;
		static PipelinePrivate* forward_pipeline = nullptr;
		static PipelinePrivate* blt_pipeline = nullptr;
		static PipelinePrivate* blurh_pipeline[10] = {};
		static PipelinePrivate* blurv_pipeline[10] = {};
		static PipelinePrivate* downsample_pipeline = nullptr;

		CanvasPrivate::CanvasPrivate(DevicePrivate* d) :
			device(d)
		{
			if (!initialized)
			{
				initialized = true;

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
					one_image_renderpass = new RenderpassPrivate(d, { &att, 1 }, { &sp,1 });
				}

				{
					RenderpassAttachmentInfo atts[2];
					atts[0].format = Swapchain::get_format();
					atts[0].clear = false;
					atts[1].format = Format_Depth16;
					RenderpassSubpassInfo sp;
					uint col_refs[] = {
						0
					};
					sp.color_attachments_count = 1;
					sp.color_attachments = col_refs;
					sp.depth_attachment = 1;
					forward_renderpass = new RenderpassPrivate(d, atts, { &sp,1 });
				}

				{
					DescriptorBindingInfo db;
					db.type = DescriptorSampledImage;
					db.count = resources_count;
					db.name = "images";
					element_descriptorsetlayout = new DescriptorSetLayoutPrivate(d, { &db, 1 });
				}

				{
					forward_descriptorsetlayout = new DescriptorSetLayoutPrivate(d, {});
				}

				{
					DescriptorBindingInfo db;
					db.type = DescriptorSampledImage;
					db.count = 1;
					db.name = "image";
					one_image_descriptorsetlayout = new DescriptorSetLayoutPrivate(d, { &db, 1 });
				}

				element_pipelinelayout = new PipelineLayoutPrivate(d, { &element_descriptorsetlayout, 1 }, sizeof(Vec4f));
				forward_pipelinelayout = new PipelineLayoutPrivate(d, { &forward_descriptorsetlayout, 1 }, 0);
				blt_pipelinelayout = new PipelineLayoutPrivate(d, { &one_image_descriptorsetlayout, 1 }, sizeof(Vec4f));
				blur_pipelinelayout = new PipelineLayoutPrivate(d, { &one_image_descriptorsetlayout, 1 }, 0);
				downsample_pipelinelayout = new PipelineLayoutPrivate(d, { &one_image_descriptorsetlayout, 1 }, 0);

				{
					VertexAttributeInfo vias[3];
					vias[0].location = 0;
					vias[0].format = Format_R32G32_SFLOAT;
					vias[0].name = "pos";
					vias[1].location = 1;
					vias[1].format = Format_R32G32_SFLOAT;
					vias[1].name = "uv";
					vias[2].location = 2;
					vias[2].format = Format_R8G8B8A8_UNORM;
					vias[2].name = "color";
					VertexBufferInfo vib;
					vib.attributes_count = size(vias);
					vib.attributes = vias;
					VertexInfo vi;
					vi.buffers_count = 1;
					vi.buffers = &vib;
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"element.vert"),
						new ShaderPrivate(d, L"element.frag")
					};
					BlendOption bo;
					bo.enable = true;
					bo.src_color = BlendFactorSrcAlpha;
					bo.dst_color = BlendFactorOneMinusSrcAlpha;
					bo.dst_alpha = BlendFactorOne;
					element_pipeline = PipelinePrivate::create(d, shaders, element_pipelinelayout, one_image_renderpass, 0, &vi, Vec2u(0), nullptr, SampleCount_1,
						nullptr, { &bo, 1 });
				}

				{
					VertexAttributeInfo vias1[1];
					vias1[0].location = 0;
					vias1[0].format = Format_R32G32B32_SFLOAT;
					vias1[0].name = "pos";
					VertexAttributeInfo vias2[4];
					vias2[0].location = 1;
					vias2[0].format = Format_R32G32B32A32_SFLOAT;
					vias2[0].name = "col0";
					vias2[1].location = 2;
					vias2[1].format = Format_R32G32B32A32_SFLOAT;
					vias2[1].name = "col1";
					vias2[2].location = 3;
					vias2[2].format = Format_R32G32B32A32_SFLOAT;
					vias2[2].name = "col2";
					vias2[3].location = 4;
					vias2[3].format = Format_R32G32B32A32_SFLOAT;
					vias2[3].name = "col3";
					VertexBufferInfo vibs[2];
					vibs[0].attributes_count = size(vias1);
					vibs[0].attributes = vias1;
					vibs[1].attributes_count = size(vias2);
					vibs[1].attributes = vias2;
					vibs[1].rate = VertexInputRateInstance;
					VertexInfo vi;
					vi.buffers_count = size(vibs);
					vi.buffers = vibs;
					DepthInfo dep;
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"forward.vert"),
						new ShaderPrivate(d, L"forward.frag")
					};
					forward_pipeline = PipelinePrivate::create(d, shaders, forward_pipelinelayout, forward_renderpass, 0, &vi, Vec2u(0), nullptr, SampleCount_1,
						&dep);
				}

				{
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"fullscreen.vert"),
						new ShaderPrivate(d, L"blt.frag")
					};
					blt_pipeline = PipelinePrivate::create(d, shaders, blt_pipelinelayout, one_image_renderpass, 0);
				}

				for (auto i = 0; i < 10; i++)
				{
					{
						ShaderPrivate* shaders[] = {
							new ShaderPrivate(d, L"fullscreen.vert", "NO_COORD"),
							new ShaderPrivate(d, L"blur.frag", "R" + std::to_string(i + 1) + " H\n")
						};
						blurh_pipeline[i] = PipelinePrivate::create(d, shaders, blur_pipelinelayout, one_image_renderpass, 0);
					}

					{
						ShaderPrivate* shaders[] = {
							new ShaderPrivate(d, L"fullscreen.vert", "NO_COORD"),
							new ShaderPrivate(d, L"blur.frag", "R" + std::to_string(i + 1) + " V\n")
						};
						blurv_pipeline[i] = PipelinePrivate::create(d, shaders, blur_pipelinelayout, one_image_renderpass, 0);
					}
				}

				{
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"fullscreen.vert", "NO_COORD"),
						new ShaderPrivate(d, L"downsample.frag")
					};
					downsample_pipeline = PipelinePrivate::create(d, shaders, downsample_pipelinelayout, one_image_renderpass, 0);
				}
			}

			element_vertex_buffer.reset(new BufferPrivate(d, 360000 * sizeof(ElementVertex), BufferUsageTransferDst | BufferUsageVertex, MemoryPropertyDevice));
			element_vertex_staging_buffer.reset(new BufferPrivate(d, element_vertex_buffer->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
			element_index_buffer.reset(new BufferPrivate(d, 240000 * sizeof(uint), BufferUsageTransferDst | BufferUsageIndex, MemoryPropertyDevice));
			element_index_staging_buffer.reset(new BufferPrivate(d, element_index_buffer->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
			model_vertex_buffer.reset(new BufferPrivate(d, 600000 * sizeof(ModelVertex), BufferUsageTransferDst | BufferUsageVertex, MemoryPropertyDevice));
			model_vertex_staging_buffer.reset(new BufferPrivate(d, model_vertex_buffer->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
			model_index_buffer.reset(new BufferPrivate(d, 400000 * sizeof(uint), BufferUsageTransferDst | BufferUsageIndex, MemoryPropertyDevice));
			model_index_staging_buffer.reset(new BufferPrivate(d, model_index_buffer->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
			object_matrix_buffer.reset(new BufferPrivate(d, 10000 * sizeof(Mat4f), BufferUsageTransferDst | BufferUsageVertex, MemoryPropertyDevice));
			object_matrix_staging_buffer.reset(new BufferPrivate(d, object_matrix_buffer->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
			object_indirect_buffer.reset(new BufferPrivate(d, 10000 * sizeof(DrawIndexedIndirectCommand), BufferUsageTransferDst | BufferUsageIndirect, MemoryPropertyDevice));
			object_indirect_staging_buffer.reset(new BufferPrivate(d, object_indirect_buffer->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
			element_vertex_staging_buffer->map();
			element_index_staging_buffer->map();
			model_vertex_staging_buffer->map();
			model_index_staging_buffer->map();
			object_matrix_staging_buffer->map();
			object_indirect_staging_buffer->map();
			{
				Vec3f ps[] = {
					Vec3f(-0.5f, 0.5f, -0.5f), Vec3f(0.5f, 0.5f, -0.5f), Vec3f(0.5f, -0.5f, -0.5f), Vec3f(-0.5f, -0.5f, -0.5f),
					Vec3f(-0.5f, 0.5f, 0.5f), Vec3f(0.5f, 0.5f, 0.5f), Vec3f(0.5f, -0.5f, 0.5f), Vec3f(-0.5f, -0.5f, 0.5f)
				};
				uint is[] = {
					0, 2, 1, 0, 3, 2,
					5, 7, 4, 5, 6, 7,
					4, 1, 5, 4, 0, 1,
					3, 6, 2, 3, 7, 6,
					4, 3, 0, 4, 7, 3,
					1, 6, 5, 1, 2, 6
				};
				memcpy(model_vertex_staging_buffer->mapped, ps, sizeof(Vec3f)* size(ps));
				memcpy(model_index_staging_buffer->mapped, is, sizeof(uint)* size(is));

				auto cb = std::make_unique<CommandBufferPrivate>(d->graphics_command_pool.get());
				cb->begin(true);
				BufferCopy cpy1;
				cpy1.size = sizeof(Vec3f) * size(ps);
				BufferCopy cpy2;
				cpy2.size = sizeof(uint) * size(is);
				cb->copy_buffer(model_vertex_staging_buffer.get(), model_vertex_buffer.get(), { &cpy1, 1 });
				cb->copy_buffer(model_index_staging_buffer.get(), model_index_buffer.get(), { &cpy2, 1 });
				cb->end();
				auto q = d->graphics_queue.get();
				q->submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
				q->wait_idle();
			}

			white_image.reset(new ImagePrivate(d, Format_R8G8B8A8_UNORM, Vec2u(1), 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled));
			white_image->clear(ImageLayoutUndefined, ImageLayoutShaderReadOnly, Vec4c(255));
			resources.resize(resources_count);
			auto iv_white = white_image->default_view.get();
			for (auto i = 0; i < resources_count; i++)
			{
				auto r = new CanvasResourcePrivate;
				r->view = iv_white;
				resources[i].reset(r);
			}

			element_descriptorset.reset(new DescriptorSetPrivate(d->descriptor_pool.get(), element_descriptorsetlayout));
			auto sp = d->sampler_linear.get();
			for (auto i = 0; i < resources_count; i++)
				element_descriptorset->set_image(0, i, iv_white, sp);
		}

		void CanvasPrivate::set_target(std::span<ImageViewPrivate*> views)
		{
			target_imageviews.clear();
			target_framebuffers.clear();
			target_descriptors.clear();

			depth_image.reset();
			forward_framebuffers.clear();

			for (auto i = 0; i < 2; i++)
			{
				back_images[i].reset();
				for (auto j = 0; j < downsample_level; j++)
				{
					back_imageviews[i][j].reset();
					back_framebuffers[i][j].reset();
					back_descriptorsets[i][j].reset();
				}
			}

			if (views.empty())
				target_size = 0.f;
			else
			{
				auto sp = device->sampler_nearest.get();

				target_size = views[0]->image->size;
				target_imageviews.resize(views.size());
				target_framebuffers.resize(views.size());
				target_descriptors.resize(views.size());
				auto cb = std::make_unique<CommandBufferPrivate>(device->graphics_command_pool.get());
				cb->begin(true);
				for (auto i = 0; i < views.size(); i++)
				{
					cb->image_barrier(views[i]->image, ImageLayoutUndefined, ImageLayoutPresent);
					target_imageviews[i] = views[i];
					target_framebuffers[i].reset(new FramebufferPrivate(device, one_image_renderpass, { &views[i], 1 }));
					target_descriptors[i].reset(new DescriptorSetPrivate(device->descriptor_pool.get(), one_image_descriptorsetlayout));
					target_descriptors[i]->set_image(0, 0, views[i], sp);
				}
				cb->end();
				auto q = device->graphics_queue.get();
				q->submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
				q->wait_idle();

				depth_image.reset(new ImagePrivate(device, Format_Depth16, target_size, 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
				forward_framebuffers.resize(views.size());
				for (auto i = 0; i < views.size(); i++)
				{
					ImageViewPrivate* vs[2];
					vs[0] = views[i];
					vs[1] = depth_image->default_view.get();
					forward_framebuffers[i].reset(new FramebufferPrivate(device, forward_renderpass, vs));
				}

				for (auto i = 0; i < 2; i++)
				{
					back_images[i].reset(new ImagePrivate(device, Format_B8G8R8A8_UNORM, target_size, downsample_level, 1, SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
					for (auto j = 0; j < downsample_level; j++)
					{
						ImageSubresource sr;
						sr.base_level = i;
						back_imageviews[i][j].reset(new ImageViewPrivate(back_images[i].get(), ImageView2D, sr));
						auto iv = back_imageviews[i][j].get();
						back_framebuffers[i][j].reset(new FramebufferPrivate(device, one_image_renderpass, { &iv, 1 }));
						back_descriptorsets[i][j].reset(new DescriptorSetPrivate(device->descriptor_pool.get(), one_image_descriptorsetlayout));
						back_descriptorsets[i][j]->set_image(0, 0, iv, sp);
					}
				}
			}
		}

		uint CanvasPrivate::set_resource(int slot, ImageViewPrivate* v, SamplerPrivate* sp, const std::string& name, ImageAtlasPrivate* image_atlas, FontAtlasPrivate* font_atlas)
		{
			if (resources.empty())
				return -1;

			auto iv_white = white_image->default_view.get();

			if (slot == -1)
			{
				assert(v);
				slot = white_slot;
				white_slot = -1;
			}

			auto r = new CanvasResourcePrivate;
			element_descriptorset->set_image(0, slot, v, sp ? sp : device->sampler_linear.get());
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
		}

		void CanvasPrivate::add_vtx(const Vec2f& pos, const Vec2f& uv, const Vec4c& col)
		{
			auto& v = *element_vertex_buffer_end;
			v.pos = pos; 
			v.uv = uv;
			v.col = col; 
			element_vertex_buffer_end++;
			
			cmds.back().v.d1.vtx_cnt++;
		}

		void CanvasPrivate::add_idx(uint idx)
		{
			*element_index_buffer_end = idx;
			element_index_buffer_end++;

			cmds.back().v.d1.idx_cnt++;
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

				auto vtx_cnt0 = cmds.back().v.d1.vtx_cnt;

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

								auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

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
								auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

								add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 2); add_idx(vtx_cnt - 2); add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt0 + 2);
								add_idx(vtx_cnt - 4); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 3); add_idx(vtx_cnt - 4); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
								add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 3); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 2); add_idx(vtx_cnt0 + 3);
							}
							else
							{
								auto p1 = points[i + 1];

								auto n1 = normals[i + 1];

								auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

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

								auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

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
								auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

								add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
								add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 2); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt0 + 2);
							}
							else
							{
								auto p1 = points[i + 1];

								auto n1 = normals[i + 1];

								auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

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
							auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

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
							auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

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

							auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

							add_vtx(p0 + n0 * thickness, uv, col);
							add_vtx(p0 - n0 * thickness, uv, col);
							add_vtx(p1 + n1 * thickness, uv, col);
							add_vtx(p1 - n1 * thickness, uv, col);
							add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 3);
						}
						else if (closed && i == points.size() - 2)
						{
							auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

							add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
						}
						else
						{
							auto p1 = points[i + 1];

							auto n1 = normals[i + 1];

							auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

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
					auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

					add_vtx(points[0], uv, col);
					add_vtx(points[i + 1], uv, col);
					add_vtx(points[i + 2], uv, col);
					add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1);
				}

				if (aa)
				{
					auto vtx_cnt0 = cmds.back().v.d1.vtx_cnt;
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

							auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

							add_vtx(p0, uv, col);
							add_vtx(p0 - n0 * _feather, uv, col_t);
							add_vtx(p1, uv, col);
							add_vtx(p1 - n1 * _feather, uv, col_t);
							add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 3);
						}
						else if (i == points.size() - 2)
						{
							auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

							add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
						}
						else
						{
							auto p1 = points[i + 1];

							auto n1 = normals[i + 1];

							auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

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

					auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

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

		void CanvasPrivate::add_object(const Mat4f& mat, uint mod_id)
		{
			*object_matrix_buffer_end = mat;
			object_matrix_buffer_end++;

			object_indirect_buffer_end->index_count = 36;
			object_indirect_buffer_end->first_index = 0;
			object_indirect_buffer_end->vertex_offset = 0;
			object_indirect_buffer_end->first_instance = object_indirect_buffer_end - object_indirect_buffer->mapped;
			object_indirect_buffer_end++;

			Cmd cmd;
			cmd.type = CmdDrawObject;
			cmd.v.d3.matrix = mat;
			cmds.push_back(cmd);
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

			auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

			add_vtx(LT, _uv0, tint_col);
			add_vtx(RT, Vec2f(_uv1.x(), _uv0.y()), tint_col);
			add_vtx(RB, _uv1, tint_col);
			add_vtx(LB, Vec2f(_uv0.x(), _uv1.y()), tint_col);
			add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 2);
		}

		void CanvasPrivate::add_blur(const Vec4f& _range, uint radius)
		{
			auto range = Vec4f(
				max(_range.x(), 0.f),
				max(_range.y(), 0.f),
				min(_range.z(), (float)target_size.x()),
				min(_range.w(), (float)target_size.y()));
			Cmd cmd;
			cmd.type = CmdBlur;
			cmd.v.d2.scissor = range;
			cmd.v.d2.radius = radius;
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
			element_vertex_buffer_end = (ElementVertex*)element_vertex_staging_buffer->mapped;
			element_index_buffer_end = (uint*)element_index_staging_buffer->mapped;
			object_matrix_buffer_end = (Mat4f*)object_matrix_staging_buffer->mapped;
			object_indirect_buffer_end = (DrawIndexedIndirectCommand*)object_indirect_staging_buffer->mapped;

			curr_scissor = Vec4f(Vec2f(0.f), Vec2f(target_size));

			cmds.clear();
		}

		void CanvasPrivate::record(CommandBufferPrivate* cb, uint image_index)
		{
			enum PassType
			{
				PassElement,
				PassObject,
				PassBlur,
				PassBloom
			};
			struct Pass
			{
				PassType type;
				std::vector<int> cmd_ids;
			};
			std::vector<Pass> passes;

			auto mat = get_project_matrix(60.f * ANG_RAD, (float)target_size.x() / (float)target_size.y(), 0.1f, 1000.f) *
				get_view_matrix(Vec3f(3.f), Vec3f(0.f), Vec3f(0.f, 1.f, 0.f));
			add_object(mat, 0);

			for (auto i = 0; i < cmds.size(); i++)
			{
				switch (cmds[i].type)
				{
				case CmdDrawElement:
				case CmdSetScissor:
				{
					Pass* p;
					if (passes.empty() || passes.back().type != PassElement)
					{
						Pass p;
						p.type = PassElement;
						passes.push_back(p);
					}
					p = &passes.back();
					p->cmd_ids.push_back(i);

				}
					break;
				case CmdDrawObject:
				{
					Pass* p;
					if (passes.empty() || passes.back().type != PassObject)
					{
						Pass p;
						p.type = PassObject;
						passes.push_back(p);
					}
					p = &passes.back();
					p->cmd_ids.push_back(i);

				}
					break;
				case CmdBlur:
				{
					Pass p;
					p.type = PassBlur;
					p.cmd_ids.push_back(i);
					passes.push_back(p);
				}
					break;
				}
			}

			cb->begin();

			auto tar = target_imageviews[image_index];

			cb->image_barrier(tar->image, ImageLayoutPresent, ImageLayoutTransferDst);
			cb->clear_image(tar->image, clear_color);

			{
				BufferCopy cpy;
				cpy.size = (char*)element_vertex_buffer_end - element_vertex_staging_buffer->mapped;
				cb->copy_buffer(element_vertex_staging_buffer.get(), element_vertex_buffer.get(), { &cpy, 1 });
			}
			{
				BufferCopy cpy;
				cpy.size = (char*)element_index_buffer_end - element_index_staging_buffer->mapped;
				cb->copy_buffer(element_index_staging_buffer.get(), element_index_buffer.get(), { &cpy, 1 });
			}
			{
				BufferCopy cpy;
				cpy.size = (char*)object_matrix_buffer_end - object_matrix_staging_buffer->mapped;
				cb->copy_buffer(object_matrix_staging_buffer.get(), object_matrix_buffer.get(), { &cpy, 1 });
			}
			{
				BufferCopy cpy;
				cpy.size = (char*)object_indirect_buffer_end - object_indirect_staging_buffer->mapped;
				cb->copy_buffer(object_indirect_staging_buffer.get(), object_indirect_buffer.get(), { &cpy, 1 });
			}
			cb->buffer_barrier(element_vertex_buffer.get(), AccessTransferWrite, AccessVertexAttributeRead);
			cb->buffer_barrier(element_index_buffer.get(), AccessTransferWrite, AccessIndexRead);
			cb->buffer_barrier(object_matrix_buffer.get(), AccessTransferWrite, AccessIndexRead);
			cb->buffer_barrier(object_indirect_buffer.get(), AccessTransferWrite, AccessIndirectCommandRead);

			cb->set_viewport(curr_scissor);
			cb->set_scissor(curr_scissor);
			auto ele_vtx_off = 0;
			auto ele_idx_off = 0;
			auto obj_off = 0;

			for (auto& p : passes)
			{
				switch (p.type)
				{
				case PassElement:
				{
					cb->set_viewport(curr_scissor);
					cb->bind_vertex_buffer(element_vertex_buffer.get(), 0);
					cb->bind_index_buffer(element_index_buffer.get(), IndiceTypeUint);
					cb->begin_renderpass(target_framebuffers[image_index].get());
					cb->bind_pipeline(element_pipeline);
					auto scale = Vec2f(2.f / target_size.x(), 2.f / target_size.y());
					cb->push_constant(0, sizeof(Vec2f), &scale, element_pipelinelayout);
					cb->bind_descriptor_set(element_descriptorset.get(), 0, element_pipelinelayout);
					for (auto& i : p.cmd_ids)
					{
						auto& cmd = cmds[i];
						switch (cmd.type)
						{
						case CmdDrawElement:
							if (cmd.v.d1.idx_cnt > 0)
							{
								cb->draw_indexed(cmd.v.d1.idx_cnt, ele_idx_off, ele_vtx_off, 1, cmd.v.d1.id);
								ele_vtx_off += cmd.v.d1.vtx_cnt;
								ele_idx_off += cmd.v.d1.idx_cnt;
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
				case PassObject:
				{
					cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)target_size));
					cb->bind_vertex_buffer(model_vertex_buffer.get(), 0);
					cb->bind_vertex_buffer(object_matrix_buffer.get(), 1);
					cb->bind_index_buffer(model_index_buffer.get(), IndiceTypeUint);
					Vec4f cvs[2];
					cvs[1] = Vec4f(1.f);
					cb->begin_renderpass(forward_framebuffers[image_index].get(), cvs);
					cb->bind_pipeline(forward_pipeline);
					cb->draw_indexed_indirect(object_indirect_buffer.get(), obj_off, p.cmd_ids.size());
					obj_off += p.cmd_ids.size();
					cb->draw_indexed(36, 0, 0, 1, 0);
					cb->end_renderpass();
				}
					break;
				case PassBlur:
				{
					auto& cmd = cmds[p.cmd_ids[0]];
					auto blur_radius = clamp(cmd.v.d2.radius, 0U, 10U);
					auto blur_range = cmd.v.d2.scissor;
					auto blur_size = Vec2f(blur_range.z() - blur_range.x(), blur_range.w() - blur_range.y());
					if (blur_size.x() < 1.f || blur_size.y() < 1.f)
						continue;

					cb->image_barrier(tar->image, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
					cb->set_viewport(Vec4f(blur_range.x() - blur_radius, blur_range.y() - blur_radius,
						blur_range.z() + blur_radius, blur_range.w() + blur_radius));
					cb->begin_renderpass(back_framebuffers[0][0].get());
					cb->bind_pipeline(blurh_pipeline[blur_radius - 1]);
					cb->bind_descriptor_set(target_descriptors[image_index].get(), 0, blur_pipelinelayout);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->image_barrier(back_images[0].get(), ImageLayoutUndefined, ImageLayoutShaderReadOnly);
					cb->set_viewport(blur_range);
					cb->begin_renderpass(target_framebuffers[image_index].get());
					cb->bind_pipeline(blurv_pipeline[blur_radius - 1]);
					cb->bind_descriptor_set(back_descriptorsets[0][0].get(), 0, blur_pipelinelayout);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();
				}
					break;
				case PassBloom:
				{
					//auto blur_range = cmd.v.d2.scissor;
					//auto blur_size = Vec2f(blur_range.z() - blur_range.x(), blur_range.w() - blur_range.y());
					//if (blur_size.x() < 1.f || blur_size.y() < 1.f)
					//	continue;
					//auto target_range = Vec4f(0.f, 0.f, target_size.x(), target_size.y());
					//uint x, y, w, h;

					//cb->image_barrier(tar->image, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
					//cb->set_viewport(target_range);
					//cb->push_constant(0, sizeof(Vec4f), &target_range, pll_blt);
					//cb->begin_renderpass(fbs_bk[0].get());
					//cb->bind_pipeline(pl_blt);
					//cb->bind_descriptor_set(dss_tar[image_index].get(), 0, pll_blt);
					//cb->draw(3, 1, 0, 0);
					//cb->end_renderpass();

					//w = target_size.x();
					//h = target_size.y();
					//for (auto i = 1; i < downsample_level; i++)
					//{
					//	cb->image_barrier(img_bk.get(), ImageLayoutUndefined, ImageLayoutShaderReadOnly, i, 1);
					//	w = max(1U, w / 2); h = max(1U, h / 2);
					//	cb->set_viewport(Vec4f(0.f, 0.f, w, h));
					//	cb->begin_renderpass(fbs_bk[i].get());
					//	cb->bind_pipeline(pl_ds);
					//	cb->bind_descriptor_set(dss_bk[i - 1].get(), 0, pll_ds);
					//	cb->draw(3, 1, 0, 0);
					//	cb->end_renderpass();
					//}

					//Vec4f kernel;
					//auto a = 1.f; auto a2 = a * a;
					//for (auto i = 0; i < 4; i++)
					//	kernel[i] = exp(-(i * i) / (2.f * a2)) / sqrt(2.f * M_PI * a2);
					//auto sum = kernel[0];
					//for (auto i = 1; i < 4; i++)
					//	sum += kernel[i] * 2.f;
					//for (auto i = 0; i < 4; i++)
					//	kernel[i] /= sum;
					//cb->push_constant(0, sizeof(Vec4f), &kernel, pll_blur);

					//x = blur_range.x();
					//y = blur_range.y();
					//w = blur_size.x();
					//h = blur_size.y();
					//for (auto i = 0; i < downsample_level; i++)
					//{
					//	cb->image_barrier(img_bk.get(), ImageLayoutUndefined, ImageLayoutShaderReadOnly, i, 1);
					//	cb->set_viewport(Vec4f(x - 3.f, y - 3.f, w + 3.f, h + 3.f));
					//	cb->begin_renderpass(fbs_pp[i].get());
					//	cb->bind_pipeline(pl_blurh);
					//	cb->bind_descriptor_set(dss_bk[i].get(), 0, pll_blur);
					//	cb->draw(3, 1, 0, 0);
					//	cb->end_renderpass();
					//	x = x / 2; y = y / 2; w = max(1U, w / 2); h = max(1U, h / 2);
					//}

					//x = blur_range.x();
					//y = blur_range.y();
					//w = blur_size.x();
					//h = blur_size.y();
					//for (auto i = 0; i < downsample_level; i++)
					//{
					//	cb->image_barrier(img_pp.get(), ImageLayoutUndefined, ImageLayoutShaderReadOnly, i, 1);
					//	cb->set_viewport(Vec4f(x, y, w, h));
					//	cb->begin_renderpass(fbs_bk[i].get());
					//	cb->bind_pipeline(pl_blurv);
					//	cb->bind_descriptor_set(dss_pp[i].get(), 0, pll_blur);
					//	cb->draw(3, 1, 0, 0);
					//	cb->end_renderpass();
					//	x = x / 2; y = y / 2; w = max(1U, w / 2); h = max(1U, h / 2);
					//}
				}
					break;
				}
			}

			cb->image_barrier(tar->image, ImageLayoutShaderReadOnly, ImageLayoutPresent);

			cb->end();

			cmds.clear();
		}

		Canvas* Canvas::create(Device* d)
		{
			return new CanvasPrivate((DevicePrivate*)d);
		}
	}
}
