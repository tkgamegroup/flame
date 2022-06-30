#pragma once

#include "../math.h"

namespace flame
{
	struct MeshDraw
	{
		uint instance_id;
		uint mesh_id;
		uint mat_id;
		cvec4 color;

		MeshDraw(uint instance_id, uint mesh_id, uint mat_id, const cvec4& color = cvec4()) :
			instance_id(instance_id),
			mesh_id(mesh_id),
			mat_id(mat_id),
			color(color)
		{
		}
	};

	struct TerrainDraw
	{
		uint instance_id;
		uint blocks;
		uint mat_id;
		cvec4 color;

		TerrainDraw(uint instance_id, uint blocks, uint mat_id, const cvec4& color = cvec4()) :
			instance_id(instance_id),
			blocks(blocks),
			mat_id(mat_id),
			color(color)
		{
		}
	};

	struct PrimitiveDraw
	{
		uint type; // "LineList"_h or "TriangleList"_h
		std::vector<vec3> points;
		cvec4 color;

		PrimitiveDraw(uint type, std::vector<vec3>&& points, const cvec4& color) :
			type(type),
			points(points),
			color(color)
		{
		}

		PrimitiveDraw(uint type, const vec3* _points, uint count, const cvec4& color) :
			type(type),
			color(color)
		{
			points.resize(count);
			for (auto i = 0; i < count; i++)
				points[i] = _points[i];
		}
	};

	struct Light
	{
		uint instance_id;
		int type;
		vec3 pos;
		vec3 color;
		float range;
		bool cast_shadow;

		Light(uint instance_id, int type, const vec3& pos, const vec3& color, float range, bool cast_shadow) :
			instance_id(instance_id),
			type(type),
			pos(pos),
			color(color),
			range(range),
			cast_shadow(cast_shadow)
		{
		}
	};

	struct DrawData
	{
		uint pass;
		uint category;

		std::vector<Light> lights;
		std::vector<MeshDraw> draw_meshes;
		std::vector<TerrainDraw> draw_terrains;
		std::vector<PrimitiveDraw> draw_primitives;

		void reset(uint _pass, uint _category)
		{
			pass = _pass;
			category = _category;

			lights.clear();
			draw_meshes.clear();
			draw_terrains.clear();
			draw_primitives.clear();
		}
	};
}
