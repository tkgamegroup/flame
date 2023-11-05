#include "../../../foundation/blueprint.h"
#include "../../../graphics/model_ext.h"
#include "../../entity_private.h"
#include "../../components/node_private.h"
#include "../../components/mesh_private.h"
#include "../../components/procedure_mesh_private.h"
#include "../../systems/renderer_private.h"

namespace flame
{
	void add_procedural_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Instance Control Mesh", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Mesh",
					.allowed_types = { TypeInfo::get<graphics::ControlMesh*>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto pcontrol_mesh = *(graphics::ControlMesh**)inputs[1].data;
					if (pcontrol_mesh)
					{
						auto node = entity->get_component<cNode>();
						if (!node)
							node = entity->add_component<cNode>();
						auto mesh = entity->get_component<cMesh>();
						if (!mesh)
							mesh = entity->add_component<cMesh>();
						auto procedure_mesh = entity->get_component<cProcedureMesh>();
						if (!procedure_mesh)
							procedure_mesh = entity->add_component<cProcedureMesh>();

						pcontrol_mesh->convert_to_mesh(procedure_mesh->converted_mesh);
						mesh->mesh = &procedure_mesh->converted_mesh;
						mesh->mesh_res_id = sRenderer::instance()->get_mesh_res(&procedure_mesh->converted_mesh, -1);
						mesh->node->mark_transform_dirty();

						mesh->set_material_name(L"default");
					}
				}
			}
		);
	}
}
