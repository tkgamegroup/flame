#include "scene_private.h"
#include "controller_private.h"

namespace flame
{
	namespace physics
	{
		ControllerPrivate::ControllerPrivate(ScenePrivate* scene, float radius, float height)
		{
			PxCapsuleControllerDesc desc;
			desc.radius = radius;
			desc.height = height;
			px_controller = scene->px_controller_manager->createController(desc);
		}

		ControllerPrivate::~ControllerPrivate()
		{
			px_controller->release();
		}

		Vec3f ControllerPrivate::get_position() const
		{
			auto p = px_controller->getPosition();
			return Vec3f(p.x, p.y, p.z);
		}

		void ControllerPrivate::set_position(const Vec3f& pos)
		{
			px_controller->setPosition(physx::PxExtendedVec3(pos.x(), pos.y(), pos.z()));
		}

		void ControllerPrivate::move(const Vec3f& disp, float delta_time)
		{
			px_controller->move(PxVec3(disp.x(), disp.y(), disp.z()), 0.f, delta_time, physx::PxControllerFilters());
		}

		Controller* Controller::create(Scene* scene, float radius, float height)
		{
			return new ControllerPrivate((ScenePrivate*)scene, radius, height);
		}
	}
}
