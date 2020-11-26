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
			if (!material)
				material = scene->device->mat.get();
#ifdef USE_PHYSX
			PxCapsuleControllerDesc desc;
			desc.material = material->px_material;
			desc.contactOffset = 0.01f;
			desc.radius = radius;
			desc.height = height;
			px_controller = scene->px_controller_manager->createController(desc);
#endif
		}

		ControllerPrivate::~ControllerPrivate()
		{
#ifdef USE_PHYSX
			px_controller->release();
#endif
		}

		vec3 ControllerPrivate::get_position() const
		{
#ifdef USE_PHYSX
			auto p = px_controller->getFootPosition();
			return vec3(p.x, p.y, p.z);
#else
			return vec3(0.f);
#endif
		}

		void ControllerPrivate::set_position(const vec3& pos)
		{
#ifdef USE_PHYSX
			px_controller->setFootPosition(physx::PxExtendedVec3(pos.x, pos.y, pos.z));
#endif
		}

		void ControllerPrivate::move(const vec3& disp, float delta_time)
		{
#ifdef USE_PHYSX
			px_controller->move(cvt(disp), 0.f, delta_time, physx::PxControllerFilters());

			// if (o->pxController->move(disp, 0.f, dist, nullptr) & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
			//	o->floatingTime = 0.f;
#endif
		}

		Controller* Controller::create(Scene* scene, Material* material, float radius, float height)
		{
			return new ControllerPrivate((ScenePrivate*)scene, (MaterialPrivate*)material, radius, height);
		}
	}
}
