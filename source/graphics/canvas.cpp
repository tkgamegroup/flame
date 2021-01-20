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
		RenderPreferencesPrivate::RenderPreferencesPrivate(DevicePrivate* device, bool hdr) :
			device(device),
			hdr(hdr)
		{
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

			mesh_wireframe_pipeline.reset(create_material_pipeline(MaterialForMesh, L"", "WIREFRAME"));
			mesh_armature_wireframe_pipeline.reset(create_material_pipeline(MaterialForMeshArmature, L"", "WIREFRAME"));
			terrain_wireframe_pipeline.reset(create_material_pipeline(MaterialForTerrain, L"", "WIREFRAME"));

			{
				ShaderPrivate* shaders[] = {
					ShaderPrivate::get(device, L"element/element.vert"),
					ShaderPrivate::get(device, L"element/element.frag")
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
				element_pipeline.reset(PipelinePrivate::create(device, shaders, PipelineLayoutPrivate::get(device, L"element/element.pll"), hdr ? image1_16_renderpass.get() : image1_8_renderpass.get(), 0, & vi, nullptr, nullptr, { &bo, 1 }));
			}

			{
				ShaderPrivate* shaders[] = {
					ShaderPrivate::get(device, L"sky/sky.vert"),
					ShaderPrivate::get(device, L"sky/sky.frag")
				};
				VertexAttributeInfo vias[2];
				vias[0].location = 0;
				vias[0].format = Format_R32G32B32_SFLOAT;
				vias[1].location = 2;
				vias[1].offset = 5 * sizeof(float);
				vias[1].format = Format_R32G32B32_SFLOAT;
				VertexBufferInfo vib;
				vib.attributes_count = size(vias);
				vib.attributes = vias;
				VertexInfo vi;
				vi.buffers_count = 1;
				vi.buffers = &vib;
				RasterInfo rst;
				rst.depth_clamp = true;
				rst.cull_mode = CullModeFront;
				DepthInfo dep;
				dep.test = false;
				dep.write = false;
				sky_pipeline.reset(PipelinePrivate::create(device, shaders, PipelineLayoutPrivate::get(device, L"sky/sky.pll"), mesh_renderpass.get(), 0, &vi, &rst, &dep));
			}

			{
				ShaderPrivate* shaders[] = {
					ShaderPrivate::get(device, L"line3.vert"),
					ShaderPrivate::get(device, L"line3.frag")
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
				line3_pipeline.reset(PipelinePrivate::create(device, shaders, PipelineLayoutPrivate::get(device, L"simple3D.pll"), hdr ? image1_16_renderpass.get() : image1_8_renderpass.get(), 0, &vi));
			}

			auto post_pll = PipelineLayoutPrivate::get(device, L"post/post.pll");
			auto fullscreen_vert = ShaderPrivate::get(device, L"fullscreen.vert");

			for (auto i = 0; i < 10; i++)
			{
				{
					ShaderPrivate* shaders[] = {
						fullscreen_vert,
						ShaderPrivate::get(device, L"post/blur.frag", "R" + std::to_string(i + 1) + " H")
					};
					blurh_pipeline[i].reset(PipelinePrivate::create(device, shaders, post_pll, hdr ? image1_16_renderpass.get() : image1_8_renderpass.get(), 0));
				}

				{
					ShaderPrivate* shaders[] = {
						fullscreen_vert,
						ShaderPrivate::get(device, L"post/blur.frag", "R" + std::to_string(i + 1) + " V")
					};
					blurv_pipeline[i].reset(PipelinePrivate::create(device, shaders, post_pll, hdr ? image1_16_renderpass.get() : image1_8_renderpass.get(), 0));
				}
			}

			{
				ShaderPrivate* shaders[] = {
					fullscreen_vert,
					ShaderPrivate::get(device, L"post/blur_depth.frag", "H")
				};
				blurh_depth_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, image1_r16_renderpass.get(), 0));
			}

			{
				ShaderPrivate* shaders[] = {
					fullscreen_vert,
					ShaderPrivate::get(device, L"post/blur_depth.frag", "V")
				};
				blurv_depth_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, image1_r16_renderpass.get(), 0));
			}

			{
				ShaderPrivate* shaders[] = {
					fullscreen_vert,
					ShaderPrivate::get(device, L"post/blit.frag")
				};
				blit_8_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, image1_8_renderpass.get(), 0));
				blit_16_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, image1_16_renderpass.get(), 0));
			}

			{
				ShaderPrivate* shaders[] = {
					fullscreen_vert,
					ShaderPrivate::get(device, L"post/filter_bright.frag")
				};
				filter_bright_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, image1_16_renderpass.get(), 0));
			}

			auto box_frag = ShaderPrivate::get(device, L"post/box.frag");

			{
				ShaderPrivate* shaders[] = {
					fullscreen_vert,
					box_frag
				};
				downsample_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, image1_16_renderpass.get(), 0));
			}

			{
				ShaderPrivate* shaders[] = {
					fullscreen_vert,
					box_frag
				};
				BlendOption bo;
				bo.enable = true;
				bo.src_color = BlendFactorOne;
				bo.dst_color = BlendFactorOne;
				bo.src_alpha = BlendFactorZero;
				bo.dst_alpha = BlendFactorOne;
				upsample_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, image1_16_renderpass.get(), 0, nullptr, nullptr, nullptr, { &bo, 1 }));
			}

			{
				ShaderPrivate* shaders[] = {
					fullscreen_vert,
					ShaderPrivate::get(device, L"post/gamma.frag")
				};
				gamma_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, image1_8_renderpass.get(), 0));
			}
		}

		PipelinePrivate* RenderPreferencesPrivate::get_material_pipeline(MaterialUsage usage, const std::filesystem::path& mat, const std::string& defines)
		{
			for (auto& p : material_pipelines[usage])
			{
				if (std::get<0>(p) == mat && std::get<1>(p) == defines)
				{
					std::get<2>(p)++;
					return std::get<3>(p).get();
				}
			}
			auto ret = create_material_pipeline(usage, mat, defines);
			material_pipelines[MaterialForMesh].emplace_back(mat, defines, 1, ret);
			return ret;
		}

		PipelinePrivate* RenderPreferencesPrivate::create_material_pipeline(MaterialUsage usage, const std::filesystem::path& mat, const std::string& _defines)
		{
			auto defines = _defines;
			auto substitutes = std::string();
			PolygonMode polygon_mode = PolygonModeFill;
			std::vector<std::filesystem::path> extra_dependencies;
			std::filesystem::file_time_type lwt = {};
			if (defines != "WIREFRAME")
			{
				defines += "MAT ";
				substitutes += "MAT_FILE " + mat.string();
				extra_dependencies.push_back(mat);
			}
			else
				polygon_mode = PolygonModeLine;
			switch (usage)
			{
			case MaterialForMesh:
			{
				ShaderPrivate* shaders[] = {
					ShaderPrivate::get(device, L"mesh/mesh.vert"),
					ShaderPrivate::get(device, L"mesh/mesh.frag", defines, substitutes, extra_dependencies)
				};
				VertexAttributeInfo vias[3];
				vias[0].location = 0;
				vias[0].format = Format_R32G32B32_SFLOAT;
				vias[1].location = 1;
				vias[1].format = Format_R32G32_SFLOAT;
				vias[2].location = 2;
				vias[2].format = Format_R32G32B32_SFLOAT;
				VertexBufferInfo vib;
				vib.attributes_count = size(vias);
				vib.attributes = vias;
				VertexInfo vi;
				vi.buffers_count = 1;
				vi.buffers = &vib;
				RasterInfo rst;
				rst.polygon_mode = polygon_mode;
				DepthInfo dep;
				return PipelinePrivate::create(device, shaders, PipelineLayoutPrivate::get(device, L"mesh/mesh.pll"), mesh_renderpass.get(), 0, &vi, &rst, &dep);
			}
				break;
			case MaterialForMeshArmature:
			{
				ShaderPrivate* shaders[] = {
					ShaderPrivate::get(device, L"mesh/mesh.vert", "ARMATURE"),
					ShaderPrivate::get(device, L"mesh/mesh.frag", defines, substitutes, extra_dependencies)
				};
				VertexAttributeInfo vias1[3];
				vias1[0].location = 0;
				vias1[0].format = Format_R32G32B32_SFLOAT;
				vias1[1].location = 1;
				vias1[1].format = Format_R32G32_SFLOAT;
				vias1[2].location = 2;
				vias1[2].format = Format_R32G32B32_SFLOAT;
				VertexAttributeInfo vias2[2];
				vias2[0].location = 5;
				vias2[0].format = Format_R32G32B32A32_INT;
				vias2[1].location = 6;
				vias2[1].format = Format_R32G32B32A32_SFLOAT;
				VertexBufferInfo vibs[2];
				vibs[0].attributes_count = size(vias1);
				vibs[0].attributes = vias1;
				vibs[1].attributes_count = size(vias2);
				vibs[1].attributes = vias2;
				VertexInfo vi;
				vi.buffers_count = 2;
				vi.buffers = vibs;
				RasterInfo rst;
				rst.polygon_mode = polygon_mode;
				DepthInfo dep;
				return PipelinePrivate::create(device, shaders, PipelineLayoutPrivate::get(device, L"mesh/mesh.pll"), mesh_renderpass.get(), 0, &vi, &rst, &dep);
			}
				break;
			case MaterialForTerrain:
			{
				ShaderPrivate* shaders[] = {
					ShaderPrivate::get(device, L"terrain/terrain.vert"),
					ShaderPrivate::get(device, L"terrain/terrain.tesc"),
					ShaderPrivate::get(device, L"terrain/terrain.tese"),
					ShaderPrivate::get(device, L"terrain/terrain.frag", defines, substitutes, extra_dependencies)
				};
				VertexInfo vi;
				vi.primitive_topology = PrimitiveTopologyPatchList;
				vi.patch_control_points = 4;
				RasterInfo rst;
				rst.polygon_mode = polygon_mode;
				DepthInfo dep;
				return PipelinePrivate::create(device, shaders, PipelineLayoutPrivate::get(device, L"terrain/terrain.pll"), mesh_renderpass.get(), 0, &vi, &rst, &dep);
			}
				break;
			case MaterialForDepth:
			{
				defines += "DEPTH_PASS ";
				ShaderPrivate* shaders[] = {
					ShaderPrivate::get(device, L"depth/depth.vert"),
					ShaderPrivate::get(device, L"depth/depth.frag", defines, substitutes, extra_dependencies)
				};
				VertexAttributeInfo vias[2];
				vias[0].location = 0;
				vias[0].format = Format_R32G32B32_SFLOAT;
				vias[1].location = 1;
				vias[1].format = Format_R32G32_SFLOAT;
				VertexBufferInfo vib;
				vib.attributes_count = size(vias);
				vib.attributes = vias;
				vib.stride = sizeof(vec3) + sizeof(vec2) + sizeof(vec3);
				VertexInfo vi;
				vi.buffers_count = 1;
				vi.buffers = &vib;
				RasterInfo rst;
				DepthInfo dep;
				return PipelinePrivate::create(device, shaders, PipelineLayoutPrivate::get(device, L"depth/depth.pll"), depth_renderpass.get(), 0, &vi, &rst, &dep);
			}
				break;
			case MaterialForDepthArmature:
			{
				defines += "DEPTH_PASS ";
				ShaderPrivate* shaders[] = {
					ShaderPrivate::get(device, L"depth/depth.vert", "ARMATURE"),
					ShaderPrivate::get(device, L"depth/depth.frag", defines, substitutes, extra_dependencies)
				};
				VertexAttributeInfo vias1[2];
				vias1[0].location = 0;
				vias1[0].format = Format_R32G32B32_SFLOAT;
				vias1[1].location = 1;
				vias1[1].format = Format_R32G32_SFLOAT;
				VertexAttributeInfo vias2[2];
				vias2[0].location = 2;
				vias2[0].format = Format_R32G32B32A32_INT;
				vias2[1].location = 3;
				vias2[1].format = Format_R32G32B32A32_SFLOAT;
				VertexBufferInfo vibs[2];
				vibs[0].attributes_count = size(vias1);
				vibs[0].attributes = vias1;
				vibs[0].stride = sizeof(vec3) + sizeof(vec2) + sizeof(vec3);
				vibs[1].attributes_count = size(vias2);
				vibs[1].attributes = vias2;
				VertexInfo vi;
				vi.buffers_count = 2;
				vi.buffers = vibs;
				RasterInfo rst;
				DepthInfo dep;
				return PipelinePrivate::create(device, shaders, PipelineLayoutPrivate::get(device, L"depth/depth.pll"), depth_renderpass.get(), 0, &vi, &rst, &dep);
			}
				break;
			}
			return nullptr;
		}

		void RenderPreferencesPrivate::release_material_pipeline(MaterialUsage usage, PipelinePrivate* p)
		{
			for (auto it = material_pipelines[usage].begin(); it != material_pipelines[usage].end(); it++)
			{
				if (std::get<3>(*it).get() == p)
				{
					for (auto s : p->shaders)
						s->release();
					if (std::get<2>(*it) == 1)
						material_pipelines[usage].erase(it);
					else
						std::get<2>(*it)--;
					break;
				}
			}
		}

		RenderPreferences* RenderPreferences::create(Device* device, bool hdr)
		{
			return new RenderPreferencesPrivate((DevicePrivate*)device, hdr);
		}

		ArmatureDeformerPrivate::ArmatureDeformerPrivate(RenderPreferencesPrivate* preferences, MeshPrivate* mesh) :
			mesh(mesh)
		{
			auto dsl = DescriptorSetLayoutPrivate::get(preferences->device, L"armature.dsl");
			poses_buffer.create(preferences->device, BufferUsageStorage, find_type(dsl->types, "mat4"), mesh->bones.size());
			descriptorset.reset(new DescriptorSetPrivate(preferences->device->dsp.get(), dsl));
			descriptorset->set_buffer(0, 0, poses_buffer.buf.get());
		}

		void ArmatureDeformerPrivate::set_pose(uint id, const mat4& pose)
		{
			auto dst = (mat4*)poses_buffer.mark_item(id);
			*dst = pose * mesh->bones[id]->offset_matrix;
		}

		ArmatureDeformer* ArmatureDeformer::create(RenderPreferences* preferences, Mesh* mesh)
		{
			return new ArmatureDeformerPrivate((RenderPreferencesPrivate*)preferences, (MeshPrivate*)mesh);
		}

		MaterialResourceSlot::~MaterialResourceSlot()
		{
			for (auto& t : textures)
				canvas->set_texture_resource(t.first, nullptr, nullptr, "");
			for (auto i = 0; i < MaterialUsageCount; i++)
			{
				if (pipelines[i])
					canvas->preferences->release_material_pipeline((MaterialUsage)i, pipelines[i]);
			}
		}

		PipelinePrivate* MaterialResourceSlot::get_pipeline(MaterialUsage u)
		{
			if (pipelines[u])
				return pipelines[u];
			auto path = material->dir / material->pipeline_file;
			if (!std::filesystem::exists(path))
			{
				path = material->pipeline_file;
				get_resource_path(path, L"assets\\shaders");
			}
			pipelines[u] = canvas->preferences->create_material_pipeline(u, path, material->pipeline_defines);
			return pipelines[u];
		}

		ModelResourceSlot::~ModelResourceSlot()
		{
			for (auto& m : materials)
				canvas->set_material_resource(m, nullptr, "");
		}

		CanvasPrivate::CanvasPrivate(RenderPreferencesPrivate* preferences) :
			preferences(preferences)
		{
			auto device = preferences->device;

			ImmediateCommandBuffer icb(device);
			auto cb = icb.cb.get();

			white_image.reset(new ImagePrivate(device, Format_R8G8B8A8_UNORM, uvec2(1), 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled));
			cb->image_barrier(white_image.get(), {}, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->clear_color_image(white_image.get(), cvec4(255));
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
			
			{
				auto dsl = DescriptorSetLayoutPrivate::get(device, L"render_data.dsl");
				render_data_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), dsl));
				render_data_buffer.create(device, BufferUsageUniform, find_type(dsl->types, "RenderData"));
				render_data_buffer.set(S<"shadow_distance"_h>, shadow_distance);
				render_data_buffer.set(S<"csm_levels"_h>, csm_levels);
				render_data_buffer.set(S<"csm_factor"_h>, csm_factor);
				render_data_buffer.set(S<"sky_rad_levels"_h>, 0.f);
				render_data_descriptorset->set_buffer(dsl->find_binding("RenderData"), 0, render_data_buffer.buf.get());
			}

			{
				auto dsl = DescriptorSetLayoutPrivate::get(device, L"sky.dsl");
				sky_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), dsl));
				auto sp = SamplerPrivate::get(preferences->device, FilterLinear, FilterLinear, AddressClampToEdge);
				default_sky_box_image.reset(new ImagePrivate(device, Format_R8G8B8A8_UNORM, uvec2(4), 1, 6, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled | ImageUsageAttachment, true));
				cb->image_barrier(default_sky_box_image.get(), { 0U, 1U, 0U, 6U }, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
				sky_descriptorset->set_image(dsl->find_binding("sky_box"), 0, default_sky_box_image->views.back().get(), sp);
				default_sky_irr_image.reset(new ImagePrivate(device, Format_R8G8B8A8_UNORM, uvec2(4), 1, 6, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled | ImageUsageAttachment, true));
				cb->image_barrier(default_sky_irr_image.get(), { 0U, 1U, 0U, 6U }, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
				sky_descriptorset->set_image(dsl->find_binding("sky_irr"), 0, default_sky_irr_image->views.back().get(), sp);
				default_sky_rad_image.reset(new ImagePrivate(device, Format_R8G8B8A8_UNORM, uvec2(4), 1, 6, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled | ImageUsageAttachment, true));
				cb->image_barrier(default_sky_rad_image.get(), { 0U, 1U, 0U, 6U }, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
				sky_descriptorset->set_image(dsl->find_binding("sky_rad"), 0, default_sky_rad_image->views.back().get(), sp);
			}

			{
				auto dsl = DescriptorSetLayoutPrivate::get(device, L"element.dsl");
				element_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), dsl));
				element_vertex_buffer.create(device, BufferUsageVertex, 360000);
				element_index_buffer.create(device, BufferUsageIndex, 240000);
				for (auto i = 0; i < element_resources.size(); i++)
					element_descriptorset->set_image(dsl->find_binding("images"), i, iv_wht, SamplerPrivate::get(device, FilterLinear, FilterLinear, AddressClampToEdge));
			}
			
			{
				auto dsl = DescriptorSetLayoutPrivate::get(device, L"mesh.dsl");
				mesh_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), dsl));
				mesh_matrix_buffer.create(device, BufferUsageStorage, find_type(dsl->types, "MeshMatrix"), 10000);
				mesh_descriptorset->set_buffer(dsl->find_binding("MeshMatrices"), 0, mesh_matrix_buffer.buf.get());
			}

			{
				auto dsl = DescriptorSetLayoutPrivate::get(device, L"material.dsl");
				material_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), dsl));
				material_info_buffer.create(device, BufferUsageStorage, find_type(dsl->types, "MaterialInfo"), 128);
				material_descriptorset->set_buffer(dsl->find_binding("MaterialInfos"), 0, material_info_buffer.buf.get());
				for (auto i = 0; i < texture_resources.size(); i++)
					material_descriptorset->set_image(dsl->find_binding("maps"), i, iv_wht, SamplerPrivate::get(device, FilterLinear, FilterLinear, AddressClampToEdge));
			}

			auto post_dsl = DescriptorSetLayoutPrivate::create(device, L"post/post.dsl");

			{
				auto dsl = DescriptorSetLayoutPrivate::get(device, L"light.dsl");
				light_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), dsl));

				shadow_depth_image.reset(new ImagePrivate(device, Format_Depth16, shadow_map_size, 1, 1, SampleCount_1, ImageUsageAttachment));
				cb->image_barrier(shadow_depth_image.get(), {}, ImageLayoutUndefined, ImageLayoutAttachment);

				shadow_blur_pingpong_image.reset(new ImagePrivate(device, Format_R16_SFLOAT, shadow_map_size, 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
				cb->image_barrier(shadow_blur_pingpong_image.get(), {}, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
				{
					auto iv = shadow_blur_pingpong_image->views[0].get();
					shadow_blur_pingpong_framebuffer.reset(new FramebufferPrivate(device, preferences->image1_r16_renderpass.get(), { &iv, 1 }));
					shadow_blur_pingpong_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), post_dsl));
					shadow_blur_pingpong_descriptorset->set_image(0, 0, iv, SamplerPrivate::get(device, FilterNearest, FilterNearest, AddressClampToEdge));
				}

				light_indices_buffer.create(device, BufferUsageStorage, find_type(dsl->types, "LightIndices"), 1);
				light_descriptorset->set_buffer(dsl->find_binding("LightIndicesList"), 0, light_indices_buffer.buf.get());

				auto sp1 = SamplerPrivate::get(device, FilterLinear, FilterLinear, AddressClampToEdge);
				auto sp2 = SamplerPrivate::get(device, FilterLinear, FilterLinear, AddressClampToBorder);

				directional_light_info_buffer.create(device, BufferUsageStorage, find_type(dsl->types, "DirectionalLightInfo"), 10);
				light_descriptorset->set_buffer(dsl->find_binding("DirectionalLightInfos"), 0, directional_light_info_buffer.buf.get());
				directional_shadow_maps.resize(4);
				directional_light_depth_framebuffers.resize(directional_shadow_maps.size() * 4);
				directional_shadow_map_framebuffers.resize(directional_shadow_maps.size() * 4);
				directional_shadow_map_descriptorsets.resize(directional_shadow_maps.size() * 4);
				for (auto i = 0; i < directional_shadow_maps.size(); i++)
				{
					directional_shadow_maps[i].reset(new ImagePrivate(device, Format_R16_SFLOAT, shadow_map_size, 1, 4, SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
					cb->image_barrier(directional_shadow_maps[i].get(), { 0U, 1U, 0U, 4U }, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
					for (auto j = 0; j < 4; j++)
					{
						auto iv = new ImageViewPrivate(directional_shadow_maps[i].get(), true, ImageView2D, { 0U, 1U, (uint)j, 1U });
						ImageViewPrivate* vs[] = {
							iv,
							shadow_depth_image->views[0].get()
						};
						directional_light_depth_framebuffers[i * 4 + j].reset(new FramebufferPrivate(device, preferences->depth_renderpass.get(), vs));
						directional_shadow_map_framebuffers[i * 4 + j].reset(new FramebufferPrivate(device, preferences->image1_r16_renderpass.get(), { &iv, 1 }));
						directional_shadow_map_descriptorsets[i * 4 + j].reset(new DescriptorSetPrivate(device->dsp.get(), post_dsl));
						directional_shadow_map_descriptorsets[i * 4 + j]->set_image(0, 0, iv, sp1);
					}
					auto iv = new ImageViewPrivate(directional_shadow_maps[i].get(), true, ImageView2DArray, { 0U, 1U, 0U, 4U });
					light_descriptorset->set_image(dsl->find_binding("directional_shadow_maps"), i, iv, sp2);
				}

				point_light_info_buffer.create(device, BufferUsageStorage, find_type(dsl->types, "PointLightInfo"), 10000);
				light_descriptorset->set_buffer(dsl->find_binding("PointLightInfos"), 0, point_light_info_buffer.buf.get());
				point_shadow_maps.resize(4);
				point_light_depth_framebuffers.resize(point_shadow_maps.size() * 6);
				point_shadow_map_framebuffers.resize(point_shadow_maps.size() * 6);
				point_shadow_map_descriptorsets.resize(point_shadow_maps.size() * 6);
				for (auto i = 0; i < point_shadow_maps.size(); i++)
				{
					point_shadow_maps[i].reset(new ImagePrivate(device, Format_R16_SFLOAT, shadow_map_size, 1, 6, SampleCount_1, ImageUsageSampled | ImageUsageAttachment, true));
					cb->image_barrier(point_shadow_maps[i].get(), { 0U, 1U, 0U, 6U }, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
					for (auto j = 0; j < 6; j++)
					{
						auto iv = new ImageViewPrivate(point_shadow_maps[i].get(), true, ImageView2D, { 0U, 1U, (uint)j, 1U });
						ImageViewPrivate* vs[] = {
							iv,
							shadow_depth_image->views[0].get()
						};
						point_light_depth_framebuffers[i * 6 + j].reset(new FramebufferPrivate(device, preferences->depth_renderpass.get(), vs));
						point_shadow_map_framebuffers[i * 6 + j].reset(new FramebufferPrivate(device, preferences->image1_r16_renderpass.get(), { &iv, 1 }));
						point_shadow_map_descriptorsets[i * 6 + j].reset(new DescriptorSetPrivate(device->dsp.get(), post_dsl));
						point_shadow_map_descriptorsets[i * 6 + j]->set_image(0, 0, iv, sp1);
					}
					auto iv = new ImageViewPrivate(point_shadow_maps[i].get(), true, ImageViewCube, { 0U, 1U, 0U, 6U });
					light_descriptorset->set_image(dsl->find_binding("point_shadow_maps"), i, iv, sp2);
				}
			}

			{
				auto dsl = DescriptorSetLayoutPrivate::get(device, L"terrain.dsl");
				terrain_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), dsl));
				terrain_info_buffer.create(device, BufferUsageStorage, find_type(dsl->types, "TerrainInfo"), 1);
				terrain_descriptorset->set_buffer(dsl->find_binding("TerrainInfos"), 0, terrain_info_buffer.buf.get());
			}

			set_model_resource(-1, (ModelPrivate*)Model::get_standard("cube"), "cube");
			set_model_resource(-1, (ModelPrivate*)Model::get_standard("sphere"), "sphere");

			line3_buffer.create(device, BufferUsageVertex, 200000);
		}

		void CanvasPrivate::set_shading(ShadingType type)
		{
			shading_type = type;
		}

		void CanvasPrivate::set_shadow(float distance, uint _csm_levels, float _csm_factor)
		{
			shadow_distance = distance;
			csm_levels = _csm_levels;
			csm_factor = _csm_factor;

			render_data_buffer.set(S<"shadow_distance"_h>, shadow_distance);
			render_data_buffer.set(S<"csm_levels"_h>, csm_levels);
			render_data_buffer.set(S<"csm_factor"_h>, csm_factor);
		}

		void CanvasPrivate::set_clear_color(const cvec4& color)
		{
			clear_color = color;

			ImmediateCommandBuffer icb(preferences->device);
			auto cb = icb.cb.get();

			cb->image_barrier(default_sky_box_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutTransferDst);
			cb->clear_color_image(default_sky_box_image.get(), color);
			cb->image_barrier(default_sky_box_image.get(), {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);

			cb->image_barrier(default_sky_irr_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutTransferDst);
			cb->clear_color_image(default_sky_irr_image.get(), color);
			cb->image_barrier(default_sky_irr_image.get(), {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);

			cb->image_barrier(default_sky_rad_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutTransferDst);
			cb->clear_color_image(default_sky_rad_image.get(), color);
			cb->image_barrier(default_sky_rad_image.get(), {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
		}

		void CanvasPrivate::set_output(std::span<ImageViewPrivate*> views)
		{
			auto device = preferences->device;
			auto hdr = preferences->hdr;

			output_imageviews.clear();
			output_framebuffers.clear();
			output_descriptorsets.clear();

			hdr_image.reset();
			hdr_framebuffer.reset();
			hdr_descriptorset.reset();

			depth_image.reset();

			mesh_framebuffers.clear();

			back_image.reset();
			back_framebuffers.clear();
			back_nearest_descriptorsets.clear();
			back_linear_descriptorsets.clear();

			auto post_dsl = DescriptorSetLayoutPrivate::get(device, L"post.dsl");

			if (views.empty())
				output_size = vec2(0.f);
			else
			{
				ImmediateCommandBuffer icb(device);
				auto cb = icb.cb.get();

				output_size = views[0]->image->sizes[0];
				output_imageviews.resize(views.size());
				output_framebuffers.resize(views.size());
				output_descriptorsets.resize(views.size());
				for (auto i = 0; i < views.size(); i++)
				{
					output_imageviews[i] = views[i];
					output_framebuffers[i].reset(new FramebufferPrivate(device, preferences->image1_8_renderpass.get(), { &views[i], 1 }));
					output_descriptorsets[i].reset(new DescriptorSetPrivate(device->dsp.get(), post_dsl));
					output_descriptorsets[i]->set_image(0, 0, views[i], SamplerPrivate::get(device, FilterNearest, FilterNearest, AddressClampToEdge));
				}

				if (hdr)
				{
					hdr_image.reset(new ImagePrivate(device, Format_R16G16B16A16_SFLOAT, output_size, 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled | ImageUsageAttachment));
					cb->image_barrier(hdr_image.get(), {}, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
					auto iv = hdr_image->views[0].get();
					hdr_framebuffer.reset(new FramebufferPrivate(device, preferences->image1_16_renderpass.get(), { &iv, 1 }));
					hdr_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), post_dsl));
					hdr_descriptorset->set_image(0, 0, iv, SamplerPrivate::get(device, FilterNearest, FilterNearest, AddressClampToEdge));
				}

				depth_image.reset(new ImagePrivate(device, Format_Depth16, output_size, 1, 1, SampleCount_1, ImageUsageTransferDst | ImageUsageSampled | ImageUsageAttachment));
				cb->image_barrier(depth_image.get(), {}, ImageLayoutUndefined, ImageLayoutAttachment);

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

				back_image.reset(new ImagePrivate(device, hdr ? Format_R16G16B16A16_SFLOAT : Format_B8G8R8A8_UNORM, output_size, 0, 1, SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
				cb->image_barrier(back_image.get(), { 0U, back_image->levels, 0U, 1U }, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
				back_framebuffers.resize(back_image->levels);
				back_nearest_descriptorsets.resize(back_image->levels);
				back_linear_descriptorsets.resize(back_image->levels);
				for (auto i = 0; i < back_image->levels; i++)
				{
					auto iv = back_image->views[i].get();
					back_framebuffers[i].reset(new FramebufferPrivate(device, hdr ? preferences->image1_16_renderpass.get() : preferences->image1_8_renderpass.get(), { &iv, 1 }));
					back_nearest_descriptorsets[i].reset(new DescriptorSetPrivate(device->dsp.get(), post_dsl));
					back_linear_descriptorsets[i].reset(new DescriptorSetPrivate(device->dsp.get(), post_dsl));
					back_nearest_descriptorsets[i]->set_image(0, 0, iv, SamplerPrivate::get(device, FilterNearest, FilterNearest, AddressClampToEdge));
					back_linear_descriptorsets[i]->set_image(0, 0, iv, SamplerPrivate::get(device, FilterLinear, FilterLinear, AddressClampToEdge));
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
					element_descriptorset->set_image(0, slot, dst.ia->image->views[0].get(), dst.ia->border ? 
						SamplerPrivate::get(device, FilterLinear, FilterLinear, AddressClampToEdge) : SamplerPrivate::get(device, FilterNearest, FilterNearest, AddressClampToEdge));
				else if (dst.fa)
					element_descriptorset->set_image(0, slot, dst.fa->view, SamplerPrivate::get(device, FilterNearest, FilterNearest, AddressClampToEdge));
				else
					element_descriptorset->set_image(0, slot, dst.iv ? dst.iv : iv_wht, SamplerPrivate::get(device, FilterLinear, FilterLinear, AddressClampToEdge));
			}
			return slot;
		}

		ImageView* CanvasPrivate::get_texture_resource(uint slot)
		{
			if (slot < texture_resources.size())
			{
				auto& r = texture_resources[slot];
				return r.iv;
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
				material_descriptorset->set_image(1, slot, iv ? iv : iv_wht, iv && sp ? sp : SamplerPrivate::get(device, FilterLinear, FilterLinear, AddressClampToEdge));
			}
			return slot;
		}

		Material* CanvasPrivate::get_material_resource(uint slot)
		{
			if (slot < material_resources.size())
			{
				auto& r = material_resources[slot];
				return r->material;
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
				if (!mat)
					material_resources[slot].reset();
				else
				{
					auto mr = new MaterialResourceSlot(this);
					mr->name = name;
					mr->material = mat;
					material_resources[slot].reset(mr);

					auto dst = material_info_buffer.mark_item(slot);

					material_info_buffer.set(dst, S<"color"_h>, mat->color);
					material_info_buffer.set(dst, S<"metallic"_h>, mat->metallic);
					material_info_buffer.set(dst, S<"roughness"_h>, mat->roughness);
					material_info_buffer.set(dst, S<"alpha_test"_h>, mat->alpha_test);

					if (!mat->dir.empty())
					{
						ivec4 ids;
						for (auto i = 0; i < size(mr->textures); i++)
						{
							auto& src = mat->textures[i];
							auto& dst = mr->textures[i];
							if (!src.filename.empty())
							{
								auto img = ImagePrivate::create(device, mat->dir / src.filename, true);
								auto idx = set_texture_resource(-1, img->views.back().get(), SamplerPrivate::get(device, src.mag_filter, src.min_filter, src.address_mode), "");
								ids[i] = dst.first = idx;
								dst.second.reset(img);
							}
							else
							{
								ids[i] = dst.first = -1;
								dst.second.reset(nullptr);
							}
						}
						material_info_buffer.set(dst, S<"map_indices"_h>, ids);
					}

					ImmediateCommandBuffer icb(device);
					auto cb = icb.cb.get();
					material_info_buffer.upload(cb);
				}
			}
			return slot;
		}

		Model* CanvasPrivate::get_model_resource(uint slot)
		{
			if (slot < model_resources.size())
			{
				auto& r = model_resources[slot];
				return r->model;
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
				if (!mod)
					model_resources[slot].reset();
				else
				{
					auto mr = new ModelResourceSlot(this);
					mr->name = name;
					mr->model = mod;
					model_resources[slot].reset(mr);

					mr->materials.resize(mod->materials.size());
					for (auto i = 0; i < mod->materials.size(); i++)
						mr->materials[i] = set_material_resource(-1, mod->materials[i].get(), "");

					ImmediateCommandBuffer icb(device);
					auto cb = icb.cb.get();

					mr->meshes.resize(mod->meshes.size());
					for (auto i = 0; i < mod->meshes.size(); i++)
					{
						auto ms = mod->meshes[i].get();

						auto mrm = new ModelResourceSlot::Mesh;

						mrm->vertex_buffer.create(device, BufferUsageVertex, ms->positions.size());
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
						mrm->vertex_buffer.upload(cb);

						if (!ms->bones.empty())
						{
							mrm->weight_buffer.create(device, BufferUsageVertex, ms->positions.size());
							std::vector<std::vector<std::pair<uint, float>>> weights;
							weights.resize(ms->positions.size());
							for (auto j = 0; j < ms->bones.size(); j++)
							{
								auto& b = ms->bones[j];
								for (auto& w : b->weights)
									weights[w.vid].emplace_back(j, w.w);
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
							mrm->weight_buffer.upload(cb);
						}

						mrm->index_buffer.create(device, BufferUsageIndex, ms->indices.size());
						mrm->index_buffer.push(ms->indices.size(), ms->indices.data());
						mrm->index_buffer.upload(cb);

						mrm->material_id = mr->materials[ms->material_index];

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

		void CanvasPrivate::add_vtx(const vec2& position, const vec2& uv, const cvec4& color)
		{
			ElementVertex v;
			v.position = position;
			v.uv = uv;
			v.color = color;
			element_vertex_buffer.push(1, &v);
			
			last_element_cmd->vertices_count++;
		}

		void CanvasPrivate::add_idx(uint idx)
		{
			element_index_buffer.push(1, &idx);

			last_element_cmd->indices_count++;
		}

		void CanvasPrivate::begin_path()
		{
			paths.clear();
			paths.emplace_back();
		}

		void CanvasPrivate::move_to(const vec2& pos)
		{
			paths.emplace_back();
			paths.back().push_back(pos);
		}

		void CanvasPrivate::line_to(const vec2& pos)
		{
			paths.back().push_back(pos);
		}

		/*
		inline void path_arc(std::vector<vec2>& points, const vec2& center, float radius, float a1, float a2, uint lod = 0)
		{
			int a = pieces_num * a1;
			int b = pieces_num * a2;
			lod += 1;
			for (; a <= b; a += lod)
				points.push_back(center + pieces[a % pieces_num] * radius);
		}

		// roundness: LT RT RB LB
		inline void path_rect(std::vector<vec2>& points, const vec2& pos, const vec2& size, const vec4& roundness = vec4(0.f), uint lod = 0)
		{
			if (roundness[0] > 0.f)
				path_arc(points, pos + vec2(roundness[0]), roundness[0], 0.5f, 0.75f, lod);
			else
				points.push_back(pos);
			if (roundness[1] > 0.f)
				path_arc(points, pos + vec2(size.x - roundness[1], roundness[1]), roundness[1], 0.75f, 1.f, lod);
			else
				points.push_back(pos + vec2(size.x, 0.f));
			if (roundness[2] > 0.f)
				path_arc(points, pos + size - vec2(roundness[2]), roundness[2], 0.f, 0.25f, lod);
			else
				points.push_back(pos + size);
			if (roundness[3] > 0.f)
				path_arc(points, pos + vec2(roundness[3], size.y - roundness[3]), roundness[3], 0.25f, 0.5f, lod);
			else
				points.push_back(pos + vec2(0.f, size.y));
		}

		inline void path_circle(std::vector<vec2>& points, const vec2& center, float radius, uint lod = 0)
		{
			path_arc(points, center, radius, 0.f, 1.f, lod);
		}

		inline void path_bezier(std::vector<vec2>& points, const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4, uint level = 0)
		{
			auto dx = p4.x - p1.x;
			auto dy = p4.y - p1.y;
			auto d2 = ((p2.x - p4.x) * dy - (p2.y - p4.y) * dx);
			auto d3 = ((p3.x - p4.x) * dy - (p3.y - p4.y) * dx);
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

		static std::vector<vec2> calculate_normals(const std::vector<vec2>& points, bool closed)
		{
			std::vector<vec2> normals(points.size());
			for (auto i = 0; i < points.size() - 1; i++)
			{
				auto d = normalize(points[i + 1] - points[i]);
				auto normal = vec2(d.y, -d.x);

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

		void CanvasPrivate::stroke(const cvec4& col, float thickness, bool aa)
		{
			thickness *= 0.5f;

			add_draw_element_cmd(0);
			auto uv = vec2(0.5f);

			for (auto& path : paths)
			{
				auto& points = path;
				if (points.size() < 2)
					continue;

				auto vtx_cnt0 = last_element_cmd->vertices_count;

				auto closed = points[0] == points[points.size() - 1];
				auto normals = calculate_normals(points, closed);

				if (aa)
				{
					auto col_c = col;
					col_c.a *= min(thickness / feather, 1.f);
					auto col_t = col;
					col_t.a = 0;

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

		void CanvasPrivate::fill(const cvec4& col, bool aa)
		{
			add_draw_element_cmd(0);
			auto uv = vec2(0.5f);

			for (auto& path : paths)
			{
				auto& points = path;
				if (points.size() < 3)
					continue;

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
					col_t.a = 0;

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

		void CanvasPrivate::draw_image(uint res_id, uint tile_id, const vec2& pos, const vec2& size, const mat2& axes, const vec2& uv0, const vec2& uv1, const cvec4& tint_col)
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
				auto tuv0 = vec2(tuv.x, tuv.y);
				auto tuv1 = vec2(tuv.z, tuv.w);
				_uv0 = mix(tuv0, tuv1, uv0);
				_uv1 = mix(tuv0, tuv1, uv1);
			}

			add_draw_element_cmd(res_id);

			auto vtx_cnt = last_element_cmd->vertices_count;

			add_vtx(pos, _uv0, tint_col);
			add_vtx(pos + size.x * axes[0], vec2(_uv1.x, _uv0.y), tint_col);
			add_vtx(pos + size.x * axes[0] + size.y * axes[1], _uv1, tint_col);
			add_vtx(pos + size.y * axes[1], vec2(_uv0.x, _uv1.y), tint_col);
			add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 2);
		}

		void CanvasPrivate::draw_text(uint res_id, const wchar_t* text_beg, const wchar_t* text_end, uint font_size, const cvec4& col, const vec2& pos, const mat2& axes)
		{
			auto& res = element_resources[res_id];
			if (!res.fa)
				return;
			auto atlas = res.fa;

			add_draw_element_cmd(res_id);

			auto p = vec2(0.f);

			auto ptext = text_beg;
			while ((!text_end || ptext != text_end) && *ptext)
			{
				auto ch = *ptext;
				if (!ch)
					break;
				if (ch == '\n')
				{
					p.y += font_size;
					p.x = 0.f;
				}
				else if (ch != '\r')
				{
					if (ch == '\t')
						ch = ' ';

					auto& g = atlas->get_glyph(ch, font_size);
					auto o = p + vec2(g.off);
					auto s = vec2(g.size);
					auto uv = g.uv;
					auto uv0 = vec2(uv.x, uv.y);
					auto uv1 = vec2(uv.z, uv.w);

					auto vtx_cnt = last_element_cmd->vertices_count;

					add_vtx(pos + o * axes, uv0, col);
					add_vtx(pos + o.x * axes[0] + (o.y - s.y) * axes[1], vec2(uv0.x, uv1.y), col);
					add_vtx(pos + (o.x + s.x) * axes[0] + (o.y - s.y) * axes[1], uv1, col);
					add_vtx(pos + (o.x + s.x) * axes[0] + o.y * axes[1], vec2(uv1.x, uv0.y), col);
					add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 2);

					p.x += g.advance;
				}

				ptext++;
			}
		}

		static void get_frustum_points(float zNear, float zFar, float tan_hf_fovy, float aspect, const mat4& transform, vec3* dst)
		{
			auto y1 = zNear * tan_hf_fovy;
			auto y2 = zFar * tan_hf_fovy;
			auto x1 = y1 * aspect;
			auto x2 = y2 * aspect;

			dst[0] = vec3(transform * vec4(-x1, y1, -zNear, 1.f));
			dst[1] = vec3(transform * vec4(x1, y1, -zNear, 1.f));
			dst[2] = vec3(transform * vec4(x1, -y1, -zNear, 1.f));
			dst[3] = vec3(transform * vec4(-x1, -y1, -zNear, 1.f));
			dst[4] = vec3(transform * vec4(-x2, y2, -zFar, 1.f));
			dst[5] = vec3(transform * vec4(x2, y2, -zFar, 1.f));
			dst[6] = vec3(transform * vec4(x2, -y2, -zFar, 1.f));
			dst[7] = vec3(transform * vec4(-x2, -y2, -zFar, 1.f));
		}

		static vec4 make_plane(const vec3& p1, const vec3& p2, const vec3& p3)
		{
			auto v1 = p2 - p1;
			auto v2 = p3 - p1;
			auto n = -normalize(cross(v1, v2));
			return vec4(n, dot(n, -p1));
		}


		void CanvasPrivate::set_camera(float _fovy, float _aspect, float _zNear, float _zFar, const mat3& dirs, const vec3& _coord)
		{
			fovy = _fovy;
			aspect = _aspect;
			zNear = _zNear;
			zFar = _zFar;
			camera_coord = _coord;
			camera_dirs = dirs;

			view_inv_matrix = mat4(dirs);
			view_inv_matrix[3] = vec4(camera_coord, 1.f);
			view_matrix = inverse(view_inv_matrix);
			proj_matrix = perspective(radians(fovy), aspect, zNear, zFar);
			proj_matrix[1][1] *= -1.f;
			proj_view_matrix = proj_matrix * view_matrix;

			render_data_buffer.set(S<"fovy"_h>, fovy);
			render_data_buffer.set(S<"aspect"_h>, aspect);
			render_data_buffer.set(S<"zNear"_h>, zNear);
			render_data_buffer.set(S<"zFar"_h>, zFar);
			render_data_buffer.set(S<"camera_coord"_h>, camera_coord);
			render_data_buffer.set(S<"camera_dirs"_h>, mat4(camera_dirs));
			render_data_buffer.set(S<"view_inv"_h>, view_inv_matrix);
			render_data_buffer.set(S<"view"_h>, view_matrix);
			render_data_buffer.set(S<"proj"_h>, proj_matrix);
			render_data_buffer.set(S<"proj_view"_h>, proj_view_matrix);

			{
				vec3 ps[8];
				get_frustum_points(zNear, zFar, tan(radians(fovy * 0.5f)), aspect, view_inv_matrix, ps);
				auto dst = (vec4*)render_data_buffer.dst(S<"frustum_planes"_h>);
				dst[0] = make_plane(ps[0], ps[1], ps[2]); // near
				dst[1] = make_plane(ps[5], ps[4], ps[6]); // far
				dst[2] = make_plane(ps[4], ps[0], ps[7]); // left
				dst[3] = make_plane(ps[1], ps[5], ps[2]); // right
				dst[4] = make_plane(ps[4], ps[5], ps[0]); // top
				dst[5] = make_plane(ps[3], ps[2], ps[7]); // bottom
			}
		}

		void CanvasPrivate::set_sky(ImageViewPrivate* box, ImageViewPrivate* irr, ImageViewPrivate* rad, ImageViewPrivate* lut)
		{
			auto device = preferences->device;
			auto dsl = DescriptorSetLayoutPrivate::get(device, L"sky.dsl");
			auto sp = SamplerPrivate::get(device, FilterLinear, FilterLinear, AddressClampToEdge);
			sky_descriptorset->set_image(dsl->find_binding("sky_box"), 0, box, sp);
			sky_descriptorset->set_image(dsl->find_binding("sky_irr"), 0, irr, sp);
			sky_descriptorset->set_image(dsl->find_binding("sky_rad"), 0, rad, sp);
			sky_descriptorset->set_image(dsl->find_binding("sky_lut"), 0, lut, sp);

			render_data_buffer.set(S<"sky_rad_levels"_h>, rad->subresource.layer_count);
		}

		void CanvasPrivate::draw_mesh(uint mod_id, uint mesh_idx, const mat4& transform, bool cast_shadow, ArmatureDeformer* deformer)
		{
			if (cmds.empty() || cmds.back()->type != Cmd::DrawMesh)
			{
				last_mesh_cmd = new CmdDrawMesh;
				cmds.emplace_back(last_mesh_cmd);
			}

			auto dst = mesh_matrix_buffer.mark_item(meshes_count);
			mesh_matrix_buffer.set(dst, S<"transform"_h>, transform);
			mesh_matrix_buffer.set(dst, S<"normal_matrix"_h>, mat4(transpose(inverse(mat3(transform)))));

			last_mesh_cmd->meshes.emplace_back(meshes_count, model_resources[mod_id]->meshes[mesh_idx].get(), cast_shadow, (ArmatureDeformerPrivate*)deformer);
			meshes_count++;
		}

		void CanvasPrivate::draw_terrain(const uvec2& blocks, const vec3& scale, const vec3& coord, float tess_levels, uint height_tex_id, uint normal_tex_id, uint material_id)
		{
			auto cmd = new CmdDrawTerrain;
			cmd->idx = terrains_count;
			cmd->drawcall_count = blocks.x * blocks.y;
			cmd->material_id = material_id;
			cmds.emplace_back(cmd);

			auto dst = terrain_info_buffer.mark_item(terrains_count);
			terrain_info_buffer.set(dst, S<"coord"_h>, coord);
			terrain_info_buffer.set(dst, S<"blocks"_h>, blocks);
			terrain_info_buffer.set(dst, S<"scale"_h>, scale);
			terrain_info_buffer.set(dst, S<"tess_levels"_h>, tess_levels);
			terrain_info_buffer.set(dst, S<"height_tex_id"_h>, height_tex_id);
			terrain_info_buffer.set(dst, S<"normal_tex_id"_h>, normal_tex_id);
			terrain_info_buffer.set(dst, S<"material_id"_h>, material_id);

			terrains_count++;
		}

		void CanvasPrivate::add_light(LightType type, const mat3& dirs, const vec3& color, bool cast_shadow)
		{
			if (type == LightDirectional)
			{
				auto dir = normalize(-dirs[2]);
				auto side = normalize(dirs[0]);
				auto up = normalize(dirs[1]);

				auto shadow_map_index = -1;
				if (cast_shadow && directional_shadows.size() < 4)
					shadow_map_index = directional_shadows.size();

				auto dst = directional_light_info_buffer.mark_item(directional_lights_count);
				directional_light_info_buffer.set(dst, S<"dir"_h>, dir);
				directional_light_info_buffer.set(dst, S<"distance"_h>, shadow_distance);
				directional_light_info_buffer.set(dst, S<"color"_h>, color);
				directional_light_info_buffer.set(dst, S<"shadow_map_index"_h>, shadow_map_index);

				if (shadow_map_index != -1)
				{
					auto zFar = shadow_distance;
					auto tan_hf_fovy = tan(radians(fovy * 0.5f));
					auto view_inv = view_inv_matrix;

					DirectionalShadow shadow;

					auto dstm = (mat4*)directional_light_info_buffer.dst(S<"shadow_matrices"_h>, dst);
					for (auto j = 0; j < csm_levels; j++)
					{
						auto n = j / (float)csm_levels;
						n = n * n * zFar;
						auto f = (j + 1) / (float)csm_levels;
						f = f * f * zFar;

						vec3 ps[8];
						get_frustum_points(n, f, tan_hf_fovy, aspect, view_inv, ps);

						auto c = (ps[0] + ps[1] + ps[2] + ps[3] +
							ps[4] + ps[5] + ps[6] + ps[7]) * 0.125f;
						auto light_inv = mat4(mat3(side, up, dir));
						light_inv[3] = vec4(c, 1.f);
						light_inv = inverse(light_inv);
						vec2 LT = vec2(zFar);
						vec2 RB = vec2(-zFar);
						for (auto k = 0; k < 8; k++)
						{
							auto p = light_inv * vec4(ps[k], 1.f);
							LT = min(LT, p.xy());
							RB = max(RB, p.xy());
						}

						shadow.matrices[j] = dstm[j] = orthoRH(LT.x, RB.x, LT.y, RB.y, 0.f, shadow_distance) *
							lookAt(c + dir * shadow_distance * 0.5f, c, up);
					}

					directional_shadows.push_back(shadow);
				}

				{
					// TODO
					auto dst = light_indices_buffer.mark_item(0);
					light_indices_buffer.set(dst, S<"directional_lights_count"_h>, directional_lights_count + 1);
				}

				directional_lights_count++;
			}
			else
			{
				auto coord = dirs[0];

				auto shadow_map_index = -1;
				if (cast_shadow && point_shadows.size() < 4)
					shadow_map_index = point_shadows.size();

				auto dst = point_light_info_buffer.mark_item(point_lights_count);
				point_light_info_buffer.set(dst, S<"coord"_h>, coord);
				point_light_info_buffer.set(dst, S<"distance"_h>, 10.f);
				point_light_info_buffer.set(dst, S<"color"_h>, color);
				point_light_info_buffer.set(dst, S<"shadow_map_index"_h>, shadow_map_index);

				if (shadow_map_index != -1)
				{
					PointShadow shadow;

					shadow.coord = coord;

					point_shadows.push_back(shadow);
				}

				{
					// TODO
					auto dst = light_indices_buffer.mark_item(0);
					light_indices_buffer.set(dst, S<"point_lights_count"_h>, point_lights_count + 1);
					auto dsta = (uint*)light_indices_buffer.dst(S<"point_light_indices"_h>, dst);
					dsta[point_lights_count] = point_lights_count;
				}

				point_lights_count++;
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

		void CanvasPrivate::set_scissor(const Rect& _scissor)
		{
			auto scissor = Rect(
				max(_scissor.LT.x, 0.f),
				max(_scissor.LT.y, 0.f),
				min(_scissor.RB.x, (float)output_size.x),
				min(_scissor.RB.y, (float)output_size.y)
			);
			if (scissor == curr_scissor)
				return;
			curr_scissor = scissor;
			cmds.emplace_back(new CmdSetScissor(scissor));
		}

		void CanvasPrivate::add_blur(const Rect& _range, uint radius)
		{
			auto range = Rect(
				max(_range.LT.x, 0.f),
				max(_range.LT.y, 0.f),
				min(_range.RB.x, (float)output_size.x),
				min(_range.RB.y, (float)output_size.y));
			cmds.emplace_back(new CmdBlur(range, radius));
		}

		void CanvasPrivate::add_bloom()
		{
			cmds.emplace_back(new CmdBloom());
		}

		void CanvasPrivate::prepare()
		{
			element_vertex_buffer.stagnum = 0;
			element_index_buffer.stagnum = 0;

			meshes_count = 0;
			terrains_count = 0;
			directional_lights_count = 0;
			point_lights_count = 0;
			{
				// TODO
				auto dst = light_indices_buffer.mark_item(0);
				light_indices_buffer.set(dst, S<"directional_lights_count"_h>, 0);
				light_indices_buffer.set(dst, S<"point_lights_count"_h>, 0);
			}
			directional_shadows.clear();
			point_shadows.clear();

			line3_buffer.stagnum = 0;

			curr_scissor = Rect(0.f, 0.f, output_size.x, output_size.y);

			cmds.clear();
		}

		void CanvasPrivate::record(CommandBufferPrivate* cb, uint image_index)
		{
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
							deformer->poses_buffer.upload(cb);
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

			cb->image_barrier(depth_image.get(), {}, ImageLayoutAttachment, ImageLayoutTransferDst);
			cb->clear_depth_image(depth_image.get(), 1.f);
			cb->image_barrier(depth_image.get(), {}, ImageLayoutTransferDst, ImageLayoutAttachment);

			cb->set_viewport(Rect(0.f, 0.f, output_size.x, output_size.y));
			cb->set_scissor(Rect(0.f, 0.f, output_size.x, output_size.y));
			auto ele_vtx_off = 0;
			auto ele_idx_off = 0;
			auto first_3d = true;
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
						element_index_buffer.upload(cb);

					}
					cb->set_scissor(Rect(0.f, 0.f, output_size.x, output_size.y));
					cb->bind_vertex_buffer(element_vertex_buffer.buf.get(), 0);
					cb->bind_index_buffer(element_index_buffer.buf.get(), IndiceTypeUint);
					cb->begin_renderpass(nullptr, dst_fb);
					cb->bind_pipeline(preferences->element_pipeline.get());
					cb->bind_descriptor_set(S<"element"_h>, element_descriptorset.get());
					cb->push_constant_t(0, vec2(2.f / output_size.x, 2.f / output_size.y));
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
					if (first_3d)
					{
						first_3d = false;

						render_data_buffer.set(S<"fb_size"_h>, output_size);

						render_data_buffer.upload(cb);
						mesh_matrix_buffer.upload(cb);
						terrain_info_buffer.upload(cb);
						light_indices_buffer.upload(cb);
						directional_light_info_buffer.upload(cb);
						point_light_info_buffer.upload(cb);

						cb->set_viewport(Rect(0.f, 0.f, shadow_map_size.x, shadow_map_size.y));
						cb->set_scissor(Rect(0.f, 0.f, shadow_map_size.x, shadow_map_size.y));

						for (auto map_idx = 0; map_idx < directional_shadows.size(); map_idx++)
						{
							auto& s = directional_shadows[map_idx];

							for (auto i = 0; i < csm_levels; i++)
							{
								vec4 cvs[] = {
									vec4(1.f, 0.f, 0.f, 0.f),
									vec4(1.f, 0.f, 0.f, 0.f)
								};
								cb->begin_renderpass(nullptr, directional_light_depth_framebuffers[map_idx * 4 + i].get(), cvs);
								auto first = true;
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
												auto mat = material_resources[mrm->material_id].get();
												cb->bind_vertex_buffer(mrm->vertex_buffer.buf.get(), 0);
												cb->bind_index_buffer(mrm->index_buffer.buf.get(), IndiceTypeUint);
												if (std::get<3>(m))
												{
													cb->bind_pipeline(mat->get_pipeline(MaterialForDepthArmature));
													cb->bind_vertex_buffer(mrm->weight_buffer.buf.get(), 1);
													cb->bind_descriptor_set(S<"armature"_h>, std::get<3>(m)->descriptorset.get());
												}
												else
													cb->bind_pipeline(mat->get_pipeline(MaterialForDepth));
												if (first)
												{
													cb->bind_descriptor_set(S<"mesh"_h>, mesh_descriptorset.get());
													cb->bind_descriptor_set(S<"material"_h>, material_descriptorset.get());
													struct
													{
														mat4 proj_view;
														float zNear;
														float zFar;
													}pc;
													pc.proj_view = s.matrices[i];
													pc.zNear = 0.f;
													pc.zFar = 100.f;
													cb->push_constant(0, sizeof(pc), &pc);
													first = false;
												}
												cb->draw_indexed(mrm->index_buffer.capacity, 0, 0, 1, (std::get<0>(m) << 16) + mrm->material_id);
											}
										}
									}
								}
								cb->end_renderpass();

								//cb->image_barrier(directional_shadow_maps[map_idx].get(), { 0U, 1U, (uint)i, 1U }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
								//cb->begin_renderpass(nullptr, shadow_blur_pingpong_framebuffer.get());
								//cb->bind_pipeline(preferences->blurh_depth_pipeline.get());
								//cb->bind_descriptor_set(S<"post"_h>, directional_shadow_map_descriptorsets[map_idx * 4 + i].get());
								//cb->push_constant_t(0, 1.f / shadow_map_size.x);
								//cb->draw(3, 1, 0, 0);
								//cb->end_renderpass();

								//cb->image_barrier(shadow_blur_pingpong_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
								//cb->begin_renderpass(nullptr, directional_shadow_map_framebuffers[map_idx * 4 + i].get());
								//cb->bind_pipeline(preferences->blurv_depth_pipeline.get());
								//cb->bind_descriptor_set(S<"post"_h>, shadow_blur_pingpong_descriptorset.get());
								//cb->push_constant_t(0, 1.f / shadow_map_size.y);
								//cb->draw(3, 1, 0, 0);
								//cb->end_renderpass();
							}

							cb->image_barrier(directional_shadow_maps[map_idx].get(), { 0U, 1U, 0U, 4U }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
						}
						for (auto map_idx = 0; map_idx < point_shadows.size(); map_idx++)
						{
							auto& s = point_shadows[map_idx];

							for (auto i = 0; i < 6; i++)
							{
								vec4 cvs[] = {
									vec4(1.f, 0.f, 0.f, 0.f),
									vec4(1.f, 0.f, 0.f, 0.f)
								};
								cb->begin_renderpass(nullptr, point_light_depth_framebuffers[map_idx * 6 + i].get(), cvs);
								auto first = true;
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
												auto mat = material_resources[mrm->material_id].get();
												cb->bind_vertex_buffer(mrm->vertex_buffer.buf.get(), 0);
												cb->bind_index_buffer(mrm->index_buffer.buf.get(), IndiceTypeUint);
												if (std::get<3>(m))
												{
													cb->bind_pipeline(mat->get_pipeline(MaterialForDepthArmature));
													cb->bind_vertex_buffer(mrm->weight_buffer.buf.get(), 1);
													cb->bind_descriptor_set(S<"armature"_h>, std::get<3>(m)->descriptorset.get());
												}
												else
													cb->bind_pipeline(mat->get_pipeline(MaterialForDepth));
												if (first)
												{
													cb->bind_descriptor_set(S<"mesh"_h>, mesh_descriptorset.get());
													cb->bind_descriptor_set(S<"material"_h>, material_descriptorset.get());
													auto proj = perspective(90.f, 1.f, 1.f, s.distance);
													proj[1][1] *= -1.f;
													struct
													{
														mat4 matrix;
														float zNear;
														float zFar;
													}pc;
													pc.matrix = mat4(1.f);
													switch (i)
													{
													case 0:
														pc.matrix[0][0] = -1.f;
														pc.matrix = pc.matrix * proj * lookAt(s.coord, s.coord + vec3(1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
														break;
													case 1:
														pc.matrix[0][0] = -1.f;
														pc.matrix = pc.matrix * proj * lookAt(s.coord, s.coord + vec3(-1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
														break;
													case 2:
														pc.matrix[1][1] = -1.f;
														pc.matrix = pc.matrix * proj * lookAt(s.coord, s.coord + vec3(0.f, 1.f, 0.f), vec3(1.f, 0.f, 0.f));
														break;
													case 3:
														pc.matrix[1][1] = -1.f;
														pc.matrix = pc.matrix * proj * lookAt(s.coord, s.coord + vec3(0.f, -1.f, 0.f), vec3(0.f, 0.f, -1.f));
														break;
													case 4:
														pc.matrix[0][0] = -1.f;
														pc.matrix = pc.matrix * proj * lookAt(s.coord, s.coord + vec3(0.f, 0.f, 1.f), vec3(0.f, 1.f, 0.f));
														break;
													case 5:
														pc.matrix[0][0] = -1.f;
														pc.matrix = pc.matrix * proj * lookAt(s.coord, s.coord + vec3(0.f, 0.f, -1.f), vec3(0.f, 1.f, 0.f));
														break;
													}
													pc.zNear = 1.f;
													pc.zFar = s.distance;
													cb->push_constant(0, sizeof(pc), &pc);
													first = false;
												}
												cb->draw_indexed(mrm->index_buffer.capacity, 0, 0, 1, (std::get<0>(m) << 16) + mrm->material_id);
											}
										}
									}
								}
								cb->end_renderpass();

								cb->image_barrier(point_shadow_maps[map_idx].get(), { 0U, 1U, (uint)i, 1U }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
								cb->begin_renderpass(nullptr, shadow_blur_pingpong_framebuffer.get());
								cb->bind_pipeline(preferences->blurh_depth_pipeline.get());
								cb->bind_descriptor_set(S<"post"_h>, point_shadow_map_descriptorsets[map_idx * 6 + i].get());
								cb->push_constant_t(0, 1.f / shadow_map_size.x);
								cb->draw(3, 1, 0, 0);
								cb->end_renderpass();

								cb->image_barrier(shadow_blur_pingpong_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
								cb->begin_renderpass(nullptr, point_shadow_map_framebuffers[map_idx * 6 + i].get());
								cb->bind_pipeline(preferences->blurv_depth_pipeline.get());
								cb->bind_descriptor_set(S<"post"_h>, shadow_blur_pingpong_descriptorset.get());
								cb->push_constant_t(0, 1.f / shadow_map_size.y);
								cb->draw(3, 1, 0, 0);
								cb->end_renderpass();
							}

							cb->image_barrier(point_shadow_maps[map_idx].get(), { 0U, 1U, 0U, 6U }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
						}

						cb->set_viewport(Rect(0.f, 0.f, output_size.x, output_size.y));
					}

					cb->set_scissor(Rect(0.f, 0.f, output_size.x, output_size.y));
					cb->begin_renderpass(nullptr, mesh_framebuffers[hdr_image ? 0 : image_index].get());

					{
						cb->bind_pipeline(preferences->sky_pipeline.get());
						cb->bind_descriptor_set(S<"render_data"_h>, render_data_descriptorset.get());
						cb->bind_descriptor_set(S<"sky"_h>, sky_descriptorset.get());
						auto mr = model_resources[1].get();
						auto mrm = mr->meshes[0].get();
						cb->bind_vertex_buffer(mrm->vertex_buffer.buf.get(), 0);
						cb->bind_index_buffer(mrm->index_buffer.buf.get(), IndiceTypeUint);
						cb->draw_indexed(mrm->index_buffer.capacity, 0, 0, 1, 0);
					}

					for (auto& i : p.cmd_ids)
					{
						auto& cmd = cmds[i];
						switch (cmd->type)
						{
						case Cmd::DrawMesh:
						{
							auto c = (CmdDrawMesh*)cmd.get();
							auto first = true;
							for (auto& m : c->meshes)
							{
								auto mrm = std::get<1>(m);
								auto mat = material_resources[mrm->material_id].get();
								cb->bind_vertex_buffer(mrm->vertex_buffer.buf.get(), 0);
								cb->bind_index_buffer(mrm->index_buffer.buf.get(), IndiceTypeUint);
								if (std::get<3>(m))
								{
									cb->bind_pipeline(mat->get_pipeline(MaterialForMeshArmature));
									cb->bind_vertex_buffer(mrm->weight_buffer.buf.get(), 1);
									cb->bind_descriptor_set(S<"armature"_h>, std::get<3>(m)->descriptorset.get());
								}
								else
									cb->bind_pipeline(mat->get_pipeline(MaterialForMesh));
								if (first)
								{
									cb->bind_descriptor_set(S<"render_data"_h>, render_data_descriptorset.get());
									cb->bind_descriptor_set(S<"mesh"_h>, mesh_descriptorset.get());
									cb->bind_descriptor_set(S<"material"_h>, material_descriptorset.get());
									cb->bind_descriptor_set(S<"light"_h>, light_descriptorset.get());
									cb->bind_descriptor_set(S<"sky"_h>, sky_descriptorset.get());
									first = false;
								}
								cb->draw_indexed(mrm->index_buffer.capacity, 0, 0, 1, (std::get<0>(m) << 16) + mrm->material_id);
							}
						}
							break;
						case Cmd::DrawTerrain:
						{
							auto c = (CmdDrawTerrain*)cmd.get();
							auto mat = material_resources[c->material_id].get();
							cb->bind_pipeline(mat->get_pipeline(MaterialForTerrain));
							cb->bind_descriptor_set(S<"render_data"_h>, render_data_descriptorset.get());
							cb->bind_descriptor_set(S<"material"_h>, material_descriptorset.get());
							cb->bind_descriptor_set(S<"light"_h>, light_descriptorset.get());
							cb->bind_descriptor_set(S<"sky"_h>, sky_descriptorset.get());
							cb->bind_descriptor_set(S<"terrain"_h>, terrain_descriptorset.get());
							cb->draw(4, c->drawcall_count, 0, c->idx << 16);
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
				case PassLine3:
				{
					if (line3_off == 0)
						line3_buffer.upload(cb);
					cb->set_scissor(Rect(0.f, 0.f, output_size.x, output_size.y));
					cb->begin_renderpass(nullptr, dst_fb);
					cb->bind_vertex_buffer(line3_buffer.buf.get(), 0);
					cb->bind_pipeline(preferences->line3_pipeline.get());
					cb->push_constant(0, sizeof(mat4), &proj_view_matrix);
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
					auto blur_size = vec2(blur_range.RB.x - blur_range.LT.x, blur_range.RB.y - blur_range.LT.y);
					if (blur_size.x < 1.f || blur_size.y < 1.f)
						continue;

					cb->image_barrier(dst, {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
					cb->set_scissor(Rect(blur_range.LT.x - blur_radius, blur_range.LT.y - blur_radius,
						blur_range.RB.x + blur_radius, blur_range.RB.y + blur_radius));
					cb->begin_renderpass(nullptr, back_framebuffers[0].get());
					cb->bind_pipeline(preferences->blurh_pipeline[blur_radius - 1].get());
					cb->bind_descriptor_set(dst_ds, 0);
					cb->push_constant_t(0, 1.f / output_size.x);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->image_barrier(back_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
					cb->set_scissor(blur_range);
					cb->begin_renderpass(nullptr, dst_fb);
					cb->bind_pipeline(preferences->blurv_pipeline[blur_radius - 1].get());
					cb->bind_descriptor_set(back_nearest_descriptorsets[0].get(), 0);
					cb->push_constant_t(0, 1.f / output_size.y);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();
				}
					break;
				case PassBloom:
				{
					if (!hdr_image)
						continue;

					cb->set_scissor(Rect(0.f, 0.f, output_size.x, output_size.y));

					cb->set_viewport(Rect(0.f, 0.f, output_size.x, output_size.y));
					cb->image_barrier(hdr_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
					cb->begin_renderpass(nullptr, back_framebuffers[0].get());
					cb->bind_pipeline(preferences->filter_bright_pipeline.get());
					cb->bind_descriptor_set(hdr_descriptorset.get(), 0);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					for (auto i = 0; i < back_image->levels - 1; i++)
					{
						cb->set_viewport(Rect(0.f, 0.f, back_image->sizes[i + 1].x, back_image->sizes[i + 1].y));
						cb->image_barrier(back_image.get(), { (uint)i }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
						cb->begin_renderpass(nullptr, back_framebuffers[i + 1].get());
						cb->bind_pipeline(preferences->downsample_pipeline.get());
						cb->bind_descriptor_set(back_linear_descriptorsets[i].get(), 0);
						cb->push_constant_t(0, 1.f / (vec2)back_image->sizes[i]);
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();
					}
					for (auto i = back_image->levels - 1; i > 0; i--)
					{
						cb->set_viewport(Rect(0.f, 0.f, back_image->sizes[i - 1].x, back_image->sizes[i - 1].y));
						cb->image_barrier(back_image.get(), { (uint)i }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
						cb->begin_renderpass(nullptr, back_framebuffers[i - 1].get());
						cb->bind_pipeline(preferences->upsample_pipeline.get());
						cb->bind_descriptor_set(back_linear_descriptorsets[i].get(), 0);
						cb->push_constant_t(0, 1.f / (vec2)back_image->sizes[(uint)i - 1]);
						cb->draw(3, 1, 0, 0);
						cb->end_renderpass();
					}
					cb->image_barrier(back_image.get(), { 1U }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
					cb->begin_renderpass(nullptr, hdr_framebuffer.get());
					cb->bind_pipeline(preferences->upsample_pipeline.get());
					cb->bind_descriptor_set(back_linear_descriptorsets[1].get(), 0);
					cb->push_constant_t(0, 1.f / output_size.y);
					cb->draw(3, 1, 0, 0);
					cb->end_renderpass();

					cb->set_viewport(Rect(0.f, 0.f, output_size.x, output_size.y));
				}
					break;
				}
			}

			if (hdr_image)
			{
				cb->image_barrier(hdr_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
				cb->image_barrier(output_imageviews[image_index]->image, {}, ImageLayoutPresent, ImageLayoutShaderReadOnly);
				cb->set_scissor(Rect(0.f, 0.f, output_size.x, output_size.y));
				cb->begin_renderpass(nullptr, output_framebuffers[image_index].get());
				cb->bind_pipeline(preferences->gamma_pipeline.get());
				cb->bind_descriptor_set(hdr_descriptorset.get(), 0);
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();
			}
			cb->image_barrier(output_imageviews[image_index]->image, {}, ImageLayoutShaderReadOnly, ImageLayoutPresent);

			cmds.clear();
		}

		Canvas* Canvas::create(RenderPreferences* preferences)
		{
			return new CanvasPrivate((RenderPreferencesPrivate*)preferences);
		}
	}
}
