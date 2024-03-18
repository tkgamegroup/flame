#include "../entity_private.h"
#include "tween_private.h"

namespace flame
{
	TweenPrivate::TweenPrivate()
	{

	}

	uint TweenPrivate::begin(EntityPtr e)
	{
		return 0;
	}

	void TweenPrivate::end(uint id)
	{

	}

	void TweenPrivate::newline(uint id)
	{

	}

	void TweenPrivate::wait(uint id, float time)
	{

	}

	void TweenPrivate::move_to(uint id, const vec3& pos, float duration)
	{
	}

	void TweenPrivate::rotate_to(uint id, const quat& rot, float duration)
	{

	}

	void TweenPrivate::rotate_to(uint id, const vec3& eul, float duration)
	{
	}

	void TweenPrivate::scale_to(uint id, float scale, float duration)
	{
	}

	void TweenPrivate::scale_to(uint id, const vec3& scale, float duration)
	{

	}

	void TweenPrivate::play_animation(uint id, uint name)
	{

	}

	void TweenPrivate::kill(uint id)
	{

	}

	static TweenPtr _instance = nullptr;

	struct TweenInstance : Tween::Instance
	{
		TweenPtr operator()() override
		{
			if (_instance == nullptr)
				_instance = new TweenPrivate();
			return _instance;
		}
	}Tween_instance;
	Tween::Instance& Tween::instance = Tween_instance;
}

