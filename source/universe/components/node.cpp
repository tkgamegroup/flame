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

	void cNodePrivate::set_euler(const Vec3f& e)
	{
		auto qy = make_quat(e.x(), Vec3f(0.f, 1.f, 0.f));
		auto qp = make_quat(e.y(), Vec3f(1.f, 0.f, 0.f));
		auto qr = make_quat(e.z(), Vec3f(0.f, 0.f, 1.f));
		set_quat(quat_mul(quat_mul(qy, qp), qr));
	}

	Vec3f cNodePrivate::get_dir(uint idx)
	{
		update_transform();

		return axes[idx];
	}

	void cNodePrivate::update_transform()
	{
		if (transform_dirty)
		{
			transform_dirty = false;

			auto pn = entity->get_parent_component_t<cNodePrivate>();
			if (pn)
			{
				pn->update_transform();
				global_pos = pn->global_pos + quat_mul(pn->global_quat, pos * pn->global_scale);
				global_quat = quat_mul(pn->global_quat, quat);
				global_scale = pn->global_scale * scale;
			}
			else
			{
				global_pos = pos;
				global_quat = quat;
				global_scale = scale;
			}

			axes = make_rotation_matrix(global_quat);
			transform = Mat4f(Mat<3, 4, float>(axes * Mat3f(global_scale), Vec3f(0.f)), Vec4f(global_pos, 1.f));

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
