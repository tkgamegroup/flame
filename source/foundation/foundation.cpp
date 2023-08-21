#include "foundation_private.h"
#include "typeinfo_private.h"
#include "typeinfo_serialize.h"
#include "system_private.h"
#include "window_private.h"
#include "blueprint_private.h"
#include "blueprint_library/library.h"
#include "application.h"

#include <exprtk.hpp>

namespace flame
{
	std::map<std::wstring, std::filesystem::path> Path::roots;
	std::map<std::filesystem::path, AssetManagemant::Asset> AssetManagemant::assets;

	AssetManagemant::Asset& AssetManagemant::get(const std::filesystem::path& path)
	{
		auto it = assets.find(path);
		if (it == assets.end())
			it = assets.emplace(std::make_pair(path, Asset())).first; 
		it->second.lwt = std::filesystem::exists(path) ? std::filesystem::last_write_time(path) : std::filesystem::file_time_type::min();
		it->second.ref++;
		return it->second;
	}

	void AssetManagemant::release(const std::filesystem::path& path) 
	{
		auto it = assets.find(path);
		if (it != assets.end())
		{
			if (it->second.ref == 1)
				assets.erase(it);
			else  
				it->second.ref--;
		}
	}

	struct exprtk_output_t : public exprtk::igeneric_function<float>
	{
		std::string* return_value;

		exprtk_output_t()
		{
			exprtk::enable_zero_parameters(*this);
		}

		inline float operator() (parameter_list_t parameters)
		{
			*return_value = "";
			for (auto i = 0; i < parameters.size(); ++i)
			{
				generic_type& gt = parameters[i];

				switch (gt.type)
				{
				case generic_type::e_scalar: 
					*return_value += str(generic_type::scalar_view(gt)());
					break;
				case generic_type::e_vector: 

					break;
				case generic_type::e_string: 
					*return_value += exprtk::to_str(generic_type::string_view(gt));
					break;
				}
			}

			return float(0);
		}
	};

	struct exprtk_to_str_t : public exprtk::igeneric_function<float>
	{
		exprtk_to_str_t()
			: igeneric_function<float>("S", e_rtrn_string)
		{
		}

		inline float operator()(std::string& result, parameter_list_t parameters)
		{
			result = str(generic_type::scalar_view(parameters[0])());

			return float(0);
		}
	}exprtk_to_str;

	struct ExpressionPrivate : Expression
	{
		struct Convertion
		{
			float v;
			DataType type;
			void* p;
		};

		exprtk::symbol_table<float> symbols;
		exprtk::expression<float> expression;
		std::string return_value;
		exprtk_output_t output_function;
		std::list<Convertion> convertions;

		ExpressionPrivate(const std::string& expression_string);

		void set_const_value(const std::string& name, float value) override;
		void set_variable(const std::string& name, float* variable) override;
		void set_const_string(const std::string& name, const std::string& value) override;
		bool compile() override;
		bool update_bindings() override;
		float get_value() override;
		std::string get_string_value() override;
	};

	ExpressionPrivate::ExpressionPrivate(const std::string& _expression_string)
	{
		expression_string = _expression_string;

		auto addr_str = str((uint64)&return_value);
		symbols.create_stringvar("return_value", addr_str);
		output_function.return_value = &return_value;
		symbols.add_function("output", output_function);
		symbols.add_function("to_str", exprtk_to_str);
	}

	void ExpressionPrivate::set_const_value(const std::string& name, float value)
	{
		symbols.add_constant(name, value);
	}

	void ExpressionPrivate::set_variable(const std::string& name, float* variable)
	{
		symbols.add_variable(name, *variable);
	}

