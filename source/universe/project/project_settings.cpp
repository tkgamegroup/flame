#include "../../xml.h"
#include "../../foundation/typeinfo_serialize.h"
#include "project_settings.h"

namespace flame
{
	void ProjectSettings::load(const std::filesystem::path& _filename)
	{
		clear();
		filename = _filename;

		if (!std::filesystem::exists(filename))
			save();
		else
		{
			pugi::xml_document doc;
			pugi::xml_node doc_root;
			if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("project_settings"))
			{
				wprintf(L"project settings is wrong format: %s\n", _filename.c_str());
				return;
			}

			unserialize_xml(doc_root, this);
		}
	}

	void ProjectSettings::save()
	{
		pugi::xml_document doc;
		serialize_xml(this, doc.append_child("project_settings"));
		doc.save_file(filename.c_str());
	}
}
