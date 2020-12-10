#pragma once

#include <flame/universe/drivers/window.h>

namespace flame
{
	struct dWindowPrivate : dWindow
	{
		void on_load_finished() override;
	};
}
