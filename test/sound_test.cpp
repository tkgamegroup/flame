#include <flame/foundation/foundation.h>
#include <flame/sound/device.h>
#include <flame/sound/context.h>
#include <flame/sound/buffer.h>
#include <flame/sound/source.h>

using namespace flame;

int main(int argc, char** args)
{
	auto recorder = sound::Device::create(sound::DeviceRecord);
	recorder->start_record();
	sleep(1000);
	auto samples = recorder->get_recorded_samples();
	auto data = new ushort[samples * 2];
	recorder->get_recorded_data(data, samples);
	recorder->stop_record();

	auto device = sound::Device::create(sound::DevicePlay);
	auto context = sound::Context::create(device);
	context->make_current();
	auto buffer = sound::Buffer::create_from_data(samples * 4, data);
	auto source = sound::Source::create(buffer);
	source->play();

	sleep(1000);

	return 0;
}
