#pragma once

#include "polygon.h"

namespace flame
{
	struct cPolygonPrivate : cPolygon
	{
		~cPolygonPrivate();
		void on_init() override;
	};
}
