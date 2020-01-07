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

		DescriptorlayoutPrivate::DescriptorlayoutPrivate(Device* _d, uint binding_count, DescriptorBindingBase* const* _bindings, Descriptorpool* default_set_pool) :
			d((DevicePrivate*)_d)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
			bindings.resize(binding_count);
			for (auto i = 0; i < binding_count; i++)
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

		uint Descriptorlayout::binding_count() const
		{
			return ((DescriptorlayoutPrivate*)this)->bindings.size();
		}

		const DescriptorBindingBase* Descriptorlayout::get_binding(uint binding) const
		{
			return ((DescriptorlayoutPrivate*)this)->bindings[binding];
		}

		Descriptorset* Descriptorlayout::default_set() const
		{
			return ((DescriptorlayoutPrivate*)this)->default_set;
		}

		Descriptorlayout* Descriptorlayout::create(Device* d, uint binding_count, DescriptorBindingBase* const* bindings, Descriptorpool* pool_to_create_default_set)
		{
			return new DescriptorlayoutPrivate(d, binding_count, bindings, pool_to_create_default_set);
		}

		void Descriptorlayout::destroy(Descriptorlayout* l)
		{
			delete (DescriptorlayoutPrivate*)l;
		}

		struct DescriptorBufferBinding$
		{
			AttributeD<bool> storage$i;
			AttributeD<uint> count$i;
			AttributeD<StringA> name$i;
			AttributeP<Buffer> buffer$i;

			AttributeD<DescriptorBufferBinding> out$o;

			FLAME_GRAPHICS_EXPORTS DescriptorBufferBinding$()
			{
				storage$i.v = false;
				count$i.v = 1;
			}

			FLAME_GRAPHICS_EXPORTS void update$(BP* scene)
			{
				auto last_out_frame = out$o.frame;
				if (storage$i.frame > last_out_frame)
				{
					out$o.v.type = storage$i.v ? DescriptorStorageBuffer : DescriptorUniformBuffer;
					out$o.frame = scene->frame;
				}
				if (count$i.frame > last_out_frame)
				{
					out$o.v.count = count$i.v;
					out$o.frame = scene->frame;
				}
				if (name$i.frame > last_out_frame)
				{
					out$o.v.name = name$i.v.v;
					out$o.frame = scene->frame;
				}
				if (buffer$i.frame > last_out_frame)
				{
					out$o.v.buffer = (Buffer*)buffer$i.v;
					out$o.frame = scene->frame;
				}
			}
		};

		struct DescriptorImageBinding$
		{
			AttributeD<uint> binding$i;
			AttributeD<bool> storage$i;
			AttributeD<uint> count$i;
			AttributeD<StringA> name$i;
			AttributeE<TargetType$> target_type$i;
			AttributeP<void> v$i;

			AttributeD<DescriptorImageBinding> out$o;
			AttributeP<Imageview> iv$o;

			FLAME_GRAPHICS_EXPORTS DescriptorImageBinding$()
			{
				storage$i.v = false;
				count$i.v = 1;
			}

			FLAME_GRAPHICS_EXPORTS void update$(BP* scene)
			{
				auto last_out_frame = out$o.frame;
				if (last_out_frame == -1)
				{
					auto d = Device::default_one();
					if (d)
						out$o.v.sampler = d->sp_linear;
					out$o.frame = scene->frame;
				}
				if (target_type$i.frame > iv$o.frame || v$i.frame > iv$o.frame)
				{
					if (iv$o.v)
						Imageview::destroy(iv$o.v);
					if (target_type$i.v == TargetImage)
						iv$o.v = Imageview::create((Image*)v$i.v);
					else
						iv$o.v = nullptr;
					iv$o.frame = scene->frame;
				}
				if (storage$i.frame > last_out_frame)
				{
					out$o.v.type = storage$i.v ? DescriptorStorageImage : DescriptorSampledImage;
					out$o.frame = scene->frame;
				}
				if (count$i.frame > last_out_frame)
				{
					out$o.v.count = count$i.v;
					out$o.frame = scene->frame;
				}
				if (name$i.frame > last_out_frame)
				{
					out$o.v.name = name$i.v.v;
					out$o.frame = scene->frame;
				}
				if (iv$o.frame > last_out_frame)
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
					out$o.frame = scene->frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~DescriptorImageBinding$()
			{
				if (iv$o.v)
					Imageview::destroy(iv$o.v);
			}
		};

		struct Descriptorlayout$
		{
			AttributeP<Array<void*>> bindings$i;
			AttributeD<bool> create_default_set$i;

			AttributeP<Descriptorlayout> out$o;
			AttributeP<Descriptorset> default_set$o;

			FLAME_GRAPHICS_EXPORTS Descriptorlayout$()
			{
				create_default_set$i.v = true;
			}

			FLAME_GRAPHICS_EXPORTS void update$(BP* scene)
			{
				if (bindings$i.frame > out$o.frame || create_default_set$i.frame > default_set$o.frame)
				{
					if (out$o.v)
						Descriptorlayout::destroy(out$o.v);
					auto d = Device::default_one();
					if (d)
					{
						std::vector<DescriptorBindingBase*> bindings(bindings$i.v ? bindings$i.v->s : 0);
						for (auto i = 0; i < bindings.size(); i++)
							bindings[i] = (DescriptorBindingBase*)bindings$i.v->v[i];
						out$o.v = Descriptorlayout::create(d, bindings.size(), bindings.data(), create_default_set$i.v ? d->dp : nullptr);
						default_set$o.v = ((Descriptorlayout*)out$o.v)->default_set();
					}
					else
					{
						printf("cannot create descriptorlayout\n");

						out$o.v = nullptr;
						default_set$o.v = nullptr;
					}
					out$o.frame = scene->frame;
					default_set$o.frame = scene->frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Descriptorlayout$()
			{
				if (out$o.v)
					Descriptorlayout::destroy(out$o.v);
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

			FLAME_GRAPHICS_EXPORTS void update$(BP* scene)
			{
				if (dsl$i.frame > out$o.frame)
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
					out$o.frame = scene->frame;
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
			AttributeD<uint> binding$i;
			AttributeD<uint> index$i;
			AttributeD<uint> count$i;
			AttributeP<void> buffer$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS BufferDescriptorWrite$()
			{
				count$i.v = 1;
			}

			FLAME_GRAPHICS_EXPORTS void update$(BP* scene)
			{
				if (binding$i.frame > out$o.frame || index$i.frame > out$o.frame || count$i.frame > out$o.frame || buffer$i.frame > out$o.frame)
				{
					out$o.v = this;
					out$o.frame = scene->frame;
				}
			}
		};

		struct ImageDescriptorWrite$
		{
			AttributeD<uint> binding$i;
			AttributeD<uint> index$i;
			AttributeD<uint> count$i;
			AttributeE<TargetType$> target_type$i;
			AttributeP<void> v$i;

			AttributeP<void> out$o;
			AttributeP<void> iv$o;

			FLAME_GRAPHICS_EXPORTS ImageDescriptorWrite$()
			{
				count$i.v = 1;
			}

			FLAME_GRAPHICS_EXPORTS void update$(BP* scene)
			{
				if (target_type$i.frame > iv$o.frame || v$i.frame > iv$o.frame)
				{
					if (iv$o.v)
						Imageview::destroy((Imageview*)iv$o.v);
					if (target_type$i.v == TargetImage)
						iv$o.v = Imageview::create((Image*)v$i.v);
					else
						iv$o.v = nullptr;
					iv$o.frame = scene->frame;
				}
				if (binding$i.frame > out$o.frame || index$i.frame > out$o.frame || count$i.frame > out$o.frame || iv$o.frame > out$o.frame)
				{
					out$o.v = this;
					out$o.frame = scene->frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~ImageDescriptorWrite$()
			{
				if (iv$o.v)
					Imageview::destroy((Imageview*)iv$o.v);
			}
		};

		struct DescriptorWriter$
		{
			AttributeP<void> set$i;
			AttributeP<Array<void*>> buffer_writes$i;
			AttributeP<Array<void*>> image_writes$i;

			AttributeP<void> set$o;

			FLAME_GRAPHICS_EXPORTS void update$(BP* scene)
			{
				if (set$i.frame > set$o.frame || buffer_writes$i.frame > set$o.frame || image_writes$i.frame > set$o.frame)
				{
					std::vector<void*> buffer_writes(buffer_writes$i.v ? buffer_writes$i.v->s : 0);
					for (auto i = 0; i < buffer_writes.size(); i++)
						buffer_writes[i] = buffer_writes$i.v->v[i];
					for (auto _w : buffer_writes)
					{
						auto w = (BufferDescriptorWrite$*)_w;
						for (auto i = 0; i < w->count$i.v; i++)
							((graphics::Descriptorset*)set$i.v)->set_buffer(w->binding$i.v, w->index$i.v + i, (graphics::Buffer*)w->buffer$i.v);
					}
					std::vector<void*> image_writes(image_writes$i.v ? image_writes$i.v->s : 0);
					for (auto i = 0; i < image_writes.size(); i++)
						image_writes[i] = image_writes$i.v->v[i];
					if (!image_writes.empty())
					{
						auto sampler = Device::default_one()->sp_linear;
						for (auto _w : image_writes)
						{
							auto w = (ImageDescriptorWrite$*)_w;
							for (auto i = 0; i < w->count$i.v; i++)
								((graphics::Descriptorset*)set$i.v)->set_image(w->binding$i.v, w->index$i.v + i, (graphics::Imageview*)w->iv$o.v, sampler);
						}
					}

					set$o.v = set$i.v;
					set$o.frame = scene->frame;
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

			dsls.resize(descriptorlayout_count);
			for (auto i = 0; i < dsls.size(); i++)
				dsls[i] = (DescriptorlayoutPrivate*)descriptorlayouts[i];
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

		struct Pipelinelayout$
		{
			AttributeP<Array<Descriptorlayout*>> descriptorlayouts$i;
			AttributeD<uint> push_constant_size$i;

			AttributeP<Pipelinelayout> out$o;

			FLAME_GRAPHICS_EXPORTS void update$(BP* scene)
			{
				if (descriptorlayouts$i.frame > out$o.frame || push_constant_size$i.frame > out$o.frame)
				{
					if (out$o.v)
						Pipelinelayout::destroy(out$o.v);
					auto d = Device::default_one();
					if (d)
						out$o.v = Pipelinelayout::create(d, descriptorlayouts$i.v ? descriptorlayouts$i.v->s : 0, descriptorlayouts$i.v ? descriptorlayouts$i.v->v : nullptr, push_constant_size$i.v);
					else
					{
						printf("cannot create pipelinelayout\n");

						out$o.v = nullptr;
					}
					out$o.frame = scene->frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Pipelinelayout$()
			{
				if (out$o.v)
					Pipelinelayout::destroy(out$o.v);
			}
		};

		struct VertexInputAttribute$
		{
			AttributeD<StringA> name$i;
			AttributeE<Format$> format$i;

			AttributeD<VertexInputAttribute> out$o;

			FLAME_GRAPHICS_EXPORTS VertexInputAttribute$()
			{
				format$i.v = Format_R8G8B8A8_UNORM;
			}

			FLAME_GRAPHICS_EXPORTS void update$(BP* scene)
			{
				auto last_out_frame = out$o.frame;
				if (name$i.frame > last_out_frame)
				{
					out$o.v.name = name$i.v.v;
					out$o.frame = scene->frame;
				}
				if (format$i.frame > last_out_frame)
				{
					out$o.v.format = format$i.v;
					out$o.frame = scene->frame;
				}
			}
		};

		struct VertexInputBuffer$
		{
			AttributeP<Array<VertexInputAttribute*>> attributes$i;
			AttributeE<VertexInputRate$> rate$i;

			AttributeD<VertexInputBuffer> out$o;

			FLAME_GRAPHICS_EXPORTS VertexInputBuffer$()
			{
				rate$i.v = VertexInputRateVertex;
			}

			FLAME_GRAPHICS_EXPORTS void update$(BP* scene)
			{
				auto last_out_frame = out$o.frame;
				if (attributes$i.frame > last_out_frame)
				{
					out$o.v.attribute_count = attributes$i.v ? attributes$i.v->s : 0;
					out$o.v.attributes = attributes$i.v ? attributes$i.v->v : nullptr;
					out$o.frame = scene->frame;
				}
				if (rate$i.frame > last_out_frame)
				{
					out$o.v.rate = rate$i.v;
					out$o.frame = scene->frame;
				}
			}
		};

		struct VertexInputInfo$
		{
			AttributeP<Array<VertexInputBuffer*>> buffers$i;
			AttributeE<PrimitiveTopology$> primitive_topology$i;
			AttributeD<uint> patch_control_points$i;

			AttributeD<VertexInputInfo> out$o;

			FLAME_GRAPHICS_EXPORTS VertexInputInfo$()
			{
				primitive_topology$i.v = PrimitiveTopologyTriangleList;
			}

			FLAME_GRAPHICS_EXPORTS void update$(BP* scene)
			{
				auto last_out_frame = out$o.frame;
				if (buffers$i.frame > last_out_frame)
				{
					out$o.v.buffer_count = buffers$i.v ? buffers$i.v->s : 0;
					out$o.v.buffers = buffers$i.v ? buffers$i.v->v : nullptr;
					out$o.frame = scene->frame;
				}
				if (primitive_topology$i.frame > last_out_frame)
				{
					out$o.v.primitive_topology = primitive_topology$i.v;
					out$o.frame = scene->frame;
				}
				if (patch_control_points$i.frame > last_out_frame)
				{
					out$o.v.patch_control_points = patch_control_points$i.v;
					out$o.frame = scene->frame;
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
									for (auto i = 0; i < vi->buffer_count; i++)
									{
										const auto& b = *vi->buffers[i];
										for (auto j = 0; j < b.attribute_count; j++)
										{
											const auto& a = *b.attributes[j];
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
									auto sp = ssplit(match[4].str());
									for (auto& p : sp)
									{
										auto sp = ssplit(p, ':');
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

				auto output = exec_and_get_output((vk_sdk_path + L"/Bin/glslc.exe").c_str(), (wchar_t*)command_line.c_str());
				std::filesystem::remove(L"out.glsl");
				if (!std::filesystem::exists(L"out.spv"))
				{
					printf("shader \"%s\" compile error:\n%s\n", w2s(s.filename).c_str(), output.v);
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

					auto output = exec_and_get_output((vk_sdk_path + L"/Bin/glslc.exe").c_str(), (wchar_t*)command_line.c_str());
					std::filesystem::remove(L"out.glsl");
					if (!std::filesystem::exists(L"out.spv"))
					{
						printf(" - failed\n error:\n%s", output.v);
						assert(0);
					}

					printf(" - ok\n");
				}

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

		PipelinePrivate::PipelinePrivate(DevicePrivate* d, const std::vector<StageInfo>& stage_infos, PipelinelayoutPrivate* pll, Renderpass* rp, uint subpass_idx, VertexInputInfo* vi, const Vec2u& vp, RasterInfo* raster, SampleCount$ sc, DepthInfo* depth, uint dynamic_state_count, const uint* dynamic_states) :
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

			vk_blend_attachment_states.resize(rp->subpass_info(subpass_idx).color_attachment_count);
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

			for (auto i = 0; i < dynamic_state_count; i++)
				vk_dynamic_states.push_back(to_enum((DynamicState)dynamic_states[i]));
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

		Pipeline* Pipeline::create(Device* d, uint shader_count, const wchar_t* const* shader_filenames, Pipelinelayout* pll, Renderpass* rp, uint subpass_idx,
			VertexInputInfo* vi, const Vec2u& vp, RasterInfo* raster, SampleCount$ sc, DepthInfo* depth, uint dynamic_state_count, const uint* dynamic_states)
		{
			std::vector<StageInfo> stage_infos;
			auto has_vert_stage = false;
			auto tess_stage_count = 0;
			for (auto i = 0; i < shader_count; i++)
			{
				StageInfo info(shader_filenames[i]);

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
					tess_stage_count++;

				stage_infos.push_back(std::move(info));
			}
			if (!has_vert_stage || (tess_stage_count != 0 && tess_stage_count != 2))
				return nullptr;
			std::sort(stage_infos.begin(), stage_infos.end(), [](const auto& a, const auto& b) {
				return (int)a.type < (int)b.type;
			});

			compile_shaders((DevicePrivate*)d, stage_infos, (PipelinelayoutPrivate*)pll, vi);

			return new PipelinePrivate((DevicePrivate*)d, stage_infos, (PipelinelayoutPrivate*)pll, rp, subpass_idx, vi, vp, raster, sc, depth, dynamic_state_count, dynamic_states);
		}

		Pipeline* Pipeline::create(Device* d, const wchar_t* compute_shader_filename, Pipelinelayout* pll)
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
			AttributeP<Array<StringW>> shader_filenames$i;
			AttributeP<Pipelinelayout> pll$i;
			AttributeP<Renderpass> renderpass$i;
			AttributeD<uint> subpass_idx$i;
			AttributeP<VertexInputInfo> vi$i;
			AttributeD<Vec2u> vp$i;
			AttributeP<RasterInfo> raster$i;
			AttributeE<SampleCount$> sc$i;
			AttributeP<DepthInfo> depth$i;
			AttributeP<Array<uint>> dynamic_states$i;

			AttributeP<Pipeline> out$o;

			FLAME_GRAPHICS_EXPORTS void update$(BP* scene)
			{
				if (renderpass$i.frame > out$o.frame || subpass_idx$i.frame > out$o.frame || shader_filenames$i.frame > out$o.frame || pll$i.frame > out$o.frame ||
					vi$i.frame > out$o.frame || vp$i.frame > out$o.frame || raster$i.frame > out$o.frame || sc$i.frame > out$o.frame || depth$i.frame > out$o.frame || dynamic_states$i.frame > out$o.frame)
				{
					if (out$o.v)
						Pipeline::destroy(out$o.v);
					auto d = Device::default_one();
					std::vector<std::wstring> shader_filenames(shader_filenames$i.v ? shader_filenames$i.v->s : 0);
					auto bp_path = std::filesystem::path(scene->filename()).parent_path().wstring() + L"/";
					for (auto i = 0; i < shader_filenames.size(); i++)
						shader_filenames[i] = bp_path + shader_filenames$i.v->v[i].str();
					if (d && renderpass$i.v && ((Renderpass*)renderpass$i.v)->subpass_count() > subpass_idx$i.v && !shader_filenames.empty() && pll$i.v)
					{
						std::vector<const wchar_t*> _names(shader_filenames.size());
						for (auto i = 0; i < _names.size(); i++)
							_names[i] = shader_filenames[i].c_str();
						out$o.v = Pipeline::create(d, _names.size(), _names.data(), (Pipelinelayout*)pll$i.v, (Renderpass*)renderpass$i.v, subpass_idx$i.v,
							(VertexInputInfo*)vi$i.v, vp$i.v, (RasterInfo*)raster$i.v, sc$i.v, (DepthInfo*)depth$i.v, dynamic_states$i.v ? dynamic_states$i.v->s : 0, dynamic_states$i.v ? dynamic_states$i.v->v : nullptr);
					}
					else
						out$o.v = nullptr;
					if (!out$o.v)
						printf("cannot create pipeline\n");

					out$o.frame = scene->frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Pipeline$()
			{
				if (out$o.v)
					Pipeline::destroy(out$o.v);
			}
		};
	}
}
