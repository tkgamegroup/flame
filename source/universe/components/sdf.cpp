#include "node_private.h"
#include "sdf_private.h"
#include "../draw_data.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cSdfPrivate::~cSdfPrivate()
	{
		node->drawers.remove("sdf"_h);
		node->measurers.remove("sdf"_h);
	}
	
	void cSdfPrivate::on_init()
	{
		node->drawers.add([this](DrawData& draw_data, cCameraPtr camera) {
			if (instance_id == -1)
				return;

			switch (draw_data.pass)
			{
			case PassInstance:
				if (dirty)
				{
					sRenderer::instance()->set_sdf_instance(instance_id, 0, nullptr, 0, nullptr);
					dirty = false;
				}
				break;
			case PassGBuffer:
				if (draw_data.categories & CateSDF)
					draw_data.sdfs.emplace_back(instance_id, 0);
				break;
			}
		}, "sdf"_h);
		node->measurers.add([this](AABB& b) {
			b.expand(AABB(AABB(vec3(0.f), 10.f).get_points(node->transform)));
		}, "sdf"_h);

		node->mark_transform_dirty();
	}

	void cSdfPrivate::on_active()
	{
		instance_id = sRenderer::instance()->register_sdf_instance(-1);

		node->mark_transform_dirty();
	}

	void cSdfPrivate::on_inactive()
	{
		sRenderer::instance()->register_sdf_instance(instance_id);
		instance_id = -1;
	}

	struct cSdfCreate : cSdf::Create
	{
		cSdfPtr operator()(EntityPtr e) override
		{
			return new cSdfPrivate();
		}
	}cSdf_create;
	cSdf::Create& cSdf::create = cSdf_create;
}
