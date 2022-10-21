#include "audio_listener_private.h"
#include "node_private.h"
#include "../entity_private.h"
#include "../../audio/listener.h"

namespace flame
{
	std::vector<cAudioListenerPtr> listeners;

	void cAudioListenerPrivate::on_active()
	{
		for (auto it = listeners.begin(); it != listeners.end(); it++)
		{
			if (entity->compare_depth((*it)->entity))
			{
				listeners.insert(it, this);
				return;
			}
		}
		listeners.push_back(this);
	}

	void cAudioListenerPrivate::on_inactive()
	{
		std::erase_if(listeners, [&](auto c) {
			return c == this;
		});
	}

	void cAudioListenerPrivate::update()
	{
#if USE_AUDIO_MODULE
		if (listeners.front() == this)
			audio::Listener::get()->set_pos(node->g_pos);
#endif
	}

	struct cAudioListenerCreate : cAudioListener::Create
	{
		cAudioListenerPtr operator()(EntityPtr e) override
		{
			return new cAudioListenerPrivate();
		}
	}cAudioListener_create;
	cAudioListener::Create& cAudioListener::create = cAudioListener_create;
}
