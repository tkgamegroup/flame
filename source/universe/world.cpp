#include <flame/foundation/typeinfo.h>
#include "world_private.h"

namespace flame
{
	WorldPrivate::WorldPrivate()
	{
		root.reset(new EntityPrivate);
		root->world = this;
		root->global_visibility = true;
	}

	WorldPrivate::~WorldPrivate()
	{
		for (auto& o : objects)
		{
			auto udt = o.second;
			if (udt)
			{
				auto dummy = malloc(udt->size());
				auto module = udt->db()->module();
				{
					auto f = udt->find_function("ctor");
					if (f && f->parameter_count() == 0)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
				}
				{
					auto f = udt->find_function("destroy");
					assert(f && check_function(f, "D#void", { "P#Object" }));
					cmf(p2f<MF_v_vp>((char*)module + (uint)f->rva()), dummy, o.first);
				}
				{
					auto f = udt->find_function("dtor");
					if (f)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
				}
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
		return nullptr;
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

	void WorldPrivate::update()
	{
		for (auto& s : systems)
		{
			s->before_update_listeners.call();
			s->update();
			s->after_update_listeners.call();
		}
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

	void World::update()
	{
		((WorldPrivate*)this)->update();
	}

	World* World::create()
	{
		return new WorldPrivate();
	}

	void World::destroy(World* w)
	{
		delete (WorldPrivate*)w;
	}

	void load_res(World* w, const wchar_t* filename)
	{
		pugi::xml_document file;
		pugi::xml_node file_root;
		if (!file.load_file(filename) || (file_root = file.first_child()).name() != std::string("res"))
			return;

		auto last_curr_path = get_curr_path();
		set_curr_path(std::filesystem::path(filename).parent_path().c_str());

		for (auto n_o : file_root)
		{
			auto udt = find_udt(FLAME_HASH((std::string("D#flame::Serializer_") + n_o.name()).c_str()));
			assert(udt);
			auto dummy = malloc(udt->size());
			auto module = udt->db()->module();
			{
				auto f = udt->find_function("ctor");
				if (f && f->parameter_count() == 0)
					cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
			}
			for (auto n_v : n_o)
			{
				auto v = udt->find_variable(n_v.name());
				v->type()->unserialize(n_v.attribute("v").value(), (char*)dummy + v->offset());
			}
			void* object;
			{
				auto f = udt->find_function("create");
				assert(f && check_function(f, "P#Object", { "P#flame::World" }));
				object = cmf(p2f<MF_vp_vp>((char*)module + (uint)f->rva()), dummy, w);
			}
			((WorldPrivate*)w)->objects.emplace_back((Object*)object, udt);
			{
				auto f = udt->find_function("dtor");
				if (f)
					cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
			}
			free(dummy);
		}

		set_curr_path(last_curr_path.v);
	}
}
