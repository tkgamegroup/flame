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
		RenderPreferencesPrivate::RenderPreferencesPrivate(DevicePrivate* device, bool hdr, bool msaa_3d) :
			device(device),
			hdr(hdr),
			msaa_3d(msaa_3d)
		{
			shadow_sampler.reset(new SamplerPrivate(device, FilterLinear, FilterLinear, AddressClampToBorder));

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
				image1_8_renderpass.reset(new RenderpassPrivate(device, { &att, 1 }, { &sp, 1 }));
				att.format = Format_R16G16B16A16_SFLOAT;
				image1_16_renderpass.reset(new RenderpassPrivate(device, { &att, 1 }, { &sp, 1 }));
				att.format = Format_R16_SFLOAT;
				image1_r16_renderpass.reset(new RenderpassPrivate(device, { &att, 1 }, { &sp, 1 }));
			}
			if (!msaa_3d)
			{
				RenderpassAttachmentInfo atts[2];
				atts[0].format = hdr ? Format_R16G16B16A16_SFLOAT : Swapchain::get_format();
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
				mesh_renderpass.reset(new RenderpassPrivate(device, atts, { &sp, 1 }));
			}
			else
			{
				RenderpassAttachmentInfo atts[3];
				atts[0].format = hdr ? Format_R16G16B16A16_SFLOAT : Swapchain::get_format();
				atts[0].load_op = AttachmentClear;
				atts[0].sample_count = msaa_sample_count;
				atts[0].initia_layout = ImageLayoutAttachment;
				atts[1].format = Format_Depth16;
				atts[1].load_op = AttachmentClear;
				atts[1].sample_count = msaa_sample_count;
				atts[1].initia_layout = ImageLayoutAttachment;
				atts[1].final_layout = ImageLayoutAttachment;
				atts[2].format = atts[0].format;
				atts[2].load_op = AttachmentDontCare;
				atts[2].initia_layout = ImageLayoutShaderReadOnly;
				RenderpassSubpassInfo sp;
				uint col_refs[] = {
					0
				};
				sp.color_attachments_count = 1;
				sp.color_attachments = col_refs;
				sp.depth_attachment = 1;
				uint res_refs[] = {
					2
				};
				sp.resolve_attachments_count = 1;
				sp.resolve_attachments = res_refs;

				mesh_renderpass.reset(new RenderpassPrivate(device, atts, { &sp, 1 }));
			}
			{
				RenderpassAttachmentInfo atts[2];
				atts[0].format = Format_R16_SFLOAT;
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
				depth_renderpass.reset(new RenderpassPrivate(device, atts, { &sp,1 }));
			}
			{
				DescriptorBindingInfo db;
				db.type = DescriptorSampledImage;
				db.count = 64;
				element_descriptorsetlayout.reset(new DescriptorSetLayoutPrivate(device, { &db, 1 }));
			}
			{
				DescriptorBindingInfo db;
				db.type = DescriptorStorageBuffer;
				mesh_descriptorsetlayout.reset(new DescriptorSetLayoutPrivate(device, { &db, 1 }));
			}
			{
				DescriptorBindingInfo db;
				db.type = DescriptorStorageBuffer;
				armature_descriptorsetlayout.reset(new DescriptorSetLayoutPrivate(device, { &db, 1 }));
			}
			{
				DescriptorBindingInfo dbs[2];
				dbs[0].type = DescriptorStorageBuffer;
				dbs[1].type = DescriptorSampledImage;
				dbs[1].count = 128;
				material_descriptorsetlayout.reset(new DescriptorSetLayoutPrivate(device, dbs));
			}
			{
				DescriptorBindingInfo dbs[5];
				dbs[0].type = DescriptorStorageBuffer;
				dbs[1].type = DescriptorStorageBuffer;
				dbs[2].type = DescriptorStorageBuffer;
				dbs[3].type = DescriptorSampledImage;
				dbs[3].count = 4;
				dbs[4].type = DescriptorSampledImage;
				dbs[4].count = 4;
				light_descriptorsetlayout.reset(new DescriptorSetLayoutPrivate(device, dbs));
			}
			{
				DescriptorBindingInfo db;
				db.type = DescriptorUniformBuffer;
				render_data_descriptorsetlayout.reset(new DescriptorSetLayoutPrivate(device, { &db, 1 }));
			}
			{
				DescriptorBindingInfo db;
				db.type = DescriptorStorageBuffer;
				terrain_descriptorsetlayout.reset(new DescriptorSetLayoutPrivate(device, { &db, 1 }));
			}
			{
				DescriptorBindingInfo db;
				db.type = DescriptorSampledImage;
				db.count = 1;
				sampler1_descriptorsetlayout.reset(new DescriptorSetLayoutPrivate(device, { &db, 1 }));
			}
			{
				DescriptorSetLayoutPrivate* dsls[] = {
					element_descriptorsetlayout.get()
				};
				element_pipelinelayout.reset(new PipelineLayoutPrivate(device, dsls, sizeof(Vec4f)));
			}
			{
				DescriptorSetLayoutPrivate* dsls[] = {
					render_data_descriptorsetlayout.get(),
					mesh_descriptorsetlayout.get(),
					material_descriptorsetlayout.get(),
					light_descriptorsetlayout.get(),
					armature_descriptorsetlayout.get()
				};
				mesh_pipelinelayout.reset(new PipelineLayoutPrivate(device, dsls, 0));
			}
			{
				DescriptorSetLayoutPrivate* dsls[] = {
					render_data_descriptorsetlayout.get(),
					material_descriptorsetlayout.get(),
					light_descriptorsetlayout.get(),
					terrain_descriptorsetlayout.get()
				};
				terrain_pipelinelayout.reset(new PipelineLayoutPrivate(device, dsls, 0));
			}
			{
				DescriptorSetLayoutPrivate* dsls[] = {
					mesh_descriptorsetlayout.get(),
					material_descriptorsetlayout.get(),
					armature_descriptorsetlayout.get()
				};
				depth_pipelinelayout.reset(new PipelineLayoutPrivate(device, dsls, sizeof(Mat4f) * 2));
			}
			{
				DescriptorSetLayoutPrivate* dsls[] = {
					sampler1_descriptorsetlayout.get()
				};
				sampler1_pc0_pipelinelayout.reset(new PipelineLayoutPrivate(device, dsls, 0));
				sampler1_pc4_pipelinelayout.reset(new PipelineLayoutPrivate(device, dsls, sizeof(float)));
				sampler1_pc8_pipelinelayout.reset(new PipelineLayoutPrivate(device, dsls, sizeof(Vec2f)));
			}
			{
				line_pipelinelayout.reset(new PipelineLayoutPrivate(device, {}, sizeof(Mat4f)));
			}

			element_vert.reset(ShaderPrivate::create(device, L"element.vert"));
			element_frag.reset(ShaderPrivate::create(device, L"element.frag"));
			mesh_vert.reset(ShaderPrivate::create(device, L"mesh.vert"));
			mesh_armature_vert.reset(ShaderPrivate::create(device, L"mesh.vert", "ARMATURE"));
			mesh_frag.reset(ShaderPrivate::create(device, L"mesh.frag"));
			terrain_vert.reset(ShaderPrivate::create(device, L"terrain.vert"));
			terrain_tesc.reset(ShaderPrivate::create(device, L"terrain.tesc"));
			terrain_tese.reset(ShaderPrivate::create(device, L"terrain.tese"));
			terrain_frag.reset(ShaderPrivate::create(device, L"terrain.frag"));
			depth_vert.reset(ShaderPrivate::create(device, L"depth.vert"));
			depth_armature_vert.reset(ShaderPrivate::create(device, L"depth.vert", "ARMATURE"));
			depth_frag.reset(ShaderPrivate::create(device, L"depth.frag"));
			line3_vert.reset(ShaderPrivate::create(device, L"line3.vert"));
			line3_frag.reset(ShaderPrivate::create(device, L"line3.frag"));
			fullscreen_vert.reset(ShaderPrivate::create(device, L"fullscreen.vert"));
			for (auto i = 0; i < 10; i++)
			{
				blurh_frag->reset(ShaderPrivate::create(device, L"blur.frag", "R" + std::to_string(i + 1) + " H"));
				blurv_frag->reset(ShaderPrivate::create(device, L"blur.frag", "R" + std::to_string(i + 1) + " V"));
			}
			blurh_depth_frag.reset(ShaderPrivate::create(device, L"blur_depth.frag", "H BAN"));
			blurv_depth_frag.reset(ShaderPrivate::create(device, L"blur_depth.frag", "V BAN"));
			blit_frag.reset(ShaderPrivate::create(device, L"blit.frag"));
			filter_bright_frag.reset(ShaderPrivate::create(device, L"filter_bright.frag"));
			box_frag.reset(ShaderPrivate::create(device, L"box.frag"));
			gamma_frag.reset(ShaderPrivate::create(device, L"gamma.frag"));

			{
				ShaderPrivate* shaders[] = {
					element_vert.get(),
					element_frag.get()
				};
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
				BlendOption bo;
				bo.enable = true;
				bo.src_color = BlendFactorSrcAlpha;
				bo.dst_color = BlendFactorOneMinusSrcAlpha;
				bo.src_alpha = BlendFactorOne;
				bo.dst_alpha = BlendFactorZero;
				element_pipeline.reset(PipelinePrivate::create(device, shaders, element_pipelinelayout.get(), hdr ? image1_16_renderpass.get() : image1_8_renderpass.get(), 0, & vi, nullptr, nullptr, { &bo, 1 }));
			}

			{
				ShaderPrivate* shaders[] = {
					mesh_vert.get(),
					mesh_frag.get()
				};
				VertexAttributeInfo vias1[3];
				vias1[0].location = 0;
				vias1[0].format = Format_R32G32B32_SFLOAT;
				vias1[1].location = 1;
				vias1[1].format = Format_R32G32_SFLOAT;
				vias1[2].location = 2;
				vias1[2].format = Format_R32G32B32_SFLOAT;
				//vias1[3].location = 3;
				//vias1[3].format = Format_R32G32B32_SFLOAT;
				//vias1[3].location = 4;
				//vias1[3].format = Format_R32G32B32_SFLOAT;
				VertexBufferInfo vibs[2];
				vibs[0].attributes_count = size(vias1);
				vibs[0].attributes = vias1;
				VertexInfo vi;
				vi.buffers_count = 1;
				vi.buffers = vibs;
				RasterInfo rst;
				DepthInfo dep;
				mesh_pipeline.reset(PipelinePrivate::create(device, shaders, mesh_pipelinelayout.get(), mesh_renderpass.get(), 0, &vi, &rst, &dep));

				shaders[0] = mesh_armature_vert.get();
				VertexAttributeInfo vias2[2];
				vias2[0].location = 5;
				vias2[0].format = Format_R32G32B32A32_INT;
				vias2[1].location = 6;
				vias2[1].format = Format_R32G32B32A32_SFLOAT;
				vibs[1].attributes_count = size(vias2);
				vibs[1].attributes = vias2;
				vi.buffers_count = 2;
				mesh_armature_pipeline.reset(PipelinePrivate::create(device, shaders, mesh_pipelinelayout.get(), mesh_renderpass.get(), 0, &vi, &rst, &dep));
			}

			make_terrain_pipeline();

			{
				ShaderPrivate* shaders[] = {
					depth_vert.get(),
					depth_frag.get()
				};
				VertexAttributeInfo vias1[2];
				vias1[0].location = 0;
				vias1[0].format = Format_R32G32B32_SFLOAT;
				vias1[1].location = 1;
				vias1[1].format = Format_R32G32_SFLOAT;
				VertexBufferInfo vibs[2];
				vibs[0].attributes_count = size(vias1);
				vibs[0].attributes = vias1;
				vibs[0].stride = 8 * sizeof(float);
				VertexInfo vi;
				vi.buffers_count = 1;
				vi.buffers = vibs;
				RasterInfo rst;
				rst.cull_mode = CullModeFront;
				DepthInfo dep;
				depth_pipeline.reset(PipelinePrivate::create(device, shaders, depth_pipelinelayout.get(), depth_renderpass.get(), 0, &vi, &rst, &dep));

				shaders[0] = depth_armature_vert.get();
				VertexAttributeInfo vias2[2];
				vias2[0].location = 2;
				vias2[0].format = Format_R32G32B32A32_INT;
				vias2[1].location = 3;
				vias2[1].format = Format_R32G32B32A32_SFLOAT;
				vibs[1].attributes_count = size(vias2);
				vibs[1].attributes = vias2;
				vi.buffers_count = 2;
				depth_armature_pipeline.reset(PipelinePrivate::create(device, shaders, depth_pipelinelayout.get(), depth_renderpass.get(), 0, &vi, &rst, &dep));
			}

			{
				ShaderPrivate* shaders[] = {
					line3_vert.get(),
					line3_frag.get()
				};
				VertexAttributeInfo vias[2];
				vias[0].location = 0;
				vias[0].format = Format_R32G32B32_SFLOAT;
				vias[1].location = 1;
				vias[1].format = Format_R8G8B8A8_UNORM;
				VertexBufferInfo vib;
				vib.attributes_count = size(vias);
				vib.attributes = vias;
				VertexInfo vi;
				vi.primitive_topology = PrimitiveTopologyLineList;
				vi.buffers_count = 1;
				vi.buffers = &vib;
				line3_pipeline.reset(PipelinePrivate::create(device, shaders, line_pipelinelayout.get(), hdr ? image1_16_renderpass.get() : image1_8_renderpass.get(), 0, &vi));
			}

			for (auto i = 0; i < 10; i++)
			{
				{
					ShaderPrivate* shaders[] = {
						fullscreen_vert.get(),
						blurh_frag->get()
					};
					blurh_pipeline[i].reset(PipelinePrivate::create(device, shaders, sampler1_pc4_pipelinelayout.get(), hdr ? image1_16_renderpass.get() : image1_8_renderpass.get(), 0));
				}

				{
					ShaderPrivate* shaders[] = {
						fullscreen_vert.get(),
						blurv_frag->get()
					};
					blurv_pipeline[i].reset(PipelinePrivate::create(device, shaders, sampler1_pc4_pipelinelayout.get(), hdr ? image1_16_renderpass.get() : image1_8_renderpass.get(), 0));
				}
			}

			{
				ShaderPrivate* shaders[] = {
					fullscreen_vert.get(),
					blurh_depth_frag.get()
				};
				blurh_depth_pipeline.reset(PipelinePrivate::create(device, shaders, sampler1_pc4_pipelinelayout.get(), image1_r16_renderpass.get(), 0));
			}

			{
				ShaderPrivate* shaders[] = {
					fullscreen_vert.get(),
					blurv_depth_frag.get()
				};
				blurv_depth_pipeline.reset(PipelinePrivate::create(device, shaders, sampler1_pc4_pipelinelayout.get(), image1_r16_renderpass.get(), 0));
			}

			{
				ShaderPrivate* shaders[] = {
					fullscreen_vert.get(),
					blit_frag.get()
				};
				blit_8_pipeline.reset(PipelinePrivate::create(device, shaders, sampler1_pc0_pipelinelayout.get(), image1_8_renderpass.get(), 0));
				blit_16_pipeline.reset(PipelinePrivate::create(device, shaders, sampler1_pc0_pipelinelayout.get(), image1_16_renderpass.get(), 0));
			}

			{
				ShaderPrivate* shaders[] = {
					fullscreen_vert.get(),
					filter_bright_frag.get()
				};
				filter_bright_pipeline.reset(PipelinePrivate::create(device, shaders, sampler1_pc0_pipelinelayout.get(), image1_16_renderpass.get(), 0));
			}

			{
				ShaderPrivate* shaders[] = {
					fullscreen_vert.get(),
					box_frag.get()
				};
				downsample_pipeline.reset(PipelinePrivate::create(device, shaders, sampler1_pc8_pipelinelayout.get(), image1_16_renderpass.get(), 0));
			}

			{
				ShaderPrivate* shaders[] = {
					fullscreen_vert.get(),
					box_frag.get()
				};
				BlendOption bo;
				bo.enable = true;
				bo.src_color = BlendFactorOne;
				bo.dst_color = BlendFactorOne;
				bo.src_alpha = BlendFactorZero;
				bo.dst_alpha = BlendFactorOne;
				upsample_pipeline.reset(PipelinePrivate::create(device, shaders, sampler1_pc8_pipelinelayout.get(), image1_16_renderpass.get(), 0, nullptr, nullptr, nullptr, { &bo, 1 }));
			}

			{
				ShaderPrivate* shaders[] = {
					fullscreen_vert.get(),
					gamma_frag.get()
				};
				gamma_pipeline.reset(PipelinePrivate::create(device, shaders, sampler1_pc0_pipelinelayout.get(), image1_8_renderpass.get(), 0));
			}
		}

		void RenderPreferencesPrivate::make_terrain_pipeline()
		{
			ShaderPrivate* shaders[] = {
				terrain_vert.get(),
				terrain_tesc.get(),
				terrain_tese.get(),
				terrain_frag.get()
			};
			VertexInfo vi;
			vi.primitive_topology = PrimitiveTopologyPatchList;
			vi.patch_control_points = 4;
			RasterInfo rst;
			if (terrain_render == RenderWireframe)
				rst.polygon_mode = PolygonModeLine;
			DepthInfo dep;
			terrain_pipeline.reset(PipelinePrivate::create(device, shaders, terrain_pipelinelayout.get(), mesh_renderpass.get(), 0, &vi, &rst, &dep));
		}

		void RenderPreferencesPrivate::set_terrain_render(RenderType type)
		{
			if (terrain_render != type)
			{
				terrain_render = type;
				switch (type)
				{
				case RenderNormal:
					terrain_frag.reset(ShaderPrivate::create(device, L"terrain.frag"));
					break;
				case RenderWireframe:
					terrain_frag.reset(ShaderPrivate::create(device, L"terrain.frag", "WIREFRAME"));
					break;
				}
				make_terrain_pipeline();
			}
		}

		RenderPreferences* RenderPreferences::create(Device* device, bool hdr, bool msaa_3d)
		{
			return new RenderPreferencesPrivate((DevicePrivate*)device, hdr, msaa_3d);
		}

		ArmatureDeformerPrivate::ArmatureDeformerPrivate(RenderPreferencesPrivate* preferences, MeshPrivate* mesh) :
			mesh(mesh)
		{
			poses_buffer.create(preferences->device, mesh->bones.size());
			descriptorset.reset(new DescriptorSetPrivate(preferences->device->dsp.get(), preferences->armature_descriptorsetlayout.get()));
			descriptorset->set_buffer(0, 0, poses_buffer.buf.get());
		}

		void ArmatureDeformerPrivate::set_pose(uint id, const Mat4f& pose)
		{
			if (id < dirty_range[0])
				dirty_range[0] = id;
			if (id >= dirty_range[1])
				dirty_range[1] = id + 1;
			poses_buffer.beg[id] = pose * mesh->bones[id]->offset_matrix;
		}

		ArmatureDeformer* ArmatureDeformer::create(RenderPreferences* preferences, Mesh* mesh)
		{
			return new ArmatureDeformerPrivate((RenderPreferencesPrivate*)preferences, (MeshPrivate*)mesh);
		}


		CanvasPrivate::CanvasPrivate(RenderPreferencesPrivate* preferences) :
			preferences(preferences)
		{
			auto device = preferences->device;

			ImmediateCommandBuffer cb;

			white_image.reset(new ImagePrivate(device, Format_R8G8B8A8_UNORM, Vec2u(1), 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled));
			cb->image_barrier(white_image.get(), {}, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->clear_color_image(white_image.get(), Vec4c(255));
			cb->image_barrier(white_image.get(), {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			auto iv_wht = white_image->views[0].get();

			element_resources.resize(64);
			for (auto i = 0; i < element_resources.size(); i++)
				element_resources[i].iv = iv_wht;

			texture_resources.resize(128);
			for (auto i = 0; i < texture_resources.size(); i++)
				texture_resources[i].iv = iv_wht;
			material_resources.resize(128);
			model_resources.resize(128);

			element_vertex_buffer.create(device, 360000);
			element_index_buffer.create(device, 240000);
			element_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), preferences->element_descriptorsetlayout.get()));
			for (auto i = 0; i < element_resources.size(); i++)
				element_descriptorset->set_image(0, i, iv_wht, device->lsp.get());
			
			mesh_matrix_buffer.create(device, 10000);
			mesh_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), preferences->mesh_descriptorsetlayout.get()));
			mesh_descriptorset->set_buffer(0, 0, mesh_matrix_buffer.buf.get());

			material_info_buffer.create(device, 128);
			material_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), preferences->material_descriptorsetlayout.get()));
			material_descriptorset->set_buffer(0, 0, material_info_buffer.buf.get());
			for (auto i = 0; i < texture_resources.size(); i++)
				material_descriptorset->set_image(1, i, iv_wht, device->lsp.get());

			light_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), preferences->light_descriptorsetlayout.get()));
			{
				shadow_depth_image.reset(new ImagePrivate(device, Format_Depth16, shadow_map_size, 1, 1, SampleCount_1, ImageUsageAttachment));
				cb->image_barrier(shadow_depth_image.get(), {}, ImageLayoutUndefined, ImageLayoutAttachment);

				shadow_blur_pingpong_image.reset(new ImagePrivate(device, Format_R16_SFLOAT, shadow_map_size, 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
				cb->image_barrier(shadow_blur_pingpong_image.get(), {}, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
				{
					auto iv = shadow_blur_pingpong_image->views[0].get();
					shadow_blur_pingpong_image_framebuffer.reset(new FramebufferPrivate(device, preferences->image1_r16_renderpass.get(), { &iv, 1 }));
					shadow_blur_pingpong_image_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), preferences->sampler1_descriptorsetlayout.get()));
					shadow_blur_pingpong_image_descriptorset->set_image(0, 0, iv, device->nsp.get());
				}

				light_indices_buffer.create(device, 1);
				light_descriptorset->set_buffer(0, 0, light_indices_buffer.buf.get());

				directional_light_info_buffer.create(device, 1);
				light_descriptorset->set_buffer(1, 0, directional_light_info_buffer.buf.get());
				directional_light_shadow_maps.resize(4);
				directional_light_shadow_map_depth_framebuffers.resize(directional_light_shadow_maps.size() * 4);
				directional_light_shadow_map_framebuffers.resize(directional_light_shadow_maps.size() * 4);
				directional_light_shadow_map_descriptorsets.resize(directional_light_shadow_maps.size() * 4);
				for (auto i = 0; i < directional_light_shadow_maps.size(); i++)
				{
					directional_light_shadow_maps[i].reset(new ImagePrivate(device, Format_R16_SFLOAT, shadow_map_size, 1, 4, SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
					cb->image_barrier(directional_light_shadow_maps[i].get(), { 0U, 1U, 0U, 4U }, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
					for (auto j = 0; j < 4; j++)
					{
						auto iv = new ImageViewPrivate(directional_light_shadow_maps[i].get(), true, ImageView2D, { 0U, 1U, (uint)j, 1U });
						ImageViewPrivate* vs[] = {
							iv,
							shadow_depth_image->views[0].get()
						};
						directional_light_shadow_map_depth_framebuffers[i * 4 + j].reset(new FramebufferPrivate(device, preferences->depth_renderpass.get(), vs));
						directional_light_shadow_map_framebuffers[i * 4 + j].reset(new FramebufferPrivate(device, preferences->image1_r16_renderpass.get(), { &iv, 1 }));
						directional_light_shadow_map_descriptorsets[i * 4 + j].reset(new DescriptorSetPrivate(device->dsp.get(), preferences->sampler1_descriptorsetlayout.get()));
						directional_light_shadow_map_descriptorsets[i * 4 + j]->set_image(0, 0, iv, device->lsp.get());
					}
					auto iv = new ImageViewPrivate(directional_light_shadow_maps[i].get(), true, ImageView2DArray, { 0U, 1U, (uint)0, 4U });
					light_descriptorset->set_image(3, i, iv, preferences->shadow_sampler.get());
				}

				point_light_info_buffer.create(device, 10000);
				light_descriptorset->set_buffer(2, 0, point_light_info_buffer.buf.get());
				point_light_shadow_maps.resize(4);
				point_light_shadow_map_depth_framebuffers.resize(point_light_shadow_maps.size() * 6);
				point_light_shadow_map_framebuffers.resize(point_light_shadow_maps.size() * 6);
				point_light_shadow_map_descriptorsets.resize(point_light_shadow_maps.size() * 6);
				for (auto i = 0; i < point_light_shadow_maps.size(); i++)
				{
					point_light_shadow_maps[i].reset(new ImagePrivate(device, Format_R16_SFLOAT, shadow_map_size, 1, 6, SampleCount_1, ImageUsageSampled | ImageUsageAttachment, true));
					cb->image_barrier(point_light_shadow_maps[i].get(), { 0U, 1U, 0U, 6U }, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
					for (auto j = 0; j < 6; j++)
					{
						auto iv = new ImageViewPrivate(point_light_shadow_maps[i].get(), true, ImageView2D, { 0U, 1U, (uint)j, 1U });
						ImageViewPrivate* vs[] = {
							iv,
							shadow_depth_image->views[0].get()
						};
						point_light_shadow_map_depth_framebuffers[i * 6 + j].reset(new FramebufferPrivate(device, preferences->depth_renderpass.get(), vs));
						point_light_shadow_map_framebuffers[i * 6 + j].reset(new FramebufferPrivate(device, preferences->image1_r16_renderpass.get(), { &iv, 1 }));
						point_light_shadow_map_descriptorsets[i * 6 + j].reset(new DescriptorSetPrivate(device->dsp.get(), preferences->sampler1_descriptorsetlayout.get()));
						point_light_shadow_map_descriptorsets[i * 6 + j]->set_image(0, 0, iv, device->lsp.get());
					}
					auto iv = new ImageViewPrivate(point_light_shadow_maps[i].get(), true, ImageViewCube, { 0U, 1U, 0U, 6U });
					light_descriptorset->set_image(4, i, iv, preferences->shadow_sampler.get());
				}
			}

			render_data_buffer.create(device, 1);
			render_data_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), preferences->render_data_descriptorsetlayout.get()));
			render_data_descriptorset->set_buffer(0, 0, render_data_buffer.buf.get());

			terrain_info_buffer.create(device, 1);
			terrain_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), preferences->terrain_descriptorsetlayout.get()));
			terrain_descriptorset->set_buffer(0, 0, terrain_info_buffer.buf.get());

			set_model_resource(-1, (ModelPrivate*)Model::get_standard("cube"), "cube");
			set_model_resource(-1, (ModelPrivate*)Model::get_standard("sphere"), "sphere");

			line3_buffer.create(device, 200000);
		}

		void CanvasPrivate::set_output(std::span<ImageViewPrivate*> views)
		{
			auto device = preferences->device;
			auto hdr = preferences->hdr;
			auto msaa_3d = preferences->msaa_3d;

			output_imageviews.clear();
			output_framebuffers.clear();
			output_descriptorsets.clear();

			hdr_image.reset();
			hdr_framebuffer.reset();
			hdr_descriptorset.reset();

			depth_image.reset();

			mesh_framebuffers.clear();

			msaa_image.reset();
			msaa_resolve_image.reset();
			msaa_descriptorset.reset();

			back_image.reset();
			back_framebuffers.clear();
			back_nearest_descriptorsets.clear();
			back_linear_descriptorsets.clear();

			if (views.empty())
				output_size = 0.f;
			else
			{
				ImmediateCommandBuffer cb;

				output_size = views[0]->image->sizes[0];
				output_imageviews.resize(views.size());
				output_framebuffers.resize(views.size());
				output_descriptorsets.resize(views.size());
				for (auto i = 0; i < views.size(); i++)
				{
					output_imageviews[i] = views[i];
					output_framebuffers[i].reset(new FramebufferPrivate(device, preferences->image1_8_renderpass.get(), { &views[i], 1 }));
					output_descriptorsets[i].reset(new DescriptorSetPrivate(device->dsp.get(), preferences->sampler1_descriptorsetlayout.get()));
					output_descriptorsets[i]->set_image(0, 0, views[i], device->nsp.get());
				}

				if (hdr)
				{
					hdr_image.reset(new ImagePrivate(device, Format_R16G16B16A16_SFLOAT, output_size, 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled | ImageUsageAttachment));
					cb->image_barrier(hdr_image.get(), {}, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
					auto iv = hdr_image->views[0].get();
					hdr_framebuffer.reset(new FramebufferPrivate(device, preferences->image1_16_renderpass.get(), { &iv, 1 }));
					hdr_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), preferences->sampler1_descriptorsetlayout.get()));
					hdr_descriptorset->set_image(0, 0, iv, device->nsp.get());
				}

				if (msaa_3d)
					depth_image.reset(new ImagePrivate(device, Format_Depth16, output_size, 1, 1, msaa_sample_count, ImageUsageAttachment));
				else
					depth_image.reset(new ImagePrivate(device, Format_Depth16, output_size, 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled | ImageUsageAttachment));
				cb->image_barrier(depth_image.get(), {}, ImageLayoutUndefined, ImageLayoutAttachment);

				if (msaa_3d)
				{
					msaa_image.reset(new ImagePrivate(device, hdr ? Format_R16G16B16A16_SFLOAT : Format_R8G8B8A8_UNORM, output_size, 1, 1, msaa_sample_count, ImageUsageAttachment));
					cb->image_barrier(msaa_image.get(), {}, ImageLayoutUndefined, ImageLayoutAttachment);
					msaa_resolve_image.reset(new ImagePrivate(device, hdr ? Format_R16G16B16A16_SFLOAT : Format_R8G8B8A8_UNORM, output_size, 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
					cb->image_barrier(msaa_resolve_image.get(), {}, ImageLayoutUndefined, ImageLayoutShaderReadOnly);

					auto iv_res = msaa_resolve_image->views[0].get();
					msaa_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), preferences->sampler1_descriptorsetlayout.get()));
					msaa_descriptorset->set_image(0, 0, iv_res, device->nsp.get());

					mesh_framebuffers.resize(1);
					ImageViewPrivate* vs[] = {
						msaa_image->views[0].get(),
						depth_image->views[0].get(),
						iv_res
					};
					mesh_framebuffers[0].reset(new FramebufferPrivate(device, preferences->mesh_renderpass.get(), vs));
				}
				else
				{
					if (hdr)
					{
						mesh_framebuffers.resize(1);
						ImageViewPrivate* vs[] = {
							hdr_image->views[0].get(),
							depth_image->views[0].get()
						};
						mesh_framebuffers[0].reset(new FramebufferPrivate(device, preferences->mesh_renderpass.get(), vs));
					}
					else
					{
						mesh_framebuffers.resize(views.size());
						for (auto i = 0; i < views.size(); i++)
						{
							ImageViewPrivate* vs[] = {
								views[i],
								depth_image->views[0].get()
							};
							mesh_framebuffers[i].reset(new FramebufferPrivate(device, preferences->mesh_renderpass.get(), vs));
						}
					}
				}

				back_image.reset(new ImagePrivate(device, hdr ? Format_R16G16B16A16_SFLOAT : Format_B8G8R8A8_UNORM, output_size, 0xFFFFFFFF, 1, SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
				cb->image_barrier(back_image.get(), { 0U, back_image->level, 0U, 1U }, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
				back_framebuffers.resize(back_image->level);
				back_nearest_descriptorsets.resize(back_image->level);
				back_linear_descriptorsets.resize(back_image->level);
				for (auto i = 0; i < back_image->level; i++)
				{
					auto iv = back_image->views[i].get();
					back_framebuffers[i].reset(new FramebufferPrivate(device, hdr ? preferences->image1_16_renderpass.get() : preferences->image1_8_renderpass.get(), { &iv, 1 }));
					back_nearest_descriptorsets[i].reset(new DescriptorSetPrivate(device->dsp.get(), preferences->sampler1_descriptorsetlayout.get()));
					back_linear_descriptorsets[i].reset(new DescriptorSetPrivate(device->dsp.get(), preferences->sampler1_descriptorsetlayout.get()));
					back_nearest_descriptorsets[i]->set_image(0, 0, iv, device->nsp.get());
					back_linear_descriptorsets[i]->set_image(0, 0, iv, device->lsp.get());
				}
			}
		}

		ElementResource CanvasPrivate::get_element_resource(uint slot)
		{
			if (slot < element_resources.size())
			{
				auto& r = element_resources[slot];
				return { r.iv, r.ia, r.fa };
			}
			return { nullptr, nullptr, nullptr };
		}

		int CanvasPrivate::find_element_resource(const std::string& name)
		{
			for (auto i = 0; i < element_resources.size(); i++)
			{
				if (element_resources[i].name == name)
					return i;
			}
			return -1;
		}

		uint CanvasPrivate::set_element_resource(int slot, ElementResource r, const std::string& name)
		{
			auto device = preferences->device;
			auto iv_wht = white_image->views[0].get();
			if (slot == -1)
			{
				for (auto i = 1; i < element_resources.size(); i++)
				{
					if (element_resources[i].iv == iv_wht)
					{
						slot = i;
						break;
					}
				}
			}
			if (slot != -1)
			{
				auto& dst = element_resources[slot];
				dst.name = name;
				dst.iv = (ImageViewPrivate*)r.iv;
				dst.ia = (ImageAtlasPrivate*)r.ia;
				dst.fa = (FontAtlasPrivate*)r.fa;
				if (dst.ia)
					element_descriptorset->set_image(0, slot, dst.ia->image->views[0].get(), dst.ia->border ? device->lsp.get() : device->nsp.get());
				else if (dst.fa)
					element_descriptorset->set_image(0, slot, dst.fa->view, device->nsp.get());
				else
					element_descriptorset->set_image(0, slot, dst.iv ? dst.iv : iv_wht, device->lsp.get());
			}
			return slot;
		}

		ImageView* CanvasPrivate::get_texture_resource(uint slot)
		{
			if (slot < texture_resources.size())
			{
				auto& r = texture_resources[slot];
				r.iv;
			}
			return nullptr;
		}

		int CanvasPrivate::find_texture_resource(const std::string& name)
		{
			for (auto i = 0; i < texture_resources.size(); i++)
			{
				if (texture_resources[i].name == name)
					return i;
			}
			return -1;
		}

		uint CanvasPrivate::set_texture_resource(int slot, ImageViewPrivate* iv, SamplerPrivate* sp, const std::string& name)
		{
			auto device = preferences->device;
			auto iv_wht = white_image->views[0].get();
			if (slot == -1)
			{
				for (auto i = 1; i < texture_resources.size(); i++)
				{
					if (texture_resources[i].iv == iv_wht)
					{
						slot = i;
						break;
					}
				}
			}
			if (slot != -1)
			{
				auto& r = texture_resources[slot];
				r.name = name;
				r.iv = iv;
				material_descriptorset->set_image(1, slot, iv ? iv : iv_wht, iv && sp ? sp : device->lsp.get());
			}
			return slot;
		}

		Material* CanvasPrivate::get_material_resource(uint slot)
		{
			if (slot < material_resources.size())
			{
				auto& r = material_resources[slot];
				r->material;
			}
			return nullptr;
		}

		int CanvasPrivate::find_material_resource(const std::string& name)
		{
			for (auto i = 0; i < material_resources.size(); i++)
			{
				if (material_resources[i] && material_resources[i]->name == name)
					return i;
			}
			return -1;
		}

		uint CanvasPrivate::set_material_resource(int slot, MaterialPrivate* mat, const std::string& name)
		{
			auto device = preferences->device;
			auto sp = device->lsp.get();

			if (slot == -1)
			{
				for (auto i = 1; i < material_resources.size(); i++)
				{
					if (!material_resources[i])
					{
						slot = i;
						break;
					}
				}
			}
			if (slot != -1)
			{
				{
					auto prev = material_resources[slot].get();
					if (prev)
					{
						for (auto& t : prev->textures)
							set_texture_resource(t.first, nullptr, nullptr, "");
					}
				}

				if (!mat)
					material_resources[slot].reset();
				else
				{
					auto mr = new MaterialResourceSlot;
					mr->name = name;
					material_resources[slot].reset(mr);

					MaterialInfoS mis;
					mis.color = mat->color;
					mis.metallic = mat->metallic;
					mis.roughness = mat->roughness;
					mis.alpha_test = mat->alpha_test;

					if (!mat->path.empty())
					{
						auto color_map = !mat->color_map.empty() ? Bitmap::create((mat->path / mat->color_map).c_str()) : nullptr;
						auto alpha_map = !mat->alpha_map.empty() ? Bitmap::create((mat->path / mat->alpha_map).c_str()) : nullptr;
						auto roughness_map = !mat->roughness_map.empty() ? Bitmap::create((mat->path / mat->roughness_map).c_str()) : nullptr;
						auto normal_map = !mat->normal_map.empty() ? Bitmap::create((mat->path / mat->normal_map).c_str()) : nullptr;
						auto height_map = !mat->height_map.empty() ? Bitmap::create((mat->path / mat->height_map).c_str()) : nullptr;

						if (color_map || alpha_map)
						{
							Vec2u size = Vec2u(0U);
							if (color_map)
								size = max(size, Vec2u(color_map->get_width(), color_map->get_height()));
							if (alpha_map)
								size = max(size, Vec2u(alpha_map->get_width(), alpha_map->get_height()));

							auto dst_map = Bitmap::create(size.x(), size.y());
							auto dst_map_d = dst_map->get_data();
							auto dst_map_pitch = size.x() * 4;

							auto color_map_d = color_map ? color_map->get_data() : nullptr;
							auto color_map_pitch = color_map ? color_map->get_pitch() : 0;
							auto color_map_ch = color_map ? color_map->get_channel() : 0;
							auto alpha_map_d = alpha_map ? alpha_map->get_data() : nullptr;
							auto alpha_map_pitch = alpha_map ? alpha_map->get_pitch() : 0;
							auto alpha_map_ch = alpha_map ? alpha_map->get_channel() : 0;

							if (color_map)
								color_map->srgb_to_linear();

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
								auto a = mat->color.a() * 255;
								for (auto y = 0; y < size.y(); y++)
								{
									for (auto x = 0; x < size.x(); x++)
									{
										(Vec4c&)dst_map_d[y * dst_map_pitch + x * 4] =
											Vec4c((Vec3c&)color_map_d[y * color_map_pitch + x * color_map_ch], a);
									}
								}
							}
							else if (!color_map && alpha_map)
							{
								auto col = Vec3c(Vec3f(mat->color) * 255.f);
								for (auto y = 0; y < size.y(); y++)
								{
									for (auto x = 0; x < size.x(); x++)
									{
										(Vec4c&)dst_map_d[y * dst_map_pitch + x * 4] =
											Vec4c(col, alpha_map_d[y * alpha_map_pitch + x * alpha_map_ch]);
									}
								}
							}

							auto img = ImagePrivate::create(device, dst_map);
							auto idx = set_texture_resource(-1, img->views.back().get(), nullptr, "");
							mr->textures.emplace_back(idx, img);
							mis.color_map_index = idx;

							dst_map->release();
						}

						if (roughness_map)
						{
							Vec2u size = Vec2u(0U);
							if (roughness_map)
								size = max(size, Vec2u(roughness_map->get_width(), roughness_map->get_height()));

							auto dst_map = Bitmap::create(size.x(), size.y());
							auto dst_map_d = dst_map->get_data();
							auto dst_map_pitch = size.x() * 4;

							auto roughness_map_d = roughness_map ? roughness_map->get_data() : nullptr;
							auto roughness_map_pitch = roughness_map ? roughness_map->get_pitch() : 0;
							auto roughness_map_ch = roughness_map ? roughness_map->get_channel() : 0;

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
							auto idx = set_texture_resource(-1, img->views.back().get(), nullptr, "");
							mr->textures.emplace_back(idx, img);
							mis.metallic_roughness_ao_map_index = idx;

							dst_map->release();
						}

						if (normal_map || height_map)
						{
							Vec2u size = Vec2u(0U);
							if (normal_map)
								size = max(size, Vec2u(normal_map->get_width(), normal_map->get_height()));
							if (height_map)
								size = max(size, Vec2u(height_map->get_width(), height_map->get_height()));

							auto dst_map = Bitmap::create(size.x(), size.y());
							auto dst_map_d = dst_map->get_data();
							auto dst_map_pitch = size.x() * 4;

							auto height_map_d = height_map ? height_map->get_data() : nullptr;
							auto height_map_pitch = height_map ? height_map->get_pitch() : 0;
							auto height_map_ch = height_map ? height_map->get_channel() : 0;

							if (normal_map && height_map)
							{

							}
							else if (normal_map && !height_map)
							{

							}
							else if (!normal_map && height_map)
							{
								for (auto y = 0; y < size.y(); y++)
								{
									for (auto x = 0; x < size.x(); x++)
									{
										(Vec4c&)dst_map_d[y * dst_map_pitch + x * 4] =
											Vec4c(Vec3f(0.f), height_map_d[y * height_map_pitch + x * height_map_ch]);
									}
								}
							}

							auto img = ImagePrivate::create(device, dst_map);
							auto idx = set_texture_resource(-1, img->views.back().get(), nullptr, "");
							mr->textures.emplace_back(idx, img);
							mis.normal_hegiht_map_index = idx;

							dst_map->release();
						}

						if (color_map)
							color_map->release();
						if (alpha_map)
							alpha_map->release();
						if (roughness_map)
							roughness_map->release();
						if (normal_map)
							normal_map->release();
						if (height_map)
							height_map->release();
					}

					material_info_buffer.push(slot, mis);

					ImmediateCommandBuffer cb;
					material_info_buffer.upload(cb.cb.get(), slot, 1);
				}
			}
			return slot;
		}

		Model* CanvasPrivate::get_model_resource(uint slot)
		{
			if (slot < model_resources.size())
			{
				auto& r = model_resources[slot];
				r->model;
			}
			return nullptr;
		}

		int CanvasPrivate::find_model_resource(const std::string& name)
		{
			for (auto i = 0; i < model_resources.size(); i++)
			{
				if (model_resources[i] && model_resources[i]->name == name)
					return i;
			}
			return -1;
		}

		uint CanvasPrivate::set_model_resource(int slot, ModelPrivate* mod, const std::string& name)
		{
			auto device = preferences->device;
			if (slot == -1)
			{
				for (auto i = 0; i < model_resources.size(); i++)
				{
					if (!model_resources[i])
					{
						slot = i;
						break;
					}
				}
			}
			if (slot != -1)
			{
				{
					auto prev = model_resources[slot].get();
					if (prev)
					{
						for (auto& m : prev->materials)
							set_material_resource(m, nullptr, "");
					}
				}

				if (!mod)
					model_resources[slot].reset();
				else
				{
					auto mr = new ModelResourceSlot;
					mr->name = name;
					mr->model = mod;
					model_resources[slot].reset(mr);

					mr->materials.resize(mod->materials.size());
					for (auto i = 0; i < mod->materials.size(); i++)
						mr->materials[i] = set_material_resource(-1, mod->materials[i].get(), "");

					ImmediateCommandBuffer cb;

					mr->meshes.resize(mod->meshes.size());
					for (auto i = 0; i < mod->meshes.size(); i++)
					{
						auto ms = mod->meshes[i].get();

						auto mrm = new ModelResourceSlot::Mesh;

						mrm->vertex_buffer.create(device, ms->positions.size());
						std::vector<MeshVertex> vertices;
						vertices.resize(ms->positions.size());
						for (auto j = 0; j < vertices.size(); j++)
							vertices[j].position = ms->positions[j];
						if (!ms->uvs.empty())
						{
							for (auto j = 0; j < vertices.size(); j++)
								vertices[j].uv = ms->uvs[j];
						}
						if (!ms->normals.empty())
						{
							for (auto j = 0; j < vertices.size(); j++)
								vertices[j].normal = ms->normals[j];
						}
						mrm->vertex_buffer.push(vertices.size(), vertices.data());
						mrm->vertex_buffer.upload(cb.cb.get());

						if (!ms->bones.empty())
						{
							mrm->weight_buffer.create(device, ms->positions.size());
							std::vector<std::vector<std::pair<uint, float>>> weights;
							weights.resize(ms->positions.size());
							for (auto j = 0; j < ms->bones.size(); j++)
							{
								auto& b = ms->bones[j];
								for (auto& w : b->weights)
									weights[w.first].emplace_back(j, w.second);
							}
							for (auto& w : weights)
							{
								std::sort(w.begin(), w.end(), [](const auto& a, const auto& b) {
									return a.second < b.second;
								});
							}
							std::vector<MeshWeight> mesh_weights;
							mesh_weights.resize(weights.size());
							for (auto j = 0; j < weights.size(); j++)
							{
								auto& src = weights[j];
								auto& dst = mesh_weights[j];
								for (auto k = 0; k < 4; k++)
								{
									if (k < src.size())
									{
										dst.ids[k] = src[k].first;
										dst.weights[k] = src[k].second;
									}
									else
										dst.ids[k] = -1;
								}
							}
							mrm->weight_buffer.push(mesh_weights.size(), mesh_weights.data());
							mrm->weight_buffer.upload(cb.cb.get());
						}

						mrm->index_buffer.create(device, ms->indices.size());
						mrm->index_buffer.push(ms->indices.size(), ms->indices.data());
						mrm->index_buffer.upload(cb.cb.get());

						mrm->material_index = mr->materials[ms->material_index];

						mr->meshes[i].reset(mrm);
					}
				}
			}
			return slot;
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

		void CanvasPrivate::add_vtx(const Vec2f& position, const Vec2f& uv, const Vec4c& color)
		{
			ElementVertex v;
			v.position = position;
			v.uv = uv;
			v.color = color;
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

			add_draw_element_cmd(0);
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
			add_draw_element_cmd(0);
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
			auto& res = element_resources[res_id];
			if (!res.fa)
				return;
			auto atlas = res.fa;

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
			if (res_id >= element_resources.size())
				res_id = 0;
			auto& res = element_resources[res_id];
			auto _uv0 = uv0;
			auto _uv1 = uv1;
			if (res.ia)
			{
				auto tile = res.ia->tiles[tile_id].get();
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

		static void get_frustum_points(float zNear, float zFar, float tan_hf_fovy, float aspect, const Mat4f& transform, Vec3f* dst)
		{
			auto y1 = zNear * tan_hf_fovy;
			auto y2 = zFar * tan_hf_fovy;
			auto x1 = y1 * aspect;
			auto x2 = y2 * aspect;

			dst[0] = Vec3f(transform * Vec4f(-x1, y1, -zNear, 1.f));
			dst[1] = Vec3f(transform * Vec4f(x1, y1, -zNear, 1.f));
			dst[2] = Vec3f(transform * Vec4f(x1, -y1, -zNear, 1.f));
			dst[3] = Vec3f(transform * Vec4f(-x1, -y1, -zNear, 1.f));
			dst[4] = Vec3f(transform * Vec4f(-x2, y2, -zFar, 1.f));
			dst[5] = Vec3f(transform * Vec4f(x2, y2, -zFar, 1.f));
			dst[6] = Vec3f(transform * Vec4f(x2, -y2, -zFar, 1.f));
			dst[7] = Vec3f(transform * Vec4f(-x2, -y2, -zFar, 1.f));
		}

		void CanvasPrivate::set_camera(float fovy, float aspect, float zNear, float zFar, const Mat3f& axes, const Vec3f& coord)
		{
			auto view_inv = Mat4f(Mat<3, 4, float>(axes, Vec3f(0.f)), Vec4f(coord, 1.f));
			auto view = inverse(view_inv);
			auto proj = make_perspective_project_matrix(fovy, aspect, zNear, zFar);

			render_data_buffer.beg->fovy = fovy;
			render_data_buffer.beg->aspect = aspect;
			render_data_buffer.beg->zNear = zNear;
			render_data_buffer.beg->zFar = zFar;
			render_data_buffer.beg->camera_coord = coord;
			render_data_buffer.beg->view_inv = view_inv;
			render_data_buffer.beg->view = view;
			render_data_buffer.beg->proj = proj;
			render_data_buffer.beg->proj_view = proj * view;

			{
				auto p = proj * Vec4f(100, 0, -100, 1);
				p /= p.w();
				int cut = 1;
			}
			{
				auto p = proj * Vec4f(-100, 0, -100, 1);
				p /= p.w();
				int cut = 1;
			}
			{
				auto p = proj * Vec4f(100, 0, -200, 1);
				p /= p.w();
				int cut = 1;
			}
			{
				auto p = proj * Vec4f(-100, 0, -200, 1);
				p /= p.w();
				int cut = 1;
			}

			Vec3f ps[8];
			get_frustum_points(zNear, zFar, tan(fovy * 0.5f * ANG_RAD), aspect, view_inv, ps);
			render_data_buffer.beg->frustum_planes[0] = make_plane(ps[0], ps[1], ps[2]); // near
			render_data_buffer.beg->frustum_planes[1] = make_plane(ps[5], ps[4], ps[6]); // far
			render_data_buffer.beg->frustum_planes[2] = make_plane(ps[4], ps[0], ps[7]); // left
			render_data_buffer.beg->frustum_planes[3] = make_plane(ps[1], ps[5], ps[2]); // right
			render_data_buffer.beg->frustum_planes[4] = make_plane(ps[4], ps[5], ps[0]); // top
			render_data_buffer.beg->frustum_planes[5] = make_plane(ps[3], ps[2], ps[7]); // bottom
		}

		void CanvasPrivate::draw_mesh(uint mod_id, uint mesh_idx, const Mat4f& transform, const Mat4f& normal_matrix, bool cast_shadow, ArmatureDeformer* deformer)
		{
			if (cmds.empty() || cmds.back()->type != Cmd::DrawMesh)
			{
				last_mesh_cmd = new CmdDrawMesh;
				cmds.emplace_back(last_mesh_cmd);
			}

			auto idx = mesh_matrix_buffer.stg_num();

			MeshMatrixS om;
			om.transform = transform;
			om.normal_matrix = normal_matrix;
			mesh_matrix_buffer.push(om);

			last_mesh_cmd->meshes.emplace_back(idx, model_resources[mod_id]->meshes[mesh_idx].get(), cast_shadow, (ArmatureDeformerPrivate*)deformer);
		}

		void CanvasPrivate::draw_terrain(const Vec2u& blocks, const Vec3f& scale, const Vec3f& coord, float tess_levels, uint height_tex_id, uint normal_tex_id, uint color_tex_id)
		{
			auto cmd = new CmdDrawTerrain;
			cmds.emplace_back(cmd);

			cmd->idx = terrain_info_buffer.stg_num();

			TerrainInfoS ti;
			ti.coord = coord;
			ti.blocks = blocks;
			ti.scale = scale;
			ti.tess_levels = tess_levels;
			ti.height_tex_id = height_tex_id;
			ti.normal_tex_id = normal_tex_id;
			ti.color_tex_id = color_tex_id;
			terrain_info_buffer.push(ti);
		}

		void CanvasPrivate::add_light(LightType type, const Mat4f& transform, const Vec3f& color, bool cast_shadow)
		{
			if (type == LightDirectional)
			{
				auto dir = normalize(-Vec3f(transform[2]));
				auto side = normalize(Vec3f(transform[0]));
				auto up = normalize(Vec3f(transform[1]));

				DirectionalLightInfoS li;
				li.dir = dir;
				li.distance = 100.f;
				li.color = color;
				li.shadow_map_index = cast_shadow ? used_directional_light_shadow_maps_count++ : -1;
				if (cast_shadow)
				{
					auto zFar = render_data_buffer.beg->shadow_distance;
					auto tan_hf_fovy = tan(render_data_buffer.beg->fovy * 0.5f * ANG_RAD);
					auto aspect = render_data_buffer.beg->aspect;
					auto view_inv = render_data_buffer.beg->view_inv;

					auto level = render_data_buffer.beg->csm_levels;
					for (auto j = 0; j < level; j++)
					{
						auto n = j / (float)level;
						n = n * n * zFar;
						auto f = (j + 1) / (float)level;
						f = f * f * zFar;

						Vec3f ps[8];
						get_frustum_points(n, f, tan_hf_fovy, aspect, view_inv, ps);

						auto c = (ps[0] + ps[1] + ps[2] + ps[3] +
							ps[4] + ps[5] + ps[6] + ps[7]) * 0.125f;
						auto light_inv = inverse(Mat4f(Mat<3, 4, float>(Mat3f(side, up, dir), Vec3f(0.f)), Vec4f(c, 1.f)));
						Vec2f LT = Vec2f(zFar);
						Vec2f RB = Vec2f(-zFar);
						for (auto k = 0; k < 8; k++)
						{
							auto p = light_inv * Vec4f(ps[k], 1.f);
							LT = min(LT, p.xy());
							RB = max(RB, p.xy());
						}

						li.shadow_matrices[j] = make_ortho_project_matrix(LT.x(), RB.x(), LT.y(), RB.y(), li.distance) *
							make_view_matrix(c + li.dir * li.distance * 0.5f, c, up);
					}
				}
				directional_light_info_buffer.push(li);

				{
					// TODO
					auto& lis = *light_indices_buffer.beg;
					lis.directional_lights_count++;
				}
			}
			else
			{
				PointLightInfoS li;
				li.color = color;
				li.distance = 10.f;
				li.coord = Vec3f(transform[3]);
				li.shadow_map_index = cast_shadow ? used_point_light_shadow_maps_count++ : -1;
				point_light_info_buffer.push(li);

				{
					// TODO
					auto& lis = *light_indices_buffer.beg;
					lis.point_light_indices[lis.point_lights_count++] = point_light_info_buffer.stg_num() - 1;
				}
			}
		}

		void CanvasPrivate::draw_lines(uint lines_count, const Line3* lines)
		{
			if (cmds.empty() || cmds.back()->type != Cmd::DrawLine3)
			{
				last_line3_cmd = new CmdDrawLine3;
				cmds.emplace_back(last_line3_cmd);
			}

			line3_buffer.push(lines_count, lines);

			last_line3_cmd->count = lines_count;
		}

		void CanvasPrivate::set_scissor(const Vec4f& _scissor)
		{
			auto scissor = Vec4f(
				max(_scissor.x(), 0.f),
				max(_scissor.y(), 0.f),
				min(_scissor.z(), (float)output_size.x()),
				min(_scissor.w(), (float)output_size.y()));
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
				min(_range.z(), (float)output_size.x()),
				min(_range.w(), (float)output_size.y()));
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
			terrain_info_buffer.stg_rewind();
			light_indices_buffer.stg_rewind();
			{
				// TODO
				LightIndicesS lis;
				lis.directional_lights_count = 0;
				lis.point_lights_count = 0;
				light_indices_buffer.push(lis);
			}
			directional_light_info_buffer.stg_rewind();
			point_light_info_buffer.stg_rewind();

			used_directional_light_shadow_maps_count = 0;
			used_point_light_shadow_maps_count = 0;

			line3_buffer.stg_rewind();

			curr_scissor = Vec4f(Vec2f(0.f), Vec2f(output_size));

			cmds.clear();
		}

		void CanvasPrivate::record(CommandBufferPrivate* cb, uint image_index)
		{
			cb->begin();

			enum PassType
			{
				PassNone = -1,
				Pass2D,
				Pass3D,
				PassLine3,
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
					if (passes.empty() || (passes.back().type != Pass2D && passes.back().type != PassNone))
						passes.emplace_back();
					passes.back().type = Pass2D;
					passes.back().cmd_ids.push_back(i);

				}
					break;
				case Cmd::DrawMesh:
				{
					auto cmd = (CmdDrawMesh*)cmds[i].get();
					for (auto& m : cmd->meshes)
					{
						auto deformer = std::get<3>(m);
						if (deformer)
						{
							if (deformer->dirty_range[1] > deformer->dirty_range[0])
							{
								deformer->poses_buffer.upload(cb, deformer->dirty_range[0], deformer->dirty_range[1] - deformer->dirty_range[0]);
								deformer->poses_buffer.barrier(cb);
							}
						}
					}
				}
				case Cmd::DrawTerrain:
				{
					if (passes.empty() || (passes.back().type != Pass3D && passes.back().type != PassNone))
						passes.emplace_back();
					passes.back().type = Pass3D;
					passes.back().cmd_ids.push_back(i);

				}
					break;
				case Cmd::DrawLine3:
				{
					if (passes.empty() || (passes.back().type != PassLine3 && passes.back().type != PassNone))
						passes.emplace_back();
					passes.back().type = PassLine3;
					passes.back().cmd_ids.push_back(i);
				}
					break;
				case Cmd::SetScissor:
				{
					if (passes.empty() || (passes.back().type != Pass2D && passes.back().type != Pass3D && passes.back().type != PassLine3 && passes.back().type != PassNone))
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

			auto dst = hdr_image ? hdr_image.get() : output_imageviews[image_index]->image;
			auto dst_fb = hdr_framebuffer ? hdr_framebuffer.get() : output_framebuffers[image_index].get();
			auto dst_ds = hdr_descriptorset ? hdr_descriptorset.get() : output_descriptorsets[image_index].get();

			cb->image_barrier(dst, {}, hdr_image ? ImageLayoutShaderReadOnly : ImageLayoutPresent, ImageLayoutTransferDst);
			cb->clear_color_image(dst, clear_color);
			cb->image_barrier(dst, {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);

			if (!msaa_image)
			{
				cb->image_barrier(depth_image.get(), {}, ImageLayoutAttachment, ImageLayoutTransferDst);
				cb->clear_depth_image(depth_image.get(), 1.f);
				cb->image_barrier(depth_image.get(), {}, ImageLayoutTransferDst, ImageLayoutAttachment);
			}

			cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)output_size));
			cb->set_scissor(Vec4f(Vec2f(0.f), (Vec2f)output_size));
			auto ele_vtx_off = 0;
			auto ele_idx_off = 0;
			auto first_mesh = true;
			auto line3_off = 0;

			for (auto& p : passes)
			{
				switch (p.type)
				{
				case Pass2D:
				{
					if (ele_idx_off == 0)
					{
						element_vertex_buffer.upload(cb);
						element_vertex_buffer.barrier(cb);
						element_index_buffer.upload(cb);
						element_index_buffer.barrier(cb);

					}
					cb->set_scissor(Vec4f(Vec2f(0.f), (Vec2f)output_size));
					cb->bind_vertex_buffer(element_vertex_buffer.buf.get(), 0);
					cb->bind_index_buffer(element_index_buffer.buf.get(), IndiceTypeUint);
					cb->begin_renderpass(nullptr, dst_fb);
					cb->bind_descriptor_set(PipelineGraphics, element_descriptorset.get(), 0, preferences->element_pipelinelayout.get());
					cb->push_constant_t(0, Vec2f(2.f / output_size.x(), 2.f / output_size.y()), preferences->element_pipelinelayout.get());
					cb->bind_pipeline(preferences->element_pipeline.get());
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
				case Pass3D:
				{
					if (first_mesh)
					{
						first_mesh = false;

						render_data_buffer.beg->fb_size = output_size;
						render_data_buffer.beg->shadow_distance = 100.f;
						render_data_buffer.beg->csm_levels = 3;

						mesh_matrix_buffer.upload(cb);
						mesh_matrix_buffer.barrier(cb);
						terrain_info_buffer.upload(cb);
						terrain_info_buffer.barrier(cb);
						render_data_buffer.upload(cb, 0, 1);
						render_data_buffer.barrier(cb);

						cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)shadow_map_size));
						cb->set_scissor(Vec4f(Vec2f(0.f), (Vec2f)shadow_map_size));

						light_indices_buffer.upload(cb);
						directional_light_info_buffer.upload(cb);
						point_light_info_buffer.upload(cb);
						light_indices_buffer.barrier(cb);
						point_light_info_buffer.barrier(cb);
						directional_light_info_buffer.barrier(cb);

						if (used_directional_light_shadow_maps_count > 0)
						{
							auto lights_count = directional_light_info_buffer.stg_num();
							for (auto i = 0; i < lights_count; i++)
							{
								auto& li = directional_light_info_buffer.beg[i];
								if (li.shadow_map_index != -1)
								{
									auto level = render_data_buffer.beg->csm_levels;
									for (auto i = 0; i < level; i++)
									{
										Vec4f cvs[] = {
											Vec4f(1.f, 0.f, 0.f, 0.f),
											Vec4f(1.f, 0.f, 0.f, 0.f)
										};
										cb->begin_renderpass(nullptr, directional_light_shadow_map_depth_framebuffers[li.shadow_map_index * 4 + i].get(), cvs);
										cb->bind_descriptor_set(PipelineGraphics, mesh_descriptorset.get(), 0, preferences->depth_pipelinelayout.get());
										cb->bind_descriptor_set(PipelineGraphics, material_descriptorset.get(), 1, preferences->depth_pipelinelayout.get());
										struct
										{
											Mat4f proj_view;
											float zNear;
											float zFar;
										}pc;
										pc.proj_view = li.shadow_matrices[i];
										pc.zNear = 0.f;
										pc.zFar = 100.f;
										cb->push_constant(0, sizeof(pc), &pc, preferences->depth_pipelinelayout.get());
										for (auto& cmd : cmds)
										{
											if (cmd->type == Cmd::DrawMesh)
											{
												auto c = (CmdDrawMesh*)cmd.get();
												for (auto& m : c->meshes)
												{
													if (std::get<2>(m))
													{
														auto mrm = std::get<1>(m);
														cb->bind_vertex_buffer(mrm->vertex_buffer.buf.get(), 0);
														cb->bind_index_buffer(mrm->index_buffer.buf.get(), IndiceTypeUint);
														if (std::get<3>(m))
														{
															cb->bind_pipeline(preferences->depth_armature_pipeline.get());
															cb->bind_vertex_buffer(mrm->weight_buffer.buf.get(), 1);
															cb->bind_descriptor_set(PipelineGraphics, std::get<3>(m)->descriptorset.get(), 2, preferences->depth_pipelinelayout.get());
														}
														else
															cb->bind_pipeline(preferences->depth_pipeline.get());
														cb->draw_indexed(mrm->index_buffer.count, 0, 0, 1, (std::get<0>(m) << 16) + mrm->material_index);
													}
												}
											}
										}
										cb->end_renderpass();

										cb->image_barrier(directional_light_shadow_maps[li.shadow_map_index].get(), { 0U, 1U, (uint)i, 1U }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
										cb->begin_renderpass(nullptr, shadow_blur_pingpong_image_framebuffer.get());
										cb->bind_descriptor_set(PipelineGraphics, directional_light_shadow_map_descriptorsets[li.shadow_map_index * 4 + i].get(), 0, preferences->sampler1_pc4_pipelinelayout.get());
										cb->push_constant_t(0, 1.f / shadow_map_size.x(), preferences->sampler1_pc4_pipelinelayout.get());
										cb->bind_pipeline(preferences->blurh_depth_pipeline.get());
										cb->draw(3, 1, 0, 0);
										cb->end_renderpass();

										cb->image_barrier(shadow_blur_pingpong_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
										cb->begin_renderpass(nullptr, directional_light_shadow_map_framebuffers[li.shadow_map_index * 4 + i].get());
										cb->bind_descriptor_set(PipelineGraphics, shadow_blur_pingpong_image_descriptorset.get(), 0, preferences->sampler1_pc4_pipelinelayout.get());
										cb->push_constant_t(0, 1.f / shadow_map_size.y(), preferences->sampler1_pc4_pipelinelayout.get());
										cb->bind_pipeline(preferences->blurv_depth_pipeline.get());
										cb->draw(3, 1, 0, 0);
										cb->end_renderpass();
									}

									cb->image_barrier(directional_light_shadow_maps[li.shadow_map_index].get(), { 0U, 1U, 0U, 4U }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
								}
							}
						}
						if (used_point_light_shadow_maps_count > 0)
						{
							auto lights_count = point_light_info_buffer.stg_num();
							for (auto i = 0; i < lights_count; i++)
							{
								auto& li = point_light_info_buffer.beg[i];
								if (li.shadow_map_index != -1)
								{
									for (auto j = 0; j < 6; j++)
									{
										Vec4f cvs[] = {
											Vec4f(1.f, 0.f, 0.f, 0.f),
											Vec4f(1.f, 0.f, 0.f, 0.f)
										};
										cb->begin_renderpass(nullptr, point_light_shadow_map_depth_framebuffers[li.shadow_map_index * 6 + j].get(), cvs);
										cb->bind_descriptor_set(PipelineGraphics, mesh_descriptorset.get(), 0, preferences->depth_pipelinelayout.get());
										cb->bind_descriptor_set(PipelineGraphics, material_descriptorset.get(), 1, preferences->depth_pipelinelayout.get());
										auto proj = make_perspective_project_matrix(90.f, 1.f, 1.f, li.distance);
										struct
										{
											Mat4f matrix;
											float zNear;
											float zFar;
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
										pc.zNear = 1.f;
										pc.zFar = li.distance;
										cb->push_constant(0, sizeof(pc), &pc, preferences->depth_pipelinelayout.get());
										for (auto& cmd : cmds)
										{
											if (cmd->type == Cmd::DrawMesh)
											{
												auto c = (CmdDrawMesh*)cmd.get();
												for (auto& m : c->meshes)
												{
													if (std::get<2>(m))
													{
														auto mrm = std::get<1>(m);
														cb->bind_vertex_buffer(mrm->vertex_buffer.buf.get(), 0);
														cb->bind_index_buffer(mrm->index_buffer.buf.get(), IndiceTypeUint);
														if (std::get<3>(m))
														{
															cb->bind_pipeline(preferences->depth_armature_pipeline.get());
															cb->bind_vertex_buffer(mrm->weight_buffer.buf.get(), 1);
															cb->bind_descriptor_set(PipelineGraphics, std::get<3>(m)->descriptorset.get(), 2, preferences->depth_pipelinelayout.get());
														}
														else
															cb->bind_pipeline(preferences->depth_pipeline.get());
														cb->draw_indexed(mrm->index_buffer.count, 0, 0, 1, (std::get<0>(m) << 16) + mrm->material_index);
													}
												}
											}
										}
										cb->end_renderpass();

										cb->image_barrier(point_light_shadow_maps[li.shadow_map_index].get(), { 0U, 1U, (uint)j, 1U }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
										cb->begin_renderpass(nullptr, shadow_blur_pingpong_image_framebuffer.get());
										cb->bind_descriptor_set(PipelineGraphics, point_light_shadow_map_descriptorsets[li.shadow_map_index * 6 + j].get(), 0, preferences->sampler1_pc4_pipelinelayout.get());
										cb->push_constant_t(0, 1.f / shadow_map_size.x(), preferences->sampler1_pc4_pipelinelayout.get());
										cb->bind_pipeline(preferences->blurh_depth_pipeline.get());
										cb->draw(3, 1, 0, 0);
										cb->end_renderpass();

										cb->image_barrier(shadow_blur_pingpong_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
										cb->begin_renderpass(nullptr, point_light_shadow_map_framebuffers[li.shadow_map_index * 6 + j].get());
										cb->bind_descriptor_set(PipelineGraphics, shadow_blur_pingpong_image_descriptorset.get(), 0, preferences->sampler1_pc4_pipelinelayout.get());
										cb->push_constant_t(0, 1.f / shadow_map_size.y(), preferences->sampler1_pc4_pipelinelayout.get());
										cb->bind_pipeline(preferences->blurv_depth_pipeline.get());
										cb->draw(3, 1, 0, 0);
										cb->end_renderpass();
									}

									cb->image_barrier(point_light_shadow_maps[li.shadow_map_index].get(), { 0U, 1U, 0U, 6U }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
								}
							}
						}

						cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)output_size));
					}

					cb->set_scissor(Vec4f(Vec2f(0.f), (Vec2f)output_size));
					if (!msaa_image)
						cb->begin_renderpass(nullptr, mesh_framebuffers[hdr_image ? 0 : image_index].get());
					else
					{
						Vec4f cvs[3];
						cvs[0] = Vec4f(0.f, 0.f, 0.f, 0.f);
						cvs[1] = Vec4f(1.f, 0.f, 0.f, 0.f);
						cvs[2] = Vec4f(0.f, 0.f, 0.f, 0.f);
						cb->begin_renderpass(nullptr, mesh_framebuffers[0].get(), cvs);
					}
					for (auto& i : p.cmd_ids)
					{
						auto& cmd = cmds[i];
						switch (cmd->type)
						{
						case Cmd::DrawMesh:
						{
							auto c = (CmdDrawMesh*)cmd.get();
							cb->bind_descriptor_set(PipelineGraphics, render_data_descriptorset.get(), 0, preferences->mesh_pipelinelayout.get());
							cb->bind_descriptor_set(PipelineGraphics, mesh_descriptorset.get(), 1, preferences->mesh_pipelinelayout.get());
							cb->bind_descriptor_set(PipelineGraphics, material_descriptorset.get(), 2, preferences->mesh_pipelinelayout.get());
							cb->bind_descriptor_set(PipelineGraphics, light_descriptorset.get(), 3, preferences->mesh_pipelinelayout.get());
							for (auto& m : c->meshes)
							{
								auto mrm = std::get<1>(m);
								cb->bind_vertex_buffer(mrm->vertex_buffer.buf.get(), 0);
								cb->bind_index_buffer(mrm->index_buffer.buf.get(), IndiceTypeUint);
								if (std::get<3>(m))
								{
									cb->bind_pipeline(preferences->mesh_armature_pipeline.get());
									cb->bind_vertex_buffer(mrm->weight_buffer.buf.get(), 1);
									cb->bind_descriptor_set(PipelineGraphics, std::get<3>(m)->descriptorset.get(), 4, preferences->mesh_pipelinelayout.get());
								}
								else
									cb->bind_pipeline(preferences->mesh_pipeline.get());
								cb->draw_indexed(mrm->index_buffer.count, 0, 0, 1, (std::get<0>(m) << 16) + mrm->material_index);
							}
						}
							break;
						case Cmd::DrawTerrain:
						{
							auto c = (CmdDrawTerrain*)cmd.get();
							cb->bind_descriptor_set(PipelineGraphics, render_data_descriptorset.get(), 0, preferences->terrain_pipelinelayout.get());
							cb->bind_descriptor_set(PipelineGraphics, material_descriptorset.get(), 1, preferences->terrain_pipelinelayout.get());
							cb->bind_descriptor_set(PipelineGraphics, light_descriptorset.get(), 2, preferences->terrain_pipelinelayout.get());
							cb->bind_descriptor_set(PipelineGraphics, terrain_descriptorset.get(), 3, preferences->terrain_pipelinelayout.get());
							cb->bind_pipeline(preferences->terrain_pipeline.get());
							cb->draw(4, terrain_info_buffer.beg[c->idx].blocks.mul(), 0, c->idx << 16);
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
					if (msaa_image)
					{
						cb->image_barrier(msaa_resolve_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
						cb->set_scissor(Vec4f(Vec2f(0.f), (Vec2f)output_size));
						cb->begin_renderpass(nullptr, dst_fb);
						cb->bind_descriptor_set(PipelineGraphics, msaa_descriptorset.get(), 0, preferences->sampler1_pc0_pipelinelayout.get());
						cb->bind_pipeline(hdr_image ? preferences->blit_16_pipeline.get() : preferences->blit_8_pipeline.get());
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();
					}
				}
					break;
				case PassLine3:
				{
					if (line3_off == 0)
					{
						line3_buffer.upload(cb);
						line3_buffer.barrier(cb);
					}
					cb->set_scissor(Vec4f(Vec2f(0.f), (Vec2f)output_size));
					cb->begin_renderpass(nullptr, dst_fb);
					cb->bind_vertex_buffer(line3_buffer.buf.get(), 0);
					cb->push_constant(0, sizeof(Mat4f), &render_data_buffer.beg->proj_view, preferences->line_pipelinelayout.get());
					cb->bind_pipeline(preferences->line3_pipeline.get());
					for (auto& i : p.cmd_ids)
					{
						auto& cmd = cmds[i];
						switch (cmd->type)
						{
						case Cmd::DrawLine3:
						{
							auto c = (CmdDrawLine3*)cmd.get();
							cb->draw(c->count * 2, 1, line3_off, 0);
							line3_off += c->count;
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
					cb->begin_renderpass(nullptr, back_framebuffers[0].get());
					cb->bind_descriptor_set(PipelineGraphics, dst_ds, 0, preferences->sampler1_pc4_pipelinelayout.get());
					cb->push_constant_t(0, 1.f / output_size.x(), preferences->sampler1_pc4_pipelinelayout.get());
					cb->bind_pipeline(preferences->blurh_pipeline[blur_radius - 1].get());
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->image_barrier(back_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
					cb->set_scissor(blur_range);
					cb->begin_renderpass(nullptr, dst_fb);
					cb->bind_descriptor_set(PipelineGraphics, back_nearest_descriptorsets[0].get(), 0, preferences->sampler1_pc4_pipelinelayout.get());
					cb->push_constant_t(0, 1.f / output_size.y(), preferences->sampler1_pc4_pipelinelayout.get());
					cb->bind_pipeline(preferences->blurv_pipeline[blur_radius - 1].get());
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();
				}
					break;
				case PassBloom:
				{
					if (!hdr_image)
						continue;

					cb->set_scissor(Vec4f(Vec2f(0.f), (Vec2f)output_size));

					cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)output_size));
					cb->image_barrier(hdr_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
					cb->begin_renderpass(nullptr, back_framebuffers[0].get());
					cb->bind_descriptor_set(PipelineGraphics, hdr_descriptorset.get(), 0, preferences->sampler1_pc0_pipelinelayout.get());
					cb->bind_pipeline(preferences->filter_bright_pipeline.get());
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					for (auto i = 0; i < back_image->level - 1; i++)
					{
						cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)back_image->sizes[i + 1]));
						cb->image_barrier(back_image.get(), { (uint)i }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
						cb->begin_renderpass(nullptr, back_framebuffers[i + 1].get());
						cb->bind_descriptor_set(PipelineGraphics, back_linear_descriptorsets[i].get(), 0, preferences->sampler1_pc8_pipelinelayout.get());
						cb->push_constant_t(0, 1.f / (Vec2f)back_image->sizes[i], preferences->sampler1_pc8_pipelinelayout.get());
						cb->bind_pipeline(preferences->downsample_pipeline.get());
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();
					}
					for (auto i = back_image->level - 1; i > 0; i--)
					{
						cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)back_image->sizes[i - 1]));
						cb->image_barrier(back_image.get(), { (uint)i }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
						cb->begin_renderpass(nullptr, back_framebuffers[i - 1].get());
						cb->bind_descriptor_set(PipelineGraphics, back_linear_descriptorsets[i].get(), 0, preferences->sampler1_pc8_pipelinelayout.get());
						cb->push_constant_t(0, 1.f / (Vec2f)back_image->sizes[(uint)i - 1], preferences->sampler1_pc8_pipelinelayout.get());
						cb->bind_pipeline(preferences->upsample_pipeline.get());
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();
					}
					cb->image_barrier(back_image.get(), { 1U }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
					cb->begin_renderpass(nullptr, hdr_framebuffer.get());
					cb->bind_descriptor_set(PipelineGraphics, back_linear_descriptorsets[1].get(), 0, preferences->sampler1_pc8_pipelinelayout.get());
					cb->push_constant_t(0, 1.f / output_size.y(), preferences->sampler1_pc8_pipelinelayout.get());
					cb->bind_pipeline(preferences->upsample_pipeline.get());
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->set_viewport(Vec4f(Vec2f(0.f), (Vec2f)output_size));
				}
					break;
				}
			}

			if (hdr_image)
			{
				cb->image_barrier(hdr_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
				cb->image_barrier(output_imageviews[image_index]->image, {}, ImageLayoutPresent, ImageLayoutShaderReadOnly);
				cb->set_scissor(Vec4f(Vec2f(0.f), Vec2f(output_size)));
				cb->begin_renderpass(nullptr, output_framebuffers[image_index].get());
				cb->bind_descriptor_set(PipelineGraphics, hdr_descriptorset.get(), 0, preferences->sampler1_pc0_pipelinelayout.get());
				cb->bind_pipeline(preferences->gamma_pipeline.get());
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();
			}
			cb->image_barrier(output_imageviews[image_index]->image, {}, ImageLayoutShaderReadOnly, ImageLayoutPresent);

			cb->end();

			cmds.clear();
		}

		Canvas* Canvas::create(RenderPreferences* preferences)
		{
			return new CanvasPrivate((RenderPreferencesPrivate*)preferences);
		}
	}
}
