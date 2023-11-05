#include "../../../foundation/blueprint.h"
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
		void add_mesh_node_templates(BlueprintNodeLibraryPtr library)
		{
			library->add_template("Control Mesh: Cube", "",
				{
					{
						.name = "Extent",
						.allowed_types = { TypeInfo::get<vec3>() },
						.default_value = "1,1,1"
					},
					{
						.name = "Color",
						.allowed_types = { TypeInfo::get<cvec4>() },
						.default_value = "255,255,255,255"
					}
				},
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh>() }
					}
				},
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
					auto extent = *(vec3*)inputs[0].data;
					auto& out_control_mesh = *(ControlMesh*)outputs[0].data;
					out_control_mesh.init_as_cube(extent);
					out_control_mesh.color = *(cvec4*)inputs[1].data;
				}
			);

			library->add_template("Control Mesh: Cone", "",
				{
					{
						.name = "Radius",
						.allowed_types = { TypeInfo::get<float>() },
						.default_value = "1"
					},
					{
						.name = "Depth",
						.allowed_types = { TypeInfo::get<float>() },
						.default_value = "2"
					},
					{
						.name = "Vertices",
						.allowed_types = { TypeInfo::get<uint>() },
						.default_value = "4"
					},
					{
						.name = "Color",
						.allowed_types = { TypeInfo::get<cvec4>() },
						.default_value = "255,255,255,255"
					}
				},
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh>() }
					}
				},
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
					auto radius = *(float*)inputs[0].data;
					auto depth = *(float*)inputs[1].data;
					auto vertices = *(uint*)inputs[2].data;
					auto& out_control_mesh = *(ControlMesh*)outputs[0].data;
					out_control_mesh.init_as_cone(radius, depth, vertices);
					out_control_mesh.color = *(cvec4*)inputs[3].data;
				}
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
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
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
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintNodePreview* preview) {
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
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
					auto extent = *(vec3*)inputs[0].data;
					auto& mesh = *(Mesh*)outputs[0].data;
					mesh.reset();
					mesh_add_cube(mesh, extent, vec3(0.f), mat3(1.f));
					mesh.calc_bounds();
				},
				nullptr,
				nullptr,
				nullptr,
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintNodePreview* preview) {
					auto& mesh = *(Mesh*)outputs[0].data;
					preview->type = "mesh"_h;
					preview->data = &mesh;
				}
			);

			library->add_template("Offset Control Mesh", "",
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh*>() }
					},
					{
						.name = "Offset",
						.allowed_types = { TypeInfo::get<vec3>() }
					}
				},
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh>() }
					}
				},
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
					auto pcontrol_mesh = *(ControlMesh**)inputs[0].data;
					auto offset = *(vec3*)inputs[1].data;
					auto& out_control_mesh = *(ControlMesh*)outputs[0].data;
					if (pcontrol_mesh)
					{
						out_control_mesh.vertices = pcontrol_mesh->vertices;
						out_control_mesh.faces = pcontrol_mesh->faces;
						for (auto& v : out_control_mesh.vertices)
							v += offset;
						out_control_mesh.color = pcontrol_mesh->color;
					}
				}
			);

			library->add_template("Scale Control Mesh", "",
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh*>() }
					},
					{
						.name = "Scale",
						.allowed_types = { TypeInfo::get<vec3>() }
					}
				},
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh>() }
					}
				},
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
					auto pcontrol_mesh = *(ControlMesh**)inputs[0].data;
					auto scale = *(vec3*)inputs[1].data;
					auto& out_control_mesh = *(ControlMesh*)outputs[0].data;
					if (pcontrol_mesh)
					{
						out_control_mesh.vertices = pcontrol_mesh->vertices;
						out_control_mesh.faces = pcontrol_mesh->faces;
						for (auto& v : out_control_mesh.vertices)
							v *= scale;
						out_control_mesh.color = pcontrol_mesh->color;
					}
				}
			);

			library->add_template("Rotate Control Mesh", "",
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh*>() }
					},
					{
						.name = "Rotation",
						.allowed_types = { TypeInfo::get<vec3>() }
					}
				},
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh>() }
					}
				},
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
					auto pcontrol_mesh = *(ControlMesh**)inputs[0].data;
					auto rotation = *(vec3*)inputs[1].data;
					auto& out_control_mesh = *(ControlMesh*)outputs[0].data;
					if (pcontrol_mesh)
					{
						out_control_mesh.vertices = pcontrol_mesh->vertices;
						out_control_mesh.faces = pcontrol_mesh->faces;
						auto rotation_matrix = eulerAngleYXZ(rotation.x, rotation.y, rotation.z);
						for (auto& v : out_control_mesh.vertices)
							v = rotation_matrix * vec4(v, 1.f);
						out_control_mesh.color = pcontrol_mesh->color;
					}
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
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
					auto pcontrol_mesh = *(ControlMesh**)inputs[0].data;
					auto levels = *(uint*)inputs[1].data;
					auto& out_control_mesh = *(ControlMesh*)outputs[0].data;
					if (pcontrol_mesh)
					{
						if (levels == 0)
						{
							out_control_mesh.vertices = pcontrol_mesh->vertices;
							out_control_mesh.faces = pcontrol_mesh->faces;
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
							out_control_mesh = std::move(current_level);
							out_control_mesh.color = pcontrol_mesh->color;
						}
					}
				}
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
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
					auto pcontrol_mesh = *(ControlMesh**)inputs[0].data;
					auto ptexture = *(Texture**)inputs[1].data;
					auto strength = *(float*)inputs[2].data;
					auto midlevel = *(float*)inputs[3].data;
					auto& out_control_mesh = *(ControlMesh*)outputs[0].data;
					if (pcontrol_mesh)
					{
						pcontrol_mesh->displace(out_control_mesh, ptexture);
						out_control_mesh.color = pcontrol_mesh->color;
					}
				}
			);

		}
	}
}
