#include "device_private.h"
#include "renderpass_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "shader_private.h"

namespace flame
{
	namespace graphics
	{
		DescriptorPoolPrivate::DescriptorPoolPrivate(DevicePrivate* d) :
			device(d)
		{
			VkDescriptorPoolSize descriptorPoolSizes[] = {
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 128},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 32},
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128},
				{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 32},
			};

			VkDescriptorPoolCreateInfo descriptorPoolInfo;
			descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			descriptorPoolInfo.pNext = nullptr;
			descriptorPoolInfo.poolSizeCount = size(descriptorPoolSizes);
			descriptorPoolInfo.pPoolSizes = descriptorPoolSizes;
			descriptorPoolInfo.maxSets = 64;
			chk_res(vkCreateDescriptorPool(device->vk_device, &descriptorPoolInfo, nullptr, &vk_descriptor_pool));
		}

		DescriptorPoolPrivate::~DescriptorPoolPrivate()
		{
			vkDestroyDescriptorPool(device->vk_device, vk_descriptor_pool, nullptr);
		}

		DescriptorPool* DescriptorPool::create(Device* d)
		{
			return new DescriptorPoolPrivate((DevicePrivate*)d);
		}

		DescriptorBindingPrivate::DescriptorBindingPrivate(uint index, const DescriptorBindingInfo& info) :
			index(index),
			type(info.type),
			count(info.count),
			name(info.name)
		{
		}

		DescriptorSetLayoutPrivate::DescriptorSetLayoutPrivate(DevicePrivate* d, std::span<const DescriptorBindingInfo> _bindings) :
			device(d)
		{
			std::vector<VkDescriptorSetLayoutBinding> vk_bindings(_bindings.size());
			bindings.resize(_bindings.size());
			for (auto i = 0; i < bindings.size(); i++)
			{
				auto& src = _bindings[i];
				auto& dst = vk_bindings[i];

				dst.binding = i;
				dst.descriptorType = to_backend(src.type);
				dst.descriptorCount = src.count;
				dst.stageFlags = to_backend_flags<ShaderStageFlags>(ShaderStageAll);
				dst.pImmutableSamplers = nullptr;

				bindings[i].reset(new DescriptorBindingPrivate(i, src));
			}

			VkDescriptorSetLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.bindingCount = vk_bindings.size();
			info.pBindings = vk_bindings.data();

			chk_res(vkCreateDescriptorSetLayout(device->vk_device, &info, nullptr, &vk_descriptor_set_layout));
		}

		DescriptorSetLayoutPrivate::~DescriptorSetLayoutPrivate()
		{
			vkDestroyDescriptorSetLayout(device->vk_device, vk_descriptor_set_layout, nullptr);
		}

		DescriptorSetLayout* DescriptorSetLayout::create(Device* d, uint binding_count, const DescriptorBindingInfo* bindings)
		{
			return new DescriptorSetLayoutPrivate((DevicePrivate*)d, { bindings, binding_count });
		}

		DescriptorSetPrivate::DescriptorSetPrivate(DescriptorPoolPrivate* p, DescriptorSetLayoutPrivate* l) :
			descriptor_pool(p),
			descriptor_layout(l)
		{
			VkDescriptorSetAllocateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.pNext = nullptr;
			info.descriptorPool = p->vk_descriptor_pool;
			info.descriptorSetCount = 1;
			info.pSetLayouts = &l->vk_descriptor_set_layout;

			chk_res(vkAllocateDescriptorSets(p->device->vk_device, &info, &vk_descriptor_set));
		}

		DescriptorSetPrivate::~DescriptorSetPrivate()
		{
			chk_res(vkFreeDescriptorSets(descriptor_pool->device->vk_device, descriptor_pool->vk_descriptor_pool, 1, &vk_descriptor_set));
		}

		void DescriptorSetPrivate::set_buffer(uint binding, uint index, BufferPrivate* b, uint offset, uint range)
		{
			VkDescriptorBufferInfo i;
			i.buffer = b->vk_buffer;
			i.offset = offset;
			i.range = range == 0 ? b->size : range;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = vk_descriptor_set;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = to_backend(descriptor_layout->bindings[binding]->type);
			write.descriptorCount = 1;
			write.pBufferInfo = &i;
			write.pImageInfo = nullptr;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(descriptor_pool->device->vk_device, 1, &write, 0, nullptr);
		}

		void DescriptorSetPrivate::set_image(uint binding, uint index, ImageViewPrivate* iv, SamplerPrivate* sampler)
		{
			VkDescriptorImageInfo i;
			i.imageView = iv->vk_image_view;
			i.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			i.sampler = sampler->vk_sampler;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = vk_descriptor_set;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = to_backend(descriptor_layout->bindings[binding]->type);
			write.descriptorCount = 1;
			write.pBufferInfo = nullptr;
			write.pImageInfo = &i;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(descriptor_pool->device->vk_device, 1, &write, 0, nullptr);
		}

		DescriptorSet* DescriptorSet::create(DescriptorPool* p, DescriptorSetLayout* l)
		{
			return new DescriptorSetPrivate((DescriptorPoolPrivate*)p, (DescriptorSetLayoutPrivate*)l);
		}

		PipelineLayoutPrivate::PipelineLayoutPrivate(DevicePrivate* d, std::span<DescriptorSetLayoutPrivate*> _descriptor_layouts, uint _push_constant_size) :
			device(d),
			push_cconstant_size(_push_constant_size)
		{
			std::vector<VkDescriptorSetLayout> raw_descriptor_set_layouts;
			raw_descriptor_set_layouts.resize(_descriptor_layouts.size());
			for (auto i = 0; i < _descriptor_layouts.size(); i++)
				raw_descriptor_set_layouts[i] = _descriptor_layouts[i]->vk_descriptor_set_layout;

			VkPushConstantRange pushconstant_range;
			pushconstant_range.offset = 0;
			pushconstant_range.size = push_cconstant_size;
			pushconstant_range.stageFlags = to_backend_flags<ShaderStageFlags>(ShaderStageAll);

			VkPipelineLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.setLayoutCount = raw_descriptor_set_layouts.size();
			info.pSetLayouts = raw_descriptor_set_layouts.data();
			info.pushConstantRangeCount = push_cconstant_size > 0 ? 1 : 0;
			info.pPushConstantRanges = push_cconstant_size > 0 ? &pushconstant_range : nullptr;

			chk_res(vkCreatePipelineLayout(device->vk_device, &info, nullptr, &vk_pipeline_layout));

			descriptor_layouts.resize(_descriptor_layouts.size());
			for (auto i = 0; i < descriptor_layouts.size(); i++)
				descriptor_layouts[i] = (DescriptorSetLayoutPrivate*)_descriptor_layouts[i];
		}

		PipelineLayoutPrivate::~PipelineLayoutPrivate()
		{
			vkDestroyPipelineLayout(device->vk_device, vk_pipeline_layout, nullptr);
		}

		PipelineLayout* PipelineLayout::create(Device* d, uint descriptorlayout_count, DescriptorSetLayout* const* descriptorlayouts, uint push_constant_size)
		{
			return new PipelineLayoutPrivate((DevicePrivate*)d, { (DescriptorSetLayoutPrivate**)descriptorlayouts, descriptorlayout_count }, push_constant_size);
		}

		ShaderPrivate::ShaderPrivate(DevicePrivate* d, const std::filesystem::path& _filename, const std::string& _prefix) :
			filename(_filename),
			prefix(_prefix),
			device(d)
		{
			filename.make_preferred();
			type = shader_stage_from_ext(_filename.extension());

			auto hash = std::hash<std::wstring>()(filename) ^ std::hash<std::string>()(prefix);
			auto str_hash = std::to_wstring(hash);

			auto path = filename;
			if (!std::filesystem::exists(path))
			{
				auto engine_path = getenv("FLAME_PATH");
				if (engine_path)
					path = std::filesystem::path(engine_path) / L"assets/shaders" / path;
				if (!std::filesystem::exists(path))
				{
					wprintf(L"cannot find shader: %s\n", filename.c_str());
					return;
				}
			}

			auto spv_path = path;
			spv_path += L".";
			spv_path += str_hash;
			//auto res_path = spv_path;
			//spv_path += L".spv";
			//res_path += L".res";

			if (std::filesystem::exists(path) && (!std::filesystem::exists(spv_path) || std::filesystem::last_write_time(spv_path) < std::filesystem::last_write_time(path)))
			{

				auto vk_sdk_path = getenv("VK_SDK_PATH");
				if (vk_sdk_path)
				{
					if (std::filesystem::exists(spv_path))
						std::filesystem::remove(spv_path);

					std::filesystem::path glslc_path = vk_sdk_path;
					glslc_path /= L"Bin/glslc.exe";

					std::wstring command_line(L" " + path.wstring() + L" -o" + spv_path.wstring());
					auto defines = SUS::split(prefix);
					for (auto& d : defines)
						command_line += L" -D" + s2w(d);

					wprintf(L"compiling shader: %s\n", path.c_str());

					std::string output;
					exec(glslc_path.c_str(), (wchar_t*)command_line.c_str(), &output);
					if (!std::filesystem::exists(spv_path))
					{
						printf("%s\n", output.c_str());
						assert(0);
					}
				}
				else
				{
					printf("cannot find vk sdk\n");
					assert(0);
				}

				//{
				//	pugi::xml_document doc;
				//	auto root = doc.append_child("res");
				//	if (!r.inputs.empty())
				//	{
				//		auto n = root.append_child("inputs");
				//		for (auto& i : r.inputs)
				//		{
				//			auto nn = n.append_child("input");
				//			nn.append_attribute("name").set_value(i.name.c_str());
				//			nn.append_attribute("type").set_value(i.type.c_str());
				//		}
				//	}
				//	if (!r.outputs.empty())
				//	{
				//		auto n = root.append_child("outputs");
				//		for (auto& o : r.outputs)
				//		{
				//			auto nn = n.append_child("input");
				//			nn.append_attribute("name").set_value(o.name.c_str());
				//			nn.append_attribute("type").set_value(o.type.c_str());
				//		}

				//	}

				//	doc.save_file(res_path.c_str());
				//}
			}
			//else
			//{
			//	pugi::xml_document doc;
			//	pugi::xml_node root;
			//	if (!doc.load_file(res_path.c_str()) || (root = doc.first_child()).name() != std::string("res"))
			//		assert(0);
			//	else
			//	{
			//		for (auto& n : root.child("inputs"))
			//		{
			//			ShaderInOut i;
			//			i.name = n.attribute("name").value();
			//			i.type = n.attribute("type").value();
			//			r.inputs.push_back(i);
			//		}
			//		for (auto& n : root.child("outputs"))
			//		{
			//			ShaderInOut o;
			//			o.name = n.attribute("name").value();
			//			o.type = n.attribute("type").value();
			//			r.outputs.push_back(o);
			//		}
			//	}
			//}

			auto spv_file = get_file_content(spv_path);
			if (spv_file.empty())
			{
				assert(0);
				return;
			}

			VkShaderModuleCreateInfo shader_info;
			shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shader_info.flags = 0;
			shader_info.pNext = nullptr;
			shader_info.codeSize = spv_file.size();
			shader_info.pCode = (uint*)spv_file.data();
			chk_res(vkCreateShaderModule(d->vk_device, &shader_info, nullptr, &vk_module));
		}

		ShaderPrivate::~ShaderPrivate()
		{
			if (vk_module)
				vkDestroyShaderModule(device->vk_device, vk_module, nullptr);
		}

		PipelinePrivate::PipelinePrivate(DevicePrivate* d, std::span<ShaderPrivate*> _shaders, PipelineLayoutPrivate* pll,
			RenderpassPrivate* rp, uint subpass_idx, VertexInfo* vi, const Vec2u& vp, RasterInfo* raster, SampleCount sc, 
			DepthInfo* depth, std::span<const BlendOption> blend_options, std::span<const uint> dynamic_states) :
			device(d),
			pipeline_layout(pll)
		{
			type = PipelineGraphics;

			std::vector<VkPipelineShaderStageCreateInfo> vk_stage_infos;
			std::vector<VkVertexInputAttributeDescription> vk_vi_attributes;
			std::vector<VkVertexInputBindingDescription> vk_vi_bindings;
			std::vector<VkPipelineColorBlendAttachmentState> vk_blend_attachment_states;
			std::vector<VkDynamicState> vk_dynamic_states;

			vk_stage_infos.resize(_shaders.size());
			for (auto i = 0; i < _shaders.size(); i++)
			{
				auto& src = _shaders[i];
				auto& dst = vk_stage_infos[i];
				dst.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				dst.flags = 0;
				dst.pNext = nullptr;
				dst.pSpecializationInfo = nullptr;
				dst.pName = "main";
				dst.stage = to_backend(src->type);
				dst.module = src->vk_module;
			}

			if (vi)
			{
				vk_vi_bindings.resize(vi->buffers_count);
				for (auto i = 0; i < vk_vi_bindings.size(); i++)
				{
					auto& src = vi->buffers[i];
					auto& dst = vk_vi_bindings[i];
					dst.binding = i;
					for (auto j = 0; j < src.attributes_count; j++)
					{
						auto& _src = src.attributes[j];
						VkVertexInputAttributeDescription _dst;
						_dst.location = _src.location;
						_dst.binding = i;
						_dst.offset = dst.stride;
						dst.stride += format_size(_src.format);
						_dst.format = to_backend(_src.format);
						vk_vi_attributes.push_back(_dst);
					}
					dst.inputRate = to_backend(src.rate);
				}
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
			assembly_state.topology = to_backend(vi ? vi->primitive_topology : PrimitiveTopologyTriangleList);
			assembly_state.primitiveRestartEnable = VK_FALSE;

			VkPipelineTessellationStateCreateInfo tess_state;
			tess_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			tess_state.pNext = nullptr;
			tess_state.flags = 0;
			tess_state.patchControlPoints = vi ? vi->patch_control_points : 0;

			VkViewport viewport;
			viewport.width = (float)vp.x();
			viewport.height = (float)vp.y();
			viewport.minDepth = (float)0.0f;
			viewport.maxDepth = (float)1.0f;
			viewport.x = 0;
			viewport.y = 0;

			VkRect2D scissor;
			scissor.extent.width = vp.x();
			scissor.extent.height = vp.y();
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
			raster_state.depthClampEnable = raster ? raster->depth_clamp : false;
			raster_state.rasterizerDiscardEnable = VK_FALSE;
			raster_state.polygonMode = to_backend(raster ? raster->polygon_mode : PolygonModeFill);
			raster_state.cullMode = to_backend(raster ? raster->cull_mode : CullModeNone);
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
			multisample_state.rasterizationSamples = to_backend(sc);
			multisample_state.sampleShadingEnable = VK_FALSE;
			multisample_state.minSampleShading = 0.f;
			multisample_state.pSampleMask = nullptr;
			multisample_state.alphaToCoverageEnable = VK_FALSE;
			multisample_state.alphaToOneEnable = VK_FALSE;

			VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
			depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depth_stencil_state.flags = 0;
			depth_stencil_state.pNext = nullptr;
			depth_stencil_state.depthTestEnable = depth ? depth->test : false;
			depth_stencil_state.depthWriteEnable = depth ? depth->write : false;
			depth_stencil_state.depthCompareOp = to_backend(depth ? depth->compare_op : CompareOpLess);
			depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
			depth_stencil_state.minDepthBounds = 0;
			depth_stencil_state.maxDepthBounds = 0;
			depth_stencil_state.stencilTestEnable = VK_FALSE;
			depth_stencil_state.front = {};
			depth_stencil_state.back = {};

			vk_blend_attachment_states.resize(rp->subpasses[subpass_idx]->color_attachments.size());
			for (auto& a : vk_blend_attachment_states)
			{
				a = {};
				a.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			}
			for (auto i = 0; i < blend_options.size(); i++)
			{
				auto& src = blend_options[i];
				auto& dst = vk_blend_attachment_states[i];
				dst.blendEnable = src.enable;
				dst.srcColorBlendFactor = to_backend(src.src_color);
				dst.dstColorBlendFactor = to_backend(src.dst_color);
				dst.colorBlendOp = VK_BLEND_OP_ADD;
				dst.srcAlphaBlendFactor = to_backend(src.src_alpha);
				dst.dstAlphaBlendFactor = to_backend(src.dst_alpha);
				dst.alphaBlendOp = VK_BLEND_OP_ADD;
				dst.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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

			for (auto i = 0; i < dynamic_states.size(); i++)
				vk_dynamic_states.push_back(to_backend((DynamicState)dynamic_states[i]));
			if (vp.x() == 0 && vp.y() == 0)
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
			pipeline_info.pTessellationState = tess_state.patchControlPoints > 0 ? &tess_state : nullptr;
			pipeline_info.pViewportState = &viewport_state;
			pipeline_info.pRasterizationState = &raster_state;
			pipeline_info.pMultisampleState = &multisample_state;
			pipeline_info.pDepthStencilState = &depth_stencil_state;
			pipeline_info.pColorBlendState = &blend_state;
			pipeline_info.pDynamicState = vk_dynamic_states.size() ? &dynamic_state : nullptr;
			pipeline_info.layout = pll->vk_pipeline_layout;
			pipeline_info.renderPass = rp ? rp->vk_renderpass : nullptr;
			pipeline_info.subpass = subpass_idx;
			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;

			chk_res(vkCreateGraphicsPipelines(d->vk_device, 0, 1, &pipeline_info, nullptr, &vk_pipeline));
		}

		PipelinePrivate::PipelinePrivate(DevicePrivate* d, ShaderPrivate* compute_shader, PipelineLayoutPrivate* pll) :
			device(d),
			pipeline_layout(pll)
		{
			type = PipelineCompute;

			VkComputePipelineCreateInfo pipeline_info;
			pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			pipeline_info.pNext = nullptr;
			pipeline_info.flags = 0;

			pipeline_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			pipeline_info.stage.flags = 0;
			pipeline_info.stage.pNext = nullptr;
			pipeline_info.stage.pSpecializationInfo = nullptr;
			pipeline_info.stage.pName = "main";
			pipeline_info.stage.stage = to_backend(ShaderStageComp);
			pipeline_info.stage.module = compute_shader->vk_module;

			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;
			pipeline_info.layout = pll->vk_pipeline_layout;

			chk_res(vkCreateComputePipelines(device->vk_device, 0, 1, &pipeline_info, nullptr, &vk_pipeline));
		}

		PipelinePrivate::~PipelinePrivate()
		{
			vkDestroyPipeline(device->vk_device, vk_pipeline, nullptr);
		}

		PipelinePrivate* PipelinePrivate::create(DevicePrivate* d, std::span<ShaderPrivate*> shaders,
			PipelineLayoutPrivate* pll, Renderpass* rp, uint subpass_idx, VertexInfo* vi, const Vec2u& vp, RasterInfo* raster, SampleCount sc, 
			DepthInfo* depth, std::span<const BlendOption> blend_options, std::span<const uint> dynamic_states)
		{
			auto has_vert_stage = false;
			auto tess_stage_count = 0;
			for (auto& s : shaders)
			{
				for (auto& ss : shaders)
				{
					if (ss != s && (ss->filename == s->filename || ss->type == s->type))
						return nullptr;
				}
				if (s->type == ShaderStageComp)
					return nullptr;
				if (s->type == ShaderStageVert)
					has_vert_stage = true;
				if (s->type == ShaderStageTesc || s->type == ShaderStageTese)
					tess_stage_count++;
			}
			if (!has_vert_stage || (tess_stage_count != 0 && tess_stage_count != 2))
				return nullptr;

			return new PipelinePrivate(d, shaders, pll, (RenderpassPrivate*)rp, subpass_idx, vi, vp, raster, sc, depth, blend_options, dynamic_states);
		}

		PipelinePrivate* PipelinePrivate::create(DevicePrivate* d, ShaderPrivate* compute_shader, PipelineLayoutPrivate* pll)
		{
			if (compute_shader->type != ShaderStageComp)
				return nullptr;

			return new PipelinePrivate(d, compute_shader, pll);
		}

		Pipeline* create(Device* d, uint shaders_count,
			Shader* const* shaders, PipelineLayout* pll, Renderpass* rp, uint subpass_idx,
			VertexInfo* vi, const Vec2u& vp, RasterInfo* raster, SampleCount sc, DepthInfo* depth,
			uint blend_options_count, const BlendOption* blend_options,
			uint dynamic_states_count, const uint* dynamic_states)
		{
			return PipelinePrivate::create((DevicePrivate*)d,  
				{ (ShaderPrivate**)shaders, shaders_count }, (PipelineLayoutPrivate*)pll, rp, subpass_idx, vi, vp, raster, sc, depth, 
				{ blend_options , blend_options_count },
				{ dynamic_states , dynamic_states_count });
		}

		Pipeline* create(Device* d, Shader* compute_shader, PipelineLayout* pll)
		{
			return PipelinePrivate::create((DevicePrivate*)d, (ShaderPrivate*)compute_shader, (PipelineLayoutPrivate*)pll);
		}
	}
}
