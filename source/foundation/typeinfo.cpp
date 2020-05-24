#include <flame/serialize.h>
#include <flame/foundation/typeinfo.h>

#include <Windows.h>

namespace flame
{
	HashMap<256, TypeInfo> typeinfos;

	TypeInfoDatabase* TypeInfoDatabase::load(const wchar_t* library_filename, bool add_to_global, bool load_with_library)
	{
		std::filesystem::path library_path(library_filename);
		if (!library_path.is_absolute())
			library_path = get_app_path().str() / library_path;
		auto typeinfo_path = library_path;
		typeinfo_path.replace_extension(L".typeinfo");
		if (!std::filesystem::exists(typeinfo_path) || std::filesystem::last_write_time(typeinfo_path) < std::filesystem::last_write_time(library_path))
		{
			auto typeinfogen_path = std::filesystem::path(get_app_path().str()) / L"typeinfogen.exe";
			if (!std::filesystem::exists(typeinfogen_path))
			{
				printf("typeinfo out of date: %s, and cannot find typeinfogen\n", typeinfo_path.string().c_str());
				assert(0);
				return nullptr;
			}
			exec_and_redirect_to_std_output(nullptr, (wchar_t*)(typeinfogen_path.wstring() + L" " + library_path.wstring()).c_str());
		}

		pugi::xml_document file;
		pugi::xml_node file_root;
		if (!file.load_file(typeinfo_path.c_str()) || (file_root = file.first_child()).name() != std::string("typeinfo"))
		{
			printf("cannot find typeinfo: %s\n", typeinfo_path.string().c_str());
			assert(0);
			return nullptr;
		}

		auto db = new TypeInfoDatabase(library_path);
		global_typeinfo_databases.push_back(db);

		for (auto n_enum : file_root.child("enums"))
		{
			auto e = db->add_enum(n_enum.attribute("name").value());

			for (auto n_item : n_enum.child("items"))
				e->add_item(n_item.attribute("name").value(), n_item.attribute("value").as_int());
		}

		for (auto n_udt : file_root.child("udts"))
		{
			auto u = db->add_udt(n_udt.attribute("name").value(), n_udt.attribute("size").as_uint(), n_udt.attribute("base_name").value());

			for (auto n_variable : n_udt.child("variables"))
			{
				auto type = TypeInfo::get(n_variable.attribute("type").value());
				auto v = u->add_variable(type, n_variable.attribute("name").value(),
					n_variable.attribute("flags").as_uint(), n_variable.attribute("offset").as_uint(), n_variable.attribute("size").as_uint());

				if (v->default_value)
					type->unserialize(n_variable.attribute("default_value").value(), v->default_value);
			}

			for (auto n_function : n_udt.child("functions"))
			{
				auto f = u->add_function(n_function.attribute("name").value(), (void*)n_function.attribute("rva").as_uint(), TypeInfo::get(n_function.attribute("return_type").value()));
				for (auto n_parameter : n_function.child("parameters"))
					f->add_parameter(TypeInfo::get(n_parameter.attribute("type").value()));
			}
		}

		if (load_with_library)
			db->library = load_library(db->library_name.v);
		if (!add_to_global)
			global_typeinfo_databases.remove(global_typeinfo_databases.s - 1);

		auto typeinfo_code_path = library_path;
		typeinfo_code_path.replace_extension(L".typeinfo.code");
		std::ifstream typeinfo_code(typeinfo_code_path);
		if (typeinfo_code.good())
		{
			struct FunctionCode
			{
				std::string name;
				std::string code;
			};
			std::vector<FunctionCode> function_codes;

			while (!typeinfo_code.eof())
			{
				std::string line;
				std::getline(typeinfo_code, line);
				if (line.empty())
					continue;

				if (line.size() > 2 && line[0] == '#' && line[1] == '#')
				{
					FunctionCode fc;
					fc.name = line.substr(2);
					function_codes.push_back(fc);
				}
				else
					function_codes.back().code += line + "\n";
			}
			typeinfo_code.close();

			for (auto u : db->udts.get_all())
			{
				for (auto f : u->functions)
				{
					if (f->name == "bp_update")
					{
						auto n = u->name.str() + "::" + f->name.str();
						for (auto& fc : function_codes)
						{
							if (fc.name == n)
							{
								f->code = fc.code;
								break;
							}
						}
					}
				}
			}
		}

		return db;
	}

	void TypeInfoDatabase::destroy(TypeInfoDatabase* db)
	{
		delete db;
	}

	Array<TypeInfoDatabase*> global_typeinfo_databases;
}
