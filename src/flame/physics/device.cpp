#include "device_private.h"

namespace flame
{
	namespace physics
	{
		Device *create_device()
		{
			auto d = new Device;
			
			d->_priv = new DevicePrivate;
			d->_priv->foundation = PxCreateFoundation(PX_FOUNDATION_VERSION, d->_priv->allocator, d->_priv->error_callback);
			d->_priv->inst = PxCreatePhysics(PX_PHYSICS_VERSION, *d->_priv->foundation, PxTolerancesScale());

			return d;
		}

		void destroy_device(Device *d)
		{
			delete d->_priv;
			delete d;
		}
	}
}

