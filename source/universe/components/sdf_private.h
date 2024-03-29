#pragma once

#include "sdf.h"

namespace flame
{
	struct cSdfPrivate : cSdf
	{
		bool dirty = true;

		~cSdfPrivate();
		void on_init() override;

		void on_active() override;
		void on_inactive() override;
	};
}
