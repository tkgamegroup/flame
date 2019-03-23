// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
