#include "drag_edit_private.h"

namespace flame
{
	cDragEdit* cDragEdit::create(void* parms)
	{
		return new cDragEditPrivate();
	}
}
