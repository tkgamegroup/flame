#include <flame/universe/components/splitter.h>

namespace flame
{
	struct cEventReceiverPrivate;

	struct cSplitterPrivate : cSplitter // R ~ on_*
	{
		SplitterType type;

		cEventReceiverPrivate* bar_event_receiver = nullptr; // R ref place=splitter_bar

		void on_gain_bar_event_receiver();
		void on_lost_bar_event_receiver();
	};
}
