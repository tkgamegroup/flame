#pragma once

#include "data_updater.h"

namespace flame
{
	struct Attribute;

	struct cDataUpdaterPrivate : cDataUpdater
	{
		struct Modifier
		{
			const Attribute* attr = nullptr;
			void* obj = nullptr;
			std::unique_ptr<Expression> expr = nullptr;
		};

		float time = 0.f;

		std::vector<Modifier> modifiers;

		void set_items(const std::vector<std::pair<std::string, std::string>>& items) override;

		void start() override;
		void update() override;
	};
}
