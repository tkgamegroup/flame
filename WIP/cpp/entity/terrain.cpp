namespace flame
{
	TerrainComponent::TerrainComponent() :
		Component(ComponentTypeTerrain),
		block_cx(64), 
		block_cy(64), 
		block_size(16.f),
		height(100.f),
		displacement_height(1.f),
		tessellation_factor(0.75f),
		tiling_scale(8.f),
		height_image(default_height_texture),
		blend_image(default_blend_texture),
		material_count(0),
		enable_physics(false),
		actor(nullptr),
		terrain_index(-1),
		attribute_dirty_frame(0),
		blend_image_dirty_frame(0)
	{
		add_material(default_material);
	}

	Material *TerrainComponent::get_material(int index) const
	{
		if (index < material_count)
			return materials[index].get();
		return nullptr;
	}

	void TerrainComponent::add_material(std::shared_ptr<Material> m)
	{
		if (material_count == TK_ARRAYSIZE(materials))
			return;

		materials[material_count] = m;
		material_count++;
		attribute_dirty_frame = total_frame_count;
	}

	void TerrainComponent::remove_material(int index)
	{
		if (index >= material_count)
			return;

		for (int i = index; i < material_count - 1; i++)
			materials[i] = materials[i + 1];
		material_count--;
		materials[material_count].reset();
		attribute_dirty_frame = total_frame_count;
	}
}
