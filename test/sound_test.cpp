#include <flame/foundation/foundation.h>
#include <flame/sound/device.h>
#include <flame/sound/context.h>
#include <flame/sound/buffer.h>
#include <flame/sound/source.h>

using namespace flame;

int main(int argc, char** args)
{
	auto recorder = sound::Device::create_recorder();
	auto player = sound::Device::create_player();
	auto context = sound::Context::create(player);
	context->make_current();

	auto data = new char[sound::get_size(1.f)];

	const auto TIME = 1000;
	while (true)
	{
		recorder->start_record();
		sleep(TIME);
		recorder->stop_record(data);
		auto buffer = sound::Buffer::create_from_data(data);
		auto source = sound::Source::create(buffer);
		source->play();
		sleep(TIME);
		source->stop();
		sound::Source::destroy(source);
		sound::Buffer::destroy(buffer);
	}

	return 0;
}
