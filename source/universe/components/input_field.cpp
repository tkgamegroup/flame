#include "input_field_private.h"

namespace flame
{

	struct cInputFieldCreate : cInputField::Create
	{
		cInputFieldPtr operator()(EntityPtr e) override
		{
			return new cInputFieldPrivate();
		}
	}cInputField_create;
	cInputField::Create& cInputField::create = cInputField_create;
}
