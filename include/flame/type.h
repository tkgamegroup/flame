#pragma once

#ifdef FLAME_WINDOWS
#ifdef FLAME_TYPE_MODULE
#define FLAME_TYPE_EXPORTS __declspec(dllexport)
#else
#define FLAME_TYPE_EXPORTS __declspec(dllimport)
#endif
#else
#define FLAME_TYPE_EXPORTS
#endif

#include <assert.h>
#include <string>
#include <vector>
#include <stack>
#include <list>
#include <map>
#include <unordered_map>
#include <memory>

namespace flame
{
	typedef char*				charptr;
	typedef wchar_t*			wcharptr;
	typedef unsigned char		uchar;
	typedef unsigned short		ushort;
	typedef unsigned int		uint;
	typedef unsigned long		ulong;
	typedef long long			longlong;
	typedef unsigned long long  ulonglong;
	typedef void*				voidptr;

	const auto INVALID_POINTER = (void*)0x7fffffffffffffff;

	template<class T>
	constexpr uint array_size(const T& a)
	{
		return sizeof(a) / sizeof(a[0]);
	}

	template <uint N>
	struct EnsureConstU
	{
		static const uint value = N;
	};

	template <class CH>
	struct _SAL // str and len
	{
		uint l;
		const CH* s;

		_SAL(uint l, const CH* s) :
			l(l),
			s(s)
		{
		}
	};

#define FLAME_SAL_S(x) EnsureConstU<__f_strlen(x)>::value, x
#define FLAME_SAL(n, x) _SAL n(FLAME_SAL_S(x))

	template<class CH>
	constexpr uint __f_strlen(const CH* str)
	{
		auto p = str;
		while (*p)
			p++;
		return p - str;
	}

	inline constexpr uint hash_update(uint h, uint v)
	{
		return h ^ (v + 0x9e3779b9 + (h << 6) + (h >> 2));
	}

	template<class CH>
	constexpr uint hash_str(const CH* str, uint seed)
	{
		return 0 == *str ? seed : hash_str(str + 1, hash_update(seed, *str));
	}

#define FLAME_HASH(x) (hash_str(x, 0))

#define FLAME_CHASH(x) (EnsureConstU<hash_str(x, 0)>::value)

	FLAME_TYPE_EXPORTS void* f_malloc(uint size);
	FLAME_TYPE_EXPORTS void* f_realloc(void* p, uint size);
	FLAME_TYPE_EXPORTS void f_free(void* p);

	FLAME_TYPE_EXPORTS void* load_module(const wchar_t* module_name);
	FLAME_TYPE_EXPORTS void* get_module_func(void* module, const char* name);
	FLAME_TYPE_EXPORTS void free_module(void* library);

	template<class F>
	void* f2v(F f) // function to void pointer
	{
		union
		{
			F f;
			void* p;
		}cvt;
		cvt.f = f;
		return cvt.p;
	}

	template<class F>
	F p2f(void* p) // void pointer to function
	{
		union
		{
			void* p;
			F f;
		}cvt;
		cvt.p = p;
		return cvt.f;
	}

	template<class F, class ...Args>
	auto cf(F f, Args... args) // call function
	{
		return (*f)(args...);
	}

	struct __f_Dummy
	{
	};
	typedef void(__f_Dummy::* MF_v_v)();
	typedef void(__f_Dummy::* MF_v_vp)(void*);
	typedef void(__f_Dummy::* MF_v_vp_u)(void*, uint);
	typedef void* (__f_Dummy::* MF_vp_v)();
	typedef void* (__f_Dummy::* MF_vp_vp)(void*);
	typedef bool(__f_Dummy::* MF_b_v)();

	template<class F, class ...Args>
	auto cmf(F f, void* p, Args... args) // call member function at an address
	{
		return (*((__f_Dummy*)p).*f)(args...);
	}

	template<class T>
	typename std::enable_if<std::is_pod<T>::value, void*>::type cf2v() // ctor function to void pointer
	{
		return nullptr;
	}

	template<class T>
	typename std::enable_if<!std::is_pod<T>::value, void*>::type cf2v() // ctor function to void pointer
	{
		struct Wrap : T
		{
			void ctor()
			{
				new(this) T;
			}
		};
		return f2v(&Wrap::ctor);
	}

