#include <flame/foundation/serialize.h>
#include "world_private.h"

namespace flame
{
	WorldPrivate::WorldPrivate()
	{
		universe_ = nullptr;

		auto e = new EntityPrivate;
		e->world_ = this;
		root.reset(e);
	}

	System*  WorldPrivate::get_system_plain(uint name_hash) const
	{
		for (auto& s : systems)
		{
			if (s->name_hash == name_hash)
				return s.get();
		}
		return nullptr;
	}

	const std::wstring& World::filename() const
	{
		return ((WorldPrivate*)this)->filename;
	}

	void World::add_object(Object* o, const std::string& id)
	{
		((WorldPrivate*)this)->objects.emplace_back(o, id);
	}

	Object* World::find_object(uint name_hash, const std::string& id)
	{
		const auto& objects = ((WorldPrivate*)this)->objects;
		for (auto& o : objects)
		{
			if (o.first->name_hash == name_hash)
			{
				if (!id.empty() && o.second == id)
					return o.first;
			}
		}
		return universe_ ? universe_->find_object(name_hash, id) : nullptr;
	}

	const std::string* World::find_id(Object* _o)
	{
		const auto& objects = ((WorldPrivate*)this)->objects;
		for (auto& o : objects)
		{
			if (o.first == _o)
				return &o.second;
		}
		return universe_ ? universe_->find_id(_o) : nullptr;
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

	World* World::create()
	{
		return new WorldPrivate;
	}

	World* World::create_from_file(const std::vector<TypeinfoDatabase*>& dbs, const std::wstring& filename)
	{
		auto file = SerializableNode::create_from_xml_file(filename);
		if (!file || file->name() != "world")
			return nullptr;

		auto w = new WorldPrivate;

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
					auto type = v->type();
					unserialize_value(dbs, type->tag(), type->hash(), n->find_attr("v")->value(), (char*)dummy + v->offset());
				}
				void* object;
				{
					auto f = udt->find_function("create");
					assert(f && f->return_type()->equal(TypeTagPointer, cH("Object")) && f->parameter_count() == 1 && f->parameter_type(0)->equal(TypeTagPointer, cH("World")));
					object = cmf(p2f<MF_vp_vp>((char*)module + (uint)f->rva()), dummy, w);
				}
				w->add_object((Object*)object, n_o->find_attr("id")->value());
				{
					auto f = udt->find_function("dtor");
					if (f)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
				}
				free_module(module);
				free(dummy);
			}
		}

		return w;
	}

	void World::destroy(World* w)
	{
		delete (WorldPrivate*)w;
	}
}
