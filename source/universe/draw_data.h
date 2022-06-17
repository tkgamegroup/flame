#pragma once

namespace flame
{
	struct MeshDraw
	{
		uint instance_id;
		uint mesh_id;
		uint mat_id;
		cvec4 color;
	};

	struct TerrainDraw
	{
		uint instance_id;
		uint blocks;
		cvec4 color;
	};

	struct LinesDraw
	{
		const vec3* points;
		uint count;
		cvec4 color;
	};

	struct DirectionalLight
	{
		uint instance_id;
		vec3 dir;
		vec3 color;
		float range;
	};

	struct DrawData
	{
		uint pass;
		uint category;

		std::vector<MeshDraw> draw_meshes;
		std::vector<TerrainDraw> draw_terrains;
		std::vector<LinesDraw> draw_lines;
		std::vector<DirectionalLight> directional_lights;
	};
}
