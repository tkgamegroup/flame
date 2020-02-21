#include <flame/foundation/foundation.h>
#include <flame/sound/device.h>
#include <flame/sound/context.h>
#include <flame/sound/buffer.h>
#include <flame/sound/source.h>

using namespace flame;

int main(int argc, char** args)
{
	auto recorder = sound::Device::create_recorder();
	auto data = new uchar[sound::sound_size(1.f)];
	recorder->start_record();
	sleep(1000);
	recorder->stop_record(data);

	auto player = sound::Device::create_player();
	auto context = sound::Context::create(player);
	context->make_current();
	auto buffer = sound::Buffer::create_from_data(data);
	auto source = sound::Source::create(buffer);
	source->play();

	sleep(1000);

	return 0;
}
