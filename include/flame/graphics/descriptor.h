#pragma once

#include <flame/graphics/graphics.h>

#include <vector>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Buffer;
		struct Imageview;
		struct Sampler;

		struct Descriptorpool
		{
			FLAME_GRAPHICS_EXPORTS static Descriptorpool *create(Device *d);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorpool *p);
		};

		struct Descriptorsetlayout
		{
			struct Binding
			{
				uint binding;
				ShaderResourceType type;
				uint count;
			};

			FLAME_GRAPHICS_EXPORTS static Descriptorsetlayout *create(Device *d, const std::vector<Binding> &bindings);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorsetlayout *l);
		};

		struct Descriptorset
		{
			FLAME_GRAPHICS_EXPORTS void set_uniformbuffer(uint binding, uint index, Buffer *b, uint offset = 0, uint range = 0);
			FLAME_GRAPHICS_EXPORTS void set_storagebuffer(uint binding, uint index, Buffer *b, uint offset = 0, uint range = 0);
			FLAME_GRAPHICS_EXPORTS void set_imageview(uint binding, uint index, Imageview *v, Sampler *sampler);
			FLAME_GRAPHICS_EXPORTS void set_storageimage(uint binding, uint index, Imageview *v);

			FLAME_GRAPHICS_EXPORTS static Descriptorset *create(Descriptorpool *p, Descriptorsetlayout *l);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorset *s);
		};
	}
}

