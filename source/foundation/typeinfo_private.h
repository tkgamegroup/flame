#pragma once

#include <flame/foundation/typeinfo.h>

namespace flame
{
	struct UdtInfoPrivate;
	struct EnumInfoPrivate;
	struct LibraryPrivate;
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
		TypeInfo* get_pointed_type() const override { return pointed_type; }

		static TypeInfoPrivate* get(TypeTag tag, const std::string& name, TypeInfoDataBasePrivate* db = nullptr);
	};

	struct ReflectMetaBridge : ReflectMeta
	{
		bool get_token(const char* str, char** pvalue) const override;
	};

	struct ReflectMetaPrivate : ReflectMetaBridge
	{
		std::vector<std::pair<std::string, std::string>> tokens;

		uint get_tokens_count() const override { return tokens.size(); }
		void get_token(char** pname, char** pvalue, uint idx) const override;
		bool get_token(const std::string& str, char** pvalue) const;

		std::string concat() const;
	};

	inline bool ReflectMetaBridge::get_token(const char* str, char** pvalue) const
	{
		return ((ReflectMetaPrivate*)this)->get_token(str, pvalue);
	}

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

		UdtInfo* get_udt() const override { return (UdtInfo*)udt; }
		uint get_index() const override { return index; }
		TypeInfo* get_type() const override { return type; }
		const char* get_name() const override { return name.c_str(); }
		uint get_offset() const override { return offset; }
		uint get_array_size() const override { return array_size; }
		uint get_array_stride() const override { return array_stride; }
		ReflectMeta* get_meta() const override { return (ReflectMeta*)&meta; }
		const char* get_default_value() const override { return default_value.c_str(); }
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
		EnumItem* add_item(const char* name, int value, int idx) override;
		void remove_item(EnumItem* item) override;
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
		EnumItemPrivate* add_item(const std::string& name, int value, int idx = -1);
		void remove_item(EnumItemPrivate* item);
	};

	inline EnumItem* EnumInfoBridge::find_item(const char* name) const
	{
		return ((EnumInfoPrivate*)this)->find_item(name);
	}

	inline EnumItem* EnumInfoBridge::find_item(int value) const
	{
		return ((EnumInfoPrivate*)this)->find_item(value);
	}

	inline EnumItem* EnumInfoBridge::add_item(const char* name, int value, int idx)
	{
		return ((EnumInfoPrivate*)this)->add_item(name, value, idx);
	}

	inline void EnumInfoBridge::remove_item(EnumItem* item)
	{
		((EnumInfoPrivate*)this)->remove_item((EnumItemPrivate*)item);
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
		uint get_rva() const override { return rva; }
		uint get_voff() const override { return voff; }
		TypeInfo* get_type() const override { return type; }

		uint get_parameters_count() const override { return parameters.size(); }
		TypeInfo* get_parameter(uint idx) const override { return parameters[idx]; }
		void add_parameter(TypeInfo* ti, int idx) override;
		void remove_parameter(uint idx) override;

		const char* get_code() const override { return code.c_str(); }

		bool check(TypeInfo* ret, uint parms_count = 0, TypeInfo* const* parms = nullptr) const override;

		void* get_address(void* obj) const override;

		void call(void* obj, void* ret, void* parameters) const override;
	};

	struct UdtInfoBridge : UdtInfo 
	{
		VariableInfo* find_variable(const char* name) const override;
		VariableInfo* add_variable(TypeInfo* ti, const char* name, uint offset, uint array_size, uint array_stride, const char* default_value, const char* meta, int idx) override;
		void remove_variable(VariableInfo* vi) override;
		FunctionInfo* find_function(const char* name) const override;
		FunctionInfo* add_function(const char* name, uint rva, uint voff, TypeInfo* ti, int idx) override;
		void remove_function(FunctionInfo* fi) override;
	};

	struct UdtInfoPrivate : UdtInfoBridge
	{
		LibraryPrivate* library;
		std::string name;
		uint size;
		std::string base_name;
		std::vector<std::unique_ptr<VariableInfoPrivate>> variables;
		std::vector<std::unique_ptr<FunctionInfoPrivate>> functions;

		int ranking = -1;

		UdtInfoPrivate(LibraryPrivate* db, const std::string& name, uint size, const std::string& base_name);

		Library* get_library() const override { return (Library*)library; }
		const char* get_name() const override { return name.c_str(); }
		uint get_size() const override { return size; }
		const char* get_base_name() const override { return base_name.c_str(); }

		uint get_variables_count() const override { return variables.size(); }
		VariableInfo* get_variable(uint idx) const override { return variables[idx].get(); }
		VariableInfoPrivate* find_variable(const std::string& name) const;
		VariableInfoPrivate* add_variable(TypeInfoPrivate* ti, const std::string& name, uint offset, uint array_size, uint array_stride, const std::string& default_value, const std::string& meta, int idx = -1);
		void remove_variable(VariableInfoPrivate* vi);

		uint get_functions_count() const override { return functions.size(); }
		FunctionInfo* get_function(uint idx) const override { return functions[idx].get(); }
		FunctionInfoPrivate* find_function(const std::string& name) const;
		FunctionInfoPrivate* add_function(const std::string& name, uint rva, uint voff, TypeInfoPrivate* ti, int idx = -1);
		void remove_function(FunctionInfoPrivate* fi);
	};

	inline VariableInfo* UdtInfoBridge::find_variable(const char* name) const
	{
		return ((UdtInfoPrivate*)this)->find_variable(name);
	}

	inline VariableInfo* UdtInfoBridge::add_variable(TypeInfo* ti, const char* name, uint offset, uint array_size, uint array_stride, const char* default_value, const char* meta, int idx)
	{
		return ((UdtInfoPrivate*)this)->add_variable((TypeInfoPrivate*)ti, name, offset, array_size, array_stride, default_value, meta, idx);
	}

	inline void UdtInfoBridge::remove_variable(VariableInfo* vi)
	{
		((UdtInfoPrivate*)this)->remove_variable((VariableInfoPrivate*)vi);
	}

	inline FunctionInfo* UdtInfoBridge::find_function(const char* name) const
	{
		return ((UdtInfoPrivate*)this)->find_function(name);
	}

	inline FunctionInfo* UdtInfoBridge::add_function(const char* name, uint rva, uint voff, TypeInfo* ti, int idx)
	{
		return ((UdtInfoPrivate*)this)->add_function(name, rva, voff, (TypeInfoPrivate*)ti, idx);
	}

	inline void UdtInfoBridge::remove_function(FunctionInfo* vi)
	{
		((UdtInfoPrivate*)this)->remove_function((FunctionInfoPrivate*)vi);
	}

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
	EnumInfoPrivate* add_enum(const std::string& name, LibraryPrivate* library, TypeInfoDataBasePrivate* db = nullptr);
	std::vector<EnumInfoPrivate*> get_enums(TypeInfoDataBasePrivate* db = nullptr);

	UdtInfoPrivate* find_udt(const std::string& name, TypeInfoDataBasePrivate* db = nullptr);
	UdtInfoPrivate* add_udt(const std::string& name, uint size, const std::string& base_name, LibraryPrivate* library, TypeInfoDataBasePrivate* db = nullptr);
	std::vector<UdtInfoPrivate*> get_udts(TypeInfoDataBasePrivate* db = nullptr);

	void load_typeinfo(const std::filesystem::path& filename, LibraryPrivate* library, TypeInfoDataBasePrivate* db = nullptr);
	void save_typeinfo(const std::filesystem::path& filename, TypeInfoDataBasePrivate* db = nullptr);

	struct LibraryPrivate : Library
	{
		char* address;
		std::filesystem::path filename;
		bool has_typeinfo = false;
		int ref_count = 1;

		LibraryPrivate(const std::filesystem::path& filename, bool require_typeinfo);
		~LibraryPrivate();

		void* _get_exported_function(const char* name);

		void release() override;

		char* get_address() const override { return address; }
		const wchar_t* get_filename() const override { return filename.c_str(); }
		void* get_exported_function(const char* name) override { return _get_exported_function(name); }

		static LibraryPrivate* load(const std::filesystem::path& filename, bool require_typeinfo = true);
	};
}
