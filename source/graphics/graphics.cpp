#include "image_private.h"
#include "swapchain_private.h"
#include "renderpass_private.h"
#include "command_private.h"
#include "window_private.h"
// let changes of app.h trigger build (that app.h will copy to include dir)
#include "application.h"

namespace flame
{
	namespace graphics
	{
		std::map<void*, std::pair<uint, void*>> backend_objects;

		void register_backend_object(void* backend_obj, uint hash, void* obj)
		{
			backend_objects[backend_obj] = std::make_pair(hash, obj);
		}

		void unregister_backend_object(void* backend_obj)
		{
			auto it = backend_objects.find(backend_obj);
			if (it != backend_objects.end())
				backend_objects.erase(it);
		}
	}
}
