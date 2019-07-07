#pragma once

#include <flame/foundation/foundation.h>
#include <flame/foundation/serialize.h>

namespace flame
{
	void* calc_rva(void* p, void* module)
	{
		return (void*)((char*)p - module);
	}

	template<uint N, class T>
	struct BP_Vec
	{
		AttributeV<T> in[N];

		AttributeV<Vec<N, T>> out;

		void update()
		{
			for (auto i = 0; i < N; i++)
			{
				if (in[i].frame > out.frame)
				{
					out.v.v_[i] = in[i].v;
					out.frame = in[i].frame;
				}
			}
		}

		static void add_udt_info(int level, const std::string& template_parameters, void* module)
		{
			auto u = add_udt(level, "Vec~" + template_parameters, sizeof(BP_Vec), L"bp.dll");

			for (auto i = 0; i < N; i++)
				u->add_variable(TypeTagAttributeV, string_split(template_parameters, '~')[1], std::string(1, "xyzw"[i]), "i", sizeof(AttributeV<T>) * i, sizeof(AttributeV<T>));
			u->add_variable(TypeTagAttributeV, "Vec~" + template_parameters, "v", "o", offsetof(BP_Vec, out), sizeof(BP_Vec::out));

			u->add_function("update", calc_rva(f2v(&BP_Vec::update), module), TypeTagVariable, "void", "");
		}
	};

	template<uint N, class T>
	struct BP_Array
	{
		AttributeV<T> in[N];

		AttributeV<std::vector<T>> out;

		void update()
		{
			auto last_out_frame = out.frame;
			out.v.resize(N);
			for (auto i = 0; i < N; i++)
			{
				if (in[i].frame > last_out_frame)
				{
					out.v[i] = in[i].v;
					out.frame = max(out.frame, in[i].frame);
				}
			}
		}

		static void add_udt_info(int level, const std::string& template_parameters, void* module)
		{
			auto sp = string_split(template_parameters, '~');
			{
				static std::regex reg_token(R"([\w\s\_\*]+)");
				auto tokens = string_regex_split(sp[1], reg_token);
				sp[1] = tokens[0];
				for (auto i = 1; i < tokens.size(); i++)
				{
					auto& t = tokens[i];
					t.erase(std::remove(t.begin(), t.end(), ' '), t.end());
					if (!t.empty())
						sp[1] += "~" + t;
				}
			}

			auto u = add_udt(level, "Array~" + sp[0] + "~" + sp[1], sizeof(BP_Array), L"bp.dll");

			auto tag = TypeTagAttributeV;
			auto in_type_name = sp[1];
			if (sp[1].back() == '*')
			{
				tag = TypeTagAttributeP;
				in_type_name.resize(in_type_name.size() - 1);
			}
			for (auto i = 0; i < N; i++)
				u->add_variable(tag, in_type_name, std::to_string(i + 1), "i", sizeof(AttributeV<T>) * i, sizeof(AttributeV<T>));
			u->add_variable(TypeTagAttributeV, "std::vector~" + sp[1], "v", "o", offsetof(BP_Array, out), sizeof(BP_Array::out));

			u->add_function("ctor", calc_rva(cf2v<BP_Array>(), module), TypeTagVariable, "void", "");
			u->add_function("dtor", calc_rva(df2v<BP_Array>(), module), TypeTagVariable, "void", "");
			u->add_function("update", calc_rva(f2v(&BP_Array::update), module), TypeTagVariable, "void", "");
		}
	};
}
