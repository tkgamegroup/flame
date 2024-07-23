#include "body2d_private.h"
#include "element_private.h"

namespace flame
{
	std::vector<cBody2dPtr> bodies2d;

	vec2 cBody2dPrivate::get_velocity()
	{
		if (body)
			return body->get_velocity();
		return vec2(0.f);
	}

	void cBody2dPrivate::upload_pos()
	{
		if (body)
			body->set_pos(element->pos);
	}

	void cBody2dPrivate::apply_force(const vec2& force)
	{
		if (body)
			body->apply_force(force);
	}

	void cBody2dPrivate::on_active()
	{
		bodies2d.push_back(this);
	}

	void cBody2dPrivate::on_inactive()
	{
		std::erase_if(bodies2d, [&](const auto i) {
			return i == this;
		});
	}

	void cBody2dPrivate::update() 
	{
		if (body)
		{
			element->set_pos(body->pos);
		}
	}

	struct cBody2dCreate : cBody2d::Create
	{
		cBody2dPtr operator()(EntityPtr e) override
		{
			return new cBody2dPrivate();
		}
	}cBody2d_create;
	cBody2d::Create& cBody2d::create = cBody2d_create;
}
