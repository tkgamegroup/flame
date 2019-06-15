#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;

		struct Shader
		{
			ShaderType$ type;

			FLAME_GRAPHICS_EXPORTS static Shader* create(Device *d, const std::wstring &filename, const std::string &prefix);
			FLAME_GRAPHICS_EXPORTS static void destroy(Shader *s);
		};
	}
}
