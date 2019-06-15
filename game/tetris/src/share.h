#pragma once

#include <flame/graphics/device.h>
#include <flame/UI/instance.h>
#include <flame/sound/device.h>
#include <flame/sound/context.h>
#include <flame/sound/buffer.h>
#include <flame/sound/source.h>

extern flame::graphics::Device *d;
extern flame::UI::Instance *ui;
extern flame::sound::Device *snd_d;
extern flame::sound::Context *snd_c;
extern flame::sound::Buffer *snd_buf_select;
extern flame::sound::Buffer *snd_buf_ok;
extern flame::sound::Buffer *snd_buf_back;
extern flame::sound::Buffer *snd_buf_mode;
extern flame::sound::Buffer *snd_buf_success;
extern flame::sound::Source *snd_src_select;
extern flame::sound::Source *snd_src_ok;
extern flame::sound::Source *snd_src_back;
extern flame::sound::Source *snd_src_mode;
extern flame::sound::Source *snd_src_success;