	template<class T>
	typename std::enable_if<std::is_pod<T>::value, void*>::type df2v() // dtor function to void pointer
	{
		return nullptr;
	}

	template<class T>
	typename std::enable_if<!std::is_pod<T>::value, void*>::type df2v() // dtor function to void pointer
	{
		struct Wrap : T
		{
			void dtor()
			{
				(*this).~Wrap();
			}
		};
		return f2v(&Wrap::dtor);
	}

	inline bool not_null_equal(void* a, void* b)
	{
		return a && a == b;
	}

	template<class T>
	std::pair<T*, uchar> only_not_null(T* a, T* b)
	{
		if (a && !b)
			return std::make_pair(a, 0);
		if (!a && b)
			return std::make_pair(b, 1);
		return std::make_pair(nullptr, 0);
	}

	template<class T>
	bool is_one_of(T v, const std::initializer_list<T>& l)
	{
		for (auto& i : l)
		{
			if (v == i)
				return true;
		}
		return false;
	}

	template<class CH>
	struct String
	{
		uint s;
		CH* v;

		String() :
			s(0),
			v(nullptr)
		{
		}

		String(const String& rhs)
		{
			s = rhs.s;
			v = (CH*)f_malloc(sizeof(CH) * (s + 1));
			memcpy(v, rhs.v, sizeof(CH) * s);
			v[s] = 0;
		}

		String(String&& rhs)
		{
			s = rhs.s;
			v = rhs.v;
			rhs.s = 0;
			rhs.v = nullptr;
		}

		String(const CH* str, uint _s)
		{
			s = _s;
			v = (CH*)f_malloc(sizeof(CH) * (s + 1));
			memcpy(v, str, sizeof(CH) * s);
			v[s] = 0;
		}

		String(const CH* str) :
			String(str, std::char_traits<CH>::length(str))
		{
		}

		String(const std::basic_string<CH>& str) :
			String(str.data(), str.size())
		{
		}

		~String()
		{
			f_free(v);
		}

		void resize(uint _s)
		{
			if (s != _s)
			{
				s = _s;
				if (s > 0)
				{
					v = (CH*)f_realloc(v, sizeof(CH) * (s + 1));
					v[s] = 0;
				}
			}
		}

		void assign(const CH* _v, uint _s)
		{
			resize(_s);
			memcpy(v, _v, sizeof(CH) * s);
		}

		void operator=(const String& rhs)
		{
			assign(rhs.v, rhs.s);
		}

		void operator=(String&& rhs)
		{
			f_free(v);
			s = rhs.s;
			v = rhs.v;
			rhs.s = 0;
			rhs.v = nullptr;
		}

		void operator=(const CH* str)
		{
			assign(str, std::char_traits<CH>::length(str));
		}

		void operator=(const std::basic_string<CH>& str)
		{
			assign(str.c_str(), str.size());
		}

		std::basic_string<CH> str()
		{
			return std::basic_string<CH>(v, s);
		}
	};

	using StringA = String<char>;
	using StringW = String<wchar_t>;

	template<class T>
	struct Array
	{
		uint s;
		T* v;

		Array() :
			s(0),
			v(nullptr)
		{
		}

		Array(const Array& rhs)
		{
			s = rhs.s;
			v = (T*)f_malloc(sizeof(T) * s);
			for (auto i = 0; i < s; i++)
			{
				new (&v[i])T;
				v[i] = rhs.v[i];
			}
		}

		Array(Array&& rhs)
		{
			s = rhs.s;
			v = rhs.v;
			rhs.s = 0;
			rhs.v = nullptr;
		}

		~Array()
		{
			for (auto i = 0; i < s; i++)
				v[i].~T();
			f_free(v);
		}

		void resize(uint _s)
		{
			if (s != _s)
			{
				for (auto i = 0; i < s; i++)
					v[i].~T();
				s = _s;
				if (s > 0)
				{
					v = (T*)f_realloc(v, sizeof(T) * s);
					for (auto i = 0; i < s; i++)
						new (&v[i])T;
				}
				else
				{
					f_free(v);
					v = nullptr;
				}
			}
		}

		const T& at(uint idx) const
		{
			return v[idx];
		}

		T& at(uint idx)
		{
			return v[idx];
		}

		const T& operator[](uint idx) const
		{
			return v[idx];
		}

		T& operator[](uint idx)
		{
			return v[idx];
		}

