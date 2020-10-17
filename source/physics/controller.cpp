#include "scene_private.h"
#include "material_private.h"
#include "controller_private.h"

namespace flame
{
	namespace physics
	{
		ControllerPrivate::ControllerPrivate(ScenePrivate* scene, MaterialPrivate* material, float radius, float height)
		{
#ifdef USE_PHYSX
			PxCapsuleControllerDesc desc;
			desc.material = material->px_material;
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

		Vec3f ControllerPrivate::get_position() const
		{
#ifdef USE_PHYSX
			auto p = px_controller->getPosition();
			return Vec3f(p.x, p.y, p.z);
#else
			return Vec3f(0.f);
#endif
		}

		void ControllerPrivate::set_position(const Vec3f& pos)
		{
#ifdef USE_PHYSX
			px_controller->setPosition(physx::PxExtendedVec3(pos.x(), pos.y(), pos.z()));
#endif
		}

		void ControllerPrivate::move(const Vec3f& disp, float delta_time)
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
