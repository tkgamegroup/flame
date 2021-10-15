#include "input_dialog_private.h"

namespace flame
{
	cInputDialog* cInputDialog::create(void* parms)
	{
		return new cInputDialogPrivate();
	}
}
