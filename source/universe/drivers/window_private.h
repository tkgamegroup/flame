#pragma once

#include "../entity_private.h"
#include <flame/universe/drivers/window.h>

namespace flame
{
	struct cElementPrivate;
	struct cReceiverPrivate;

	struct dWindowPrivate : dWindow
	{
		cElementPrivate* element;
		cReceiverPrivate* receiver;
		cReceiverPrivate* size_dragger_receiver;

		void on_load_finished() override;
	};
}
