#include <flame/script/script.h>

struct lua_State;

namespace flame
{
	namespace script
	{
		struct InstancePrivate : Instance
		{
			lua_State* lua_state = nullptr;

			void* assert_callback = nullptr;

			InstancePrivate();
			~InstancePrivate();

			void push_int(int i) override;
			void push_float(float f) override;
			void push_vec2(const vec2& v) override;
			void push_vec3(const vec3& v) override;
			void push_vec4(const vec4& v) override;
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
			bool excute(const char* str) override;
			bool excute_file(const wchar_t* filename) override;
		};

		extern thread_local InstancePrivate* default_instance;
	}
}
