#include "camera_private.h"

namespace flame
{
	cCamera* cCamera::create()
	{
		return f_new<cCameraPrivate>();
	}
}
