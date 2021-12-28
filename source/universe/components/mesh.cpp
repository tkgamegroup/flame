//#include "../../graphics/device.h"
//#include "../../graphics/model.h"
//#include "../entity_private.h"
//#include "../world_private.h"
//#include "node_private.h"
//#include "mesh_private.h"
//#include "armature_private.h"
//#include "../systems/node_renderer_private.h"
//
//namespace flame
//{
//	void cMeshPrivate::set_model_name(const std::filesystem::path& _model_name)
//	{
//		if (model_name == _model_name)
//			return;
//		model_name = _model_name;
//		apply_src();
//		if (node)
//			node->mark_transform_dirty();
//		data_changed(S<"model_name"_h>);
//	}
//
//	void cMeshPrivate::set_mesh_index(uint idx)
//	{
//		if (mesh_index == idx)
//			return;
//		mesh_index = idx;
//		apply_src();
//		if (node)
//			node->mark_transform_dirty();
//		data_changed(S<"mesh_index"_h>);
//	}
//
//	void cMeshPrivate::set_skin_index(uint idx)
//	{
//		if (skin_index == idx)
//			return;
//		skin_index = idx;
//		if (node)
//			node->mark_drawing_dirty();
//		data_changed(S<"skin_index"_h>);
//	}
//
//	void cMeshPrivate::set_cast_shadow(bool v)
//	{
//		if (cast_shadow == v)
//			return;
//		cast_shadow = v;
//		data_changed(S<"cast_shadow"_h>);
//	}
//
//	void cMeshPrivate::set_shading_flags(ShadingFlags flags)
//	{
//		if (shading_flags == flags)
//			return;
//		shading_flags = flags;
//		if (node)
//			node->mark_drawing_dirty();
//		data_changed(S<"shading_flags"_h>);
//	}
//
//	void cMeshPrivate::apply_src()
//	{
//		mesh_id = -1;
//		mesh = nullptr;
//		if (!s_renderer || model_name.empty())
//			return;
//
//		graphics::Model* model = nullptr;
//		auto fn = model_name;
//		if (fn.extension().empty())
//			model = graphics::Model::get(fn);
//		else
//		{
//			fn = Path::get(fn);
//			model = graphics::Model::get(fn);
//		}
//		if (!model)
//			return;
//
//		if (mesh_index >= model->meshes.size())
//			return;
//		mesh = &model->meshes[mesh_index];
//
//		mesh_id = s_renderer->find_mesh_res(mesh);
//		if (mesh_id == -1)
//		{
//			mesh_id = s_renderer->set_mesh_res(-1, mesh);
//			if (mesh_id == -1)
//			{
//				mesh = nullptr;
//				return;
//			}
//		}
//
//		parmature = entity->parent->get_component_t<cArmaturePrivate>();
//	}
//
//	void cMeshPrivate::on_added()
//	{
//		node = entity->get_component_i<cNodePrivate>(0);
//		if (!node)
//			return;
//
//		drawer_lis = node->drawers.add([this](sNodeRendererPtr s_renderer, bool shadow_pass) {
//			if (mesh_id != -1)
//			{
//				auto get_idx = [&]() {
//					if (parmature)
//						return parmature->armature_id;
//					if (frame < frames)
//					{
//						transform_id = s_renderer->add_mesh_transform(node->transform, node->g_rot);
//						frame = frames;
//					}
//					return transform_id;
//				};
//				auto idx = get_idx();
//				if (shadow_pass)
//				{
//					if (cast_shadow)
//						s_renderer->draw_mesh(idx, mesh_id, skin_index, ShadingShadow);
//				}
//				else
//					s_renderer->draw_mesh(idx, mesh_id, skin_index, shading_flags);
//			}
//		});
//		measurer_lis = node->measurers.add([this](AABB* ret) {
//			if (!mesh)
//				return false;
//			auto b = mesh->bounds;
//			vec3 ps[8];
//			b.get_points(ps);
//			b.reset();
//			auto& mat = parmature ? parmature->node->transform : node->transform;
//			for (auto i = 0; i < 8; i++)
//				b.expand(mat * vec4(ps[i], 1.f));
//			*ret = b;
//			return true;
//		});
//
//		node->mark_drawing_dirty();
//	}
//
//	void cMeshPrivate::on_removed()
//	{
//		if (node)
//		{
//			node->drawers.remove(drawer_lis);
//			node->measurers.remove(measurer_lis);
//			node = nullptr;
//		}
//	}
//
//	void cMeshPrivate::on_entered_world()
//	{
//		s_renderer = entity->world->get_system_t<sNodeRendererPrivate>();
//
//		apply_src();
//		if (node)
//			node->mark_bounds_dirty(false);
//	}
//
//	void cMeshPrivate::on_left_world()
//	{
//		s_renderer = nullptr;
//		mesh_id = -1;
//		mesh = nullptr;
//	}
//
//	struct cMeshCreatePrivate : cMesh::Create
//	{
//		cMeshPtr operator()() override
//		{
//			return new cMeshPrivate();
//		}
//	}cMesh_create_private;
//	cMesh::Create& cMesh::create = cMesh_create_private;
//}
