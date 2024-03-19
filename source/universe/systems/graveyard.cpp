#include "../entity_private.h"
#include "graveyard_private.h"

namespace flame
{
	sGraveyardPrivate::sGraveyardPrivate()
	{
	}

	void sGraveyardPrivate::set_duration(float v)
	{
		if (duration == v)
			return;
		duration = v;
	}

	void sGraveyardPrivate::add(EntityPtr e)
	{
		entities.emplace_back(duration, e);
	}

	void sGraveyardPrivate::clear()
	{
		entities.clear();
	}

	void sGraveyardPrivate::update()
	{
		for (auto it = entities.begin(); it != entities.end(); )
		{
			if (it->first > 0)
			{
				it->first -= delta_time;
				++it;
			}
			else
			{
				auto e = it->second;
				if (e->parent)
					e->remove_from_parent();
				else
					delete e;
				it = entities.erase(it);
			}
		}
	}

	static sGraveyardPtr _instance = nullptr;

	struct sGraveyardInstance : sGraveyard::Instance
	{
		sGraveyardPtr operator()() override
		{
			return _instance;
		}
	}sGraveyard_instance;
	sGraveyard::Instance& sGraveyard::instance = sGraveyard_instance;

	struct sGraveyardCreate : sGraveyard::Create
	{
		sGraveyardPtr operator()(WorldPtr w) override
		{
			if (!w)
				return nullptr;

			assert(!_instance);
			_instance = new sGraveyardPrivate();
			return _instance;
		}
	}sGraveyard_create;
	sGraveyard::Create& sGraveyard::create = sGraveyard_create;
}
