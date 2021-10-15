#include "edit_private.h"

namespace flame
{
	void cEditPrivate::set_select_start(uint v)
	{
		select_start = v;
	}
	
	void cEditPrivate::set_select_end(uint v)
	{
		select_end = v;
	}
	
	cEdit* cEdit::create(void* parms)
	{
		return new cEditPrivate();
	}
}
