#pragma once

#include <flame/foundation/typeinfo.h>

namespace flame
{
	struct UdtInfoPrivate;
	struct EnumInfoPrivate;
	struct TypeInfoDatabasePrivate;

	struct TypeInfoPrivate : TypeInfo
	{
		TypeTag tag;
		bool is_array;
		std::string base_name;
		uint base_hash;
		std::string name;
		uint hash;

		TypeInfoPrivate(TypeTag tag, const std::string& base_name, bool is_array);

		TypeTag get_tag() const override { return tag; }
		bool get_is_array() const override { return is_array; }
		const char* get_base_name() const override { return base_name.c_str(); }
		uint get_base_hash() const override { return base_hash; }
		const char* get_name() const override { return name.c_str(); }
		uint get_hash() const override { return hash; }
	};

	struct VariableInfoPrivate : VariableInfo
	{
		UdtInfoPrivate* udt;
		uint index;
		TypeInfo* type;
		std::string name;
		uint name_hash;
		uint flags;
		uint offset;
		uint size;
		void* default_value;

		VariableInfoPrivate(UdtInfoPrivate* udt, uint index, TypeInfo* type, const std::string& name, uint flags, uint offset, uint size);
		~VariableInfoPrivate();

		UdtInfo* get_udt() const override { return (UdtInfo*)udt; }
		uint get_index() const override { return index; }
		TypeInfo* get_type() const override { return type; }
		const char* get_name() const override { return name.c_str(); }
		uint get_name_hash() const override { return name_hash; }
		uint get_flags() const override { return flags; }
		uint get_offset() const override { return offset; }
		uint get_size() const override { return size; }
		const void* get_default_value() const override { return default_value; }
	};

	struct EnumItemPrivate : EnumItem
	{
		EnumInfoPrivate* ei;
		uint index;
		std::string name;
		int value;

		EnumItemPrivate(EnumInfoPrivate* ei, uint index, const std::string& name, int value);

		EnumInfo* get_enum() const override { return (EnumInfo*)ei; }
		uint get_index() const override { return index; }
		const char* get_name() const override { return name.c_str(); }
		int get_value() const override { return value; }
	};

	struct EnumInfoPrivate : EnumInfo
	{
		TypeInfoDatabasePrivate* db;
		std::string name;
		std::vector<std::unique_ptr<EnumItemPrivate>> items;

		EnumInfoPrivate(TypeInfoDatabasePrivate* db, const std::string& name);

		TypeInfoDatabase* get_database() const override { return (TypeInfoDatabase*)db; }
		const char* get_name() const override { return name.c_str(); }
		uint get_items_count() const override { return items.size(); }
		EnumItem* get_item(uint idx) const override { return items[idx].get(); }
		EnumItem* find_item(const char* name) const override { return _find_item(name); }
		EnumItemPrivate* _find_item(const std::string& name) const;
		EnumItem* find_item(int value) const override { return _find_item(value); }
		EnumItemPrivate* _find_item(int value) const;
	};

	struct FunctionInfoPrivate : FunctionInfo
	{
		TypeInfoDatabasePrivate* db;
		UdtInfoPrivate* udt;
		uint index;
		std::string name;
		void* rva;
		TypeInfoPrivate* type;
		std::vector<TypeInfo*> parameters;
		std::string code;

		FunctionInfoPrivate(TypeInfoDatabasePrivate* db, UdtInfoPrivate* udt, uint index, const std::string& name, void* rva, TypeInfoPrivate* type);

		TypeInfoDatabase* get_database() const override { return (TypeInfoDatabase*)db; }
		UdtInfo* get_udt() const override { return (UdtInfo*)udt; }
		uint get_index() const override { return index; }
		const char* get_name() const override { return name.c_str(); }
		const void* get_rva() const override { return rva; }
		TypeInfo* get_type() const override { return type; }
		uint get_parameters_count() const override { return parameters.size(); }
		TypeInfo* get_parameter(uint idx) const override { return parameters[idx]; }
		const char* get_code() const override { return code.c_str(); }
	};

	struct UdtInfoPrivate : UdtInfo
	{
		TypeInfoDatabasePrivate* db;
		std::string name;
		uint size;
		std::string base_name;
		std::vector<std::unique_ptr<VariableInfoPrivate>> variables;
		std::vector<std::unique_ptr<FunctionInfoPrivate>> functions;

		UdtInfoPrivate(TypeInfoDatabasePrivate* db, const std::string& name, uint size, const std::string& base_name);

		TypeInfoDatabase* get_database() const override { return (TypeInfoDatabase*)db; }
		const char* get_name() const override { return name.c_str(); }
		uint get_size() const override { return size; }
		const char* get_base_name() const override { return base_name.c_str(); }
		uint get_variables_count() const override { return variables.size(); }
		VariableInfo* get_variable(uint idx) const override { return variables[idx].get(); }
		VariableInfo* find_variable(const char* name) const override { return _find_variable(name); }
		VariableInfoPrivate* _find_variable(const std::string& name) const;
		uint get_functions_count() const override { return functions.size(); }
		FunctionInfo* get_function(uint idx) const override { return functions[idx].get(); }
		FunctionInfo* find_function(const char* name) const override { return _find_function(name); }
		FunctionInfoPrivate* _find_function(const std::string& name) const;
	};

	struct TypeInfoDatabasePrivate : TypeInfoDatabase
	{
		void* library;
		std::wstring library_name;
		std::map<uint, std::unique_ptr<EnumInfoPrivate>> enums;
		std::map<uint, std::unique_ptr<FunctionInfoPrivate>> funs;
		std::map<uint, std::unique_ptr<UdtInfoPrivate>> udts;

		TypeInfoDatabasePrivate(const std::wstring& library_name);
		~TypeInfoDatabasePrivate();

		void release() override { delete this; }

		const void* get_library() const override { return library; }
		const wchar_t* get_library_name() const override { return library_name.c_str(); }

		EnumInfo* get_enum(uint hash) const override;
		UdtInfo* get_udt(uint hash) const override;
	};
}
