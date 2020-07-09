#pragma once

#include <flame/foundation/typeinfo.h>

namespace flame
{
	struct UdtInfoPrivate;
	struct EnumInfoPrivate;
	struct LibraryPrivate;

	struct TypeInfoPrivate__;

	struct TypeInfoPrivate : TypeInfo
	{
		TypeTag tag;
		std::string name;
		uint size;

		TypeInfoPrivate__* operator->() { return (TypeInfoPrivate__*)this; }
		TypeInfoPrivate__* operator->() const { return (TypeInfoPrivate__*)this; }

		TypeInfoPrivate(TypeTag tag, const std::string& base_name, uint size);

		TypeTag get_tag() const override { return tag; }
		const char* get_name() const override { return name.c_str(); }
		uint get_size() const override { return size; }
	};

	struct TypeInfoPrivate__ : TypeInfoPrivate
	{
		static TypeInfoPrivate* get(TypeTag tag, const std::string& name);
	};

	struct VariableInfoPrivate__;

	struct VariableInfoPrivate : VariableInfo
	{
		UdtInfoPrivate* udt;
		uint index;
		TypeInfoPrivate* type;
		std::string name;
		uint flags;
		uint offset;
		void* default_value;

		VariableInfoPrivate__* operator->() { return (VariableInfoPrivate__*)this; }
		VariableInfoPrivate__* operator->() const { return (VariableInfoPrivate__*)this; }

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

	struct VariableInfoPrivate__ : VariableInfoPrivate
	{
	};

	struct EnumItemPrivate__;

	struct EnumItemPrivate : EnumItem
	{
		EnumInfoPrivate* ei;
		uint index;
		std::string name;
		int value;

		EnumItemPrivate__* operator->() { return (EnumItemPrivate__*)this; }
		EnumItemPrivate__* operator->() const { return (EnumItemPrivate__*)this; }

		EnumItemPrivate(EnumInfoPrivate* ei, uint index, const std::string& name, int value);

		EnumInfo* get_enum() const override { return (EnumInfo*)ei; }
		uint get_index() const override { return index; }
		const char* get_name() const override { return name.c_str(); }
		int get_value() const override { return value; }
	};

	struct EnumItemPrivate__ : EnumItemPrivate
	{
	};

	struct EnumInfoPrivate__;

	struct EnumInfoPrivate : EnumInfo
	{
		LibraryPrivate* library;
		std::string name;
		std::vector<std::unique_ptr<EnumItemPrivate>> items;

		EnumInfoPrivate__* operator->() { return (EnumInfoPrivate__*)this; }
		EnumInfoPrivate__* operator->() const { return (EnumInfoPrivate__*)this; }

		EnumInfoPrivate(LibraryPrivate* db, const std::string& name);

		Library* get_library() const override { return (Library*)library; }
		const char* get_name() const override { return name.c_str(); }
		uint get_items_count() const override { return items.size(); }
		EnumItem* get_item(uint idx) const override { return items[idx].get(); }
		EnumItem* find_item(const char* name) const override;
		EnumItem* find_item(int value) const override;
	};

	struct EnumInfoPrivate__ : EnumInfoPrivate
	{
		EnumItemPrivate* find_item(const std::string& name) const;
		EnumItemPrivate* find_item(int value) const;
	};

	struct FunctionInfoPrivate__;

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

		FunctionInfoPrivate__* operator->() { return (FunctionInfoPrivate__*)this; }
		FunctionInfoPrivate__* operator->() const { return (FunctionInfoPrivate__*)this; }

		FunctionInfoPrivate(LibraryPrivate* db, UdtInfoPrivate* udt, uint index, const std::string& name, uint rva, uint voff, TypeInfoPrivate* type);

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

		bool check(void* type, ...) const override;

		void call(void* obj, void* ret, ...) const override;
	};

	struct FunctionInfoPrivate__ : FunctionInfoPrivate
	{
	};

	struct UdtInfoPrivate__;

	struct UdtInfoPrivate : UdtInfo
	{
		LibraryPrivate* library;
		std::string name;
		uint size;
		std::string base_name;
		std::vector<std::unique_ptr<VariableInfoPrivate>> variables;
		std::vector<std::unique_ptr<FunctionInfoPrivate>> functions;

		UdtInfoPrivate__* operator->() { return (UdtInfoPrivate__*)this; }
		UdtInfoPrivate__* operator->() const { return (UdtInfoPrivate__*)this; }

		UdtInfoPrivate(LibraryPrivate* db, const std::string& name, uint size, const std::string& base_name);

		Library* get_library() const override { return (Library*)library; }
		const char* get_name() const override { return name.c_str(); }
		uint get_size() const override { return size; }
		const char* get_base_name() const override { return base_name.c_str(); }
		uint get_variables_count() const override { return variables.size(); }
		VariableInfo* get_variable(uint idx) const override { return variables[idx].get(); }
		VariableInfo* find_variable(const char* name) const override;
		uint get_functions_count() const override { return functions.size(); }
		FunctionInfo* get_function(uint idx) const override { return functions[idx].get(); }
		FunctionInfo* find_function(const char* name) const override;
	};

	struct UdtInfoPrivate__ : UdtInfoPrivate
	{
		VariableInfoPrivate* find_variable(const std::string& name) const;
		FunctionInfoPrivate* find_function(const std::string& name) const;
	};

	struct LibraryPrivate__;

	struct LibraryPrivate : Library
	{
		char* address;
		std::wstring filename;
		bool has_typeinfo = false;
		int ref_count = 1;

		LibraryPrivate__* operator->() { return (LibraryPrivate__*)this; }
		LibraryPrivate__* operator->() const { return (LibraryPrivate__*)this; }

		LibraryPrivate(const std::wstring& filename, bool require_typeinfo);
		~LibraryPrivate();

		void _release();

		void* _get_exported_function(const char* name);

		void release() override { _release(); }

		char* get_address() const override { return address; }
		const wchar_t* get_filename() const override { return filename.c_str(); }
		void* get_exported_function(const char* name) override { return _get_exported_function(name); }
	};

	struct LibraryPrivate__ : LibraryPrivate
	{
	};

	EnumInfoPrivate* find_enum(const std::string& name);
	UdtInfoPrivate* find_udt(const std::string& name);
}
