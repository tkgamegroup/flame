#pragma once

#include "../math.h"

namespace flame
{
	struct LightDraw
	{
		uint ins_id;
		bool cast_shadow;

		LightDraw(uint ins_id, bool cast_shadow) :
			ins_id(ins_id),
			cast_shadow(cast_shadow)
		{
		}
	};

	struct MeshDraw
	{
		uint ins_id;
		uint mesh_id;
		uint mat_id;
		cvec4 color;

		MeshDraw(uint ins_id, uint mesh_id, uint mat_id, const cvec4& color = cvec4()) :
			ins_id(ins_id),
			mesh_id(mesh_id),
			mat_id(mat_id),
			color(color)
		{
		}
	};

	struct TerrainDraw
	{
		uint ins_id;
		uint blocks;
		uint mat_id;
		cvec4 color;

		TerrainDraw(uint ins_id, uint blocks, uint mat_id, const cvec4& color = cvec4()) :
			ins_id(ins_id),
			blocks(blocks),
			mat_id(mat_id),
			color(color)
		{
		}
	};

	struct SdfDraw
	{
		uint ins_id;
		uint mat_id;

		SdfDraw(uint ins_id, uint mat_id) :
			ins_id(ins_id),
			mat_id(mat_id)
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

	struct DrawData
	{
		uint pass;
		uint category;

		std::vector<LightDraw>		directional_lights;
		std::vector<LightDraw>		point_lights;
		std::vector<MeshDraw>		meshes;
		std::vector<TerrainDraw>	terrains;
		std::vector<SdfDraw>		sdfs;
		std::vector<PrimitiveDraw>	primitives;

		void reset(uint _pass, uint _category)
		{
			pass = _pass;
			category = _category;

			directional_lights.clear();
			point_lights.clear();
			meshes.clear();
			terrains.clear();
			sdfs.clear();
			primitives.clear();
		}
	};
}