		void operator=(const Array& rhs)
		{
			resize(rhs.s);
			for (auto i = 0; i < s; i++)
				v[i] = rhs.v[i];
		}

		void operator=(Array&& rhs)
		{
			for (auto i = 0; i < s; i++)
				v[i].~T();
			f_free(v);
			s = rhs.s;
			v = rhs.v;
			rhs.s = 0;
			rhs.v = nullptr;
		}

		void push_back(const T& _v)
		{
			resize(s + 1);
			v[s - 1] = _v;
		}
	};

#pragma pack(1)

	struct AttributeBase
	{
		uint satisfied : 1;
		int frame : 31;
	};

	template<class T>
	struct AttributeE : AttributeBase // enum type attribute
	{
		T v;
	};

	template<class T>
	struct AttributeD : AttributeBase // data type attribute
	{
		T v;
	};

	template<class T>
	struct AttributeP : AttributeBase // pointer type attribute
	{
		T* v;
	};

#pragma pack()

	struct Object
	{
		const char* name;
		const uint name_hash;
		uint id;

		Object(const char* name) :
			name(name),
			name_hash(FLAME_HASH(name)),
			id(0)
		{
		}
	};

	enum TypeTag$
	{
		TypeEnumSingle,
		TypeEnumMulti,
		TypeData,
		TypePointer,

		TypeTagCount
	};

	inline char type_tag(TypeTag$ tag)
	{
		static char names[] = {
			'S',
			'M',
			'D',
			'P'
		};
		return names[tag];
	}

	struct EnumInfo;
	struct VariableInfo;
	struct FunctionInfo;
	struct UdtInfo;
	struct TypeinfoDatabase;

	typedef EnumInfo* EnumInfoPtr;
	typedef VariableInfo* VariableInfoPtr;
	typedef FunctionInfo* FunctionInfoPtr;
	typedef UdtInfo* UdtInfoPtr;
	typedef TypeinfoDatabase* TypeinfoDatabasePtr;

	// type name archive:
	// ， no space
	// ， 'unsigned ' will be replaced to 'u'
	// ， '< ' will be replaced to '('
	// ， '> ' will be replaced to ')'
	// ， ', ' will be replaced to '+'

	inline std::string tn_c2a(const std::string& name) // type name code to archive
	{
		auto ret = name;
		for (auto& ch : ret)
		{
			if (ch == '<')
				ch = '(';
			else if (ch == '>')
				ch = ')';
			else if (ch == ',')
				ch = '+';
		}
		return ret;
	}

	inline std::string tn_a2c(const std::string& name) // type name archive to code
	{
		auto ret = name;
		for (auto& ch : ret)
		{
			if (ch == '(')
				ch = '<';
			else if (ch == ')')
				ch = '>';
			else if (ch == '+')
				ch = ',';
		}
		return ret;
	}

	struct TypeInfo
	{
		FLAME_TYPE_EXPORTS TypeTag$ tag() const;
		FLAME_TYPE_EXPORTS bool is_attribute() const;
		FLAME_TYPE_EXPORTS bool is_array() const;
		FLAME_TYPE_EXPORTS const char* base_name() const;
		FLAME_TYPE_EXPORTS const char* name() const; // tag[A][V]#base, order matters
		FLAME_TYPE_EXPORTS uint base_hash() const;
		FLAME_TYPE_EXPORTS uint hash() const;

		inline static std::string make_str(TypeTag$ tag, const std::string& base_name, bool is_attribute = false, bool is_array = false)
		{
			std::string ret;
			ret = type_tag(tag);
			if (is_attribute)
				ret += "A";
			if (is_array)
				ret += "V";
			ret += "#" + base_name;
			return ret;
		}

		inline static void break_str(const std::string& str, TypeTag$& tag, std::string& base_name, bool& is_attribute, bool& is_array)
		{
			auto pos_hash = str.find('#');
			{
				auto ch = str[0];
				for (auto i = 0; i < TypeTagCount; i++)
				{
					if (type_tag((TypeTag$)i) == ch)
					{
						tag = (TypeTag$)i;
						break;
					}
				}
			}
			is_attribute = false;
			is_array = false;
			for (auto i = 1; i < pos_hash; i++)
			{
				auto ch = str[i];
				if (ch == 'A')
					is_attribute = true;
				else if (ch == 'V')
					is_array = true;
			}
			base_name = std::string(str.begin() + pos_hash + 1, str.end());
		}

