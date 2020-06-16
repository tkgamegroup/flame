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

		TypeTag get_tag() const override;
		bool get_is_array() const override;
		const char* get_base_name() const override; 
		uint get_base_hash() const override;
		const char* get_name() const override; 
		uint get_hash() const override;
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

		UdtInfo* get_udt() const override;
		uint get_index() const override;
		TypeInfo* get_type() const override;
		const char* get_name() const override;
		uint get_name_hash() const override;
		uint get_flags() const override;
		uint get_offset() const override;
		uint get_size() const override;
		const void* get_default_value() const override;
	};

	struct EnumItemPrivate : EnumItem
	{
		EnumInfoPrivate* ei;
		uint index;
		std::string name;
		int value;

		EnumItemPrivate(EnumInfoPrivate* ei, uint index, const std::string& name, int value);

		EnumInfo* get_enum() const override;
		uint get_index() const override;
		const char* get_name() const override;
		int get_value() const override;
	};

	struct EnumInfoPrivate : EnumInfo
	{
		TypeInfoDatabasePrivate* db;
		std::string name;
		std::vector<std::unique_ptr<EnumItemPrivate>> items;

		EnumInfoPrivate(TypeInfoDatabasePrivate* db, const std::string& name);

		TypeInfoDatabase* get_database() const override;
		const char* get_name() const override;
		uint get_items_count() const override;
		EnumItem* get_item(uint idx) const override;
		EnumItem* find_item(const char* name) const override;
		EnumItemPrivate* _find_item(const std::string& name) const;
		EnumItem* find_item(int value) const override;
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

		TypeInfoDatabase* get_database() const override;
		UdtInfo* get_udt() const override;
		uint get_index() const override;
		const char* get_name() const override;
		const void* get_rva() const override;
		TypeInfo* get_type() const override;
		uint get_parameters_count() const override;
		TypeInfo* get_parameter(uint idx) const override;
		const char* get_code() const override;
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

		TypeInfoDatabase* get_database() const override;
		const char* get_name() const override;
		uint get_size() const override;
		const char* get_base_name() const override; 
		uint get_variables_count() const override;
		VariableInfo* get_variable(uint idx) const override;
		VariableInfo* find_variable(const char* name) const override;
		VariableInfoPrivate* _find_variable(const std::string& name) const;
		uint get_functions_count() const override;
		FunctionInfo* get_function(uint idx) const override;
		FunctionInfo* find_function(const char* name) const override;
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

		void release() override;

		const void* get_library() const override;
		const wchar_t* get_library_name() const override;

		EnumInfo* get_enum(uint hash) const override;
		UdtInfo* get_udt(uint hash) const override;
	};
}
