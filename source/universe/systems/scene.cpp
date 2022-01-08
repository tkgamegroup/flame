#include "scene_private.h"

namespace flame
{
	sScenePrivate::~sScenePrivate()
	{
	}

	struct sSceneCreate : sScene::Create
	{
		sScenePtr operator()() override
		{
			return new sScenePrivate();
		}
	}sScene_create_private;
	sScene::Create& sScene::create = sScene_create_private;
}
