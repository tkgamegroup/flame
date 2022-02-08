#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "camera_private.h"

namespace flame
{
	void cCameraPrivate::update()
	{
		auto node = entity->get_component_i<cNodeT>(0);

		view_mat = mat4(node->rot);
		view_mat[3] = vec4(node->g_pos, 1.f);
		view_mat = inverse(view_mat);

		proj_mat = perspective(radians(fovy), aspect, zNear, zFar);
		proj_mat[1][1] *= -1.f;
	}

	static cCameraPtr _main = nullptr;

	void cCameraPrivate::on_active()
	{
		if (!_main || entity->compare_depth(_main->entity))
			_main = this;
	}

	void cCameraPrivate::on_inactive()
	{
		if (_main == this)
		{
			_main = nullptr;

			std::deque<EntityPtr> es;
			es.push_back(World::instance()->root.get());
			while (!es.empty())
			{
				auto e = es.front();
				es.pop_front();
				if (e != entity)
					_main = e->get_component_t<cCameraT>();
				if (_main)
					break;
				for (auto& c : e->children)
					es.push_back(c.get());
			}
		}
	}

	struct cCameraMain : cCamera::Main
	{
		cCameraPtr operator()() override
		{
			return _main;
		}
	}cCamera_main_private;
	cCamera::Main& cCamera::main = cCamera_main_private;

	struct cCameraCreate : cCamera::Create
	{
		cCameraPtr operator()(EntityPtr e) override
		{
			return new cCameraPrivate;
		}
	}cCamera_create_private;
	cCamera::Create& cCamera::create = cCamera_create_private;
}
