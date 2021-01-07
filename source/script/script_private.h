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
			void push_string(const char* value) override;
			void push_pointer(void* p) override;
			void push_object() override;
			void pop(uint number) override;
			void get_global(const char* name) override;
			void get_member(const char* name, uint idx = 0) override;
			void set_object_type(const char* type_name) override;
			void set_member_name(const char* name) override;
			void set_global_name(const char* name) override;
			void call(uint parameters_count) override;
		};

		inline bool InstanceBridge::excute(const wchar_t* filename)
		{
			return ((InstancePrivate*)this)->excute(filename);
		}

		extern thread_local InstancePrivate* default_instance;
	}
}