	bool ExpressionPrivate::compile()
	{
		exprtk::symbol_table<float> unknown_symbols;
		expression.register_symbol_table(unknown_symbols);
		expression.register_symbol_table(symbols);

		exprtk::parser<float> parser;
		parser.enable_unknown_symbol_resolver();
		auto ok = parser.compile(expression_string, expression);
		if (ok)
		{
			std::vector<std::string> variable_list;
			unknown_symbols.get_variable_list(variable_list);
			unknown_symbols.clear();
			for (auto& var_name : variable_list)
			{
				auto chain = SUS::split(var_name, '.');
				auto di = find_data(sh(chain[0].data()));
				if (di)
				{
					auto bind_data = [&](void* address, TypeInfo_Data* ti) {
						if (ti->data_type == DataFloat)
						{
							auto& c = convertions.emplace_back();
							symbols.add_variable(var_name, c.v);
							c.type = ti->data_type;
							c.p = address;
						}
					};

					switch (di->type->tag)
					{
					case TagD:
					{
						auto ti = (TypeInfo_Data*)di->type;
						bind_data(di->address(), ti);
					}
						break;
					case TagU:
					{
						if (auto ui = di->type->retrive_ui(); ui)
						{
							chain.erase(chain.begin());
							voidptr obj = di->address();
							if (auto attr = ui->find_attribute(chain, obj); attr)
							{
								if (attr->type->tag == TagD)
								{
									auto ti = (TypeInfo_Data*)attr->type;
									bind_data((char*)obj + attr->var_off(), ti);
								}
							}
						}
					}
						break;
					}
				}

			}

			ok = parser.compile(expression_string, expression);
		}
		return ok;
	}

	void ExpressionPrivate::set_const_string(const std::string& name, const std::string& value)
	{
		symbols.create_stringvar(name, value);
	}

	bool ExpressionPrivate::update_bindings()
	{
		auto changed = false;
		for (auto& c : convertions)
		{
			switch (c.type)
			{
			case DataInt:
			{
				auto v = (float)*(int*)c.p;
				if (c.v != v)
				{
					c.v = v;
					changed = true;
				}
			}
				break;
			case DataFloat:
			{
				auto v = *(float*)c.p;
				if (c.v != v)
				{
					c.v = *(float*)c.p;
					changed = true;
				}
			}
				break;
			}
		}
		return changed;
	}

	float ExpressionPrivate::get_value()
	{
		return expression.value();
	}

	std::string ExpressionPrivate::get_string_value()
	{
		return_value.clear();
		auto float_ret = expression.value();
		if (!return_value.empty())
			return return_value;
		return str(float_ret);
	}

	Expression* Expression::create(const std::string& expression_string)
	{
		return new ExpressionPrivate(expression_string);
	}

	void* load_preset_file(const std::filesystem::path& _filename, void* obj, UdtInfo** out_ui)
	{
		pugi::xml_document doc;
		pugi::xml_node doc_root;

		auto filename = Path::get(_filename);
		if (!std::filesystem::exists(filename))
		{
			wprintf(L"preset does not exist: %s\n", _filename.c_str());
			return nullptr;
		}
		if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("preset"))
		{
			wprintf(L"preset is wrong format: %s\n", _filename.c_str());
			return nullptr;
		}

		UdtInfo* ui = nullptr;
		if (auto a = doc_root.attribute("type"); a)
			ui = find_udt(sh(a.value()));

		if (!ui)
		{
			wprintf(L"preset type is not found: %s\n", _filename.c_str());
			return nullptr;
		}

		if (!obj)
			obj = ui->create_object();
		if (out_ui)
			*out_ui = ui;

		auto base_path = Path::reverse(filename).parent_path();
		UnserializeXmlSpec spec;
		spec.typed_delegates[TypeInfo::get<std::filesystem::path>()] = [&](const std::string& str, void* dst) {
			*(std::filesystem::path*)dst = Path::combine(base_path, str);
		};

		unserialize_xml(*ui, doc_root, obj, spec);

