#include "device_private.h"
#include "renderpass_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "shader_private.h"

#include <flame/foundation/blueprint.h>

namespace flame
{
	namespace graphics
	{
		DescriptorpoolPrivate::DescriptorpoolPrivate(Device* _d)
		{
			d = (DevicePrivate*)_d;

#if defined(FLAME_VULKAN)
			VkDescriptorPoolSize descriptorPoolSizes[] = {
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 128},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 32},
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2048},
				{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 32},
			};

			VkDescriptorPoolCreateInfo descriptorPoolInfo;
			descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			descriptorPoolInfo.pNext = nullptr;
			descriptorPoolInfo.poolSizeCount = FLAME_ARRAYSIZE(descriptorPoolSizes);
			descriptorPoolInfo.pPoolSizes = descriptorPoolSizes;
			descriptorPoolInfo.maxSets = 64;
			chk_res(vkCreateDescriptorPool(((DevicePrivate*)d)->v, &descriptorPoolInfo, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		DescriptorpoolPrivate::~DescriptorpoolPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyDescriptorPool(((DevicePrivate*)d)->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Descriptorpool* Descriptorpool::create(Device* d)
		{
			return new DescriptorpoolPrivate(d);
		}

		void Descriptorpool::destroy(Descriptorpool* p)
		{
			delete (DescriptorpoolPrivate*)p;
		}

		DescriptorsetlayoutPrivate::DescriptorsetlayoutPrivate(Device* _d, const std::vector<void*>& bindings)
		{
			d = (DevicePrivate*)_d;

#if defined(FLAME_VULKAN)
			std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
			vk_bindings.resize(bindings.size());
			for (auto i = 0; i < bindings.size(); i++)
			{
				auto bd = (DescriptorsetBinding*)bindings[i];

				vk_bindings[i].binding = bd->binding;
				vk_bindings[i].descriptorType = to_enum(bd->type);
				vk_bindings[i].descriptorCount = bd->count;
				vk_bindings[i].stageFlags = to_flags(ShaderStageAll);
				vk_bindings[i].pImmutableSamplers = nullptr;
			}

			VkDescriptorSetLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.bindingCount = vk_bindings.size();
			info.pBindings = vk_bindings.data();

			chk_res(vkCreateDescriptorSetLayout(((DevicePrivate*)d)->v,
				&info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		DescriptorsetlayoutPrivate::~DescriptorsetlayoutPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyDescriptorSetLayout(((DevicePrivate*)d)->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Descriptorsetlayout* Descriptorsetlayout::create(Device* d, const std::vector<void*>& bindings)
		{
			return new DescriptorsetlayoutPrivate(d, bindings);
		}

		void Descriptorsetlayout::destroy(Descriptorsetlayout* l)
		{
			delete (DescriptorsetlayoutPrivate*)l;
		}

		struct DescriptorsetBinding$
		{
			AttributeV<uint> binding$i;
			AttributeE<DescriptorType$> type$i;
			AttributeV<uint> count$i;

			AttributeV<DescriptorsetBinding> out$o;

			FLAME_GRAPHICS_EXPORTS DescriptorsetBinding$()
			{
				count$i.v = 1;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (binding$i.frame > out$o.frame)
					out$o.v.binding = binding$i.v;
				if (type$i.frame > out$o.frame)
					out$o.v.type = type$i.v;
				if (count$i.frame > out$o.frame)
					out$o.v.count = count$i.v;
				out$o.frame = maxN(binding$i.frame, type$i.frame, count$i.frame);
			}

		};

		struct Descriptorsetlayout$
		{
			AttributeP<std::vector<void*>> bindings$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (bindings$i.frame > out$o.frame)
				{
					if (out$o.v)
						Descriptorsetlayout::destroy((Descriptorsetlayout*)out$o.v);
					auto d = (Device*)bp_environment().graphics_device;
					if (d)
						out$o.v = Descriptorsetlayout::create(d, bindings$i.v ? *bindings$i.v : std::vector<void*>());
					else
					{
						printf("cannot create descriptorsetlayout\n");

						out$o.v = nullptr;
					}
					out$o.frame = bindings$i.frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Descriptorsetlayout$()
			{
				if (out$o.v)
					Descriptorsetlayout::destroy((Descriptorsetlayout*)out$o.v);
			}

		};

		DescriptorsetPrivate::DescriptorsetPrivate(Descriptorpool* _p, Descriptorsetlayout* l)
		{
			p = (DescriptorpoolPrivate*)_p;

#if defined(FLAME_VULKAN)
			VkDescriptorSetAllocateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.pNext = nullptr;
			info.descriptorPool = p->v;
			info.descriptorSetCount = 1;
			info.pSetLayouts = &((DescriptorsetlayoutPrivate*)l)->v;

			chk_res(vkAllocateDescriptorSets(p->d->v, &info, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		DescriptorsetPrivate::~DescriptorsetPrivate()
		{
#if defined(FLAME_VULKAN)
			chk_res(vkFreeDescriptorSets(p->d->v, p->v, 1, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		void DescriptorsetPrivate::set_uniformbuffer(uint binding, uint index, Buffer* b, uint offset, uint range)
		{
#if defined(FLAME_VULKAN)
			VkDescriptorBufferInfo i;
			i.buffer = ((BufferPrivate*)b)->v;
			i.offset = offset;
			i.range = range == 0 ? b->size : range;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = v;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.descriptorCount = 1;
			write.pBufferInfo = &i;
			write.pImageInfo = nullptr;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(p->d->v, 1, &write, 0, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void DescriptorsetPrivate::set_storagebuffer(uint binding, uint index, Buffer* b, uint offset, uint range)
		{
#if defined(FLAME_VULKAN)
			VkDescriptorBufferInfo i;
			i.buffer = ((BufferPrivate*)b)->v;
			i.offset = offset;
			i.range = range == 0 ? b->size : range;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = v;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write.descriptorCount = 1;
			write.pBufferInfo = &i;
			write.pImageInfo = nullptr;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(p->d->v, 1, &write, 0, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void DescriptorsetPrivate::set_imageview(uint binding, uint index, Imageview* iv, Sampler* sampler)
		{
#if defined(FLAME_VULKAN)
			VkDescriptorImageInfo i;
			i.imageView = ((ImageviewPrivate*)iv)->v;
			i.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			i.sampler = ((SamplerPrivate*)sampler)->v;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = v;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.descriptorCount = 1;
			write.pBufferInfo = nullptr;
			write.pImageInfo = &i;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(p->d->v, 1, &write, 0, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void DescriptorsetPrivate::set_storageimage(uint binding, uint index, Imageview* iv)
		{
#if defined(FLAME_VULKAN)
			VkDescriptorImageInfo i;
			i.imageView = ((ImageviewPrivate*)iv)->v;
			i.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			i.sampler = 0;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = v;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			write.descriptorCount = 1;
			write.pBufferInfo = nullptr;
			write.pImageInfo = &i;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(p->d->v, 1, &write, 0, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void Descriptorset::set_uniformbuffer(uint binding, uint index, Buffer* b, uint offset, uint range)
		{
			((DescriptorsetPrivate*)this)->set_uniformbuffer(binding, index, b, offset, range);
		}

		void Descriptorset::set_storagebuffer(uint binding, uint index, Buffer* b, uint offset, uint range)
		{
			((DescriptorsetPrivate*)this)->set_storagebuffer(binding, index, b, offset, range);
		}

		void Descriptorset::set_imageview(uint binding, uint index, Imageview* v, Sampler* sampler)
		{
			((DescriptorsetPrivate*)this)->set_imageview(binding, index, v, sampler);
		}

		void Descriptorset::set_storageimage(uint binding, uint index, Imageview* v)
		{
			((DescriptorsetPrivate*)this)->set_storageimage(binding, index, v);
		}

		Descriptorset* Descriptorset::create(Descriptorpool* p, Descriptorsetlayout* l)
		{
			return new DescriptorsetPrivate(p, l);
		}

		void Descriptorset::destroy(Descriptorset* s)
		{
			delete (DescriptorsetPrivate*)s;
		}

		ShaderPrivate::ShaderPrivate(Device* d, const std::wstring& filename, const std::string& prefix) :
			d((DevicePrivate*)d)
		{
			stages.resize(1);
			auto ext = std::fs::path(filename).extension();
			if (ext == L".vert")
				stages[0].stage = ShaderStageVert;
			else if (ext == L".tesc")
				stages[0].stage = ShaderStageTesc;
			else if (ext == L".tese")
				stages[0].stage = ShaderStageTese;
			else if (ext == L".geom")
				stages[0].stage = ShaderStageGeom;
			else if (ext == L".frag")
				stages[0].stage = ShaderStageFrag;
			else if (ext == L".comp")
				stages[0].stage = ShaderStageComp;
			stages[0].entry_name = "main";

			auto hash = H(prefix.c_str());
			std::wstring spv_filename(filename + L"." + std::to_wstring(hash) + L".spv");

			if (!std::fs::exists(spv_filename) || std::fs::last_write_time(spv_filename) < std::fs::last_write_time(filename))
			{
				auto vk_sdk_path = s2w(getenv("VK_SDK_PATH"));
				assert(vk_sdk_path != L"");

				std::fs::remove(spv_filename);

				std::string pfx;
				pfx += "#version 450 core\n";
				pfx += "#extension GL_ARB_shading_language_420pack : enable\n";
				if (stages[0].stage != ShaderStageComp)
					pfx += "#extension GL_ARB_separate_shader_objects : enable\n";
				pfx += "\n" + prefix;
				auto temp_filename = L"temp" + ext.wstring();
				{
					std::ofstream ofile(temp_filename);
					auto file = get_file_content(filename);
					ofile.write(pfx.c_str(), pfx.size());
					ofile.write(file.first.get(), file.second);
					ofile.close();
				}
				std::wstring command_line(L" " + temp_filename + L" -flimit-file shader.conf -o" + spv_filename);
				auto output = exec_and_get_output((vk_sdk_path + L"/Bin/glslc.exe"), command_line);
				std::fs::remove(temp_filename);
				if (!std::fs::exists(spv_filename))
					printf("shader \"%s\" compile error:\n%s\n", w2s(filename).c_str(), output.p->c_str());
				delete_mail(output);
			}

			auto spv_file = get_file_content(spv_filename);
			if (!spv_file.first)
				assert(0);

#if defined(FLAME_VULKAN)
			VkShaderModuleCreateInfo shader_info;
			shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shader_info.flags = 0;
			shader_info.pNext = nullptr;
			shader_info.codeSize = spv_file.second;
			shader_info.pCode = (uint32_t*)spv_file.first.get();
			chk_res(vkCreateShaderModule(((DevicePrivate*)d)->v, &shader_info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		ShaderPrivate::ShaderPrivate(Device* d, const std::string& content, const std::vector<void*>& stages) :
			d((DevicePrivate*)d)
		{

		}

		ShaderPrivate::~ShaderPrivate()
		{
#if defined(FLAME_VULKAN)
			if (v)
				vkDestroyShaderModule(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Shader* Shader::create(Device* d, const std::wstring& filename, const std::string& prefix)
		{
			return new ShaderPrivate(d, filename, prefix);
		}

		Shader* create(Device* d, const std::string& content, const std::vector<void*>& stages)
		{
			return new ShaderPrivate(d, content, stages);
		}

		void Shader::destroy(Shader* s)
		{
			delete (ShaderPrivate*)s;
		}

		struct Shader$
		{
			AttributeV<std::wstring> filename$i;
			AttributeV<std::string> prefix$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS Shader$()
			{
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (out$o.frame || filename$i.frame > out$o.frame || prefix$i.frame > out$o.frame)
				{
					if (out$o.v)
						Shader::destroy((Shader*)out$o.v);
					auto d = (Device*)bp_environment().graphics_device;
					if (d)
						out$o.v = Shader::create(d, bp_environment().path + L"/" + filename$i.v, prefix$i.v);
					else
					{
						printf("cannot create shader\n");

						out$o.v = nullptr;
					}
					out$o.frame = max(filename$i.frame, prefix$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Shader$()
			{
				if (out$o.v)
					Shader::destroy((Shader*)out$o.v);
			}

		};

		PipelinelayoutPrivate::PipelinelayoutPrivate(Device* _d, const std::vector<void*>& descriptorsetlayouts, uint push_constant_size)
		{
			d = (DevicePrivate*)_d;

#if defined(FLAME_VULKAN)
			std::vector<VkDescriptorSetLayout> vk_descriptorsetlayouts;
			vk_descriptorsetlayouts.resize(descriptorsetlayouts.size());
			for (auto i = 0; i < vk_descriptorsetlayouts.size(); i++)
				vk_descriptorsetlayouts[i] = ((DescriptorsetlayoutPrivate*)descriptorsetlayouts[i])->v;

			VkPushConstantRange vk_pushconstant;
			vk_pushconstant.offset = 0;
			vk_pushconstant.size = push_constant_size;
			vk_pushconstant.stageFlags = to_flags(ShaderStageAll);

			VkPipelineLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.setLayoutCount = vk_descriptorsetlayouts.size();
			info.pSetLayouts = vk_descriptorsetlayouts.data();
			info.pushConstantRangeCount = push_constant_size > 0 ? 1 : 0;
			info.pPushConstantRanges = push_constant_size > 0 ? &vk_pushconstant : nullptr;

			chk_res(vkCreatePipelineLayout(d->v, &info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		PipelinelayoutPrivate::~PipelinelayoutPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyPipelineLayout(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		struct Pipelinelayout$
		{
			AttributeP<std::vector<void*>> descriptorsetlayouts$i;
			AttributeV<uint> push_constant_size$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (descriptorsetlayouts$i.frame > out$o.frame || push_constant_size$i.frame > out$o.frame)
				{
					if (out$o.v)
						Pipelinelayout::destroy((Pipelinelayout*)out$o.v);
					auto d = (Device*)bp_environment().graphics_device;
					if (d && descriptorsetlayouts$i.v && !descriptorsetlayouts$i.v->empty())
						out$o.v = Pipelinelayout::create(d, *descriptorsetlayouts$i.v, push_constant_size$i.v);
					else
					{
						printf("cannot create pipelinelayout\n");

						out$o.v = nullptr;
					}
					out$o.frame = max(descriptorsetlayouts$i.frame, push_constant_size$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Pipelinelayout$()
			{
				if (out$o.v)
					Pipelinelayout::destroy((Pipelinelayout*)out$o.v);
			}

		};

		Pipelinelayout* Pipelinelayout::create(Device* d, const std::vector<void*>& descriptorsetlayouts, uint push_constant_size)
		{
			return new PipelinelayoutPrivate(d, descriptorsetlayouts, push_constant_size);
		}

		void Pipelinelayout::destroy(Pipelinelayout* l)
		{
			delete (PipelinelayoutPrivate*)l;
		}

		PipelinePrivate::PipelinePrivate(Device* _d, const GraphicsPipelineInfo& info)
		{
			d = (DevicePrivate*)_d;
			pll = (PipelinelayoutPrivate*)info.layout;

#if defined(FLAME_VULKAN)
			std::vector<VkPipelineShaderStageCreateInfo> vk_stage_infos;
			std::vector<VkVertexInputAttributeDescription> vk_vi_attributes;
			std::vector<VkVertexInputBindingDescription> vk_vi_bindings;
			std::vector<VkPipelineColorBlendAttachmentState> vk_blend_attachment_states;
			std::vector<VkDynamicState> vk_dynamic_states;

			for (auto _sh : info.shaders)
			{
				auto sh = (ShaderPrivate*)_sh;
				for (auto& st : sh->stages)
				{
					VkPipelineShaderStageCreateInfo info;
					info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
					info.flags = 0;
					info.pNext = nullptr;
					info.pSpecializationInfo = nullptr;
					info.pName = st.entry_name.c_str();
					info.stage = to_enum(st.stage);
					info.module = sh->v;
					vk_stage_infos.push_back(info);
				}
			}

			auto vi_binding = 0;
			auto vi_location = 0;
			vk_vi_attributes.resize(info.vi_attribs.size());
			for (auto i = 0; i < info.vi_attribs.size(); i++)
			{
				auto& src = info.vi_attribs[i];
				auto& dst = vk_vi_attributes[i];
				dst.location = src.location;
				dst.binding = src.buffer_id;
				dst.offset = src.offset;
				dst.format = to_enum(src.format);
			}
			vk_vi_bindings.resize(info.vi_buffers.size());
			for (auto i = 0; i < info.vi_buffers.size(); i++)
			{
				auto& src = info.vi_buffers[i];
				auto& dst = vk_vi_bindings[i];
				dst.binding = src.id;
				dst.stride = src.stride;
				dst.inputRate = to_enum(src.rate);
			}

			VkPipelineVertexInputStateCreateInfo vertex_input_state;
			vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertex_input_state.pNext = nullptr;
			vertex_input_state.flags = 0;
			vertex_input_state.vertexBindingDescriptionCount = vk_vi_bindings.size();
			vertex_input_state.pVertexBindingDescriptions = vk_vi_bindings.empty() ? nullptr : vk_vi_bindings.data();
			vertex_input_state.vertexAttributeDescriptionCount = vk_vi_attributes.size();
			vertex_input_state.pVertexAttributeDescriptions = vk_vi_attributes.empty() ? nullptr : vk_vi_attributes.data();

			VkPipelineInputAssemblyStateCreateInfo assembly_state;
			assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			assembly_state.flags = 0;
			assembly_state.pNext = nullptr;
			assembly_state.topology = to_enum(info.primitive_topology);
			assembly_state.primitiveRestartEnable = VK_FALSE;

			VkPipelineTessellationStateCreateInfo tess_state;
			tess_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			tess_state.pNext = nullptr;
			tess_state.flags = 0;
			tess_state.patchControlPoints = info.patch_control_points;

			VkViewport viewport;
			viewport.width = (float)info.viewport_size.x();
			viewport.height = (float)info.viewport_size.y();
			viewport.minDepth = (float)0.0f;
			viewport.maxDepth = (float)1.0f;
			viewport.x = 0;
			viewport.y = 0;

			VkRect2D scissor;
			scissor.extent.width = info.viewport_size.x();
			scissor.extent.height = info.viewport_size.y();
			scissor.offset.x = 0;
			scissor.offset.y = 0;

			VkPipelineViewportStateCreateInfo viewport_state;
			viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewport_state.pNext = nullptr;
			viewport_state.flags = 0;
			viewport_state.viewportCount = 1;
			viewport_state.scissorCount = 1;
			viewport_state.pScissors = &scissor;
			viewport_state.pViewports = &viewport;

			VkPipelineRasterizationStateCreateInfo raster_state;
			raster_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			raster_state.pNext = nullptr;
			raster_state.flags = 0;
			raster_state.depthClampEnable = info.depth_clamp;
			raster_state.rasterizerDiscardEnable = VK_FALSE;
			raster_state.polygonMode = to_enum(info.polygon_mode);
			raster_state.cullMode = to_enum(info.cull_mode);
			raster_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			raster_state.depthBiasEnable = VK_FALSE;
			raster_state.depthBiasConstantFactor = 0.f;
			raster_state.depthBiasClamp = 0.f;
			raster_state.depthBiasSlopeFactor = 0.f;
			raster_state.lineWidth = 1.f;

			VkPipelineMultisampleStateCreateInfo multisample_state;
			multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisample_state.flags = 0;
			multisample_state.pNext = nullptr;
			multisample_state.rasterizationSamples = to_enum(info.sample_count);
			multisample_state.sampleShadingEnable = VK_FALSE;
			multisample_state.minSampleShading = 0.f;
			multisample_state.pSampleMask = nullptr;
			multisample_state.alphaToCoverageEnable = VK_FALSE;
			multisample_state.alphaToOneEnable = VK_FALSE;

			VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
			depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depth_stencil_state.flags = 0;
			depth_stencil_state.pNext = nullptr;
			depth_stencil_state.depthTestEnable = info.depth_test;
			depth_stencil_state.depthWriteEnable = info.depth_write;
			depth_stencil_state.depthCompareOp = to_enum(info.depth_compare_op);
			depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
			depth_stencil_state.minDepthBounds = 0;
			depth_stencil_state.maxDepthBounds = 0;
			depth_stencil_state.stencilTestEnable = VK_FALSE;
			depth_stencil_state.front = {};
			depth_stencil_state.back = {};

			for (auto& a : info.blend_states)
			{
				VkPipelineColorBlendAttachmentState s;
				s.blendEnable = a.blend_enable;
				s.srcColorBlendFactor = to_enum(a.blend_src_color);
				s.dstColorBlendFactor = to_enum(a.blend_dst_color);
				s.colorBlendOp = VK_BLEND_OP_ADD;
				s.srcAlphaBlendFactor = to_enum(a.blend_src_alpha);
				s.dstAlphaBlendFactor = to_enum(a.blend_dst_alpha);
				s.alphaBlendOp = VK_BLEND_OP_ADD;
				s.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				vk_blend_attachment_states.push_back(s);
			}

			VkPipelineColorBlendStateCreateInfo blend_state;
			blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			blend_state.flags = 0;
			blend_state.pNext = nullptr;
			blend_state.blendConstants[0] = 0.f;
			blend_state.blendConstants[1] = 0.f;
			blend_state.blendConstants[2] = 0.f;
			blend_state.blendConstants[3] = 0.f;
			blend_state.logicOpEnable = VK_FALSE;
			blend_state.logicOp = VK_LOGIC_OP_COPY;
			blend_state.attachmentCount = vk_blend_attachment_states.size();
			blend_state.pAttachments = vk_blend_attachment_states.data();

			for (auto& s : info.dynamic_states)
				vk_dynamic_states.push_back(to_enum(s));
			if (info.viewport_size.x() == 0 && info.viewport_size.y() == 0)
			{
				if (std::find(vk_dynamic_states.begin(), vk_dynamic_states.end(), VK_DYNAMIC_STATE_VIEWPORT) == vk_dynamic_states.end())
					vk_dynamic_states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
				if (std::find(vk_dynamic_states.begin(), vk_dynamic_states.end(), VK_DYNAMIC_STATE_SCISSOR) == vk_dynamic_states.end())
					vk_dynamic_states.push_back(VK_DYNAMIC_STATE_SCISSOR);
			}

			VkPipelineDynamicStateCreateInfo dynamic_state;
			dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamic_state.pNext = nullptr;
			dynamic_state.flags = 0;
			dynamic_state.dynamicStateCount = vk_dynamic_states.size();
			dynamic_state.pDynamicStates = vk_dynamic_states.data();

			VkGraphicsPipelineCreateInfo pipeline_info;
			pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipeline_info.pNext = nullptr;
			pipeline_info.flags = 0;
			pipeline_info.stageCount = vk_stage_infos.size();
			pipeline_info.pStages = vk_stage_infos.data();
			pipeline_info.pVertexInputState = &vertex_input_state;
			pipeline_info.pInputAssemblyState = &assembly_state;
			pipeline_info.pTessellationState = info.patch_control_points > 0 ? &tess_state : nullptr;
			pipeline_info.pViewportState = &viewport_state;
			pipeline_info.pRasterizationState = &raster_state;
			pipeline_info.pMultisampleState = &multisample_state;
			pipeline_info.pDepthStencilState = &depth_stencil_state;
			pipeline_info.pColorBlendState = &blend_state;
			pipeline_info.pDynamicState = vk_dynamic_states.size() ? &dynamic_state : nullptr;
			pipeline_info.layout = pll->v;
			pipeline_info.renderPass = info.renderpass ? ((RenderpassPrivate*)info.renderpass)->v : nullptr;
			pipeline_info.subpass = info.subpass_idx;
			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;

			chk_res(vkCreateGraphicsPipelines(d->v, 0, 1, &pipeline_info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif

			type = PipelineGraphics;
		}

		PipelinePrivate::PipelinePrivate(Device* _d, const ComputePipelineInfo& info)
		{
			d = (DevicePrivate*)_d;
			pll = (PipelinelayoutPrivate*)info.layout;

#if defined(FLAME_VULKAN)
			VkPipelineShaderStageCreateInfo vk_stage_info;
			{
				vk_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				vk_stage_info.flags = 0;
				vk_stage_info.pNext = nullptr;
				vk_stage_info.pSpecializationInfo = nullptr;
				vk_stage_info.pName = "main";
				vk_stage_info.stage = to_enum(ShaderStageComp);
				vk_stage_info.module = ((ShaderPrivate*)info.compute_shader)->v;
			}

			VkComputePipelineCreateInfo pipeline_info;
			pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			pipeline_info.flags = 0;
			pipeline_info.pNext = nullptr;
			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;
			pipeline_info.layout = pll->v;
			pipeline_info.stage = vk_stage_info;

			chk_res(vkCreateComputePipelines(d->v, 0, 1, &pipeline_info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif

			type = PipelineCompute;
		}

		PipelinePrivate::~PipelinePrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyPipeline(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Pipeline* Pipeline::create(Device* d, const GraphicsPipelineInfo& info)
		{
			return new PipelinePrivate(d, info);
		}

		Pipeline* Pipeline::create(Device* d, const ComputePipelineInfo& info)
		{
			return new PipelinePrivate(d, info);
		}

		void Pipeline::destroy(Pipeline* p)
		{
			delete (PipelinePrivate*)p;
		}

		struct PipelineFullscreen$
		{
			AttributeP<void> renderpass$i;
			AttributeV<uint> subpass_idx$i;
			AttributeP<std::vector<void*>> shaders$i;
			AttributeP<void> layout$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (renderpass$i.frame > out$o.frame || subpass_idx$i.frame > out$o.frame || shaders$i.frame > out$o.frame ||layout$i.frame > out$o.frame)
				{
					if (out$o.v)
						Pipeline::destroy((Pipeline*)out$o.v);
					auto d = (Device*)bp_environment().graphics_device;
					if (d && renderpass$i.v && ((Renderpass*)renderpass$i.v)->subpass_count() > subpass_idx$i.v && shaders$i.v && !shaders$i.v->empty() && layout$i.v)
					{
						GraphicsPipelineInfo info((Pipelinelayout*)layout$i.v, (Renderpass*)renderpass$i.v, subpass_idx$i.v);
						info.shaders = *shaders$i.v;
						out$o.v = Pipeline::create(d, info);
					}
					else
					{
						printf("cannot create pipelinefullscreen\n");

						out$o.v = nullptr;
					}
					out$o.frame = maxN(renderpass$i.frame, subpass_idx$i.frame, shaders$i.frame, layout$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~PipelineFullscreen$()
			{
				if (out$o.v)
					Pipeline::destroy((Pipeline*)out$o.v);
			}

		};
	}
}
