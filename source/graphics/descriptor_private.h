#pragma once

#include <flame/graphics/descriptor.h>
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;

		struct DescriptorpoolPrivate : Descriptorpool
		{
			DevicePrivate *d;
#if defined(FLAME_VULKAN)
			VkDescriptorPool v;
#elif defined(FLAME_D3D12)

#endif

			DescriptorpoolPrivate(Device *d);
			~DescriptorpoolPrivate();
		};

		struct DescriptorsetlayoutPrivate : Descriptorsetlayout
		{
			std::vector<Binding> bindings;

			DevicePrivate *d;
#if defined(FLAME_VULKAN)
			VkDescriptorSetLayout v;
#elif defined(FLAME_D3D12)

#endif
			DescriptorsetlayoutPrivate(Device *d, const std::vector<Binding> &_bindings);
			~DescriptorsetlayoutPrivate();
		};

		struct DescriptorsetPrivate : Descriptorset
		{
			DescriptorpoolPrivate *p;
#if defined(FLAME_VULKAN)
			VkDescriptorSet v;
#elif defined(FLAME_D3D12)

#endif

			DescriptorsetPrivate(Descriptorpool *p, Descriptorsetlayout *l);
			~DescriptorsetPrivate();

			void set_uniformbuffer(uint binding, uint index, Buffer *b, uint offset = 0, uint range = 0);
			void set_storagebuffer(uint binding, uint index, Buffer *b, uint offset = 0, uint range = 0);
			void set_imageview(uint binding, uint index, Imageview *iv, Sampler *sampler);
			void set_storageimage(uint binding, uint index, Imageview *iv);
		};
	}
}
