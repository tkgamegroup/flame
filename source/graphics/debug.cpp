#include "debug.h"
#include "image_private.h"

namespace flame
{
	namespace graphics
	{
		struct DebugGetAllImages : Debug::GetAllImages
		{
			std::vector<ImagePtr> operator()() override
			{
				return all_images;
			}
		}Debug_get_all_images;
		Debug::GetAllImages& Debug::get_all_images = Debug_get_all_images;
	}
}
