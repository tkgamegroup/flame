#pragma once

#include "data_updater.h"

namespace flame
{
	struct Attribute;

	struct cDataUpdaterPrivate : cDataUpdater
	{
		std::vector<std::tuple<const Attribute*, void*, std::unique_ptr<Expression>>> expressions;

		void set_items(const std::vector<std::pair<std::string, std::string>>& items) override;

		void update() override;
	};
}
