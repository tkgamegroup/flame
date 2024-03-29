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
			library->add_template("Control Mesh: Cube", "", BlueprintNodeFlagNone,
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
				[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
					auto extent = *(vec3*)inputs[0].data;
					auto& out_control_mesh = *(ControlMesh*)outputs[0].data;
					out_control_mesh.init_as_cube(extent);
					out_control_mesh.color = *(cvec4*)inputs[1].data;
				}
			);

			library->add_template("Control Mesh: Cone", "", BlueprintNodeFlagNone,
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
				[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
					auto radius = *(float*)inputs[0].data;
					auto depth = *(float*)inputs[1].data;
					auto vertices = *(uint*)inputs[2].data;
					auto& out_control_mesh = *(ControlMesh*)outputs[0].data;
					out_control_mesh.init_as_cone(radius, depth, vertices);
					out_control_mesh.color = *(cvec4*)inputs[3].data;
				}
			);

			library->add_template("Convert To Mesh", "", BlueprintNodeFlagNone,
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
				[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
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
				[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintNodePreview* preview) {
					auto& mesh = *(Mesh*)outputs[0].data;
					preview->type = "mesh"_h;
					preview->data = &mesh;
				}
			);

			library->add_template("Cube Mesh", "", BlueprintNodeFlagNone,
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
				[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
					auto extent = *(vec3*)inputs[0].data;
					auto& mesh = *(Mesh*)outputs[0].data;
					mesh.reset();
					mesh_add_cube(mesh, extent, vec3(0.f), mat3(1.f));
					mesh.calc_bounds();
				},
				nullptr,
				nullptr,
				nullptr,
				[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintNodePreview* preview) {
					auto& mesh = *(Mesh*)outputs[0].data;
					preview->type = "mesh"_h;
					preview->data = &mesh;
				}
			);

			library->add_template("Offset Control Mesh", "", BlueprintNodeFlagNone,
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
				[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
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

			library->add_template("Scale Control Mesh", "", BlueprintNodeFlagNone,
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh*>() }
					},
					{
						.name = "Scale",
						.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<vec3>() }
					}
				},
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh>() }
					}
				},
				[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
					auto pcontrol_mesh = *(ControlMesh**)inputs[0].data;
					auto scale = inputs[1].type == TypeInfo::get<float>() ? vec3(*(float*)inputs[1].data) : *(vec3*)inputs[1].data;
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

			library->add_template("Rotate Control Mesh", "", BlueprintNodeFlagNone,
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
				[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
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
						for (auto& f : out_control_mesh.faces)
							f.normal = rotation_matrix * vec4(f.normal, 0.f);
						out_control_mesh.color = pcontrol_mesh->color;
					}
				}
			);

			library->add_template("Subdivide Control Mesh", "", BlueprintNodeFlagNone,
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
				[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
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
								current_level.subdivide_CatmullClark(next_level);
								current_level = next_level;
							}
							out_control_mesh = std::move(current_level);
							out_control_mesh.color = pcontrol_mesh->color;
						}
					}
				}
			);

			library->add_template("Loop Cut Control Mesh XZ-Plane", "", BlueprintNodeFlagNone,
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh*>() }
					},
					{
						.name = "Offset",
						.allowed_types = { TypeInfo::get<float>() }
					}
				},
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh>() }
					}
				},
				[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
					auto pcontrol_mesh = *(ControlMesh**)inputs[0].data;
					auto offset = *(float*)inputs[1].data;
					auto& out_control_mesh = *(ControlMesh*)outputs[0].data;
					if (pcontrol_mesh)
						pcontrol_mesh->loop_cut(out_control_mesh, { Plane(vec3(0.f, 1.f, 0.f), vec3(0.f, offset, 0.f)) });
				}
			);

			library->add_template("Loop Cut Control Mesh XZ-Planes", "", BlueprintNodeFlagNone,
				{
					{
						.name = "Mesh",
						.allowed_types = { TypeInfo::get<ControlMesh*>() }
					},
					{
						.name = "Start Offset",
						.allowed_types = { TypeInfo::get<float>() }
					},
					{
						.name = "Offset",
						.allowed_types = { TypeInfo::get<float>() }
					},
					{
						.name = "Number",
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
				[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
					auto pcontrol_mesh = *(ControlMesh**)inputs[0].data;
					auto start_offset = *(float*)inputs[1].data;
					auto offset = *(float*)inputs[2].data;
					auto number = *(uint*)inputs[3].data;
					auto& out_control_mesh = *(ControlMesh*)outputs[0].data;
					if (pcontrol_mesh)
					{
						if (number <= 1)
						{
							out_control_mesh.vertices = pcontrol_mesh->vertices;
							out_control_mesh.faces = pcontrol_mesh->faces;
						}
						else
						{
							std::vector<Plane> planes(number);
							for (auto i = 0; i < number; i++)
								planes[i] = Plane(vec3(0.f, 1.f, 0.f), vec3(0.f, start_offset + i * offset, 0.f));
							std::reverse(planes.begin(), planes.end());
							pcontrol_mesh->loop_cut(out_control_mesh, planes);
						}
					}
				}
			);

			library->add_template("Displace Control Mesh", "", BlueprintNodeFlagNone,
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
				[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
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
