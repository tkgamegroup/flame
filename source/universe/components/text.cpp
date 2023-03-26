#include "text_private.h"

namespace flame
{
	struct cTextCreate : cText::Create
	{
		cTextPtr operator()(EntityPtr) override
		{
			return new cTextPrivate();
		}
	}cText_create;
	cText::Create& cText::create = cText_create;
}
