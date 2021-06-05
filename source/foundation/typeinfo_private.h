#pragma once

#include "typeinfo.h"

namespace flame
{
	struct TypeInfoPrivate : TypeInfo
	{
		TypeTag tag;
		std::string name;
		std::string full_name;
		std::string short_name;
		uint size;

		BasicType basic_type = ElseType;
		bool is_signed = true;
		uint vec_size = 1;
		uint col_size = 1;
		TypeInfoPrivate* pointed_type = nullptr;

		TypeInfoPrivate(TypeTag tag, const std::string& base_name, uint size);

		TypeTag get_tag() const override { return tag; }
		const char* get_name() const override { return name.c_str(); }
		const char* get_code_name() const override { return name.c_str(); }
		const char* get_full_name() const override { return full_name.c_str(); }
		uint get_size() const override { return size; }

		BasicType get_basic() const override { return basic_type; }
		bool get_signed() const override { return is_signed; }
		uint get_vec_size() const override { return vec_size; }
		uint get_col_size() const override { return col_size; }
		TypeInfoPtr get_pointed_type() const override { return pointed_type; }

		static TypeInfoPrivate* get(TypeTag tag, const std::string& name, TypeInfoDataBasePrivate* db = nullptr);
	};

	struct Metas
	{
		std::vector<std::pair<TypeMeta, LightCommonValue>> d;

		inline void from_string(const std::string& str)
		{
			auto e_meta = TypeInfoPrivate::get(TypeEnumSingle, "flame::TypeMeta");
			for (auto& i : SUS::split(str, ';'))
			{
				auto sp = SUS::split(i, ':');
				auto& m = d.emplace_back();
				e_meta->unserialize(&m.first, sp[0].c_str());
				m.second.u = std::stoul(sp[1], 0, 16);;
			}
		}

		inline std::string to_string() const
		{
			std::string ret;
			auto e_meta = TypeInfoPrivate::get(TypeEnumSingle, "flame::TypeMeta");
			for (auto& i : d)
				ret += e_meta->serialize(&i.first) + ":" + to_hex_string(i.second.u, false) + ";";
			return ret;
		}

		inline bool get_meta(TypeMeta m, LightCommonValue* v) const
		{
			for (auto& i : d)
			{
				if (i.first == m)
				{
					if (v)
						*v = i.second;
					return true;
				}
			}
			return false;
		}
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
		void* default_value = nullptr;
		Metas metas;

		VariableInfoPrivate(UdtInfoPrivate* udt, uint index, TypeInfoPrivate* type, const std::string& name, uint offset, 
			uint array_size, uint array_stride, void* default_value, const std::string& metas);
		~VariableInfoPrivate();

		UdtInfoPtr get_udt() const override { return udt; }
		uint get_index() const override { return index; }
		TypeInfoPtr get_type() const override { return type; }
		const char* get_name() const override { return name.c_str(); }
		uint get_offset() const override { return offset; }
		uint get_array_size() const override { return array_size; }
		uint get_array_stride() const override { return array_stride; }
		void* get_default_value() const override { return default_value; }
		bool get_meta(TypeMeta m, LightCommonValue* v) const override;
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
		std::string full_name;
		uint rva;
		int voff;
		bool is_static;
		TypeInfoPrivate* type;
		std::string code;
		Metas metas;
		std::vector<TypeInfoPrivate*> parameters;

		void(*caller)(void*, void*, void*, void**) = nullptr;

		FunctionInfoPrivate(UdtInfoPrivate* udt, void* library, uint index, const std::string& name, uint rva, int voff, 
			bool is_static, TypeInfoPrivate* type, const std::string& metas);

		void* get_library() const override { return library; }
		UdtInfoPtr get_udt() const override { return udt; }
		uint get_index() const override { return index; }
		const char* get_name() const override { return name.c_str(); }
		void update_full_name();
		const char* get_full_name() override;
		uint get_rva() const override { return rva; }
		int get_voff() const override { return voff; }
		bool get_is_static() const override { return is_static; }
		TypeInfoPtr get_type() const override { return type; }
		const char* get_code() const override { return code.c_str(); }
		bool get_meta(TypeMeta m, LightCommonValue* v) const override;

		uint get_parameters_count() const override { return parameters.size(); }
		TypeInfoPtr get_parameter(uint idx) const override { return parameters[idx]; }
		void add_parameter(TypeInfoPtr ti, int idx) override;
		void remove_parameter(uint idx) override;

		bool check(TypeInfoPtr ret, uint parms_count = 0, TypeInfoPtr const* parms = nullptr) const override;

		void* get_address(void* obj) const override;
		void call(void* obj, void* ret, void** parameters) override;
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
		VariableInfoPtr add_variable(TypeInfoPtr ti, const std::string& name, uint offset, uint array_size, uint array_stride, 
			void* default_value, const std::string& metas, int idx = -1);
		VariableInfoPtr add_variable(TypeInfoPtr ti, const char* name, uint offset, uint array_size, uint array_stride, 
			void* default_value, const char* metas, int idx) override 
			{ return add_variable(ti, std::string(name), offset, array_size, array_stride, default_value, std::string(metas), idx); }
		void remove_variable(VariableInfoPtr vi) override;

		uint get_functions_count() const override { return functions.size(); }
		FunctionInfoPtr get_function(uint idx) const override { return functions[idx].get(); }
		FunctionInfoPtr find_function(const std::string& name) const;
		FunctionInfoPtr find_function(const char* name) const override { return find_function(std::string(name)); }
		FunctionInfoPtr add_function(const std::string& name, uint rva, int voff, bool is_static, TypeInfoPtr ti, const std::string& metas, int idx = -1);
		FunctionInfoPtr add_function(const char* name, uint rva, int voff, bool is_static, TypeInfoPtr ti, const char* metas, int idx) override
			{ return add_function(std::string(name), rva, voff, is_static, ti, std::string(metas), idx); }
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
