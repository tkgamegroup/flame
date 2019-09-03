namespace flame
{
	class WaterComponent : public Component
	{
	private:
		int block_cx = 64;
		int block_cy = 64;
		float block_size = 16.f;
		float height = 10.f;
		float tessellation_factor = 0.75f;
		float tiling_scale = 8.f;

		int water_index;
	};
}
