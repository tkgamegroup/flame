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
					assert(f && f->return_type() == TypeInfo(TypeData, "void") && f->parameter_count() == 1 && f->parameter_type(0) == TypeInfo(TypePointer, "Object"));
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

	void World::destroy(World* w)
	{
		delete (WorldPrivate*)w;
	}

	void load_res(World* w, const std::vector<TypeinfoDatabase*>& dbs, const std::wstring& filename)
	{
		auto file = SerializableNode::create_from_xml_file(filename);
		if (!file || file->name() != "res")
			return;

		auto last_curr_path = get_curr_path();
		set_curr_path(std::filesystem::path(filename).parent_path().wstring());

		auto this_module = load_module(L"flame_universe.dll");
		TypeinfoDatabase* this_db = nullptr;
		for (auto db : dbs)
		{
			if (std::filesystem::path(db->module_name()).filename() == L"flame_universe.dll")
			{
				this_db = db;
				break;
			}
		}
		assert(this_module && this_db);

		for (auto i_o = 0; i_o < file->node_count(); i_o++)
		{
			auto n_o = file->node(i_o);

			auto udt = find_udt(dbs, H(("D#Serializer_" + n_o->name()).c_str()));
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
				v->type().unserialize(dbs, n->find_attr("v")->value(), (char*)dummy + v->offset(), this_module, this_db);
			}
			void* object;
			{
				auto f = udt->find_function("create");
				assert(f && f->return_type() == TypeInfo(TypePointer, "Object") && f->parameter_count() == 1 && f->parameter_type(0) == TypeInfo(TypePointer, "World"));
				object = cmf(p2f<MF_vp_vp>((char*)module + (uint)f->rva()), dummy, w);
			}
			((WorldPrivate*)w)->objects.emplace_back((Object*)object, udt);
			{
				auto f = udt->find_function("dtor");
				if (f)
					cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
			}
			free_module(module);
			free(dummy);
		}

		free_module(this_module);

		set_curr_path(*last_curr_path.p);
		delete_mail(last_curr_path);
	}
}