		inline static uint get_hash(TypeTag$ tag, const std::string& base_name, bool is_attribute = false, bool is_array = false)
		{
			return FLAME_HASH(make_str(tag, base_name, is_attribute, is_array).c_str());
		}

		inline static uint get_hash(const std::string& str)
		{
			TypeTag$ tag;
			std::string base_name;
			bool is_attribute;
			bool is_array;
			break_str(str, tag, base_name, is_attribute, is_array);
			return TypeInfo::get_hash(tag, base_name, is_attribute, is_array);
		}

		FLAME_TYPE_EXPORTS static const TypeInfo* get(TypeTag$ tag, const char* base_name, bool is_attribute = false, bool is_array = false);
		FLAME_TYPE_EXPORTS static const TypeInfo* get(const char* str);

		inline std::string serialize(const void* src, int precision) const;
		inline void unserialize(const std::string& src, void* dst) const;
		inline void copy_from(const void* src, void* dst) const;
	};

	struct VariableInfo
	{
		FLAME_TYPE_EXPORTS const TypeInfo* type() const;
		FLAME_TYPE_EXPORTS const char* name() const;
		FLAME_TYPE_EXPORTS uint name_hash() const;
		FLAME_TYPE_EXPORTS const char* decoration() const;
		FLAME_TYPE_EXPORTS uint offset() const;
		FLAME_TYPE_EXPORTS uint size() const;
		FLAME_TYPE_EXPORTS const char* default_value() const;
	};

	struct EnumItem
	{
		FLAME_TYPE_EXPORTS const char* name() const;
		FLAME_TYPE_EXPORTS int value() const;
	};

	struct EnumInfo
	{
		FLAME_TYPE_EXPORTS TypeinfoDatabase* db() const;

		FLAME_TYPE_EXPORTS const char* name() const;

		FLAME_TYPE_EXPORTS uint item_count() const;
		FLAME_TYPE_EXPORTS EnumItem* item(int idx) const;
		FLAME_TYPE_EXPORTS EnumItem* find_item(const char* name, int* out_idx = nullptr) const;
		FLAME_TYPE_EXPORTS EnumItem* find_item(int value, int* out_idx = nullptr) const;
		FLAME_TYPE_EXPORTS EnumItem* add_item(const char* name, int value);
	};

	struct FunctionInfo
	{
		FLAME_TYPE_EXPORTS TypeinfoDatabase* db() const;

		FLAME_TYPE_EXPORTS const char* name() const;
		FLAME_TYPE_EXPORTS void* rva() const;
		FLAME_TYPE_EXPORTS const TypeInfo* return_type() const;

		FLAME_TYPE_EXPORTS uint parameter_count() const;
		FLAME_TYPE_EXPORTS const TypeInfo* parameter_type(uint idx) const;
		FLAME_TYPE_EXPORTS void add_parameter(const TypeInfo* type);

	};

	struct UdtInfo
	{
		FLAME_TYPE_EXPORTS TypeinfoDatabase* db() const;

		FLAME_TYPE_EXPORTS const TypeInfo* type() const;
		FLAME_TYPE_EXPORTS uint size() const;

		FLAME_TYPE_EXPORTS uint variable_count() const;
		FLAME_TYPE_EXPORTS VariableInfo* variable(uint idx) const;
		FLAME_TYPE_EXPORTS VariableInfo* find_variable(const char* name, int* out_idx = nullptr) const;
		FLAME_TYPE_EXPORTS VariableInfo* add_variable(const TypeInfo* type, const char* name, const char* decoration, uint offset, uint size);

		FLAME_TYPE_EXPORTS uint function_count() const;
		FLAME_TYPE_EXPORTS FunctionInfo* function(uint idx) const;
		FLAME_TYPE_EXPORTS FunctionInfo* find_function(const char* name, int* out_idx = nullptr) const;
		FLAME_TYPE_EXPORTS FunctionInfo* add_function(const char* name, void* rva, const TypeInfo* return_type);
	};

