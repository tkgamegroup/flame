#include <flame/serialize.h>
#include "typeinfo_private.h"

#include <Windows.h>

namespace flame
{
	std::map<uint, std::unique_ptr<TypeInfoPrivate>> typeinfos;
	std::vector<TypeInfoDatabasePrivate*> global_typeinfo_databases;

	TypeInfoPrivate::TypeInfoPrivate(TypeTag tag, const std::string& base_name, bool is_array) :
		tag(tag),
		base_name(base_name),
		is_array(is_array)
	{
		base_hash = FLAME_HASH(base_name.c_str());
		name = make_str(tag, base_name, is_array);
		hash = FLAME_HASH(name.c_str());
	}

	TypeInfoPrivate* TypeInfoPrivate::_get(TypeTag tag, const std::string& base_name, bool is_array)
	{
		auto hash = FLAME_HASH(make_str(tag, base_name, is_array).c_str());
		auto it = typeinfos.find(hash);
		if (it != typeinfos.end())
			return it->second.get();
		auto t = new TypeInfoPrivate(tag, base_name, is_array);
		typeinfos.emplace(t->hash, t);
		return t;
	}

	TypeInfo* TypeInfo::get(TypeTag tag, const char* base_name, bool is_array) { return TypeInfoPrivate::_get(tag, base_name, is_array); }

	VariableInfoPrivate::VariableInfoPrivate(UdtInfoPrivate* udt, uint index, TypeInfoPrivate* type, const std::string& name, uint flags, uint offset, uint size) :
		udt(udt),
		index(index),
		type(type),
		name(name),
		flags(flags),
		offset(offset),
		size(size),
		default_value(nullptr)
	{
		name_hash = FLAME_HASH(name.c_str());
		if (type->is_pod())
		{
			default_value = new char[size];
			memset(default_value, 0, size);
		}
	}

	VariableInfoPrivate::~VariableInfoPrivate()
	{
		delete[]default_value;
	}

	EnumItemPrivate::EnumItemPrivate(EnumInfoPrivate* ei, uint index, const std::string& name, int value) :
		ei(ei),
		index(index),
		name(name),
		value(value)
	{
	}

	EnumInfoPrivate::EnumInfoPrivate(TypeInfoDatabasePrivate* db, const std::string& name) :
		db(db),
		name(name)
	{
	}

	EnumItemPrivate* EnumInfoPrivate::_find_item(const std::string& name) const
	{
		for (auto& i : items)
		{
			if (i->name == name)
				return i.get();
		}
		return nullptr;
	}
	EnumItemPrivate* EnumInfoPrivate::_find_item(int value) const
	{
		for (auto& i : items)
		{
			if (i->value == value)
				return i.get();
		}
		return nullptr;
	}

	FunctionInfoPrivate::FunctionInfoPrivate(TypeInfoDatabasePrivate* db, UdtInfoPrivate* udt, uint index, const std::string& name, void* rva, TypeInfoPrivate* type) :
		db(db),
		udt(udt),
		index(index),
		name(name),
		rva(rva),
		type(type)
	{
	}

	UdtInfoPrivate::UdtInfoPrivate(TypeInfoDatabasePrivate* db, const std::string& name, uint size, const std::string& base_name) :
		db(db),
		name(name),
		size(size),
		base_name(base_name)
	{
	}

	VariableInfoPrivate* UdtInfoPrivate::_find_variable(const std::string& name) const
	{
		for (auto& v : variables)
		{
			if (v->name == name)
				return v.get();
		}
		return nullptr;
	}
	FunctionInfoPrivate* UdtInfoPrivate::_find_function(const std::string& name) const
	{
		for (auto& f : functions)
		{
			if (f->name == name)
				return f.get();
		}
		return nullptr;
	}

	TypeInfoDatabasePrivate::TypeInfoDatabasePrivate(const std::wstring& library_name) :
		library(nullptr),
		library_name(library_name)
	{
	}

	TypeInfoDatabasePrivate::~TypeInfoDatabasePrivate()
	{
		if (library)
			free_library(library);
	}

	EnumInfo* TypeInfoDatabasePrivate::get_enum(uint hash) const 
	{
		auto it = enums.find(hash);
		if (it != enums.end())
			return it->second.get();
		return nullptr;
	}
	UdtInfo* TypeInfoDatabasePrivate::get_udt(uint hash) const
	{
		auto it = udts.find(hash);
		if (it != udts.end())
			return it->second.get();
		return nullptr;
	}

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

		auto db = new TypeInfoDatabasePrivate(library_path);
		global_typeinfo_databases.push_back(db);

		for (auto n_enum : file_root.child("enums"))
		{
			auto e = new EnumInfoPrivate(db, n_enum.attribute("name").value());
			db->enums.emplace(FLAME_HASH(e->name.c_str()), e);

			for (auto n_item : n_enum.child("items"))
				e->items.emplace_back(new EnumItemPrivate(e, e->items.size(), n_item.attribute("name").value(), n_item.attribute("value").as_int()));
		}

		for (auto n_udt : file_root.child("udts"))
		{
			auto u = new UdtInfoPrivate(db, n_udt.attribute("name").value(), n_udt.attribute("size").as_uint(), n_udt.attribute("base_name").value());
			db->udts.emplace(FLAME_HASH(u->name.c_str()), u);

			for (auto n_variable : n_udt.child("variables"))
			{
				auto type = TypeInfoPrivate::_get(n_variable.attribute("type").value());
				auto v = new VariableInfoPrivate(u, u->variables.size(), type, n_variable.attribute("name").value(),
					n_variable.attribute("flags").as_uint(), n_variable.attribute("offset").as_uint(), n_variable.attribute("size").as_uint());
				u->variables.emplace_back(v);

				if (v->default_value)
					type->unserialize(n_variable.attribute("default_value").value(), v->default_value);
			}

			for (auto n_function : n_udt.child("functions"))
			{
				auto f = new FunctionInfoPrivate(db, u, u->functions.size(), n_function.attribute("name").value(), (void*)n_function.attribute("rva").as_uint(), TypeInfoPrivate::_get(n_function.attribute("return_type").value()));
				u->functions.emplace_back(f);
				for (auto n_parameter : n_function.child("parameters"))
					f->parameters.push_back(TypeInfoPrivate::_get(n_parameter.attribute("type").value()));
			}
		}

		if (load_with_library)
			db->library = load_library(db->library_name.c_str());
		if (!add_to_global)
			global_typeinfo_databases.erase(global_typeinfo_databases.begin() + global_typeinfo_databases.size() - 1);

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

			for (auto& u : db->udts)
			{
				for (auto& f : u.second->functions)
				{
					if (f->name == "bp_update")
					{
						auto n = u.second->name + "::" + f->name;
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

	void push_global_typeinfo_database(TypeInfoDatabase* db)
	{
		global_typeinfo_databases.push_back((TypeInfoDatabasePrivate*)db);
	}

	void pop_global_typeinfo_database()
	{
		global_typeinfo_databases.erase(global_typeinfo_databases.begin() + global_typeinfo_databases.size() - 1);
	}

	EnumInfo* find_enum(uint hash)
	{
		for (auto db : global_typeinfo_databases)
		{
			auto it = db->enums.find(hash);
			if (it != db->enums.end())
				return it->second.get();
		}
		return nullptr;
	}
	UdtInfo* find_udt(uint hash)
	{
		for (auto db : global_typeinfo_databases)
		{
			auto it = db->udts.find(hash);
			if (it != db->udts.end())
				return it->second.get();
		}
		return nullptr;
	}
}
