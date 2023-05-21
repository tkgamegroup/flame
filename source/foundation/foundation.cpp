#include "foundation_private.h"
#include "system_private.h"
#include "window_private.h"
#include "application.h"

#include <exprtk.hpp>

namespace flame
{
	std::map<std::wstring, std::filesystem::path> Path::roots;
	std::map<std::filesystem::path, AssetManagemant::Asset> AssetManagemant::assets;

	struct _Initializer
	{
		_Initializer()
		{
			auto p = getenv("FLAME_PATH");
			assert(p);
			std::filesystem::path engine_path = p;
			engine_path.make_preferred();
			engine_path /= L"assets";
			Path::set_root(L"flame", engine_path);
		}
	};
	static _Initializer _initializer;

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

		using exprtk::igeneric_function<float>::operator();

		exprtk_output_t()
		{
			exprtk::enable_zero_parameters(*this);
		}

		inline float operator() (igeneric_function<float>::parameter_list_t parameters)
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

	struct ExpressionPrivate : Expression
	{
		exprtk::symbol_table<float> symbols;
		exprtk::expression<float> expression;
		std::string return_value;
		exprtk_output_t output_function;

		ExpressionPrivate(const std::string& expression_string);

		void set_const_value(const std::string& name, float value) override;
		void set_variable(const std::string& name, float* variable) override;
		void set_const_string(const std::string& name, const std::string& value) override;
		void compile() override;
		std::string get_value() override;
	};

	ExpressionPrivate::ExpressionPrivate(const std::string& _expression_string)
	{
		expression_string = _expression_string;
		expression.register_symbol_table(symbols);
		auto addr_str = str((uint64)&return_value);
		symbols.create_stringvar("return_value", addr_str);
		output_function.return_value = &return_value;
		symbols.add_function("output", output_function);
	}

	void ExpressionPrivate::set_const_value(const std::string& name, float value)
	{
		symbols.add_constant(name, value);
	}

	void ExpressionPrivate::set_variable(const std::string& name, float* variable)
	{
		symbols.add_variable(name, *variable);
	}

	void ExpressionPrivate::compile()
	{
		exprtk::parser<float> parser; 
		parser.compile(expression_string, expression);

	}

	void ExpressionPrivate::set_const_string(const std::string& name, const std::string& value)
	{
		symbols.create_stringvar(name, value);
	}

	std::string ExpressionPrivate::get_value()
	{
		auto float_ret = expression.value();
		if (!return_value.empty())
			return return_value;
		return str(float_ret);
	}

	Expression* Expression::create(const std::string& expression_string)
	{
		return new ExpressionPrivate(expression_string);
	}

	uint frames = 0;
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
}
