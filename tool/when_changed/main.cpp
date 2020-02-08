#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

using namespace flame;

static ulonglong last_change_time = 0;

int main(int argc, char **args)
{
	if (argc != 3)
	{
		printf("usage:\n  filename \"command\"\nnote:\n  command must be wraped in \"\"\n");
		system("pause");
		return 0;
	}

	printf("watch=\"%s\" cmd=\"%s\"\n", args[1], args[2]);

	struct Capture
	{
		std::filesystem::path p;
		std::string c;
	}capture;
	capture.p = args[1];
	capture.c = args[2];
	add_file_watcher(std::filesystem::path(args[1]).parent_path().c_str(), [](void* c, FileChangeType type, const std::wstring& filename) {
		auto& capture = *(Capture*)c;

		auto now_time = get_now_ns();
		if (capture.p == filename && now_time - last_change_time > 1'000'000'000)
		{
			printf("changed\n");

			system(capture.c.c_str());
		}
		last_change_time = now_time;

	}, new_mail(&capture), false);
}