		return obj;
	}

	void save_preset_file(const std::filesystem::path& _filename, void* obj, UdtInfo* ui)
	{
		pugi::xml_document doc;
		pugi::xml_node doc_root = doc.append_child("preset");
		doc_root.append_attribute("type").set_value(ui->name.c_str());

		auto filename = Path::get(_filename);

		SerializeXmlSpec spec;
		auto base_path = Path::reverse(filename).parent_path();
		spec.typed_delegates[TypeInfo::get<std::filesystem::path>()] = [&](void* src)->std::string {
			auto& path = *(std::filesystem::path*)src;
			if (path.native().starts_with(L"0x"))
				return "";
			return Path::rebase(base_path, path).string();
		};

		serialize_xml(*ui, obj, doc_root, spec);
		doc.save_file(filename.c_str());
	}

	uint frames = 1;
	uint fps = 0;
	float delta_time = 0.f;
	float total_time = 0.f;
	bool app_exiting = false;

	static uint64 last_time = 0;
	static float fps_delta = 0.f;
	static uint fps_counting = 0;

	struct Event
	{
		bool dead = false;
		float time_interval;
		float time_counter;
		uint frames_interval;
		int frames_counter;
		std::function<bool()> callback;
	};

	static std::vector<std::unique_ptr<Event>> events;
	static std::recursive_mutex event_mtx;

	static const uint64 counter_freq = performance_frequency();
	static const auto limited_fps = 60;

	int run(const std::function<bool()>& callback)
	{
		if (!callback)
		{
			for (;;)
			{
				MSG msg;
				while (GetMessageW(&msg, NULL, 0, 0))
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
			}
		}

		auto windows = NativeWindow::list();
		if (windows.empty())
			return 1;

		last_time = performance_counter();
		frames = 0;

		for (;;)
		{
			MSG msg;
			while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}

			for (auto it = windows.begin(); it != windows.end(); )
			{
				auto w = *it;
				if (w->dead)
				{
					it = windows.erase(it);
					delete w;
				}
				else
					it++;
			}

			if (windows.empty())
			{
				app_exiting = true;
				return 0;
			}

			process_events();

			if (!callback())
			{
				app_exiting = true;
				return 0;
			}

			for (auto w : windows)
				w->has_input = false;

			frames++;
			auto et = last_time;
			last_time = performance_counter();
			et = last_time - et;
			delta_time = (double)et / (double)counter_freq;
			total_time += delta_time;
			fps_counting++;
			fps_delta += delta_time;
			if (fps_delta >= 1.f)
			{
				fps = fps_counting;
				fps_counting = 0;
				fps_delta = 0.f;
			}

			if (delta_time < 1.f / limited_fps)
				sleep((1.f / limited_fps - delta_time) * 1000);
		}
	}

	void* add_event(const std::function<bool()>& callback, float time, uint frames)
	{
		std::lock_guard<std::recursive_mutex> lock(event_mtx);
		auto e = new Event;
		e->time_interval = time;
		e->time_counter = time;
		e->frames_interval = frames;
		e->frames_counter = frames;
		e->callback = callback;
		events.emplace_back(e);
		return e;
	}

	void reset_event(void* _ev)
	{
		std::lock_guard<std::recursive_mutex> lock(event_mtx);
		auto ev = (Event*)_ev;
		ev->time_counter = ev->time_interval;
		ev->frames_counter = ev->frames_interval;
	}

	void remove_event(void* ev)
	{
		std::lock_guard<std::recursive_mutex> lock(event_mtx);
		for (auto it = events.begin(); it != events.end(); it++)
		{
			if (it->get() == ev)
			{
				events.erase(it);
				break;
			}
		}
	}

	void clear_events()
	{
		std::lock_guard<std::recursive_mutex> lock(event_mtx);
		events.clear();
	}

	void process_events()
	{
		static std::vector<Event*> _events;
		std::lock_guard<std::recursive_mutex> lock(event_mtx);
		_events.resize(events.size());
		for (auto i = 0; i < events.size(); i++)
			_events[i] = events[i].get();
		for (auto e : _events)
		{
			e->time_counter -= delta_time;
			e->frames_counter -= 1;
			if (e->time_counter <= 0 && e->frames_counter <= 0)
			{
				if (!e->callback())
					e->dead = true;
				else
				{
					e->time_counter = e->time_interval;
					e->frames_counter = e->frames_interval;
				}
			}
		}
		for (auto it = events.begin(); it != events.end();)
		{
			if ((*it)->dead)
				it = events.erase(it);
			else
				it++;
		}
	}

	struct _Initializer
	{
		_Initializer()
		{
			printf("foundation init\n");

			auto p = getenv("FLAME_PATH");
			assert(p);
			std::filesystem::path engine_path = p;
			engine_path.make_preferred();
			engine_path /= L"assets";
			Path::set_root(L"flame", engine_path);

			add_event([]() {
				init_typeinfo();
				init_library();
				return false;
			});
		}
	};
	static _Initializer _initializer;
}