	/*
		something end with '$[a]' means it is reflectable
		the 'a' is called decoration, and is optional
		if first char of member name is '_', then the '_' will be ignored in reflection

		such as:
			struct Apple$ // mark this will be collected by typeinfogen
			{
				float size$; // mark this member will be collected
				Vec3f color$i; // mark this member will be collected, and its decoration is 'i'
				float _1$i; // mark this member will be collected, and reflected name is '1'
			};

		the decoration can be one or more chars, and order doesn't matter

		currently, the following attributes are used by typeinfogen, others are free to use:
			'm' for enum variable, means it can hold combination of the enum
			'c' for function, means to collect the code of the function
	*/

	struct TypeinfoDatabase
	{
		FLAME_TYPE_EXPORTS void* module() const;
		FLAME_TYPE_EXPORTS const wchar_t* module_name() const;

		FLAME_TYPE_EXPORTS Array<EnumInfo*> get_enums();
		FLAME_TYPE_EXPORTS EnumInfo* find_enum(uint name_hash);
		FLAME_TYPE_EXPORTS EnumInfo* add_enum(const char* name);

		FLAME_TYPE_EXPORTS Array<FunctionInfo*> get_functions();
		FLAME_TYPE_EXPORTS FunctionInfo* find_function(uint name_hash);
		FLAME_TYPE_EXPORTS FunctionInfo* add_function(const char* name, void* rva, const TypeInfo* return_type);

		FLAME_TYPE_EXPORTS Array<UdtInfo*> get_udts();
		FLAME_TYPE_EXPORTS UdtInfo* find_udt(uint name_hash);
		FLAME_TYPE_EXPORTS UdtInfo* add_udt(const TypeInfo* type, uint size);

		FLAME_TYPE_EXPORTS static void collect(const wchar_t* module_filename, const wchar_t* pdb_filename = nullptr);
		FLAME_TYPE_EXPORTS static TypeinfoDatabase* load(const wchar_t* typeinfo_filename, bool add_to_global, bool load_with_module);
		FLAME_TYPE_EXPORTS static void destroy(TypeinfoDatabase* db);
	};

	FLAME_TYPE_EXPORTS uint global_db_count();
	FLAME_TYPE_EXPORTS TypeinfoDatabase* global_db(uint idx);
	FLAME_TYPE_EXPORTS extern uint extra_global_db_count;
	FLAME_TYPE_EXPORTS extern TypeinfoDatabase* const* extra_global_dbs;

	inline EnumInfo* find_enum(uint name_hash)
	{
		for (auto i = 0; i < global_db_count(); i++)
		{
			auto db = global_db(i);
			auto info = db->find_enum(name_hash);
			if (info)
				return info;
		}
		for (auto i = 0; i < extra_global_db_count; i++)
		{
			auto db = extra_global_dbs[i];
			auto info = db->find_enum(name_hash);
			if (info)
				return info;
		}
		return nullptr;
	}

	inline UdtInfo* find_udt(uint name_hash)
	{
		for (auto i = 0; i < global_db_count(); i++)
		{
			auto db = global_db(i);
			auto info = db->find_udt(name_hash);
			if (info)
				return info;
		}
		for (auto i = 0; i < extra_global_db_count; i++)
		{
			auto db = extra_global_dbs[i];
			auto info = db->find_udt(name_hash);
			if (info)
				return info;
		}
		return nullptr;
	}

	inline FunctionInfo* find_function(uint name_hash)
	{
		for (auto i = 0; i < global_db_count(); i++)
		{
			auto db = global_db(i);
			auto info = db->find_function(name_hash);
			if (info)
				return info;
		}
		for (auto i = 0; i < extra_global_db_count; i++)
		{
			auto db = extra_global_dbs[i];
			auto info = db->find_function(name_hash);
			if (info)
				return info;
		}
		return nullptr;
	}

	inline bool check_function(FunctionInfo* info, const char* return_type, const std::vector<const char*>& parameter_types)
	{
		TypeTag$ tag;
		std::string base_name;
		bool is_attribute;
		bool is_array;
		TypeInfo::break_str(return_type, tag, base_name, is_attribute, is_array);
		if (info->return_type()->hash() != TypeInfo::get_hash(tag, base_name, is_attribute, is_array) || 
			info->parameter_count() != parameter_types.size())
			return false;
		for (auto i = 0; i < parameter_types.size(); i++)
		{
			TypeInfo::break_str(parameter_types[i], tag, base_name, is_attribute, is_array);
			if (info->parameter_type(i)->hash() != TypeInfo::get_hash(tag, base_name, is_attribute, is_array))
				return false;
		}
		return true;
	}
}
