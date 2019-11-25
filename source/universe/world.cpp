#include <flame/foundation/serialize.h>
#include "universe_private.h"
#include "world_private.h"

namespace flame
{
	WorldPrivate::WorldPrivate(Universe* u)
	{
		universe_ = u;
		((UniversePrivate*)u)->worlds.emplace_back(this);

		auto e = new EntityPrivate;
		e->world_ = this;
		root.reset(e);
	}

	WorldPrivate::~WorldPrivate()
	{
		for (auto& o : objects)
		{
			auto udt = o.second;
			if (udt)
			{
				auto dummy = malloc(udt->size());
				auto module = load_module(udt->db()->module_name());
				{
					auto f = udt->find_function("ctor");
					if (f && f->parameter_count() == 0)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
				}
				{
					auto f = udt->find_function("destroy");
					assert(f && f->return_type().equal(TypeTagVariable, cH("void")) && f->parameter_count() == 1 && f->parameter_type(0).equal(TypeTagPointer, cH("Object")));
					cmf(p2f<MF_v_vp>((char*)module + (uint)f->rva()), dummy, o.first);
				}
				{
					auto f = udt->find_function("dtor");
					if (f)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
				}
				free_module(module);
				free(dummy);
			}
		}
	}

	const std::wstring& World::filename() const
	{
		return ((WorldPrivate*)this)->filename;
	}

	void World::add_object(Object* o)
	{
		((WorldPrivate*)this)->objects.emplace_back(o, nullptr);
	}

	Object* World::find_object(uint name_hash, uint id)
	{
		const auto& objects = ((WorldPrivate*)this)->objects;
		for (auto& o : objects)
		{
			if (o.first->name_hash == name_hash)
			{
				if (!id || o.first->id == id)
					return o.first;
			}
		}
		return universe_->find_object(name_hash, id);
	}

	System* WorldPrivate::get_system_plain(uint name_hash) const
	{
		for (auto& s : systems)
		{
			if (s->name_hash == name_hash)
				return s.get();
		}
		return nullptr;
	}

	System* World::get_system_plain(uint name_hash) const
	{
		return ((WorldPrivate*)this)->get_system_plain(name_hash);
	}

	void World::add_system(System* s)
	{
		s->world_ = this;
		((WorldPrivate*)this)->systems.emplace_back(s);
		s->on_added();
	}

	Entity* World::root() const
	{
		return ((WorldPrivate*)this)->root.get();
	}

	World* World::create(Universe* u)
	{
		return new WorldPrivate(u);
	}

	World* World::create_from_file(Universe* u, const std::vector<TypeinfoDatabase*>& dbs, const std::wstring& filename)
	{
		auto file = SerializableNode::create_from_xml_file(filename);
		if (!file || file->name() != "world")
			return nullptr;

		auto w = new WorldPrivate(u);
		w->filename = filename;

		auto last_curr_path = get_curr_path();
		set_curr_path(std::filesystem::path(filename).parent_path().wstring());

		auto n_os = file->find_node("objects");
		if (n_os)
		{
			for (auto i_o = 0; i_o < n_os->node_count(); i_o++)
			{
				auto n_o = n_os->node(i_o);

				auto udt = find_udt(dbs, H(("Serializer_" + n_o->name()).c_str()));
				assert(udt);
				auto dummy = malloc(udt->size());
				auto module = load_module(udt->db()->module_name());
				{
					auto f = udt->find_function("ctor");
					if (f && f->parameter_count() == 0)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
				}
				for (auto i = 0; i < n_o->node_count(); i++)
				{
					auto n = n_o->node(i);

					auto v = udt->find_variable(n->name());
					unserialize_value(dbs, v->type().tag, v->type().hash, n->find_attr("v")->value(), (char*)dummy + v->offset());
				}
				void* object;
				{
					auto f = udt->find_function("create");
					assert(f && f->return_type().equal(TypeTagPointer, cH("Object")) && f->parameter_count() == 1 && f->parameter_type(0).equal(TypeTagPointer, cH("World")));
					object = cmf(p2f<MF_vp_vp>((char*)module + (uint)f->rva()), dummy, w);
				}
				w->objects.emplace_back((Object*)object, udt);
				{
					auto f = udt->find_function("dtor");
					if (f)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
				}
				free_module(module);
				free(dummy);
			}
		}

		auto n_ss = file->find_node("systems");
		if (n_ss)
		{
			for (auto i_s = 0; i_s < n_ss->node_count(); i_s++)
			{
				auto n_s = n_ss->node(i_s);

				auto udt = find_udt(dbs, H(("Serializer_" + n_s->name()).c_str()));
				assert(udt);
				auto dummy = malloc(udt->size());
				auto module = load_module(udt->db()->module_name());
				{
					auto f = udt->find_function("ctor");
					if (f && f->parameter_count() == 0)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
				}
				for (auto i = 0; i < n_s->node_count(); i++)
				{
					auto n = n_s->node(i);

					auto v = udt->find_variable(n->name());
					unserialize_value(dbs, v->type().tag, v->type().hash, n->find_attr("v")->value(), (char*)dummy + v->offset());
				}
				void* system;
				{
					auto f = udt->find_function("create");
					assert(f && f->return_type().equal(TypeTagPointer, cH("System")) && f->parameter_count() == 1 && f->parameter_type(0).equal(TypeTagPointer, cH("World")));
					system = cmf(p2f<MF_vp_vp>((char*)module + (uint)f->rva()), dummy, w);
				}
				w->add_system((System*)system);
				{
					auto f = udt->find_function("dtor");
					if (f)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
				}
				free_module(module);
				free(dummy);
			}
		}

		set_curr_path(*last_curr_path.p);
		delete_mail(last_curr_path);

		return w;
	}

	void World::destroy(World* w)
	{
		delete (WorldPrivate*)w;
	}
}
