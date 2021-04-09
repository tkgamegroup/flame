#pragma once

#include <flame/foundation/typeinfo.h>

namespace flame
{
	struct UdtInfoPrivate;
	struct EnumInfoPrivate;
	struct TypeInfoDataBasePrivate;

	struct TypeInfoPrivate : TypeInfo
	{
		TypeTag tag;
		std::string name;
		uint size;

		BasicType basic_type = ElseType;
		bool is_signed = true;
		uint vec_size = 1;
		uint col_size = 1;
		bool ret_by_reg;
		TypeInfoPrivate* pointed_type = nullptr;

		TypeInfoPrivate(TypeTag tag, const std::string& base_name, uint size);

		TypeTag get_tag() const override { return tag; }
		const char* get_name() const override { return name.c_str(); }
		const char* get_code_name() const override { return name.c_str(); }
		uint get_size() const override { return size; }

		BasicType get_basic() const override { return basic_type; }
		bool get_signed() const override { return is_signed; }
		uint get_vec_size() const override { return vec_size; }
		uint get_col_size() const override { return col_size; }
		TypeInfoPtr get_pointed_type() const override { return pointed_type; }

		static TypeInfoPrivate* get(TypeTag tag, const std::string& name, TypeInfoDataBasePrivate* db = nullptr);
	};

	struct ReflectMetaPrivate : ReflectMeta
	{
		std::vector<std::pair<std::string, std::string>> tokens;

		uint get_tokens_count() const override { return tokens.size(); }
		void get_token(char** pname, char** pvalue, uint idx) const override;
		bool get_token(const std::string& str, char** pvalue) const;
		bool get_token(const char* str, char** pvalue) const override { return get_token(std::string(str), pvalue); }

		std::string concat() const;
	};

	struct VariableInfoPrivate : VariableInfo
	{
		UdtInfoPrivate* udt;
		uint index;
		TypeInfoPrivate* type;
		std::string name;
		uint offset;
		uint array_size = 1;
		uint array_stride = 0;
		ReflectMetaPrivate meta;
		std::string default_value;

		VariableInfoPrivate(UdtInfoPrivate* udt, uint index, TypeInfoPrivate* type, const std::string& name, uint offset, uint array_size, uint array_stride, const std::string& default_value, const std::string& meta);

		UdtInfoPtr get_udt() const override { return udt; }
		uint get_index() const override { return index; }
		TypeInfoPtr get_type() const override { return type; }
		const char* get_name() const override { return name.c_str(); }
		uint get_offset() const override { return offset; }
		uint get_array_size() const override { return array_size; }
		uint get_array_stride() const override { return array_stride; }
		ReflectMetaPtr get_meta() const override { return (const ReflectMetaPtr)&meta; }
		const char* get_default_value() const override { return default_value.c_str(); }
	};

	struct EnumItemInfoPrivate : EnumItemInfo
	{
		EnumInfoPrivate* ei;
		uint index;
		std::string name;
		int value;

		EnumItemInfoPrivate(EnumInfoPrivate* ei, uint index, const std::string& name, int value);

		EnumInfoPtr get_enum() const override { return ei; }
		uint get_index() const override { return index; }
		const char* get_name() const override { return name.c_str(); }
		int get_value() const override { return value; }
	};

	struct EnumInfoPrivate : EnumInfo
	{
		std::string name;
		std::vector<std::unique_ptr<EnumItemInfoPrivate>> items;

		EnumInfoPrivate(const std::string& name);

		const char* get_name() const override { return name.c_str(); }
		uint get_items_count() const override { return items.size(); }
		EnumItemInfoPtr get_item(uint idx) const override { return items[idx].get(); }
		EnumItemInfoPtr find_item(const std::string& name) const;
		EnumItemInfoPtr find_item(const char* name) const override { return find_item(std::string(name)); }
		EnumItemInfoPtr find_item(int value) const override;
		EnumItemInfoPtr add_item(const std::string& name, int value, int idx = -1);
		EnumItemInfoPtr add_item(const char* name, int value, int idx) override { return add_item(std::string(name), value, idx); }
		void remove_item(EnumItemInfoPtr item) override;
	};

	struct FunctionInfoPrivate : FunctionInfo
	{
		void* library;
		UdtInfoPrivate* udt;
		uint index;
		std::string name;
		uint rva;
		uint voff;
		TypeInfoPrivate* type;
		std::vector<TypeInfoPrivate*> parameters;
		std::string code;

		FunctionInfoPrivate(UdtInfoPrivate* udt, void* library, uint index, const std::string& name, uint rva, uint voff, TypeInfoPrivate* type);

		void* get_library() const override { return library; }
		UdtInfoPtr get_udt() const override { return udt; }
		uint get_index() const override { return index; }
		const char* get_name() const override { return name.c_str(); }
		uint get_rva() const override { return rva; }
		uint get_voff() const override { return voff; }
		TypeInfoPtr get_type() const override { return type; }

