#include "text_private.h"

namespace flame
{
	struct cImageCreate : cImage::Create
	{
		cImagePtr operator()(EntityPtr) override
		{
			return new cImagePrivate();
		}
	}cImage_create;
	cImage::Create& cImage::create = cImage_create;
}
