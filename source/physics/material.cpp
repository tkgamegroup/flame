#include "material_private.h"
#include "device_private.h"

namespace flame
{
	namespace physics
	{
		Material *create_material(Device *d, float static_friction, float dynamic_friction, float restitution)
		{
			auto m = new Material;
			
			m->_priv = new MaterialPrivate;
			m->_priv->v = d->_priv->inst->createMaterial(static_friction, dynamic_friction, restitution);

			return m;
		}

		void destroy_material(Material *m)
		{
			m->_priv->v->release();

			delete m->_priv;
			delete m;
		}
	}
}