		uint get_parameters_count() const override { return parameters.size(); }
		TypeInfoPtr get_parameter(uint idx) const override { return parameters[idx]; }
		void add_parameter(TypeInfo* ti, int idx) override;
		void remove_parameter(uint idx) override;

		const char* get_code() const override { return code.c_str(); }

		bool check(TypeInfo* ret, uint parms_count = 0, TypeInfo* const* parms = nullptr) const override;

		void* get_address(void* obj) const override;

		void call(void* obj, void* ret, void* parameters) const override;
	};

	struct UdtInfoPrivate : UdtInfo
	{
		void* library;
		std::string name;
		uint size;
		std::string base_name;
		std::vector<std::unique_ptr<VariableInfoPrivate>> variables;
		std::vector<std::unique_ptr<FunctionInfoPrivate>> functions;

		int ranking = -1;

		UdtInfoPrivate(void* library, const std::string& name, uint size, const std::string& base_name);

		void* get_library() const override { return library; }
		const char* get_name() const override { return name.c_str(); }
		uint get_size() const override { return size; }
		const char* get_base_name() const override { return base_name.c_str(); }

		uint get_variables_count() const override { return variables.size(); }
		VariableInfoPtr get_variable(uint idx) const override { return variables[idx].get(); }
		VariableInfoPtr find_variable(const std::string& name) const;
		VariableInfoPtr find_variable(const char* name) const override { return find_variable(std::string(name)); }
		VariableInfoPtr add_variable(TypeInfoPtr ti, const std::string& name, uint offset, uint array_size, uint array_stride, const std::string& default_value, const std::string& meta, int idx = -1);
		VariableInfoPtr add_variable(TypeInfoPtr ti, const char* name, uint offset, uint array_size, uint array_stride, const char* default_value, const char* meta, int idx) override { return add_variable(ti, std::string(name), offset, array_size, array_stride, std::string(default_value), std::string(meta), idx); }
		void remove_variable(VariableInfoPtr vi) override;

		uint get_functions_count() const override { return functions.size(); }
		FunctionInfoPtr get_function(uint idx) const override { return functions[idx].get(); }
		FunctionInfoPtr find_function(const std::string& name) const;
		FunctionInfoPtr find_function(const char* name) const override { return find_function(std::string(name)); }
		FunctionInfoPtr add_function(const std::string& name, uint rva, uint voff, TypeInfoPtr ti, int idx = -1);
		FunctionInfoPtr add_function(const char* name, uint rva, uint voff, TypeInfoPtr ti, int idx) override { return add_function(std::string(name), rva, voff, ti, idx); }
		void remove_function(FunctionInfoPtr fi) override;
	};

	struct TypeInfoKey
	{
		int t;
		std::string n;

		TypeInfoKey(int t, const std::string& n) :
			t(t),
			n(n)
		{
		}

		bool operator==(const TypeInfoKey& rhs) const
		{
			return t == rhs.t && n == rhs.n;
		}
	};

	struct Hasher_TypeInfoKey
	{
		std::size_t operator()(const TypeInfoKey& k) const
		{
			return std::hash<std::string>()(k.n) ^ std::hash<int>()(k.t);
		}
	};


	struct TypeInfoDataBasePrivate : TypeInfoDataBase
	{
		std::unordered_map<TypeInfoKey, std::unique_ptr<TypeInfoPrivate>, Hasher_TypeInfoKey> typeinfos;

		std::unordered_map<std::string, std::unique_ptr<EnumInfoPrivate>> enums;
		std::unordered_map<std::string, std::unique_ptr<FunctionInfoPrivate>> functions;
		std::unordered_map<std::string, std::unique_ptr<UdtInfoPrivate>> udts;

		bool udts_sorted = false;

		void release() override { delete this; }

		void sort_udts();
	};

	EnumInfoPrivate* find_enum(const std::string& name, TypeInfoDataBasePrivate* db = nullptr);
	EnumInfoPrivate* add_enum(const std::string& name, TypeInfoDataBasePrivate* db = nullptr);
	std::vector<EnumInfoPrivate*> get_enums(TypeInfoDataBasePrivate* db = nullptr);

	UdtInfoPrivate* find_udt(const std::string& name, TypeInfoDataBasePrivate* db = nullptr);
	UdtInfoPrivate* add_udt(const std::string& name, uint size, const std::string& base_name, void* library, TypeInfoDataBasePrivate* db = nullptr);
	std::vector<UdtInfoPrivate*> get_udts(TypeInfoDataBasePrivate* db = nullptr);

	void load_typeinfo(const std::filesystem::path& filename, TypeInfoDataBasePrivate* db = nullptr);
	void save_typeinfo(const std::filesystem::path& filename, TypeInfoDataBasePrivate* db = nullptr);
}
