#pragma once

#include "graveyard.h"

namespace flame
{
	struct sGraveyardPrivate : sGraveyard
	{
		std::vector<std::pair<float, EntityPtr>> entities;

		sGraveyardPrivate();
		sGraveyardPrivate(int); // dummy

		void set_duration(float v) override;

		void add(EntityPtr e) override;
		void clear() override;

		void update() override;
	};
}
