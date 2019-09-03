namespace flame
{
	LightComponent::LightComponent() :
		Component(ComponentTypeLight), 
		type(LightTypePoint),
		color(0.5f),
		range(0.5f),
		enable_shadow(false),
		light_index(-1),
		shadow_index(-1),
		attribute_dirty_frame(0)
	{
	}

	void LightComponent::set_enable_shadow(bool v)
	{
		if (enable_shadow == v)
			return;

		enable_shadow = v;
		attribute_dirty_frame = total_frame_count;
		broadcast(this, MessageToggleShaodw);
	}
}
