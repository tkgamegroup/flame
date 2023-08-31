#include "node_mesh.h"
#include "../../../foundation/typeinfo.h"
#include "../../image_private.h"
#include "../../texture.h"
#include "../../noise.h"
#include "../../model_private.h"
#include "../../model_ext.h"

namespace flame
{
	namespace graphics
	{
		void add_node_templates_mesh(BlueprintNodeLibraryPtr library)
		{
			library->add_template("Control Mesh: Cube", "",
				{
					{
						.name = "Extent",
						.allowed_types = { TypeInfo::get<vec3>() },
						.default_value = "1,1,1"
					}
				},
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh>() }
					}
				},
				[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
					auto extent = *(vec3*)inputs[0].data;
					auto& mesh = *(ControlMesh*)outputs[0].data;
					mesh.init_as_cube(extent);
				},
				nullptr,
				nullptr,
				nullptr,
				nullptr
			);
			library->add_template("Convert To Mesh", "",
				{
					{
						.name = "Control Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh*>() }
					}
				},
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<Mesh>() }
					}
				},
				[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
					auto pcontrol_mesh = *(ControlMesh**)inputs[0].data;
					auto& mesh = *(Mesh*)outputs[0].data;
					if (pcontrol_mesh)
					{
						mesh.reset();
						pcontrol_mesh->convert_to_mesh(mesh);
						mesh.calc_bounds();
					}
				},
				nullptr,
				nullptr,
				nullptr,
				[](BlueprintArgument* inputs, BlueprintArgument* outputs, BlueprintNodePreview* preview) {
					auto& mesh = *(Mesh*)outputs[0].data;
					preview->type = "mesh"_h;
					preview->data = &mesh;
				}
			);
			library->add_template("Cube Mesh", "",
				{
					{
						.name = "Extent",
						.allowed_types = { TypeInfo::get<vec3>() },
						.default_value = "1,1,1"
					}
				},
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<Mesh>() }
					}
				},
				[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
					auto extent = *(vec3*)inputs[0].data;
					auto& mesh = *(Mesh*)outputs[0].data;
					mesh.reset();
					mesh_add_cube(mesh, extent, vec3(0.f), mat3(1.f));
					mesh.calc_bounds();
				},
				nullptr,
				nullptr,
				nullptr,
				[](BlueprintArgument* inputs, BlueprintArgument* outputs, BlueprintNodePreview* preview) {
					auto& mesh = *(Mesh*)outputs[0].data;
					preview->type = "mesh"_h;
					preview->data = &mesh;
				}
			);
			library->add_template("Subdivide Control Mesh", "",
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh*>() }
					},
					{
						.name = "Levels",
						.allowed_types = { TypeInfo::get<uint>() },
						.default_value = "1"
					}
				},
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh>() }
					}
				},
				[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
					auto pcontrol_mesh = *(ControlMesh**)inputs[0].data;
					auto levels = *(uint*)inputs[1].data;
					auto& mesh = *(ControlMesh*)outputs[0].data;
					if (pcontrol_mesh)
					{
						if (levels == 0)
						{
							mesh.vertices = pcontrol_mesh->vertices;
							mesh.faces = pcontrol_mesh->faces;
						}
						else
						{
							auto& current_level = *pcontrol_mesh;
							ControlMesh next_level;
							for (auto i = 0; i < levels; i++)
							{
								current_level.subdivide(next_level);
								current_level = next_level;
							}
							mesh = std::move(next_level);
						}
					}
				},
				nullptr,
				nullptr,
				nullptr,
				nullptr
			);
			library->add_template("Displace Control Mesh", "",
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh*>() }
					},
					{
						.name = "Texture",
						.allowed_types = { TypeInfo::get<Texture*>() }
					},
					{
						.name = "Strength",
						.allowed_types = { TypeInfo::get<float>() },
						.default_value = "1"
					},
					{
						.name = "Midlevel",
						.allowed_types = { TypeInfo::get<float>() },
						.default_value = "0.5"
					}
				},
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh>() }
					}
				},
				[](BlueprintArgument* inputs, BlueprintArgument* outputs) {
					auto pcontrol_mesh = *(ControlMesh**)inputs[0].data;
					auto ptexture = *(Texture**)inputs[1].data;
					auto strength = *(float*)inputs[2].data;
					auto midlevel = *(float*)inputs[3].data;
					auto& mesh = *(ControlMesh*)outputs[0].data;
					if (pcontrol_mesh && ptexture)
					{
						std::vector<std::vector<uint>> vertices_adjacent_faces(pcontrol_mesh->vertices.size());

						for (auto i = 0; i < pcontrol_mesh->faces.size(); i++)
						{
							auto& f = pcontrol_mesh->faces[i];

							uint v0 = f.corners[0].vertex_id;
							uint v1 = f.corners[1].vertex_id;
							uint v2 = f.corners[2].vertex_id;
							uint v3 = f.corners[3].vertex_id;

							vertices_adjacent_faces[v0].push_back(i);
							vertices_adjacent_faces[v1].push_back(i);
							vertices_adjacent_faces[v2].push_back(i);
							vertices_adjacent_faces[v3].push_back(i);
						}

						mesh.reset();
						mesh.vertices = pcontrol_mesh->vertices;
						mesh.faces = pcontrol_mesh->faces;

						for (auto i = 0; i < pcontrol_mesh->vertices.size(); i++)
						{
							vec3 normal(0.f);
							for (auto fi : vertices_adjacent_faces[i])
								normal += pcontrol_mesh->faces[fi].normal;
							normal = normalize(normal);

							auto v = pcontrol_mesh->vertices[i];
							auto disp = triplanar_sample<float>(normal, v, voronoi_noise);
							v += normal * disp;
							mesh.vertices[i] = v;
						}

						for (auto& f : mesh.faces)
						{
							f.normal = normalize(cross(mesh.vertices[f.corners[1].vertex_id] - mesh.vertices[f.corners[0].vertex_id],
								mesh.vertices[f.corners[3].vertex_id] - mesh.vertices[f.corners[0].vertex_id]));
						}
					}
				},
				nullptr,
				nullptr,
				nullptr,
				nullptr
			);
		}
	}
}
