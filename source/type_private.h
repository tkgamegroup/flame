#pragma once

#include <flame/type.h>

namespace flame
{
	struct TypeInfoPrivate : TypeInfo
	{
		TypeTag tag;
		bool is_array;
		std::string base_name;
		std::string name;
		uint base_hash;
		uint hash;

		inline TypeInfoPrivate(TypeTag tag, const std::string& base_name, bool is_array) :
			tag(tag),
			base_name(base_name),
			base_hash(FLAME_HASH(base_name.c_str())),
			is_array(is_array)
		{
			name = make_str(tag, base_name, is_array);
			hash = FLAME_HASH(name.c_str());
		}
	};

	struct VariableInfoPrivate : VariableInfo
	{
		TypeInfoPrivate* type;
		std::string name;
		uint name_hash;
		uint flags;
		uint offset, size;
		std::string default_value;
	};

	struct EnumItemPrivate : EnumItem
	{
		std::string name;
		int value;
	};

	struct EnumInfoPrivate : EnumInfo
	{
		TypeinfoDatabase* db;

		std::string name;
		std::vector<std::unique_ptr<EnumItemPrivate>> items;

		inline EnumItemPrivate* add_item(const std::string& name, int value)
		{
			auto i = new EnumItemPrivate;
			i->name = name;
			i->value = value;
			items.emplace_back(i);
			return i;
		}
	};

	struct FunctionInfoPrivate : FunctionInfo
	{
		TypeinfoDatabase* db;

		std::string name;
		void* rva;
		TypeInfoPrivate* return_type;
		std::vector<TypeInfoPrivate*> parameter_types;

		inline void add_parameter(const TypeInfo* type)
		{
			parameter_types.push_back((TypeInfoPrivate*)type);
		}
	};

	struct UdtInfoPrivate : UdtInfo
	{
		TypeinfoDatabase* db;

		const TypeInfoPrivate* type;
		uint size;
		std::vector<std::unique_ptr<VariableInfoPrivate>> variables;
		std::vector<std::unique_ptr<FunctionInfoPrivate>> functions;

		inline VariableInfoPrivate* add_variable(const TypeInfo* type, const std::string& name, uint flags, uint offset, uint size)
		{
			auto v = new VariableInfoPrivate;
			v->type = (TypeInfoPrivate*)type;
			v->name = name;
			v->name_hash = FLAME_HASH(name.c_str());
			v->flags = flags;
			v->offset = offset;
			v->size = size;
			variables.emplace_back(v);
			return v;
		}

		inline VariableInfoPrivate* find_variable(const std::string& name, int* out_idx)
		{
			for (auto i = 0; i < variables.size(); i++)
			{
				if (variables[i]->name == name)
				{
					if (out_idx)
						*out_idx = i;
					return variables[i].get();
				}
			}
			if (out_idx)
				*out_idx = -1;
			return nullptr;
		}

		inline FunctionInfoPrivate* find_function(const std::string& name, int* out_idx)
		{
			for (auto i = 0; i < functions.size(); i++)
			{
				if (functions[i]->name == name)
				{
					if (out_idx)
						*out_idx = i;
					return functions[i].get();
				}
			}
			if (out_idx)
				*out_idx = -1;
			return nullptr;
		}

		inline FunctionInfoPrivate* add_function(const std::string& name, void* rva, const TypeInfo* return_type)
		{
			auto f = new FunctionInfoPrivate;
			f->return_type = (TypeInfoPrivate*)return_type;
			f->db = db;
			f->name = name;
			f->rva = rva;
			functions.emplace_back(f);
			return f;
		}
	};

	struct TypeinfoDatabasePrivate : TypeinfoDatabase
	{
		void* module;
		std::wstring module_name;

		std::map<uint, std::unique_ptr<EnumInfoPrivate>> enums;
		std::map<uint, std::unique_ptr<UdtInfoPrivate>> udts;

		inline TypeinfoDatabasePrivate()
		{
			module = nullptr;
		}

		inline ~TypeinfoDatabasePrivate()
		{
			if (module)
				free_module(module);
		}

		inline EnumInfoPrivate* add_enum(const std::string name)
		{
			auto e = new EnumInfoPrivate;
			e->db = this;
			e->name = name;
			enums.emplace(FLAME_HASH(name.c_str()), e);
			return e;
		}

		inline UdtInfoPrivate* add_udt(const TypeInfo* type, uint size)
		{
			auto u = new UdtInfoPrivate;
			u->type = (const TypeInfoPrivate*)type;
			u->db = this;
			u->size = size;
			udts.emplace(((const TypeInfoPrivate*)type)->hash, u);
			return u;
		}
	};
}
