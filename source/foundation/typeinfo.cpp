#include <flame/serialize.h>
#include "typeinfo_private.h"

#include <Windows.h>

namespace flame
{
	std::map<uint, std::unique_ptr<TypeInfoPrivate>> typeinfos;
	std::vector<TypeInfoDatabasePrivate*> global_typeinfo_databases;

	TypeInfoPrivate::TypeInfoPrivate(TypeTag tag, const std::string& base_name, bool is_array) :
		_tag(tag),
		_base_name(base_name),
		_is_array(is_array)
	{
		_base_hash = FLAME_HASH(base_name.c_str());
		_name = get_name(tag, base_name, is_array);
		_hash = FLAME_HASH(_name.c_str());
	}

	std::string TypeInfoPrivate::get_name(TypeTag tag, const std::string& base_name, bool is_array)
	{
		std::string ret;
		ret = type_tag(tag);
		if (is_array)
			ret += "A";
		ret += "#" + base_name;
		return ret;
	}

	TypeInfoPrivate* TypeInfoPrivate::_get(TypeTag tag, const std::string& base_name, bool is_array)
	{
		auto hash = FLAME_HASH(get_name(tag, base_name, is_array).c_str());
		auto it = typeinfos.find(hash);
		if (it != typeinfos.end())
			return it->second.get();
		auto t = new TypeInfoPrivate(tag, base_name, is_array);
		typeinfos.emplace(hash, t);
		return t;
	}

	TypeInfoPrivate* TypeInfoPrivate::_get(const std::string& str)
	{
		TypeTag tag;
		auto pos_hash = str.find('#');
		{
			auto ch = str[0];
			for (auto i = 0; i < TypeTagCount; i++)
			{
				if (type_tag((TypeTag)i) == ch)
				{
					tag = (TypeTag)i;
					break;
				}
			}
		}
		auto is_array = false;
		if (pos_hash > 1 && str[1] == 'A')
			is_array = true;
		auto base_name = std::string(str.begin() + pos_hash + 1, str.end());
		return _get(tag, base_name, is_array);
	}

	VariableInfoPrivate::VariableInfoPrivate(UdtInfoPrivate* udt, uint index, TypeInfoPrivate* type, const std::string& name, uint flags, uint offset, uint size) :
		_udt(udt),
		_index(index),
		_type(type),
		_name(name),
		_flags(flags),
		_offset(offset),
		_size(size),
		_default_value(nullptr)
	{
		_name_hash = FLAME_HASH(name.c_str());
		if (!type->_is_array && type->_tag != TypePointer &&
			!(type->_tag == TypeData && (type->_base_hash == FLAME_CHASH("flame::StringA") || type->_base_hash == FLAME_CHASH("flame::StringW"))))
		{
			_default_value = new char[size];
			memset(_default_value, 0, size);
		}
	}

	VariableInfoPrivate::~VariableInfoPrivate()
	{
		delete[]_default_value;
	}

	EnumItemPrivate::EnumItemPrivate(EnumInfoPrivate* ei, uint index, const std::string& name, int value) :
		_ei(ei),
		_index(index),
		_name(name),
		_value(value)
	{
	}

	EnumInfoPrivate::EnumInfoPrivate(TypeInfoDatabasePrivate* db, const std::string& name) :
		_db(db),
		_name(name)
	{
	}

	EnumItemPrivate* EnumInfoPrivate::_find_item(const std::string& name) const
	{
		for (auto& i : _items)
		{
			if (i->_name == name)
				return i.get();
		}
		return nullptr;
	}
	EnumItemPrivate* EnumInfoPrivate::_find_item(int value) const
	{
		for (auto& i : _items)
		{
			if (i->_value == value)
				return i.get();
		}
		return nullptr;
	}

	FunctionInfoPrivate::FunctionInfoPrivate(TypeInfoDatabasePrivate* db, UdtInfoPrivate* udt, uint index, const std::string& name, void* rva, TypeInfoPrivate* type) :
		_db(db),
		_udt(udt),
		_index(index),
		_name(name),
		_rva(rva),
		_type(type)
	{
	}

	UdtInfoPrivate::UdtInfoPrivate(TypeInfoDatabasePrivate* db, const std::string& name, uint size, const std::string& base_name) :
		_db(db),
		_name(name),
		_size(size),
		_base_name(base_name)
	{
	}

