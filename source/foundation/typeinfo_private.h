#pragma once

#include <flame/foundation/typeinfo.h>

namespace flame
{
	struct UdtInfoPrivate;
	struct EnumInfoPrivate;
	struct LibraryPrivate;

	struct TypeInfoPrivate : TypeInfo
	{
		TypeTag _tag;
		std::string _name;
		uint _size;

		TypeInfoPrivate(TypeTag tag, const std::string& base_name, uint size);

		static TypeInfoPrivate* _get(TypeTag tag, const std::string& name);

		TypeTag get_tag() const override { return _tag; }
		const char* get_name() const override { return _name.c_str(); }
		uint get_size() const override { return _size; }
	};

	struct VariableInfoPrivate : VariableInfo
	{
		UdtInfoPrivate* _udt;
		uint _index;
		TypeInfoPrivate* _type;
		std::string _name;
		uint _flags;
		uint _offset;
		void* _default_value;

		VariableInfoPrivate(UdtInfoPrivate* udt, uint index, TypeInfoPrivate* type, const std::string& name, uint flags, uint offset);
		~VariableInfoPrivate();

		UdtInfo* get_udt() const override { return (UdtInfo*)_udt; }
		uint get_index() const override { return _index; }
		TypeInfo* get_type() const override { return _type; }
		const char* get_name() const override { return _name.c_str(); }
		uint get_flags() const override { return _flags; }
		uint get_offset() const override { return _offset; }
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
		LibraryPrivate* _library;
		std::string _name;
		std::vector<std::unique_ptr<EnumItemPrivate>> _items;

		EnumInfoPrivate(LibraryPrivate* db, const std::string& name);

		EnumItemPrivate* _find_item(const std::string& name) const;
		EnumItemPrivate* _find_item(int value) const;

		Library* get_library() const override { return (Library*)_library; }
		const char* get_name() const override { return _name.c_str(); }
		uint get_items_count() const override { return _items.size(); }
		EnumItem* get_item(uint idx) const override { return _items[idx].get(); }
		EnumItem* find_item(const char* name) const override { return _find_item(name); }
		EnumItem* find_item(int value) const override { return _find_item(value); }
	};

	struct FunctionInfoPrivate : FunctionInfo
	{
		LibraryPrivate* _library;
		UdtInfoPrivate* _udt;
		uint _index;
		std::string _name;
		uint _rva;
		uint _voff;
		TypeInfoPrivate* _type;
		std::vector<TypeInfoPrivate*> _parameters;
		std::string _code;

		FunctionInfoPrivate(LibraryPrivate* db, UdtInfoPrivate* udt, uint index, const std::string& name, uint rva, uint voff, TypeInfoPrivate* type);

		bool _check_v(TypeInfoPrivate* type, char* ap) const;
		bool _check(TypeInfoPrivate* type, ...) const { return _check_v(type, (char*)var_end(&type)); }

		Library* get_library() const override { return (Library*)_library; }
		UdtInfo* get_udt() const override { return (UdtInfo*)_udt; }
		uint get_index() const override { return _index; }
		const char* get_name() const override { return _name.c_str(); }
		uint get_rva() const override { return _rva; }
		uint get_voff() const override { return _voff; }
		TypeInfo* get_type() const override { return _type; }
		uint get_parameters_count() const override { return _parameters.size(); }
		TypeInfo* get_parameter(uint idx) const override { return _parameters[idx]; }
		const char* get_code() const override { return _code.c_str(); }

		bool check(TypeInfo* type, ...) const override { return _check((TypeInfoPrivate*)type, var_end(&type)); }
	};

	struct UdtInfoPrivate : UdtInfo
	{
		LibraryPrivate* _library;
		std::string _name;
		uint _size;
		std::string _base_name;
		std::vector<std::unique_ptr<VariableInfoPrivate>> _variables;
		std::vector<std::unique_ptr<FunctionInfoPrivate>> _functions;

		UdtInfoPrivate(LibraryPrivate* db, const std::string& name, uint size, const std::string& base_name);

		VariableInfoPrivate* _find_variable(const std::string& name) const;
		FunctionInfoPrivate* _find_function(const std::string& name) const;

		Library* get_library() const override { return (Library*)_library; }
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

	struct LibraryPrivate : Library
	{
		char* _address;
		std::wstring _filename;
		bool _has_typeinfo = false;
		int _ref_count = 1;

		LibraryPrivate(const std::wstring& filename, bool require_typeinfo);
		~LibraryPrivate();

		void _release();

		void* _get_exported_function(const char* name);

		void release() override { _release(); }

		char* get_address() const override { return _address; }
		const wchar_t* get_filename() const override { return _filename.c_str(); }
		void* get_exported_function(const char* name) override { return _get_exported_function(name); }
	};

	EnumInfoPrivate* _find_enum(const std::string& name);
	UdtInfoPrivate* _find_udt(const std::string& name);
}
