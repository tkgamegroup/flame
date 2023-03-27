#include "../entity_private.h"
#include "element_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cElementPrivate::set_pos(const vec2& p)
	{
		if (pos == p)
			return;
		pos = p;
		mark_transform_dirty();
		data_changed("pos"_h);
	}

	void cElementPrivate::set_ext(const vec2& e)
	{
		if (ext == e)
			return;
		ext = e;
		mark_transform_dirty();
		data_changed("ext"_h);
	}

	void cElementPrivate::set_scl(const vec2& s)
	{
		if (scl == s)
			return;
		scl = s;
		mark_transform_dirty();
		data_changed("scl"_h);
	}

	void cElementPrivate::mark_transform_dirty()
	{
		transform_dirty = true;
		mark_drawing_dirty();
	}

	void cElementPrivate::mark_drawing_dirty()
	{
		if (entity->depth != (ushort)-1)
			sRenderer::instance()->dirty = true;
	}

	bool cElementPrivate::update_transform()
	{
		if (!transform_dirty)
			return false;

		mat3 m;
		if (auto pnode = entity->get_parent_component_i<cElementT>(0); pnode)
		{
			m = pnode->transform;
		}
		else
		{
			m = mat3(1.f);
		}
		m = translate(m, pos);
		m = scale(m, scl);
		transform = m;

		data_changed("transform"_h);
		transform_dirty = false;

		return true;
	}

	struct cElementCreate : cElement::Create
	{
		cElementPtr operator()(EntityPtr) override
		{
			return new cElementPrivate();
		}
	}cElement_create;
	cElement::Create& cElement::create = cElement_create;
}
