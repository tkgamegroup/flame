namespace flame
{
	enum LightType
	{
		LightTypeParallax,
		LightTypePoint,
		LightTypeSpot
	};

	class LightComponent : public Component
	{
	private:
		LightType type;
		glm::vec3 color;
		float range;
		bool enable_shadow;
		int light_index;
		int shadow_index;
	};
}
