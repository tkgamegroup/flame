#include "../../xml.h"
#include "../../foundation/typeinfo_serialize.h"
#include "../world_private.h"
#include "world_settings_private.h"

namespace flame
{
	cWorldSettingsPrivate::~cWorldSettingsPrivate()
	{
		auto world = World::instance();
		for (auto& s : world->systems)
		{
			auto ui = find_udt(s->type_hash);
			for (auto& a : ui->attributes)
			{
				if (a.type->tag == TagD)
					a.unserialize(s.get(), a.default_value);
			}
		}
	}

	void cWorldSettingsPrivate::set_filename(const std::filesystem::path& path)
	{
		if (filename == path)
			return;
		filename = path;
	}

	void cWorldSettingsPrivate::save()
	{
		if (filename.empty())
			return;

		pugi::xml_document doc;
		auto doc_root = doc.append_child("world");
		for (auto& s : World::instance()->systems)
		{
			auto ui = find_udt(s->type_hash);
			auto n = doc_root.append_child("system");
			n.append_attribute("name").set_value(ui->name.c_str());
			serialize_xml(*ui, s.get(), n);
		}

		doc.save_file(Path::get(filename).c_str());
	}

	void cWorldSettingsPrivate::on_active()
	{
		if (filename.empty())
			return;

		pugi::xml_document doc;
		pugi::xml_node doc_root;
		auto fn = Path::get(filename);
		if (!std::filesystem::exists(fn))
		{
			wprintf(L"file does not exist: %s\n", filename.c_str());
			return;
		}
		if (!doc.load_file(fn.c_str()) || (doc_root = doc.first_child()).name() != std::string("world"))
		{
			wprintf(L"file is wrong format: %s\n", filename.c_str());
			return;
		}

		auto world = World::instance();
		for (auto n : doc_root.children())
		{
			auto hash = sh(n.attribute("name").value());
			if (auto system = world->get_system(hash); system)
			{
				auto ui = find_udt(hash);
				unserialize_xml(*ui, n, system);
			}
		}
	}

	struct cWorldSettingsCreate : cWorldSettings::Create
	{
		cWorldSettingsPtr operator()(EntityPtr e) override
		{
			return new cWorldSettingsPrivate();
		}
	}cWorldSettings_create;
	cWorldSettings::Create& cWorldSettings::create = cWorldSettings_create;
}
