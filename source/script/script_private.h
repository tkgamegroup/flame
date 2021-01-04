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

			void* assert_callback = nullptr;

			InstancePrivate();
			~InstancePrivate();

			bool excute(const std::filesystem::path& filename);
			void push_string(const char* value, const char* member_name = nullptr) override;
			void push_object(const char* member_name = nullptr) override;
			void set_global_name(const char* name) override;
			void set_object_type(const char* type_name) override;
		};

		inline bool InstanceBridge::excute(const wchar_t* filename)
		{
			return ((InstancePrivate*)this)->excute(filename);
		}

		extern thread_local InstancePrivate* default_instance;
	}
}
