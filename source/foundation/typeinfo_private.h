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

		TypeTag get_tag() const override { return tag; }
		const char* get_name() const override { return name.c_str(); }
		uint get_size() const override { return size; }

		static TypeInfoPrivate* get(TypeTag tag, const std::string& name);
	};

	struct ReflectMetaBridge : ReflectMeta
	{
		bool get_token(const char* str, void* value_dst) const override;
	};

	struct ReflectMetaPrivate : ReflectMetaBridge
	{
		std::vector<std::pair<std::string, std::string>> tokens;

		uint get_tokens_count() const override { return tokens.size(); }
		void get_token(void* name_dst, void* value_dst, uint idx) const override;
		bool get_token(const std::string& str, void* value_dst) const;
	};

	inline bool ReflectMetaBridge::get_token(const char* str, void* value_dst) const
	{
		return ((ReflectMetaPrivate*)this)->get_token(str, value_dst);
	}

	struct VariableInfoPrivate : VariableInfo
	{
		UdtInfoPrivate* udt;
		uint index;
		TypeInfoPrivate* type;
		std::string name;
		uint offset;
		ReflectMetaPrivate meta;
		void* default_value;

		VariableInfoPrivate(UdtInfoPrivate* udt, uint index, TypeInfoPrivate* type, const std::string& name, uint offset, const std::string& meta);
		~VariableInfoPrivate();

		UdtInfo* get_udt() const override { return (UdtInfo*)udt; }
		uint get_index() const override { return index; }
		TypeInfo* get_type() const override { return type; }
		const char* get_name() const override { return name.c_str(); }
		uint get_offset() const override { return offset; }
		ReflectMeta* get_meta() const override { return (ReflectMeta*)&meta; }
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

	struct EnumInfoBridge : EnumInfo 
	{
		EnumItem* find_item(const char* name) const override;
		EnumItem* find_item(int value) const override;
	};

	struct EnumInfoPrivate : EnumInfoBridge
	{
		LibraryPrivate* library;
		std::string name;
		std::vector<std::unique_ptr<EnumItemPrivate>> items;

		EnumInfoPrivate(LibraryPrivate* db, const std::string& name);

		Library* get_library() const override { return (Library*)library; }
		const char* get_name() const override { return name.c_str(); }
		uint get_items_count() const override { return items.size(); }
		EnumItem* get_item(uint idx) const override { return items[idx].get(); }
		EnumItemPrivate* find_item(const std::string& name) const;
		EnumItemPrivate* find_item(int value) const;
	};

	inline EnumItem* EnumInfoBridge::find_item(const char* name) const
	{
		return ((EnumInfoPrivate*)this)->find_item(name);
	}

	inline EnumItem* EnumInfoBridge::find_item(int value) const
	{
		return ((EnumInfoPrivate*)this)->find_item(value);
	}

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

		Library* get_library() const override { return (Library*)library; }
		UdtInfo* get_udt() const override { return (UdtInfo*)udt; }
		uint get_index() const override { return index; }
		const char* get_name() const override { return name.c_str(); }
		void* get_address(void* obj) const override;
		TypeInfo* get_type() const override { return type; }
		uint get_parameters_count() const override { return parameters.size(); }
		TypeInfo* get_parameter(uint idx) const override { return parameters[idx]; }
		const char* get_code() const override { return code.c_str(); }

		bool check(void* type, ...) const override;

		void call(void* obj, void* ret, void** parameters) const override;
	};

	struct UdtInfoBridge : UdtInfo 
	{
		VariableInfo* find_variable(const char* name) const override;
		FunctionInfo* find_function(const char* name) const override;
	};

	struct UdtInfoPrivate : UdtInfoBridge
	{
		LibraryPrivate* library;
		std::string name;
		uint size;
		std::string base_name;
		std::vector<std::unique_ptr<VariableInfoPrivate>> variables;
		std::vector<std::unique_ptr<FunctionInfoPrivate>> functions;

		UdtInfoPrivate(LibraryPrivate* db, const std::string& name, uint size, const std::string& base_name);

		Library* get_library() const override { return (Library*)library; }
		const char* get_name() const override { return name.c_str(); }
		uint get_size() const override { return size; }
		const char* get_base_name() const override { return base_name.c_str(); }
		uint get_variables_count() const override { return variables.size(); }
		VariableInfo* get_variable(uint idx) const override { return variables[idx].get(); }
		VariableInfoPrivate* find_variable(const std::string& name) const;
		uint get_functions_count() const override { return functions.size(); }
		FunctionInfo* get_function(uint idx) const override { return functions[idx].get(); }
		FunctionInfoPrivate* find_function(const std::string& name) const;
	};

	inline VariableInfo* UdtInfoBridge::find_variable(const char* name) const
	{
		return ((UdtInfoPrivate*)this)->find_variable(name);
	}

	inline FunctionInfo* UdtInfoBridge::find_function(const char* name) const
	{
		return ((UdtInfoPrivate*)this)->find_function(name);
	}

	struct LibraryPrivate : Library
	{
		char* address;
		std::wstring filename;
		bool has_typeinfo = false;
		int ref_count = 1;

		LibraryPrivate(const std::wstring& filename, bool require_typeinfo);
		~LibraryPrivate();

		void* _get_exported_function(const char* name);

		void release() override;

		char* get_address() const override { return address; }
		const wchar_t* get_filename() const override { return filename.c_str(); }
		void* get_exported_function(const char* name) override { return _get_exported_function(name); }
	};

	EnumInfoPrivate* find_enum(const std::string& name);
	UdtInfoPrivate* find_udt(const std::string& name);
}
