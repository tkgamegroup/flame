#include <flame/foundation/blueprint.h>
#include <flame/foundation/typeinfo.h>
#include "device_private.h"
#include "renderpass_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "shader_private.h"

#if defined(FLAME_VULKAN)
#include <spirv_glsl.hpp>
#endif

#include <flame/reflect_macros.h>

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
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128},
				{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 32},
			};

			VkDescriptorPoolCreateInfo descriptorPoolInfo;
			descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			descriptorPoolInfo.pNext = nullptr;
			descriptorPoolInfo.poolSizeCount = array_size(descriptorPoolSizes);
			descriptorPoolInfo.pPoolSizes = descriptorPoolSizes;
			descriptorPoolInfo.maxSets = 8;
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

		DescriptorlayoutPrivate::DescriptorlayoutPrivate(Device* _d, uint binding_count, DescriptorBinding* const* _bindings, bool create_default) :
			d((DevicePrivate*)_d)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
			bindings.resize(binding_count);
			for (auto i = 0; i < binding_count; i++)
			{
				auto& b = *_bindings[i];
				bindings[i] = b;

				VkDescriptorSetLayoutBinding vk_binding;
				vk_binding.binding = i;
				vk_binding.descriptorType = to_backend(b.type);
				vk_binding.descriptorCount = b.count;
				vk_binding.stageFlags = to_backend_flags<ShaderStage>(ShaderStageAll);
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
			if (create_default)
			{
				default_set = Descriptorset::create(d->dp, this);
				for (auto i = 0; i < bindings.size(); i++)
				{
					auto& b = bindings[i];
					auto type = b.type;
					if (type == DescriptorUniformBuffer || type == DescriptorStorageBuffer)
					{
						if (b.buffer)
						{
							for (auto j = 0; j < b.count; j++)
								default_set->set_buffer(i, j, b.buffer);
						}
					}
					else if (type == DescriptorSampledImage || type == DescriptorStorageImage)
					{
						if (b.view)
						{
							for (auto j = 0; j < b.count; j++)
								default_set->set_image(i, j, b.view, b.sampler);
						}
					}
				}
			}
			else
				default_set = nullptr;

			hash = 0;
			for (auto& b : bindings)
			{
				hash_update(hash, b.type);
				hash_update(hash, FLAME_HASH(b.name));
				hash_update(hash, b.count);
			}
		}

		DescriptorlayoutPrivate::~DescriptorlayoutPrivate()
		{
			if (default_set)
				Descriptorset::destroy(default_set);
#if defined(FLAME_VULKAN)
			vkDestroyDescriptorSetLayout(((DevicePrivate*)d)->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		uint Descriptorlayout::binding_count() const
		{
			return ((DescriptorlayoutPrivate*)this)->bindings.size();
		}

		const DescriptorBinding& Descriptorlayout::get_binding(uint binding) const
		{
			return ((DescriptorlayoutPrivate*)this)->bindings[binding];
		}

		Descriptorset* Descriptorlayout::default_set() const
		{
			return ((DescriptorlayoutPrivate*)this)->default_set;
		}

		Descriptorlayout* Descriptorlayout::create(Device* d, uint binding_count, DescriptorBinding* const* bindings, bool create_default_set)
		{
			return new DescriptorlayoutPrivate(d, binding_count, bindings, create_default_set);
		}

		void Descriptorlayout::destroy(Descriptorlayout* l)
		{
			delete (DescriptorlayoutPrivate*)l;
		}

		struct R(R_DescriptorBinding)
		{
			BP::Node* n;

			BASE0;
			RV(DescriptorType, type, i);
			RV(uint, count, i);
			RV(StringA, name, i);
			RV(Buffer*, buffer, i);
			RV(TargetType, target_type, i);
			RV(void*, v, i);

			BASE1;
			RV(DescriptorBinding, out, o);
			RV(Imageview*, iv, o);

			FLAME_GRAPHICS_EXPORTS RF(R_DescriptorBinding)()
			{
				count = 1;
			}

			FLAME_GRAPHICS_EXPORTS void RF(update)(uint frame)
			{
				auto out_frame = out_s()->frame();
				auto out_updated = false;
				if (out_frame == -1)
				{
					auto d = Device::default_one();
					if (d)
						out.sampler = d->sp_linear;
					out_updated = true;
				}
				auto iv_frame = iv_s()->frame();
				if (target_type_s()->frame() > iv_frame || v_s()->frame() > iv_frame)
				{
					if (iv)
						Imageview::destroy(iv);
					if (target_type == TargetImage && v)
						iv = Imageview::create((Image*)v);
					else
						iv = nullptr;
					iv_s()->set_frame(frame);
				}
				if (type_s()->frame() > out_frame)
				{
					out.type = type;
					out_updated = true;
				}
				if (count_s()->frame() > out_frame)
				{
					out.count = count;
					out_updated = true;
				}
				if (name_s()->frame() > out_frame)
				{
					out.name = name.v;
					out_updated = true;
				}
				if (buffer_s()->frame() > out_frame)
				{
					out.buffer = buffer;
					out_updated = true;
				}
				if (iv_s()->frame() > out_frame)
				{
					switch (target_type)
					{
					case TargetImage:
						out.view = iv;
						break;
					case TargetImageview:
						out.view = (Imageview*)v;
						break;
					case TargetImages:
						out.view = nullptr;
						break;
					}
					out_updated = true;
				}
				if (out_updated)
					out_s()->set_frame(frame);
			}

			FLAME_GRAPHICS_EXPORTS RF(~R_DescriptorBinding)()
			{
				if (iv)
					Imageview::destroy(iv);
			}
		};

		struct R(R_Descriptorlayout)
		{
			BP::Node* n;

			BASE0;
			RV(Array<DescriptorBinding*>*, bindings, i);
			RV(bool, create_default_set, i);

			BASE1;
			RV(Descriptorlayout*, out, o);
			RV(Descriptorset*, default_set, o);

			FLAME_GRAPHICS_EXPORTS RF(R_Descriptorlayout)()
			{
				create_default_set = true;
			}

			FLAME_GRAPHICS_EXPORTS void RF(update)(uint frame)
			{
				if (bindings_s()->frame() > out_s()->frame() || create_default_set_s()->frame() > default_set_s()->frame())
				{
					if (out)
						Descriptorlayout::destroy(out);
					auto d = Device::default_one();
					if (d && bindings->s > 0)
					{
						out = Descriptorlayout::create(d, bindings->s, bindings->v, create_default_set);
						default_set = (out)->default_set();
					}
					else
					{
						printf("cannot create descriptorlayout\n");

						out = nullptr;
						default_set = nullptr;
					}
					out_s()->set_frame(frame);
					default_set_s()->set_frame(frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS RF(~R_Descriptorlayout)()
			{
				if (out)
					Descriptorlayout::destroy(out);
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
			write.descriptorType = to_backend(l->bindings[binding].type);
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
			write.descriptorType = to_backend(l->bindings[binding].type);
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

		struct R(R_Descriptorset)
		{
			BP::Node* n;

			BASE0;
			RV(Descriptorlayout*, dsl, i);

			BASE1;
			RV(Descriptorset*, out, o);

			FLAME_GRAPHICS_EXPORTS void RF(update)(uint frame)
			{
				if (dsl_s()->frame() > out_s()->frame())
				{
					if (out)
						Descriptorset::destroy(out);
					auto d = Device::default_one();
					if (d && dsl)
						out = Descriptorset::create(d->dp, dsl);
					else
					{
						printf("cannot create descriptorsetset\n");

						out = nullptr;
					}
					out_s()->set_frame(frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS RF(~R_Descriptorset)()
			{
				if (out)
					Descriptorset::destroy((Descriptorset*)out);
			}
		};

		struct DescriptorWrite
		{
			uint binding;
			uint index;
			uint count;
			Buffer* buffer;
			Imageview* view;
		};

		struct R(R_DescriptorWrite)
		{
			BP::Node* n;

			BASE0;
			RV(uint, binding, i);
			RV(uint, index, i);
			RV(uint, count, i);
			RV(Buffer*, buffer, i);
			RV(TargetType, target_type, i);
			RV(void*, v, i);

			BASE1;
			RV(DescriptorWrite, out, o);
			RV(Imageview*, iv, o);

			FLAME_GRAPHICS_EXPORTS RF(R_DescriptorWrite)()
			{
				count = 1;
			}

			FLAME_GRAPHICS_EXPORTS void RF(update)(uint frame)
			{
				auto iv_frame = iv_s()->frame();
				if (target_type_s()->frame() > iv_frame || v_s()->frame() > iv_frame)
				{
					if (iv)
						Imageview::destroy(iv);
					if (target_type == TargetImage)
						iv = Imageview::create((Image*)v);
					else
						iv = nullptr;
					iv_s()->set_frame(frame);
				}
				auto out_frame = out_s()->frame();
				auto out_updated = false;
				if (count_s()->frame() > out_frame)
				{
					out.count = count;
					out_updated = true;
				}
				if (buffer_s()->frame() > out_frame)
				{
					out.buffer = buffer;
					out_updated = true;
				}
				if (iv_s()->frame() > out_frame)
				{
					switch (target_type)
					{
					case TargetImage:
						out.view = iv;
						break;
					case TargetImageview:
						out.view = (Imageview*)v;
						break;
					case TargetImages:
						out.view = nullptr;
						break;
					}
					out_updated = true;
				}
				if (out_updated)
					out_s()->set_frame(frame);
			}

			FLAME_GRAPHICS_EXPORTS RF(~R_DescriptorWrite)()
			{
				if (iv)
					Imageview::destroy(iv);
			}
		};

		struct R(R_DescriptorWriter)
		{
			BP::Node* n;

			BASE0;
			RV(Descriptorset*, in, i);
			RV(Array<DescriptorWrite*>*, writes, i);

			BASE1;
			RV(Descriptorset*, out, o);

			FLAME_GRAPHICS_EXPORTS void RF(update)(uint frame)
			{
				auto out_frame = out_s()->frame();
				if (in_s()->frame() > out_frame || writes_s()->frame() > out_frame)
				{
					if (writes)
					{
						auto sampler = Device::default_one()->sp_linear;
						for (auto i = 0; i < writes->s; i++)
						{
							auto& w = writes->at(i);
							if (w->buffer)
							{
								for (auto j = 0; j < w->count; j++)
									in->set_buffer(w->binding, w->index + j, w->buffer);
							}
							else
							{
								for (auto j = 0; j < w->count; j++)
									in->set_image(w->binding, w->index + j, w->view, sampler);
							}
						}
					}

					out = in;
					out_s()->set_frame(frame);
				}
			}
		};

		PipelinelayoutPrivate::PipelinelayoutPrivate(Device* d, uint descriptorlayout_count, Descriptorlayout* const* descriptorlayouts, uint push_constant_size) :
			d((DevicePrivate*)d),
			pc_size(push_constant_size)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkDescriptorSetLayout> vk_descriptorsetlayouts;
			vk_descriptorsetlayouts.resize(descriptorlayout_count);
			for (auto i = 0; i < descriptorlayout_count; i++)
				vk_descriptorsetlayouts[i] = ((DescriptorlayoutPrivate*)descriptorlayouts[i])->v;

			VkPushConstantRange vk_pushconstant;
			vk_pushconstant.offset = 0;
			vk_pushconstant.size = pc_size;
			vk_pushconstant.stageFlags = to_backend_flags<ShaderStage>(ShaderStageAll);

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

			dsls.resize(descriptorlayout_count);
			for (auto i = 0; i < dsls.size(); i++)
				dsls[i] = (DescriptorlayoutPrivate*)descriptorlayouts[i];

			hash = 0;
			for (auto& d : dsls)
				hash_update(hash, d->hash);
			hash_update(hash, pc_size);
		}

		PipelinelayoutPrivate::~PipelinelayoutPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyPipelineLayout(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Pipelinelayout* Pipelinelayout::create(Device* d, uint descriptorlayout_count, Descriptorlayout* const* descriptorlayouts, uint push_constant_size)
		{
			return new PipelinelayoutPrivate(d, descriptorlayout_count, descriptorlayouts, push_constant_size);
		}

		void Pipelinelayout::destroy(Pipelinelayout* l)
		{
			delete (PipelinelayoutPrivate*)l;
		}

		struct R(R_Pipelinelayout)
		{
			BP::Node* n;

			BASE0;
			RV(Array<Descriptorlayout*>*, descriptorlayouts, i);
			RV(uint, push_constant_size, i);

			BASE1;
			RV(Pipelinelayout*, out, o);

			FLAME_GRAPHICS_EXPORTS void RF(update)(uint frame)
			{
				auto out_frame = out_s()->frame();
				if (descriptorlayouts_s()->frame() > out_frame || push_constant_size_s()->frame() > out_frame)
				{
					if (out)
						Pipelinelayout::destroy(out);
					auto d = Device::default_one();
					if (d)
						out = Pipelinelayout::create(d, descriptorlayouts ? descriptorlayouts->s : 0, descriptorlayouts ? descriptorlayouts->v : nullptr, push_constant_size);
					else
					{
						printf("cannot create pipelinelayout\n");

						out = nullptr;
					}
					out_s()->set_frame(frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS RF(~R_Pipelinelayout)()
			{
				if (out)
					Pipelinelayout::destroy(out);
			}
		};

		struct R(R_VertexInputAttribute)
		{
			BP::Node* n;

			BASE0;
			RV(StringA, name, i);
			RV(Format, format, i);

			BASE1;
			RV(VertexInputAttribute, out, o);

			FLAME_GRAPHICS_EXPORTS RF(R_VertexInputAttribute)()
			{
				format = Format_R8G8B8A8_UNORM;
			}

			FLAME_GRAPHICS_EXPORTS void RF(update)(uint frame)
			{
				auto out_frame = out_s()->frame();
				auto out_updated = false;
				if (name_s()->frame() > out_frame)
				{
					out.name = name.v;
					out_updated = true;
				}
				if (format_s()->frame() > out_frame)
				{
					out.format = format;
					out_updated = true;
				}
				if (out_updated)
					out_s()->set_frame(frame);
			}
		};

		struct R(R_VertexInputBuffer)
		{
			BP::Node* n;

			BASE0;
			RV(Array<VertexInputAttribute*>*, attributes, i);
			RV(VertexInputRate, rate, i);

			BASE1;
			RV(VertexInputBuffer, out, o);

			FLAME_GRAPHICS_EXPORTS RF(R_VertexInputBuffer)()
			{
				rate = VertexInputRateVertex;
			}

			FLAME_GRAPHICS_EXPORTS void RF(update)(uint frame)
			{
				auto out_frame = out_s()->frame();
				auto out_updated = false;
				if (attributes_s()->frame() > out_frame)
				{
					out.attribute_count = attributes ? attributes->s : 0;
					out.attributes = attributes ? attributes->v : nullptr;
					out_updated = true;
				}
				if (rate_s()->frame() > out_frame)
				{
					out.rate = rate;
					out_updated = true;
				}
				if (out_updated)
					out_s()->set_frame(frame);
			}
		};

		struct R(R_VertexInputInfo)
		{
			BP::Node* n;

			BASE0;
			RV(Array<VertexInputBuffer*>*, buffers, i);
			RV(PrimitiveTopology, primitive_topology, i);
			RV(uint, patch_control_points, i);

			BASE1;
			RV(VertexInputInfo, out, o);

			FLAME_GRAPHICS_EXPORTS RF(R_VertexInputInfo)()
			{
				primitive_topology = PrimitiveTopologyTriangleList;
			}

			FLAME_GRAPHICS_EXPORTS void RF(update)(uint frame)
			{
				auto out_frame = out_s()->frame();
				auto out_updated = false;
				if (buffers_s()->frame() > out_frame)
				{
					out.buffer_count = buffers ? buffers->s : 0;
					out.buffers = buffers ? buffers->v : nullptr;
					out_updated = true;
				}
				if (primitive_topology_s()->frame() > out_frame)
				{
					out.primitive_topology = primitive_topology;
					out_updated = true;
				}
				if (patch_control_points_s()->frame() > out_frame)
				{
					out.patch_control_points = patch_control_points;
					out_updated = true;
				}
				if (out_updated)
					out_s()->set_frame(frame);
			}
		};

		static BP* execution_context = nullptr;

		bool compile_shaders(DevicePrivate* d, const std::filesystem::path& dir, std::vector<StageInfo>& stage_infos, PipelinelayoutPrivate* pll, const VertexInputInfo* vi)
		{
			const std::regex regex_in(R"(\s*in\s+([\w]+)\s+i_([\w]+)\s*;)");
			const std::regex regex_out(R"(\s*out\s+([\w]+)\s+o_([\w]+)(\{([\w:\s]+)\})?\s*;)");
			const std::regex regex_pc(R"(\s*pushconstant)");
			const std::regex regex_ubo(R"(\s*uniform\s+([\w]+))");
			const std::regex regex_tex(R"(\s*sampler2D\s+([\w]+)([\[\]0-9\s]+)?;)");

			auto hash = 0U;
			for (auto& s : stage_infos)
			{
				hash = hash_update(hash, FLAME_HASH(s.path.c_str()));
				hash = hash_update(hash, FLAME_HASH(s.prefix.c_str()));
			}
			hash = hash_update(hash, pll->hash);
			if (vi)
			{
				for (auto i = 0; i < vi->buffer_count; i++)
				{
					auto b = vi->buffers[i];
					for (auto j = 0; j < b->attribute_count; j++)
					{
						auto a = b->attributes[j];
						hash = hash_update(hash, a->format);
						hash = hash_update(hash, FLAME_HASH(a->name));
					}
					hash = hash_update(hash, b->rate);
				}
				hash = hash_update(hash, vi->primitive_topology);
				hash = hash_update(hash, vi->patch_control_points);
			}
			auto str_hash = std::to_wstring(hash);

			for (auto i = 0; i < stage_infos.size(); i++)
			{
				auto& s = stage_infos[i];

				std::filesystem::path spv_path = dir / s.path;
				spv_path += L".";
				spv_path += str_hash;
				auto res_path = spv_path;
				spv_path += L".spv";
				res_path += L".res";

				auto ok = true;
				auto shader_path = dir / s.path;
				if (std::filesystem::exists(shader_path) && (!std::filesystem::exists(spv_path) || std::filesystem::last_write_time(spv_path) < std::filesystem::last_write_time(shader_path)))
				{
					wprintf(L"begin compiling shader:%s\n", shader_path.c_str());

					std::ifstream src(shader_path);

					std::string glsl_header = "#version 450 core\n"
						"#extension GL_ARB_shading_language_420pack : enable\n";
					if (s.type != ShaderStageComp)
						glsl_header += "#extension GL_ARB_separate_shader_objects : enable\n";
					glsl_header += s.prefix + "\n";

					auto type_id = 0;

					printf("generating glsl file");
					std::ofstream glsl_file(L"out.glsl");
					glsl_file << glsl_header;
					{
						std::string line;
						std::smatch res;
						while (!src.eof())
						{
							std::getline(src, line);

							auto get_formated_type = [](const std::string& type) {
								auto ret = type;
								if (ret[0] == 'i' || ret[0] == 'u')
									ret = "flat " + ret;
								return ret;
							};

							if (std::regex_search(line, res, regex_in))
							{
								StageInfo::InOut in;
								in.type = res[1].str();
								in.name = res[2].str();
								if (s.type == ShaderStageVert)
								{
									auto location = 0;
									if (vi)
									{
										for (auto i = 0; i < vi->buffer_count; i++)
										{
											const auto& b = *vi->buffers[i];
											for (auto j = 0; j < b.attribute_count; j++)
											{
												const auto& a = *b.attributes[j];
												if (a.name == in.name)
												{
													glsl_file << "layout (location = " + std::to_string(location) + +") in " + get_formated_type(in.type) + " i_" + in.name + ";\n";
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
											glsl_file << "layout (location = " + std::to_string(j) + +") in " + get_formated_type(in.type) + " i_" + in.name + ";\n";
											break;
										}
									}
								}
								s.inputs.push_back(in);
							}
							else if (std::regex_search(line, res, regex_out))
							{
								StageInfo::InOut out;
								out.type = res[1].str();
								out.name = res[2].str();
								auto dual = false;
								if (s.type == ShaderStageFrag)
								{
									BlendOptions bo;
									if (res[3].matched)
									{
										bo.enable = true;
										auto sp = SUS::split(res[4].str());
										for (auto& p : sp)
										{
											auto sp = SUS::split(p, ':');
											BlendFactor f;
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
												bo.src_color = f;
											else if (sp[0] == "dc")
												bo.dst_color = f;
											else if (sp[0] == "sa")
												bo.src_alpha = f;
											else if (sp[0] == "da")
												bo.dst_alpha = f;
										}
									}
									s.blend_options.push_back(bo);
								}
								if (dual)
								{
									glsl_file << "layout (location = " + std::to_string((int)s.outputs.size()) + +", index = 0) out " + get_formated_type(out.type) + " o_" + out.name + "0;\n";
									glsl_file << "layout (location = " + std::to_string((int)s.outputs.size()) + +", index = 1) out " + get_formated_type(out.type) + " o_" + out.name + "1;\n";
								}
								else
									glsl_file << "layout (location = " + std::to_string((int)s.outputs.size()) + +") out " + get_formated_type(out.type) + " o_" + out.name + ";\n";
								s.outputs.push_back(out);
							}
							else if (std::regex_search(line, res, regex_pc))
							{
								if (pll && pll->pc_size > 0)
									glsl_file << "layout (push_constant) uniform pc_t\n";
								else
									glsl_file << "struct type_" + std::to_string(type_id++) + "\n";
							}
							else if (std::regex_search(line, res, regex_ubo))
							{
								auto set = 0;
								auto binding = -1;
								if (pll)
								{
									auto name = res[1].str();
									for (auto j = 0; j < pll->dsls.size(); j++)
									{
										auto dsl = pll->dsls[j];
										for (auto k = 0; k < dsl->bindings.size(); k++)
										{
											if (dsl->bindings[k].name == name)
											{
												set = j;
												binding = k;
												glsl_file << "layout (set = " + std::to_string(set) + ", binding = " + std::to_string(binding) + ") uniform type_" + std::to_string(type_id) + "\n";
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
							else if (std::regex_search(line, res, regex_tex))
							{
								auto set = 0;
								auto binding = -1;
								if (pll)
								{
									auto name = res[1].str();
									for (auto j = 0; j < pll->dsls.size(); j++)
									{
										auto dsl = pll->dsls[j];
										for (auto k = 0; k < dsl->bindings.size(); k++)
										{
											if (dsl->bindings[k].name == name)
											{
												set = j;
												binding = k;
												glsl_file << "layout (set = " + std::to_string(set) + ", binding = " + std::to_string(binding) + ") uniform sampler2D " + name + (res[2].matched ? res[2].str() : "") + ";\n";
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
					printf(" - done\n");

					if (std::filesystem::exists(spv_path))
						std::filesystem::remove(spv_path);

					auto vk_sdk_path = getenv("VK_SDK_PATH");
					if (vk_sdk_path)
					{
						std::filesystem::path glslc_path = vk_sdk_path;
						glslc_path /= L"Bin/glslc.exe";

						std::wstring command_line(L" -fshader-stage=" + shader_stage_name(s.type) + L" out.glsl -o" + spv_path.wstring());

						auto output = exec_and_get_output(glslc_path.c_str(), (wchar_t*)command_line.c_str());
						std::filesystem::remove(L"out.glsl");
						if (!std::filesystem::exists(spv_path))
						{
							printf("compile error:\n%s\n", output.v);
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

							auto output = exec_and_get_output(glslc_path.c_str(), (wchar_t*)command_line.c_str());
							std::filesystem::remove(L"out.glsl");
							if (!std::filesystem::exists(spv_path))
							{
								printf(" - failed\n error:\n%s", output.v);
								assert(0);
								ok = false;
							}
							else
								printf(" - ok\n");
						}

						wprintf(L"end compiling shader:%s\n", shader_path.c_str());
					}
					else
					{
						printf("cannot find vk sdk\n");
						assert(0);
					}

					{
						nlohmann::json json;
						if (s.type == ShaderStageFrag)
						{
							auto info = find_udt(FLAME_CHASH("D#flame::graphics::BlendOptions"));
							auto& bos = json["blend_options"];
							for (auto i = 0; i < s.blend_options.size(); i++)
								info->serialize(&s.blend_options[i], 2, bos[i]);
						}
						std::ofstream res(res_path);
						res << json.dump();
						res.close();
					}
				}
				else
				{
					auto res = get_file_content(res_path);
					if (!res.empty())
					{
						auto json = nlohmann::json::parse(res);
						if (s.type == ShaderStageFrag)
						{
							auto info = find_udt(FLAME_CHASH("D#flame::graphics::BlendOptions"));
							auto& bos = json["blend_options"];
							for (auto i = 0; i < bos.size(); i++)
							{
								BlendOptions dst;
								info->unserialize(bos[i], &dst);
								s.blend_options.push_back(dst);
							}
						}
					}
				}

				if (!ok)
					return false;

				auto spv_file = get_file_content(spv_path);
				if (spv_file.empty())
				{
					assert(0);
					return false;
				}

				if (execution_context)
					execution_context->report_used_resource(spv_path.c_str());

#if defined(FLAME_VULKAN)
				VkShaderModuleCreateInfo shader_info;
				shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				shader_info.flags = 0;
				shader_info.pNext = nullptr;
				shader_info.codeSize = spv_file.size();
				shader_info.pCode = (uint*)spv_file.data();
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

			return true;
		}

		PipelinePrivate::PipelinePrivate(DevicePrivate* d, const std::vector<StageInfo>& stage_infos, PipelinelayoutPrivate* pll, Renderpass* rp, uint subpass_idx, VertexInputInfo* vi, const Vec2u& vp, RasterInfo* raster, SampleCount sc, DepthInfo* depth, uint dynamic_state_count, const uint* dynamic_states) :
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
				dst.stage = to_backend(src.type);
				dst.module = src.vk_shader_module;
			}

			if (vi)
			{
				auto attribute_location = 0;
				vk_vi_bindings.resize(vi->buffer_count);
				for (auto i = 0; i < vk_vi_bindings.size(); i++)
				{
					const auto& src = *vi->buffers[i];
					auto& dst = vk_vi_bindings[i];
					dst.binding = i;
					for (auto j = 0; j < src.attribute_count; j++)
					{
						const auto& _src = *src.attributes[j];
						VkVertexInputAttributeDescription _dst;
						_dst.location = attribute_location++;
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

			vk_blend_attachment_states.resize(rp->subpass_info(subpass_idx).color_attachment_count);
			for (auto& a : vk_blend_attachment_states)
			{
				memset(&a, 0, sizeof(a));
				a.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			}
			if (stage_infos.back().type == ShaderStageFrag)
			{
				auto& bos = stage_infos.back().blend_options;
				for (auto i = 0; i < bos.size(); i++)
				{
					const auto& src = bos[i];
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

			for (auto i = 0; i < dynamic_state_count; i++)
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
			pipeline_info.stage.stage = to_backend(ShaderStageComp);
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

		Pipeline* Pipeline::create(Device* d, const wchar_t* shader_dir, uint shader_count, const wchar_t* const* shader_filenames, Pipelinelayout* pll, Renderpass* rp, uint subpass_idx,
			VertexInputInfo* vi, const Vec2u& vp, RasterInfo* raster, SampleCount sc, DepthInfo* depth, uint dynamic_state_count, const uint* dynamic_states)
		{
			std::vector<StageInfo> stage_infos;
			auto has_vert_stage = false;
			auto tess_stage_count = 0;
			for (auto i = 0; i < shader_count; i++)
			{
				StageInfo info(shader_filenames[i]);

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
					tess_stage_count++;

				stage_infos.push_back(std::move(info));
			}
			if (!has_vert_stage || (tess_stage_count != 0 && tess_stage_count != 2))
				return nullptr;
			std::sort(stage_infos.begin(), stage_infos.end(), [](const auto& a, const auto& b) {
				return (int)a.type < (int)b.type;
			});

			if (!compile_shaders((DevicePrivate*)d, shader_dir, stage_infos, (PipelinelayoutPrivate*)pll, vi))
				return nullptr;

			return new PipelinePrivate((DevicePrivate*)d, stage_infos, (PipelinelayoutPrivate*)pll, rp, subpass_idx, vi, vp, raster, sc, depth, dynamic_state_count, dynamic_states);
		}

		Pipeline* Pipeline::create(Device* d, const wchar_t* shader_dir, const wchar_t* compute_shader_filename, Pipelinelayout* pll)
		{
			StageInfo compute_stage_info(compute_shader_filename);
			if (compute_stage_info.type != ShaderStageComp)
				return nullptr;

			std::vector<StageInfo> stage_infos;
			stage_infos.push_back(std::move(compute_stage_info));

			if (!compile_shaders((DevicePrivate*)d, shader_dir, stage_infos, (PipelinelayoutPrivate*)pll, nullptr))
				return nullptr;

			return new PipelinePrivate((DevicePrivate*)d, compute_stage_info, (PipelinelayoutPrivate*)pll);
		}

		void Pipeline::destroy(Pipeline* p)
		{
			delete (PipelinePrivate*)p;
		}

		struct R(R_Pipeline)
		{
			BP::Node* n;

			BASE0;
			RV(Array<StringW>*, shader_filenames, i);
			RV(Pipelinelayout*, pll, i);
			RV(Renderpass*, renderpass, i);
			RV(uint, subpass_idx, i);
			RV(VertexInputInfo*, vi, i);
			RV(Vec2u, vp, i);
			RV(RasterInfo*, raster, i);
			RV(SampleCount, sc, i);
			RV(DepthInfo*, depth, i);
			RV(Array<uint>*, dynamic_states, i);

			BASE1;
			RV(Pipeline*, out, o);

			FLAME_GRAPHICS_EXPORTS void RF(update)(uint frame)
			{
				auto out_frame = out_s()->frame();
				if (renderpass_s()->frame() > out_frame || subpass_idx_s()->frame() > out_frame || shader_filenames_s()->frame() > out_frame || pll_s()->frame() > out_frame ||
					vi_s()->frame() > out_frame || vp_s()->frame() > out_frame || raster_s()->frame() > out_frame || sc_s()->frame() > out_frame || depth_s()->frame() > out_frame || dynamic_states_s()->frame() > out_frame)
				{
					if (out)
						Pipeline::destroy(out);
					auto d = Device::default_one();
					if (d && renderpass && renderpass->subpass_count() > subpass_idx && shader_filenames->s > 0 && pll)
					{
						std::vector<const wchar_t*> _names(shader_filenames->s);
						for (auto i = 0; i < _names.size(); i++)
							_names[i] = shader_filenames->at(i).v;
						execution_context = n->scene();
						out = Pipeline::create(d, std::filesystem::path(n->scene()->filename()).parent_path().c_str(), _names.size(), _names.data(), pll, renderpass, subpass_idx,
							vi, vp, raster, sc, depth, dynamic_states ? dynamic_states->s : 0, dynamic_states ? dynamic_states->v : nullptr);
						execution_context = nullptr;
					}
					else
						out = nullptr;
					if (!out)
						printf("cannot create pipeline\n");
					out_s()->set_frame(frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS RF(~R_Pipeline)()
			{
				if (out)
					Pipeline::destroy(out);
			}
		};
	}
}
