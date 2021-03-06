#pragma once

#include <flame/universe/driver.h>

namespace flame
{
	struct dMenu : Driver
	{
		inline static auto type_name = "flame::dMenu";
		inline static auto type_hash = ch(type_name);

		dMenu() :
			Driver(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static dMenu* create(void* parms = nullptr);
	};

	struct dMenuItem : Driver
	{
		inline static auto type_name = "flame::dMenuItem";
		inline static auto type_hash = ch(type_name);

		dMenuItem() :
			Driver(type_name, type_hash)
		{
		}

		virtual bool get_checkable() const = 0;
		virtual void set_checkable(bool v) = 0;

		virtual bool get_checked() const = 0;
		virtual void set_checked(bool v) = 0;
		virtual void set_single_checked() = 0;

		FLAME_UNIVERSE_EXPORTS static dMenuItem* create(void* parms = nullptr);
	};

	struct dMenuBar : Driver
	{
		inline static auto type_name = "flame::dMenuBar";
		inline static auto type_hash = ch(type_name);

		dMenuBar() :
			Driver(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static dMenuBar* create(void* parms = nullptr);
	};
}
