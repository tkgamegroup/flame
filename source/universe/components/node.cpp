#include <flame/foundation/typeinfo.h>
#include "../entity_private.h"
#include "node_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cNodePrivate::set_pos(const Vec3f& p)
	{
		if (pos == p)
			return;
		pos = p;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<ch("pos")>::v);
	}

	void cNodePrivate::set_quat(const Vec4f& q)
	{
		if (quat == q)
			return;
		quat = q;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<ch("quat")>::v);
	}

	void cNodePrivate::set_scale(const Vec3f& s)
	{
		if (scale == s)
			return;
		scale = s;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<ch("scale")>::v);
	}

	void cNodePrivate::update_transform()
	{
		if (transform_dirty)
		{
			transform_dirty = false;

			rotate_matrix = get_rotation_matrix(quat);
			transform = Mat4f(Mat<3, 4, float>(rotate_matrix * Mat3f(Vec3f(scale.x(), 0.f, 0.f), Vec3f(0.f, scale.y(), 0.f), Vec3f(0.f, 0.f, scale.z())),
				Vec3f(0.f)), Vec4f(pos, 1.f));

			auto p = entity->parent;
			if (p)
			{
				auto pn = p->get_component_t<cNodePrivate>();
				if (pn)
				{
					pn->update_transform();
					transform = pn->transform * transform;
				}
			}

			Entity::report_data_changed(this, S<ch("transform")>::v);
		}
	}

	void cNodePrivate::mark_transform_dirty()
	{
		if (!transform_dirty)
		{
			transform_dirty = true;
			for (auto& c : entity->children)
			{
				auto e = c->get_component_t<cNodePrivate>();
				e->mark_transform_dirty();
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
					if (f && f->check(TypeInfo::get(TypeData, "void"), TypeInfo::get(TypePointer, "flame::graphics::Canvas"), TypeInfo::get(TypePointer, "flame::cCamera"), nullptr))
					{
						auto addr = f->get_address();
						if (addr)
						{
							drawers.emplace_back((Component*)p, (void(*)(Component*, graphics::Canvas*, cCamera*))addr);
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
