#include "drop_down_private.h"

namespace flame
{
	cDropDown* cDropDown::create(void* parms)
	{
		return new cDropDownPrivate();
	}
}
