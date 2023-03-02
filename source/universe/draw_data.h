#pragma once

#include "universe.h"

namespace flame
{
	struct LightDraw
	{
		LightType type;
		uint ins_id;
		bool cast_shadow;

		LightDraw(LightType type, uint ins_id, bool cast_shadow) :
			type(type),
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
		uvec2 blocks;
		uint mat_id;
		cvec4 color;

		TerrainDraw(uint ins_id, const uvec2& blocks, uint mat_id, const cvec4& color = cvec4()) :
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

	struct VolumeDraw
	{
		uint ins_id;
		uvec3 blocks;
		uint mat_id;

		VolumeDraw(uint ins_id, const uvec3& blocks, uint mat_id) :
			ins_id(ins_id),
			blocks(blocks),
			mat_id(mat_id)
		{
		}
	};

	struct ParticleDraw
	{
		struct Ptc
		{
			float time;
			vec3 pos;
			vec3 x_ext;
			vec3 y_ext;
			vec4 uv;
			cvec4 col;
		};

		uint type; // "Billboard"_h
		uint mat_id;
		std::vector<Ptc> ptcs;
	};

	struct PrimitiveDraw
	{
		uint type; // "LineList"_h, "LineStrip"_h, "TriangleList"_h
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

	enum DrawPass
	{
		PassInstance,
		PassLight,
		PassGBuffer,
		PassForward,
		PassOcculder,
		PassOutline,
		PassPrimitive,
		PassPickUp,
		PassTransformFeedback
	};

	enum DrawCategory
	{
		CateLight = 1 << 0,
		CateMesh = 1 << 1,
		CateTerrain = 1 << 2,
		CateGrassField = 1 << 3,
		CateSDF = 1 << 4,
		CateVolume = 1 << 5,
		CateMarchingCubes = 1 << 6,
		CateParticle = 1 << 7,
		CatePrimitive = 1 << 8
	};

	struct DrawData
	{
		DrawPass pass;
		uint categories;

		std::vector<LightDraw>		lights;
		std::vector<MeshDraw>		meshes;
		std::vector<TerrainDraw>	terrains; // or grass fields
		std::vector<SdfDraw>		sdfs;
		std::vector<VolumeDraw>		volumes; // or marching cubes
		std::vector<ParticleDraw>	particles;
		std::vector<PrimitiveDraw>	primitives;

		float line_width;

		bool graphics_debug; // could use this to mark a capture

		void reset(DrawPass _pass, uint _categories)
		{
			pass = _pass;
			categories = _categories;

			lights.clear();
			meshes.clear();
			terrains.clear();
			sdfs.clear();
			volumes.clear();
			particles.clear();
			primitives.clear();

			line_width = 1.f;

			graphics_debug = false;
		}
	};
}
