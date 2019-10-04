#include "device_private.h"
#include "renderpass_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "shader_private.h"

#include <flame/foundation/serialize.h>
#include <flame/foundation/blueprint.h>

#if defined(FLAME_VULKAN)
#include <spirv_glsl.hpp>
#endif

#include <functional>

namespace flame
{
	namespace graphics
	{
		DescriptorpoolPrivate::DescriptorpoolPrivate(Device* _d) :
			d((DevicePrivate*)_d)
		{
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

		DescriptorlayoutPrivate::DescriptorlayoutPrivate(Device* _d, const std::vector<void*>& _bindings) :
			d((DevicePrivate*)_d)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
			for (auto i = 0; i < _bindings.size(); i++)
			{
				auto b = (DescriptorBinding*)_bindings[i];
				auto binding = b->binding;

				assert(binding < 64);
				assert(bindings_map.size() <= binding || bindings_map[binding].binding >= 64); // if the slot is empty?

				if (bindings_map.size() <= binding)
					bindings_map.resize(binding + 1);
				bindings_map[binding] = *b;

				VkDescriptorSetLayoutBinding vk_binding;
				vk_binding.binding = b->binding;
				vk_binding.descriptorType = to_enum(b->type);
				vk_binding.descriptorCount = b->count;
				vk_binding.stageFlags = to_flags(ShaderStageAll);
				vk_binding.pImmutableSamplers = nullptr;
				vk_bindings.push_back(vk_binding);
			}

			VkDescriptorSetLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.bindingCount = vk_bindings.size();
			info.pBindings = vk_bindings.data();

			chk_res(vkCreateDescriptorSetLayout(((DevicePrivate*)d)->v, &info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		DescriptorlayoutPrivate::~DescriptorlayoutPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyDescriptorSetLayout(((DevicePrivate*)d)->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		const DescriptorBinding& Descriptorlayout::get_binding(uint binding)
		{
			return ((DescriptorlayoutPrivate*)this)->bindings_map[binding];
		}

		Descriptorlayout* Descriptorlayout::create(Device* d, const std::vector<void*>& bindings)
		{
			return new DescriptorlayoutPrivate(d, bindings);
		}

		void Descriptorlayout::destroy(Descriptorlayout* l)
		{
			delete (DescriptorlayoutPrivate*)l;
		}

		struct DescriptorBinding$
		{
			AttributeV<uint> binding$i;
			AttributeE<DescriptorType$> type$i;
			AttributeV<uint> count$i;
			AttributeV<std::string> name$i;
			AttributeV<std::string> buffer_udt_name$i;

			AttributeV<DescriptorBinding> out$o;

			FLAME_GRAPHICS_EXPORTS DescriptorBinding$()
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
				if (name$i.frame > out$o.frame)
					out$o.v.name = name$i.v;
				if (buffer_udt_name$i.frame > out$o.frame)
				{
					if (!buffer_udt_name$i.v.empty())
					{
						out$o.v.buffer_udt = find_udt(bp_env().dbs, H(buffer_udt_name$i.v.c_str()));
						assert(out$o.v.buffer_udt);
					}
				}
				out$o.frame = maxN(binding$i.frame, type$i.frame, count$i.frame, name$i.frame, buffer_udt_name$i.frame);
			}

			FLAME_GRAPHICS_EXPORTS ~DescriptorBinding$()
			{
			}
		};

		struct Descriptorlayout$
		{
			AttributeP<std::vector<void*>> bindings$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (bindings$i.frame > out$o.frame)
				{
					if (out$o.v)
						Descriptorlayout::destroy((Descriptorlayout*)out$o.v);
					auto d = (Device*)bp_env().graphics_device;
					if (d)
						out$o.v = Descriptorlayout::create(d, get_attribute_vec(bindings$i));
					else
					{
						printf("cannot create descriptorsetlayout\n");

						out$o.v = nullptr;
					}
					out$o.frame = bindings$i.frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Descriptorlayout$()
			{
				if (out$o.v)
					Descriptorlayout::destroy((Descriptorlayout*)out$o.v);
			}
		};

		DescriptorsetPrivate::DescriptorsetPrivate(Descriptorpool* _p, Descriptorlayout* _l) :
			p((DescriptorpoolPrivate*)_p),
			l((DescriptorlayoutPrivate*)_l)
		{
#if defined(FLAME_VULKAN)
			VkDescriptorSetAllocateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.pNext = nullptr;
			info.descriptorPool = p->v;
			info.descriptorSetCount = 1;
			info.pSetLayouts = &l->v;

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

		void DescriptorsetPrivate::set_buffer(uint binding, uint index, Buffer* b, uint offset, uint range)
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
			write.descriptorType = to_enum(l->bindings_map[binding].type);
			write.descriptorCount = 1;
			write.pBufferInfo = &i;
			write.pImageInfo = nullptr;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(p->d->v, 1, &write, 0, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void DescriptorsetPrivate::set_image(uint binding, uint index, Imageview* iv, Sampler* sampler)
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
			write.descriptorType = to_enum(l->bindings_map[binding].type);
			write.descriptorCount = 1;
			write.pBufferInfo = nullptr;
			write.pImageInfo = &i;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(p->d->v, 1, &write, 0, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Descriptorlayout* Descriptorset::layout()
		{
			return ((DescriptorsetPrivate*)this)->l;
		}

		void Descriptorset::set_buffer(uint binding, uint index, Buffer* b, uint offset, uint range)
		{
			((DescriptorsetPrivate*)this)->set_buffer(binding, index, b, offset, range);
		}

		void Descriptorset::set_image(uint binding, uint index, Imageview* v, Sampler* sampler)
		{
			((DescriptorsetPrivate*)this)->set_image(binding, index, v, sampler);
		}

		Descriptorset* Descriptorset::create(Descriptorpool* p, Descriptorlayout* l)
		{
			return new DescriptorsetPrivate(p, l);
		}

		void Descriptorset::destroy(Descriptorset* s)
		{
			delete (DescriptorsetPrivate*)s;
		}

		struct Descriptorset$
		{
			AttributeP<void> dsl$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (dsl$i.frame > out$o.frame)
				{
					if (out$o.v)
						Descriptorset::destroy((Descriptorset*)out$o.v);
					auto d = (Device*)bp_env().graphics_device;
					if (d && dsl$i.v)
						out$o.v = Descriptorset::create(d->dp, (Descriptorlayout*)dsl$i.v);
					else
					{
						printf("cannot create descriptorsetset\n");

						out$o.v = nullptr;
					}
					out$o.frame = dsl$i.frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Descriptorset$()
			{
				if (out$o.v)
					Descriptorset::destroy((Descriptorset*)out$o.v);
			}
		};

		struct BufferDescriptorWrite$
		{
			AttributeP<void> set$i;
			AttributeP<void> v$i;
			AttributeV<uint> binding$i;
			AttributeV<uint> index$i;
			AttributeV<uint> count$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS BufferDescriptorWrite$()
			{
				count$i.v = 1;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (set$i.frame > out$o.frame)
				{
					auto d = (Device*)bp_env().graphics_device;
					if (d && set$i.v && v$i.v)
					{
						for (auto i = 0; i < count$i.v; i++)
							((Descriptorset*)set$i.v)->set_buffer(binding$i.v, index$i.v + i, (Buffer*)v$i.v);
					}
					else
						printf("cannot write buffer descriptor\n");
					out$o.frame = set$i.frame;
				}
			}
		};

		struct ImageDescriptorWrite$
		{
			AttributeP<void> set$i;
			AttributeE<TargetType$> type$i;
			AttributeP<void> v$i;
			AttributeV<uint> binding$i;
			AttributeV<uint> index$i;
			AttributeV<uint> count$i;

			AttributeP<void> out$o;
			AttributeP<void> iv$o;

			FLAME_GRAPHICS_EXPORTS ImageDescriptorWrite$()
			{
				count$i.v = 1;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (set$i.frame > out$o.frame)
				{
					if (iv$o.v)
					{
						Imageview::destroy((Imageview*)iv$o.v);
						iv$o.v = nullptr;
					}
					auto d = (Device*)bp_env().graphics_device;
					if (d && set$i.v && v$i.v)
					{
						Imageview* iv = nullptr;
						assert(type$i.v != TargetImages);
						if (type$i.v == TargetImageview)
							iv = (Imageview*)v$i.v;
						else
						{
							iv = Imageview::create((Image*)v$i.v);
							iv$o.v = iv;
						}
						for (auto i = 0; i < count$i.v; i++)
							((Descriptorset*)set$i.v)->set_image(binding$i.v, index$i.v + i, iv, d->sp_linear);
					}
					else
						printf("cannot write image descriptor\n");
					out$o.frame = set$i.frame;
					iv$o.frame = set$i.frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~ImageDescriptorWrite$()
			{
				if (iv$o.v)
					Imageview::destroy((Imageview*)iv$o.v);
			}
		};

		PipelinelayoutPrivate::PipelinelayoutPrivate(Device* d, const std::vector<void*>& descriptorsetlayouts, uint push_constant_size, UdtInfo* push_constant_udt) :
			d((DevicePrivate*)d)
		{
			if (push_constant_udt)
			{
				pc_udt = push_constant_udt;
				pc_size = pc_udt->size();
			}
			else
			{
				pc_udt = nullptr;
				pc_size = push_constant_size;
			}

#if defined(FLAME_VULKAN)
			std::vector<VkDescriptorSetLayout> vk_descriptorsetlayouts;
			vk_descriptorsetlayouts.resize(descriptorsetlayouts.size());
			for (auto i = 0; i < vk_descriptorsetlayouts.size(); i++)
				vk_descriptorsetlayouts[i] = ((DescriptorlayoutPrivate*)descriptorsetlayouts[i])->v;

			VkPushConstantRange vk_pushconstant;
			vk_pushconstant.offset = 0;
			vk_pushconstant.size = pc_size;
			vk_pushconstant.stageFlags = to_flags(ShaderStageAll);

			VkPipelineLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.setLayoutCount = vk_descriptorsetlayouts.size();
			info.pSetLayouts = vk_descriptorsetlayouts.data();
			info.pushConstantRangeCount = pc_size > 0 ? 1 : 0;
			info.pPushConstantRanges = pc_size > 0 ? &vk_pushconstant : nullptr;

			chk_res(vkCreatePipelineLayout(((DevicePrivate*)d)->v, &info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif

			dsls.resize(descriptorsetlayouts.size());
			for (auto i = 0; i < dsls.size(); i++)
				dsls[i] = (DescriptorlayoutPrivate*)descriptorsetlayouts[i];
		}

		PipelinelayoutPrivate::~PipelinelayoutPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyPipelineLayout(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Pipelinelayout* Pipelinelayout::create(Device* d, const std::vector<void*>& descriptorsetlayouts, uint push_constant_size, UdtInfo* push_constant_udt)
		{
			return new PipelinelayoutPrivate(d, descriptorsetlayouts, push_constant_size, push_constant_udt);
		}

		void Pipelinelayout::destroy(Pipelinelayout* l)
		{
			delete (PipelinelayoutPrivate*)l;
		}

		struct Pipelinelayout$
		{
			AttributeP<std::vector<void*>> descriptorlayouts$i;
			AttributeV<uint> push_constant_size$i;
			AttributeV<std::string> push_constant_udt_name$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS Pipelinelayout$()
			{
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (descriptorlayouts$i.frame > out$o.frame || push_constant_size$i.frame > out$o.frame || push_constant_udt_name$i.frame > out$o.frame)
				{
					if (out$o.v)
						Pipelinelayout::destroy((Pipelinelayout*)out$o.v);
					auto& env = bp_env();
					auto d = env.graphics_device;
					auto descriptorlayouts = get_attribute_vec(descriptorlayouts$i);
					if (d && !descriptorlayouts.empty())
					{
						UdtInfo* pc_udt = nullptr;
						if (!push_constant_udt_name$i.v.empty())
						{
							pc_udt = find_udt(env.dbs, H(push_constant_udt_name$i.v.c_str()));
							assert(pc_udt);
						}
						out$o.v = Pipelinelayout::create(d, descriptorlayouts, push_constant_size$i.v, pc_udt);
					}
					else
					{
						printf("cannot create pipelinelayout\n");

						out$o.v = nullptr;
					}
					out$o.frame = maxN(descriptorlayouts$i.frame, push_constant_size$i.frame, push_constant_udt_name$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Pipelinelayout$()
			{
				if (out$o.v)
					Pipelinelayout::destroy((Pipelinelayout*)out$o.v);
			}
		};

		struct VertexInputAttribute$
		{
			AttributeV<uint> location$i;
			AttributeV<uint> buffer_id$i;
			AttributeV<uint> offset$i;
			AttributeE<Format$> format$i;
			AttributeV<std::string> name$i;

			AttributeV<VertexInputAttribute> out$o;

			FLAME_GRAPHICS_EXPORTS VertexInputAttribute$()
			{
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (location$i.frame > out$o.frame)
					out$o.v.location = location$i.v;
				if (buffer_id$i.frame > out$o.frame)
					out$o.v.buffer_id = buffer_id$i.v;
				if (offset$i.frame > out$o.frame)
					out$o.v.offset = offset$i.v;
				if (format$i.frame > out$o.frame)
					out$o.v.format = format$i.v;
				if (name$i.frame > out$o.frame)
					out$o.v.name = name$i.v;
				out$o.frame = maxN(location$i.frame, buffer_id$i.frame, offset$i.frame, format$i.frame, name$i.frame);
			}

			FLAME_GRAPHICS_EXPORTS ~VertexInputAttribute$()
			{
			}
		};

		struct VertexInputBuffer$
		{
			AttributeV<uint> id$i;
			AttributeV<uint> stride$i;
			AttributeE<VertexInputRate$> rate$i;

			AttributeV<VertexInputBuffer> out$o;

			FLAME_GRAPHICS_EXPORTS VertexInputBuffer$()
			{
				rate$i.v = VertexInputRateVertex;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (id$i.frame > out$o.frame)
					out$o.v.id = id$i.v;
				if (stride$i.frame > out$o.frame)
					out$o.v.stride = stride$i.v;
				if (rate$i.frame > out$o.frame)
					out$o.v.rate = rate$i.v;
				out$o.frame = maxN(id$i.frame, stride$i.frame, rate$i.frame);
			}
		};

		struct VertexInputInfo$
		{
			AttributeP<std::vector<void*>> attribs$i;
			AttributeP<std::vector<void*>> buffers$i;
			AttributeE<PrimitiveTopology$> primitive_topology$i;
			AttributeV<uint> patch_control_points$i;

			AttributeV<VertexInputInfo> out$o;

			FLAME_GRAPHICS_EXPORTS VertexInputInfo$()
			{
				primitive_topology$i.v = PrimitiveTopologyTriangleList;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (attribs$i.frame > out$o.frame)
					out$o.v.attribs = get_attribute_vec(attribs$i);
				if (buffers$i.frame > out$o.frame)
					out$o.v.buffers = get_attribute_vec(buffers$i);
				if (primitive_topology$i.frame > out$o.frame)
					out$o.v.primitive_topology = primitive_topology$i.v;
				if (patch_control_points$i.frame > out$o.frame)
					out$o.v.patch_control_points = patch_control_points$i.v;
				out$o.frame = maxN(attribs$i.frame, buffers$i.frame, primitive_topology$i.frame, patch_control_points$i.frame);
			}
		};

		struct StageInOut$
		{
			AttributeV<uint> location$i;
			AttributeE<Format$> format$i;
			AttributeV<std::string> name$i;

			AttributeV<StageInOut> out$o;

			FLAME_GRAPHICS_EXPORTS StageInOut$()
			{
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (location$i.frame > out$o.frame)
					out$o.v.location = location$i.v;
				if (format$i.frame > out$o.frame)
					out$o.v.format = format$i.v;
				if (name$i.frame > out$o.frame)
					out$o.v.name = name$i.v;
				out$o.frame = maxN(location$i.frame, format$i.frame, name$i.frame);
			}

			FLAME_GRAPHICS_EXPORTS ~StageInOut$()
			{
			}
		};

		struct OutputAttachmentInfo$
		{
			AttributeV<uint> location$i;
			AttributeE<Format$> format$i;
			AttributeV<std::string> name$i;
			AttributeV<bool> blend_enable$i;
			AttributeE<BlendFactor$> blend_src_color$i;
			AttributeE<BlendFactor$> blend_dst_color$i;
			AttributeE<BlendFactor$> blend_src_alpha$i;
			AttributeE<BlendFactor$> blend_dst_alpha$i;

			AttributeV<OutputAttachmentInfo> out$o;

			FLAME_GRAPHICS_EXPORTS OutputAttachmentInfo$()
			{
				format$i.v = Format_R8G8B8A8_UNORM;
				blend_src_color$i.v = BlendFactorOne;
				blend_src_alpha$i.v = BlendFactorOne;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (location$i.frame > out$o.frame)
					out$o.v.location = location$i.v;
				if (format$i.frame > out$o.frame)
					out$o.v.format = format$i.v;
				if (name$i.frame > out$o.frame)
					out$o.v.name = name$i.v;
				if (blend_enable$i.frame > out$o.frame)
					out$o.v.blend_enable = blend_enable$i.v;
				if (blend_src_color$i.frame > out$o.frame)
				{
					out$o.v.blend_src_color = blend_src_color$i.v;
					out$o.v.judge_dual();
				}
				if (blend_dst_color$i.frame > out$o.frame)
				{
					out$o.v.blend_dst_color = blend_dst_color$i.v;
					out$o.v.judge_dual();
				}
				if (blend_src_alpha$i.frame > out$o.frame)
				{
					out$o.v.blend_src_alpha = blend_src_alpha$i.v;
					out$o.v.judge_dual();
				}
				if (blend_dst_alpha$i.frame > out$o.frame)
				{
					out$o.v.blend_dst_alpha = blend_dst_alpha$i.v;
					out$o.v.judge_dual();
				}
				out$o.frame = maxN(location$i.frame, format$i.frame, name$i.frame, blend_enable$i.frame, 
					blend_src_color$i.frame, blend_dst_color$i.frame, blend_src_alpha$i.frame, blend_dst_alpha$i.frame);
			}

			FLAME_GRAPHICS_EXPORTS ~OutputAttachmentInfo$()
			{
			}
		};

		ShaderPrivate::ShaderPrivate(Device* d, const std::wstring& filename, const std::string& _prefix, const std::vector<void*>& _inputs, const std::vector<void*>& _outputs, Pipelinelayout* _pll, bool autogen_code) :
			d((DevicePrivate*)d)
		{
			assert(std::filesystem::exists(filename));

			auto ext = std::filesystem::path(filename).extension();

			stage = shader_stage_from_filename(filename);

			auto pll = (PipelinelayoutPrivate*)_pll;

			auto prefix = _prefix;
			if (autogen_code)
			{
				auto code = get_shader_autogen_code(stage, _inputs, _outputs, _pll);
				prefix += *code.p;
				delete_mail(code);
			}
			
			auto hash = H(prefix.c_str());
			std::wstring spv_filename(filename + L"." + std::to_wstring(hash) + L".spv");

			if (!std::filesystem::exists(spv_filename) || std::filesystem::last_write_time(spv_filename) < std::filesystem::last_write_time(filename))
			{
				auto vk_sdk_path = s2w(getenv("VK_SDK_PATH"));
				assert(vk_sdk_path != L"");

				prefix = std::string(
					"#version 450 core\n"
					"#extension GL_ARB_shading_language_420pack : enable\n") + 
					(stage != ShaderStageComp ? "#extension GL_ARB_separate_shader_objects : enable\n" : "")
					+ prefix;

				std::filesystem::remove(spv_filename);
				auto temp_filename = L"temp" + ext.wstring();
				std::wstring command_line(L" " + temp_filename + L" -flimit-file shader.conf -o" + spv_filename);

				std::ofstream temp_file(temp_filename);
				auto content = get_file_string(filename);
				content.erase(std::remove(content.begin(), content.end(), '\r'), content.end());
				temp_file << prefix << "\n";
				temp_file << content;
				temp_file.close();

				auto output1 = exec_and_get_output((vk_sdk_path + L"/Bin/glslc.exe"), command_line);
				std::filesystem::remove(temp_filename);
				if (!std::filesystem::exists(spv_filename))
				{
					printf("shader \"%s\" compile error:\n%s\n", w2s(filename).c_str(), output1.p->c_str());
					printf("trying to use fallback");

					std::ofstream temp_file(temp_filename);
					temp_file << prefix << "\n";

					switch (stage)
					{
					case ShaderStageVert:
						temp_file << "void main()\n"
							"{\n"
							"\tgl_Position = vec4(0, 0, 0, 1);"
							"}\n";
						break;
					case ShaderStageFrag:
						temp_file << 
							"void main()\n"
							"{\n"
							"}\n";
						break;
					default:
						assert(0); // WIP
					}

					temp_file.close();

					auto output2 = exec_and_get_output((vk_sdk_path + L"/Bin/glslc.exe"), command_line);
					std::filesystem::remove(temp_filename);
					if (!std::filesystem::exists(spv_filename))
					{
						printf(" - failed\n error:\n%s", output2.p->c_str());
						assert(0);
					}
					delete_mail(output2);

					printf(" - ok\n");
				}
				delete_mail(output1);
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
			shader_info.pCode = (uint*)spv_file.first.get();
			chk_res(vkCreateShaderModule(((DevicePrivate*)d)->v, &shader_info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif
			{
				spirv_cross::CompilerGLSL compiler((uint*)spv_file.first.get(), spv_file.second / sizeof(uint));
				auto resources = compiler.get_shader_resources();

				std::function<void(uint, Variable*)> get_v;
				get_v = [&](uint type_id, Variable* v) {
					const auto* t = &compiler.get_type(type_id);
					while (t->pointer)
					{
						type_id = t->parent_type;
						t = &compiler.get_type(type_id);
					}

					assert(t->array.size() <= 1); // multidimensional array is WIP
					v->count = t->array.empty() ? 1 : t->array[0];
					if (t->array.empty())
						v->array_stride = 0;
					else
						v->array_stride = compiler.get_decoration(type_id, spv::DecorationArrayStride);

					if (t->basetype == spirv_cross::SPIRType::Struct)
					{
						v->type_name = compiler.get_name(type_id);
						v->size = compiler.get_declared_struct_size(*t);
						for (auto i = 0; i < t->member_types.size(); i++)
						{
							auto m = new Variable;
							m->name = compiler.get_member_name(type_id, i);
							m->offset = compiler.type_struct_member_offset(*t, i);
							m->size = compiler.get_declared_struct_member_size(*t, i);
							v->members.emplace_back(m);
							get_v(t->member_types[i], m);
						}
					}
					else
					{
						std::string base_name;
						switch (t->basetype)
						{
						case spirv_cross::SPIRType::SByte:
							base_name = "char";
							break;
						case spirv_cross::SPIRType::UByte:
							base_name = "uchar";
							break;
						case spirv_cross::SPIRType::Short:
							base_name = "short";
							break;
						case spirv_cross::SPIRType::UShort:
							base_name = "ushort";
							break;
						case spirv_cross::SPIRType::Int:
							base_name = "int";
							break;
						case spirv_cross::SPIRType::UInt:
							base_name = "uint";
							break;
						case spirv_cross::SPIRType::Float:
							base_name = "float";
							break;
						case spirv_cross::SPIRType::SampledImage:
							base_name = "SampledImage";
							break;
						default:
							assert(0);
						}
						if (t->columns <= 1)
						{
							if (t->vecsize <= 1)
								v->type_name = base_name;
							else
								v->type_name = "Vec(" + std::to_string(t->vecsize) + "+" + base_name + ")";
						}
						else
							v->type_name = "Mat(" + std::to_string(t->vecsize) + "+" + std::to_string(t->columns) + "+" + base_name + ")";
					}
				};

				for (auto& src : resources.stage_inputs)
				{
					auto r = new Resource;
					r->location = compiler.get_decoration(src.id, spv::DecorationLocation);
					r->name = src.name;
					get_v(src.type_id, &r->v);
					inputs.emplace_back(r);
				}

				for (auto& src : resources.stage_outputs)
				{
					auto r = new Resource;
					r->location = compiler.get_decoration(src.id, spv::DecorationLocation);
					r->index = compiler.get_decoration(src.id, spv::DecorationIndex);
					r->name = src.name;
					get_v(src.type_id, &r->v);
					outputs.emplace_back(r);
				}

				for (auto& src : resources.uniform_buffers)
				{
					auto r = new Resource;
					r->set = compiler.get_decoration(src.id, spv::DecorationDescriptorSet);
					r->binding = compiler.get_decoration(src.id, spv::DecorationBinding);
					r->name = src.name;
					get_v(src.type_id, &r->v);
					uniform_buffers.emplace_back(r);
				}

				for (auto& src : resources.storage_buffers)
				{
					auto r = new Resource;
					r->set = compiler.get_decoration(src.id, spv::DecorationDescriptorSet);
					r->binding = compiler.get_decoration(src.id, spv::DecorationBinding);
					r->name = src.name;
					get_v(src.type_id, &r->v);
					storage_buffers.emplace_back(r);
				}

				for (auto& src : resources.sampled_images)
				{
					auto r = new Resource;
					r->set = compiler.get_decoration(src.id, spv::DecorationDescriptorSet);
					r->binding = compiler.get_decoration(src.id, spv::DecorationBinding);
					r->name = src.name;
					get_v(src.type_id, &r->v);
					sampled_images.emplace_back(r);
				}

				for (auto& src : resources.storage_images)
				{
					auto r = new Resource;
					r->set = compiler.get_decoration(src.id, spv::DecorationDescriptorSet);
					r->binding = compiler.get_decoration(src.id, spv::DecorationBinding);
					r->name = src.name;
					get_v(src.type_id, &r->v);
					storage_images.emplace_back(r);
				}

				assert(resources.push_constant_buffers.size() <= 1);
				if (!resources.push_constant_buffers.empty())
				{
					auto& src = resources.push_constant_buffers[0];
					push_constant.reset(new Resource);
					push_constant->name = src.name;

					get_v(src.type_id, &push_constant->v);
				}
				
				assert(resources.subpass_inputs.empty()); // subpass inputs are WIP
				assert(resources.atomic_counters.empty()); // atomic counters are WIP
				assert(resources.acceleration_structures.empty()); // acceleration structures are WIP

				// do validate

				if (!_inputs.empty())
				{
					if (stage == ShaderStageVert)
					{
						for (auto& r : inputs)
						{
							VertexInputAttribute* via = nullptr;
							for (auto _i : _inputs)
							{
								auto i = (VertexInputAttribute*)_i;
								if (r->location == i->location)
								{
									via = i;
									break;
								}
							}
							assert(via && r->v.type_name == format_to_cpp_typename(via->format));
						}
					}
					else
					{
						for (auto& r : inputs)
						{
							StageInOut* io = nullptr;
							for (auto _i : _inputs)
							{
								auto i = (StageInOut*)_i;
								if (r->location == i->location)
								{
									io = i;
									break;
								}
							}
							assert(io && r->v.type_name == format_to_cpp_typename(io->format));
						}
					}
				}

				if (!_outputs.empty())
				{
					if (stage == ShaderStageFrag)
					{
						for (auto& r : outputs)
						{
							OutputAttachmentInfo* oa = nullptr;
							for (auto _o : _outputs)
							{
								auto o = (OutputAttachmentInfo*)_o;
								if (r->location == o->location)  
								{
									oa = o;
									break;
								}
							}
							assert(oa && r->v.type_name == format_to_cpp_typename(oa->format));
							assert(r->index <= 1 && (r->index != 1 || oa->dual_src));
						}
					}
					else
					{
						for (auto& r : outputs)
						{
							StageInOut* io = nullptr;
							for (auto _o : _outputs)
							{
								auto o = (StageInOut*)_o;
								if (r->location == o->location)
								{
									io = o;
									break;
								}
							}
							assert(io && r->v.type_name == format_to_cpp_typename(io->format));
						}
					}
				}

				if (pll) 
				{
					auto validate_binding = [&](DescriptorType$ type, Resource* r) -> DescriptorBinding& {
						assert(r->set < pll->dsls.size());
						auto dsl = pll->dsls[r->set];
						assert(r->binding < dsl->bindings_map.size());
						auto& binding = dsl->bindings_map[r->binding];
						assert(binding.binding < 64 && binding.type == type && r->v.count == binding.count);
						return binding;
					};
					auto validate_variable = [&](UdtInfo* u, Variable* v) {
						assert(v->members.size() == u->variable_count());
						for (auto i = 0; i < v->members.size(); i++)
						{
							auto m = v->members[i].get();
							assert(m->members.empty()); // nested structs are WIP

							auto v = u->variable(i);
							assert(m->type_name == v->type()->name());
							assert(m->name == v->name());
							assert(m->offset == v->offset());
							assert(m->size == v->size());
							assert(m->count == 1); // count is WIP
						}
					};

					for (auto& r : uniform_buffers)
					{
						auto& binding = validate_binding(DescriptorUniformBuffer, r.get());
						if (binding.buffer_udt)
							validate_variable(binding.buffer_udt, &r->v);
					}
					for (auto& r : storage_buffers)
						validate_binding(DescriptorStorageBuffer, r.get());
					for (auto& r : sampled_images)
						validate_binding(DescriptorSampledImage, r.get());
					for (auto& r : storage_images)
						validate_binding(DescriptorStorageImage, r.get());

					if (push_constant)
					{
						assert(!pll->pc_udt || push_constant->v.size == pll->pc_size);
						if (pll->pc_udt)
							validate_variable(pll->pc_udt, &push_constant->v);
					}
				}
			}
		}

		ShaderPrivate::~ShaderPrivate()
		{
#if defined(FLAME_VULKAN)
			if (v)
				vkDestroyShaderModule(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Mail<std::string> get_shader_autogen_code(ShaderStage$ stage, const std::vector<void*>& inputs, const std::vector<void*>& outputs, Pipelinelayout* _pll)
		{
			auto ret = new_mail<std::string>();

			if (!inputs.empty())
			{
				*ret.p += "\n";
				if (stage == ShaderStageVert)
				{
					for (auto _i : inputs)
					{
						auto i = (VertexInputAttribute*)_i;
						*ret.p += "layout (location = " + std::to_string(i->location) + ") in " + format_to_glsl_typename(i->format) + " in_" + i->name + ";\n";
					}
				}
				else
				{
					for (auto _i : inputs)
					{
						auto i = (StageInOut*)_i;
						*ret.p += "layout (location = " + std::to_string(i->location) + ") in " + format_to_glsl_typename(i->format) + " in_" + i->name + ";\n";
					}
				}
			}

			if (!outputs.empty())
			{
				*ret.p += "\n";
				if (stage == ShaderStageFrag)
				{
					for (auto _o : outputs)
					{
						auto o = (OutputAttachmentInfo*)_o;
						if (o->dual_src)
						{
							*ret.p += "layout (location = " + std::to_string(o->location) + ", index = 0) out " + format_to_glsl_typename(o->format) + " out_" + o->name + "0;\n";
							*ret.p += "layout (location = " + std::to_string(o->location) + ", index = 1) out " + format_to_glsl_typename(o->format) + " out_" + o->name + "1;\n";
						}
						else
							*ret.p += "layout (location = " + std::to_string(o->location) + ") out " + format_to_glsl_typename(o->format) + " out_" + o->name + ";\n";
					}
				}
				else
				{
					for (auto _o : outputs)
					{
						auto o = (StageInOut*)_o;
						*ret.p += "layout (location = " + std::to_string(o->location) + ") out " + format_to_glsl_typename(o->format) + " out_" + o->name + ";\n";
					}
				}
			}

			if (_pll)
			{
				auto pll = (PipelinelayoutPrivate*)_pll;

				auto print_udt = [](UdtInfo* udt, std::string& out) {
					for (auto i = 0; i < udt->variable_count(); i++)
					{
						auto v = udt->variable(i);
						auto t = v->type();
						assert(t->tag() == TypeTagVariable);
						out += "\t" + cpp_typehash_to_glsl_typename(t->hash()) + " " + v->name() + ";\n";
					}
				};

				if (!pll->dsls.empty())
				{
					*ret.p += "\n";
					for (auto i = 0; i < pll->dsls.size(); i++)
					{
						auto dsl = pll->dsls[i];
						for (auto j = 0; j < dsl->bindings_map.size(); j++)
						{
							auto& b = dsl->bindings_map[j];
							if (b.binding < 64)
							{
								switch (b.type)
								{
								case DescriptorUniformBuffer:
								{
									assert(b.buffer_udt);
									*ret.p += "layout (binding = " + std::to_string(j) + ") uniform " + b.buffer_udt->name() + "\n{\n";
									print_udt(b.buffer_udt, *ret.p);
									*ret.p += "}" + b.name + ";\n";
								}
								break;
								case DescriptorSampledImage:
									*ret.p += "layout (binding = " + std::to_string(j) + ") uniform sampler2D " + b.name + (b.count > 1 ? ("[" + std::to_string(b.count) + "]") : "") + ";\n";
									break;
								default:
									assert(0); // others are WIP
								}
							}
						}
					}
				}

				if (pll->pc_udt)
				{
					*ret.p += "\nlayout(push_constant) uniform PushconstantT\n{\n";
					print_udt(pll->pc_udt, *ret.p);
					*ret.p += "}pc;\n";
				}
			}

			return ret;
		}

		Shader* Shader::create(Device* d, const std::wstring& filename, const std::string& prefix, const std::vector<void*>& inputs, const std::vector<void*>& outputs, Pipelinelayout* pll, bool autogen_code)
		{
			return new ShaderPrivate(d, filename, prefix, inputs, outputs, pll, autogen_code);
		}

		void Shader::destroy(Shader* s)
		{
			delete (ShaderPrivate*)s;
		}

		struct Shader$
		{
			AttributeV<std::wstring> filename$i;
			AttributeV<std::string> prefix$i;
			AttributeP<std::vector<void*>> inputs$i;
			AttributeP<std::vector<void*>> outputs$i;
			AttributeP<void> pll$i;
			AttributeV<bool> autogen_code$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS Shader$()
			{
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (filename$i.frame > out$o.frame || prefix$i.frame > out$o.frame || inputs$i.frame > out$o.frame || outputs$i.frame > out$o.frame || pll$i.frame > out$o.frame || autogen_code$i.frame > out$o.frame)
				{
					if (out$o.v)
						Shader::destroy((Shader*)out$o.v);
					auto d = (Device*)bp_env().graphics_device;
					if (d)
						out$o.v = Shader::create(d, bp_env().path + L"/" + filename$i.v, prefix$i.v, get_attribute_vec(inputs$i), get_attribute_vec(outputs$i), (Pipelinelayout*)pll$i.v, autogen_code$i.v);
					else
					{
						printf("cannot create shader\n");

						out$o.v = nullptr;
					}
					out$o.frame = maxN(filename$i.frame, prefix$i.frame, inputs$i.frame, outputs$i.frame, pll$i.frame, autogen_code$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Shader$()
			{
				if (out$o.v)
					Shader::destroy((Shader*)out$o.v);
			}
		};

		PipelinePrivate::PipelinePrivate(Device* _d, const std::vector<void*>& shaders, Pipelinelayout* _pll, Renderpass* rp, uint subpass_idx, VertexInputInfo* _vi, const Vec2u& _vp, RasterInfo* _raster, SampleCount$ _sc, DepthInfo* _depth, const std::vector<void*>& _outputs, const std::vector<uint>& _dynamic_states) :
			d((DevicePrivate*)_d),
			pll((PipelinelayoutPrivate*)_pll)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkPipelineShaderStageCreateInfo> vk_stage_infos;
			std::vector<VkVertexInputAttributeDescription> vk_vi_attributes;
			std::vector<VkVertexInputBindingDescription> vk_vi_bindings;
			std::vector<VkPipelineColorBlendAttachmentState> vk_blend_attachment_states;
			std::vector<VkDynamicState> vk_dynamic_states;

			vk_stage_infos.resize(shaders.size());
			for (auto i = 0; i < shaders.size(); i++)
			{
				auto src = (ShaderPrivate*)shaders[i];
				auto& dst = vk_stage_infos[i];
				dst.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				dst.flags = 0;
				dst.pNext = nullptr;
				dst.pSpecializationInfo = nullptr;
				dst.pName = "main";
				dst.stage = to_enum(src->stage);
				dst.module = src->v;
			}

			if (_vi)
			{
				vk_vi_attributes.resize(_vi->attribs.size());
				for (auto i = 0; i < _vi->attribs.size(); i++)
				{
					const auto& src = *(VertexInputAttribute*)_vi->attribs[i];
					auto& dst = vk_vi_attributes[i];
					dst.location = src.location;
					dst.binding = src.buffer_id;
					dst.offset = src.offset;
					dst.format = to_enum(src.format);
				}
				vk_vi_bindings.resize(_vi->buffers.size());
				for (auto i = 0; i < _vi->buffers.size(); i++)
				{
					const auto& src = *(VertexInputBuffer*)_vi->buffers[i];
					auto& dst = vk_vi_bindings[i];
					dst.binding = src.id;
					dst.stride = src.stride;
					dst.inputRate = to_enum(src.rate);
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
			assembly_state.topology = to_enum(_vi ? _vi->primitive_topology : PrimitiveTopologyTriangleList);
			assembly_state.primitiveRestartEnable = VK_FALSE;

			VkPipelineTessellationStateCreateInfo tess_state;
			tess_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			tess_state.pNext = nullptr;
			tess_state.flags = 0;
			tess_state.patchControlPoints = _vi ? _vi->patch_control_points : 0;

			VkViewport viewport;
			viewport.width = (float)_vp.x();
			viewport.height = (float)_vp.y();
			viewport.minDepth = (float)0.0f;
			viewport.maxDepth = (float)1.0f;
			viewport.x = 0;
			viewport.y = 0;

			VkRect2D scissor;
			scissor.extent.width = _vp.x();
			scissor.extent.height = _vp.y();
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
			raster_state.depthClampEnable = _raster ? _raster->depth_clamp : false;
			raster_state.rasterizerDiscardEnable = VK_FALSE;
			raster_state.polygonMode = to_enum(_raster ? _raster->polygon_mode : PolygonModeFill);
			raster_state.cullMode = to_enum(_raster ? _raster->cull_mode : CullModeNone);
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
			multisample_state.rasterizationSamples = to_enum(_sc);
			multisample_state.sampleShadingEnable = VK_FALSE;
			multisample_state.minSampleShading = 0.f;
			multisample_state.pSampleMask = nullptr;
			multisample_state.alphaToCoverageEnable = VK_FALSE;
			multisample_state.alphaToOneEnable = VK_FALSE;

			VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
			depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depth_stencil_state.flags = 0;
			depth_stencil_state.pNext = nullptr;
			depth_stencil_state.depthTestEnable = _depth ? _depth->test : false;
			depth_stencil_state.depthWriteEnable = _depth ? _depth->write : false;
			depth_stencil_state.depthCompareOp = to_enum(_depth ? _depth->compare_op : CompareOpLess);
			depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
			depth_stencil_state.minDepthBounds = 0;
			depth_stencil_state.maxDepthBounds = 0;
			depth_stencil_state.stencilTestEnable = VK_FALSE;
			depth_stencil_state.front = {};
			depth_stencil_state.back = {};

			vk_blend_attachment_states.resize(rp->color_attachment_count());
			for (auto& a : vk_blend_attachment_states)
			{
				memset(&a, 0, sizeof(a));
				a.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			}
			for (auto i = 0; i < _outputs.size(); i++)
			{
				const auto& src = *(OutputAttachmentInfo*)_outputs[i];
				auto& dst = vk_blend_attachment_states[i];
				dst.blendEnable = src.blend_enable;
				dst.srcColorBlendFactor = to_enum(src.blend_src_color);
				dst.dstColorBlendFactor = to_enum(src.blend_dst_color);
				dst.colorBlendOp = VK_BLEND_OP_ADD;
				dst.srcAlphaBlendFactor = to_enum(src.blend_src_alpha);
				dst.dstAlphaBlendFactor = to_enum(src.blend_dst_alpha);
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

			for (auto& s : _dynamic_states)
				vk_dynamic_states.push_back(to_enum((DynamicState)s));
			if (_vp.x() == 0 && _vp.y() == 0)
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
			pipeline_info.layout = pll->v;
			pipeline_info.renderPass = rp ? ((RenderpassPrivate*)rp)->v : nullptr;
			pipeline_info.subpass = subpass_idx;
			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;

			chk_res(vkCreateGraphicsPipelines(d->v, 0, 1, &pipeline_info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif

			type = PipelineGraphics;
		}

		PipelinePrivate::PipelinePrivate(Device* _d, Shader* compute_shader, Pipelinelayout* _pll) :
			d((DevicePrivate*)_d),
			pll((PipelinelayoutPrivate*)_pll)
		{
#if defined(FLAME_VULKAN)

			VkComputePipelineCreateInfo pipeline_info;
			pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			pipeline_info.pNext = nullptr;
			pipeline_info.flags = 0;

			pipeline_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			pipeline_info.stage.flags = 0;
			pipeline_info.stage.pNext = nullptr;
			pipeline_info.stage.pSpecializationInfo = nullptr;
			pipeline_info.stage.pName = "main";
			pipeline_info.stage.stage = to_enum(ShaderStageComp);
			pipeline_info.stage.module = ((ShaderPrivate*)compute_shader)->v;

			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;
			pipeline_info.layout = pll->v;

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

		Pipeline* Pipeline::create(Device* d, const std::vector<void*>& shaders, Pipelinelayout* pll, Renderpass* rp, uint subpass_idx, VertexInputInfo* vi, const Vec2u& vp, RasterInfo* raster, SampleCount$ sc, DepthInfo* depth, const std::vector<void*>& outputs, const std::vector<uint>& dynamic_states)
		{
			return new PipelinePrivate(d, shaders, pll, rp, subpass_idx, vi, vp, raster, sc, depth, outputs, dynamic_states);
		}

		Pipeline* Pipeline::create(Device* d, Shader* compute_shader, Pipelinelayout* pll)
		{
			return new PipelinePrivate(d, compute_shader, pll);
		}

		void Pipeline::destroy(Pipeline* p)
		{
			delete (PipelinePrivate*)p;
		}

		struct Pipeline$
		{
			AttributeP<std::vector<void*>> shaders$i;
			AttributeP<void> pll$i;
			AttributeP<void> renderpass$i;
			AttributeV<uint> subpass_idx$i;
			AttributeP<void> vi$i;
			AttributeV<Vec2u> vp$i;
			AttributeP<void> raster$i;
			AttributeE<SampleCount$> sc$i;
			AttributeP<void> depth$i;
			AttributeP<std::vector<void*>> outputs$i;
			AttributeP<std::vector<uint>> dynamic_states$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS Pipeline$()
			{
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (renderpass$i.frame > out$o.frame || subpass_idx$i.frame > out$o.frame || shaders$i.frame > out$o.frame || pll$i.frame > out$o.frame ||
					vi$i.frame > out$o.frame || vp$i.frame > out$o.frame || raster$i.frame > out$o.frame || sc$i.frame > out$o.frame || depth$i.frame > out$o.frame || outputs$i.frame > out$o.frame || dynamic_states$i.frame > out$o.frame)
				{
					if (out$o.v)
						Pipeline::destroy((Pipeline*)out$o.v);
					auto d = (Device*)bp_env().graphics_device;
					auto shaders = get_attribute_vec(shaders$i);
					if (d && renderpass$i.v && ((Renderpass*)renderpass$i.v)->subpass_count() > subpass_idx$i.v && !shaders.empty() && pll$i.v)
						out$o.v = Pipeline::create(d, shaders, (Pipelinelayout*)pll$i.v, (Renderpass*)renderpass$i.v, subpass_idx$i.v,
						(VertexInputInfo*)vi$i.v, vp$i.v, (RasterInfo*)raster$i.v, sc$i.v, (DepthInfo*)depth$i.v, get_attribute_vec(outputs$i), dynamic_states$i.v ? *dynamic_states$i.v : std::vector<uint>());
					else
					{
						printf("cannot create pipeline\n");

						out$o.v = nullptr;
					}
					out$o.frame = maxN(shaders$i.frame, pll$i.frame, renderpass$i.frame, subpass_idx$i.frame, 
						vi$i.frame, vp$i.frame, raster$i.frame, sc$i.frame, depth$i.frame, outputs$i.frame, dynamic_states$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Pipeline$()
			{
				if (out$o.v)
					Pipeline::destroy((Pipeline*)out$o.v);
			}

		};
	}
}
