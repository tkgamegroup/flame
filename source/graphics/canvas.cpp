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

		static SamplerPrivate* shadow_sampler = nullptr;
		static RenderpassPrivate* image1_8_renderpass = nullptr;
		static RenderpassPrivate* image1_16_renderpass = nullptr;
		static RenderpassPrivate* image1_r32_renderpass = nullptr;
		static RenderpassPrivate* forward8_renderpass = nullptr;
		static RenderpassPrivate* forward16_renderpass = nullptr;
		static RenderpassPrivate* forward8_msaa_renderpass = nullptr;
		static RenderpassPrivate* forward16_msaa_renderpass = nullptr;
		static RenderpassPrivate* depth_renderpass = nullptr;
		static DescriptorSetLayoutPrivate* element_descriptorsetlayout = nullptr;
		static DescriptorSetLayoutPrivate* mesh_descriptorsetlayout = nullptr;
		static DescriptorSetLayoutPrivate* material_descriptorsetlayout = nullptr;
		static DescriptorSetLayoutPrivate* light_descriptorsetlayout = nullptr;
		static DescriptorSetLayoutPrivate* forward_descriptorsetlayout = nullptr;
		static DescriptorSetLayoutPrivate* sampler1_descriptorsetlayout = nullptr;
		static PipelineLayoutPrivate* element_pipelinelayout = nullptr;
		static PipelineLayoutPrivate* forward_pipelinelayout = nullptr;
		static PipelineLayoutPrivate* sampler1_pc0_pipelinelayout = nullptr;
		static PipelineLayoutPrivate* sampler1_pc4_pipelinelayout = nullptr;
		static PipelineLayoutPrivate* sampler1_pc8_pipelinelayout = nullptr;
		static PipelineLayoutPrivate* image1_pc0_pipelinelayout = nullptr;
		static PipelineLayoutPrivate* image2_pc0_pipelinelayout = nullptr;
		static PipelineLayoutPrivate* depth_pipelinelayout = nullptr;
		static PipelinePrivate* element8_pipeline = nullptr;
		static PipelinePrivate* element16_pipeline = nullptr;
		static PipelinePrivate* forward8_pipeline = nullptr;
		static PipelinePrivate* forward16_pipeline = nullptr;
		static PipelinePrivate* forward8_msaa_pipeline = nullptr;
		static PipelinePrivate* forward16_msaa_pipeline = nullptr;
		static PipelinePrivate* depth_pipeline = nullptr;
		static PipelinePrivate* blurh8_pipeline[10] = {};
		static PipelinePrivate* blurh16_pipeline[10] = {};
		static PipelinePrivate* blurv8_pipeline[10] = {};
		static PipelinePrivate* blurv16_pipeline[10] = {};
		static PipelinePrivate* blurh_depth_pipeline = nullptr;
		static PipelinePrivate* blurv_depth_pipeline = nullptr;
		static PipelinePrivate* blit8_pipeline = nullptr;
		static PipelinePrivate* blit16_pipeline = nullptr;
		static PipelinePrivate* filter_bright_pipeline = nullptr;
		static PipelinePrivate* downsample_pipeline = nullptr;
		static PipelinePrivate* upsample_pipeline = nullptr;
		static PipelinePrivate* gamma_pipeline = nullptr;

		CanvasPrivate::CanvasPrivate(DevicePrivate* d, bool hdr, bool msaa_3d) :
			device(d),
			hdr(hdr),
			msaa_3d(msaa_3d)
		{
			if (!initialized)
			{
				initialized = true;

				shadow_sampler = new SamplerPrivate(device, FilterLinear, FilterLinear, false, false);

				{
					RenderpassAttachmentInfo att;
					att.format = Swapchain::get_format();
					att.load_op = AttachmentLoad;
					att.initia_layout = ImageLayoutShaderReadOnly;
					att.final_layout = ImageLayoutShaderReadOnly;
					RenderpassSubpassInfo sp;
					uint col_refs[] = {
						0
					};
					sp.color_attachments_count = 1;
					sp.color_attachments = col_refs;
					image1_8_renderpass = new RenderpassPrivate(d, { &att, 1 }, { &sp, 1 });
					att.format = Format_R16G16B16A16_SFLOAT;
					image1_16_renderpass = new RenderpassPrivate(d, { &att, 1 }, { &sp, 1 });
					att.format = Format_R32_SFLOAT;
					image1_r32_renderpass = new RenderpassPrivate(d, { &att, 1 }, { &sp, 1 });
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
					forward8_renderpass = new RenderpassPrivate(d, atts, { &sp, 1 });
					atts[0].format = Format_R16G16B16A16_SFLOAT;
					forward16_renderpass = new RenderpassPrivate(d, atts, { &sp, 1 });

					{
						RenderpassAttachmentInfo atts[3];
						atts[0].load_op = AttachmentClear;
						atts[0].sample_count = msaa_sample_count;
						atts[0].initia_layout = ImageLayoutAttachment;
						atts[1].format = Format_Depth16;
						atts[1].load_op = AttachmentClear;
						atts[1].sample_count = msaa_sample_count;
						atts[1].initia_layout = ImageLayoutAttachment;
						atts[1].final_layout = ImageLayoutAttachment;
						atts[2].load_op = AttachmentDontCare;
						atts[2].initia_layout = ImageLayoutShaderReadOnly;

						uint res_refs[] = {
							2
						};
						sp.resolve_attachments_count = 1;
						sp.resolve_attachments = res_refs;

						atts[0].format = Swapchain::get_format();
						atts[2].format = Swapchain::get_format();
						forward8_msaa_renderpass = new RenderpassPrivate(d, atts, { &sp,1 });
						atts[0].format = Format_R16G16B16A16_SFLOAT;
						atts[2].format = Format_R16G16B16A16_SFLOAT;
						forward16_msaa_renderpass = new RenderpassPrivate(d, atts, { &sp,1 });
					}
				}

				{
					RenderpassAttachmentInfo atts[2];
					atts[0].format = Format_R32_SFLOAT;
					atts[0].load_op = AttachmentClear;
					atts[0].initia_layout = ImageLayoutShaderReadOnly;
					atts[1].format = Format_Depth16;
					atts[1].load_op = AttachmentClear;
					atts[1].initia_layout = ImageLayoutAttachment;
					atts[1].final_layout = ImageLayoutAttachment;
					RenderpassSubpassInfo sp;
					uint col_refs[] = {
						0
					};
					sp.color_attachments_count = 1;
					sp.color_attachments = col_refs;
					sp.depth_attachment = 1;
					depth_renderpass = new RenderpassPrivate(d, atts, { &sp,1 });
				}

				{
					DescriptorBindingInfo db;
					db.type = DescriptorSampledImage;
					db.count = resources_count;
					element_descriptorsetlayout = new DescriptorSetLayoutPrivate(d, { &db, 1 });
				}

				{
					DescriptorBindingInfo db;
					db.type = DescriptorStorageBuffer;
					mesh_descriptorsetlayout = new DescriptorSetLayoutPrivate(d, { &db, 1 });
				}

				{
					DescriptorBindingInfo dbs[2];
					dbs[0].type = DescriptorStorageBuffer;
					dbs[1].type = DescriptorSampledImage;
					dbs[1].count = 128;
					material_descriptorsetlayout = new DescriptorSetLayoutPrivate(d, dbs);
				}

				{
					DescriptorBindingInfo dbs[3];
					dbs[0].type = DescriptorStorageBuffer;
					dbs[1].type = DescriptorStorageBuffer;
					dbs[2].type = DescriptorSampledImage;
					dbs[2].count = 4;
					light_descriptorsetlayout = new DescriptorSetLayoutPrivate(d, dbs);
				}

				{
					DescriptorBindingInfo db;
					db.type = DescriptorUniformBuffer;
					forward_descriptorsetlayout = new DescriptorSetLayoutPrivate(d, { &db, 1 });
				}

				{
					DescriptorBindingInfo db;
					db.type = DescriptorSampledImage;
					db.count = 1;
					sampler1_descriptorsetlayout = new DescriptorSetLayoutPrivate(d, { &db, 1 });
				}

				element_pipelinelayout = new PipelineLayoutPrivate(d, { &element_descriptorsetlayout, 1 }, sizeof(Vec4f));
				{
					DescriptorSetLayoutPrivate* dsls[] = {
						mesh_descriptorsetlayout,
						material_descriptorsetlayout,
						light_descriptorsetlayout,
						forward_descriptorsetlayout
					};
					forward_pipelinelayout = new PipelineLayoutPrivate(d, dsls, 0);
				}
				sampler1_pc0_pipelinelayout = new PipelineLayoutPrivate(d, { &sampler1_descriptorsetlayout, 1 }, 0);
				sampler1_pc4_pipelinelayout = new PipelineLayoutPrivate(d, { &sampler1_descriptorsetlayout, 1 }, sizeof(float));
				sampler1_pc8_pipelinelayout = new PipelineLayoutPrivate(d, { &sampler1_descriptorsetlayout, 1 }, sizeof(Vec2f));
				{
					DescriptorSetLayoutPrivate* dsls[] = {
						mesh_descriptorsetlayout,
						material_descriptorsetlayout
					};
					depth_pipelinelayout = new PipelineLayoutPrivate(d, dsls, sizeof(Mat4f) * 2);
				}

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
					bo.src_alpha = BlendFactorOne;
					bo.dst_alpha = BlendFactorZero;
					element8_pipeline = PipelinePrivate::create(d, shaders, element_pipelinelayout, image1_8_renderpass, 0, &vi, Vec2u(0), nullptr, SampleCount_1,
						nullptr, { &bo, 1 });
					element16_pipeline = PipelinePrivate::create(d, shaders, element_pipelinelayout, image1_16_renderpass, 0, &vi, Vec2u(0), nullptr, SampleCount_1,
						nullptr, { &bo, 1 });
				}

				{
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"forward.vert"),
						new ShaderPrivate(d, L"forward.frag")
					};
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
					VertexInfo vi;
					vi.buffers_count = 1;
					vi.buffers = &vib;
					RasterInfo rst;
					DepthInfo dep;
					forward8_pipeline = PipelinePrivate::create(d, shaders, forward_pipelinelayout, forward8_renderpass, 0, &vi, Vec2u(0), &rst, SampleCount_1,
						&dep);
					forward16_pipeline = PipelinePrivate::create(d, shaders, forward_pipelinelayout, forward16_renderpass, 0, &vi, Vec2u(0), &rst, SampleCount_1,
						&dep);
					forward8_msaa_pipeline = PipelinePrivate::create(d, shaders, forward_pipelinelayout, forward8_msaa_renderpass, 0, &vi, Vec2u(0), &rst, msaa_sample_count,
						&dep);
					forward16_msaa_pipeline = PipelinePrivate::create(d, shaders, forward_pipelinelayout, forward16_msaa_renderpass, 0, &vi, Vec2u(0), &rst, msaa_sample_count,
						&dep);
				}

				{
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"depth.vert"),
						new ShaderPrivate(d, L"depth.frag")
					};
					VertexAttributeInfo vias[2];
					vias[0].location = 0;
					vias[0].format = Format_R32G32B32_SFLOAT;
					vias[1].location = 1;
					vias[1].format = Format_R32G32_SFLOAT;
					VertexBufferInfo vib;
					vib.attributes_count = size(vias);
					vib.attributes = vias;
					vib.stride = 8 * sizeof(float);
					VertexInfo vi;
					vi.buffers_count = 1;
					vi.buffers = &vib;
					RasterInfo rst;
					DepthInfo dep;
					depth_pipeline = PipelinePrivate::create(d, shaders, depth_pipelinelayout, depth_renderpass, 0, &vi, Vec2u(0), &rst, SampleCount_1,
						&dep);
				}

				for (auto i = 0; i < 10; i++)
				{
					{
						ShaderPrivate* shaders[] = {
							new ShaderPrivate(d, L"fullscreen.vert"),
							new ShaderPrivate(d, L"blur.frag", "R" + std::to_string(i + 1) + " H\n")
						};
						blurh8_pipeline[i] = PipelinePrivate::create(d, shaders, sampler1_pc4_pipelinelayout, image1_8_renderpass, 0);
						blurh16_pipeline[i] = PipelinePrivate::create(d, shaders, sampler1_pc4_pipelinelayout, image1_16_renderpass, 0);
					}

					{
						ShaderPrivate* shaders[] = {
							new ShaderPrivate(d, L"fullscreen.vert"),
							new ShaderPrivate(d, L"blur.frag", "R" + std::to_string(i + 1) + " V\n")
						};
						blurv8_pipeline[i] = PipelinePrivate::create(d, shaders, sampler1_pc4_pipelinelayout, image1_8_renderpass, 0);
						blurv16_pipeline[i] = PipelinePrivate::create(d, shaders, sampler1_pc4_pipelinelayout, image1_16_renderpass, 0);
					}
				}

				{
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"fullscreen.vert"),
						new ShaderPrivate(d, L"blur_depth.frag", "R5 H\n")
					};
					blurh_depth_pipeline = PipelinePrivate::create(d, shaders, sampler1_pc4_pipelinelayout, image1_r32_renderpass, 0);
				}

				{
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"fullscreen.vert"),
						new ShaderPrivate(d, L"blur_depth.frag", "R5 V\n")
					};
					blurv_depth_pipeline = PipelinePrivate::create(d, shaders, sampler1_pc4_pipelinelayout, image1_r32_renderpass, 0);
				}

				{
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"fullscreen.vert"),
						new ShaderPrivate(d, L"blit.frag")
					};
					blit8_pipeline = PipelinePrivate::create(d, shaders, sampler1_pc0_pipelinelayout, image1_8_renderpass, 0);
					blit16_pipeline = PipelinePrivate::create(d, shaders, sampler1_pc0_pipelinelayout, image1_16_renderpass, 0);
				}

				{
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"fullscreen.vert"),
						new ShaderPrivate(d, L"filter_bright.frag")
					};
					filter_bright_pipeline = PipelinePrivate::create(d, shaders, sampler1_pc0_pipelinelayout, image1_16_renderpass, 0);
				}

				{
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"fullscreen.vert"),
						new ShaderPrivate(d, L"box.frag")
					};
					downsample_pipeline = PipelinePrivate::create(d, shaders, sampler1_pc8_pipelinelayout, image1_16_renderpass, 0);
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
					upsample_pipeline = PipelinePrivate::create(d, shaders, sampler1_pc8_pipelinelayout, image1_16_renderpass, 0, nullptr, Vec2u(0), nullptr, SampleCount_1, 
						nullptr, { &bo, 1 });
				}

				{
					ShaderPrivate* shaders[] = {
						new ShaderPrivate(d, L"fullscreen.vert"),
						new ShaderPrivate(d, L"gamma.frag")
					};
					gamma_pipeline = PipelinePrivate::create(d, shaders, sampler1_pc0_pipelinelayout, image1_8_renderpass, 0);
				}
			}

			ImmediateCommandBuffer cb(device);

			element_vertex_buffer.create(d, 360000);
			element_index_buffer.create(d, 240000);
			model_vertex_buffer_1.create(d, 600000);
			model_index_buffer.create(d, 400000);
			camera_data_buffer.create(d, 1);
			material_info_buffer.create(d, 128);
			mesh_matrix_buffer.create(d, 10000);
			point_light_info_buffer.create(d, 10000);
			point_light_indices_buffer.create(d, 1);

			white_image.reset(new ImagePrivate(d, Format_R8G8B8A8_UNORM, Vec2u(1), 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled));
			cb->image_barrier(white_image.get(), {}, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->clear_color_image(white_image.get(), Vec4c(255));
			cb->image_barrier(white_image.get(), {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			auto iv_wht = white_image->views[0].get();
			for (auto i = 0; i < resources_count; i++)
			{
				auto r = new CanvasResourcePrivate;
				r->view = iv_wht;
				resources[i].reset(r);
			}

			auto sp_nr = d->sampler_nearest.get();
			auto sp_ln = d->sampler_linear.get();

			element_descriptorset.reset(new DescriptorSetPrivate(d->descriptor_pool.get(), element_descriptorsetlayout));
			for (auto i = 0; i < resources_count; i++)
				element_descriptorset->set_image(0, i, iv_wht, sp_ln);

			mesh_descriptorset.reset(new DescriptorSetPrivate(d->descriptor_pool.get(), mesh_descriptorsetlayout));
			mesh_descriptorset->set_buffer(0, 0, mesh_matrix_buffer.buf.get());

			material_descriptorset.reset(new DescriptorSetPrivate(d->descriptor_pool.get(), material_descriptorsetlayout));
			material_descriptorset->set_buffer(0, 0, material_info_buffer.buf.get());
			model_textures.resize(128);
			for (auto i = 0; i < model_textures.size(); i++)
				material_descriptorset->set_image(1, i, iv_wht, sp_ln);

			light_descriptorset.reset(new DescriptorSetPrivate(d->descriptor_pool.get(), light_descriptorsetlayout));
			light_descriptorset->set_buffer(0, 0, point_light_info_buffer.buf.get());
			light_descriptorset->set_buffer(1, 0, point_light_indices_buffer.buf.get());
			{
				shadow_depth_image.reset(new ImagePrivate(device, Format_Depth16, shadow_map_size, 1, 1, SampleCount_1, ImageUsageAttachment));
				cb->image_barrier(shadow_depth_image.get(), {}, ImageLayoutUndefined, ImageLayoutAttachment);
				shadow_blur_pingpong_image.reset(new ImagePrivate(device, Format_R32_SFLOAT, shadow_map_size, 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
				cb->image_barrier(shadow_blur_pingpong_image.get(), {}, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
				auto iv_pingpong = shadow_blur_pingpong_image->views[0].get();
				shadow_blur_pingpong_image_framebuffer.reset(new FramebufferPrivate(device, image1_r32_renderpass, { &iv_pingpong, 1 }));
				shadow_blur_pingpong_image_descriptorset.reset(new DescriptorSetPrivate(d->descriptor_pool.get(), sampler1_descriptorsetlayout));
				shadow_blur_pingpong_image_descriptorset->set_image(0, 0, iv_pingpong, sp_nr);

				point_light_shadow_maps.resize(4);
				point_light_shadow_map_depth_framebuffers.resize(point_light_shadow_maps.size() * 6);
				point_light_shadow_map_framebuffers.resize(point_light_shadow_maps.size() * 6);
				point_light_shadow_map_descriptorsets.resize(point_light_shadow_maps.size() * 6);
				for (auto i = 0; i < point_light_shadow_maps.size(); i++)
				{
					point_light_shadow_maps[i].reset(new ImagePrivate(device, Format_R32_SFLOAT, shadow_map_size, 1, 6, SampleCount_1, ImageUsageSampled | ImageUsageAttachment, true));
					cb->image_barrier(point_light_shadow_maps[i].get(), { 0U, 1U, 0U, 6U }, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
					for (auto j = 0; j < 6; j++)
					{
						auto iv = new ImageViewPrivate(point_light_shadow_maps[i].get(), true, ImageView2D, { 0U, 1U, (uint)j, 1U });
						ImageViewPrivate* vs[] = {
							iv,
							shadow_depth_image->views[0].get()
						};
						point_light_shadow_map_depth_framebuffers[i * 6 + j].reset(new FramebufferPrivate(device, depth_renderpass, vs));
						point_light_shadow_map_framebuffers[i * 6 + j].reset(new FramebufferPrivate(device, image1_r32_renderpass, { &iv, 1 }));
						point_light_shadow_map_descriptorsets[i * 6 + j].reset(new DescriptorSetPrivate(d->descriptor_pool.get(), sampler1_descriptorsetlayout));
						point_light_shadow_map_descriptorsets[i * 6 + j]->set_image(0, 0, iv, sp_nr);
					}
					auto iv = new ImageViewPrivate(point_light_shadow_maps[i].get(), true, ImageViewCube, { 0U, 1U, 0U, 6U });
					light_descriptorset->set_image(2, i, iv, shadow_sampler);
				}
			}

			forward_descriptorset.reset(new DescriptorSetPrivate(d->descriptor_pool.get(), forward_descriptorsetlayout));
			forward_descriptorset->set_buffer(0, 0, camera_data_buffer.buf.get());

			bind_model((ModelPrivate*)Model::get_standard(StandardModelCube), "cube");
			bind_model((ModelPrivate*)Model::get_standard(StandardModelSphere), "sphere");
		}

		void CanvasPrivate::set_target(std::span<ImageViewPrivate*> views)
		{
			target_imageviews.clear();
			target_framebuffers.clear();
			target_descriptorsets.clear();

			dst_image.reset();
			dst_framebuffer.reset();
			dst_descriptorset.reset();

			depth_image.reset();

			forward_framebuffers.clear();

			msaa_image.reset();
			msaa_resolve_image.reset();
			msaa_descriptorset.reset();

			back_image.reset();
			back_framebuffers.clear();
			back_nearest_descriptorsets.clear();
			back_linear_descriptorsets.clear();

			if (views.empty())
				target_size = 0.f;
			else
			{
				ImmediateCommandBuffer cb(device);

				auto sp_nr = device->sampler_nearest.get();
				auto sp_ln = device->sampler_linear.get();

				target_size = views[0]->image->sizes[0];
				target_imageviews.resize(views.size());
				target_framebuffers.resize(views.size());
				target_descriptorsets.resize(views.size());
				for (auto i = 0; i < views.size(); i++)
				{
					target_imageviews[i] = views[i];
					target_framebuffers[i].reset(new FramebufferPrivate(device, image1_8_renderpass, { &views[i], 1 }));
					target_descriptorsets[i].reset(new DescriptorSetPrivate(device->descriptor_pool.get(), sampler1_descriptorsetlayout));
					target_descriptorsets[i]->set_image(0, 0, views[i], sp_nr);
				}

				if (hdr)
				{
					dst_image.reset(new ImagePrivate(device, Format_R16G16B16A16_SFLOAT, target_size, 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled | ImageUsageAttachment));
					cb->image_barrier(dst_image.get(), {}, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
					auto iv = dst_image->views[0].get();
					dst_framebuffer.reset(new FramebufferPrivate(device, image1_16_renderpass, { &iv, 1 }));
					dst_descriptorset.reset(new DescriptorSetPrivate(device->descriptor_pool.get(), sampler1_descriptorsetlayout));
					dst_descriptorset->set_image(0, 0, iv, sp_nr);
				}

				if (msaa_3d)
					depth_image.reset(new ImagePrivate(device, Format_Depth16, target_size, 1, 1, msaa_sample_count, ImageUsageAttachment));
				else
					depth_image.reset(new ImagePrivate(device, Format_Depth16, target_size, 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled | ImageUsageAttachment));
				cb->image_barrier(depth_image.get(), {}, ImageLayoutUndefined, ImageLayoutAttachment);

				if (msaa_3d)
				{
					msaa_image.reset(new ImagePrivate(device, hdr ? Format_R16G16B16A16_SFLOAT : Format_R8G8B8A8_UNORM, target_size, 1, 1, msaa_sample_count, ImageUsageAttachment));
					cb->image_barrier(msaa_image.get(), {}, ImageLayoutUndefined, ImageLayoutAttachment);
					msaa_resolve_image.reset(new ImagePrivate(device, hdr ? Format_R16G16B16A16_SFLOAT : Format_R8G8B8A8_UNORM, target_size, 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
					cb->image_barrier(msaa_resolve_image.get(), {}, ImageLayoutUndefined, ImageLayoutShaderReadOnly);

					auto iv_res = msaa_resolve_image->views[0].get();
					msaa_descriptorset.reset(new DescriptorSetPrivate(device->descriptor_pool.get(), sampler1_descriptorsetlayout));
					msaa_descriptorset->set_image(0, 0, iv_res, sp_nr);

					forward_framebuffers.resize(1);
					ImageViewPrivate* vs[] = {
						msaa_image->views[0].get(),
						depth_image->views[0].get(),
						iv_res
					};
					forward_framebuffers[0].reset(new FramebufferPrivate(device, hdr ? forward16_msaa_renderpass : forward8_msaa_renderpass, vs));
				}
				else
				{
					if (hdr)
					{
						forward_framebuffers.resize(1);
						ImageViewPrivate* vs[] = {
							dst_image->views[0].get(),
							depth_image->views[0].get()
						};
						forward_framebuffers[0].reset(new FramebufferPrivate(device, forward16_renderpass, vs));
					}
					else
					{
						forward_framebuffers.resize(views.size());
						for (auto i = 0; i < views.size(); i++)
						{
							ImageViewPrivate* vs[] = {
								views[i],
								depth_image->views[0].get()
							};
							forward_framebuffers[i].reset(new FramebufferPrivate(device, forward8_renderpass, vs));
						}
					}
				}

				back_image.reset(new ImagePrivate(device, hdr ? Format_R16G16B16A16_SFLOAT : Format_B8G8R8A8_UNORM, target_size, 0xFFFFFFFF, 1, SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
				cb->image_barrier(back_image.get(), { 0U, back_image->level, 0U, 1U }, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
				back_framebuffers.resize(back_image->level);
				back_nearest_descriptorsets.resize(back_image->level);
				back_linear_descriptorsets.resize(back_image->level);
				for (auto i = 0; i < back_image->level; i++)
				{
					auto iv = back_image->views[i].get();
					back_framebuffers[i].reset(new FramebufferPrivate(device, hdr ? image1_16_renderpass : image1_8_renderpass, { &iv, 1 }));
					back_nearest_descriptorsets[i].reset(new DescriptorSetPrivate(device->descriptor_pool.get(), sampler1_descriptorsetlayout));
					back_linear_descriptorsets[i].reset(new DescriptorSetPrivate(device->descriptor_pool.get(), sampler1_descriptorsetlayout));
					back_nearest_descriptorsets[i]->set_image(0, 0, iv, sp_nr);
					back_linear_descriptorsets[i]->set_image(0, 0, iv, sp_ln);
				}
			}
		}

		uint CanvasPrivate::set_resource(int slot, ImageViewPrivate* v, SamplerPrivate* sp, const std::string& name, ImageAtlasPrivate* image_atlas, FontAtlasPrivate* font_atlas)
		{
			auto iv_wht = white_image->views[0].get();

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
			if (cmds.empty())
			{
				last_element_cmd = new CmdDrawElement(id);
				cmds.emplace_back(last_element_cmd);
			}
			else
			{
				auto back = cmds.back().get();
				if (back->type != Cmd::DrawElement || ((CmdDrawElement*)back)->id != id)
				{
					last_element_cmd = new CmdDrawElement(id);
					cmds.emplace_back(last_element_cmd);
				}
			}
		}

		void CanvasPrivate::add_vtx(const Vec2f& pos, const Vec2f& uv, const Vec4c& col)
		{
			ElementVertex v;
			v.pos = pos;
			v.uv = uv;
			v.col = col;
			element_vertex_buffer.push(v);
			
			last_element_cmd->vertices_count++;
		}

		void CanvasPrivate::add_idx(uint idx)
		{
			element_index_buffer.push(idx);

			last_element_cmd->indices_count++;
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

				auto vtx_cnt0 = last_element_cmd->vertices_count;

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

								auto vtx_cnt = last_element_cmd->vertices_count;

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
								auto vtx_cnt = last_element_cmd->vertices_count;

								add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 2); add_idx(vtx_cnt - 2); add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt0 + 2);
								add_idx(vtx_cnt - 4); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 3); add_idx(vtx_cnt - 4); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
								add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 3); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 2); add_idx(vtx_cnt0 + 3);
							}
							else
							{
								auto p1 = points[i + 1];

								auto n1 = normals[i + 1];

								auto vtx_cnt = last_element_cmd->vertices_count;

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

								auto vtx_cnt = last_element_cmd->vertices_count;

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
								auto vtx_cnt = last_element_cmd->vertices_count;

								add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt - 3); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
								add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 2); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt0 + 2);
							}
							else
							{
								auto p1 = points[i + 1];

								auto n1 = normals[i + 1];

								auto vtx_cnt = last_element_cmd->vertices_count;

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
							auto vtx_cnt = last_element_cmd->vertices_count;

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
							auto vtx_cnt = last_element_cmd->vertices_count;

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

							auto vtx_cnt = last_element_cmd->vertices_count;

							add_vtx(p0 + n0 * thickness, uv, col);
							add_vtx(p0 - n0 * thickness, uv, col);
							add_vtx(p1 + n1 * thickness, uv, col);
							add_vtx(p1 - n1 * thickness, uv, col);
							add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 3);
						}
						else if (closed && i == points.size() - 2)
						{
							auto vtx_cnt = last_element_cmd->vertices_count;

							add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
						}
						else
						{
							auto p1 = points[i + 1];

							auto n1 = normals[i + 1];

							auto vtx_cnt = last_element_cmd->vertices_count;

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
					auto vtx_cnt = last_element_cmd->vertices_count;

					add_vtx(points[0], uv, col);
					add_vtx(points[i + 1], uv, col);
					add_vtx(points[i + 2], uv, col);
					add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1);
				}

				if (aa)
				{
					auto vtx_cnt0 = last_element_cmd->vertices_count;
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

							auto vtx_cnt = last_element_cmd->vertices_count;

							add_vtx(p0, uv, col);
							add_vtx(p0 - n0 * _feather, uv, col_t);
							add_vtx(p1, uv, col);
							add_vtx(p1 - n1 * _feather, uv, col_t);
							add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 3);
						}
						else if (i == points.size() - 2)
						{
							auto vtx_cnt = last_element_cmd->vertices_count;

							add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 1); add_idx(vtx_cnt - 1); add_idx(vtx_cnt - 2); add_idx(vtx_cnt0 + 0); add_idx(vtx_cnt0 + 1);
						}
						else
						{
							auto p1 = points[i + 1];

							auto n1 = normals[i + 1];

							auto vtx_cnt = last_element_cmd->vertices_count;

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

					auto vtx_cnt = last_element_cmd->vertices_count;

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

			auto vtx_cnt = last_element_cmd->vertices_count;

			add_vtx(LT, _uv0, tint_col);
			add_vtx(RT, Vec2f(_uv1.x(), _uv0.y()), tint_col);
			add_vtx(RB, _uv1, tint_col);
			add_vtx(LB, Vec2f(_uv0.x(), _uv1.y()), tint_col);
			add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 2);
		}

		void CanvasPrivate::set_camera(const Mat4f& proj, const Mat4f& view, const Vec3f& coord)
		{
			project_view_matrix = proj * view;

			camera_data_buffer.end->proj = proj;
			camera_data_buffer.end->view = view;
			camera_data_buffer.end->proj_view = project_view_matrix;
			camera_data_buffer.end->coord = Vec4f(coord, 1.f);
		}

		void CanvasPrivate::draw_mesh(uint mod_id, uint mesh_idx, const Mat4f& model, const Mat4f& normal, bool cast_shadow)
		{
			if (uploaded_models_count != models.size())
			{
				model_vertex_buffer_1.stg_rewind();
				model_index_buffer.stg_rewind();
				material_info_buffer.stg_rewind();
				auto tex_idx = uploaded_model_textures_count;

				auto sp = device->sampler_linear.get();

				for (auto i = uploaded_models_count; i < models.size(); i++)
				{
					auto& m = models[i];
					auto path = m.model->filename.parent_path();

					for (auto& ma : m.model->materials)
					{
						BoundMaterial bm;
						bm.color = ma->color;
						bm.metallic = ma->metallic;
						bm.roughness = ma->roughness;
						bm.alpha_test = ma->alpha_test;

						if (!path.empty())
						{
							auto color_map = !ma->color_map.empty() ? Bitmap::create((path / ma->color_map).c_str()) : nullptr;
							auto alpha_map = !ma->alpha_map.empty() ? Bitmap::create((path / ma->alpha_map).c_str()) : nullptr;
							auto roughness_map = !ma->roughness_map.empty() ? Bitmap::create((path / ma->roughness_map).c_str()) : nullptr;

							if (color_map || alpha_map)
							{
								Vec2u size = Vec2u(0U);
								if (color_map)
									size = Vec2u(color_map->get_width(), color_map->get_height());
								else if (alpha_map)
									size = Vec2u(alpha_map->get_width(), alpha_map->get_height());

								auto dst_map = Bitmap::create(size.x(), size.y());
								auto dst_map_d = dst_map->get_data();
								auto dst_map_pitch = size.x() * 4;

								auto color_map_d = color_map ? color_map->get_data() : nullptr;
								auto color_map_pitch = color_map ? color_map->get_pitch() : 0;
								auto alpha_map_d = alpha_map ? alpha_map->get_data() : nullptr;
								auto alpha_map_pitch = alpha_map ? alpha_map->get_pitch() : 0;
								auto alpha_map_ch = alpha_map ? alpha_map->get_byte_per_channel() : 0;

								if (color_map && alpha_map)
								{
									for (auto y = 0; y < size.y(); y++)
									{
										for (auto x = 0; x < size.x(); x++)
										{
										(Vec4c&)dst_map_d[y * dst_map_pitch + x * 4] =
											Vec4c((Vec3c&)color_map_d[y * color_map_pitch + x * 3], alpha_map_d[y * alpha_map_pitch + x * alpha_map_ch]);
										}
									}
								}
								else if (color_map && !alpha_map)
								{
								auto a = ma->color.a() * 255;
								for (auto y = 0; y < size.y(); y++)
								{
									for (auto x = 0; x < size.x(); x++)
									{
										(Vec4c&)dst_map_d[y * dst_map_pitch + x * 4] =
											Vec4c((Vec3c&)color_map_d[y * color_map_pitch + x * 3], a);
									}
								}
								}
								else if (!color_map && alpha_map)
								{
								auto col = Vec3c(Vec3f(ma->color) * 255.f);
								for (auto y = 0; y < size.y(); y++)
								{
									for (auto x = 0; x < size.x(); x++)
									{
										(Vec4c&)dst_map_d[y * dst_map_pitch + x * 4] =
											Vec4c(col, alpha_map_d[y * alpha_map_pitch + x * 3]);
									}
								}
								}

								auto img = ImagePrivate::create(device, dst_map);
								model_textures[tex_idx].reset(img);
								material_descriptorset->set_image(1, tex_idx, img->views.back().get(), sp);
								bm.color_map_index = tex_idx;
								tex_idx++;

								if (color_map)
									color_map->release();
								if (alpha_map)
									alpha_map->release();
								dst_map->release();
							}

							if (roughness_map)
							{
								Vec2u size = Vec2u(0U);
								if (roughness_map)
									size = Vec2u(roughness_map->get_width(), roughness_map->get_height());

								auto dst_map = Bitmap::create(size.x(), size.y());
								auto dst_map_d = dst_map->get_data();
								auto dst_map_pitch = size.x() * 4;

								auto roughness_map_d = roughness_map ? roughness_map->get_data() : nullptr;
								auto roughness_map_pitch = roughness_map ? roughness_map->get_pitch() : 0;
								auto roughness_map_ch = roughness_map ? roughness_map->get_byte_per_channel() : 0;

								if (roughness_map)
								{
									for (auto y = 0; y < size.y(); y++)
									{
										for (auto x = 0; x < size.x(); x++)
										{
											(Vec4c&)dst_map_d[y * dst_map_pitch + x * 4] =
												Vec4c(0, roughness_map_d[y * roughness_map_pitch + x * roughness_map_ch], 0, 0);
										}
									}
								}

								auto img = ImagePrivate::create(device, dst_map);
								model_textures[tex_idx].reset(img);
								material_descriptorset->set_image(1, tex_idx, img->views.back().get(), sp);
								bm.metallic_roughness_ao_map_index = tex_idx;
								tex_idx++;

								if (roughness_map)
									roughness_map->release();
								dst_map->release();
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

				{
					ImmediateCommandBuffer cb(device);
					model_vertex_buffer_1.upload(cb.cb.get());
					model_index_buffer.upload(cb.cb.get());
					material_info_buffer.upload(cb.cb.get());
				}

				uploaded_models_count = models.size();
				uploaded_vertices_count += model_vertex_buffer_1.stg_num();
				uploaded_indices_count += model_index_buffer.stg_num();
				uploaded_materials_count += material_info_buffer.stg_num();
				uploaded_model_textures_count = tex_idx;
			}

			if (cmds.empty() || cmds.back()->type != Cmd::DrawMesh)
			{
				last_mesh_cmd = new CmdDrawMesh;
				cmds.emplace_back(last_mesh_cmd);
			}

			MeshMatrix om;
			om.model = model;
			om.normal = normal;
			mesh_matrix_buffer.push(om);

			auto ms = &models[mod_id].meshes[mesh_idx];
			last_mesh_cmd->meshes.push_back(ms);
			if (cast_shadow)
				shadow_casters.emplace_back(ms, mesh_matrix_buffer.stg_num() - 1);
		}

		void CanvasPrivate::add_light(LightType type, const Vec3f& color, const Vec3f& coord, bool cast_shadow)
		{
			PointLightInfo li;
			li.color = color;
			li.coord = coord;
			li.shadow_map_index = cast_shadow ? used_point_light_shadow_maps_count++ : -1;
			point_light_info_buffer.push(li);

			{
				// TODO
				auto& lis = *(PointLightIndices*)point_light_indices_buffer.stg->mapped;
				lis.indices[lis.count++] = point_light_indices_buffer.stg_num() - 1;
			}
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
			cmds.emplace_back(new CmdSetScissor(scissor));
		}

		void CanvasPrivate::add_blur(const Vec4f& _range, uint radius)
		{
			auto range = Vec4f(
				max(_range.x(), 0.f),
				max(_range.y(), 0.f),
				min(_range.z(), (float)target_size.x()),
				min(_range.w(), (float)target_size.y()));
			cmds.emplace_back(new CmdBlur(range, radius));
		}

		void CanvasPrivate::add_bloom()
		{
			cmds.emplace_back(new CmdBloom());
		}

		void CanvasPrivate::prepare()
		{
			element_vertex_buffer.stg_rewind();
			element_index_buffer.stg_rewind();
			mesh_matrix_buffer.stg_rewind();
			point_light_info_buffer.stg_rewind();
			point_light_indices_buffer.stg_rewind();

			{
				// TODO
				PointLightIndices lis;
				lis.count = 0;
				point_light_indices_buffer.push(lis);
			}

			shadow_casters.clear();
			used_point_light_shadow_maps_count = 0;

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
				switch (cmds[i]->type)
				{
				case Cmd::DrawElement:
				{
					if (passes.empty() || (passes.back().type != PassElement && passes.back().type != PassNone))
						passes.emplace_back();
					passes.back().type = PassElement;
					passes.back().cmd_ids.push_back(i);

				}
				break;
				case Cmd::DrawMesh:
				{
					if (passes.empty() || (passes.back().type != PassObject && passes.back().type != PassNone))
						passes.emplace_back();
					passes.back().type = PassObject;
					passes.back().cmd_ids.push_back(i);

				}
				break;
				case Cmd::SetScissor:
				{
					if (passes.empty() || (passes.back().type != PassElement && passes.back().type != PassObject && passes.back().type != PassNone))
						passes.emplace_back();
					passes.back().cmd_ids.push_back(i);
				}
				break;
				case Cmd::Blur:
				{
					Pass p;
					p.type = PassBlur;
					p.cmd_ids.push_back(i);
					passes.push_back(p);
				}
				break;
				case Cmd::Bloom:
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

			auto dst = hdr ? dst_image.get() : target_imageviews[image_index]->image;
			auto dst_fb = hdr ? dst_framebuffer.get() : target_framebuffers[image_index].get();
			auto dst_ds = hdr ? dst_descriptorset.get() : target_descriptorsets[image_index].get();
			auto pl_ele = hdr ? element16_pipeline : element8_pipeline;
			auto pl_fwd = msaa_3d ? (hdr ? forward16_msaa_pipeline : forward8_msaa_pipeline) : (hdr ? forward16_pipeline : forward8_pipeline);

			cb->image_barrier(dst, {}, hdr ? ImageLayoutShaderReadOnly : ImageLayoutPresent, ImageLayoutTransferDst);
			cb->clear_color_image(dst, clear_color);
			cb->image_barrier(dst, {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);

			if (!msaa_3d)
			{
				cb->image_barrier(depth_image.get(), {}, ImageLayoutAttachment, ImageLayoutTransferDst);
				cb->clear_depth_image(depth_image.get(), 1.f);
				cb->image_barrier(depth_image.get(), {}, ImageLayoutTransferDst, ImageLayoutAttachment);
			}

			element_vertex_buffer.upload(cb);
			element_index_buffer.upload(cb);
			camera_data_buffer.upload(cb, true);
			mesh_matrix_buffer.upload(cb);
			point_light_info_buffer.upload(cb);
			point_light_indices_buffer.upload(cb);

			cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)target_size));
			cb->set_scissor(Vec4f(Vec2f(0.f), (Vec2f)target_size));
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
					cb->set_scissor(Vec4f(Vec2f(0.f), (Vec2f)target_size));
					cb->bind_vertex_buffer(element_vertex_buffer.buf.get(), 0);
					cb->bind_index_buffer(element_index_buffer.buf.get(), IndiceTypeUint);
					cb->begin_renderpass(dst_fb);
					cb->bind_pipeline(pl_ele);
					cb->bind_descriptor_set(element_descriptorset.get(), 0, element_pipelinelayout);
					cb->push_constant_t(0, Vec2f(2.f / target_size.x(), 2.f / target_size.y()), element_pipelinelayout);
					for (auto& i : p.cmd_ids)
					{
						auto& cmd = cmds[i];
						switch (cmd->type)
						{
						case Cmd::DrawElement:
						{
							auto c = (CmdDrawElement*)cmd.get();
							if (c->indices_count > 0)
							{
								cb->draw_indexed(c->indices_count, ele_idx_off, ele_vtx_off, 1, c->id);
								ele_vtx_off += c->vertices_count;
								ele_idx_off += c->indices_count;
							}
						}
							break;
						case Cmd::SetScissor:
						{
							auto c = (CmdSetScissor*)cmd.get();
							cb->set_scissor(c->scissor);
						}
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
						camera_data_buffer.barrier(cb);
						mesh_matrix_buffer.barrier(cb);
						point_light_info_buffer.barrier(cb);
						point_light_indices_buffer.barrier(cb);

						if (used_point_light_shadow_maps_count > 0)
						{
							cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)shadow_map_size));
							cb->set_scissor(Vec4f(Vec2f(0.f), (Vec2f)shadow_map_size));
							cb->bind_vertex_buffer(model_vertex_buffer_1.buf.get(), 0);
							cb->bind_index_buffer(model_index_buffer.buf.get(), IndiceTypeUint);

							auto lights_count = point_light_info_buffer.stg_num();
							for (auto i = 0; i < lights_count; i++)
							{
								auto& li = point_light_info_buffer.beg[i];
								if (li.shadow_map_index != -1)
								{
									for (auto j = 0; j < 6; j++)
									{
										Vec4f cvs[] = {
											Vec4f(1000.f, 0.f, 0.f, 0.f),
											Vec4f(1.f, 0.f, 0.f, 0.f)
										};
										cb->begin_renderpass(point_light_shadow_map_depth_framebuffers[li.shadow_map_index * 6 + j].get(), cvs);
										cb->bind_pipeline(depth_pipeline);
										cb->bind_descriptor_set(mesh_descriptorset.get(), 0, depth_pipelinelayout);
										cb->bind_descriptor_set(material_descriptorset.get(), 1, depth_pipelinelayout);
										static const auto proj = make_project_matrix(90.f * ANG_RAD, 1.f, 1.f, 1000.f);
										struct
										{
											Mat4f matrix;
											Vec4f coord;
											Vec4f dummy[3];
										}pc;
										pc.matrix = Mat4f(1.f);
										switch (j)
										{
										case 0:
											pc.matrix[0][0] = -1.f;
											pc.matrix = pc.matrix * proj * make_view_matrix(li.coord, li.coord + Vec3f(1.f, 0.f, 0.f), Vec3f(0.f, 1.f, 0.f));
											break;
										case 1:
											pc.matrix[0][0] = -1.f;
											pc.matrix = pc.matrix * proj * make_view_matrix(li.coord, li.coord + Vec3f(-1.f, 0.f, 0.f), Vec3f(0.f, 1.f, 0.f));
											break;
										case 2:
											pc.matrix[1][1] = -1.f;
											pc.matrix = pc.matrix * proj * make_view_matrix(li.coord, li.coord + Vec3f(0.f, 1.f, 0.f), Vec3f(1.f, 0.f, 0.f));
											break;
										case 3:
											pc.matrix[1][1] = -1.f;
											pc.matrix = pc.matrix * proj * make_view_matrix(li.coord, li.coord + Vec3f(0.f, -1.f, 0.f), Vec3f(0.f, 0.f, -1.f));
											break;
										case 4:
											pc.matrix[0][0] = -1.f;
											pc.matrix = pc.matrix * proj * make_view_matrix(li.coord, li.coord + Vec3f(0.f, 0.f, 1.f), Vec3f(0.f, 1.f, 0.f));
											break;
										case 5:
											pc.matrix[0][0] = -1.f;
											pc.matrix = pc.matrix * proj * make_view_matrix(li.coord, li.coord + Vec3f(0.f, 0.f, -1.f), Vec3f(0.f, 1.f, 0.f));
											break;
										}
										pc.coord = Vec4f(li.coord, 0.f);
										cb->push_constant(0, sizeof(pc), &pc, depth_pipelinelayout);
										for (auto& m : shadow_casters)
											cb->draw_indexed(m.first->idx_cnt, m.first->idx_off, m.first->vtx_off, 1, (m.second << 16) + m.first->mat_idx);
										cb->end_renderpass();

										cb->image_barrier(point_light_shadow_maps[li.shadow_map_index].get(), { 0U, 1U, (uint)j, 1U }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
										cb->begin_renderpass(shadow_blur_pingpong_image_framebuffer.get());
										cb->bind_pipeline(blurh_depth_pipeline);
										cb->bind_descriptor_set(point_light_shadow_map_descriptorsets[li.shadow_map_index * 6 + j].get(), 0, sampler1_pc4_pipelinelayout);
										cb->push_constant_t(0, 1.f / shadow_map_size.x(), sampler1_pc4_pipelinelayout);
										cb->draw(3, 1, 0, 0);
										cb->end_renderpass();

										cb->image_barrier(shadow_blur_pingpong_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
										cb->begin_renderpass(point_light_shadow_map_framebuffers[li.shadow_map_index * 6 + j].get());
										cb->bind_pipeline(blurv_depth_pipeline);
										cb->bind_descriptor_set(shadow_blur_pingpong_image_descriptorset.get(), 0, sampler1_pc4_pipelinelayout);
										cb->push_constant_t(0, 1.f / shadow_map_size.y(), sampler1_pc4_pipelinelayout);
										cb->draw(3, 1, 0, 0);
										cb->end_renderpass();
									}

									cb->image_barrier(point_light_shadow_maps[li.shadow_map_index].get(), { 0U, 1U, 0U, 6U }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
								}
							}

							cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)target_size));
						}
					}

					cb->set_scissor(Vec4f(Vec2f(0.f), (Vec2f)target_size));
					cb->bind_vertex_buffer(model_vertex_buffer_1.buf.get(), 0);
					cb->bind_index_buffer(model_index_buffer.buf.get(), IndiceTypeUint);
					if (!msaa_3d)
						cb->begin_renderpass(forward_framebuffers[hdr ? 0 : image_index].get());
					else
					{
						Vec4f cvs[3];
						cvs[0] = Vec4f(0.f, 0.f, 0.f, 0.f);
						cvs[1] = Vec4f(1.f, 0.f, 0.f, 0.f);
						cvs[2] = Vec4f(0.f, 0.f, 0.f, 0.f);
						cb->begin_renderpass(forward_framebuffers[0].get(), cvs);
					}
					cb->bind_pipeline(pl_fwd);
					cb->bind_descriptor_set(mesh_descriptorset.get(), 0, forward_pipelinelayout);
					cb->bind_descriptor_set(material_descriptorset.get(), 1, forward_pipelinelayout);
					cb->bind_descriptor_set(light_descriptorset.get(), 2, forward_pipelinelayout);
					cb->bind_descriptor_set(forward_descriptorset.get(), 3, forward_pipelinelayout);
					for (auto& i : p.cmd_ids)
					{
						auto& cmd = cmds[i];
						switch (cmd->type)
						{
						case Cmd::DrawMesh:
						{
							auto c = (CmdDrawMesh*)cmd.get();
							for (auto m : c->meshes)
							{
								cb->draw_indexed(m->idx_cnt, m->idx_off, m->vtx_off, 1, (obj_indirect_off << 16) + m->mat_idx);
								obj_indirect_off++;
							}
						}
							break;
						case Cmd::SetScissor:
						{
							auto c = (CmdSetScissor*)cmd.get();
							cb->set_scissor(c->scissor);
						}
							break;
						}
					}
					cb->end_renderpass();
					if (msaa_3d)
					{
						cb->image_barrier(msaa_resolve_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
						cb->set_scissor(Vec4f(Vec2f(0.f), (Vec2f)target_size));
						cb->begin_renderpass(dst_framebuffer.get());
						cb->bind_pipeline(hdr ? blit16_pipeline : blit8_pipeline);
						cb->bind_descriptor_set(msaa_descriptorset.get(), 0, sampler1_pc0_pipelinelayout);
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();
					}
				}
					break;
				case PassBlur:
				{
					auto c = (CmdBlur*)cmds[p.cmd_ids[0]].get();
					auto blur_radius = clamp(c->radius, 0U, 10U);
					auto blur_range = c->range;
					auto blur_size = Vec2f(blur_range.z() - blur_range.x(), blur_range.w() - blur_range.y());
					if (blur_size.x() < 1.f || blur_size.y() < 1.f)
						continue;

					cb->image_barrier(dst, {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
					cb->set_scissor(Vec4f(blur_range.x() - blur_radius, blur_range.y() - blur_radius,
						blur_range.z() + blur_radius, blur_range.w() + blur_radius));
					cb->begin_renderpass(back_framebuffers[0].get());
					cb->bind_pipeline(hdr ? blurh16_pipeline[blur_radius - 1] : blurh8_pipeline[blur_radius - 1]);
					cb->bind_descriptor_set(dst_ds, 0, sampler1_pc4_pipelinelayout);
					cb->push_constant_t(0, 1.f / target_size.x(), sampler1_pc4_pipelinelayout);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->image_barrier(back_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
					cb->set_scissor(blur_range);
					cb->begin_renderpass(dst_fb);
					cb->bind_pipeline(hdr ? blurv16_pipeline[blur_radius - 1] : blurv8_pipeline[blur_radius - 1]);
					cb->bind_descriptor_set(back_nearest_descriptorsets[0].get(), 0, sampler1_pc4_pipelinelayout);
					cb->push_constant_t(0, 1.f / target_size.y(), sampler1_pc4_pipelinelayout);
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
					cb->image_barrier(dst_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
					cb->begin_renderpass(back_framebuffers[0].get());
					cb->bind_pipeline(filter_bright_pipeline);
					cb->bind_descriptor_set(dst_descriptorset.get(), 0, sampler1_pc0_pipelinelayout);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					for (auto i = 0; i < back_image->level - 1; i++)
					{
						cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)back_image->sizes[i + 1]));
						cb->image_barrier(back_image.get(), { (uint)i }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
						cb->begin_renderpass(back_framebuffers[i + 1].get());
						cb->bind_pipeline(downsample_pipeline);
						cb->bind_descriptor_set(back_linear_descriptorsets[i].get(), 0, sampler1_pc8_pipelinelayout);
						cb->push_constant_t(0, 1.f / (Vec2f)back_image->sizes[i], sampler1_pc8_pipelinelayout);
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();
					}
					for (auto i = back_image->level - 1; i > 0; i--)
					{
						cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)back_image->sizes[i - 1]));
						cb->image_barrier(back_image.get(), { (uint)i }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
						cb->begin_renderpass(back_framebuffers[i - 1].get());
						cb->bind_pipeline(upsample_pipeline);
						cb->bind_descriptor_set(back_linear_descriptorsets[i].get(), 0, sampler1_pc8_pipelinelayout);
						cb->push_constant_t(0, 1.f / (Vec2f)back_image->sizes[(uint)i - 1], sampler1_pc8_pipelinelayout);
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();
					}
					cb->image_barrier(back_image.get(), { 1U }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
					cb->begin_renderpass(dst_framebuffer.get());
					cb->bind_pipeline(upsample_pipeline);
					cb->bind_descriptor_set(back_linear_descriptorsets[1].get(), 0, sampler1_pc8_pipelinelayout);
					cb->push_constant_t(0, 1.f / target_size.y(), sampler1_pc8_pipelinelayout);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)target_size));
				}
					break;
				}
			}

			if (hdr)
			{
				cb->image_barrier(dst_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
				cb->image_barrier(target_imageviews[image_index]->image, {}, ImageLayoutPresent, ImageLayoutShaderReadOnly);
				cb->set_scissor(Vec4f(Vec2f(0.f), Vec2f(target_size)));
				cb->begin_renderpass(target_framebuffers[image_index].get());
				cb->bind_pipeline(gamma_pipeline);
				cb->bind_descriptor_set(dst_descriptorset.get(), 0, sampler1_pc0_pipelinelayout);
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();
			}
			cb->image_barrier(target_imageviews[image_index]->image, {}, ImageLayoutShaderReadOnly, ImageLayoutPresent);

			cb->end();

			cmds.clear();
		}

		Canvas* Canvas::create(Device* d, bool hdr, bool msaa_3d)
		{
			return new CanvasPrivate((DevicePrivate*)d, hdr, msaa_3d);
		}
	}
}
