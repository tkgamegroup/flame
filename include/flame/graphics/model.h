#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		enum StandardModel
		{
			StandardModelCube,

			StandardModelCount
		};

		struct Model
		{
			FLAME_GRAPHICS_EXPORTS static Model* get_standard(StandardModel m);
			FLAME_GRAPHICS_EXPORTS static Model* create(const wchar_t* filename);
		};
	}
}
