#pragma once

#include "graveyard.h"

namespace flame
{
	struct GraveyardPrivate : Graveyard
	{
		std::vector<std::pair<float, EntityPtr>> entities;

		GraveyardPrivate();

		void set_duration(float v) override;

		void add(EntityPtr e) override;
		void clear() override;
	};
}
