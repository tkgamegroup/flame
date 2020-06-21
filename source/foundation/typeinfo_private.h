#pragma once

#include <flame/foundation/typeinfo.h>

namespace flame
{
	struct UdtInfoPrivate;
	struct EnumInfoPrivate;
	struct TypeInfoDatabasePrivate;

	struct TypeInfoPrivate : TypeInfo
	{
		TypeTag _tag;
		bool _is_array;
		std::string _base_name;
		uint _base_hash;
		std::string _name;
		uint _hash;

		TypeInfoPrivate(TypeTag tag, const std::string& base_name, bool is_array);

		static std::string get_name(TypeTag tag, const std::string& base_name, bool is_array = false);
		static TypeInfoPrivate* _get(TypeTag tag, const std::string& base_name, bool is_array = false);
		static TypeInfoPrivate* _get(const std::string& str);

		TypeTag get_tag() const override { return _tag; }
		bool get_is_array() const override { return _is_array; }
		const char* get_base_name() const override { return _base_name.c_str(); }
		uint get_base_hash() const override { return _base_hash; }
		const char* get_name() const override { return _name.c_str(); }
		uint get_hash() const override { return _hash; }
	};

	struct VariableInfoPrivate : VariableInfo
	{
		UdtInfoPrivate* _udt;
		uint _index;
		TypeInfoPrivate* _type;
		std::string _name;
		uint _name_hash;
		uint _flags;
		uint _offset;
		uint _size;
		void* _default_value;

		VariableInfoPrivate(UdtInfoPrivate* udt, uint index, TypeInfoPrivate* type, const std::string& name, uint flags, uint offset, uint size);
		~VariableInfoPrivate();

		UdtInfo* get_udt() const override { return (UdtInfo*)_udt; }
		uint get_index() const override { return _index; }
		TypeInfo* get_type() const override { return _type; }
		const char* get_name() const override { return _name.c_str(); }
		uint get_name_hash() const override { return _name_hash; }
		uint get_flags() const override { return _flags; }
		uint get_offset() const override { return _offset; }
		uint get_size() const override { return _size; }
		const void* get_default_value() const override { return _default_value; }
	};

	struct EnumItemPrivate : EnumItem
	{
		EnumInfoPrivate* _ei;
		uint _index;
		std::string _name;
		int _value;

		EnumItemPrivate(EnumInfoPrivate* ei, uint index, const std::string& name, int value);

		EnumInfo* get_enum() const override { return (EnumInfo*)_ei; }
		uint get_index() const override { return _index; }
		const char* get_name() const override { return _name.c_str(); }
		int get_value() const override { return _value; }
	};

	struct EnumInfoPrivate : EnumInfo
	{
		TypeInfoDatabasePrivate* _db;
		std::string _name;
		std::vector<std::unique_ptr<EnumItemPrivate>> _items;

		EnumInfoPrivate(TypeInfoDatabasePrivate* db, const std::string& name);

		EnumItemPrivate* _find_item(const std::string& name) const;
		EnumItemPrivate* _find_item(int value) const;

		TypeInfoDatabase* get_database() const override { return (TypeInfoDatabase*)_db; }
		const char* get_name() const override { return _name.c_str(); }
		uint get_items_count() const override { return _items.size(); }
		EnumItem* get_item(uint idx) const override { return _items[idx].get(); }
		EnumItem* find_item(const char* name) const override { return _find_item(name); }
		EnumItem* find_item(int value) const override { return _find_item(value); }
	};

	struct FunctionInfoPrivate : FunctionInfo
	{
		TypeInfoDatabasePrivate* _db;
		UdtInfoPrivate* _udt;
		uint _index;
		std::string _name;
		void* _rva;
		TypeInfoPrivate* _type;
		std::vector<TypeInfoPrivate*> _parameters;
		std::string _code;

		FunctionInfoPrivate(TypeInfoDatabasePrivate* db, UdtInfoPrivate* udt, uint index, const std::string& name, void* rva, TypeInfoPrivate* type);

		TypeInfoDatabase* get_database() const override { return (TypeInfoDatabase*)_db; }
		UdtInfo* get_udt() const override { return (UdtInfo*)_udt; }
		uint get_index() const override { return _index; }
		const char* get_name() const override { return _name.c_str(); }
		const void* get_rva() const override { return _rva; }
		TypeInfo* get_type() const override { return _type; }
		uint get_parameters_count() const override { return _parameters.size(); }
		TypeInfo* get_parameter(uint idx) const override { return _parameters[idx]; }
		const char* get_code() const override { return _code.c_str(); }
	};

	struct UdtInfoPrivate : UdtInfo
	{
		TypeInfoDatabasePrivate* _db;
		std::string _name;
		uint _size;
		std::string _base_name;
		std::vector<std::unique_ptr<VariableInfoPrivate>> _variables;
		std::vector<std::unique_ptr<FunctionInfoPrivate>> _functions;

		UdtInfoPrivate(TypeInfoDatabasePrivate* db, const std::string& name, uint size, const std::string& base_name);

		VariableInfoPrivate* _find_variable(const std::string& name) const;
		FunctionInfoPrivate* _find_function(const std::string& name) const;

		TypeInfoDatabase* get_database() const override { return (TypeInfoDatabase*)_db; }
		const char* get_name() const override { return _name.c_str(); }
		uint get_size() const override { return _size; }
		const char* get_base_name() const override { return _base_name.c_str(); }
		uint get_variables_count() const override { return _variables.size(); }
		VariableInfo* get_variable(uint idx) const override { return _variables[idx].get(); }
		VariableInfo* find_variable(const char* name) const override { return _find_variable(name); }
		uint get_functions_count() const override { return _functions.size(); }
		FunctionInfo* get_function(uint idx) const override { return _functions[idx].get(); }
		FunctionInfo* find_function(const char* name) const override { return _find_function(name); }
	};

	struct TypeInfoDatabasePrivate : TypeInfoDatabase
	{
		void* _library;
		std::wstring _library_name;
		std::map<uint, std::unique_ptr<EnumInfoPrivate>> _enums;
		std::map<uint, std::unique_ptr<FunctionInfoPrivate>> _funs;
		std::map<uint, std::unique_ptr<UdtInfoPrivate>> _udts;

		TypeInfoDatabasePrivate(const std::wstring& library_name);
		~TypeInfoDatabasePrivate();

		void release() override { delete this; }

		const void* get_library() const override { return _library; }
		const wchar_t* get_library_name() const override { return _library_name.c_str(); }
	};

	void _push_global_typeinfo_database(TypeInfoDatabasePrivate* db);
	void _pop_global_typeinfo_database();

	EnumInfoPrivate* _find_enum(uint hash);
	UdtInfoPrivate* _find_udt(uint hash);

	inline void push_global_typeinfo_database(TypeInfoDatabase* db) { _push_global_typeinfo_database((TypeInfoDatabasePrivate*)db); }
	inline void pop_global_typeinfo_database() { _pop_global_typeinfo_database(); }

	inline EnumInfo* find_enum(uint hash) { return _find_enum(hash); }
	inline UdtInfo* find_udt(uint hash) { return _find_udt(hash); }
}
