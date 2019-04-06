#include <flame/engine/entity/node.h>
#include <flame/engine/entity/camera.h>
#include <flame/engine/entity/camera_third_person.h>

namespace flame
{
	void set_camera_third_person_position(CameraComponent *camera, const glm::vec3 target, float length)
	{
		auto n = camera->get_parent();
		n->set_coord(target + n->get_axis()[2] * length);
	}
}
