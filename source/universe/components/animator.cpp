#include "../entity_private.h"
#include "animator_private.h"

namespace flame
{
	void cAnimatorPrivate::set_modifiers(const std::vector<Modifier>& _modifiers)
	{
		if (modifiers == _modifiers)
			return;
		modifiers = _modifiers;

		modifiers_private.clear();
		for (auto& m : modifiers)
		{
			ModifierPrivate mp(m, entity, { std::make_pair("time", &time) });
			modifiers_private.push_back(std::move(mp));
		}

		data_changed("items"_h);
	}

	void cAnimatorPrivate::start()
	{
		time = 0.f;

		for (auto& m : modifiers_private)
			m.update(true);
	}

	void cAnimatorPrivate::update()
	{
		time += delta_time;

		for (auto& m : modifiers_private)
			m.update(false);
	}

	struct cAnimatorCreate : cAnimator::Create
	{
		cAnimatorPtr operator()(EntityPtr e) override
		{
			return new cAnimatorPrivate();
		}
	}cAnimator_create;
	cAnimator::Create& cAnimator::create = cAnimator_create;
}
