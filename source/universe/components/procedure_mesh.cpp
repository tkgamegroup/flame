#include "../../foundation/blueprint.h"
#include "../../graphics/model_ext.h"
#include "node_private.h"
#include "mesh_private.h"
#include "procedure_mesh_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cProcedureMeshPrivate::~cProcedureMeshPrivate()
	{
		if (cmesh->mesh_res_id != -1)
		{
			sRenderer::instance()->release_mesh_res(cmesh->mesh_res_id);
			cmesh->mesh = nullptr;
			cmesh->mesh_res_id = -1;
		}

		if (converted_mesh)
			delete converted_mesh;
	}

	void cProcedureMeshPrivate::set_blueprint_name(const std::filesystem::path& name)
	{
		if (blueprint_name == name)
			return;
		blueprint_name = name;

		if (!blueprint_name.empty())
		{
			if (auto bp = Blueprint::get(blueprint_name); bp)
			{
				auto ins = BlueprintInstance::create(bp);
				std::vector<voidptr> outputs;

				graphics::ControlMesh* pcontrol_mesh = nullptr;
				outputs.push_back(&pcontrol_mesh);
				if (auto g = ins->find_group("main"_h); g)
					ins->call(g, nullptr, outputs.data());

				if (pcontrol_mesh)
				{
					if (!converted_mesh)
						converted_mesh = graphics::Mesh::create();
					pcontrol_mesh->convert_to_mesh(*converted_mesh);
					assert(cmesh->mesh_res_id == -1);
					cmesh->mesh = converted_mesh;
					cmesh->mesh_res_id = sRenderer::instance()->get_mesh_res(converted_mesh, -1);
					cmesh->color = pcontrol_mesh->color;
					cmesh->node->mark_transform_dirty();

					cmesh->set_material_name(L"default");
				}

				delete ins;
				Blueprint::release(bp);
			}
		}
	}

	struct cProcedureMeshCreate : cProcedureMesh::Create
	{
		cProcedureMeshPtr operator()(EntityPtr e) override
		{
			return new cProcedureMeshPrivate();
		}
	}cProcedureMesh_create;
	cProcedureMesh::Create& cProcedureMesh::create = cProcedureMesh_create;
}
