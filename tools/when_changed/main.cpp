#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

using namespace flame;

static uint64 last_change_time = 0;

int main(int argc, char **args)
{
	if (argc != 3)
	{
		printf("usage:\n  filename \"command\"\nnote:\n  command must be wraped in \"\"\n");
		system("pause");
		return 0;
	}

	printf("watch=\"%s\" cmd=\"%s\"\n", args[1], args[2]);

	struct Capturing
	{
		char path[256];
		char cmd[256];
	}capture;
	strcpy_s(capture.path, args[1]);
	strcpy_s(capture.cmd, args[2]);
	add_file_watcher(std::filesystem::path(args[1]).parent_path().c_str(), [](Capture& c, FileChangeType type, const wchar_t* filename) {
		auto& capture = c.data<Capturing>();

		auto now_time = get_now_ns();
		if (std::filesystem::path(capture.path) == filename && now_time - last_change_time > 1'000'000'000)
		{
			printf("changed\n");

			system(capture.cmd);
		}
		last_change_time = now_time;

	}, Capture().set_data(&capture), false);
}
