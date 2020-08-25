#include "camera_private.h"

namespace flame
{
	void cCameraPrivate::update_matrix()
	{
		if (project_dirty)
		{
			project_dirty = false;


		}

		if (view_dirty)
		{
			view_dirty = false;


		}
	}
}
