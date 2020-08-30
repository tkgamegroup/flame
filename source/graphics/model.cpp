#include <flame/foundation/foundation.h>
#include "model_private.h"

namespace flame
{
	namespace graphics
	{
		void ModelMesh::set_vertices_p(const std::initializer_list<float>& v)
		{
			assert(v.size() % 3 == 0);
			vertices.resize(v.size() / 3);
			for (auto i = 0; i < vertices.size(); i++)
			{
				vertices[i].pos.x() = v.begin()[i * 3 + 0];
				vertices[i].pos.y() = v.begin()[i * 3 + 1];
				vertices[i].pos.z() = v.begin()[i * 3 + 2];
			}
		}

		void ModelMesh::set_vertices_pn(const std::initializer_list<float>& v)
		{
			assert(v.size() % 6 == 0);
			vertices.resize(v.size() / 6);
			for (auto i = 0; i < vertices.size(); i++)
			{
				vertices[i].pos.x() = v.begin()[i * 6 + 0];
				vertices[i].pos.y() = v.begin()[i * 6 + 1];
				vertices[i].pos.z() = v.begin()[i * 6 + 2];
				vertices[i].normal.x() = v.begin()[i * 6 + 3];
				vertices[i].normal.y() = v.begin()[i * 6 + 4];
				vertices[i].normal.z() = v.begin()[i * 6 + 5];
			}
		}

		void ModelMesh::set_indices(const std::initializer_list<uint>& v)
		{
			indices.resize(v.size());
			auto idx = 0;
			for (auto i = 0; i < indices.size(); i++)
				indices[i] = v.begin()[i];
		}

		static ModelPrivate* standard_models[StandardModelCount];

		Model* Model::get_standard(StandardModel m)
		{
			auto ret = standard_models[m];
			if (ret)
				return ret;
			ret = new ModelPrivate;
			switch (m)
			{
			case StandardModelCube:
			{
				auto mesh = new ModelMesh;

				mesh->set_vertices_pn({
					// F
					-0.5f, +0.5f, +0.5f, +0.f, +0.f, +1.f,
					+0.5f, +0.5f, +0.5f, +0.f, +0.f, +1.f,
					+0.5f, -0.5f, +0.5f, +0.f, +0.f, +1.f,
					-0.5f, -0.5f, +0.5f, +0.f, +0.f, +1.f,
					// B
					-0.5f, +0.5f, -0.5f, +0.f, +0.f, -1.f,
					+0.5f, +0.5f, -0.5f, +0.f, +0.f, -1.f,
					+0.5f, -0.5f, -0.5f, +0.f, +0.f, -1.f,
					-0.5f, -0.5f, -0.5f, +0.f, +0.f, -1.f,
					// T
					-0.5f, +0.5f, -0.5f, +0.f, +1.f, +0.f,
					+0.5f, +0.5f, -0.5f, +0.f, +1.f, +0.f,
					+0.5f, +0.5f, +0.5f, +0.f, +1.f, +0.f,
					-0.5f, +0.5f, +0.5f, +0.f, +1.f, +0.f,
					// B
					-0.5f, -0.5f, -0.5f, +0.f, -1.f, +0.f,
					+0.5f, -0.5f, -0.5f, +0.f, -1.f, +0.f,
					+0.5f, -0.5f, +0.5f, +0.f, -1.f, +0.f,
					-0.5f, -0.5f, +0.5f, +0.f, -1.f, +0.f,
					// L
					-0.5f, +0.5f, -0.5f, -1.f, +0.f, +0.f,
					-0.5f, +0.5f, +0.5f, -1.f, +0.f, +0.f,
					-0.5f, -0.5f, +0.5f, -1.f, +0.f, +0.f,
					-0.5f, -0.5f, -0.5f, -1.f, +0.f, +0.f,
					// R
					+0.5f, +0.5f, -0.5f, +1.f, +0.f, +0.f,
					+0.5f, +0.5f, +0.5f, +1.f, +0.f, +0.f,
					+0.5f, -0.5f, +0.5f, +1.f, +0.f, +0.f,
					+0.5f, -0.5f, -0.5f, +1.f, +0.f, +0.f,
				});

				mesh->set_indices({
					0, 2, 1, 0, 3, 2, // F
					5, 7, 4, 5, 6, 7, // B
					8, 10, 9, 8, 11, 10, // T
					15, 13, 14, 15, 12, 13, // B
					16, 18, 17, 16, 19, 18, // L
					21, 23, 20, 21, 22, 23, // R

				});

				ret->meshes.emplace_back(mesh);
			}
				break;
			}
			standard_models[m] = ret;
			return ret;
		}

		Model* Model::create(const wchar_t* filename)
		{
			if (!std::filesystem::exists(filename))
				return nullptr;

			return new ModelPrivate();
		}
	}
}
