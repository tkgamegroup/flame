#include "device_private.h"
#include "shader_private.h"
#include "renderpass_private.h"
#include "descriptor_private.h"
#include "pipeline_private.h"

namespace flame
{
	namespace graphics
	{
		PipelinelayoutPrivate::PipelinelayoutPrivate(Device* _d, const std::vector<Descriptorsetlayout*>& _setlayouts, const std::vector<PushconstantInfo>& _pushconstants)
		{
			d = (DevicePrivate*)_d;
			dsls.resize(_setlayouts.size());
			for (auto i = 0; i < dsls.size(); i++)
				dsls[i] = (DescriptorsetlayoutPrivate*)_setlayouts[i];
			pcs = _pushconstants;

#if defined(FLAME_VULKAN)
			std::vector<VkDescriptorSetLayout> vk_descriptorsetlayouts;
			vk_descriptorsetlayouts.resize(dsls.size());
			for (auto i = 0; i < vk_descriptorsetlayouts.size(); i++)
				vk_descriptorsetlayouts[i] = ((DescriptorsetlayoutPrivate*)dsls[i])->v;

			std::vector<VkPushConstantRange> vk_pushconstants;
			vk_pushconstants.resize(pcs.size());
			for (auto i = 0; i < vk_pushconstants.size(); i++)
			{
				vk_pushconstants[i].offset = pcs[i].offset;
				vk_pushconstants[i].size = pcs[i].size;
				vk_pushconstants[i].stageFlags = to_flags(ShaderAll);
			}

			VkPipelineLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.setLayoutCount = vk_descriptorsetlayouts.size();
			info.pSetLayouts = vk_descriptorsetlayouts.data();
			info.pushConstantRangeCount = vk_pushconstants.size();
			info.pPushConstantRanges = vk_pushconstants.data();

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

		Descriptorsetlayout* Pipelinelayout::dsl(uint index) const
		{
			return ((PipelinelayoutPrivate*)this)->dsls[index];
		}

		Pipelinelayout* Pipelinelayout::create(Device* d, const std::vector<Descriptorsetlayout*>& setlayouts, const std::vector<PushconstantInfo>& pushconstants)
		{
			return new PipelinelayoutPrivate(d, setlayouts, pushconstants);
		}

		void Pipelinelayout::destroy(Pipelinelayout* l)
		{
			delete (PipelinelayoutPrivate*)l;
		}

		void PipelinePrivate::init()
		{
			vert_shader = nullptr;
			tesc_shader = nullptr;
			tese_shader = nullptr;
			geom_shader = nullptr;
			frag_shader = nullptr;
			comp_shader = nullptr;
		}

		PipelinePrivate::PipelinePrivate(Device* _d, const GraphicsPipelineInfo& info)
		{
			init();

			d = (DevicePrivate*)_d;

#if defined(FLAME_VULKAN)
			std::vector<VkVertexInputBindingDescription> vk_vi_bindings;
			std::vector<VkVertexInputAttributeDescription> vk_vi_attributes;
			std::vector<VkPipelineColorBlendAttachmentState> vk_blend_attachment_states;
			std::vector<VkDynamicState> vk_dynamic_states;

			for (auto& s : info.shaders)
				add_shader(s);

			auto vk_stage_infos = process_stages();

			auto vi_binding = 0;
			auto vi_location = 0;
			for (auto& b : info.vi_buffers)
			{
				VkVertexInputBindingDescription binding;
				binding.binding = vi_binding;
				binding.stride = 0;
				binding.inputRate = to_enum(b.rate);

				for (auto& a : b.attributes)
				{
					VkVertexInputAttributeDescription attribute;
					attribute.location = vi_location;
					attribute.binding = vi_binding;
					attribute.offset = binding.stride;
					attribute.format = to_enum(a);

					switch (a)
					{
					case Format_R8G8B8A8_UNORM:
						binding.stride += 4;
						break;
					case Format_R32_SFLOAT:
						binding.stride += 4;
						break;
					case Format_R32G32_SFLOAT:
						binding.stride += 8;
						break;
					case Format_R32G32B32_SFLOAT:
						binding.stride += 12;
						break;
					case Format_R32G32B32A32_SFLOAT:
						binding.stride += 16;
						break;
					default:
						assert(0);
					}

					vi_location++;
					vk_vi_attributes.push_back(attribute);
				}

				vi_binding++;
				vk_vi_bindings.push_back(binding);
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
			pipeline_info.layout = layout->v;
			pipeline_info.renderPass = info.renderpass ? ((RenderpassPrivate*)info.renderpass)->v : nullptr;
			pipeline_info.subpass = info.subpass_index;
			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;

			chk_res(vkCreateGraphicsPipelines(d->v, 0, 1, &pipeline_info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif

			type = PipelineGraphics;
		}

		PipelinePrivate::PipelinePrivate(Device* _d, const ShaderInfo& compute_shader)
		{
			init();

			d = (DevicePrivate*)_d;

			add_shader(compute_shader);

#if defined(FLAME_VULKAN)
			auto vk_stage_infos = process_stages();
			assert(vk_stage_infos.size() == 1);

			VkComputePipelineCreateInfo pipeline_info;
			pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			pipeline_info.flags = 0;
			pipeline_info.pNext = nullptr;
			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;
			pipeline_info.layout = layout->v;
			pipeline_info.stage = vk_stage_infos[0];

			chk_res(vkCreateComputePipelines(d->v, 0, 1, &pipeline_info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif

			type = PipelineCompute;
		}

		PipelinePrivate::~PipelinePrivate()
		{
			if (vert_shader)
				Shader::destroy(vert_shader);
			if (tesc_shader)
				Shader::destroy(tesc_shader);
			if (tese_shader)
				Shader::destroy(tese_shader);
			if (geom_shader)
				Shader::destroy(geom_shader);
			if (frag_shader)
				Shader::destroy(frag_shader);
			if (comp_shader)
				Shader::destroy(comp_shader);
			for (auto& l : dsls)
				Descriptorsetlayout::destroy(l);
			Pipelinelayout::destroy(layout);
#if defined(FLAME_VULKAN)
			vkDestroyPipeline(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void PipelinePrivate::add_shader(const ShaderInfo& info)
		{
			auto s = Shader::create(d, info.filename, info.prefix);
			switch (s->type)
			{
			case ShaderVert:
				vert_shader = (ShaderPrivate*)s;
				break;
			case ShaderTesc:
				tesc_shader = (ShaderPrivate*)s;
				break;
			case ShaderTese:
				tese_shader = (ShaderPrivate*)s;
				break;
			case ShaderGeom:
				geom_shader = (ShaderPrivate*)s;
				break;
			case ShaderFrag:
				frag_shader = (ShaderPrivate*)s;
				break;
			case ShaderComp:
				comp_shader = (ShaderPrivate*)s;
				break;
			default:
				assert(0);
			}
		}

#if defined(FLAME_VULKAN)
		static void process_stage(ShaderPrivate* s, std::vector<VkPipelineShaderStageCreateInfo>& stage_infos, std::vector<std::vector<Descriptorsetlayout::Binding>>& sets, std::vector<PushconstantInfo>& pcs)
		{
			VkPipelineShaderStageCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.pSpecializationInfo = nullptr;
			info.pName = "main";
			info.stage = to_enum(s->type);
			info.module = s->v;
			stage_infos.push_back(info);

			for (auto& r : s->resources)
			{
				if (r->type == ShaderResourcePushConstant)
				{
					auto same = false;
					for (auto& p : pcs)
					{
						if (p.offset == r->var.offset && p.size == r->var.size)
						{
							same = true;
							break;
						}
					}
					if (!same)
					{
						PushconstantInfo pc;
						pc.offset = r->var.offset;
						pc.size = r->var.size;
						pcs.push_back(pc);
					}
				}
				else
				{
					if (r->set + 1 > sets.size())
						sets.resize(r->set + 1);
					Descriptorsetlayout::Binding b;
					b.binding = r->binding;
					b.type = r->type;
					b.count = r->var.count;
					sets[r->set].push_back(b);
				}
			}
		}

		std::vector<VkPipelineShaderStageCreateInfo> PipelinePrivate::process_stages()
		{
			std::vector<VkPipelineShaderStageCreateInfo> stage_infos;
			std::vector<std::vector<Descriptorsetlayout::Binding>> sets;
			std::vector<PushconstantInfo> pcs;

			if (vert_shader)
				process_stage(vert_shader, stage_infos, sets, pcs);
			if (tesc_shader)
				process_stage(tesc_shader, stage_infos, sets, pcs);
			if (tese_shader)
				process_stage(tese_shader, stage_infos, sets, pcs);
			if (geom_shader)
				process_stage(geom_shader, stage_infos, sets, pcs);
			if (frag_shader)
				process_stage(frag_shader, stage_infos, sets, pcs);
			if (comp_shader)
				process_stage(comp_shader, stage_infos, sets, pcs);

			for (auto& g : sets)
			{
				auto l = Descriptorsetlayout::create(d, g);
				dsls.push_back(l);
			}
			layout = (PipelinelayoutPrivate*)Pipelinelayout::create(d, dsls, pcs);

			return stage_infos;
		}
#elif defined(FLAME_D3D12)

#endif

		Pipelinelayout* Pipeline::layout() const
		{
			return ((PipelinePrivate*)this)->layout;
		}

		Pipeline* Pipeline::create(Device* d, const GraphicsPipelineInfo& info)
		{
			return new PipelinePrivate(d, info);
		}

		Pipeline* Pipeline::create(Device* d, const ShaderInfo& compute_shader)
		{
			return new PipelinePrivate(d, compute_shader);
		}

		void Pipeline::destroy(Pipeline* p)
		{
			delete (PipelinePrivate*)p;
		}

		struct Pipeline$
		{
			AttributeP<void> device$i;
			AttributeV<std::string> vert_shader$i;
			AttributeV<std::string> frag_shader$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{

			}

			FLAME_GRAPHICS_EXPORTS ~Pipeline$()
			{
				if (out$o.v)
					Pipeline::destroy((Pipeline*)out$o.v);
			}

		}bp_pipeline_unused;
	}
}
