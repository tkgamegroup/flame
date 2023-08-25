#include "image_private.h"
#include "renderpass_private.h"
#include "command_private.h"
#include "window_private.h"
// let changes of the following files trigger build (that headers will copy to include dir)
#include "application.h"
#include "gui.h"
#include "blueprint_library/library.h"

PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasks = nullptr;

namespace flame
{
	namespace graphics
	{
		std::map<void*, TrackedObject> tracked_objects;

		void register_object(void* backend_obj, const std::string& type, void* obj)
		{
			static void* ev_cleanup = nullptr;
			if (!ev_cleanup)
			{
				ev_cleanup = add_event([]() {
					for (auto it = tracked_objects.begin(); it != tracked_objects.end();)
					{
						if (it->second.die_frames == 1)
							it = tracked_objects.erase(it);
						else
							it++;
					}
					return true;
				});
			}
			tracked_objects[backend_obj] = TrackedObject{ type, obj };
		}

		void unregister_object(void* backend_obj)
		{
			if (app_exiting)
				return;
			auto it = tracked_objects.find(backend_obj);
			if (it != tracked_objects.end())
				it->second.die_frames = 3;
		}

		struct _Initializer
		{
			_Initializer()
			{
				printf("graphics init\n");

				add_event([]() {
					init_library();
					return false;
				});
			}
		};
		static _Initializer _initializer;
	}
}
