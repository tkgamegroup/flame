#include <flame/foundation/foundation.h>
#include <flame/sound/device.h>
#include <flame/sound/context.h>
#include <flame/sound/buffer.h>
#include <flame/sound/source.h>

using namespace flame;

int main(int argc, char** args)
{
	auto player = sound::Device::create_player();
	auto context = sound::Context::create(player);
	context->make_current();

	std::vector<float> samples;
	samples.resize(sound::get_sample_count(1.f, 44100U));
	auto size = sound::get_size(samples.size(), true, true);
	auto data = new char[size];
	sound::wave(samples, 44100U, sound::WavaSin, 440U);
	sound::fill_data(data, 44100U, true, true, samples, 0.1f);
	auto buffer = sound::Buffer::create_from_data(data);
	auto source = sound::Source::create(buffer);
	source->play();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	return 0;
}
