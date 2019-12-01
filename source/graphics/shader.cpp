#include <flame/foundation/blueprint.h>
#include "device_private.h"
#include "renderpass_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "shader_private.h"

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

		DescriptorlayoutPrivate::DescriptorlayoutPrivate(Device* _d, const std::vector<void*>& _bindings, Descriptorpool* default_set_pool) :
			d((DevicePrivate*)_d)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
			bindings.resize(_bindings.size());
			for (auto i = 0; i < bindings.size(); i++)
			{
				auto b = (DescriptorBindingBase*)_bindings[i];
				auto type = b->type;

				DescriptorBindingBase* new_b = nullptr;
				if (type == DescriptorUniformBuffer || type == DescriptorStorageBuffer)
					bindings[i] = new DescriptorBufferBinding(*(DescriptorBufferBinding*)b);
				else if (type == DescriptorSampledImage || type == DescriptorStorageImage)
					bindings[i] = new DescriptorImageBinding(*(DescriptorImageBinding*)b);
				else
					assert(0);

				VkDescriptorSetLayoutBinding vk_binding;
				vk_binding.binding = i;
				vk_binding.descriptorType = to_enum(type);
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
			if (default_set_pool)
			{
				default_set = Descriptorset::create(default_set_pool, this);
				for (auto i = 0; i < bindings.size(); i++)
				{
					auto& _b = bindings[i];
					auto type = _b->type;
					if (type == DescriptorUniformBuffer || type == DescriptorStorageBuffer)
					{
						auto b = (DescriptorBufferBinding*)_b;
						if (b->buffer)
						{
							for (auto j = 0; j < b->count; j++)
								default_set->set_buffer(i, j, b->buffer);
						}
					}
					else if (type == DescriptorSampledImage || type == DescriptorStorageImage)
					{
						auto b = (DescriptorImageBinding*)_b;
						if (b->view)
						{
							for (auto j = 0; j < b->count; j++)
								default_set->set_image(i, j, b->view, b->sampler);
						}
					}
				}
			}
			else
				default_set = nullptr;
		}

		DescriptorlayoutPrivate::~DescriptorlayoutPrivate()
		{
			if (default_set)
				Descriptorset::destroy(default_set);

			for (auto b : bindings)
			{
				auto type = b->type;
				if (type == DescriptorUniformBuffer || type == DescriptorStorageBuffer)
					delete(DescriptorBufferBinding*)b;
				else if (type == DescriptorSampledImage || type == DescriptorStorageImage)
					delete(DescriptorImageBinding*)b;
			}
#if defined(FLAME_VULKAN)
			vkDestroyDescriptorSetLayout(((DevicePrivate*)d)->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		const DescriptorBindingBase* Descriptorlayout::get_binding(uint binding) const
		{
			return ((DescriptorlayoutPrivate*)this)->bindings[binding];
		}

		Descriptorset* Descriptorlayout::default_set() const
		{
			return ((DescriptorlayoutPrivate*)this)->default_set;
		}

		Descriptorlayout* Descriptorlayout::create(Device* d, const std::vector<void*>& bindings, Descriptorpool* pool_to_create_default_set)
		{
			return new DescriptorlayoutPrivate(d, bindings, pool_to_create_default_set);
		}

		void Descriptorlayout::destroy(Descriptorlayout* l)
		{
			delete (DescriptorlayoutPrivate*)l;
		}

		struct DescriptorBufferBinding$
		{
			AttributeD<bool> storage$i;
			AttributeD<uint> count$i;
			AttributeD<std::string> name$i;
			AttributeP<void> buffer$i;

			AttributeD<DescriptorBufferBinding> out$o;

			FLAME_GRAPHICS_EXPORTS DescriptorBufferBinding$()
			{
				storage$i.v = false;
				count$i.v = 1;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				auto last_out_frame = out$o.b.frame;
				if (storage$i.b.frame > last_out_frame)
				{
					out$o.v.type = storage$i.v ? DescriptorStorageBuffer : DescriptorUniformBuffer;
					out$o.b.frame = looper().frame;
				}
				if (count$i.b.frame > last_out_frame)
				{
					out$o.v.count = count$i.v;
					out$o.b.frame = looper().frame;
				}
				if (name$i.b.frame > last_out_frame)
				{
					out$o.v.name = name$i.v;
					out$o.b.frame = looper().frame;
				}
				if (buffer$i.b.frame > last_out_frame)
				{
					out$o.v.buffer = (Buffer*)buffer$i.v;
					out$o.b.frame = looper().frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~DescriptorBufferBinding$()
			{
			}
		};

		struct DescriptorImageBinding$
		{
			AttributeD<uint> binding$i;
			AttributeD<bool> storage$i;
			AttributeD<uint> count$i;
			AttributeD<std::string> name$i;
			AttributeE<TargetType$> target_type$i;
			AttributeP<void> v$i;

			AttributeD<DescriptorImageBinding> out$o;
			AttributeP<void> iv$o;

			FLAME_GRAPHICS_EXPORTS DescriptorImageBinding$()
			{
				storage$i.v = false;
				count$i.v = 1;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				auto last_out_frame = out$o.b.frame;
				if (last_out_frame == -1)
				{
					auto d = Device::default_one();
					if (d)
						out$o.v.sampler = d->sp_linear;
					out$o.b.frame = looper().frame;
				}
				if (target_type$i.b.frame > iv$o.b.frame || v$i.b.frame > iv$o.b.frame)
				{
					if (iv$o.v)
						Imageview::destroy((Imageview*)iv$o.v);
					if (target_type$i.v == TargetImage)
						iv$o.v = Imageview::create((Image*)v$i.v);
					else
						iv$o.v = nullptr;
					iv$o.b.frame = looper().frame;
				}
				if (storage$i.b.frame > last_out_frame)
				{
					out$o.v.type = storage$i.v ? DescriptorStorageImage : DescriptorSampledImage;
					out$o.b.frame = looper().frame;
				}
				if (count$i.b.frame > last_out_frame)
				{
					out$o.v.count = count$i.v;
					out$o.b.frame = looper().frame;
				}
				if (name$i.b.frame > last_out_frame)
				{
					out$o.v.name = name$i.v;
					out$o.b.frame = looper().frame;
				}
				if (iv$o.b.frame > last_out_frame)
				{
					switch (target_type$i.v)
					{
					case TargetImage:
						out$o.v.view = (Imageview*)iv$o.v;
						break;
					case TargetImageview:
						out$o.v.view = (Imageview*)v$i.v;
						break;
					case TargetImages:
						out$o.v.view = nullptr;
						break;
					}
					out$o.b.frame = looper().frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~DescriptorImageBinding$()
			{
				if (iv$o.v)
					Imageview::destroy((Imageview*)iv$o.v);
			}
		};

		struct Descriptorlayout$
		{
			AttributeP<std::vector<void*>> bindings$i;
			AttributeD<bool> create_default_set$i;

			AttributeP<void> out$o;
			AttributeP<void> default_set$o;

			FLAME_GRAPHICS_EXPORTS Descriptorlayout$()
			{
				create_default_set$i.v = true;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (bindings$i.b.frame > out$o.b.frame || create_default_set$i.b.frame > default_set$o.b.frame)
				{
					if (out$o.v)
						Descriptorlayout::destroy((Descriptorlayout*)out$o.v);
					auto d = Device::default_one();
					if (d)
					{
						out$o.v = Descriptorlayout::create(d, get_attribute_vec(bindings$i), create_default_set$i.v ? d->dp : nullptr);
						default_set$o.v = ((Descriptorlayout*)out$o.v)->default_set();
					}
					else
					{
						printf("cannot create descriptorlayout\n");

						out$o.v = nullptr;
						default_set$o.v = nullptr;
					}
					auto frame = looper().frame;
					out$o.b.frame = frame;
					default_set$o.b.frame = frame;
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
			write.descriptorType = to_enum(l->bindings[binding]->type);
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
			write.descriptorType = to_enum(l->bindings[binding]->type);
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

		void Descriptorset::set_image(uint binding, uint index, Imageview* v, Filter filter)
		{
			auto d = ((DescriptorsetPrivate*)this)->p->d;
			((DescriptorsetPrivate*)this)->set_image(binding, index, v, filter == FilterNearest ? d->sp_nearest : d->sp_linear);
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
				if (dsl$i.b.frame > out$o.b.frame)
				{
					if (out$o.v)
						Descriptorset::destroy((Descriptorset*)out$o.v);
					auto d = Device::default_one();
					if (d && dsl$i.v)
						out$o.v = Descriptorset::create(d->dp, (Descriptorlayout*)dsl$i.v);
					else
					{
						printf("cannot create descriptorsetset\n");

						out$o.v = nullptr;
					}
					out$o.b.frame = looper().frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Descriptorset$()
			{
				if (out$o.v)
					Descriptorset::destroy((Descriptorset*)out$o.v);
			}
		};

		PipelinelayoutPrivate::PipelinelayoutPrivate(Device* d, const std::vector<void*>& descriptorsetlayouts, uint push_constant_size) :
			d((DevicePrivate*)d),
			pc_size(push_constant_size)
		{
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

		Pipelinelayout* Pipelinelayout::create(Device* d, const std::vector<void*>& descriptorsetlayouts, uint push_constant_size)
		{
			return new PipelinelayoutPrivate(d, descriptorsetlayouts, push_constant_size);
		}

		void Pipelinelayout::destroy(Pipelinelayout* l)
		{
			delete (PipelinelayoutPrivate*)l;
		}

		struct Pipelinelayout$
		{
			AttributeP<std::vector<void*>> descriptorlayouts$i;
			AttributeD<uint> push_constant_size$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS Pipelinelayout$()
			{
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (descriptorlayouts$i.b.frame > out$o.b.frame || push_constant_size$i.b.frame > out$o.b.frame)
				{
					if (out$o.v)
						Pipelinelayout::destroy((Pipelinelayout*)out$o.v);
					auto& env = bp_env();
					auto d = Device::default_one();
					auto descriptorlayouts = get_attribute_vec(descriptorlayouts$i);
					if (d)
						out$o.v = Pipelinelayout::create(d, !descriptorlayouts.empty() ? descriptorlayouts : std::vector<void*>(), push_constant_size$i.v);
					else
					{
						printf("cannot create pipelinelayout\n");

						out$o.v = nullptr;
					}
					out$o.b.frame = looper().frame;
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
			AttributeD<std::string> name$i;
			AttributeE<Format$> format$i;

			AttributeD<VertexInputAttribute> out$o;

			FLAME_GRAPHICS_EXPORTS VertexInputAttribute$()
			{
				format$i.v = Format_R8G8B8A8_UNORM;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				auto last_out_frame = out$o.b.frame;
				if (name$i.b.frame > last_out_frame)
				{
					out$o.v.name = name$i.v;
					out$o.b.frame = looper().frame;
				}
				if (format$i.b.frame > last_out_frame)
				{
					out$o.v.format = format$i.v;
					out$o.b.frame = looper().frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~VertexInputAttribute$()
			{
			}
		};

		struct VertexInputBuffer$
		{
			AttributeP<std::vector<void*>> attributes$i;
			AttributeE<VertexInputRate$> rate$i;

			AttributeD<VertexInputBuffer> out$o;

			FLAME_GRAPHICS_EXPORTS VertexInputBuffer$()
			{
				rate$i.v = VertexInputRateVertex;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				auto last_out_frame = out$o.b.frame;
				if (attributes$i.b.frame > last_out_frame)
				{
					out$o.v.attributes = get_attribute_vec(attributes$i);
					out$o.b.frame = looper().frame;
				}
				if (rate$i.b.frame > last_out_frame)
				{
					out$o.v.rate = rate$i.v;
					out$o.b.frame = looper().frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~VertexInputBuffer$()
			{
			}
		};

		struct VertexInputInfo$
		{
			AttributeP<std::vector<void*>> buffers$i;
			AttributeE<PrimitiveTopology$> primitive_topology$i;
			AttributeD<uint> patch_control_points$i;

			AttributeD<VertexInputInfo> out$o;

			FLAME_GRAPHICS_EXPORTS VertexInputInfo$()
			{
				primitive_topology$i.v = PrimitiveTopologyTriangleList;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				auto last_out_frame = out$o.b.frame;
				if (buffers$i.b.frame > last_out_frame)
				{
					out$o.v.buffers = get_attribute_vec(buffers$i);
					out$o.b.frame = looper().frame;
				}
				if (primitive_topology$i.b.frame > last_out_frame)
				{
					out$o.v.primitive_topology = primitive_topology$i.v;
					out$o.b.frame = looper().frame;
				}
				if (patch_control_points$i.b.frame > last_out_frame)
				{
					out$o.v.patch_control_points = patch_control_points$i.v;
					out$o.b.frame = looper().frame;
				}
			}
		};

		const std::regex shader_regex_in(R"(^\s*in\s+([\w]+)\s+i_([\w]+)\s*;\s*$)");
		const std::regex shader_regex_out(R"(^\s*out\s+([\w]+)\s+o_([\w]+)(\{([\w:\s]+)\})?\s*;\s*$)");
		const std::regex shader_regex_pc(R"(^\s*pushconstant\s*$)");
		const std::regex shader_regex_ubo(R"(^\s*uniform\s+([\w]+)\s*$)");
		const std::regex shader_regex_tex(R"(^\s*sampler2D\s+([\w]+)([\[\]0-9\s]+)?;\s*$)");

		void compile_shaders(DevicePrivate* d, std::vector<StageInfo>& stage_infos, PipelinelayoutPrivate* pll, const VertexInputInfo* vi)
		{
			for (auto i = 0; i < stage_infos.size(); i++)
			{
				auto& s = stage_infos[i];

				std::ifstream src(s.path);

				std::string glsl_header = "#version 450 core\n"
					"#extension GL_ARB_shading_language_420pack : enable\n";
				if (s.type != ShaderStageComp)
					glsl_header += "#extension GL_ARB_separate_shader_objects : enable\n";
				glsl_header += s.prefix + "\n";

				auto type_id = 0;

				std::ofstream glsl_file(L"out.glsl");
				glsl_file << glsl_header;
				{
					std::string line;
					std::smatch match;
					while (!src.eof())
					{
						std::getline(src, line);

						if (std::regex_search(line, match, shader_regex_in))
						{
							StageInfo::InOut in(match[2].str(), match[1].str());
							if (s.type == ShaderStageVert)
							{
								auto location = 0;
								if (vi)
								{
									for (auto _b : vi->buffers)
									{
										const auto& b = *(VertexInputBuffer*)_b;
										for (auto _a : b.attributes)
										{
											const auto& a = *(VertexInputAttribute*)_a;
											if (a.name == in.name)
											{
												glsl_file << "layout (location = " + std::to_string(location) + +") in " + in.formated_type + " i_" + in.name + ";\n";
												location = -1;
												break;
											}
											location++;
										}
										if (location == -1)
											break;
									}
								}
							}
							else
							{
								auto& ps = stage_infos[i - 1];
								for (auto j = 0; j < ps.outputs.size(); j++)
								{
									if (ps.outputs[j].name == in.name)
									{
										glsl_file << "layout (location = " + std::to_string(j) + +") in " + in.formated_type + " i_" + in.name + ";\n";
										break;
									}
								}
								s.inputs.push_back(in);
							}
						}
						else if (std::regex_search(line, match, shader_regex_out))
						{
							StageInfo::InOut out(match[2].str(), match[1].str());
							auto dual = false;
							if (s.type == ShaderStageFrag)
							{
								if (match[3].matched)
								{
									out.blend_enable = true;
									auto sp = string_split(match[4].str());
									for (auto& p : sp)
									{
										auto sp = string_split(p, ':');
										BlendFactor$ f;
										if (sp[1] == "0")
											f = BlendFactorZero;
										else if (sp[1] == "1")
											f = BlendFactorOne;
										else if (sp[1] == "sa")
											f = BlendFactorSrcAlpha;
										else if (sp[1] == "1msa")
											f = BlendFactorOneMinusSrcAlpha;
										else if (sp[1] == "s1c")
										{
											dual = true;
											f = BlendFactorSrc1Color;
										}
										else if (sp[1] == "1ms1c")
										{
											dual = true;
											f = BlendFactorOneMinusSrc1Color;
										}
										else
											continue;
										if (sp[0] == "sc")
											out.blend_src_color = f;
										else if (sp[0] == "dc")
											out.blend_dst_color = f;
										else if (sp[0] == "sa")
											out.blend_src_alpha = f;
										else if (sp[0] == "da")
											out.blend_dst_alpha = f;
									}
								}
							}
							if (dual)
							{
								glsl_file << "layout (location = " + std::to_string((int)s.outputs.size()) + +", index = 0) out " + out.formated_type + " o_" + out.name + "0;\n";
								glsl_file << "layout (location = " + std::to_string((int)s.outputs.size()) + +", index = 1) out " + out.formated_type + " o_" + out.name + "1;\n";
							}
							else
								glsl_file << "layout (location = " + std::to_string((int)s.outputs.size()) + +") out " + out.formated_type + " o_" + out.name + ";\n";
							s.outputs.push_back(out);
						}
						else if (std::regex_search(line, match, shader_regex_pc))
						{
							if (pll && pll->pc_size > 0)
								glsl_file << "layout (push_constant) uniform pc_t\n";
							else
								glsl_file << "struct type_" + std::to_string(type_id++) + "\n";
						}
						else if (std::regex_search(line, match, shader_regex_ubo))
						{
							auto set = 0;
							auto binding = -1;
							if (pll)
							{
								auto name = match[1].str();
								for (auto j = 0; j < pll->dsls.size(); j++)
								{
									auto dsl = pll->dsls[j];
									for (auto k = 0; k < dsl->bindings.size(); k++)
									{
										if (dsl->bindings[k]->name == name)
										{
											set = j;
											binding = k;
											glsl_file << "layout (set = " + std::to_string(set) + ", binding = "+ std::to_string(binding) + ") uniform type_" + std::to_string(type_id) + "\n";
											break;
										}
									}
									if (binding != -1)
										break;
								}
							}
							if (binding == -1)
								glsl_file << "struct eliminate_" + std::to_string(type_id) + "\n";
							type_id++;
						}
						else if (std::regex_search(line, match, shader_regex_tex))
						{
							auto set = 0;
							auto binding = -1;
							if (pll)
							{
								auto name = match[1].str();
								for (auto j = 0; j < pll->dsls.size(); j++)
								{
									auto dsl = pll->dsls[j];
									for (auto k = 0; k < dsl->bindings.size(); k++)
									{
										if (dsl->bindings[k]->name == name)
										{
											set = j;
											binding = k;
											glsl_file << "layout (set = " + std::to_string(set) + ", binding = " + std::to_string(binding) + ") uniform sampler2D " + name + (match[2].matched ? match[2].str() : "") + ";\n";
											break;
										}
									}
									if (binding != -1)
										break;
								}
							}
						}
						else
							glsl_file << line + "\n";
					}

				}
				glsl_file.close();

				if (std::filesystem::exists(L"out.spv"))
					std::filesystem::remove(L"out.spv");

				auto vk_sdk_path = s2w(getenv("VK_SDK_PATH"));
				assert(vk_sdk_path != L"");

				std::wstring command_line(L" -fshader-stage=" + shader_stage_name(s.type) + L" out.glsl -flimit-file shader.conf -o out.spv");

				auto output1 = exec_and_get_output((vk_sdk_path + L"/Bin/glslc.exe"), command_line);
				std::filesystem::remove(L"out.glsl");
				if (!std::filesystem::exists(L"out.spv"))
				{
					printf("shader \"%s\" compile error:\n%s\n", w2s(s.filename).c_str(), output1.p->c_str());
					printf("trying to use fallback");

					std::ofstream glsl_file(L"out.glsl");
					glsl_file << glsl_header;
					switch (s.type)
					{
					case ShaderStageVert:
						glsl_file << "void main()\n"
							"{\n"
							"\tgl_Position = vec4(0, 0, 0, 1);"
							"}\n";
						break;
					case ShaderStageFrag:
						glsl_file <<
							"void main()\n"
							"{\n"
							"}\n";
						break;
					default:
						assert(0); // WIP
					}
					glsl_file.close();

					auto output2 = exec_and_get_output((vk_sdk_path + L"/Bin/glslc.exe"), command_line);
					std::filesystem::remove(L"out.glsl");
					if (!std::filesystem::exists(L"out.spv"))
					{
						printf(" - failed\n error:\n%s", output2.p->c_str());
						assert(0);
					}
					delete_mail(output2);

					printf(" - ok\n");
				}
				delete_mail(output1);

				auto spv_file = get_file_content(L"out.spv");
				if (!spv_file.first)
					assert(0);

				if (std::filesystem::exists(L"out.spv"))
					std::filesystem::remove(L"out.spv");

#if defined(FLAME_VULKAN)
				VkShaderModuleCreateInfo shader_info;
				shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				shader_info.flags = 0;
				shader_info.pNext = nullptr;
				shader_info.codeSize = spv_file.second;
				shader_info.pCode = (uint*)spv_file.first.get();
				chk_res(vkCreateShaderModule(d->v, &shader_info, nullptr, &s.vk_shader_module));
#elif defined(FLAME_D3D12)

#endif
				//spirv_cross::CompilerGLSL compiler((uint*)spv_file.first.get(), spv_file.second / sizeof(uint));
				//auto resources = compiler.get_shader_resources();

				//std::function<void(uint, Variable*)> get_v;
				//get_v = [&](uint type_id, Variable* v) {
				//	const auto* t = &compiler.get_type(type_id);
				//	while (t->pointer)
				//	{
				//		type_id = t->parent_type;
				//		t = &compiler.get_type(type_id);
				//	}

				//	assert(t->array.size() <= 1); // no support multidimensional array
				//	v->count = t->array.empty() ? 1 : t->array[0];
				//	if (t->array.empty())
				//		v->array_stride = 0;
				//	else
				//		v->array_stride = compiler.get_decoration(type_id, spv::DecorationArrayStride);

				//	if (t->basetype == spirv_cross::SPIRType::Struct)
				//	{
				//		v->type_name = compiler.get_name(type_id);
				//		v->size = compiler.get_declared_struct_size(*t);
				//		for (auto i = 0; i < t->member_types.size(); i++)
				//		{
				//			auto m = new Variable;
				//			m->name = compiler.get_member_name(type_id, i);
				//			m->offset = compiler.type_struct_member_offset(*t, i);
				//			m->size = compiler.get_declared_struct_member_size(*t, i);
				//			v->members.emplace_back(m);
				//			get_v(t->member_types[i], m);
				//		}
				//}
				//	else
				//	{
				//		std::string base_name;
				//		switch (t->basetype)
				//		{
				//		case spirv_cross::SPIRType::SByte:
				//			base_name = "char";
				//			break;
				//		case spirv_cross::SPIRType::UByte:
				//			base_name = "uchar";
				//			break;
				//		case spirv_cross::SPIRType::Short:
				//			base_name = "short";
				//			break;
				//		case spirv_cross::SPIRType::UShort:
				//			base_name = "ushort";
				//			break;
				//		case spirv_cross::SPIRType::Int:
				//			base_name = "int";
				//			break;
				//		case spirv_cross::SPIRType::UInt:
				//			base_name = "uint";
				//			break;
				//		case spirv_cross::SPIRType::Float:
				//			base_name = "float";
				//			break;
				//		case spirv_cross::SPIRType::SampledImage:
				//			base_name = "SampledImage";
				//			break;
				//		default:
				//			assert(0);
				//		}
				//		if (t->columns <= 1)
				//		{
				//			if (t->vecsize <= 1)
				//				v->type_name = base_name;
				//			else
				//				v->type_name = "Vec(" + std::to_string(t->vecsize) + "+" + base_name + ")";
				//		}
				//		else
				//			v->type_name = "Mat(" + std::to_string(t->vecsize) + "+" + std::to_string(t->columns) + "+" + base_name + ")";
				//	}
				//	};

				//for (auto& src : resources.uniform_buffers)
				//{
				//	auto r = new Resource;
				//	r->set = compiler.get_decoration(src.id, spv::DecorationDescriptorSet);
				//	r->binding = compiler.get_decoration(src.id, spv::DecorationBinding);
				//	r->name = src.name;
				//	get_v(src.type_id, &r->v);
				//	uniform_buffers.emplace_back(r);
				//}

				//assert(resources.push_constant_buffers.size() <= 1);
				//if (!resources.push_constant_buffers.empty())
				//{
				//	auto& src = resources.push_constant_buffers[0];
				//	push_constant.reset(new Resource);
				//	push_constant->name = src.name;

				//	get_v(src.type_id, &push_constant->v);
				//}
			}
		}

		PipelinePrivate::PipelinePrivate(DevicePrivate* d, const std::vector<StageInfo>& stage_infos, PipelinelayoutPrivate* pll, Renderpass* rp, uint subpass_idx, VertexInputInfo* vi, const Vec2u& vp, RasterInfo* raster, SampleCount$ sc, DepthInfo* depth, const std::vector<uint>& dynamic_states) :
			d(d),
			pll(pll)
		{
			type = PipelineGraphics;

#if defined(FLAME_VULKAN)
			std::vector<VkPipelineShaderStageCreateInfo> vk_stage_infos;
			std::vector<VkVertexInputAttributeDescription> vk_vi_attributes;
			std::vector<VkVertexInputBindingDescription> vk_vi_bindings;
			std::vector<VkPipelineColorBlendAttachmentState> vk_blend_attachment_states;
			std::vector<VkDynamicState> vk_dynamic_states;

			vk_stage_infos.resize(stage_infos.size());
			for (auto i = 0; i < stage_infos.size(); i++)
			{
				auto& src = stage_infos[i];
				auto& dst = vk_stage_infos[i];
				vk_shader_modules.push_back(src.vk_shader_module);
				dst.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				dst.flags = 0;
				dst.pNext = nullptr;
				dst.pSpecializationInfo = nullptr;
				dst.pName = "main";
				dst.stage = to_enum(src.type);
				dst.module = src.vk_shader_module;
			}

			if (vi)
			{
				auto attribute_location = 0;
				vk_vi_bindings.resize(vi->buffers.size());
				for (auto i = 0; i < vi->buffers.size(); i++)
				{
					const auto& src = *(VertexInputBuffer*)vi->buffers[i];
					auto& dst = vk_vi_bindings[i];
					dst.binding = i;
					for (auto a : src.attributes)
					{
						const auto& _src = *(VertexInputAttribute*)a;
						VkVertexInputAttributeDescription _dst;
						_dst.location = attribute_location++;
						_dst.binding = i;
						_dst.offset = dst.stride;
						dst.stride += format_size(_src.format);
						_dst.format = to_enum(_src.format);
						vk_vi_attributes.push_back(_dst);
					}
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
			assembly_state.topology = to_enum(vi ? vi->primitive_topology : PrimitiveTopologyTriangleList);
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
			raster_state.polygonMode = to_enum(raster ? raster->polygon_mode : PolygonModeFill);
			raster_state.cullMode = to_enum(raster ? raster->cull_mode : CullModeNone);
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
			multisample_state.rasterizationSamples = to_enum(sc);
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
			depth_stencil_state.depthCompareOp = to_enum(depth ? depth->compare_op : CompareOpLess);
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
			if (stage_infos.back().type == ShaderStageFrag)
			{
				auto& outputs = stage_infos.back().outputs;
				for (auto i = 0; i < outputs.size(); i++)
				{
					const auto& src = outputs[i];
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

			for (auto& s : dynamic_states)
				vk_dynamic_states.push_back(to_enum((DynamicState)s));
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
			pipeline_info.layout = pll->v;
			pipeline_info.renderPass = rp ? ((RenderpassPrivate*)rp)->v : nullptr;
			pipeline_info.subpass = subpass_idx;
			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;

			chk_res(vkCreateGraphicsPipelines(d->v, 0, 1, &pipeline_info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		PipelinePrivate::PipelinePrivate(DevicePrivate* d, const StageInfo& compute_shader_info, PipelinelayoutPrivate* pll) :
			d(d),
			pll(pll)
		{
			type = PipelineCompute;

#if defined(FLAME_VULKAN)
			vk_shader_modules.push_back(compute_shader_info.vk_shader_module);

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
			pipeline_info.stage.module = compute_shader_info.vk_shader_module;

			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;
			pipeline_info.layout = pll->v;

			chk_res(vkCreateComputePipelines(d->v, 0, 1, &pipeline_info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		PipelinePrivate::~PipelinePrivate()
		{
#if defined(FLAME_VULKAN)
			for (auto v : vk_shader_modules)
				vkDestroyShaderModule(d->v, v, nullptr);

			vkDestroyPipeline(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Pipeline* Pipeline::create(Device* d, const std::vector<std::wstring>& shader_filenames, Pipelinelayout* pll, Renderpass* rp, uint subpass_idx, VertexInputInfo* vi, const Vec2u& vp, RasterInfo* raster, SampleCount$ sc, DepthInfo* depth, const std::vector<uint>& dynamic_states)
		{
			std::vector<StageInfo> stage_infos;
			auto has_vert_stage = false;
			auto tess_stages_count = 0;
			for (auto& f : shader_filenames)
			{
				StageInfo info(f);

				auto sp = string_split(f, L':');
				if (!std::filesystem::exists(info.path))
					return nullptr;
				for (auto& s : stage_infos)
				{
					if (s.path == info.path || s.type == info.type)
						return nullptr;
				}
				if (info.type == ShaderStageComp)
					return nullptr;
				if (info.type == ShaderStageVert)
					has_vert_stage = true;
				if (info.type == ShaderStageTesc || info.type == ShaderStageTese)
					tess_stages_count++;

				stage_infos.push_back(std::move(info));
			}
			if (!has_vert_stage || (tess_stages_count != 0 && tess_stages_count != 2))
				return nullptr;
			std::sort(stage_infos.begin(), stage_infos.end(), [](const auto& a, const auto& b) {
				return (int)a.type < (int)b.type;
			});

			compile_shaders((DevicePrivate*)d, stage_infos, (PipelinelayoutPrivate*)pll, vi);

			return new PipelinePrivate((DevicePrivate*)d, stage_infos, (PipelinelayoutPrivate*)pll, rp, subpass_idx, vi, vp, raster, sc, depth, dynamic_states);
		}

		Pipeline* Pipeline::create(Device* d, const std::wstring& compute_shader_filename, Pipelinelayout* pll)
		{
			StageInfo compute_stage_info(compute_shader_filename);
			if (!std::filesystem::exists(compute_stage_info.path) || compute_stage_info.type != ShaderStageComp)
				return nullptr;

			std::vector<StageInfo> stage_infos;
			stage_infos.push_back(std::move(compute_stage_info));
			compile_shaders((DevicePrivate*)d, stage_infos, (PipelinelayoutPrivate*)pll, nullptr);

			return new PipelinePrivate((DevicePrivate*)d, compute_stage_info, (PipelinelayoutPrivate*)pll);
		}

		void Pipeline::destroy(Pipeline* p)
		{
			delete (PipelinePrivate*)p;
		}

		struct Pipeline$
		{
			AttributeP<std::vector<std::wstring>> shader_filenames$i;
			AttributeP<void> pll$i;
			AttributeP<void> renderpass$i;
			AttributeD<uint> subpass_idx$i;
			AttributeP<void> vi$i;
			AttributeD<Vec2u> vp$i;
			AttributeP<void> raster$i;
			AttributeE<SampleCount$> sc$i;
			AttributeP<void> depth$i;
			AttributeP<std::vector<uint>> dynamic_states$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS Pipeline$()
			{
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (renderpass$i.b.frame > out$o.b.frame || subpass_idx$i.b.frame > out$o.b.frame || shader_filenames$i.b.frame > out$o.b.frame || pll$i.b.frame > out$o.b.frame ||
					vi$i.b.frame > out$o.b.frame || vp$i.b.frame > out$o.b.frame || raster$i.b.frame > out$o.b.frame || sc$i.b.frame > out$o.b.frame || depth$i.b.frame > out$o.b.frame || dynamic_states$i.b.frame > out$o.b.frame)
				{
					if (out$o.v)
						Pipeline::destroy((Pipeline*)out$o.v);
					auto d = Device::default_one();
					if (d && renderpass$i.v && ((Renderpass*)renderpass$i.v)->subpass_count() > subpass_idx$i.v&& shader_filenames$i.v && !shader_filenames$i.v->empty() && pll$i.v)
					{
						auto filenames = *shader_filenames$i.v;
						for (auto& fn : filenames)
							fn = std::filesystem::path(bp_env().filename).parent_path().wstring() + L"/" + fn;
						out$o.v = Pipeline::create(d, filenames, (Pipelinelayout*)pll$i.v, (Renderpass*)renderpass$i.v, subpass_idx$i.v,
							(VertexInputInfo*)vi$i.v, vp$i.v, (RasterInfo*)raster$i.v, sc$i.v, (DepthInfo*)depth$i.v, dynamic_states$i.v ? *dynamic_states$i.v : std::vector<uint>());
					}
					else
					{
						printf("cannot create pipeline\n");

						out$o.v = nullptr;
					}
					out$o.b.frame = looper().frame;
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
