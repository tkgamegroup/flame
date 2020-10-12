#include <flame/graphics/model.h>
#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "node_private.h"
#include "mesh_private.h"

namespace flame
{
	cMeshPrivate::~cMeshPrivate()
	{
		destroy_deformer();
	}

	void cMeshPrivate::set_src(const std::string& _src)
	{
		if (src == _src)
			return;
		src = _src;
		apply_src();
		if (node)
			node->mark_transform_dirty();
	}

	void cMeshPrivate::set_cast_shadow(bool v)
	{
		cast_shadow = v;
	}

	void cMeshPrivate::destroy_deformer()
	{
		if (deformer)
		{
			deformer->release();
			deformer = nullptr;
			for (auto& b : bones)
				b.first->entity->remove_local_data_changed_listener(b.second);
			bones.clear();
		}
	}

	void cMeshPrivate::apply_src()
	{
		model_id = -1;
		mesh_id = -1;
		mesh = nullptr;
		if (canvas && !src.empty())
		{
			auto sp = SUS::split(src, '.');
			if (sp.size() == 2)
			{
				model_id = canvas->find_resource(graphics::ResourceModel, sp[0].c_str());
				auto model = (graphics::Model*)canvas->get_resource(graphics::ResourceModel, model_id);
				mesh_id = model->find_mesh(sp[1].c_str());
				mesh = model->get_mesh(mesh_id);
				auto bones_count = mesh->get_bones_count();
				if (bones_count > 0)
				{
					deformer = canvas->create_armature_deformer(mesh);
					bones.resize(bones_count);
					auto armature = entity->parent;
					if (armature)
					{
						for (auto i = 0; i < bones_count; i++)
						{
							auto& b = bones[i];
							auto e = armature->find_child(mesh->get_bone(i)->get_name());
							if (e)
							{
								auto n = e->get_component_t<cNodePrivate>();
								if (n)
								{
									b.first = n;
									b.second = e->add_local_data_changed_listener([](Capture& c, Component* t, uint64 h) {
										auto thiz = c.thiz<cMeshPrivate>();
										auto id = c.data<int>();
										auto& b = thiz->bones[id];
										if (t == b.first && h == S<ch("transform")>::v)
										{
											b.first->update_transform();
											thiz->deformer->set_pose(id, b.first->transform);
										}
									}, Capture().set_thiz(this).set_data(&i));
								}
							}
						}
					}
				}
			}
		}
	}

	void cMeshPrivate::on_gain_canvas()
	{
		apply_src();
	}

	void cMeshPrivate::draw(graphics::Canvas* canvas)
	{
		if (model_id != -1 && mesh_id != -1)
			canvas->draw_mesh(model_id, mesh_id, node->transform, Mat4f(node->axes), cast_shadow, deformer);
	}

	cMesh* cMesh::create()
	{
		return f_new<cMeshPrivate>();
	}
}
