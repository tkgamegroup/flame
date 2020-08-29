#include <flame/script/script.h>

struct lua_State;

namespace flame
{
	namespace script
	{
		struct InstanceBridge : Instance
		{
			bool excute(const wchar_t* filename) override;
		};

		struct InstancePrivate : InstanceBridge
		{
			lua_State* lua_state = nullptr;

			InstancePrivate();

			bool check_result(int res);
			bool excute(const std::filesystem::path& filename);
			void add_object(void* p, const char* name, const char* type_name) override;
			void call_slot(uint s, uint parameters_count, Parameter* parameters) override;
			void release_slot(uint s) override;
		};

		inline bool InstanceBridge::excute(const wchar_t* filename)
		{
			return ((InstancePrivate*)this)->excute(filename);
		}
	}
}