	VariableInfoPrivate* UdtInfoPrivate::_find_variable(const std::string& name) const
	{
		for (auto& v : _variables)
		{
			if (v->_name == name)
				return v.get();
		}
		return nullptr;
	}
	FunctionInfoPrivate* UdtInfoPrivate::_find_function(const std::string& name) const
	{
		for (auto& f : _functions)
		{
			if (f->_name == name)
				return f.get();
		}
		return nullptr;
	}

	TypeInfoDatabasePrivate::TypeInfoDatabasePrivate(const std::wstring& library_name) :
		_library(nullptr),
		_library_name(library_name)
	{
	}

	TypeInfoDatabasePrivate::~TypeInfoDatabasePrivate()
	{
		if (_library)
			free_library(_library);
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
			db->_enums.emplace(FLAME_HASH(e->_name.c_str()), e);

			for (auto n_item : n_enum.child("items"))
				e->_items.emplace_back(new EnumItemPrivate(e, e->_items.size(), n_item.attribute("name").value(), n_item.attribute("value").as_int()));
		}

		for (auto n_udt : file_root.child("udts"))
		{
			auto u = new UdtInfoPrivate(db, n_udt.attribute("name").value(), n_udt.attribute("size").as_uint(), n_udt.attribute("base_name").value());
			db->_udts.emplace(FLAME_HASH(u->_name.c_str()), u);

			for (auto n_variable : n_udt.child("variables"))
			{
				auto type = TypeInfoPrivate::_get(n_variable.attribute("type").value());
				auto v = new VariableInfoPrivate(u, u->_variables.size(), type, n_variable.attribute("name").value(),
					n_variable.attribute("flags").as_uint(), n_variable.attribute("offset").as_uint(), n_variable.attribute("size").as_uint());
				u->_variables.emplace_back(v);

				if (v->_default_value)
					type->unserialize(n_variable.attribute("default_value").value(), v->_default_value);
			}

			for (auto n_function : n_udt.child("functions"))
			{
				auto f = new FunctionInfoPrivate(db, u, u->_functions.size(), n_function.attribute("name").value(), (void*)n_function.attribute("rva").as_uint(), TypeInfoPrivate::_get(n_function.attribute("return_type").value()));
				u->_functions.emplace_back(f);
				for (auto n_parameter : n_function.child("parameters"))
					f->_parameters.push_back(TypeInfoPrivate::_get(n_parameter.attribute("type").value()));
			}
		}

		if (load_with_library)
			db->_library = load_library(db->_library_name.c_str());
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

			for (auto& u : db->_udts)
			{
				for (auto& f : u.second->_functions)
				{
					if (f->_name == "bp_update")
					{
						auto n = u.second->_name + "::" + f->_name;
						for (auto& fc : function_codes)
						{
							if (fc.name == n)
							{
								f->_code = fc.code;
								break;
							}
						}
					}
				}
			}
		}

		return db;
	}

	void _push_global_typeinfo_database(TypeInfoDatabasePrivate* db)
	{
		global_typeinfo_databases.push_back(db);
	}

	void _pop_global_typeinfo_database()
	{
		global_typeinfo_databases.erase(global_typeinfo_databases.begin() + global_typeinfo_databases.size() - 1);
	}

	EnumInfoPrivate* _find_enum(uint hash)
	{
		for (auto db : global_typeinfo_databases)
		{
			auto it = db->_enums.find(hash);
			if (it != db->_enums.end())
				return it->second.get();
		}
		return nullptr;
	}
	UdtInfoPrivate* _find_udt(uint hash)
	{
		for (auto db : global_typeinfo_databases)
		{
			auto it = db->_udts.find(hash);
			if (it != db->_udts.end())
				return it->second.get();
		}
		return nullptr;
	}

	void push_global_typeinfo_database(TypeInfoDatabase* db) { _push_global_typeinfo_database((TypeInfoDatabasePrivate*)db); }
	void pop_global_typeinfo_database() { _pop_global_typeinfo_database(); }

	EnumInfo* find_enum(uint hash) { return _find_enum(hash); }
	UdtInfo* find_udt(uint hash) { return _find_udt(hash); }
}
