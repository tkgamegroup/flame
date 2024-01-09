#pragma once

#include "universe.h"

namespace flame
{
	struct LightData
	{
		LightType type;
		uint ins_id;
		bool cast_shadow;

		LightData(LightType type, uint ins_id, bool cast_shadow) :
			type(type),
			ins_id(ins_id),
			cast_shadow(cast_shadow)
		{
		}
	};

	struct MeshDrawData
	{
		uint ins_id;
		uint mesh_id;
		uint mat_id;
		cvec4 color;

		MeshDrawData(uint ins_id, uint mesh_id, uint mat_id, const cvec4& color = cvec4()) :
			ins_id(ins_id),
			mesh_id(mesh_id),
			mat_id(mat_id),
			color(color)
		{
		}
	};

	struct TerrainDrawData
	{
		uint ins_id;
		uvec2 blocks;
		uint mat_id;
		cvec4 color;

		TerrainDrawData(uint ins_id, const uvec2& blocks, uint mat_id, const cvec4& color = cvec4()) :
			ins_id(ins_id),
			blocks(blocks),
			mat_id(mat_id),
			color(color)
		{
		}
	};

	struct SdfDrawData
	{
		uint ins_id;
		uint mat_id;

		SdfDrawData(uint ins_id, uint mat_id) :
			ins_id(ins_id),
			mat_id(mat_id)
		{
		}
	};

	struct VolumeDrawData
	{
		uint ins_id;
		uvec3 blocks;
		uint mat_id;

		VolumeDrawData(uint ins_id, const uvec3& blocks, uint mat_id) :
			ins_id(ins_id),
			blocks(blocks),
			mat_id(mat_id)
		{
		}
	};

	struct ParticleDrawData
	{
		struct Ptc
		{
			float time;
			vec3 pos0;
			vec3 pos1;
			vec3 pos2;
			vec3 pos3;
			vec4 uv;
			cvec4 col;
		};

		uint mat_id;
		std::vector<Ptc> ptcs;
	};

	struct ObjectDrawData
	{
		// type: "mesh"_h, "terrain"_h
		uint type;
		uint res_id;
		uint ins_id;

		ObjectDrawData(uint type, uint res_id, uint ins_id) :
			type(type),
			res_id(res_id),
			ins_id(ins_id)
		{
		}
	};

	enum DrawPass
	{
		PassInstance,
		PassLight,
		PassGBuffer,
		PassForward,
		PassOcculder,
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

		std::vector<LightData>			lights;
		std::vector<MeshDrawData>		meshes;
		std::vector<TerrainDrawData>	terrains; // or grass fields
		std::vector<SdfDrawData>		sdfs;
		std::vector<VolumeDrawData>		volumes; // or marching cubes
		std::vector<ParticleDrawData>	particles;

		bool graphics_debug; // could use this to request a capture

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

			graphics_debug = false;
		}
	};
}
