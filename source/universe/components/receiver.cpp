#include "receiver_private.h"

namespace flame
{
	struct cReceiverCreate : cReceiver::Create
	{
		cReceiverPtr operator()(EntityPtr) override
		{
			return new cReceiverPrivate();
		}
	}cReceiver_create;
	cReceiver::Create& cReceiver::create = cReceiver_create;
}
