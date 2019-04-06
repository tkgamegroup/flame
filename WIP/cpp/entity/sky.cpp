#include <flame/engine/entity/node.h>
#include <flame/engine/entity/scene.h>
#include <flame/engine/entity/light.h>
#include <flame/engine/entity/sky.h>

namespace flame
{
	const char *get_sky_type_name(SkyType type)
	{
		switch (type)
		{
			case SkyTypeNull:
				return "null";
			case SkyTypeDebug:
				return "debug";
			case SkyTypeAtmosphereScattering:
				return "atmosphere scattering";
			case SkyTypePanorama:
				return "panorama";
		}
	}

	Sky::Sky(SkyType _type)
		:type(_type)
	{
	}

	SkyAtmosphereScattering::SkyAtmosphereScattering(Scene *_scene) :
		Sky(SkyTypeAtmosphereScattering), 
		scene(_scene)
	{
		node = new Node(NodeTypeNode);
		node->name = "Sun Light";
		sun_light = new LightComponent;
		sun_light->set_type(LightTypeParallax);
		node->add_component(sun_light);
		scene->add_child(node);
	}

	SkyAtmosphereScattering::~SkyAtmosphereScattering()
	{
		scene->remove_child(node);
	}

	SkyPanorama::SkyPanorama() :
		Sky(SkyTypePanorama)
	{
	}
}
