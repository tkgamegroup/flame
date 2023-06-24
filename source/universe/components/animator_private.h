#pragma once

#include "../universe_private.h"
#include "animator.h"

namespace flame
{
	struct cAnimatorPrivate : cAnimator
	{
		float time = 0.f;

		std::vector<ModifierPrivate> modifiers_private;

		void set_modifiers(const std::vector<Modifier>& modifiers) override;

		void start() override;
		void update() override;
	};
}
