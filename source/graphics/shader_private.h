#pragma once

#include <flame/foundation/foundation.h>
#include <flame/graphics/shader.h>
#include "graphics_private.h"

namespace flame
{
	struct SerializableNode;

	namespace graphics
	{
		struct DevicePrivate;

		struct ShaderVariableInfo
		{
			std::string name;
			uint offset;
			uint size;
			uint count;
			uint array_stride;

			std::vector<std::unique_ptr<ShaderVariableInfo>> members;
		};

		struct ShaderResource
		{
			ShaderResourceType type;
			uint set;
			uint binding;
			ShaderVariableInfo var;
		};

		struct ShaderPrivate : Shader
		{
			std::wstring filename_;
			std::string prefix_;
			std::vector<std::unique_ptr<ShaderResource>> resources;

			DevicePrivate *d;
#if defined(FLAME_VULKAN)
			VkShaderModule v;
#elif defined(FLAME_D3D12)

#endif

			ShaderPrivate(Device *d, const std::wstring &filename, const std::string &prefix);
			~ShaderPrivate();
		};
	}
}
