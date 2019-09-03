namespace flame
{
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
}
