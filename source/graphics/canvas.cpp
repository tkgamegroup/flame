#include <flame/serialize.h>
#include "device_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "font_private.h"
#include "model_private.h"
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

		static RenderpassPrivate* one_image8_renderpass = nullptr;
		static RenderpassPrivate* one_image16_renderpass = nullptr;
		static RenderpassPrivate* forward8_renderpass = nullptr;
		static RenderpassPrivate* forward16_renderpass = nullptr;
		static DescriptorSetLayoutPrivate* element_descriptorsetlayout = nullptr;
		static DescriptorSetLayoutPrivate* forward_descriptorsetlayout = nullptr;
		static DescriptorSetLayoutPrivate* one_image_descriptorsetlayout = nullptr;
		static PipelineLayoutPrivate* element_pipelinelayout = nullptr;
		static PipelineLayoutPrivate* forward_pipelinelayout = nullptr;
		static PipelineLayoutPrivate* one_image_pc0_pipelinelayout = nullptr;
		static PipelineLayoutPrivate* one_image_pc4_pipelinelayout = nullptr;
		static PipelineLayoutPrivate* one_image_pc8_pipelinelayout = nullptr;
		static PipelinePrivate* element8_pipeline = nullptr;
		static PipelinePrivate* element16_pipeline = nullptr;
		static PipelinePrivate* forward8_pipeline = nullptr;
		static PipelinePrivate* forward16_pipeline = nullptr;
		static PipelinePrivate* blurh8_pipeline[10] = {};
		static PipelinePrivate* blurh16_pipeline[10] = {};
		static PipelinePrivate* blurv8_pipeline[10] = {};
		static PipelinePrivate* blurv16_pipeline[10] = {};
		static PipelinePrivate* blit8_pipeline = nullptr;
		static PipelinePrivate* blit16_pipeline = nullptr;
		static PipelinePrivate* filter_bright_pipeline = nullptr;
		static PipelinePrivate* downsample_pipeline = nullptr;
		static PipelinePrivate* upsample_pipeline = nullptr;

		CanvasPrivate::CanvasPrivate(DevicePrivate* d) :
			device(d)
		{
			if (!initialized)
			{
				initialized = true;

				{
					RenderpassAttachmentInfo att;
					att.format = Swapchain::get_format();
					att.load_op = AttachmentLoad;
					att.initia_layout = ImageLayoutShaderReadOnly;
					RenderpassSubpassInfo sp;
					uint col_refs[] = {
						0
					};
					sp.color_attachments_count = 1;
					sp.color_attachments = col_refs;
					one_image8_renderpass = new RenderpassPrivate(d, { &att, 1 }, { &sp,1 });
				}

				{
					RenderpassAttachmentInfo att;
					att.format = Format_R16G16B16A16_SFLOAT;
					att.load_op = AttachmentLoad;
					att.initia_layout = ImageLayoutShaderReadOnly;
					RenderpassSubpassInfo sp;
					uint col_refs[] = {
						0
					};
					sp.color_attachments_count = 1;
					sp.color_attachments = col_refs;
					one_image16_renderpass = new RenderpassPrivate(d, { &att, 1 }, { &sp,1 });
				}

				{
					RenderpassAttachmentInfo atts[2];
					atts[0].format = Swapchain::get_format();
					atts[0].load_op = AttachmentLoad;
					atts[0].initia_layout = ImageLayoutShaderReadOnly;
					atts[1].format = Format_Depth16;
					atts[1].load_op = AttachmentLoad;
					atts[1].initia_layout = ImageLayoutAttachment;
					atts[1].final_layout = ImageLayoutAttachment;
					RenderpassSubpassInfo sp;
					uint col_refs[] = {
						0
					};
					sp.color_attachments_count = 1;
					sp.color_attachments = col_refs;
					sp.depth_attachment = 1;
					forward8_renderpass = new RenderpassPrivate(d, atts, { &sp,1 });
				}

				{
					RenderpassAttachmentInfo atts[2];
					atts[0].format = Format_R16G16B16A16_SFLOAT;
					atts[0].load_op = AttachmentLoad;
					atts[0].initia_layout = ImageLayoutShaderReadOnly;
					atts[1].format = Format_Depth16;
					atts[1].load_op = AttachmentLoad;
					atts[1].initia_layout = ImageLayoutAttachment;
					atts[1].final_layout = ImageLayoutAttachment;
					RenderpassSubpassInfo sp;
					uint col_refs[] = {
						0
					};
					sp.color_attachments_count = 1;
					sp.color_attachments = col_refs;
					sp.depth_attachment = 1;
					forward16_renderpass = new RenderpassPrivate(d, atts, { &sp,1 });
				}

				{
					DescriptorBindingInfo db;
					db.type = DescriptorSampledImage;
					db.count = resources_count;
					element_descriptorsetlayout = new DescriptorSetLayoutPrivate(d, { &db, 1 });
				}

				{
					DescriptorBindingInfo dbs[5];
					dbs[0].type = DescriptorStorageBuffer;
					dbs[1].type = DescriptorStorageBuffer;
					dbs[2].type = DescriptorStorageBuffer;
					dbs[3].type = DescriptorSampledImage;
					dbs[3].count = 128;
					dbs[4].type = DescriptorUniformBuffer;
					forward_descriptorsetlayout = new DescriptorSetLayoutPrivate(d, dbs);
				}

				{
					DescriptorBindingInfo db;
					db.type = DescriptorSampledImage;
					db.count = 1;
					one_image_descriptorsetlayout = new DescriptorSetLayoutPrivate(d, { &db, 1 });
				}

				element_pipelinelayout = new PipelineLayoutPrivate(d, { &element_descriptorsetlayout, 1 }, sizeof(Vec4f));
				forward_pipelinelayout = new PipelineLayoutPrivate(d, { &forward_descriptorsetlayout, 1 }, 0);
				one_image_pc0_pipelinelayout = new PipelineLayoutPrivate(d, { &one_image_descriptorsetlayout, 1 }, 0);
				one_image_pc4_pipelinelayout = new PipelineLayoutPrivate(d, { &one_image_descriptorsetlayout, 1 }, sizeof(float));
				one_image_pc8_pipelinelayout = new PipelineLayoutPrivate(d, { &one_image_descriptorsetlayout, 1 }, sizeof(Vec2f));

				{
					VertexAttributeInfo vias[3];
					vias[0].location = 0;
					vias[0].format = Format_R32G32_SFLOAT;
					vias[1].location = 1;
					vias[1].format = Format_R32G32_SFLOAT;
					vias[2].location = 2;
					vias[2].format = Format_R8G8B8A8_UNORM;
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
					element8_pipeline = PipelinePrivate::create(d, shaders, element_pipelinelayout, one_image8_renderpass, 0, &vi, Vec2u(0), nullptr, SampleCount_1,
						nullptr, { &bo, 1 });
					element16_pipeline = PipelinePrivate::create(d, shaders, element_pipelinelayout, one_image16_renderpass, 0, &vi, Vec2u(0), nullptr, SampleCount_1,
						nullptr, { &bo, 1 });
				}

				{
					VertexAttributeInfo vias[3];
					vias[0].location = 0;
					vias[0].format = Format_R32G32B32_SFLOAT;
					vias[1].location = 1;
					vias[1].format = Format_R32G32_SFLOAT;
					vias[2].location = 2;
					vias[2].format = Format_R32G32B32_SFLOAT;
					//vias[3].location = 3;
					//vias[3].format = Format_R32G32B32_SFLOAT;
					VertexBufferInfo vib;
					vib.attributes_count = size(vias);
					vib.attributes = vias;
					RasterInfo rst;
					VertexInfo vi;
					vi.buffers_count = 1;
					vi.buffers = &vib;
					DepthInfo dep;
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"forward.vert"),
						new ShaderPrivate(d, L"forward.frag")
					};
					forward8_pipeline = PipelinePrivate::create(d, shaders, forward_pipelinelayout, forward8_renderpass, 0, &vi, Vec2u(0), &rst, SampleCount_1,
						&dep);
					forward16_pipeline = PipelinePrivate::create(d, shaders, forward_pipelinelayout, forward16_renderpass, 0, &vi, Vec2u(0), &rst, SampleCount_1,
						&dep);
				}

				for (auto i = 0; i < 10; i++)
				{
					{
						ShaderPrivate* shaders[] = {
							new ShaderPrivate(d, L"fullscreen.vert"),
							new ShaderPrivate(d, L"blur.frag", "R" + std::to_string(i + 1) + " H\n")
						};
						blurh8_pipeline[i] = PipelinePrivate::create(d, shaders, one_image_pc4_pipelinelayout, one_image8_renderpass, 0);
						blurh16_pipeline[i] = PipelinePrivate::create(d, shaders, one_image_pc4_pipelinelayout, one_image16_renderpass, 0);
					}

					{
						ShaderPrivate* shaders[] = {
							new ShaderPrivate(d, L"fullscreen.vert"),
							new ShaderPrivate(d, L"blur.frag", "R" + std::to_string(i + 1) + " V\n")
						};
						blurv8_pipeline[i] = PipelinePrivate::create(d, shaders, one_image_pc4_pipelinelayout, one_image8_renderpass, 0);
						blurv16_pipeline[i] = PipelinePrivate::create(d, shaders, one_image_pc4_pipelinelayout, one_image16_renderpass, 0);
					}
				}

				{
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"fullscreen.vert"),
						new ShaderPrivate(d, L"blit.frag")
					};
					blit8_pipeline = PipelinePrivate::create(d, shaders, one_image_pc0_pipelinelayout, one_image8_renderpass, 0);
					blit16_pipeline = PipelinePrivate::create(d, shaders, one_image_pc0_pipelinelayout, one_image16_renderpass, 0);
				}

				{
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"fullscreen.vert"),
						new ShaderPrivate(d, L"filter_bright.frag")
					};
					filter_bright_pipeline = PipelinePrivate::create(d, shaders, one_image_pc0_pipelinelayout, one_image16_renderpass, 0);
				}

				{
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"fullscreen.vert"),
						new ShaderPrivate(d, L"box.frag")
					};
					downsample_pipeline = PipelinePrivate::create(d, shaders, one_image_pc8_pipelinelayout, one_image16_renderpass, 0);
				}

				{
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"fullscreen.vert"),
						new ShaderPrivate(d, L"box.frag")
					};
					BlendOption bo;
					bo.enable = true;
					bo.src_color = BlendFactorOne;
					bo.dst_color = BlendFactorOne;
					bo.src_alpha = BlendFactorZero;
					bo.dst_alpha = BlendFactorOne;
					upsample_pipeline = PipelinePrivate::create(d, shaders, one_image_pc8_pipelinelayout, one_image16_renderpass, 0, nullptr, Vec2u(0), nullptr, SampleCount_1, 
						nullptr, { &bo, 1 });
				}
			}

			element_vertex_buffer.init(d, 360000);
			element_index_buffer.init(d, 240000);
			model_vertex_buffer_1.init(d, 600000);
			model_index_buffer.init(d, 400000);
			material_info_buffer.init(d, 128);
			mesh_matrix_buffer.init(d, 10000);
			mesh_indirect_buffer.init(d, 10000);
			light_info_buffer.init(d, 10000);
			light_indices_buffer.init(d, 1);

			white_image.reset(new ImagePrivate(d, Format_R8G8B8A8_UNORM, Vec2u(1), 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled));
			white_image->clear(ImageLayoutUndefined, ImageLayoutShaderReadOnly, Vec4c(255));
			auto iv_wht = white_image->default_views[0].get();
			for (auto i = 0; i < resources_count; i++)
			{
				auto r = new CanvasResourcePrivate;
				r->view = iv_wht;
				resources[i].reset(r);
			}

			bind_model((ModelPrivate*)Model::get_standard(StandardModelCube), "cube");
			bind_model((ModelPrivate*)Model::get_standard(StandardModelSphere), "sphere");

			auto sp = d->sampler_linear.get();

			element_descriptorset.reset(new DescriptorSetPrivate(d->descriptor_pool.get(), element_descriptorsetlayout));
			forward_descriptorset.reset(new DescriptorSetPrivate(d->descriptor_pool.get(), forward_descriptorsetlayout));
			for (auto i = 0; i < resources_count; i++)
				element_descriptorset->set_image(0, i, iv_wht, sp);
			forward_descriptorset->set_buffer(0, 0, mesh_matrix_buffer.buf.get());
			forward_descriptorset->set_buffer(1, 0, light_info_buffer.buf.get());
			forward_descriptorset->set_buffer(2, 0, light_indices_buffer.buf.get());
			forward_descriptorset->set_buffer(4, 0, material_info_buffer.buf.get());
		}

		void CanvasPrivate::set_target(std::span<ImageViewPrivate*> views)
		{
			target_imageviews.clear();
			target_framebuffers.clear();
			target_nearest_descriptorsets.clear();

			hdr_image.reset();
			hdr_framebuffer.reset();
			hdr_nearest_descriptorset.reset();

			depth_image.reset();
			forward8_framebuffers.clear();

			back8_image.reset();
			back8_framebuffers.clear();
			back8_nearest_descriptorsets.clear();
			back8_linear_descriptorsets.clear();
			back16_image.reset();
			back16_framebuffers.clear();
			back16_nearest_descriptorsets.clear();
			back16_linear_descriptorsets.clear();

			if (views.empty())
				target_size = 0.f;
			else
			{
				auto cb = std::make_unique<CommandBufferPrivate>(device->graphics_command_pool.get());
				cb->begin(true);

				auto sp_nr = device->sampler_nearest.get();
				auto sp_ln = device->sampler_linear.get();

				target_size = views[0]->image->sizes[0];
				target_imageviews.resize(views.size());
				target_framebuffers.resize(views.size());
				target_nearest_descriptorsets.resize(views.size());
				target_linear_descriptorsets.resize(views.size());
				for (auto i = 0; i < views.size(); i++)
				{
					target_imageviews[i] = views[i];
					target_framebuffers[i].reset(new FramebufferPrivate(device, one_image8_renderpass, { &views[i], 1 }));
					target_nearest_descriptorsets[i].reset(new DescriptorSetPrivate(device->descriptor_pool.get(), one_image_descriptorsetlayout));
					target_linear_descriptorsets[i].reset(new DescriptorSetPrivate(device->descriptor_pool.get(), one_image_descriptorsetlayout));
					target_nearest_descriptorsets[i]->set_image(0, 0, views[i], sp_nr);
					target_linear_descriptorsets[i]->set_image(0, 0, views[i], sp_ln);
				}

				hdr_image.reset(new ImagePrivate(device, Format_R16G16B16A16_SFLOAT, target_size, 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled | ImageUsageAttachment));
				{
					cb->image_barrier(hdr_image.get(), {}, ImageLayoutUndefined, ImageLayoutShaderReadOnly);

					auto iv = hdr_image->default_views[0].get();
					hdr_framebuffer.reset(new FramebufferPrivate(device, one_image16_renderpass, { &iv, 1 }));
					hdr_nearest_descriptorset.reset(new DescriptorSetPrivate(device->descriptor_pool.get(), one_image_descriptorsetlayout));
					hdr_linear_descriptorset.reset(new DescriptorSetPrivate(device->descriptor_pool.get(), one_image_descriptorsetlayout));
					hdr_nearest_descriptorset->set_image(0, 0, iv, sp_nr);
					hdr_linear_descriptorset->set_image(0, 0, iv, sp_ln);
				}

				depth_image.reset(new ImagePrivate(device, Format_Depth16, target_size, 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled | ImageUsageAttachment));
				cb->image_barrier(depth_image.get(), {}, ImageLayoutUndefined, ImageLayoutAttachment);

				forward8_framebuffers.resize(views.size());
				for (auto i = 0; i < views.size(); i++)
				{
					ImageViewPrivate* vs[] = {
						views[i],
						depth_image->default_views[0].get()
					};
					forward8_framebuffers[i].reset(new FramebufferPrivate(device, forward8_renderpass, vs));
				}
				{
					ImageViewPrivate* vs[] = {
						hdr_image->default_views[0].get(),
						depth_image->default_views[0].get()
					};
					forward16_framebuffer.reset(new FramebufferPrivate(device, forward16_renderpass, vs));
				}

				back8_image.reset(new ImagePrivate(device, Format_B8G8R8A8_UNORM, target_size, 0xFFFFFFFF, 1, SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
				back8_framebuffers.resize(back8_image->level);
				back8_nearest_descriptorsets.resize(back8_image->level);
				back8_linear_descriptorsets.resize(back8_image->level);
				for (auto i = 0; i < back8_image->level; i++)
				{
					cb->image_barrier(back8_image.get(), { (uint)i }, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
					auto iv = back8_image->default_views[i].get();
					back8_framebuffers[i].reset(new FramebufferPrivate(device, one_image8_renderpass, { &iv, 1 }));
					back8_nearest_descriptorsets[i].reset(new DescriptorSetPrivate(device->descriptor_pool.get(), one_image_descriptorsetlayout));
					back8_linear_descriptorsets[i].reset(new DescriptorSetPrivate(device->descriptor_pool.get(), one_image_descriptorsetlayout));
					back8_nearest_descriptorsets[i]->set_image(0, 0, iv, sp_nr);
					back8_linear_descriptorsets[i]->set_image(0, 0, iv, sp_ln);
				}
				back16_image.reset(new ImagePrivate(device, Format_R16G16B16A16_SFLOAT, target_size, 0xFFFFFFFF, 1, SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
				back16_framebuffers.resize(back16_image->level);
				back16_nearest_descriptorsets.resize(back16_image->level);
				back16_linear_descriptorsets.resize(back16_image->level);
				for (auto i = 0; i < back16_image->level; i++)
				{
					cb->image_barrier(back16_image.get(), { (uint)i }, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
					auto iv = back16_image->default_views[i].get();
					back16_framebuffers[i].reset(new FramebufferPrivate(device, one_image16_renderpass, { &iv, 1 }));
					back16_nearest_descriptorsets[i].reset(new DescriptorSetPrivate(device->descriptor_pool.get(), one_image_descriptorsetlayout));
					back16_linear_descriptorsets[i].reset(new DescriptorSetPrivate(device->descriptor_pool.get(), one_image_descriptorsetlayout));
					back16_nearest_descriptorsets[i]->set_image(0, 0, iv, sp_nr);
					back16_linear_descriptorsets[i]->set_image(0, 0, iv, sp_ln);
				}

				cb->end();
				auto q = device->graphics_queue.get();
				q->submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
				q->wait_idle();
			}
		}

		uint CanvasPrivate::set_resource(int slot, ImageViewPrivate* v, SamplerPrivate* sp, const std::string& name, ImageAtlasPrivate* image_atlas, FontAtlasPrivate* font_atlas)
		{
			auto iv_wht = white_image->default_views[0].get();

			if (slot == -1)
			{
				assert(v);
				slot = white_slot;
				white_slot = -1;
			}

			auto r = new CanvasResourcePrivate;
			element_descriptorset->set_image(0, slot, v, sp ? sp : device->sampler_linear.get());
			r->name = name;
			r->view = v ? v : iv_wht;
			r->image_atlas = image_atlas;
			r->font_atlas = font_atlas;
			resources[slot].reset(r);

			if (white_slot == -1)
			{
				for (auto i = 0; i < resources_count; i++)
				{
					if (resources[i]->view == iv_wht)
					{
						white_slot = i;
						break;
					}
				}
				assert(white_slot != -1);
			}

			return slot;
		}

		uint CanvasPrivate::bind_model(ModelPrivate* model, const std::string& name)
		{
			assert(model);
			BoundModel m;
			m.name = name;
			m.model = model;
			models.push_back(m);
			return models.size() - 1;
		}

		int CanvasPrivate::find_model(const char* name)
		{
			for (auto i = 0; i < models.size(); i++)
			{
				if (models[i].name == name)
					return i;
			}
			return -1;
		}

		void CanvasPrivate::add_draw_element_cmd(uint id)
		{
			auto add = [&]() {
				Cmd cmd;
				cmd.type = CmdDrawElement;
				cmd.v.d1.id = id;
				cmd.v.d1.vtx_cnt = 0;
				cmd.v.d1.idx_cnt = 0;
				cmds.push_back(cmd);
			};
			if (cmds.empty())
				add();
			else
			{
				auto& back = cmds.back();
				if (back.type != CmdDrawElement || back.v.d1.id != id)
					add();
			}
		}

		void CanvasPrivate::add_vtx(const Vec2f& pos, const Vec2f& uv, const Vec4c& col)
		{
			ElementVertex v;
			v.pos = pos;
			v.uv = uv;
			v.col = col;
			element_vertex_buffer.push(v);
			
			cmds.back().v.d1.vtx_cnt++;
		}

		void CanvasPrivate::add_idx(uint idx)
		{
			element_index_buffer.push(idx);

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

			add_draw_element_cmd(white_slot);
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
			add_draw_element_cmd(white_slot);
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

		void CanvasPrivate::draw_text(uint res_id, const wchar_t* text_beg, const wchar_t* text_end, uint font_size, const Vec4c& col, const Vec2f& pos, const Mat2f& axes)
		{
			auto atlas = resources[res_id]->font_atlas;

			add_draw_element_cmd(res_id);

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

		void CanvasPrivate::draw_image(uint res_id, uint tile_id, const Vec2f& LT, const Vec2f& RT, const Vec2f& RB, const Vec2f& LB, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col)
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

			add_draw_element_cmd(res_id);

			auto vtx_cnt = cmds.back().v.d1.vtx_cnt;

			add_vtx(LT, _uv0, tint_col);
			add_vtx(RT, Vec2f(_uv1.x(), _uv0.y()), tint_col);
			add_vtx(RB, _uv1, tint_col);
			add_vtx(LB, Vec2f(_uv0.x(), _uv1.y()), tint_col);
			add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 2);
		}

		void CanvasPrivate::draw_mesh(uint mod_id, uint mesh_idx, const Mat4f& proj, const Mat4f& view, const Mat4f& model, const Mat4f& normal)
		{
			if (uploaded_models_count != models.size())
			{
				model_vertex_buffer_1.stg_rewind();
				model_index_buffer.stg_rewind();
				material_info_buffer.stg_rewind();
				auto tex_cnt = 0;

				auto sp = device->sampler_linear.get();

				for (auto i = uploaded_models_count; i < models.size(); i++)
				{
					auto& m = models[i];
					for (auto& ma : m.model->materials)
					{
						BoundMaterial bm;
						bm.albedo_alpha = Vec4f(ma->albedo, 1.f);
						bm.spec_roughness = Vec4f(ma->spec, ma->roughness);
						if (!ma->albedo_map_filename.empty())
						{
							auto img = ImagePrivate::create(device, ma->albedo_map_filename);
							if (img)
							{
								auto idx = uploaded_model_textures_count + tex_cnt;
								model_textures[idx].reset(img);
								forward_descriptorset->set_image(3, idx, img->default_views[img->level].get(), sp);
								bm.albedo_map_index = idx;
								tex_cnt++;
							}
						}
						if (!ma->spec_map_filename.empty())
						{
							auto img = ImagePrivate::create(device, ma->spec_map_filename);
							if (img)
							{
								auto idx = uploaded_model_textures_count + tex_cnt;
								model_textures[idx].reset(img);
								forward_descriptorset->set_image(3, idx, img->default_views[img->level].get(), sp);
								bm.spec_map_index = idx;
								tex_cnt++;
							}
						}
						if (!ma->normal_map_filename.empty())
						{
							auto img = ImagePrivate::create(device, ma->normal_map_filename);
							if (img)
							{
								auto idx = uploaded_model_textures_count + tex_cnt;
								model_textures[idx].reset(img);
								forward_descriptorset->set_image(3, idx, img->default_views[img->level].get(), sp);
								bm.normal_map_index = idx;
								tex_cnt++;
							}
						}
						if (!ma->roughness_map_filename.empty())
						{
							auto img = ImagePrivate::create(device, ma->roughness_map_filename);
							if (img)
							{
								auto idx = uploaded_model_textures_count + tex_cnt;
								model_textures[idx].reset(img);
								forward_descriptorset->set_image(3, idx, img->default_views[img->level].get(), sp);
								bm.roughness_map_index = idx;
								tex_cnt++;
							}
						}
						m.mat_idxs.push_back(uploaded_materials_count + material_info_buffer.stg_num());

						material_info_buffer.push(bm);
					}
					for (auto& ms : m.model->meshes)
					{
						BoundMesh bm;
						bm.vtx_off = model_vertex_buffer_1.stg_num();
						bm.idx_off = model_index_buffer.stg_num();
						bm.idx_cnt = ms->indices.size();
						bm.mat_idx = m.mat_idxs[ms->material_index];
						m.meshes.push_back(bm);

						model_vertex_buffer_1.push(ms->vertices_1.size(), ms->vertices_1.data());
						model_index_buffer.push(ms->indices.size(), ms->indices.data());
					}
				}

				auto cb = std::make_unique<CommandBufferPrivate>(device->graphics_command_pool.get());
				cb->begin(true);
				model_vertex_buffer_1.upload(cb.get());
				model_index_buffer.upload(cb.get());
				material_info_buffer.upload(cb.get());
				cb->end();
				auto q = device->graphics_queue.get();
				q->submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
				q->wait_idle();

				uploaded_models_count = models.size();
				uploaded_vertices_count += model_vertex_buffer_1.stg_num();
				uploaded_indices_count += model_index_buffer.stg_num();
				uploaded_materials_count += material_info_buffer.stg_num();
				uploaded_model_textures_count += tex_cnt;
			}

			auto add_cmd = [&]() {
				Cmd cmd;
				cmd.type = CmdDrawMesh;
				cmd.v.d3.count = 0;
				cmds.push_back(cmd);
			};
			if (cmds.empty())
				add_cmd();
			else
			{
				auto& back = cmds.back();
				if (back.type != CmdDrawMesh)
					add_cmd();
			}

			auto m_id = mesh_matrix_buffer.stg_num() << 16;
			auto& m = models[mod_id];
			MeshMatrix om;
			om.model = model;
			om.view = view;
			om.proj = proj;
			om.mvp = proj * view * model;
			om.nor = normal;
			mesh_matrix_buffer.push(om);
			{
				auto& ms = m.meshes[mesh_idx];
				DrawIndexedIndirectCommand ic;
				ic.index_count = ms.idx_cnt;
				ic.instance_count = 1;
				ic.first_index = ms.idx_off;
				ic.vertex_offset = ms.vtx_off;
				ic.first_instance = m_id;
				mesh_indirect_buffer.push(ic);
			}

			cmds.back().v.d3.count += m.meshes.size();
		}

		void CanvasPrivate::add_light(LightType type, const Vec3f& color, const Vec3f& pos)
		{
			LightInfo li;
			li.col = Vec4f(color, 0.f);
			li.pos = Vec4f(pos, type == LightPoint ? 1.f : 0.f);
			light_info_buffer.push(li);

			{
				// TODO
				auto& lis = *(LightIndices*)light_indices_buffer.stg->mapped;
				lis.indices[lis.count++] = light_info_buffer.stg_num() - 1;
			}
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

		void CanvasPrivate::add_bloom()
		{
			Cmd cmd;
			cmd.type = CmdBloom;
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
			element_vertex_buffer.stg_rewind();
			element_index_buffer.stg_rewind();
			mesh_matrix_buffer.stg_rewind();
			mesh_indirect_buffer.stg_rewind();
			light_info_buffer.stg_rewind();
			light_indices_buffer.stg_rewind();

			{
				// TODO
				LightIndices lis;
				lis.count = 0;
				light_indices_buffer.push(lis);
			}

			curr_scissor = Vec4f(Vec2f(0.f), Vec2f(target_size));

			cmds.clear();
		}

		void CanvasPrivate::record(CommandBufferPrivate* cb, uint image_index)
		{
			enum PassType
			{
				PassNone = -1,
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

			for (auto i = 0; i < cmds.size(); i++)
			{
				switch (cmds[i].type)
				{
				case CmdDrawElement:
				{
					if (passes.empty() || (passes.back().type != PassElement && passes.back().type != PassNone))
						passes.emplace_back();
					passes.back().type = PassElement;
					passes.back().cmd_ids.push_back(i);

				}
					break;
				case CmdDrawMesh:
				{
					if (passes.empty() || (passes.back().type != PassObject && passes.back().type != PassNone))
						passes.emplace_back();
					passes.back().type = PassObject;
					passes.back().cmd_ids.push_back(i);

				}
					break;
				case CmdSetScissor:
				{
					if (passes.empty() || (passes.back().type != PassElement && passes.back().type != PassObject && passes.back().type != PassNone))
						passes.emplace_back();
					passes.back().cmd_ids.push_back(i);
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
				case CmdBloom:
				{
					Pass p;
					p.type = PassBloom;
					p.cmd_ids.push_back(i);
					passes.push_back(p);
				}
				break;
				}
			}

			cb->begin();

			auto dst = hdr ? hdr_image.get() : target_imageviews[image_index]->image;
			auto dst_fb = hdr ? hdr_framebuffer.get() : target_framebuffers[image_index].get();
			auto dst_ds = hdr ? hdr_nearest_descriptorset.get() : target_nearest_descriptorsets[image_index].get();
			auto pl_ele = hdr ? element16_pipeline : element8_pipeline;
			auto pl_fwd = hdr ? forward16_pipeline : forward8_pipeline;

			cb->image_barrier(dst, {}, hdr ? ImageLayoutShaderReadOnly : ImageLayoutPresent, ImageLayoutTransferDst);
			cb->clear_color_image(dst, clear_color);
			cb->image_barrier(dst, {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);

			cb->image_barrier(depth_image.get(), {}, ImageLayoutAttachment, ImageLayoutTransferDst);
			cb->clear_depth_image(depth_image.get(), 1.f);
			cb->image_barrier(depth_image.get(), {}, ImageLayoutTransferDst, ImageLayoutAttachment);

			element_vertex_buffer.upload(cb);
			element_index_buffer.upload(cb);
			mesh_matrix_buffer.upload(cb);
			mesh_indirect_buffer.upload(cb);
			light_info_buffer.upload(cb);
			light_indices_buffer.upload(cb);

			cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)target_size));
			cb->set_scissor(curr_scissor);
			auto ele_vtx_off = 0;
			auto ele_idx_off = 0;
			auto obj_indirect_off = 0;

			for (auto& p : passes)
			{
				switch (p.type)
				{
				case PassElement:
				{
					if (ele_idx_off == 0)
					{
						element_vertex_buffer.barrier(cb);
						element_index_buffer.barrier(cb);

					}
					cb->set_scissor(curr_scissor);
					cb->bind_vertex_buffer(element_vertex_buffer.buf.get(), 0);
					cb->bind_index_buffer(element_index_buffer.buf.get(), IndiceTypeUint);
					cb->begin_renderpass(dst_fb);
					cb->bind_pipeline(pl_ele);
					cb->bind_descriptor_set(element_descriptorset.get(), 0, element_pipelinelayout);
					cb->push_constant_t(0, Vec2f(2.f / target_size.x(), 2.f / target_size.y()), element_pipelinelayout);
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
					if (obj_indirect_off == 0)
					{
						mesh_matrix_buffer.barrier(cb);
						mesh_indirect_buffer.barrier(cb);
						light_info_buffer.barrier(cb);
						light_indices_buffer.barrier(cb);
					}
					cb->set_scissor(curr_scissor);
					cb->bind_vertex_buffer(model_vertex_buffer_1.buf.get(), 0);
					cb->bind_index_buffer(model_index_buffer.buf.get(), IndiceTypeUint);
					cb->begin_renderpass(hdr ? forward16_framebuffer.get() : forward8_framebuffers[image_index].get());
					cb->bind_pipeline(pl_fwd);
					cb->bind_descriptor_set(forward_descriptorset.get(), 0, forward_pipelinelayout);
					for (auto& i : p.cmd_ids)
					{
						auto& cmd = cmds[i];
						switch (cmd.type)
						{
						case CmdDrawMesh:
							cb->draw_indexed_indirect(mesh_indirect_buffer.buf.get(), obj_indirect_off, cmd.v.d3.count);
							obj_indirect_off += cmd.v.d3.count;
							break;
						case CmdSetScissor:
							cb->set_scissor(cmd.v.d2.scissor);
							break;
						}
					}
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

					cb->image_barrier(dst, {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly);
					cb->set_scissor(Vec4f(blur_range.x() - blur_radius, blur_range.y() - blur_radius,
						blur_range.z() + blur_radius, blur_range.w() + blur_radius));
					cb->begin_renderpass(hdr ? back16_framebuffers[0].get() : back8_framebuffers[0].get());
					cb->bind_pipeline(hdr ? blurh16_pipeline[blur_radius - 1] : blurh8_pipeline[blur_radius - 1]);
					cb->bind_descriptor_set(dst_ds, 0, one_image_pc4_pipelinelayout);
					cb->push_constant_t(0, 1.f / target_size.x(), element_pipelinelayout);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->image_barrier(hdr ? back16_image.get() : back8_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly);
					cb->set_scissor(blur_range);
					cb->begin_renderpass(dst_fb);
					cb->bind_pipeline(hdr ? blurv16_pipeline[blur_radius - 1] : blurv8_pipeline[blur_radius - 1]);
					cb->bind_descriptor_set(hdr ? back16_nearest_descriptorsets[0].get() : back8_nearest_descriptorsets[0].get(), 0, one_image_pc4_pipelinelayout);
					cb->push_constant_t(0, 1.f / target_size.y(), element_pipelinelayout);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();
				}
					break;
				case PassBloom:
				{
					if (!hdr)
						continue;

					cb->set_scissor(Vec4f(Vec2f(0.f), (Vec2f)target_size));

					cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)target_size));
					cb->image_barrier(hdr_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly);
					cb->begin_renderpass(back16_framebuffers[0].get());
					cb->bind_pipeline(filter_bright_pipeline);
					cb->bind_descriptor_set(hdr_nearest_descriptorset.get(), 0, one_image_pc0_pipelinelayout);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					for (auto i = 0; i < back16_image->level - 1; i++)
					{
						cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)back16_image->sizes[i + 1]));
						cb->image_barrier(back16_image.get(), { (uint)i }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly);
						cb->begin_renderpass(back16_framebuffers[i + 1].get());
						cb->bind_pipeline(downsample_pipeline);
						cb->bind_descriptor_set(back16_linear_descriptorsets[i].get(), 0, one_image_pc8_pipelinelayout);
						cb->push_constant_t(0, 1.f / (Vec2f)back16_image->sizes[i], one_image_pc8_pipelinelayout);
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();
					}
					for (auto i = back16_image->level - 1; i > 0; i--)
					{
						cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)back16_image->sizes[i - 1]));
						cb->image_barrier(back16_image.get(), { (uint)i }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly);
						cb->begin_renderpass(back16_framebuffers[i - 1].get());
						cb->bind_pipeline(upsample_pipeline);
						cb->bind_descriptor_set(back16_linear_descriptorsets[i].get(), 0, one_image_pc8_pipelinelayout);
						cb->push_constant_t(0, 1.f / (Vec2f)back16_image->sizes[(uint)i - 1], one_image_pc8_pipelinelayout);
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();
					}
					cb->image_barrier(back16_image.get(), { 1U }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly);
					cb->begin_renderpass(hdr_framebuffer.get());
					cb->bind_pipeline(upsample_pipeline);
					cb->bind_descriptor_set(back16_linear_descriptorsets[1].get(), 0, one_image_pc8_pipelinelayout);
					cb->push_constant_t(0, 1.f / target_size.y(), one_image_pc8_pipelinelayout);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)target_size));
				}
					break;
				}
			}

			if (hdr)
			{
				cb->image_barrier(hdr_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly);
				cb->image_barrier(target_imageviews[image_index]->image, {}, ImageLayoutPresent, ImageLayoutShaderReadOnly);
				cb->set_scissor(Vec4f(Vec2f(0.f), Vec2f(target_size)));
				cb->begin_renderpass(target_framebuffers[image_index].get());
				cb->bind_pipeline(blit8_pipeline);
				cb->bind_descriptor_set(hdr_nearest_descriptorset.get(), 0, one_image_pc0_pipelinelayout);
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();
			}
			cb->image_barrier(target_imageviews[image_index]->image, {}, ImageLayoutShaderReadOnly, ImageLayoutPresent);

			cb->end();

			cmds.clear();
		}

		Canvas* Canvas::create(Device* d)
		{
			return new CanvasPrivate((DevicePrivate*)d);
		}
	}
}
