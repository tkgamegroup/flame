#pragma once

#include "../system.h"

namespace flame
{
	struct sNodeRenderer : System
	{
		inline static auto type_name = "flame::sNodeRenderer";
		inline static auto type_hash = ch(type_name);

		bool dirty = false;

		sNodeRenderer() : System(type_name, type_hash)
		{
		}

		virtual int set_mesh_res(int idx, graphics::Mesh* mesh) = 0;
		virtual int find_mesh_res(graphics::Mesh* mesh) const = 0;

		// return: transform id
		virtual uint add_mesh_transform(const mat4& mat, const mat3& nor) = 0;
		// return: armature id
		virtual uint add_mesh_armature(const mat4* bones, uint count) = 0;
		// id: transform or armature id
		virtual void draw_mesh(uint id, uint mesh_id, uint skin, ShadingFlags flags = ShadingMaterial) = 0;

		struct Create
		{
			virtual sNodeRendererPtr operator()() = 0;
		};
		FLAME_UNIVERSE_EXPORTS static Create& create;
	};
}
