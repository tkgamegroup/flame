#pragma once

#include "world_settings.h"

namespace flame
{
	struct cWorldSettingsPrivate : cWorldSettings
	{
		~cWorldSettingsPrivate();
		void set_filename(const std::filesystem::path& path) override;
		void save() override;
		void on_active() override;
	};
}
