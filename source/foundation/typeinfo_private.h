#pragma once

#include <flame/foundation/typeinfo.h>

namespace flame
{
	struct UdtInfoPrivate;
	struct EnumInfoPrivate;
	struct LibraryPrivate;

	struct TypeInfoPrivate : TypeInfo
	{
		TypeTag tag;
		std::string name;
		uint size;

		TypeInfoPrivate(TypeTag tag, const std::string& base_name, uint size);

		static TypeInfoPrivate* _get(TypeTag tag, const std::string& name);

		TypeTag get_tag() const override { return tag; }
		const char* get_name() const override { return name.c_str(); }
		uint get_size() const override { return size; }
	};

	struct VariableInfoPrivate : VariableInfo
	{
		UdtInfoPrivate* udt;
		uint index;
		TypeInfoPrivate* type;
		std::string name;
		uint flags;
		uint offset;
		void* default_value;

		VariableInfoPrivate(UdtInfoPrivate* udt, uint index, TypeInfoPrivate* type, const std::string& name, uint flags, uint offset);
		~VariableInfoPrivate();

		UdtInfo* get_udt() const override { return (UdtInfo*)udt; }
		uint get_index() const override { return index; }
		TypeInfo* get_type() const override { return type; }
		const char* get_name() const override { return name.c_str(); }
		uint get_flags() const override { return flags; }
		uint get_offset() const override { return offset; }
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
		LibraryPrivate* library;
		std::string name;
		std::vector<std::unique_ptr<EnumItemPrivate>> items;

		EnumInfoPrivate(LibraryPrivate* db, const std::string& name);

		EnumItemPrivate* _find_item(const std::string& name) const;
		EnumItemPrivate* _find_item(int value) const;

		Library* get_library() const override { return (Library*)library; }
		const char* get_name() const override { return name.c_str(); }
		uint get_items_count() const override { return items.size(); }
		EnumItem* get_item(uint idx) const override { return items[idx].get(); }
		EnumItem* find_item(const char* name) const override { return _find_item(name); }
		EnumItem* find_item(int value) const override { return _find_item(value); }
	};

	struct FunctionInfoPrivate : FunctionInfo
	{
		LibraryPrivate* library;
		UdtInfoPrivate* udt;
		uint index;
		std::string name;
		uint rva;
		uint voff;
		TypeInfoPrivate* type;
		std::vector<TypeInfoPrivate*> parameters;
		std::string code;

		FunctionInfoPrivate(LibraryPrivate* db, UdtInfoPrivate* udt, uint index, const std::string& name, uint rva, uint voff, TypeInfoPrivate* type);

		bool _check_v(TypeInfoPrivate* type, char* ap) const;
		bool _check(TypeInfoPrivate* type, ...) const { return _check_v(type, (char*)var_end(&type)); }

		void _call_v(void* obj, void* ret, char* ap) const;
		void _call(void* obj, void* ret, ...) const { return _call_v(obj, ret, (char*)var_end(&ret)); }

		Library* get_library() const override { return (Library*)library; }
		UdtInfo* get_udt() const override { return (UdtInfo*)udt; }
		uint get_index() const override { return index; }
		const char* get_name() const override { return name.c_str(); }
		uint get_rva() const override { return rva; }
		uint get_voff() const override { return voff; }
		TypeInfo* get_type() const override { return type; }
		uint get_parameters_count() const override { return parameters.size(); }
		TypeInfo* get_parameter(uint idx) const override { return parameters[idx]; }
		const char* get_code() const override { return code.c_str(); }

		bool check(TypeInfo* type, ...) const override { return _check_v((TypeInfoPrivate*)type, (char*)var_end(&type)); }

		void call(void* obj, void* ret, ...) const override { return _call_v(obj, ret, (char*)var_end(&ret)); }
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
