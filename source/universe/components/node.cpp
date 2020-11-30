#include <flame/foundation/typeinfo.h>
#include "../entity_private.h"
#include "node_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cNodePrivate::set_pos(const vec3& p)
	{
		if (pos == p)
			return;
		pos = p;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"pos"_h>);
	}

	void cNodePrivate::set_quat(const quat& q)
	{
		if (qut == q)
			return;
		qut = q;
		rot_dirty = true;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"quat"_h>);
	}

	void cNodePrivate::set_scale(const vec3& s)
	{
		if (scl == s)
			return;
		scl = s;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"scale"_h>);
	}

	void cNodePrivate::set_euler(const vec3& e)
	{
		auto m = mat4(1.f);
		m = rotate(m, radians(e.x), vec3(0.f, 1.f, 0.f));
		m = rotate(m, radians(e.y), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(e.z), vec3(0.f, 0.f, 1.f));
		rot = mat3(m);
		qut_dirty = true;
		mark_transform_dirty();
	}

	vec3 cNodePrivate::get_local_dir(uint idx)
	{
		update_rot();

		return rot[idx];
	}

	vec3 cNodePrivate::get_global_pos()
	{
		update_transform();

		return transform[3];
	}

	vec3 cNodePrivate::get_global_dir(uint idx)
	{
		update_transform();

		return g_rot[idx];
	}

	void cNodePrivate::update_qut()
	{
		if (qut_dirty)
		{
			qut_dirty = false;

			qut = quat(rot);
		}
	}

	void cNodePrivate::update_rot()
	{
		if (rot_dirty)
		{
			rot_dirty = false;

			rot = mat3(qut);
		}
	}

	void cNodePrivate::update_transform()
	{
		if (transform_dirty)
		{
			transform_dirty = false;

			update_rot();
			update_qut();

			g_qut = qut;
			g_rot = rot;
			g_scl = scl;
			mat4 m(1.f);
			auto pn = entity->get_parent_component_t<cNodePrivate>();
			if (pn)
			{
				pn->update_transform();
				g_qut = pn->g_qut * g_qut;
				g_rot = pn->g_rot * g_rot;
				g_scl = pn->g_scl * g_scl;
				m = pn->transform;
			}
			m = translate(m, pos);
			g_pos = m[3];
			m = m * mat4(rot);
			m = scale(m, scl);
			transform = m;

			Entity::report_data_changed(this, S<"transform"_h>);
		}
	}

	void cNodePrivate::mark_transform_dirty()
	{
		if (!transform_dirty)
		{
			transform_dirty = true;

			for (auto& c : entity->children)
			{
				auto n = c->get_component_t<cNodePrivate>();
				if (n)
					n->mark_transform_dirty();
			}
		}
		mark_drawing_dirty();
	}

	void cNodePrivate::mark_drawing_dirty()
	{
		if (renderer)
			renderer->dirty = true;
	}

	void cNodePrivate::on_local_message(Message msg, void* p)
	{
		switch (msg)
		{
		case MessageComponentAdded:
		{
			auto udt = find_underlay_udt(((Component*)p)->type_name);
			if (udt)
			{
				{
					auto f = udt->find_function("draw");
					if (f && f->check(TypeInfo::get(TypeData, "void"), TypeInfo::get(TypePointer, "flame::graphics::Canvas"), nullptr))
					{
						auto addr = f->get_address();
						if (addr)
						{
							drawers.emplace_back((Component*)p, (void(*)(Component*, graphics::Canvas*))addr);
							mark_drawing_dirty();
						}
					}
				}
			}
		}
			break;
		case MessageComponentRemoved:
		{
			if (std::erase_if(drawers, [&](const auto& i) {
				return i.first == (Component*)p;
			}))
				mark_drawing_dirty();
		}
			break;
		}
	}

	cNode* cNode::create()
	{
		return f_new<cNodePrivate>();
	}
}
