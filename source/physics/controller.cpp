#include "device_private.h"
#include "scene_private.h"
#include "material_private.h"
#include "controller_private.h"

namespace flame
{
	namespace physics
	{
		ControllerPrivate::ControllerPrivate(ScenePrivate* scene, MaterialPrivate* material, float radius, float height)
		{
			PxCapsuleControllerDesc desc;
			desc.material = material->px_material.get();
			desc.contactOffset = 0.01f;
			desc.radius = radius;
			desc.height = height;
			px_controller.reset(scene->px_controller_manager->createController(desc));
			auto px_actor = px_controller->getActor();
			px_actor->setName("Controller");
			px_actor->userData = this;
		}

		vec3 ControllerPrivate::get_position() const
		{
			auto p = px_controller->getFootPosition();
			return vec3(p.x, p.y, p.z);
		}

		void ControllerPrivate::set_position(const vec3& pos)
		{
			px_controller->setFootPosition(physx::PxExtendedVec3(pos.x, pos.y, pos.z));
		}

		bool ControllerPrivate::move(const vec3& disp, float delta_time)
		{
			return px_controller->move(cvt(disp), 0.f, delta_time, {}).isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN);
		}

		Controller* Controller::create(Scene* scene, Material* material, float radius, float height)
		{
			return new ControllerPrivate((ScenePrivate*)scene, (MaterialPrivate*)material, radius, height);
		}
	}
}
