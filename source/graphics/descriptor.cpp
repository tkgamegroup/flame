// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "descriptor_private.h"
#include "device_private.h"
#include "pipeline_private.h"
#include "buffer_private.h"
#include "image_private.h"

namespace flame
{
	namespace graphics
	{
		inline DescriptorpoolPrivate::DescriptorpoolPrivate(Device *_d)
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
			vk_chk_res(vkCreateDescriptorPool(((DevicePrivate*)d)->v, &descriptorPoolInfo, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		inline DescriptorpoolPrivate::~DescriptorpoolPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyDescriptorPool(((DevicePrivate*)d)->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Descriptorpool *Descriptorpool::create(Device *d)
		{
			return new DescriptorpoolPrivate(d);
		}

		void Descriptorpool::destroy(Descriptorpool *p)
		{
			delete (DescriptorpoolPrivate*)p;
		}

		inline bool operator==(const Descriptorsetlayout::Binding &lhs, const Descriptorsetlayout::Binding &rhs)
		{
			return lhs.binding == rhs.binding && lhs.type == rhs.type && lhs.count == rhs.count;
		}

		inline DescriptorsetlayoutPrivate::DescriptorsetlayoutPrivate(Device *_d, const std::vector<Binding> &_bindings)
		{
			bindings = _bindings;

			d = (DevicePrivate*)_d;

#if defined(FLAME_VULKAN)
			std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
			vk_bindings.resize(bindings.size());
			for (auto i = 0; i < bindings.size(); i++)
			{
				vk_bindings[i].binding = bindings[i].binding;
				vk_bindings[i].descriptorType = Z(bindings[i].type);
				vk_bindings[i].descriptorCount = bindings[i].count;
				vk_bindings[i].stageFlags = Z(ShaderAll);
				vk_bindings[i].pImmutableSamplers = nullptr;
			}

			VkDescriptorSetLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.bindingCount = vk_bindings.size();
			info.pBindings = vk_bindings.data();

			vk_chk_res(vkCreateDescriptorSetLayout(((DevicePrivate*)d)->v,
				&info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		inline DescriptorsetlayoutPrivate::~DescriptorsetlayoutPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyDescriptorSetLayout(((DevicePrivate*)d)->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		static std::vector<DescriptorsetlayoutPrivate*> created_layouts;

		Descriptorsetlayout *Descriptorsetlayout::get(Device *d, const std::vector<Binding> &bindings)
		{
			for (auto &l : created_layouts)
			{
				if (!(l->bindings == bindings))
					continue;
				l->ref_count++;
				return l;
			}

			auto l = new DescriptorsetlayoutPrivate(d, bindings);
			l->ref_count = 1;
			created_layouts.push_back(l);
			return l;
		}

		void Descriptorsetlayout::release(Descriptorsetlayout *l)
		{
			for (auto it = created_layouts.begin(); it != created_layouts.end(); it++)
			{
				if (*it == l)
				{
					if ((*it)->ref_count == 1)
					{
						created_layouts.erase(it);
						break;
					}
					else
					{
						(*it)->ref_count--;
						return;
					}
				}
			}
			
			delete (DescriptorsetlayoutPrivate*)l;
		}

		inline DescriptorsetPrivate::DescriptorsetPrivate(Descriptorpool *_p, Descriptorsetlayout *l)
		{
			p = (DescriptorpoolPrivate*)_p;

#if defined(FLAME_VULKAN)
			VkDescriptorSetAllocateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.pNext = nullptr;
			info.descriptorPool = p->v;
			info.descriptorSetCount = 1;
			info.pSetLayouts = &((DescriptorsetlayoutPrivate*)l)->v;

			vk_chk_res(vkAllocateDescriptorSets(p->d->v, &info, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		inline DescriptorsetPrivate::~DescriptorsetPrivate()
		{
#if defined(FLAME_VULKAN)
			vk_chk_res(vkFreeDescriptorSets(p->d->v, p->v, 1, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		inline void DescriptorsetPrivate::set_uniformbuffer(int binding, int index, Buffer *b, int offset, int range)
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

		inline void DescriptorsetPrivate::set_storagebuffer(int binding, int index, Buffer *b, int offset, int range)
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

		inline void DescriptorsetPrivate::set_imageview(int binding, int index, Imageview *iv, Sampler *sampler)
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

		inline void DescriptorsetPrivate::set_storageimage(int binding, int index, Imageview *iv)
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

		void Descriptorset::set_uniformbuffer(int binding, int index, Buffer *b, int offset, int range)
		{
			((DescriptorsetPrivate*)this)->set_uniformbuffer(binding, index, b, offset, range);
		}

		void Descriptorset::set_storagebuffer(int binding, int index, Buffer *b, int offset, int range)
		{
			((DescriptorsetPrivate*)this)->set_storagebuffer(binding, index, b, offset, range);
		}

		void Descriptorset::set_imageview(int binding, int index, Imageview *v, Sampler *sampler)
		{
			((DescriptorsetPrivate*)this)->set_imageview(binding, index, v, sampler);
		}

		void Descriptorset::set_storageimage(int binding, int index, Imageview *v)
		{
			((DescriptorsetPrivate*)this)->set_storageimage(binding, index, v);
		}

		Descriptorset *Descriptorset::create(Descriptorpool *p, Descriptorsetlayout *l)
		{
			return new DescriptorsetPrivate(p, l);
		}

		void Descriptorset::destroy(Descriptorset *s)
		{
			delete (DescriptorsetPrivate*)s;
		}
	}
}
