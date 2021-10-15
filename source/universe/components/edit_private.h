#pragma once

#include "edit.h"

namespace flame
{
	struct cEditPrivate : cEdit
	{
		uint select_start = 0;
		uint select_end = 0;
		
		uint get_select_start() const override { return select_start; }
		void set_select_start(uint v) override;
		uint get_select_end() const override { return select_end; }
		void set_select_end(uint v) override;
	};
}
