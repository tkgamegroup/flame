#pragma once

#include "input_field.h"

namespace flame
{
	struct cInputFieldPrivate : cInputField
	{
		void set_text_component(const GUID& guid) override;
	};
}
